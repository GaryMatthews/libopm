#ifndef CONFIG_H
#define CONFIG_H

#include "libopm.h" 

struct CONFIG_HASH {
   int key;
   int type;
};

OPM_CONFIG_T *config_create();
void config_free(OPM_CONFIG_T *);
OPM_ERR_T config_set(OPM_CONFIG_T *, int , void *);
void *config(OPM_CONFIG_T *, int);
int config_gettype(int);

#endif /* CONFIG_H */
