#ifndef	_COMMON_HDR_H_
#define	_COMMON_HDR_H_

#include <stdint.h>
#include <sys/time.h>
typedef	struct _dpkt_
{
	uint32_t seq;
    struct timeval sendTime;
	uint8_t buflen;
	uint8_t buf[1];
}dpkt_t;

#endif //	_COMMON_HDR_H_
