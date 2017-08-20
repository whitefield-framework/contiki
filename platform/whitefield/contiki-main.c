/**
 * \ingroup platform
 *
 * \defgroup whitefield_platform Whitefield platform
 *
 * Platform running in the Whitefield environment.
 *
 * Used mainly for Whitefield based simulatiom purpose.
 * @{
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/select.h>
#include <errno.h>
#include "contiki.h"

#include "dev/button-sensor.h"
#include "dev/pir-sensor.h"
#include "dev/vib-sensor.h"

#include "net/netstack.h"
#include "net/ip/uip.h"
#include "lib/random.h"
#include "commline/commline.h"
#include "dev/wfradio_driver.h"

#if NETSTACK_CONF_WITH_IPV6
#include "net/ipv6/uip-ds6.h"
#endif /* NETSTACK_CONF_WITH_IPV6 */

SENSORS(&pir_sensor, &vib_sensor, &button_sensor);
static uint8_t serial_id[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
uint16_t gNodeID=0;
#if !NETSTACK_CONF_WITH_IPV6
#error "Supports only IPV6 based stackline..."
#endif /* !NETSTACK_CONF_WITH_IPV6 */
extern void queuebuf_init(void);

void sig_handler(int signum)
{
	printf("Sayonara... Shot with signal:%d\n", signum);
	exit(0);
}

/*---------------------------------------------------------------------------*/
void id2addr8B(const uint16_t id, uint8_t *addr)
{
	char *ptr=(char *)&id;

	memcpy(addr, serial_id, 8);
	addr[6] = ptr[1];
	addr[7] = ptr[0];
}

void print_loc_addr(void)
{
	uip_ds6_addr_t *lladdr;
	int i;
	printf("Tentative link-local IPv6 address ");
	lladdr = uip_ds6_get_link_local(-1);
	for(i = 0; i < 7; ++i) {
		printf("%02x%02x:", lladdr->ipaddr.u8[i * 2],
				lladdr->ipaddr.u8[i * 2 + 1]);
	}
	/* make it hardcoded... */
	lladdr->state = ADDR_AUTOCONF;

	printf("%02x%02x\n", lladdr->ipaddr.u8[14], lladdr->ipaddr.u8[15]);
}

pthread_mutex_t gMutex=PTHREAD_MUTEX_INITIALIZER;

void LOCK(void)
{
	pthread_mutex_lock(&gMutex);
}

void UNLOCK(void)
{
	pthread_mutex_unlock(&gMutex);
}

/*---------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
	int retval, usec;
#if NETSTACK_CONF_WITH_IPV6
#if UIP_CONF_IPV6_RPL
	printf(CONTIKI_VERSION_STRING " started with IPV6, RPL\n");
#else
	printf(CONTIKI_VERSION_STRING " started with IPV6\n");
#endif
#else
	printf(CONTIKI_VERSION_STRING " started\n");
#endif
	signal(SIGINT, sig_handler);
	signal(SIGKILL, sig_handler);
	signal(SIGTERM, sig_handler);

	if(argc < 2) {
		ERROR("Incomplete argv set\n");
		return 1;
	}
	gNodeID=atoi(argv[1]);

	random_init(gNodeID);
	process_init();
	process_start(&etimer_process, NULL);
	ctimer_init();
	rtimer_init();
	if(cl_init(CL_ATTACHQ)!=CL_SUCCESS) {
		ERROR("commline init failed\n");
		return 1;
	}

	INFO("Using node id=%d\n", gNodeID);
	id2addr8B(gNodeID, uip_lladdr.addr);

	netstack_init();
	INFO("MAC %s RDC %s NETWORK %s\n", NETSTACK_MAC.name, NETSTACK_RDC.name, NETSTACK_NETWORK.name);
	INFO("route_table_max_sz=%d,nbr_table_max_sz=%d\n", UIP_DS6_ROUTE_NB, NBR_TABLE_MAX_NEIGHBORS);

	queuebuf_init();
//	memcpy(&uip_lladdr.addr, serial_id, sizeof(uip_lladdr.addr));

	process_start(&tcpip_process, NULL);
	process_start(&wfradio_process, NULL);

	print_loc_addr();

	autostart_start(autostart_processes);

	while(1) {
		retval = process_run();
		usec = retval ? 1 : 1000;
		usleep(usec);
		LOCK();
		etimer_request_poll();
		process_poll(&wfradio_process);
		UNLOCK();
	}

	return 0;
}

/*---------------------------------------------------------------------------*/
void
log_message(char *m1, char *m2)
{
  fprintf(stdout, "%s%s\n", m1, m2);
}
/*---------------------------------------------------------------------------*/
void
uip_log(char *m)
{
  fprintf(stdout, "%s\n", m);
}
/*---------------------------------------------------------------------------*/
