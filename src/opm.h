#ifndef OPM_H
#define OPM_H

#include "opm_common.h"

typedef struct  _OPM_CONFIG           OPM_CONFIG_T;
typedef struct  _OPM                  OPM_T;
typedef struct  _OPM_REMOTE           OPM_REMOTE_T;

typedef         int                   OPM_ERR_T;

typedef void OPM_CALLBACK_T (OPM_REMOTE_T *, int);

struct _OPM_CONFIG {
   void **vars;
};

struct _OPM {
   OPM_CONFIG_T *config;
   list_t       *scans;
   list_t       *protocols;
};

struct _OPM_REMOTE {

   char               *ip;
   struct sockaddr_in *addr;

   OPM_CALLBACK_T     *fun_openproxy;
   OPM_CALLBACK_T     *fun_negfail;
   OPM_CALLBACK_T     *fun_end;
   OPM_CALLBACK_T     *fun_error;
   OPM_CALLBACK_T     *fun_timeout;

   unsigned short int  port;
   unsigned short int  protocol;
   unsigned short int  bytes_read;
};

OPM_T *opm_create();
void opm_free(OPM_T *);

OPM_REMOTE_T *opm_remote_create();
void opm_remote_free(OPM_REMOTE_T *);

OPM_ERR_T opm_config(OPM_T *, int, void *);
OPM_ERR_T opm_scan(OPM_T *, OPM_REMOTE_T *);

OPM_CONFIG_T *config_create();
OPM_ERR_T config_set(OPM_CONFIG_T *, int , void *);

#endif /* OPM_H */
