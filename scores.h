#ifndef SCORES_H
#define SCORES_H

#define MAX_SCORES 100
#define NAME_LEN   32

typedef struct {
  char name[NAME_LEN];
  int score;
} entry_t;

/* lit le fichier, remplit table, renvoie le nombre de lignes lues */
int scores_load(const char *fname, entry_t table[]);

/* ajoute une entree a la fin, renvoie le nouveau nombre d'entrees */
int scores_add(entry_t table[], int n, const char *name, int score);

/* tri a bulles : meilleur score en premier */
void scores_sort(entry_t table[], int n);

/* ecrit le fichier (une ligne "pseudo:score" par entree) */
void scores_save(const char *fname, entry_t table[], int n);

/* affiche les 'top' meilleurs scores dans le terminal */
void scores_print_top(entry_t table[], int n, int top);

#endif /* SCORES_H */
