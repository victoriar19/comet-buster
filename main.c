#include <SDL.h>
#include <SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "sprite.h"
#include "collider.h"
#include "linkedlist.h"
#include "scores.h"

/* ---- Reglages du jeu ---- */
#define MODE_RECUL          1     /* 1 = on avance seulement grace au recul des tirs */
#define RECOIL              0.8   /* poussee (vers l'arriere) a chaque tir */
#if MODE_RECUL
#define SPACESHIP_FRICTION  0.01  /* un peu de frottement pour pouvoir freiner */
#else
#define SPACESHIP_FRICTION  0.002
#endif
#define SPACESHIP_BOOST     0.25
#define BULLET_LIFETIME     25
#define BULLET_SPEED        10
#define DEFAULT_PTSIZE      24
#define NUMBER_OF_LIFES     5
#define REC_SIZE            500   /* 500 images = 10 s (une image = 20 ms) */
#define SCORE_FILE          "scores.txt"
#include "level.h"

//#define GDB()  __asm__("int $0x3")   /* desactive : la touche D faisait planter le jeu */

bool gameover;
SDL_Surface *screen;
sprite_t sprite_ship;
list_ptr l_sprite_bullet;
list_ptr l_sprite_comet;
list_ptr l_sprite_explosion;
list_ptr l_sprite_text;
list_ptr l_sprite_life_counter;
list_ptr l_score_el = NULL;

bool shoot_again;
int score;
int level = LEVEL_MIN;

/* ---- Nyan cat a escorter ---- */
sprite_t nyan = NULL;
int nyan_timer = 0;

/* ---- Fantome (rejeu de la vie precedente) ---- */
sprite_t ghost = NULL;
int rec_x[REC_SIZE], rec_y[REC_SIZE], rec_a[REC_SIZE];   /* enregistrement en cours */
int rec_pos = 0;          /* prochaine case a ecrire */
int rec_count = 0;        /* nombre d'images enregistrees (max REC_SIZE) */
int ghost_x[REC_SIZE], ghost_y[REC_SIZE], ghost_a[REC_SIZE];   /* rejeu */
int ghost_len = 0;
int ghost_index = 0;
bool ghost_active = false;

/* ---- Statistiques de fin de partie ---- */
int shots_fired = 0;
int hits = 0;
int comets_destroyed = 0;
int nyan_points = 0;

/* Declaration of few prototypes because there is no .h file with them */
int init_sdl(void);
void general_events(char *keys);
void game_events(char *keys);
void draw_explosion(int i,int j);
void draw_fire(void);
void draw_life_counter(void);
void draw_score(TTF_Font *font);
void next_level(TTF_Font *font);
void draw_sprites(list_ptr *l_sprite);
void split(sprite_t old_comet, list_ptr **l_sprite_comet, enum sprite_type new_type);
void split_and_score(list_ptr element, list_ptr *l_sprite_comet, bool update_score);
void lose_life(void);
void ship_destroyed(void);
void start_level(void);
void new_nyan(void);
void play_nyan(TTF_Font *font);
void record_frame(void);
void start_ghost(void);
void play_ghost(TTF_Font *font);
void ask_nickname(char *nickname);
void end_of_game(unsigned int start_ticks);

/* SDL Initialisation. Create windows and so on
 *  return 0 if everything is ok, otherwise 1.
 * */
int init_sdl(void) {
  /* initialize SDL */
  SDL_Init(SDL_INIT_VIDEO);
  /* set the title bar */
  SDL_WM_SetCaption("Comet buster", "w00t!");
  /* create window */
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);
  if (!screen)
    return 1;
  /* Initialize the TTF library a nd open the font */
  if ( TTF_Init() < 0 ) {
    return 1;
  }

  /* set keyboard repeat */
  SDL_EnableKeyRepeat(50, 50);
  return 0;
}


/* general SDL events hanlder
 * */
void general_events(char* keys) {
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        gameover = true;
        break;
      case SDL_KEYUP:
        keys[event.key.keysym.sym] = 0;
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
          case SDLK_q:
            gameover = true;
            break;
          default:
            break;
        }
        keys[event.key.keysym.sym] = 1;
        break;
      default:
        break;
    }
  }
}


/* In game event handler
 * */
void game_events(char* key) {
  /* en mode recul, pas de moteur : seuls les tirs font avancer le vaisseau */
  if (!MODE_RECUL) {
    if (key[SDLK_UP]) {
      sprite_boost(sprite_ship, SPACESHIP_BOOST);
    }
    if (key[SDLK_DOWN]) {
      sprite_boost(sprite_ship, -SPACESHIP_BOOST);
    }
  }
  if (key[SDLK_LEFT]) {
    sprite_turn_left(sprite_ship);
  }
  if (key[SDLK_RIGHT]) {
    sprite_turn_right(sprite_ship);
  }
  if (key[SDLK_SPACE]) {
    if (shoot_again) {
      draw_fire();
      shoot_again = false;
    }
  } else {
    shoot_again = true;
  }
}

/* Add a new explosion to the list
 * */
void draw_explosion(int i,int j) {
  sprite_t sprite;
  int colorkey = SDL_MapRGB(screen->format, 255, 0, 255);
  sprite = sprite_new(EXPLOSION, "sprites/explosion17.bmp", colorkey, 64, 25, 0, i-32, j-32, 0., 0., 0.);
  sprite->lifetime = 25;
  l_sprite_explosion = list_add(sprite, l_sprite_explosion);
}


/* Launch a bullet ;)
 *  Add the bullet to the list
 *  Le tir repousse le vaisseau vers l'arriere (mode recul)
 * */
void draw_fire(void) {
  int colorkey;
  float dir_angle;
  sprite_t sprite;
  colorkey = SDL_MapRGB(screen->format, 255, 125, 0);
  dir_angle = 2.*PI*sprite_ship->anim_sprite_num/sprite_ship->anim_sprite_num_max;
  sprite = sprite_new(BULLET, "sprites/bullet02.bmp", colorkey, 4, 1, 0, sprite_ship->rc_screen_xy.x+sprite_ship->size/2*(1+cos(dir_angle)), sprite_ship->rc_screen_xy.y+sprite_ship->size/2*(1-sin(dir_angle)), BULLET_SPEED*cos(dir_angle), BULLET_SPEED*(-sin(dir_angle)), 0.);
  sprite->lifetime = BULLET_LIFETIME;
  l_sprite_bullet = list_add(sprite, l_sprite_bullet);
  shots_fired++;
  if (MODE_RECUL) {
    sprite_boost(sprite_ship, -RECOIL);
  }
}

/* Draw the score sprite
 *  Add it to the list
 * */
void draw_score(TTF_Font * font) {

  SDL_Surface *score_surf;
  SDL_Color score_color = {255, 255, 255, 0};
  char score_text[1024];
  sprite_t sprite;

  sprintf(score_text, "%08d", score);
  score_surf = TTF_RenderText_Solid(font, score_text, score_color);
  sprite = sprite_new_text(score_surf, 5, 5);
  if (l_score_el)
    list_remove(l_score_el, &l_sprite_text);
  l_score_el = l_sprite_text = list_add(sprite, l_sprite_text);
}

/* Draw the life counter sprites
 *  Add sprites to the list
 * */
void draw_life_counter(void) {
  int x = 5;
  int i;
  sprite_t sprite;
  int colorkey = SDL_MapRGB(screen->format, 255, 0, 255);

  /* create and initialize the life counter sprite */
  l_sprite_life_counter = list_new();
  for (i = 0; i < NUMBER_OF_LIFES; i++) {
    sprite = sprite_new(LIFE_COUNTER, "sprites/grayship_16x16.bmp", colorkey, 16, 1, 0, x, 35, 0., 0., 0.);
    l_sprite_life_counter = list_add(sprite, l_sprite_life_counter);
    x += 15;
  }
}

/* Draw the new level ban,
 *  and level up
 * */
void next_level( TTF_Font * font) {
  SDL_Color text_color = {255, 255, 0, 0};//R,G,B,A
  char text[1024];
  sprite_t sprite_text;

  printf("DEBUG: Congratulation! You won the level %d (score=%d)\n",level,score);
  fflush(stdout);
  level++;
  printf("DEBUG: Start level %d, good luck!\n",level);
  fflush(stdout);
  // Add the "next level" text sprite to the text sprite list
  sprintf(text, "LEVEL %d", level);
  SDL_Surface *text_surf = TTF_RenderText_Solid(font, text, text_color);
  sprite_text = sprite_new_text(text_surf, SCREEN_WIDTH/2-60, SCREEN_HEIGHT/3);
  sprite_text->lifetime = 150;
  l_sprite_text = list_add(sprite_text, l_sprite_text);
}


/* Blit a sprites list.
 *  Depending on the sprite type, we had physics & effects
 * */
void draw_sprites(list_ptr *l_sprite) {
  sprite_t sprite;
  list_ptr l_ptr = *(l_sprite);

  // nothing to draw
  if (list_is_empty(l_ptr))
    return;

  // loop throught the list and draw
  while (!list_is_empty(l_ptr)) {
    sprite = list_head_sprite(l_ptr);
    list_ptr list_el = l_ptr;

    l_ptr = list_next(l_ptr);

    // let's take some age if needed
    if (sprite_can_die(sprite)) {
      sprite_get_older(sprite);

      if (sprite_is_dead(sprite)) {
        list_remove(list_el, l_sprite);
        continue;
      }
    }

    if (sprite) {
      if (sprite_is_ennemy(sprite)||sprite->type==BULLET) {
        sprite_play_physics(sprite);
      }

      // comet are dangerous, they rotate on themself to maximize damages
      if (sprite_is_comet(sprite)) {
        sprite_turn_right(sprite);
      }

      // explosions effect is simple, simply add a rotation :)
      if (sprite->type == EXPLOSION) {
        sprite_turn_left(sprite);
      }

      // TXT sprite don't provide animation files, so provide a special blit
      if (sprite->type == TXT)
        SDL_BlitSurface(sprite->sprite, NULL, screen, &sprite->rc_screen_xy);
      else
        SDL_BlitSurface(sprite->sprite, &sprite->rc_anim_xy, screen, &sprite->rc_screen_xy);
    }
  }
}


/* Handle consequences of a collision:
 *   increase score depending on the sprite->type
 *   split object if needed
 * */
void split_and_score(list_ptr element, list_ptr *l_sprite_comet, bool update_score) {
  //score + split
  int diff = 0;
  sprite_t data = list_head_sprite(element);
  switch (data->type) {
    case L_COMET:
      diff = 20;
      split(data, &l_sprite_comet, M_COMET);
      break;
    case M_COMET:
      diff = 50;
      split(data, &l_sprite_comet, S_COMET);
      break;
    case S_COMET:
      diff = 100;
      break;
    case ET:
      diff = 60;
      break;
    default:
      break;
  }
  score += (update_score)?diff:0;
}

/* Split an asteroid into two new objects which get most of parameters from
 * their parent (old_comet). They are also added to the list of comets.
 * */
void split(sprite_t old_comet, list_ptr **l_sprite_comet, enum sprite_type new_type) {
  float speed = get_base_speed(level, new_type);
  float angle = (float)(rand()%360)/360*2*PI;   /* float (etait un int : angle tronque) */
  int colorkey = old_comet->colorkey;
  int x = old_comet->x;
  int y = old_comet->y;
  int new_sprite_size = old_comet->size/2; //64 32 26
  const char * fname = get_comet_sprite(level, new_type);
  sprite_t first = sprite_new(new_type, fname, colorkey, new_sprite_size, 32, 0, x, y, speed*cos(angle), speed*sin(angle), 0.);
  //change angle for the second asteroid
  angle = (float)(rand()%360)/360*2*PI;
  sprite_t second = sprite_new(new_type, fname, colorkey, new_sprite_size, 32, 0, x, y, speed*cos(angle), speed*sin(angle), 0.);
  **l_sprite_comet = list_add(first, **l_sprite_comet);
  **l_sprite_comet = list_add(second, **l_sprite_comet);
}

/* ------------------------------------------------------------------ */
/*                           Vies                                     */
/* ------------------------------------------------------------------ */

/* Retire une vie (une icone) ou termine la partie s'il n'en reste plus */
void lose_life(void) {
  if (!list_is_empty(l_sprite_life_counter)) {
    sprite_t dead_sprite = list_pop_sprite(&l_sprite_life_counter);
    if (dead_sprite)
      sprite_free(dead_sprite);
  } else {
    printf(" ============ Game Over ============= \n");
    printf("Score: you reached level %d with %d points\n", level, score);
    fflush(stdout);
    gameover = true;
  }
}

/* Le vaisseau est detruit : son dernier trajet devient un fantome,
 * puis il repart du centre de l'ecran */
void ship_destroyed(void) {
  start_ghost();
  sprite_ship->x = sprite_ship->rc_screen_xy.x = SPACESHIP_INIT_X;
  sprite_ship->y = sprite_ship->rc_screen_xy.y = SPACESHIP_INIT_Y;
  sprite_ship->vx = 0.;
  sprite_ship->vy = 0.;
  lose_life();
}

/* ------------------------------------------------------------------ */
/*                 Niveau : fichier texte ou generation               */
/* ------------------------------------------------------------------ */

/* Nouveau Nyan cat a proteger (l'ancien est libere) */
void new_nyan(void) {
  if (nyan != NULL)
    sprite_free(nyan);
  nyan = gen_nyancat_sprite(level, screen);
  nyan_timer = 0;
}

/* Prepare le niveau courant : fichier levels/levelN.txt s'il existe,
 * sinon generation automatique. Puis on fait apparaitre le Nyan cat. */
void start_level(void) {
  if (!load_level_file(level, &l_sprite_comet, screen)) {
    gen_level(level, &l_sprite_comet, screen);
  }
  new_nyan();
}

/* ------------------------------------------------------------------ */
/*                     Escorte du Nyan cat                            */
/* ------------------------------------------------------------------ */

/* Deplace et dessine le Nyan cat avec son arc-en-ciel.
 * Chaque seconde survecue (50 images) rapporte 10 points. */
void play_nyan(TTF_Font *font) {
  int rainbow[6][3] = { {255,0,0}, {255,150,0}, {255,255,0}, {0,255,0}, {0,150,255}, {150,0,255} };
  SDL_Rect dot;
  int k;

  sprite_play_physics(nyan);

  /* traine arc-en-ciel : 6 petits carres derriere le chat */
  for (k = 0; k < 6; k++) {
    dot.x = (int)(nyan->x - nyan->vx * 4 * (k + 1)) + nyan->size / 2;
    dot.y = (int)(nyan->y - nyan->vy * 4 * (k + 1)) + 16;
    dot.w = 6;
    dot.h = 6;
    SDL_FillRect(screen, &dot, SDL_MapRGB(screen->format, rainbow[k][0], rainbow[k][1], rainbow[k][2]));
  }
  SDL_BlitSurface(nyan->sprite, &nyan->rc_anim_xy, screen, &nyan->rc_screen_xy);

  nyan_timer++;
  if (nyan_timer % 50 == 0) {
    score += 10;
    nyan_points += 10;
    draw_score(font);
  }
}

/* ------------------------------------------------------------------ */
/*                           Fantome                                  */
/* ------------------------------------------------------------------ */

/* Memorise la position et l'orientation du vaisseau (tableau circulaire :
 * quand il est plein, on ecrase les plus anciennes images) */
void record_frame(void) {
  rec_x[rec_pos] = sprite_ship->rc_screen_xy.x;
  rec_y[rec_pos] = sprite_ship->rc_screen_xy.y;
  rec_a[rec_pos] = sprite_ship->anim_sprite_num;
  rec_pos = (rec_pos + 1) % REC_SIZE;
  if (rec_count < REC_SIZE)
    rec_count++;
}

/* Copie l'enregistrement dans l'ordre chronologique et lance le fantome */
void start_ghost(void) {
  int i;
  int start = (rec_count < REC_SIZE) ? 0 : rec_pos;   /* case la plus ancienne */
  for (i = 0; i < rec_count; i++) {
    ghost_x[i] = rec_x[(start + i) % REC_SIZE];
    ghost_y[i] = rec_y[(start + i) % REC_SIZE];
    ghost_a[i] = rec_a[(start + i) % REC_SIZE];
  }
  ghost_len = rec_count;
  ghost_index = 0;
  ghost_active = (ghost_len > 0);
  /* on recommence un enregistrement vide pour la prochaine vie */
  rec_pos = 0;
  rec_count = 0;
}

/* Rejoue une image du fantome. Il detruit les comettes qu'il touche. */
void play_ghost(TTF_Font *font) {
  list_ptr l_ptr;
  int cu, cv;

  if (!ghost_active)
    return;

  ghost->rc_screen_xy.x = ghost_x[ghost_index];
  ghost->rc_screen_xy.y = ghost_y[ghost_index];
  ghost->anim_sprite_num = ghost_a[ghost_index];
  ghost->rc_anim_xy.x = ghost->anim_sprite_num * ghost->size;
  SDL_BlitSurface(ghost->sprite, &ghost->rc_anim_xy, screen, &ghost->rc_screen_xy);

  l_ptr = l_sprite_comet;
  while (!list_is_empty(l_ptr)) {
    sprite_t comet = list_head_sprite(l_ptr);
    list_ptr list_el = l_ptr;
    l_ptr = list_next(l_ptr);

    if (collide_test(ghost, comet, screen->format, &cu, &cv)) {
      draw_explosion(cu, cv);
      split_and_score(list_el, &l_sprite_comet, true);
      list_remove(list_el, &l_sprite_comet);
      comets_destroyed++;
      draw_score(font);
    }
  }

  ghost_index++;
  if (ghost_index >= ghost_len)
    ghost_active = false;
}

/* ------------------------------------------------------------------ */
/*                     Fin de partie : scores                         */
/* ------------------------------------------------------------------ */

/* Demande le pseudo dans le terminal (SDL 1.2 n'a pas de champ de saisie) */
void ask_nickname(char *nickname) {
  int i;
  printf("Ton pseudo : ");
  fflush(stdout);
  if (fgets(nickname, NAME_LEN, stdin) == NULL) {
    strcpy(nickname, "anonyme");
  }
  /* on enleve le retour a la ligne, et les ':' qui casseraient le format */
  for (i = 0; nickname[i] != '\0'; i++) {
    if (nickname[i] == '\n')
      nickname[i] = '\0';
    else if (nickname[i] == ':')
      nickname[i] = '_';
  }
  if (nickname[0] == '\0') {
    strcpy(nickname, "anonyme");
  }
}

/* Statistiques, puis sauvegarde et affichage du classement */
void end_of_game(unsigned int start_ticks) {
  entry_t table[MAX_SCORES];
  char nickname[NAME_LEN];
  int n;
  int accuracy = (shots_fired > 0) ? (100 * hits) / shots_fired : 0;
  unsigned int seconds = (SDL_GetTicks() - start_ticks) / 1000;

  printf("\n======== STATISTIQUES ========\n");
  printf("Niveau atteint      : %d\n", level);
  printf("Score               : %d\n", score);
  printf("Tirs                : %d\n", shots_fired);
  printf("Touches             : %d\n", hits);
  printf("Precision           : %d%%\n", accuracy);
  printf("Comettes detruites  : %d\n", comets_destroyed);
  printf("Points du Nyan cat  : %d\n", nyan_points);
  printf("Temps de jeu        : %u s\n\n", seconds);

  ask_nickname(nickname);
  n = scores_load(SCORE_FILE, table);
  n = scores_add(table, n, nickname, score);
  scores_sort(table, n);
  scores_save(SCORE_FILE, table, n);
  scores_print_top(table, n, 10);
}


int main(int argc, char* argv[]) {
  SDL_Surface *temp, *bg;
  SDL_Rect rcBg;
  int colorkey;
  bool collide;
  int cu, cv;
  sprite_t current_sprite;
  int ret;
  TTF_Font * font_score;
  TTF_Font * font_next_level;
  unsigned int start_ticks;

  (void)argc;
  (void)argv;

  ret = init_sdl();
  if (ret) {
    perror("Error init_sdl: ");
    SDL_Quit();
    return(ret);
  }
  srand(time(NULL));    /* une seule fois, ici (avant : a chaque niveau) */

  // default colorkey
  colorkey = SDL_MapRGB(screen->format, 255, 0, 255);

  // fonts
  font_score = TTF_OpenFont("fonts/LinLibertine_DR.ttf", 24);
  font_next_level = TTF_OpenFont("fonts/LinLibertine_DR.ttf", 36);
  if (font_score == NULL || font_next_level == NULL) {
    printf("Police introuvable : fonts/LinLibertine_DR.ttf\n");
    TTF_Quit();
    SDL_Quit();
    return 1;
  }

  // create the text sprites list
  l_sprite_text = list_new();

  // initialize score and score sprite
  score = 0;
  draw_score(font_score);
  draw_life_counter();

  //create and initialize the spaceship sprite
  sprite_ship = sprite_new(SHIP, "sprites/greenship-v1.bmp", colorkey, 32, 36, 9, SPACESHIP_INIT_X, SPACESHIP_INIT_Y, 0., 0., SPACESHIP_FRICTION);

  // le fantome : meme image que le vaisseau, semi-transparent
  ghost = sprite_new(SHIP, "sprites/greenship-v1.bmp", colorkey, 32, 36, 9, 0, 0, 0., 0., 0.);
  SDL_SetAlpha(ghost->sprite, SDL_SRCALPHA, 90);

  /* comets + nyan cat du premier niveau */
  l_sprite_comet = list_new();
  start_level();

  //create the bullets list
  l_sprite_bullet = list_new();

  //load background sprite
  temp = SDL_LoadBMP("sprites/backgroundlvl1.bmp");
  if (temp == NULL) {
    printf("Fond introuvable : sprites/backgroundlvl1.bmp\n");
    SDL_Quit();
    return 1;
  }
  bg = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);
  rcBg.x = 0;
  rcBg.y = 0;

  gameover = false;
  shoot_again = true;
  start_ticks = SDL_GetTicks();

  char key[SDLK_LAST] = {0};
  unsigned int lasttime = 0;
  /* message pump */
  while (!gameover) {
    lasttime = SDL_GetTicks();
    list_ptr l_ptr;

    general_events(key);
    game_events(key);

    /* draw the background */
    SDL_BlitSurface(bg, NULL, screen, &rcBg);

    // draw the text sprites
    draw_sprites(&l_sprite_text);

    // draw the life counter sprites
    draw_sprites(&l_sprite_life_counter);

    /* play & draw the spaceship sprite */
    sprite_play_physics(sprite_ship);
    SDL_BlitSurface(sprite_ship->sprite, &sprite_ship->rc_anim_xy, screen, &sprite_ship->rc_screen_xy);
    record_frame();

    // draw comets
    draw_sprites(&l_sprite_comet);
    // le Nyan cat (a proteger) et le fantome
    play_nyan(font_score);
    play_ghost(font_score);
    // draw bullets
    draw_sprites(&l_sprite_bullet);

    /* collide tests ship <-> comets */
    l_ptr = l_sprite_comet;
    while (!list_is_empty(l_ptr)) {
      current_sprite = list_head_sprite(l_ptr);
      list_ptr list_el = l_ptr;
      l_ptr = list_next(l_ptr);

      collide = collide_test(sprite_ship, current_sprite, screen->format, &cu, &cv);
      if (collide) {
        draw_explosion(cu,cv);
        //no additional score if the ship is destroyed
        split_and_score(list_el, &l_sprite_comet, false);
        list_remove(list_el, &l_sprite_comet);
        ship_destroyed();
      }
    }

    /* collide tests comets <-> Nyan cat : le chat touche = une vie perdue */
    l_ptr = l_sprite_comet;
    while (!list_is_empty(l_ptr)) {
      current_sprite = list_head_sprite(l_ptr);
      list_ptr list_el = l_ptr;
      l_ptr = list_next(l_ptr);

      if (collide_test(nyan, current_sprite, screen->format, &cu, &cv)) {
        draw_explosion(cu, cv);
        split_and_score(list_el, &l_sprite_comet, false);
        list_remove(list_el, &l_sprite_comet);
        lose_life();
        new_nyan();
      }
    }

    /* collide tests bullets <-> comets */
    l_ptr = l_sprite_bullet;
    while (!list_is_empty(l_ptr)) {
      current_sprite = list_head_sprite(l_ptr);
      list_ptr list_el = l_ptr;
      list_ptr l_ptr_c = l_sprite_comet;

      l_ptr = list_next(l_ptr);
      while (!list_is_empty(l_ptr_c)) {
        sprite_t sprite_comet = list_head_sprite(l_ptr_c);
        list_ptr list_el_c = l_ptr_c;

        l_ptr_c = list_next(l_ptr_c);
        collide = collide_test(current_sprite, sprite_comet, screen->format, &cu, &cv);
        if (collide) {
          draw_explosion(cu,cv);
          split_and_score(list_el_c, &l_sprite_comet, true);//addtional points
          list_remove(list_el_c, &l_sprite_comet);
          list_remove(list_el, &l_sprite_bullet);
          hits++;
          comets_destroyed++;
          draw_score(font_score);
          break;
        }
      }
    }

    draw_sprites(&l_sprite_explosion);

    //when all comets are destroyed, the level is passed
    if (list_is_empty(l_sprite_comet)) {
      next_level(font_next_level);
      list_free(l_sprite_comet);
      l_sprite_comet = list_new();
      start_level();
    }

    /* update the screen */
    SDL_UpdateRect(screen, 0, 0, 0, 0);

    while(SDL_Flip(screen)!=0) { /* if GC not ready to blit */
      SDL_Delay(1); /* wait and keep vertical sync*/
    }
    while(SDL_GetTicks()-lasttime<20) { /* minimal frame time: 20ms */
      SDL_Delay(1);
    }

  }//loop !gameover

  end_of_game(start_ticks);

  printf("Bye bye.\n");
  /* free everything */
  sprite_free(sprite_ship);
  sprite_free(ghost);
  if (nyan != NULL)
    sprite_free(nyan);
  list_free(l_sprite_comet);
  list_free(l_sprite_bullet);
  list_free(l_sprite_explosion);
  list_free(l_sprite_text);
  list_free(l_sprite_life_counter);
  SDL_FreeSurface(bg);
  // Shutdown the TTF library
  TTF_CloseFont(font_next_level);
  TTF_CloseFont(font_score);
  TTF_Quit();
  /* cleanup SDL */
  SDL_Quit();
  return 0;
}//main
