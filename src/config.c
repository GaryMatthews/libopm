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
#include "inet.h"
#include "opm_error.h"
#include "opm_types.h"
#include <string.h>

struct CONFIG_HASH HASH[] = {
   {OPM_CONFIG_FD_LIMIT,       OPM_TYPE_INT},
   {OPM_CONFIG_BIND_IP ,       OPM_TYPE_ADDRESS},
   {OPM_CONFIG_DNSBL_HOST,     OPM_TYPE_STRING},
   {OPM_CONFIG_TARGET_STRING,  OPM_TYPE_STRING},
   {OPM_CONFIG_SCAN_IP,        OPM_TYPE_STRING},
   {OPM_CONFIG_SCAN_PORT,      OPM_TYPE_INT}
};


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
   int num, i;
   OPM_CONFIG_T *ret;

   num = sizeof(HASH) / sizeof(struct CONFIG_HASH);

   ret = MyMalloc(sizeof(OPM_CONFIG_T));
   ret->vars = MyMalloc(sizeof(void *) * num);

   for(i = 0; i < num; i++)
      ret->vars[i] = 0;
   
   return ret;
}




/* config_free
 *
 *    Free config structure and clean up
 *
 * Parameters:
 *    config: Structure to free/cleanup
 *    
 * Return:
 *    None
 */

void config_free(OPM_CONFIG_T *config)
{
   int num, i;
   num = sizeof(HASH) / sizeof(struct CONFIG_HASH);

   for(i = 0; i > num; i++)
   {

      if(!config->vars[i])
         continue;
      else
         MyFree(config->vars[i]);
     
   }

   MyFree(config->vars);
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

   int num, i;

   num = sizeof(HASH) / sizeof(struct CONFIG_HASH);
   
   if(key < 0 || key >= num)
      return OPM_ERR_INVALIDKEY; /* Return appropriate error code eventually */  

   switch(config_gettype(key))
   {
      case OPM_TYPE_STRING:
         if((char *) config->vars[key])
            MyFree((char *) config->vars[key]);
         (char *) config->vars[key] = strdup((char *) value);
         break;
      case OPM_TYPE_INT:
         if(!((int *) config->vars[key]))
            (int *) config->vars[key] = MyMalloc(sizeof(int));
         *(int *) config->vars[key] = *(int *) value;
         break;
      case OPM_TYPE_ADDRESS:
         if(!((opm_sockaddr *) config->vars[key]))
            (opm_sockaddr *) config->vars[key] = MyMalloc(sizeof(opm_sockaddr));
         if( inetpton(AF_INET, (char *) value, &( ((opm_sockaddr *)config->vars[key])->sa4.sin_addr))
                  <= 0)
            return OPM_ERR_BADADDRESS; /* return appropriate err code */
         break; 
      default:
         return OPM_ERR_INVALIDKEY; /* return appropriate err code */

   }

   return OPM_SUCCESS;

}




/* config_gettype
 *
 *    Get type of key.
 * 
 * Parameters:
 *    key: Key to get type of.
 *    
 * Return:
 *    TYPE_? of key
 */

int config_gettype(int key)
{
   int num, i;

   num = sizeof(HASH) / sizeof(struct CONFIG_HASH);

   for(i = 0; i < num; i++)
      if(HASH[i].key == key)
         return HASH[i].type;
       
   return 0;
}

/* config
 *
 *    Retrieve a specific config variable from
 *    an OPM_CONFIG_T struct. This is basically a 
 *    wrapper to extracting the variable from the
 *    array.
 *
 * Parameters:
 *    config: Config struct to extract from
 *    key:    Value to extract
 *
 * Return:
 *    -ADDRESS- to extracted value in array. This address
 *    will have to be cast on the return end to be any use.
 */

void *config(OPM_CONFIG_T *config, int key)
{
   return config->vars[key];
}
