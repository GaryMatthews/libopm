#include "config.h"
#include "list.h"

#ifndef LIBOPM_H
#define LIBOPM_H

#define OPM_TYPE_HTTP    1
#define OPM_TYPE_SOCKS4  2
#define OPM_TYPE_SOCKS5  3
#define OPM_TYPE_WINGATE 4
#define OPM_TYPE_ROUTER  5

#define OPM_STATE_UNESTABLISHED 0
#define OPM_STATE_ESTABLISHED   1

typedef struct  _OPM_CONFIG           OPM_CONFIG_T;
typedef struct  _OPM                  OPM_T;
typedef struct  _OPM_SCAN             OPM_SCAN_T;
typedef struct  _OPM_REMOTE           OPM_REMOTE_T;
typedef struct  _OPM_CONNECTION       OPM_CONNECTION_T;

typedef struct  _OPM_PROTOCOL_CONFIG  OPM_PROTOCOL_CONFIG_T;
typedef struct  _OPM_PROTOCOL         OPM_PROTOCOL_T;

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

struct _OPM_SCAN { 
   OPM_REMOTE_T        *remote;
   list_t              *connections;
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

struct _OPM_CONNECTION {

   OPM_PROTOCOL_T     *protocol; 
   int                fd;  
   unsigned short int bytes_read; 
   char               readbuf[128];
   unsigned short int readlen;   
   unsigned short int state; 
};

struct _OPM_PROTOCOL_CONFIG
{
   OPM_PROTOCOL_T *type;
   unsigned short int port;
};

struct _OPM_PROTOCOL
{
   int type;
   int function;  //Replace later with function ptr to protocols
};

OPM_T *opm_create();
void opm_free(OPM_T *);

OPM_REMOTE_T *opm_remote_create();
void opm_remote_free(OPM_REMOTE_T *);

OPM_ERR_T opm_config(OPM_T *, int, void *);
OPM_ERR_T opm_scan(OPM_T *, OPM_REMOTE_T *);

#endif /* LIBOPM_H */
