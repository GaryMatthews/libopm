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

OPM_NODE_T *libopm_node_create(void *);
OPM_LIST_T *libopm_list_create(void);

OPM_NODE_T *libopm_list_add(OPM_LIST_T *, OPM_NODE_T *);
OPM_NODE_T *libopm_list_remove(OPM_LIST_T *, OPM_NODE_T *);

void libopm_list_free(OPM_LIST_T *);
void libopm_node_free(OPM_NODE_T *);

#endif /* LIST_H */
