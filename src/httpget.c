/* vim: set shiftwidth=3 softtabstop=3 expandtab: */

/*
 * Copyright (C) 2002-2003  Mark Bergsma
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

int libopm_proxy_httpget_write(OPM_T *, OPM_SCAN_T *, OPM_CONNECTION_T *);

extern char LIBOPM_SENDBUF[];

/*
 * HTTP GET Scanning
 *
 */
int libopm_proxy_httpget_write(OPM_T *scanner, OPM_SCAN_T *scan, OPM_CONNECTION_T *conn)
{
   int len;
   char *scan_url;

   USE_VAR(scan);
   
   scan_url = (char *) libopm_config(scanner->config, "scan_url");
   
   len = snprintf(LIBOPM_SENDBUF, LIBOPM_SENDBUFLEN,
       "GET %s HTTP/1.0\r\n\r\n", scan_url);

   send(conn->fd, LIBOPM_SENDBUF, len, 0);
   return 1;
}
