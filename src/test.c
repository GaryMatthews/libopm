
#include <stdio.h>
#include "opm.h"
#include "opm_error.h"
#include "opm_types.h"
#include <unistd.h>

#define NODES 111

void open_proxy(OPM_REMOTE_T *, int);
void negotiation_failed(OPM_REMOTE_T *, int);
void timeout(OPM_REMOTE_T *, int);
void end(OPM_REMOTE_T *, int);

int main(int argc, char **argv)
{
   int fdlimit = 1024;
   int scan_port = 6667;
   OPM_T *scanner;
   OPM_REMOTE_T *remote;

   scanner = opm_create();
  
   if(argc >= 2)
      remote  = opm_remote_create(argv[1]);
   else
      remote  = opm_remote_create("208.245.162.250");

   /* Setup callbacks */
   remote->fun_openproxy = &open_proxy;
   remote->fun_negfail   = &negotiation_failed;
   remote->fun_timeout   = &timeout;
   remote->fun_end       = &end;
 
   opm_config(scanner, OPM_CONFIG_FD_LIMIT, &fdlimit);
   opm_config(scanner, OPM_CONFIG_SCAN_IP, "203.56.139.100");
   opm_config(scanner, OPM_CONFIG_SCAN_PORT, &scan_port);
   opm_config(scanner, OPM_CONFIG_TARGET_STRING, "*** Looking up your hostname...");

   opm_addtype(scanner, OPM_TYPE_HTTP, 8080);
   opm_addtype(scanner, OPM_TYPE_HTTP, 80);
   opm_addtype(scanner, OPM_TYPE_HTTP, 3128);
   opm_addtype(scanner, OPM_TYPE_WINGATE, 23);
   opm_addtype(scanner, OPM_TYPE_ROUTER, 23);
   opm_addtype(scanner, OPM_TYPE_SOCKS4, 1080);
   opm_addtype(scanner, OPM_TYPE_SOCKS5, 1080);

   opm_scan(scanner, remote);

   while(1)
      opm_cycle(scanner);
   
   opm_remote_free(remote);
   opm_free(scanner);
   
   return 0; 
}

void open_proxy(OPM_REMOTE_T *remote, int notused)
{
   printf("Open proxy on %s:%d [%d bytes read]\n", remote->ip, remote->port, remote->bytes_read);
}

void negotiation_failed(OPM_REMOTE_T *remote, int notused)
{
   printf("Negotiation on %s:%d failed [%d bytes read]\n", remote->ip, remote->port, remote->bytes_read);
}

void timeout(OPM_REMOTE_T *remote, int notused)
{
   printf("Negotiation timed out on %s:%d\n", remote->ip, remote->port);
}

void end(OPM_REMOTE_T *remote, int notused)
{
   printf("Scan on %s has ended\n", remote->ip);
}
