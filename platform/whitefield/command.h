#ifndef	_COMMAND_H_
#define	_COMMAND_H_

#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "net/packetbuf.h"
#include "net/netstack.h"
#include "net/rpl/rpl-private.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6-route.h"

#define INFO    printf
#define ERROR   printf

#define ADD2BUF(FP, ...) \
{\
    FILE *mylocfp = (FP);\
    if(mylocfp) {\
        fprintf(mylocfp, __VA_ARGS__);\
    } else {\
        n += snprintf(buf+n, buflen-n, __VA_ARGS__); \
    }\
}

int uip_ipaddr_to_str(const uip_ipaddr_t *addr, char *buf, int buflen);

int cmd_def_route(uint16_t id, char *buf, int buflen);

int cmd_rtsize(uint16_t id, char *buf, int buflen);

int get_route_list(FILE *fp, char *buf, int buflen);

#endif //_COMMAND_H_
