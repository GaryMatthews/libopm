#ifndef OPM_COMMON_H
#define OPM_COMMON_H

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

#endif /* OPM_COMMON_H */
