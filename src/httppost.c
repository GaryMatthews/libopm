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

int libopm_proxy_httppost_write(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);

extern char LIBOPM_SENDBUF[];

/*
 * HTTP POST Scanning
 *
 */

int libopm_proxy_httppost_write(OPM_T *scanner, OPM_SCAN_T *scan, OPM_CONNECTION_T *conn)
{
   int len, scan_port;
   char *scan_ip;
   USE_VAR(scan);

   scan_ip = (char *) libopm_config(scanner->config, "scan_ip");
   scan_port = *(int *) libopm_config(scanner->config, "scan_port");

   len = snprintf(LIBOPM_SENDBUF, LIBOPM_SENDBUFLEN,
       "POST http://%s:%d/ HTTP/1.0\r\n"
       "Content-type: text/plain\r\n"
       "Content-length: 5\r\n\r\n"
       "quit\r\n\r\n", scan_ip, scan_port);

   send(conn->fd, LIBOPM_SENDBUF, (unsigned int)len, 0);

   return OPM_SUCCESS;
}
