
#include <stdio.h>
#include "opm.h"
#include "opm_error.h"
#include "opm_types.h"
#include <unistd.h>

#define NODES 111

int main()
{
   int fdlimit = 1024;
   OPM_T *scanner;
   OPM_REMOTE_T *remote;

   scanner = opm_create();
   remote  = opm_remote_create("127.0.0.1");

   opm_config(scanner, OPM_CONFIG_FD_LIMIT, &fdlimit);


   while(1)
      opm_cycle();
   
   opm_remote_free(remote);
   opm_free(scanner);
   
   return 0; 
}
