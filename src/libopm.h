#include "config.h"
#include "opm_common.h"
#include "opm.h"

#ifndef LIBOPM_H
#define LIBOPM_H

typedef struct  _OPM_SCAN             OPM_SCAN_T;
typedef struct  _OPM_CONNECTION       OPM_CONNECTION_T;
typedef struct  _OPM_PROTOCOL_CONFIG  OPM_PROTOCOL_CONFIG_T;
typedef struct  _OPM_PROTOCOL         OPM_PROTOCOL_T;

struct _OPM_SCAN { 
   OPM_REMOTE_T        *remote;
   list_t              *connections;
};

struct _OPM_CONNECTION {

   OPM_PROTOCOL_T     *protocol; 
   unsigned short int  port;

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

#endif /* LIBOPM_H */
