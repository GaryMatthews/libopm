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

list_t *list_create(void *data)
{
   list_t *node = (list_t *) malloc(sizeof(list_t));
   node->next = 0;
   node->prev = 0;
   node->data = (void *) data;

   return node;
}

list_t *list_add(list_t *list, list_t *node)
{

   list_t *p;

   if(!node)
      return 0;

   node->next = 0;

   if(!list)
   {
      list = node;
      list->prev = 0;
   }
   else
      for(p = list; p; p = p-> next)
      {
         if(!p->next)
         {
            p->next = node;
            node->prev = p;
         }
      }

   return node;
}
