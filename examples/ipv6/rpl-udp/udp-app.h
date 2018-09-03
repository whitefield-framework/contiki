#ifndef __UDP_APP_H__

#include "contiki.h"
typedef struct _udpapp_stat{
  unsigned int totalpktsent; /*Requests*/
  unsigned int totalpktrecvd; /*Request/response received*/
  unsigned int totalduppkt; /*Duplicate request/response*/
  long minroudtriptime;
  long maxroundtriptime;
  long minupwardtime;
  long maxupwardtime;
}udpapp_stat_t;

void start_udp_process();
void udp_get_app_stat(udpapp_stat_t *);

#endif /*__UDP_APP_H__*/
