#include <stdio.h>
#include <stdlib.h>

#include "linkedlist.h"

/* Initialisation of the list
 * */
list_ptr list_new(void)
{
  return NULL;
}

/* Add a new cel to a list.
 *  store the sprite_t to the new cel
 * */
list_ptr list_add(sprite_t sprite, list_ptr list)
{
  list_ptr nouveau_noeud = (list_ptr)malloc(sizeof(s_list_node_t));
  nouveau_noeud->data = sprite;
  nouveau_noeud->next = list;
  return nouveau_noeud;
}

/* Return true if the list is empty
 * */
bool list_is_empty(list_ptr l)
{
  if (l == NULL)
    return true;
  else
    return false;
}

/* Return the next cel in list or NULL
 * */
list_ptr list_next(list_ptr l)
{
  if (l == NULL)
    return NULL;
  return l->next;
}

/* Search the first cel of the list &
 *  return the associated sprite
 * */
sprite_t list_head_sprite(list_ptr l)
{
  if (l == NULL)
    return NULL;
  else
    return l->data;
}

/* Search the last cel of a list
 *  Remove the cel from the list
 *  Return the associated sprite
 * */
sprite_t list_pop_sprite(list_ptr *l)
{
/* ne fonctionne pas si la liste est vide : * */
  if (l == NULL || *l == NULL)
    return NULL;
/* On compte le nombre de noeuds dans la liste pour trouver le dernier noeud : * */
  int taille = 0;
  list_ptr courant = *l;
  while (courant != NULL){
    taille ++;
    courant = courant->next;
  }
/* si on a que 1 élément dans la liste : * */
  if (taille == 1){
/* on récupère son sprite : * */
    sprite_t data = (*l)->data;
/* on libère le noeud : * */
    free(*l);
    *l = NULL;
    return data;
  }
/* sinon on avance jusqu'à l'avant-dernier noeud : * */
  list_ptr avant_dernier = *l;
  int i = 0;
  while (i < taille - 2){
    avant_dernier = avant_dernier -> next;
    i++;
  }
/* on récupère le sprite du dernier (avant_dernier->next) : * */
  sprite_t data = avant_dernier->next->data;
/* on libère le noeud : * */
  free(avant_dernier->next);
/* avant dernier devient le dernier noeud de la liste : * */
  avant_dernier-> next = NULL;
  return data;
}


/* Remove the given cel in a list
 * */
void list_remove(list_ptr elt, list_ptr *l)
{
  /* Si le noeud qu'on veut supprimer est en premier dans la liste, on avance la tête de liste pour être juste avant ce qu'on veut supprimer (elt)  * */
  if (elt == *l)
  {
    *l = elt->next;
/* On supprime elt : * */
    free(elt); }
/* Sinon on cherche le noeud juste avant elt, on défini elt et on le supprime : * */
    else{
      list_ptr current = *l;
/* Tant que le noeud n'est pas elt on avance : * */
      while (current->next != elt){
          current = current-> next;
    }
    current ->next = elt->next;
    free(elt);
  }
}

/* Wipe out a list.
 *  Don't forget to sprite_free() for each sprite
 * */
void list_free(list_ptr l)
{
/* Si la liste est vide on a rien à libérer : * */
  if (l == NULL)
    return;
/* on libère tout ce qui a après l : * */
  list_free(l->next);
/* on oublie pas de libérer le sprite : * */
  sprite_free(l->data);
/* on libère le noeud : * */
  free(l);
}

/* Return the length of a list
 * */
int list_length(list_ptr l)
{
/* si elle est vide on return 0 : * */
  if (l == NULL) 
    return 0;
/* sinon on compte les noeuds et on return le compteur : * */
  int compteur = 0;
  while (l != NULL)
  {
    compteur++;
    l = l->next;
  }
  return compteur;
}

/* Reverse the order of a list
 * */
void list_reverse(list_ptr *l)
{
/* on créer une nouvelle liste vide : * */
  list_ptr new_list = NULL;
/* on parcourt l'ancienne liste depuis le début : * */
  list_ptr current = *l;
/* on remet chaque sprite de l'ancienne liste dans la nouvelle, 1 par 1 avec list_add qui ajoute un spite en en tête dans une liste : * */
  while (current != NULL){
    new_list = list_add(current->data, new_list);
    current = current-> next;
  }
/* la nouvelle liste remplace l'ancienne : * */
  *l = new_list;
}

/* Copy a list to another one.
 *  Return the new list
 * */
list_ptr list_clone(list_ptr list)
{
/* si la liste est vide on clone rien : * */
  if (list == NULL)
    return NULL;

  /* sinon on créer un clone : * */
  list_ptr clone = (list_ptr)malloc(sizeof(s_list_node_t));
  /* on copie le sprite dans le clone : * */
  clone-> data = list-> data;
  /* on clone le reste de list et on le mets à la suite : * */
  clone->next = list_clone(list->next);
  return clone;
}
