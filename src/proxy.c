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
 * along with this program; if not, write to the Free Software
 *
 *       Foundation, Inc.
 *       59 Temple Place - Suite 330
 *       Boston, MA  02111-1307, USA.
 *
 *
 */

#include <stdio.h> /* snprintf */

#include "config.h"
#include "proxy.h"
#include "opm_common.h"
#include "opm_types.h"
#include "opm_error.h"
#include "libopm.h"
#include "inet.h"

static char SENDBUFF[512];

int proxy_http_write(OPM_T *scanner, OPM_SCAN_T *scan, OPM_CONNECTION_T *conn)
{
   snprintf(SENDBUFF, 128, "CONNECT %s:%d HTTP/1.0\r\n\r\n",
      (char *) config(scanner->config, OPM_CONFIG_SCAN_IP), 
      *(int *) config(scanner->config, OPM_CONFIG_SCAN_PORT));
 
   if(send(conn->fd, SENDBUFF, strlen(SENDBUFF), 0) == -1)
      return 0; /* Return error code ? */

   return OPM_SUCCESS;
}
