#ifndef PROXY_H
#define PROXY_H

#include "libopm.h"

int proxy_http_write(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);
int proxy_socks4_write(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);
int proxy_socks5_write(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);
int proxy_wingate_write(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);
int proxy_router_write(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);


#endif /* PROXY_H */
