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
#include "proxy.h"
#include <errno.h>

OPM_PROTOCOL_CONFIG_T *protocol_config_create();
void protocol_config_free(OPM_PROTOCOL_CONFIG_T *);
OPM_SCAN_T *scan_create(OPM_T *, OPM_REMOTE_T *);
void scan_free(OPM_SCAN_T *);
OPM_CONNECTION_T *connection_create();

void check_establish(OPM_T *);
void check_poll(OPM_T *);

void do_connect(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);
void do_readready(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);
void do_writeready(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);
void do_hup(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);

void do_read(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);
void do_openproxy(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);

/* OPM_PROTOCOLS hash
 *
 *    OPM_PPROTOCOLS hashes the protocol types (int) to functions
 *    which handle the protocol (sending/receiving protocol specific
 *    data).
 *
 */

OPM_PROTOCOL_T OPM_PROTOCOLS[] = {
    {OPM_TYPE_HTTP,               proxy_http_write, NULL},
    {OPM_TYPE_SOCKS4,             NULL, NULL},
    {OPM_TYPE_SOCKS5,             NULL, NULL},
    {OPM_TYPE_WINGATE,            NULL, NULL},
    {OPM_TYPE_ROUTER,             NULL, NULL}
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
 

   ret->fun_openproxy = NULL;
   ret->fun_negfail   = NULL;
   ret->fun_end       = NULL;
   ret->fun_error     = NULL;
   ret->fun_timeout   = NULL;
 
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
 *    OPM_ERR_T containing error code
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

OPM_ERR_T opm_addtype(OPM_T *scanner, int type, int port)
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

         return OPM_SUCCESS;

      }
   }
   return OPM_ERR_BADPROTOCOL;
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
   ret->protocol   = 0;
   ret->port       = 0;

   ret->state      = OPM_STATE_UNESTABLISHED;


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
   node_t *node1, *node2;

   OPM_SCAN_T *scan;
   OPM_CONNECTION_T *conn;

   if(LIST_SIZE(scanner->scans) == 0)
      return;

   LIST_FOREACH(node1, scanner->scans->head)
   {
      scan = (OPM_SCAN_T *) node1->data;
      LIST_FOREACH(node2, scan->connections->head)
      {
         conn = (OPM_CONNECTION_T *) node2->data;
         if(conn->state == OPM_STATE_UNESTABLISHED)
            do_connect(scanner, scan, conn);
      } 
   }
}

/* do_connect
 *
 * Call socket() and connect() to start a scan.
 *
 * Parametsr:
 *    scan: Scan struct containing the connection
 *    conn: Connection to establish
 * Return: 
 *    None
 */

void do_connect(OPM_T * scanner, OPM_SCAN_T *scan, OPM_CONNECTION_T *conn)
{
   struct sockaddr_in *addr;
  
   addr = (struct sockaddr_in *) &(scan->remote->addr.sa4); /* Already have the IP in byte format from 
                                    opm_remote_connect */
   addr->sin_family   = AF_INET;
   addr->sin_port     = htons(conn->port);

   /* Do bind */

   conn->fd = socket(PF_INET, SOCK_STREAM, 0);

   if(conn->fd == -1)
      ; /* Handle error */

   /* Set socket non blocking */
   fcntl(conn->fd, F_SETFL, O_NONBLOCK);

   connect(conn->fd, (struct sockaddr *) addr, sizeof(*addr));

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
   node_t *node1, *node2;
   OPM_SCAN_T *scan;
   OPM_CONNECTION_T *conn;

   static struct pollfd ufds[1024]; /* REPLACE WITH MAX_POLL */
   unsigned int size, i;
   size = 0;

   if(LIST_SIZE(scanner->scans) == 0)
      return;

   LIST_FOREACH(node1, scanner->scans->head)
   {
      scan = (OPM_SCAN_T *) node1->data;
      LIST_FOREACH(node2, scan->connections->head)
      {
         if(size >= 1024)
            break;

         conn = (OPM_CONNECTION_T *) node2->data;
       
         if(conn->state < OPM_STATE_ESTABLISHED ||
            conn->state == OPM_STATE_CLOSED)
               continue;                /* This needs work */

         ufds[size].events = 0;
         ufds[size].revents = 0;
         ufds[size].fd = conn->fd;

         /* Check for HUNG UP. */
         ufds[size].events |= POLLHUP;
         /* Check for INVALID FD */
         ufds[size].events |= POLLNVAL;

         switch(conn->state)
         {
            case OPM_STATE_ESTABLISHED:
               ufds[size].events |= POLLOUT;
               break;
            case OPM_STATE_NEGSENT:
               ufds[size].events |= POLLIN;
               break;
         }
         size++;
      }
   }

   switch (poll(ufds, size, 1000))
   {
        case -1:
                /* error in select/poll */
                return;
        case 0:
                /* Nothing to do */
                return;
                /* Pass pointer to connection to handler. */
   }

   LIST_FOREACH(node1, scanner->scans->head)
   {
      scan = (OPM_SCAN_T *) node1->data;
 
      LIST_FOREACH(node2, scan->connections->head)
      {
         conn = (OPM_CONNECTION_T *) node2->data;

         for(i = 0; i < size; i++)
         {
            if(ufds[i].fd == conn->fd)
            {
               if(ufds[i].revents & POLLIN)
                  do_readready(scanner, scan, conn);
               if(ufds[i].revents & POLLOUT)
                  do_writeready(scanner, scan, conn);
               if(ufds[i].revents & POLLHUP)
                  do_hup(scanner, scan, conn);
            }
         }
      }
   }
}

/* do_readready
 *
 *    Remote connection is read ready, read the data into a buffer and check it against 
 *    the target_string if neccessary 
 *
 *    Parameters:
 *       scanner: Scanner doing the scan
 *       scan: Specific scan
 *       conn: Specific connection in the scan
 *
 *    Return:
 *       None
 */

void do_readready(OPM_T *scanner, OPM_SCAN_T *scan, OPM_CONNECTION_T *conn)
{
   char c;
   while(1)
   {
      switch (read(conn->fd, &c, 1))
      {
         case  0:
         case -1:
            return;

         default:

            conn->bytes_read++;

            if(c == 0 || c == '\r')
               continue;

            if(c == '\n')
            {
               conn->readbuf[conn->readlen] = '\0';
               conn->readlen = 0;
               do_read(scanner, scan, conn);
               continue;
            }

            if(conn->readlen < 128) 
            {  /* -1 to pad for null term */
               conn->readbuf[++(conn->readlen) - 1] = c;
            }
      }
   }
}

/* do_read
 *
 *    A line of data has been read from the socket, check it against
 *    target string.
 *
 *    
 *
 *    Parameters:
 *       scanner: Scanner doing the scan
 *       scan: Specific scan
 *       conn: Specific connection in the scan
 *
 *    Return:
 *       None
 */

void do_read(OPM_T *scanner, OPM_SCAN_T *scan, OPM_CONNECTION_T *conn)
{
   char *target_string;
 
   target_string = (char *) config(scanner->config, OPM_CONFIG_TARGET_STRING);

   if(strstr(conn->readbuf, target_string))
      do_openproxy(scanner, scan, conn);
}


/* do_openproxy
 *
 *    An open proxy was found on connection conn. Cleanup the connection and 
 *    call the appropriate callback to let the client know the proxy was found.
 *
 *    Parameters:
 *       scanner: Scanner doing the scan
 *       scan: Specific scan
 *       conn: Specific connection in the scan
 *
 *    Return:
 *       None
 */

void do_openproxy(OPM_T *scanner, OPM_SCAN_T *scan, OPM_CONNECTION_T *conn)
{
   OPM_REMOTE_T *remote;
   OPM_PROTOCOL_T *protocol;

   remote = scan->remote;

   /* Close the socket */
   close(conn->fd);

   /* Mark the connection for close */
   conn->state = OPM_STATE_CLOSED;

   /* Setup the remote struct with callback info */
   remote->port = conn->port;
   remote->bytes_read = conn->bytes_read;
   remote->protocol = conn->protocol->type;

   /* Call client's open proxy callback */
   if(remote->fun_openproxy != NULL)
      remote->fun_openproxy(remote, 0);
}

/*  do_writeready
 *
 *    Remote connection is write ready, call the specific protocol
 *    function for writing to this socket.
 *
 *    Parameters:
 *       scanner: Scanner doing the scan
 *       scan: Specific scan
 *       conn: Specific connection in the scan
 *
 *    Return:
 *       None
 */

void do_writeready(OPM_T *scanner, OPM_SCAN_T *scan, OPM_CONNECTION_T *conn)
{
   OPM_PROTOCOL_T *protocol;

   protocol = conn->protocol;

   /* Call write function for specific protocol */
   protocol->write_function(scanner, scan, conn);

   /* Flag as NEGSENT so we don't have to send data again*/
   conn->state = OPM_STATE_NEGSENT;  
}



/* do_hup
 *
 *    Connection ended prematurely
 *
 * Parameters:
 *       scanner: Scanner doing the scan
 *       scan: Specific scan
 *       conn: Specific connection in the scan
 *       error: OPM_ERR_T containing the error type
 * Return:
 *       None
 */

void do_hup(OPM_T *scanner, OPM_SCAN_T *scan, OPM_CONNECTION_T *conn)
{
   OPM_REMOTE_T *remote;
   OPM_PROTOCOL_T *protocol;

   remote = scan->remote;

   /* Close the socket */
   close(conn->fd);

  /* Mark the connection for close */
   conn->state = OPM_STATE_CLOSED;

  /* Setup the remote struct with callback info */
   remote->port = conn->port;
   remote->bytes_read = conn->bytes_read;
   remote->protocol = conn->protocol->type;

  /* Call client's open proxy callback */
   if(remote->fun_negfail != NULL)
      remote->fun_negfail(remote, 0);
}

/* do_error
 * 
 *    Handle error by calling callback fun_error
 *  
 * Parameters:
 *       scanner: Scanner doing the scan
 *       scan: Specific scan
 *       conn: Specific connection in the scan
 *       error: OPM_ERR_T containing the error type
 */

void do_error(OPM_REMOTE_T *remote, int error)
{
   remote->fun_error(remote, error);
}
