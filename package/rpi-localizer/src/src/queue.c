#include "queue.h"

void queue_add(queue_node **phead, void *data) {
  queue_node *c = (queue_node *)malloc(sizeof(queue_node));
  c -> data = data;
  c -> prev = 0;
  c -> next = *phead;
  
  if (*phead) {
    (*phead) -> prev = c;
  }

  *phead = c;
}

void* queue_remove(queue_node **n) {
  queue_node *t = (*n);
  void *t_data = t -> data;
  
  if (t -> prev != 0) {
    t -> prev -> next = t -> next;

  }
  if (t -> next != 0) {
    t -> next -> prev = t -> prev;
  }
  
  *n = t -> next;
  free(t);
  
  return t_data;
}

void queue_clean(queue_node **n) {
  queue_node *t;

  while (*n) {
    t = *n;
    n = &((*n) -> next);
    free(t -> data);
    free(t);
  }
  *n = 0;
}
