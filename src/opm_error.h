#ifndef LIBOPM_ERROR_H
#define LIBOPM_ERROR_H

/* Success */
#define OPM_SUCCESS               1

/* Configuration Errors */
#define OPM_ERR_BADKEY            2 /* Unknown or bad key value */
#define OPM_ERR_BADVALUE          3 /* Bad value matching key */
#define OPM_ERR_BADPROTOCOL       4 /* Unknown protocol in config */

/* Read Errors */
#define OPM_ERR_MAX_READ          5 /* Socket reached MAX_READ */

/* Callback Registration Errors */
#define OPM_ERR_CBNOTFOUND        6 /* Callback is out of range */

#endif /* LIBOPM_ERROR_H */
