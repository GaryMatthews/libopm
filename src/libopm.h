#ifndef LIBOPM_H
#define LIBOPM_H

#include "list.h"

typedef struct  _OPM_CONFIG       OPM_CONFIG_T;
typedef struct  _OPM              OPM_T;
typedef struct  _OPM_SCAN         OPM_SCAN_T;
typedef struct  _OPM_REMOTE       OPM_REMOTE_T;
typedef struct  _OPM_CONNECTION   OPM_CONNECTION_T;

typedef void OPM_CALLBACK_T (OPM_REMOTE_T *, int);


struct _OPM {
   OPM_CONFIG_T *config;
};

struct _OPM_CONFIG {
   char *bind_ip;
   char *dnsbl_host;
   int fd_limit;
};


struct _OPM_SCAN {
   //struct sockaddr_in addr;   
   OPM_REMOTE_T        *remote;
   list_t              *scans;
};

struct _OPM_REMOTE {

   char               *ip; 

   OPM_CALLBACK_T     *fun_openproxy;
   OPM_CALLBACK_T     *fun_negfail;
   OPM_CALLBACK_T     *fun_end;
   OPM_CALLBACK_T     *fun_error;
   OPM_CALLBACK_T     *fun_timeout;

   int                port;
   int                protocol;                  
   int                bytes_read; 
};

struct _OPM_CONNECTION {

  /* OPM_PROTOCOL       *protocol; */
   int                fd;  

   int                bytes_read; 
   char               readbuf[128];
   int                readlen;   

   int                state; 
};


OPM_REMOTE_T *opm_new();

#endif /* LIBOPM_H */
