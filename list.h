#ifndef LIST_H
#define LIST_H

typedef struct _list list_t;

struct _list {

   struct _list *next;
   struct _list *prev;

   void *data;

};

list_t *list_create(void *);
list_t *list_add(list_t*, list_t*);

#endif /* LIST_H */
