#ifndef LIST_H
#define LIST_H

node_t *node_create(void *);
list_t *list_create(void);

node_t *list_add(list_t *, node_t *);
node_t *list_remove(list_t *, node_t *);

void list_free(list_t *);
void node_free(node_t *);


#endif /* LIST_H */
