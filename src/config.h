#ifndef CONFIG_H
#define CONFIG_H

#include "libopm.h"

#define CONFIG_BIND_IP      1
#define CONFIG_DNSBL_HOST   2
#define CONFIG_FD_LIMIT     3

#define TYPE_INT            1
#define TYPE_STRING         2

struct CONFIG_HASH {
   int key;
   int type;
   void *var;
};

OPM_CONFIG_T *config_create();
void config_free(OPM_CONFIG_T *);
int config_set(OPM_CONFIG_T *, int , void *);

#endif /* CONFIG_H */
