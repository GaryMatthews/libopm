#ifndef LIST_H
#define LIST_H


/*  Copyright (C) 2002 by the past and present ircd coders, and others.
    Adapted from Hybrid7 DLINK macros
*/
#define LIST_FOREACH(pos, head) for (pos = (head); pos != NULL; pos = pos->next)
#define LIST_FOREACH_SAFE(pos, n, head) for (pos = (head), n = pos ? pos->next : NULL; pos != NULL; pos = n, n = pos ? pos->next : NULL)
#define LIST_FOREACH_PREV(pos, head) for (pos = (head); pos != NULL; pos = pos->prev)
#define LIST_SIZE(list) list->elements
/* End Copyright */

node_t *node_create(void *);
list_t *list_create(void);

node_t *list_add(list_t *, node_t *);
node_t *list_remove(list_t *, node_t *);

void list_free(list_t *);
void node_free(node_t *);

#endif /* LIST_H */
