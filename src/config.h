#ifndef CONFIG_H
#define CONFIG_H

#include "libopm.h" 

void libopm_config_free(OPM_LIST_T *);
void *libopm_config(OPM_LIST_T *, const char *);
OPM_LIST_T *libopm_config_create(void);
OPM_ERR_T libopm_config_set(OPM_LIST_T *, const char *, void *);
OPM_ERR_T libopm_config_add(OPM_LIST_T *, const char *, int);


#endif /* CONFIG_H */
