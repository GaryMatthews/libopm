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

#include "malloc.h"
#include "config.h"
#include "libopm_error.h"
#include <string.h>

/* config_create
 *
 *    Create an OPM_CONFIG_T struct, set default values and return it
 *
 * Parameters:
 *    None;
 *
 * Return:
 *    Pointer to allocated OPM_CONFIG_T struct
 */

OPM_CONFIG_T *config_create()
{
   OPM_CONFIG_T *ret;
   ret = MyMalloc(sizeof(OPM_CONFIG_T));

   ret->bind_ip    = 0;
   ret->dnsbl_host = 0;
   ret->fd_limit   = 0;

   return ret;
}




/*
 *
 *
 *
 *
 *
 */

void config_free(OPM_CONFIG_T *config)
{
   if(config->bind_ip)
      MyFree(config->bind_ip);

   MyFree(config);

}


/* config_set
 *
 *    Set configuration options on config struct.
 *
 * Parameters:
 *    config: Config struct to set parameters on
 *    key:    Variable within the struct to set
 *    value:  Address of value to set 
 *
 * Return:
 *    1: Variable was set
 *    0: Some error occured
 */

OPM_ERR_T config_set(OPM_CONFIG_T *config, int key, void *value)
{

   int i;
   struct CONFIG_HASH hash[] = {
      {CONFIG_BIND_IP,      TYPE_STRING, &(config->bind_ip)  },
      {CONFIG_FD_LIMIT,     TYPE_INT,    &(config->fd_limit) }
   }; 

   for(i = 0; i < sizeof(hash) / sizeof(struct CONFIG_HASH); i++)
   {
      if(key == hash->key)
      {
         switch(hash->type)
         {
            case TYPE_STRING:
              if(*(char**) hash[i].var)
                 MyFree(*(char**) hash[i].var);
              *(char**) hash[i].var = strdup((char *)value); 
                               
               break;
            case TYPE_INT:
               *(int *) hash[i].var = *(int*) value;
               break;
            default:
               return 0;
         }
         return 1;
      }
   }
   return 0;
}
