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

#include "config.h"
#include "libopm.h"
#include "malloc.h"
#include "opm_error.h"
#include "opm_types.h"
#include "opm_common.h"
#include "list.h"
#include "inet.h"

OPM_PROTOCOL_CONFIG_T *protocol_config_create();
void protocol_config_free(OPM_PROTOCOL_CONFIG_T *);
OPM_SCAN_T *scan_create(OPM_T *, OPM_REMOTE_T *);
void scan_free(OPM_SCAN_T *);
OPM_CONNECTION_T *connection_create();

void check_establish(OPM_T *);
void check_poll(OPM_T *);
void do_connect(OPM_CONNECTION_T *, OPM_REMOTE_T *);

/* OPM_PROTOCOLS hash
 *
 *    OPM_PPROTOCOLS hashes the protocol types (int) to functions
 *    which handle the protocol (sending/receiving protocol specific
 *    data).
 *
 */

OPM_PROTOCOL_T OPM_PROTOCOLS[] = {
    {OPM_TYPE_HTTP,               0},
    {OPM_TYPE_SOCKS4,             0},
    {OPM_TYPE_SOCKS5,             0},
    {OPM_TYPE_WINGATE,            0},
    {OPM_TYPE_ROUTER,             0}
};




/* opm_create
 *
 *    Initialize a new scanner and return a pointer to it.
 *
 * Parameters:
 *    None
 *  
 * Return 
 *    Pointer to new OPM_T (scanner)
 */

OPM_T *opm_create()
{
   OPM_T *ret;

   ret = MyMalloc(sizeof(OPM_T));

   ret->config = config_create();
   ret->scans = list_create();
   ret->protocols = list_create();

   return ret;
}




/* opm_remote_create
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

OPM_REMOTE_T *opm_remote_create(char *ip)
{ 

   OPM_REMOTE_T *ret;
   ret = MyMalloc(sizeof(OPM_REMOTE_T));

   /* Do initializations */
   if(ip == NULL)
      return NULL;

   ret->ip = (char*) strdup(ip);  /* replace with custom strdup function */
 

   ret->fun_openproxy = 0;
   ret->fun_negfail   = 0;
   ret->fun_end       = 0;
   ret->fun_error     = 0;
   ret->fun_timeout   = 0;
 
   ret->port          = 0;
   ret->protocol      = 0;
   ret->bytes_read    = 0;

   memset(&(ret->addr), 0, sizeof(opm_sockaddr));

   if(inetpton(AF_INET, ret->ip, &(ret->addr.sa4.sin_addr) ) <= 0)
   {
      opm_remote_free(ret);
      return NULL;
   }

   return ret;
}



/* opm_remote_free
 *
 *    Free OPM_REMOTE_T struct and cleanup
 *
 * Parameters:
 *    remote: Struct to free
 *
 * Return:
 *    None
 */

void opm_remote_free(OPM_REMOTE_T *remote)
{
   if(remote->ip)
      MyFree(remote->ip);

   MyFree(remote);
}




/* opm_free
 *
 *    Free OPM_T (scanner) and cleanup
 *
 * Parameters:
 *    scanner: Address of OPM_T to cleanup
 *
 * Return:
 *    None
 */

void opm_free(OPM_T *scanner)
{
   node_t *p, *next;
   OPM_PROTOCOL_CONFIG_T *ppc;


   config_free(scanner->config);

   LIST_FOREACH_SAFE(p, next, scanner->protocols->head)
   {
      ppc = (OPM_PROTOCOL_CONFIG_T *) p->data;
      protocol_config_free(ppc);
      list_remove(scanner->protocols, p);
      node_free(p);
   }

   list_free(scanner->protocols);
   list_free(scanner->scans);

   MyFree(scanner);
}




/* opm_config
 *
 *    Wrapper to config_set. Set configuration variables
 *    on the config struct.
 *
 * Parameters:
 *    scanner: OPM_T struct the config struct resides in
 *    key: Variable within the config struct to set
 *    value: Address of value to set variable (key) to
 *
 * Return:
 *    1: Config variable set
 *    0: Some error occured
 */

OPM_ERR_T opm_config(OPM_T *scanner, int key, void *value)
{
   return config_set((scanner->config), key, value);
}




/* opm_addtype
 *
 *    Add a proxy type and port to the list of protocols 
 *    a scanner will use.
 * 
 * Parameters:
 *    scanner: pointer to scanner struct
 *    type:    type of proxy to scan (used in hashing to the functions)
 *    port:    port this specific type/protocol will scan on
 * Return:
 *    (write in future error codes)
 */

int opm_addtype(OPM_T *scanner, int type, int port)
{
   int i;
   node_t *node;
   OPM_PROTOCOL_CONFIG_T *protocol;

   for(i = 0; i < sizeof(OPM_PROTOCOLS) / sizeof(OPM_PROTOCOL_T); i++)
   {
      if(type == OPM_PROTOCOLS[i].type)
      {
         protocol = protocol_config_create();

         protocol->type = &OPM_PROTOCOLS[i];
         protocol->port = port;
  
         node = node_create(protocol);
         list_add(scanner->protocols, node);

      }
   }
}




/* protocol_config_create
 *
 *    Allocate and return address of a new OPM_PROTOCOL_CONFIG_T
 *
 * Parameters:
 *    None
 *
 * Return:
 *    Address of new OPM_PROTOCOL_CONFIG_T
 */

OPM_PROTOCOL_CONFIG_T *protocol_config_create()
{
   OPM_PROTOCOL_CONFIG_T *ret;
   ret = MyMalloc(sizeof(OPM_PROTOCOL_CONFIG_T));

   return ret;
}




/* protocol_config_free
 *
 *    Free OPM_PROTOCOL_CONFIG_T struct
 *
 * Parameters:
 *    protocol: struct to free
 *
 * Return:
 *    None
 */

void protocol_config_free(OPM_PROTOCOL_CONFIG_T *protocol)
{
   MyFree(protocol);
}




/* opm_scan
 *
 *    Scan remote host. The opm_scan function takes an OPM_REMOTE_T
 *    struct, calculates the in_addr of the remote host, and creates
 *    a scan list based on protocols defined in the scanner.
 *
 * Parameters:
 *    scanner: Scanner to scan host on
 *    remote:  OPM_REMOTE_T defining remote host
 *    
 * Return:
 *    (to be written)
 */

OPM_ERR_T opm_scan(OPM_T *scanner, OPM_REMOTE_T *remote)
{
   OPM_SCAN_T *scan; /* New scan for OPM_T */
   node_t *node;     /* Node we'll add scan to
                        when we link it to scans */

   scan = scan_create(scanner, remote);
   node = node_create(scan);

   list_add(scanner->scans, node);

   return OPM_SUCCESS;
}



/* scan_create
 *
 *    Create new OPM_SCAN_T struct
 *
 * Parameters: 
 *    scanner: Scanner the scan is being created for. This
 *             is needed to get information on currently set
 *             protocols/config.
 *             
 *    remote: Remote host this scan will be scanning
 *    
 * Return
 *    Address of new struct
 */
OPM_SCAN_T *scan_create(OPM_T *scanner, OPM_REMOTE_T *remote)
{
   OPM_SCAN_T *ret;

   OPM_CONNECTION_T *conn;
   node_t *node, *p;

   ret = MyMalloc(sizeof(OPM_SCAN_T));

   ret->remote = remote;
   ret->connections = list_create();
 
   /* Setup list of connections, one for each protocol */ 
   LIST_FOREACH(p, scanner->protocols->head)
   {
      conn = connection_create();

      conn->protocol = ((OPM_PROTOCOL_CONFIG_T *) p->data)->type;
      conn->port     = ((OPM_PROTOCOL_CONFIG_T *) p->data)->port;
 
      node = node_create(conn);

      list_add(ret->connections, node);
   }

   

   return ret;
}




/* scan_free
 *
 *    Free and cleanup OPM_SCAN_T struct
 *
 * Parametsr:
 *    scan: Scan struct to free
 * 
 * Return:
 *    None
 */

void scan_free(OPM_SCAN_T *scan)
{
   MyFree(scan);
}




/* connection_create
 *
 *    Allocate new OPM_CONNECTION_T
 *
 * Parameters:
 *    None
 *
 * Return:
 *    Address of new OPM_CONNECTION_T
 */

OPM_CONNECTION_T *connection_create()
{
   OPM_CONNECTION_T *ret;
   ret = MyMalloc(sizeof(OPM_CONNECTION_T));

   ret->fd         = 0;
   ret->bytes_read = 0;
   ret->readlen    = 0;
   ret->state      = OPM_STATE_UNESTABLISHED;
   ret->protocol   = 0;
   ret->port       = 0;

   return ret;
}




/* connection_free
 *
 *    Free OPM_CONNECTION_T struct
 *
 * Parameters:
 *    conn: Address of struct to free
 *
 * Return:
 *    None
 */

void connection_free(OPM_CONNECTION_T *conn)
{
   MyFree(conn);
}


/* opm_cycle
 *
 *   Perform tasks (called by client's loop)
 *
 * Parameters:
 *   None
 *  Return:
 *    None
 */

void opm_cycle(OPM_T *scanner)
{
   /* Make new connections if FDs are free*/
   check_establish(scanner);
   check_poll(scanner);
}

/* check_establish
 *
 * Make new connections if there are free file descriptors and connections
 * to be made.
 *
 * Parameters:
 *   scanner: Scanner to check for establish on
 * Return:
 *   None
 */

void check_establish(OPM_T *scanner)
{
   node_t *node1;
   node_t *node2;

   OPM_SCAN_T *scan;
   OPM_CONNECTION_T *conn;

   LIST_FOREACH(node1, scanner->scans->head)
   {
      scan = (OPM_SCAN_T *) node1->data;
      LIST_FOREACH(node2, scan->connections->head)
      {
         conn = (OPM_CONNECTION_T *) node2->data;
         if(conn->state == OPM_STATE_UNESTABLISHED)
            do_connect(conn, scan->remote);
      } 
   }
}

/* do_connect
 *
 * Call socket() and connect() to start a scan.
 *
 * Parametsr:
 *    conn: Connection to establish
 *    remote: Information regarding remote connection (addr, and callback functions)
 * Return: 
 *    None
 */

void do_connect(OPM_CONNECTION_T *conn, OPM_REMOTE_T *remote)
{
   opm_sockaddr *addr;
  
   addr = &(remote->addr); /* Already have the IP in byte format from 
                              opm_remote_connect */
   addr->sa4.sin_family   = AF_INET;
   addr->sa4.sin_port     = conn->port;

   printf("do_connect ON %s PORT %d\n",  inet_ntoa(addr->sa4.sin_addr), conn->port);

   /* Do bind */

   conn->fd = socket(AF_INET, SOCK_STREAM, 0);

   if(conn->fd == -1)
      ; /* Handle error */

   /* Set socket non blocking */
   fcntl(conn->fd, F_SETFL, O_NONBLOCK);
  
   connect(conn->fd, (struct sockaddr *) &(addr->sa4),
            sizeof(struct sockaddr));
   
   conn->state = OPM_STATE_ESTABLISHED;
}


/* check_poll
 *
 * Check sockets for ready read/write
 *
 * Parameters:
 *    scanner: Scanner to isolate check on
 * Return:
 *    None
 */

void check_poll(OPM_T *scanner)
{


}
