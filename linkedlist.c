#include <stdio.h>
#include <stdlib.h>

#include "linkedlist.h"

/* Liste chainee simple. Chaque cellule contient un sprite et pointe vers la
 * suivante. Une liste vide est representee par NULL.
 * list_add ajoute toujours EN TETE de liste.
 * */

/* Initialisation de la liste : liste vide */
list_ptr list_new(void)
{
  return NULL;
}

/* Ajoute une cellule en tete de liste et renvoie la nouvelle tete */
list_ptr list_add(sprite_t sprite, list_ptr list)
{
  list_ptr cell = (list_ptr)malloc(sizeof(s_list_node_t));
  if (cell == NULL) {
    return list;            /* plus de memoire : on ne change rien */
  }
  cell->data = sprite;
  cell->next = list;
  return cell;
}

/* Vrai si la liste est vide */
bool list_is_empty(list_ptr l)
{
  return l == NULL;
}

/* Cellule suivante (ou NULL) */
list_ptr list_next(list_ptr l)
{
  if (l == NULL) {
    return NULL;
  }
  return l->next;
}

/* Sprite de la cellule donnee */
sprite_t list_head_sprite(list_ptr l)
{
  if (l == NULL) {
    return NULL;
  }
  return l->data;
}

/* Retire la DERNIERE cellule de la liste et renvoie son sprite.
 * Le sprite n'est pas libere : c'est a l'appelant de le faire. */
sprite_t list_pop_sprite(list_ptr * l)
{
  list_ptr prev = NULL;
  list_ptr cur = *l;
  sprite_t sprite;

  if (cur == NULL) {
    return NULL;
  }
  while (cur->next != NULL) {
    prev = cur;
    cur = cur->next;
  }
  sprite = cur->data;
  if (prev == NULL) {
    *l = NULL;              /* la liste n'avait qu'une cellule */
  } else {
    prev->next = NULL;
  }
  free(cur);
  return sprite;
}

/* Retire la cellule elt de la liste ET libere son sprite (sinon fuite memoire) */
void list_remove(list_ptr elt, list_ptr *l)
{
  list_ptr prev = NULL;
  list_ptr cur;

  if (elt == NULL || l == NULL) {
    return;
  }
  cur = *l;
  while (cur != NULL && cur != elt) {
    prev = cur;
    cur = cur->next;
  }
  if (cur == NULL) {
    return;                 /* elt n'est pas dans la liste */
  }
  if (prev == NULL) {
    *l = cur->next;         /* on retire la tete */
  } else {
    prev->next = cur->next;
  }
  if (cur->data != NULL) {
    sprite_free(cur->data);
  }
  free(cur);
}

/* Vide toute la liste (cellules + sprites) */
void list_free(list_ptr l)
{
  list_ptr next;
  while (l != NULL) {
    next = l->next;
    if (l->data != NULL) {
      sprite_free(l->data);
    }
    free(l);
    l = next;
  }
}

/* Nombre de cellules */
int list_length(list_ptr l)
{
  int n = 0;
  while (l != NULL) {
    n++;
    l = l->next;
  }
  return n;
}

/* Inverse l'ordre de la liste */
void list_reverse(list_ptr * l)
{
  list_ptr prev = NULL;
  list_ptr cur = *l;
  list_ptr next;
  while (cur != NULL) {
    next = cur->next;
    cur->next = prev;
    prev = cur;
    cur = next;
  }
  *l = prev;
}

/* Copie la liste (les cellules sont copiees, les sprites sont PARTAGES).
 * On copie en tete (ce qui inverse), puis on remet dans le bon ordre. */
list_ptr list_clone(list_ptr list)
{
  list_ptr copy = NULL;
  while (list != NULL) {
    copy = list_add(list->data, copy);
    list = list->next;
  }
  list_reverse(&copy);
  return copy;
}
