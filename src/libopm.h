#ifndef LIBOPM_H
#define LIBOPM_H

#include "config.h"
#include "opm_common.h"
#include "opm.h"

typedef struct  _OPM_SCAN             OPM_SCAN_T;
typedef struct  _OPM_CONNECTION       OPM_CONNECTION_T;
typedef struct  _OPM_PROTOCOL_CONFIG  OPM_PROTOCOL_CONFIG_T;
typedef struct  _OPM_PROTOCOL         OPM_PROTOCOL_T;

struct _OPM_SCAN { 
   OPM_REMOTE_T        *remote;         /* Pointed to the OPM_REMOTE_T for this scan, passed by client */
   list_t              *connections;    /* List of individual connections of this scan (1 for each protocol) */
};

struct _OPM_CONNECTION {

   OPM_PROTOCOL_T     *protocol;        /* Pointer to specific protocol this connection handles */
   unsigned short int  port;            /* Some protocols have multiple ports, eg. HTTP */

   int                fd;               /* Allocated file descriptor, 0 if not yet allocated */
   unsigned short int bytes_read;       /* Bytes read so far in this connection */
   char               readbuf[128];     /* 128 byte read buffer, anything over 128 is probably not of use */
   unsigned short int readlen;          /* Length of readbuf */
   unsigned short int state;            /* State of connection */
};

struct _OPM_PROTOCOL_CONFIG
{
   OPM_PROTOCOL_T *type;                /* Protocol type */
   unsigned short int port;             /* Port to connect on */
};

struct _OPM_PROTOCOL
{
   int type;                            /* Protocol type */
   int write_function;                  /* Write function handler for this protocol */
   int read_function;                   /* Read function handler for this protocol */
};

#endif /* LIBOPM_H */
