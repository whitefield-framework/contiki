#ifndef	_COMMON_HDR_H_
#define	_COMMON_HDR_H_

#include <stdint.h>
#include <sys/time.h>
#include <uip.h>
typedef	struct _dpkt_
{
  uint32_t seq;
  struct timeval sendTime;
  uint8_t buflen;
  uint8_t buf[1];
}dpkt_t;

typedef struct _dpkt_stat_
{
   uip_ipaddr_t ip;
   uint32_t lastseq;
   uint32_t dropcnt;
   uint32_t unordered;
   uint32_t rcvcnt;
   uint32_t dupcnt;
   long leastLatency;
   long maxLatency;
}dpkt_stat_t;

#endif //	_COMMON_HDR_H_
