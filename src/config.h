#ifndef CONFIG_H
#define CONFIG_H

#include "libopm.h" 

struct CONFIG_HASH {
   int key;
   int type;
};

void config_free(OPM_CONFIG_T *);
void *config(OPM_CONFIG_T *, int);
int config_gettype(int);

#endif /* CONFIG_H */
