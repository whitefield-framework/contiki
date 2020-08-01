/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"
#include "net/rime/rimeaddr.h"

#include "net/netstack.h"
#include "dev/button-sensor.h"
#include "dev/serial-line.h"
//#if CONTIKI_TARGET_Z1
//#include "dev/uart0.h"
//#else
//#include "dev/uart1.h"
//#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "collect-common.h"
#include "collect-view.h"
#include "common-hdr.h"
#include "udp-app.h"


#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define UDP_EXAMPLE_ID  190

static struct uip_udp_conn *server_conn;

PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process,&collect_common_process);

#define	MAX_NODES	1000
	
	#if 0
	typedef struct _dpkt_stat_
	{
		uip_ipaddr_t ip;
		uint32_t lastseq;
		uint32_t dropcnt;
		uint32_t unordered;
		uint32_t rcvcnt;
	        uint32_t dupcnt;
	        long leastLatency;
	}dpkt_stat_t;
	#endif
	
	dpkt_stat_t g_dstats[MAX_NODES];
	uint32_t g_ds_cnt;

	
	dpkt_stat_t *get_dpkt_stat(uip_ipaddr_t *srcip)
	{
		int i;
		dpkt_stat_t *ds;
	
		for(i=0;i<g_ds_cnt;i++) {
			ds = &(g_dstats[i]);
			if(uip_ipaddr_cmp(srcip, &ds->ip)) {
				return ds;
			}
		}
		return NULL;
	}
	
	long dpkt_latency_time(struct timeval *tv)
	
	{
	  long duration=0;
	  long sentTime=0;
	  long recvTime=0;
	  struct timeval curTime;
	  gettimeofday(&curTime, NULL);
	
	
	  sentTime = tv->tv_sec * 1000000;
	  sentTime += tv->tv_usec;
	
	  recvTime = curTime.tv_sec * 1000000;
	  recvTime += curTime.tv_usec;
	
	  if (recvTime > sentTime){
	    duration = recvTime-sentTime;
	  }
	
	#if 0
	  if (curTime.tv_sec > tv->tv_sec){
	    duration = (curTime.tv_sec - tv->tv_sec) * 1000000;
	  }
	
	  if (curTime.tv_usec > tv->tv_usec){
	    duration += (curTime.tv_usec - tv->tv_usec); 
	  }
	#endif
	
	  return duration;
	}
/*---------------------------------------------------------------------------*/
void
collect_common_set_sink(void)
{
}
/*---------------------------------------------------------------------------*/
void
collect_common_net_print(void)
{
  printf("I am sink!\n");
}
/*---------------------------------------------------------------------------*/
void
collect_common_send(void)
{
  /* Server never sends */
}
/*---------------------------------------------------------------------------*/
void
collect_common_net_init(void)
{
/*#if CONTIKI_TARGET_Z1
  uart0_set_input(serial_line_input_byte);
#else
  uart1_set_input(serial_line_input_byte);
#endif */
  serial_line_init();

  PRINTF("I am sink!\n");
}
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  dpkt_t *pkt;
  dpkt_stat_t *ds;
  long curpktlatency;

  if(!uip_newdata()) {
    return;
  }

  pkt = (dpkt_t *)uip_appdata;
  ds = get_dpkt_stat(&(UIP_IP_BUF->srcipaddr));
  if(!ds) {
    if(g_ds_cnt>=MAX_NODES) {
      printf("dstats exceeded!\n");
      return;
    }

    ds = &g_dstats[g_ds_cnt++];
    memset(ds, 0, sizeof(dpkt_stat_t));
    ds->ip = UIP_IP_BUF->srcipaddr;
  }

  if (!ds->lastseq){
    ds->lastseq = pkt->seq;
    ds->rcvcnt++;
    ds->leastLatency = ds->maxLatency = curpktlatency = dpkt_latency_time(&(pkt->sendTime));
    goto SEND_REPLY;
  }

  if (pkt->seq == ds->lastseq){
    ds->dupcnt++;
  }
  else if(pkt->seq < ds->lastseq) {
    ds->unordered++;
    ds->rcvcnt++;
  }
  else{
    ds->lastseq = pkt->seq;
    ds->rcvcnt++;
  }
  
  curpktlatency = dpkt_latency_time(&(pkt->sendTime));
  if (curpktlatency < ds->leastLatency){
    ds->leastLatency = curpktlatency;
  }
  
  if (curpktlatency > ds->maxLatency){
    ds->maxLatency = curpktlatency;
  }
  
  SEND_REPLY:
  PRINTF("DATA Received from [%d] with seq[%d] in duration[%ld mus] min duration[%ld mus] pkt drop[%u]\n",
         ds->ip.u8[sizeof(ds->ip.u8) - 1], pkt->seq, curpktlatency, ds->leastLatency, (ds->lastseq - ds->rcvcnt));

#if SERVER_REPLY
  PRINTF("DATA sending reply\n");
  uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
  //uip_udp_packet_send(server_conn, "Reply", sizeof("Reply"));
  uip_udp_packet_send(server_conn, (unsigned char *)pkt, sizeof(dpkt_t));
  uip_create_unspecified(&server_conn->ripaddr);
#endif
}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Server IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(state == ADDR_TENTATIVE || state == ADDR_PREFERRED) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
        uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  uip_ipaddr_t ipaddr;
  struct uip_ds6_addr *root_if;

  PROCESS_BEGIN();

  PROCESS_PAUSE();

  SENSORS_ACTIVATE(button_sensor);

  PRINTF("UDP server started\n");

#if UIP_CONF_ROUTER
/* The choice of server address determines its 6LoWPAN header compression.
 * Obviously the choice made here must also be selected in udp-client.c.
 *
 * For correct Wireshark decoding using a sniffer, add the /64 prefix to the
 * 6LowPAN protocol preferences,
 * e.g. set Context 0 to fd00::. At present Wireshark copies Context/128 and
 * then overwrites it.
 * (Setting Context 0 to fd00::1111:2222:3333:4444 will report a 16 bit
 * compressed address of fd00::1111:22ff:fe33:xxxx)
 * Note Wireshark's IPCMV6 checksum verification depends on the correct
 * uncompressed addresses.
 */
 
#if 0
/* Mode 1 - 64 bits inline */
   uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 1);
#elif 1
/* Mode 2 - 16 bits inline */
  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
#else
/* Mode 3 - derived from link local (MAC) address */
  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
#endif

  uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
  root_if = uip_ds6_addr_lookup(&ipaddr);
  if(root_if != NULL) {
    rpl_dag_t *dag;
    dag = rpl_set_root(RPL_DEFAULT_INSTANCE,(uip_ip6addr_t *)&ipaddr);
    uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &ipaddr, 64);
    PRINTF("created a new RPL dag\n");
  } else {
    PRINTF("failed to create a new RPL DAG\n");
  }
#endif /* UIP_CONF_ROUTER */

  print_local_addresses();

  /* The data sink runs with a 100% duty cycle in order to ensure high
     packet reception rates. */
  NETSTACK_RDC.off(1);

  server_conn = udp_new(NULL, UIP_HTONS(UDP_CLIENT_PORT), NULL);
if(server_conn == NULL) {
	    PRINTF("No UDP connection available, exiting the process!\n");
	    PROCESS_EXIT();
	  }
  udp_bind(server_conn, UIP_HTONS(UDP_SERVER_PORT));

  PRINTF("Created a server connection with remote address ");
  PRINT6ADDR(&server_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n", UIP_HTONS(server_conn->lport),
         UIP_HTONS(server_conn->rport));

  while(1) {
    PROCESS_YIELD();

    if(ev == tcpip_event) {
      tcpip_handler();
    } else if (ev == sensors_event && data == &button_sensor) {
      PRINTF("Initiating global repair\n");
      rpl_repair_root(RPL_DEFAULT_INSTANCE);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

	void start_udp_process()
	{
	   return;
	}
	/*
	uint32_t lastseq;
	        uint32_t dropcnt;
	        uint32_t unordered;
	        uint32_t rcvcnt;
	        uint32_t dupcnt;
	*/
	void udp_get_app_stat(udpapp_stat_t *appstat)
	{
	 unsigned int s = 0;
	 unsigned int r = 0;
	 unsigned int d = 0;
	 unsigned int i = 0;
	
	 PRINTF("Stats Called on BR\n");
	 dpkt_stat_t *ds;
	 for(i=0;i<g_ds_cnt;i++) {
	  ds = &(g_dstats[i]);
	   if (!ds->rcvcnt){
	     continue;
	   }  
	
	   s += ds->lastseq;
	   r += ds->rcvcnt;
	   d += ds->dupcnt;
	 }
	 
	 appstat->totalpktsent = s;
	 appstat->totalpktrecvd = r;
	 appstat->totalduppkt = d;
	}
