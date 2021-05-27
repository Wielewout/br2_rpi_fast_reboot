#ifndef QUEUE_H_
#define QUEUE_H_
#include <stdlib.h>

typedef struct queue_node_t {
  void *data;
  struct queue_node_t *prev;
  struct queue_node_t *next;
} queue_node;

void queue_add(queue_node **phead, void *data);

void* queue_remove(queue_node **n);

void queue_clean(queue_node **n);

#endif /* QUEUE_H_ */