/* vim: set shiftwidth=3 softtabstop=3 expandtab: */

/*
 * Copyright (C) 2002-2003  Andy Smith
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

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#ifdef HAVE_STRING_H
# include <string.h>
#endif

#include "opm.h"
#include "opm_error.h"
#include "opm_types.h"
#include "libopm.h"
#include "defs.h"

RCSID("$Id$");

int libopm_trojan_mindjail_write(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);
int libopm_trojan_mindjail_read(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);

#define LIBOPM_MINDJAIL_301            1
#define LIBOPM_MINDJAIL_LOCATION       2
#define LIBOPM_MINDJAIL_CONTENT_TYPE   4
#define LIBOPM_MINDJAIL_XPAD           8

extern char LIBOPM_SENDBUF[];

int libopm_trojan_mindjail_read(OPM_T *scanner, OPM_SCAN_T *scan,
    OPM_CONNECTION_T *conn)
{
   int max_read;
   char c;

   max_read = *(int *) libopm_config(scanner->config, "max_read");

   while(1)
   {
      switch(read(conn->fd, &c, 1))
      {
         case  0:
            libopm_do_hup(scanner, scan, conn);
            return 0;

         case -1:
            if(errno != EAGAIN)
               libopm_do_hup(scanner, scan, conn);
            return 0;

         default:
            conn->bytes_read++;
            
            if(conn->bytes_read >= max_read)
            {
               libopm_do_callback(scanner,
                   libopm_setup_remote(scan->remote, conn),
                   OPM_CALLBACK_ERROR, OPM_ERR_MAX_READ);
               conn->state = OPM_STATE_CLOSED;
               return 0;
            }
            
            if(c == '\0' || c == '\r')
               continue;
            
            if(c == '\n')
            {
               conn->readbuf[conn->readlen] = '\0';
               conn->readlen = 0;
               
               /*
                * A line of data has been read from the socket, check
                * against the various Mindjail fingerprints:
                *
                * GET /asdfargashakwuhgaeurghaejrghejrgherhgeiuheirgh HTTP/1.0
                * HTTP/1.1 301 Moved Permanently
                * Date: Mon, 23 Jun 2003 14:12:26 GMT
                * Server: Apache/1.3.26 (Unix)
                * Location: mindjail.zip
                * Last-Modified: Sat, 24 May 2003 15:56:09 GMT
                * ETag: "45401b-150e0-3ecf9619"
                * Accept-Ranges: bytes
                * Connection: close
                * Content-Type: application/zip
                * X-Pad: avoid browser bug
                *
                */
               if(strstr(conn->readbuf, "HTTP/1.1 301 Moved Permanently"))
                  conn->flags |= LIBOPM_MINDJAIL_301;
               else if(strstr(conn->readbuf, "Location: mindjail.zip"))
                  conn->flags |= LIBOPM_MINDJAIL_LOCATION;
               else if(strstr(conn->readbuf, "Content-Type: application/zip"))
                  conn->flags |= LIBOPM_MINDJAIL_CONTENT_TYPE;
               else if(strstr(conn->readbuf, "X-Pad: avoid browser bug"))
                  conn->flags |= LIBOPM_MINDJAIL_XPAD;
               
               if(conn->flags & (LIBOPM_MINDJAIL_301 |
                   LIBOPM_MINDJAIL_LOCATION |
                   LIBOPM_MINDJAIL_CONTENT_TYPE | LIBOPM_MINDJAIL_XPAD))
               {
                  /* Found one! */
                  libopm_do_openproxy(scanner, scan, conn);
               }
               
               if(conn->state == OPM_STATE_CLOSED)
                  return 0;
               
               continue;
            }
            
            if(conn->readlen < LIBOPM_READBUFLEN)
            {
               /* -1 to pad for null term. */
               conn->readbuf[++(conn->readlen) - 1] = c;
            }
      }
   }
}

int libopm_trojan_mindjail_write(OPM_T *scanner, OPM_SCAN_T *scan,
    OPM_CONNECTION_T *conn)
{
   USE_VAR(scan);
   USE_VAR(scanner);
   
   /*
    * GET some arbitrary stupid string.  Everything redirects to mindjail.zip
    * anyway, and that's a fingerprint.
    */
   snprintf(LIBOPM_SENDBUF, LIBOPM_SENDBUFLEN,
       "GET /asdfjhsdfaklaksdjalskdjalksjdlaksdjlaksfdkmrlkthmweg");
   if(send(conn->fd, LIBOPM_SENDBUF, strlen(LIBOPM_SENDBUF), 0) == -1)
      return 0; /* Return error code ? */
   
   return OPM_SUCCESS;
}
