#ifndef OPM_H
#define OPM_H

#include "opm_common.h"
#include "inet.h"

/* Stuff to shut up warnings about rcsid being unused. */
#define USE_VAR(var)    static char sizeof##var = sizeof(sizeof##var) + sizeof(var)
/* RCS tag. */
#define RCSID(x)        static char rcsid[] = x; USE_VAR(rcsid);


typedef struct  _OPM_CONFIG           OPM_CONFIG_T;
typedef struct  _OPM                  OPM_T;
typedef struct  _OPM_REMOTE           OPM_REMOTE_T;

typedef         int                   OPM_ERR_T;

typedef void OPM_CALLBACK_T (OPM_REMOTE_T *, int);

struct _OPM_CONFIG {
   void **vars;
};

struct _OPM {
   OPM_CONFIG_T *config;               /* Individual scanner configuration */
   list_t       *scans;                /* List of scans (each scan containing a list of connections) */
   list_t       *protocols;            /* List of protocols this scanner handles */
   unsigned int  fd_use;               /* Number of file descriptors in use */
};

struct _OPM_REMOTE {

   char                *ip;              /* Readable IP address */
   opm_sockaddr         addr;            /* Store the IP in a sockaddr_in aswell, this is
                                            done in opm_remote_create  */

   OPM_CALLBACK_T      *fun_openproxy;   /* Callback for when an open proxy is found */
   OPM_CALLBACK_T      *fun_negfail;     /* Callback for when negotiation with a proxy fails */
   OPM_CALLBACK_T      *fun_end;         /* Callback for when scan on remote host is complete */
   OPM_CALLBACK_T      *fun_error;       /* Callback for when an error has occured */
   OPM_CALLBACK_T      *fun_timeout;     /* Callback for when a specific connection has timed out */

   unsigned short int   port;            /* Port passed back on certain callbacks */
   unsigned short int   protocol;        /* Protocol passed back on certain callbacks */
   unsigned short int   bytes_read;      /* Bytes read passed back on certain callbacks */
};

OPM_T *opm_create();
void opm_free(OPM_T *);

OPM_REMOTE_T *opm_remote_create(char *);
void opm_remote_free(OPM_REMOTE_T *);

OPM_ERR_T opm_config(OPM_T *, int, void *);
OPM_ERR_T opm_scan(OPM_T *, OPM_REMOTE_T *);
OPM_ERR_T opm_addtype(OPM_T *, int, int);


OPM_CONFIG_T *config_create();
OPM_ERR_T config_set(OPM_CONFIG_T *, int , void *);

void opm_cycle(OPM_T *);

#endif /* OPM_H */
