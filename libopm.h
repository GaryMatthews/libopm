#ifndef LIBOPM_H
#define LIBOPM_H

typedef struct  _OPM        OPM_T;
typedef struct  _OPM_REMOTE OPM_REMOTE_T;
typedef struct  _OPM_SCAN   OPM_SCAN_T;

typedef void OPM_CALLBACK_T (OPM_REMOTE_T *, int);

struct _OPM {            

   OPM_T               *next;
   OPM_T               *last;

   //struct sockaddr_in addr;   

   OPM_REMOTE_T        *remote;
   OPM_SCAN_T          *scans;
};

struct _OPM_REMOTE {

   char               *ip; 

   OPM_CALLBACK_T     *fun_openproxy;
   OPM_CALLBACK_T     *fun_negfail;
   OPM_CALLBACK_T     *fun_end;
   OPM_CALLBACK_T     *fun_error;

   int                port;
   int                protocol;                  
   int                bytes_read; 
};

struct _OPM_SCAN {

   OPM_SCAN_T           *next;
   OPM_SCAN_T           *last;

  /* OPM_PROTOCOL       *protocol; */
   int                fd;  

   int                bytes_read; 
   char               readbuf[128];
   int                readlen;   

   int                state; 
};


OPM_REMOTE_T *opm_new();

#endif /* LIBOPM_H */
