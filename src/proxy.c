/* Copyright (C) 2002  Erik Fears
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

#include <stdio.h> /* snprintf */

#include "config.h"
#include "proxy.h"
#include "opm_common.h"
#include "opm_types.h"
#include "opm_error.h"
#include "libopm.h"
#include "inet.h"
#include "compat.h"

RCSID("$Id$");

static char SENDBUFF[512];


/* proxy_custom
 *
 * Handle writing of custom protocol
 *
 * Parameters:
 *    scanner: scanner object
 *    scan: scan object
 *    conn: specific connection to handle scan on
 *
 * Return:
 *    None
 */

int libopm_proxy_custom(OPM_T *scanner, OPM_SCAN_T *scan, OPM_CONNECTION_T *conn)
{
   OPM_PROTOCOL_T *protocol;
   int sblen, fmpos;
   char c, tmp[32];

   char *scan_ip                =  (char *) libopm_config(scanner->config, OPM_CONFIG_SCAN_IP);
   unsigned short int scan_port =  *(int *) libopm_config(scanner->config, OPM_CONFIG_SCAN_PORT);
  
   protocol = conn->protocol;
   sblen = fmpos = 0;
   SENDBUFF[0] = '\0';

   while((c = protocol->format[fmpos++]) != '\0' && sblen < 512)
   {
      if(c == '%')
      {
         switch(protocol->format[fmpos])
         {
            case '\0':
               continue;
            case '%':
               break;
            case 'i':
               strncat(SENDBUFF, scan_ip, (511 - sblen));
               break;
            case 'p':
               snprintf(tmp, 31, "%d", scan_port);
               strncat(SENDBUFF, tmp, (511 - sblen));
               break;
         }

         sblen = strlen(SENDBUFF);
         fmpos++;
      }
      else
      {
         SENDBUFF[sblen] = c;
         SENDBUFF[++sblen] = '\0';
      }
   }


   printf("SENDBUFF: [%s]\n", SENDBUFF);

   return OPM_SUCCESS;
}


int libopm_proxy_http_write(OPM_T *scanner, OPM_SCAN_T *scan, OPM_CONNECTION_T *conn)
{
   snprintf(SENDBUFF, 128, "CONNECT %s:%d HTTP/1.0\r\n\r\n",
      (char *) libopm_config(scanner->config, OPM_CONFIG_SCAN_IP), 
      *(int *) libopm_config(scanner->config, OPM_CONFIG_SCAN_PORT));
 
   if(send(conn->fd, SENDBUFF, strlen(SENDBUFF), 0) == -1)
      return 0; /* Return error code ? */

   return OPM_SUCCESS;
}


/*
 * CONNECT request byte order for socks4
 *
 *              +----+----+----+----+----+----+----+----+----+----+....+----+
 *              | VN | CD | DSTPORT |      DSTIP        | USERID       |NULL|
 *              +----+----+----+----+----+----+----+----+----+----+....+----+
 *   # of bytes:  1    1      2              4           variable       1
 *
 *  VN = Version, CD = Command Code (1 is connect request)
 */

int libopm_proxy_socks4_write(OPM_T *scanner, OPM_SCAN_T *scan, OPM_CONNECTION_T *conn)
{
   struct in_addr addr;
   unsigned long laddr;
   int len, scan_port;
   char *scan_ip;
         
   scan_ip = (char *) libopm_config(scanner->config, OPM_CONFIG_SCAN_IP);
   scan_port = *(int *) libopm_config(scanner->config, OPM_CONFIG_SCAN_PORT);

   if (inet_aton(scan_ip, &addr) == 0)
       ; /* handle error */

   laddr = htonl(addr.s_addr);

   len = snprintf(SENDBUFF, 512, "%c%c%c%c%c%c%c%c%c",  4, 1,
                 (((unsigned short) scan_port) >> 8) & 0xFF,
                 (((unsigned short) scan_port) & 0xFF),
                 (char) (laddr >> 24) & 0xFF, (char) (laddr >> 16) & 0xFF,
                 (char) (laddr >> 8) & 0xFF, (char) laddr & 0xFF, 0);

   send(conn->fd, SENDBUFF, len, 0);

   return OPM_SUCCESS;
}


/*
 * Send version authentication selection message to socks5
 *
 *       +----+----------+----------+
 *       |VER | NMETHODS | METHODS  |
 *       +----+----------+----------+
 *       | 1  |    1     | 1 to 255 |
 *       +----+----------+----------+
 *
 *  VER always contains 5, for socks version 5
 *  Method 0 is 'No authentication required'
 *
 *
 *
 *  The SOCKS request is formed as follows:
 *
 *        +----+-----+-------+------+----------+----------+
 *       |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
 *       +----+-----+-------+------+----------+----------+
 *       | 1  |  1  | X'00' |  1   | Variable |    2     |
 *       +----+-----+-------+------+----------+----------+
 *
 *
 *         o  VER    protocol version: X'05'
 *         o  CMD
 *            o  CONNECT X'01'
 *            o  BIND X'02'
 *            o  UDP ASSOCIATE X'03'
 *         o  RSV    RESERVED
 *         o  ATYP   address type of following address
 *            o  IP V4 address: X'01'
 *            o  DOMAINNAME: X'03'
 *            o  IP V6 address: X'04'
 *         o  DST.ADDR       desired destination address
 *         o  DST.PORT desired destination port in network octet
 *            order
 *
 *
 */

int libopm_proxy_socks5_write(OPM_T *scanner, OPM_SCAN_T *scan, OPM_CONNECTION_T *conn)
{
   struct in_addr addr;
   unsigned long laddr;
   int len, scan_port;
   char *scan_ip;

   scan_ip = (char *) libopm_config(scanner->config, OPM_CONFIG_SCAN_IP);
   scan_port = *(int *) libopm_config(scanner->config, OPM_CONFIG_SCAN_PORT);


   if (inet_aton(scan_ip, &addr) == 0) 
       ; /* handle error */

   laddr = htonl(addr.s_addr);

   /* Form authentication string */
   /* Version 5, 1 number of methods, 0 method (no auth). */
   len = snprintf(SENDBUFF, 512, "%c%c%c", 5, 1, 0);
   send(conn->fd, SENDBUFF, len, 0);

   /* Form request string */

   /* Will need to write ipv6 support here in future
    * as socks5 is ipv6 compatible
    */
   len = snprintf(SENDBUFF, 512, "%c%c%c%c%c%c%c%c%c%c", 5, 1, 0, 1,
                 (char) (laddr >> 24) & 0xFF, (char) (laddr >> 16) & 0xFF,
                 (char) (laddr >> 8) & 0xFF, (char) laddr & 0xFF,
                 (((unsigned short) scan_port) >> 8) & 0xFF,
                 (((unsigned short) scan_port) & 0xFF));

   send(conn->fd, SENDBUFF, len, 0);
   return OPM_SUCCESS;

}

/*
 * Open wingates require no authentication, they will send a prompt when
 * connect.
 */

int libopm_proxy_wingate_write(OPM_T *scanner, OPM_SCAN_T *scan, OPM_CONNECTION_T *conn)
{
   int scan_port, len;
   char *scan_ip;

   scan_ip = (char *) libopm_config(scanner->config, OPM_CONFIG_SCAN_IP);
   scan_port = *(int *) libopm_config(scanner->config, OPM_CONFIG_SCAN_PORT);

   len = snprintf(SENDBUFF, 512, "%s:%d\r\n", scan_ip, scan_port);
   send(conn->fd, SENDBUFF, len, 0);

   return OPM_SUCCESS;
}


/*
 * Cisco scanning
 *
 * Some cisco routers have 'cisco' set as password which allow open telnet
 * relay. Attempt to connect using cisco as a password, then give command for
 * telnet to the scanip/scanport
 */

int libopm_proxy_router_write(OPM_T *scanner, OPM_SCAN_T *scan, OPM_CONNECTION_T *conn)
{
   int len, scan_port;
   char *scan_ip;

   scan_ip = (char *) libopm_config(scanner->config, OPM_CONFIG_SCAN_IP);
   scan_port = *(int *) libopm_config(scanner->config, OPM_CONFIG_SCAN_PORT);

   len = snprintf(SENDBUFF, 512, "cisco\r\n");
   send(conn->fd, SENDBUFF, len, 0);

   len = snprintf(SENDBUFF, 512, "telnet %s %d\r\n", scan_ip, scan_port);
   send(conn->fd, SENDBUFF, len, 0);

   return OPM_SUCCESS;
}

