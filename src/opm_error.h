#ifndef LIBOPM_ERROR_H
#define LIBOPM_ERROR_H

/* Success */
#define OPM_SUCCESS               1

/* Configuration Errors */
#define OPM_ERR_BADKEY            2 /* Unknown or bad key value */
#define OPM_ERR_BADVALUE          3 /* Bad value matching key */
#define OPM_ERR_BADPROTOCOL       4 /* Unknown protocol in config */
#define OPM_ERR_DUPE              5 /* Attempt to add duplicate key */
#define OPM_ERR_BADTYPE           6 /* Bad config type specified */

/* Read Errors */
#define OPM_ERR_MAX_READ          7 /* Socket reached MAX_READ */

/* Callback Registration Errors */
#define OPM_ERR_CBNOTFOUND        8 /* Callback is out of range */

/* opm_scan errors */
#define OPM_ERR_BADADDR           9  /* IP in remote struct is bad */
#define OPM_ERR_NOPROTOCOLS       10 /* No protocols to scan! */

/* bind/connect errors */
#define OPM_ERR_BIND              11 /* Error binding to BIND_IP */
#define OPM_ERR_NOFD              12 /* Unable to allocate file descriptor */

/* other errors */
#define OPM_ERR_UNKNOWN           13 /* Unknown error */

#endif /* LIBOPM_ERROR_H */
