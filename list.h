#ifndef LIST_H
#define LIST_H

typedef struct _node node_t;
typedef struct _list list_t;


struct _list {

  struct _node *head;
  struct _node *tail;
  int elements;

};

struct _node {

   struct _node *next; 
   struct _node *prev;
   void *data;

};

node_t *node_create(void *);
list_t *list_create(void);

node_t *list_add(list_t **, node_t *);
node_t *list_remove(list_t **, node_t *);

#endif /* LIST_H */
