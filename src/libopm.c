/*
 * Copyright (C) 2002  Erik Fears
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 *
 *       Foundation, Inc.
 *       59 Temple Place - Suite 330
 *       Boston, MA  02111-1307, USA.
 *
 *                  
 */

#include "list.h"
#include "libopm.h"
#include "malloc.h"
#include "config.h"

/* opm_init
 *
 *    Initialize a new scanner and return a pointer to it.
 *
 * Parameters:
 *    None
 *  
 * Return 
 *    Pointer to new OPM_T (scanner)
 */

OPM_T *opm_init()
{
   OPM_T *ret;
   ret = MyMalloc(sizeof(OPM_T));
   ret->config = config_create();
   ret->scans  = list_create();

   free(ret);
}

/* opm_new
 *
 *    Create OPM_REMOTE_T struct, fill it with neccessary
 *    default values and return it to the client.
 *
 * Parameters:
 *    ip: IP of remote host
 *
 * Return:
 *    Address of OPM_REMOTE_T created
 *
 */

OPM_REMOTE_T *opm_remote(char *ip)
{ 

   OPM_REMOTE_T *ret;
   ret = MyMalloc(sizeof(OPM_REMOTE_T));

   /* Do initializations */

   ret->ip = (char*) strdup(ip);  /* replace with custom strdup function */

   ret->fun_openproxy = 0;
   ret->fun_negfail   = 0;
   ret->fun_end       = 0;
   ret->fun_error     = 0;
   ret->fun_timeout   = 0;
 
   ret->port          = 0;
   ret->protocol      = 0;
   ret->bytes_read    = 0;

   return ret;
}




/* opm_free
 *
 *    Free OPM_REMOTE_T and cleanup
 *
 * Parameters:
 *    var: Address of OPM_REMOTE_T to cleanup
 *
 * Return:
 *    None
 */

void opm_free(OPM_REMOTE_T *var)
{
   if(var)
      MyFree(var);
}
