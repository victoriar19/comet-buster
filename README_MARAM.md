==========================================
Bilan Comet Busters
Autrice : Maram MAAROUFI et Victoria RUF
==========================================

Lors de ce projet j'ai appris à gérer les aspects dynamiques et logiques d'un jeu, à coordonner les mécaniques de gameplay et à mettre en place une expérience utilisateur cohérente.
Ce projet a été particulièrement enrichissant car il m'a demandé de réfléchir à la structure globale du jeu et comment les différentes fonctionnalités s'articulent entre elles.
Il m'a également appris à me coordonner et à travailler avec une camarade, ce qui n'est pas toujours évident lorsqu'on code.

Nous avons donc réparti le travail :

Maram :
- Gestion de la vitesse du vaisseau : j'ai travaillé sur le déplacement du vaisseau en mettant à jour sa position à chaque frame selon une vitesse donnée. Cela permet d'obtenir un mouvement fluide et continu.
- Délimitation des bordures du jeu : j'ai implémenté des conditions pour bloquer le vaisseau lorsqu'il touche les bords de l'écran afin qu'il ne sorte pas de la fenêtre.
- Mise en place du GameOver : j'ai ajouté une vérification des vies du joueur et un état de fin de partie lorsque celles-ci tombent à zéro.
- Affichage du niveau : j'ai ajouté un système pour afficher le niveau actuel et le faire évoluer selon la progression du joueur.
- Musique de fond : j'ai chargé et lancé une musique en boucle pour améliorer l'ambiance du jeu.

Victoria :
- Ajout des fonctions dans le linkedlist.c
- Ajout des 2 fonctions dans le main.c (afficher_scores, sauvegarder_score) pour répondre à la demande du fichier texte.
- Corrections de quelques erreurs comme mettre des float à la place des int pour les angles.


## Gestion de la vitesse du vaisseau

Pour implémenter la vitesse du vaisseau, j'ai travaillé sur les variables de position et de déplacement. 
Le principe est simple : à chaque frame, on met à jour la position du vaisseau en ajoutant une valeur de vitesse. Cela permet au vaisseau de se déplacer progressivement et de manière fluide, plutôt que de sauter d'une position à l'autre.

```c
ship_x += velocity_x * delta_time;
ship_y += velocity_y * delta_time;
```

Cette méthode est utile car elle dépend du temps écoulé, ce qui rend le déplacement plus stable et plus lisible. 
Le but était d'obtenir un contrôle précis du vaisseau tout en gardant une fluidité de jeu correcte.


## Délimitation des bordures du jeu

Pour les bordures, j'ai mis en place une vérification à chaque frame pour empêcher le vaisseau de sortir de l'écran. 
Le principe est de vérifier si la position du vaisseau dépasse les limites de la fenêtre et, si c'est le cas, de la ramener à la limite correspondante.

```c
if (ship_x < 0) ship_x = 0;
if (ship_x > SCREEN_WIDTH - SHIP_WIDTH) ship_x = SCREEN_WIDTH - SHIP_WIDTH;
if (ship_y < 0) ship_y = 0;
if (ship_y > SCREEN_HEIGHT - SHIP_HEIGHT) ship_y = SCREEN_HEIGHT - SHIP_HEIGHT;
```

Cette logique est essentielle pour la jouabilité : le vaisseau doit rester visible et contrôlable à tout moment, sans pouvoir s'échapper de l'écran.


## Mise en place du GameOver

La gestion du GameOver a nécessité de comprendre le système de vies et comment arrêter la boucle principale du jeu.
J'ai donc ajouté une condition qui vérifie si le joueur n'a plus de vie :

```c
if (lives <= 0) {
    state = GAME_OVER;
}
```

Quand l'état du jeu passe à `GAME_OVER`, la boucle principale est interrompue, les entrées du joueur sont bloquées et un écran de fin de partie s'affiche. 
Le principe est de faire évoluer le jeu selon un état global, ce qui permet de gérer proprement le lancement, la pause et la fin de partie.

Une structure du type suivante a souvent été utilisée :

```c
enum GameState { RUNNING, PAUSED, GAME_OVER };
enum GameState state = RUNNING;
```

Cela permet de garder le jeu cohérent et de gérer les situations de fin sans bug ni conflit.


## Affichage du niveau

L'affichage du niveau a été mis en place pour permettre au joueur de voir sa progression dans le jeu. 
Le principe est simple : on garde une variable `level` qui représente le niveau actuel, puis on l'incrémente lorsqu'une condition de progression est remplie.

```c
int level = 1;
```

Ensuite, à chaque étape réussie, on met à jour la valeur :

```c
if (condition_fin_niveau) {
    level++;
}
```

Enfin, le niveau est affiché sur l'écran dans le HUD, afin que le joueur puisse suivre sa progression :

```c
printf("Niveau : %d\n", level);
```

Dans un jeu plus visuel, cela peut aussi être rendu avec une fonction d'affichage graphique comme :

```c
draw_text("Niveau : %d", level);
```

Le rôle de ce système est double : il apporte une meilleure compréhension de la progression du joueur et il rend la partie plus dynamique en montrant qu'il avance dans les défis.


## Musique de fond

Pour intégrer la musique, j'ai utilisé une bibliothèque audio pour charger et lire un fichier musical en boucle.
Le principe général est le suivant :

```c
Mix_Music *background_music = Mix_LoadMUS("assets/music/background.wav");

if (background_music != NULL) {
    Mix_PlayMusic(background_music, -1);
}
```

Le paramètre `-1` indique que la musique doit être répétée indéfiniment. 
J'ai également vérifié que le fichier avait bien été chargé avant de le lancer, afin d'éviter les erreurs au démarrage du jeu. 
À la fin du jeu, on libère ensuite la ressource pour éviter les fuites mémoire :

```c
Mix_FreeMusic(background_music);
```

Cette étape améliore l'immersion du joueur et donne une ambiance plus agréable au jeu.


## Apprentissages et défis

Ce projet m'a montré l'importance de la coordination dans un projet de groupe. 
Victoria et moi avons dû nous assurer que nos modifications ne conflictaient pas et que le jeu restait cohérent.

J'ai également appris à :
- penser à l'expérience utilisateur globale et à la logique du jeu ;
- gérer les déplacements et les limites de l'écran ;
- mettre en place des états de jeu (en cours, pause, fin) ;
- afficher la progression du joueur via le niveau ;
- intégrer des éléments audio pour améliorer l'immersion ;
- résoudre des problèmes techniques en C dans un projet collectif.

Ce projet a renforcé ma compréhension de la programmation de jeux et de la manière dont les différentes mécaniques doivent s'articuler entre elles pour créer une expérience de jeu fluide et agréable.
