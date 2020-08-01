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
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "lib/random.h"
#include "sys/ctimer.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"
#include "net/rpl/rpl.h"
#include "sys/ctimer.h"
#ifdef WITH_COMPOWER
#include "powertrace.h"
#endif
#include <stdio.h>
#include <string.h>
#include "common-hdr.h"
#include "dev/serial-line.h"
#include "net/rime/rimeaddr.h"
#include "common-hdr.h"
#include "udp-app.h"

#ifdef BUTTON_INTERFERENCE
#include "dev/button-sensor.h"
#endif
//#ifdef USE_LEDS_DEF_PARENT
#include "dev/leds.h"
//#endif


#if CONTIKI_TARGET_Z1
#include "dev/uart0.h"
#else
#include "dev/uart1.h"
#endif
#include "collect-common.h"
#include "collect-view.h"

#include <stdio.h>
#include <string.h>

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#include "net/ipv6/uip-ds6-route.h"

#define UDP_EXAMPLE_ID  190

#ifndef PERIOD
#define PERIOD 300
#endif

#define START_INTERVAL		(15 * CLOCK_SECOND)
#define SEND_INTERVAL		(PERIOD * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))
#define MAX_PAYLOAD_LEN		1024
#if 0
typedef struct _app_stat_
{
  uint32_t lastseq;
  uint32_t dropcnt;
  uint32_t unordered;
  uint32_t rcvcnt;
  uint32_t dupcnt;
  long leastLatency;
}app_stat_t;
	
app_stat_t g_stats;
#endif
	
dpkt_stat_t  g_pktstat;

static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process, &collect_common_process);

static int seq_id;
	static int reply;
	
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
  /* A udp client can never become sink */
}
/*---------------------------------------------------------------------------*/

/* Information printed with "net" in serial port interface */
void
collect_common_net_print(void)
{
  rpl_dag_t *dag;
  uip_ds6_route_t *r;

  /* Let's suppose we have only one instance */
  dag = rpl_get_any_dag();
  if(dag->preferred_parent != NULL) {
    PRINTF("Preferred parent: ");
    PRINT6ADDR(rpl_get_parent_ipaddr(dag->preferred_parent));
    PRINTF("\n");
  }
  for(r = uip_ds6_route_head();
      r != NULL;
      r = uip_ds6_route_next(r)) {
    PRINT6ADDR(&r->ipaddr);
    PRINTF("\n");
  }
  PRINTF("---\n");

}
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  dpkt_t *pkt;
  long curpktlatency;

  if(uip_newdata()) {
    pkt = (dpkt_t *)uip_appdata;
	    
	    if (!g_pktstat.lastseq){
	      g_pktstat.lastseq = pkt->seq;
	      g_pktstat.rcvcnt++;
	      g_pktstat.leastLatency = g_pktstat.maxLatency = curpktlatency = dpkt_latency_time(&(pkt->sendTime));
	      return;
	    }
	
	    PRINTF("Recvd Response with seq[%u] last rsp seq[%u]\n",pkt->seq, g_pktstat.lastseq);
	
	    if (pkt->seq == g_pktstat.lastseq){
	      g_pktstat.dupcnt++;
	    }
	    else if(pkt->seq < g_pktstat.lastseq) {
	      g_pktstat.unordered++;
	      g_pktstat.rcvcnt++;
	    }
	    else{
	      g_pktstat.lastseq = pkt->seq;
	      g_pktstat.rcvcnt++;
	    }
	
	    curpktlatency = dpkt_latency_time(&(pkt->sendTime));
	    if (curpktlatency < g_pktstat.leastLatency){
	      g_pktstat.leastLatency = curpktlatency;

  }
            if (curpktlatency > g_pktstat.maxLatency){
	    g_pktstat.maxLatency = curpktlatency;
}

reply++;
	    printf("DATA recv (s:%d, r:%d) end2nd latency[%lu] minlatency[%lu]\n", seq_id, reply, curpktlatency, g_pktstat.leastLatency);
	  }
	}
	
	uint32_t g_seq = 0;
	uint32_t g_payload_len=32;

static void
send_packet(void *ptr)
{
  char buf[MAX_PAYLOAD_LEN];
  dpkt_t *pkt=(dpkt_t*)buf;

  if(sizeof(buf) < sizeof(pkt)+g_payload_len) {
    printf("buffer size mismatch .. expect no UDP pkt\n");
    return;
  }

  seq_id++;
  pkt->seq = seq_id;
  gettimeofday(&(pkt->sendTime), NULL);

  PRINTF("DATA send to %d 'Hello %d' size-%lu\n",
         server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id, sizeof(pkt)+g_payload_len);

  uip_udp_packet_sendto(client_conn, buf, sizeof(pkt)+g_payload_len,
                        &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
}
/*---------------------------------------------------------------------------*/
void
collect_common_send(void)
{
  static uint8_t seqno;
  struct {
    uint8_t seqno;
    uint8_t for_alignment;
    struct collect_view_data_msg msg;
  } msg;
  /* struct collect_neighbor *n; */
  uint16_t parent_etx;
  uint16_t rtmetric;
  uint16_t num_neighbors;
  uint16_t beacon_interval;
  rpl_parent_t *preferred_parent;
  rimeaddr_t parent;
  rpl_dag_t *dag;

  if(client_conn == NULL) {
    /* Not setup yet */
    return;
  }
  memset(&msg, 0, sizeof(msg));
  seqno++;
  if(seqno == 0) {
    /* Wrap to 128 to identify restarts */
    seqno = 128;
  }
  msg.seqno = seqno;

  //rimeaddr_copy(&parent, &rimeaddr_null);
  parent_etx = 0;

  /* Let's suppose we have only one instance */
  dag = rpl_get_any_dag();
  if(dag != NULL) {
    preferred_parent = dag->preferred_parent;
    if(preferred_parent != NULL) {
      uip_ds6_nbr_t *nbr;
      nbr = uip_ds6_nbr_lookup(rpl_get_parent_ipaddr(preferred_parent));
      if(nbr != NULL) {
        /* Use parts of the IPv6 address as the parent address, in reversed byte order. */
        parent.u8[RIMEADDR_SIZE - 1] = nbr->ipaddr.u8[sizeof(uip_ipaddr_t) - 2];
        parent.u8[RIMEADDR_SIZE - 2] = nbr->ipaddr.u8[sizeof(uip_ipaddr_t) - 1];

	parent_etx= preferred_parent->etx;
	/*parent_etx is divided by 8 in the java graphics*/
	parent_etx *= 8;

      }
    }
	rtmetric = dag->rank;
    beacon_interval = (uint16_t) ((2L << dag->instance->dio_intcurrent) / 1000);

    num_neighbors= 0;


  } else {
    rtmetric = 0;
    beacon_interval = 0;
    num_neighbors = 0;
  }

  /* num_neighbors = collect_neighbor_list_num(&tc.neighbor_list); */
  collect_view_construct_message(&msg.msg, &parent,
                                 parent_etx, rtmetric,
                                 num_neighbors, beacon_interval);

  uip_udp_packet_sendto(client_conn, &msg, sizeof(msg),
                        &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));

#ifdef STATISTICS
  {
  static uint16_t pack_sent=0;
  printf("30 Packet sent: %u\n",++pack_sent);
  }
#endif /*STATISTICS*/

}
/*---------------------------------------------------------------------------*/
void
collect_common_net_init(void)
{
/*#if CONTIKI_TARGET_Z1
  uart0_set_input(serial_line_input_byte);
#else
  uart1_set_input(serial_line_input_byte);
#endif*/
  serial_line_init();
}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Client IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
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
static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;

  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

/* Mode 2 - 16 bits inline */
  uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);


}

	int g_send_interval=0;
	int g_auto_start = 1;
	void set_udp_param(void)
	{
	  char *ptr = getenv("UDPCLI_SEND_INT");
	  if(!ptr){
	    PRINTF("UDP_SEND_INT env var not found\n");
	    return;
	  }
	
	  g_send_interval = (int)(atof(ptr)*CLOCK_SECOND);
	
	  char *ptr1 = getenv("AUTO_START");
	  if(!ptr1){
	    PRINTF("AUTO_START env var not found\n");
	    return;
	  }
	
	  g_auto_start = (int)(atof(ptr1));
	
	  ptr = getenv("UDP_PAYLOAD_LEN");
	  if(ptr) g_payload_len = (int)atoi(ptr);
		PRINTF("UDP g_send_interval:%d g_payload_len:%d\n", 
	    g_send_interval, g_payload_len);
	  
	  memset(&g_pktstat, 0, sizeof(g_pktstat));
	}
	
	static struct etimer periodic;
	int udp_started = 0;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  
//static struct etimer periodic;
  static struct ctimer backoff_timer;
  unsigned long  backoff;
  PROCESS_BEGIN();

  PROCESS_PAUSE();


#ifdef USE_LEDS_DEF_PARENT
  SENSORS_ACTIVATE(button_sensor);
#endif 


  set_global_address();

  PRINTF("UDP client process started\n");

  print_local_addresses();

  /* new connection with remote host */
  client_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL);
  if(client_conn == NULL) {
	   PRINTF("No UDP connection available, exiting the process!\n");
	   PROCESS_EXIT();
	  }
  udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT));

  PRINTF("Created a connection with the server ");
  PRINT6ADDR(&client_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
        UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));

backoff = g_send_interval/4 + (g_send_interval / 4 * (uint32_t)random_rand()) / RANDOM_RAND_MAX;
	  PRINTF("Gnerated backoff[%lu]\n",backoff);
	
	  if(g_send_interval > 0){
	    etimer_set(&periodic, g_send_interval);
	    udp_started = 1;
	  }
	  
	  PRINTF("Will enter to while\n");



  while(1) {
    PRINTF("Will call PROCESS Yield\n");
    PROCESS_YIELD();
    PRINTF("PROCESS Yield exited\n");
    if(ev == tcpip_event) {
      tcpip_handler();
    }

PRINTF("udp_started[%d]\n", udp_started);
	    if(etimer_expired(&periodic)) {
	      etimer_reset(&periodic);
	      PRINTF("Timer Expired [%d]\n",g_auto_start);
				//send_packet(NULL);
	      if (g_auto_start){
	       //ctimer_set(&backoff_timer, SEND_TIME, send_packet, NULL);
	       backoff = g_send_interval/4 + (g_send_interval / 4 * (uint32_t)random_rand()) / RANDOM_RAND_MAX;
	       ctimer_set(&backoff_timer, backoff, send_packet, NULL);
		      }
	
	    }
	  }
	  PRINTF("Process Stopped\n");
	  PROCESS_END();
	}

	void start_udp_process()	
	{	
	  PRINTF("Need to start the UDP process\n ");	
	  if(!g_auto_start && g_send_interval > 0){	
	    udp_started = 1;	
	    g_auto_start = 1;	
	    PRINTF("Started the UDP process[%d]\n", g_send_interval);	
	  }	
	}

void udp_get_app_stat(udpapp_stat_t *appstat)
{
  PRINTF("Stats Called on Node\n");

  appstat->totalpktsent = seq_id;
  appstat->totalpktrecvd = g_pktstat.rcvcnt;
  appstat->totalduppkt = g_pktstat.dupcnt;
  appstat->minroudtriptime = g_pktstat.leastLatency;
  appstat->maxroundtriptime = g_pktstat.maxLatency; 
}

/*---------------------------------------------------------------------------*/
