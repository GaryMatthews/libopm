/* vim: set shiftwidth=3 softtabstop=3 cinoptions=(0 expandtab: */

#ifndef LIBOPM_H
#define LIBOPM_H

#include "config.h"
#include "inet.h"
#include "opm_common.h"
#include "opm.h"

/* Number of callback functions. */
#define LIBOPM_CBLEN 5
/* Size of conn->readbuf. */
#define LIBOPM_READBUFLEN 128
/* Size of sendbuffer in proxy.c. */
#define LIBOPM_SENDBUFLEN 512

typedef struct  _OPM_SCAN             OPM_SCAN_T;
typedef struct  _OPM_CONNECTION       OPM_CONNECTION_T;
typedef struct  _OPM_PROTOCOL_CONFIG  OPM_PROTOCOL_CONFIG_T;
typedef struct  _OPM_PROTOCOL         OPM_PROTOCOL_T;

/*
 * Types of hard-coded proxy READ/WRITE functions which are 
 * set up in a table in libopm.c.
 */
typedef int OPM_PROXYWRITE_T (OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);
typedef int OPM_PROXYREAD_T  (OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);

struct _OPM_SCAN { 
   /* Address in byte order of remote client. */
   opm_sockaddr addr;
   /* Pointer to the OPM_REMOTE_T for this scan, passed by client. */
   OPM_REMOTE_T *remote;
   /* List of individual connections of this scan (1 for each protocol). */
   OPM_LIST_T *connections;
};

struct _OPM_CONNECTION {
   /* Pointer to specific protocol this connection handles. */
   OPM_PROTOCOL_T *protocol; 
   /* Proxies have a port as well as a protocol. */
   unsigned short int port;

   /* Allocated file descriptor, 0 if not yet allocated. */
   int fd;
   /* Bytes read so far in this connection. */
   unsigned short int bytes_read;
   /* 128 byte read buffer, anything over 128 is probably not of use. */
   char readbuf[LIBOPM_READBUFLEN + 1];
   /* Length of readbuf. */
   unsigned short int readlen;
   /* State of connection. */
   unsigned short int state;
   unsigned char flags;
   /* When this connection was established. */
   time_t creation;
   /* Arbitrary protocol-specific data. */
   void *data;
};

struct _OPM_PROTOCOL_CONFIG
{
   /* Protocol type. */
   OPM_PROTOCOL_T *type;
   /* Port to connect on. */
   unsigned short int port;

};

struct _OPM_PROTOCOL
{
   /* Protocol type. */
   const char *type;

   /* Write function handler for this protocol. */
   OPM_PROXYWRITE_T *write_function;
   /* Read function handler for this protocol. */
   OPM_PROXYREAD_T *read_function;
};


void libopm_do_hup(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);
void libopm_do_callback(OPM_T *, OPM_REMOTE_T *, int, int);
OPM_REMOTE_T *libopm_setup_remote(OPM_REMOTE_T *, OPM_CONNECTION_T *);
void libopm_do_openproxy(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);
OPM_ERR_T libopm_protocol_add(const char *, OPM_PROXYWRITE_T *,
                              OPM_PROXYREAD_T *);

#endif /* LIBOPM_H */
