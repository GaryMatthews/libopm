/* vim: set shiftwidth=3 softtabstop=3 expandtab: */

/*
 * Copyright (C) 2002-2003  Erik Fears
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
 * along with this program; if not, write to
 *
 *       The Free Software Foundation, Inc.
 *       59 Temple Place - Suite 330
 *       Boston, MA  02111-1307, USA.
 *
 *
 */

#include "setup.h"

#include "malloc.h"
#include "config.h"
#include "inet.h"
#include "opm_error.h"
#include "opm_types.h"
#include "opm_common.h"
#include "list.h"
#include "defs.h"

#ifdef STDC_HEADERS
# include <string.h>
#endif

RCSID("$Id$");




/* libopm_config_create
 *
 *    Set up the default minimal list of config keys.
 *
 * Parameters:
 *    None;
 *
 * Return:
 *    Pointer to list.
 */

OPM_LIST_T *libopm_config_create()
{
   OPM_LIST_T *list;

   list = libopm_list_create();
   libopm_config_add(list, "fd_limit", OPM_TYPE_INT);
   libopm_config_add(list, "bind_ip", OPM_TYPE_ADDRESS);
   libopm_config_add(list, "dnsbl_host", OPM_TYPE_STRING);
   libopm_config_add(list, "target_string", OPM_TYPE_STRINGLIST);
   libopm_config_add(list, "scan_ip",	OPM_TYPE_STRING);
   libopm_config_add(list, "scan_port", OPM_TYPE_INT);
   libopm_config_add(list, "max_read", OPM_TYPE_INT);
   libopm_config_add(list, "timeout", OPM_TYPE_INT);
   libopm_config_add(list, "scan_url", OPM_TYPE_STRING);
   return list;
}




/* libopm_config_free
 *
 *    Free config list and clean up
 *
 * Parameters:
 *    config: List to free/cleanup
 *    
 * Return:
 *    None
 */

void libopm_config_free(OPM_LIST_T *config)
{
   OPM_NODE_T *p, *next, *lp, *lnext;
   OPM_LIST_T *list;
   OPM_CONFIG_T *c;

   LIST_FOREACH_SAFE(p, next, config->head)
   {
      c = (OPM_CONFIG_T *) p->data;
      
      switch(c->type)
      {
         case OPM_TYPE_STRINGLIST:
            list = (OPM_LIST_T *) c->value;

            LIST_FOREACH_SAFE(lp, lnext, list->head)
            {
               MyFree(lp->data);
               libopm_list_remove(list, lp);
               libopm_node_free(lp);
            }

            libopm_list_free(list);
            break;

         default:
            MyFree(c->value);
            break;
      }

      MyFree(c->key);
      MyFree(c);
      libopm_list_remove(config, p);
      libopm_node_free(p);
   }

   libopm_list_free(config);
}



/* libopm_config_set
 *
 *    Set configuration options on config struct.
 *
 * Parameters:
 *    config: Config list to set parameters on
 *    key:    Variable within the struct to set
 *    value:  Address of value to set 
 *
 * Return:
 *    OPM_SUCCESS: Variable was set
 *    OPM_ERR_BADVALUE: Unable to set the given value (bad formatting).
 *    OPM_ERR_BADKEY: The key (configuration option) is unknown.
 *    OPM_ERR_UNKNOWN: Unknown error.
 */

OPM_ERR_T libopm_config_set(OPM_LIST_T *config, const char *key, void *value)
{
   OPM_NODE_T *node, *lnode;
   OPM_CONFIG_T *c;
   opm_sockaddr *sa;

   if(!key)
      return OPM_ERR_BADKEY;
   
   LIST_FOREACH(node, config->head)
   {
      c = (OPM_CONFIG_T *) node->data;
      
      if(strcmp(key, c->key) == 0)
      {
         switch(c->type)
         {
            case OPM_TYPE_STRING:
               if((char *) c->value != NULL)
                  MyFree(c->value);
               (char *) c->value = strdup((char *) value);
               break;

            case OPM_TYPE_INT:
               memcpy(c->value, value, sizeof(int));
               break;

            case OPM_TYPE_ADDRESS:
               sa = (opm_sockaddr *) c->value;
               
               if(inet_pton(AF_INET, (char *) value,
                            &sa->sa4.sin_addr) <= 0)
                  return OPM_ERR_BADVALUE;
               break; 

            case OPM_TYPE_STRINGLIST:
               lnode = libopm_node_create(strdup((char *) value));
               libopm_list_add((OPM_LIST_T *) c->value, lnode);
               break;

            default:
               return OPM_ERR_UNKNOWN;
         }
         
         return OPM_SUCCESS;
      }
   }

   return OPM_ERR_BADKEY;
}



/* libopm_config_add
 *
 *    Add a new config key.
 *
 * Parameters:
 *    config: list to add the key to
 *    key: key to add
 *    type: type of data the key holds
 *
 * Return:
 *    OPM_SUCCESS: succeeded
 *    OPM_ERR_BADTYPE: Unknown config item type
 *    OPM_ERR_DUPE: Duplicate config key specified
 *    OPM_ERR_UNKNOWN: Unknown error
 */

OPM_ERR_T libopm_config_add(OPM_LIST_T *config, const char *key, int type)
{
   OPM_NODE_T *node;
   OPM_CONFIG_T *c;

   LIST_FOREACH(node, config->head)
   {
      c = node->data;
      
      if(strcmp(key, c->key) == 0)
         return OPM_ERR_DUPE;
   }

   c = MyMalloc(sizeof *c);
   c->key = strdup(key);
   c->type = type;

   node = libopm_node_create(c);
   
   switch(type)
   {
      case OPM_TYPE_STRINGLIST:
         c->value = libopm_list_create();
         break;

      case OPM_TYPE_STRING:
         c->value = strdup("");
         break;

      case OPM_TYPE_INT:
         c->value = MyMalloc(sizeof(int));
         memset(c->value, 0, sizeof(int));
         break;

      case OPM_TYPE_ADDRESS:
         c->value = MyMalloc(sizeof(opm_sockaddr));
         memset(c->value, 0, sizeof(opm_sockaddr));
         break;

      default:
         MyFree(c->key);
         MyFree(c);
         libopm_node_free(node);
         return OPM_ERR_BADTYPE;
   }

   libopm_list_add(config, node);
   return OPM_SUCCESS;
}



         
/* libopm_config
 *
 *    Retrieve a specific config variable from
 *    a config list.
 *
 * Parameters:
 *    config: Config list to extract from
 *    key:    Value to extract
 *
 * Return:
 *    -ADDRESS- to extracted value in array. This address
 *    will have to be cast on the return end to be any use.
 */

void *libopm_config(OPM_LIST_T *config, const char *key)
{
   OPM_NODE_T *node;
   OPM_CONFIG_T *c;

   LIST_FOREACH(node, config->head)
   {
      c = node->data;

      if(strcmp(key, c->key) == 0)
         return c->value;
   }

   return NULL;
}
