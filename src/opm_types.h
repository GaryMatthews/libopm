#ifndef OPM_TYPES_H
#define OPM_TYPES_H

/* Configuration Directives */
#define CONFIG_FD_LIMIT     0
#define CONFIG_BIND_IP      1
#define CONFIG_DNSBL_HOST   2


/* Configuration Variable Types */
#define TYPE_INT            1
#define TYPE_STRING         2
#define TYPE_ADDRESS        3

/* Protocols */
#define OPM_TYPE_HTTP    1
#define OPM_TYPE_SOCKS4  2
#define OPM_TYPE_SOCKS5  3
#define OPM_TYPE_WINGATE 4
#define OPM_TYPE_ROUTER  5

/* States */
#define OPM_STATE_UNESTABLISHED 0
#define OPM_STATE_ESTABLISHED   1


#endif /* OPM_TYPES_H */
