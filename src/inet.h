#ifndef INET_H 
#define INET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifndef AF_INET6
#define AF_INET6 10
#endif

struct opm_sockaddr {
    union {
        struct sockaddr_in sa4;
#ifdef IPV6
        struct sockaddr_in6 sa6;
#endif
    } sas;
};


#endif /* INET_H */
