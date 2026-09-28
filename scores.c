#include <stdio.h>
#include <string.h>

#include "scores.h"

int scores_load(const char *fname, entry_t table[])
{
  int n = 0;
  FILE *f = fopen(fname, "r");
  if (f == NULL) {
    return 0;               /* pas de fichier : pas encore de scores */
  }
  while (n < MAX_SCORES &&
         fscanf(f, " %31[^:]:%d", table[n].name, &table[n].score) == 2) {
    n++;
  }
  fclose(f);
  return n;
}

int scores_add(entry_t table[], int n, const char *name, int score)
{
  if (n >= MAX_SCORES) {
    return n;               /* tableau plein */
  }
  strncpy(table[n].name, name, NAME_LEN - 1);
  table[n].name[NAME_LEN - 1] = '\0';
  table[n].score = score;
  return n + 1;
}

void scores_sort(entry_t table[], int n)
{
  int i, j;
  entry_t tmp;
  for (i = 0; i < n - 1; i++) {
    for (j = 0; j < n - 1 - i; j++) {
      if (table[j].score < table[j + 1].score) {
        tmp = table[j];
        table[j] = table[j + 1];
        table[j + 1] = tmp;
      }
    }
  }
}

void scores_save(const char *fname, entry_t table[], int n)
{
  int i;
  FILE *f = fopen(fname, "w");
  if (f == NULL) {
    printf("Impossible d'ecrire %s\n", fname);
    return;
  }
  for (i = 0; i < n; i++) {
    fprintf(f, "%s:%d\n", table[i].name, table[i].score);
  }
  fclose(f);
}

void scores_print_top(entry_t table[], int n, int top)
{
  int i;
  printf("======== MEILLEURS SCORES ========\n");
  for (i = 0; i < n && i < top; i++) {
    printf("%2d. %-20s %8d\n", i + 1, table[i].name, table[i].score);
  }
}
