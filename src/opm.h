/* vim: set shiftwidth=3 softtabstop=3 expandtab: */ 

/** \file opm.h
 *  \brief Main header for libopm.
 *  \author Erik Fears
 *  \version $Id$
 */

#ifndef OPM_H
#define OPM_H

#include "opm_common.h"

typedef struct  _OPM_CONFIG           OPM_CONFIG_T;
typedef struct  _OPM                  OPM_T;
typedef struct  _OPM_REMOTE           OPM_REMOTE_T;
typedef struct  _OPM_CALLBACK         OPM_CALLBACK_T;

typedef         int                   OPM_ERR_T;

typedef void OPM_CALLBACK_FUNC (OPM_T *, OPM_REMOTE_T *, int, void *);

struct _OPM_CALLBACK {
   OPM_CALLBACK_FUNC *func;
   void *data;
};

struct _OPM_CONFIG {
   const char *key;
   int type;
   void *value;
};

struct _OPM {
   /* List of individual scanner configuration key/values. */
   OPM_LIST_T *config;
   /* List of scans in the queue (not yet established). */
   OPM_LIST_T *queue;
   /* List of scans (each scan containing a list of connections). */
   OPM_LIST_T *scans;
   /* List of protocols this scanner handles. */
   OPM_LIST_T *protocols;
   /* Number of file descriptors in use. */
   unsigned int fd_use;

   /* Scanner-wide callbacks. */
   OPM_CALLBACK_T *callbacks;
};

struct _OPM_REMOTE {
   /* Presentation-format IPv4 address. */
   char *ip;

   /* Port passed back on certain callbacks. */
   unsigned short int  port;
   /* Protocol passed back on certain callbacks. */
   unsigned short int protocol;
   /* Bytes read passed back on certain callbacks. */
   unsigned short int bytes_read;

   /* Remote-specific protocols. */
   OPM_LIST_T *protocols;

   /* Arbitrary data that the user can point to for any purpose. */
   void *data;
};

OPM_T *opm_create(void);
void opm_free(OPM_T *);

OPM_REMOTE_T *opm_remote_create(const char *);
void opm_remote_free(OPM_REMOTE_T *);

OPM_ERR_T opm_config(OPM_T *, const char *, void *);
OPM_ERR_T opm_config_add(OPM_T *, const char *, int);
OPM_ERR_T opm_scan(OPM_T *, OPM_REMOTE_T *);
void opm_end(OPM_T *, OPM_REMOTE_T *);
void opm_endscan(OPM_T *, OPM_REMOTE_T *);

OPM_ERR_T opm_addtype(OPM_T *, int, unsigned short int);
OPM_ERR_T opm_remote_addtype(OPM_REMOTE_T *, int, unsigned short int);
OPM_ERR_T opm_callback(OPM_T *, int, OPM_CALLBACK_FUNC *, void *);

void opm_cycle(OPM_T *);

size_t opm_active(OPM_T *);

#endif /* OPM_H */
