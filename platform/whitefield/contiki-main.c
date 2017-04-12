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
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

#include "contiki.h"
#include "net/netstack.h"
#include "net/ip/uip.h"

#if NETSTACK_CONF_WITH_IPV6
#include "net/ipv6/uip-ds6.h"
#endif /* NETSTACK_CONF_WITH_IPV6 */

#include "net/rime/rime.h"

static uint8_t serial_id[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
#if !NETSTACK_CONF_WITH_IPV6
static uint16_t node_id = 0x0102;
#endif /* !NETSTACK_CONF_WITH_IPV6 */
/*---------------------------------------------------------------------------*/
static void
set_rime_addr(void)
{
  linkaddr_t addr;
  int i;

  memset(&addr, 0, sizeof(linkaddr_t));
#if NETSTACK_CONF_WITH_IPV6
  memcpy(addr.u8, serial_id, sizeof(addr.u8));
#else
  if(node_id == 0) {
    for(i = 0; i < sizeof(linkaddr_t); ++i) {
      addr.u8[i] = serial_id[7 - i];
    }
  } else {
    addr.u8[0] = node_id & 0xff;
    addr.u8[1] = node_id >> 8;
  }
#endif
  linkaddr_set_node_addr(&addr);
  printf("Rime started with address ");
  for(i = 0; i < sizeof(addr.u8) - 1; i++) {
    printf("%d.", addr.u8[i]);
  }
  printf("%d\n", addr.u8[i]);
}


/*---------------------------------------------------------------------------*/
int
main(int argc, char **argv)
{
#if NETSTACK_CONF_WITH_IPV6
#if UIP_CONF_IPV6_RPL
  printf(CONTIKI_VERSION_STRING " started with IPV6, RPL\n");
#else
  printf(CONTIKI_VERSION_STRING " started with IPV6\n");
#endif
#else
  printf(CONTIKI_VERSION_STRING " started\n");
#endif

  process_init();
  process_start(&etimer_process, NULL);
  ctimer_init();
  rtimer_init();

  set_rime_addr();

  netstack_init();
  printf("MAC %s RDC %s NETWORK %s\n", NETSTACK_MAC.name, NETSTACK_RDC.name, NETSTACK_NETWORK.name);

#if NETSTACK_CONF_WITH_IPV6
  queuebuf_init();

  memcpy(&uip_lladdr.addr, serial_id, sizeof(uip_lladdr.addr));

  process_start(&tcpip_process, NULL);
  printf("Tentative link-local IPv6 address ");
  {
    uip_ds6_addr_t *lladdr;
    int i;
    lladdr = uip_ds6_get_link_local(-1);
    for(i = 0; i < 7; ++i) {
      printf("%02x%02x:", lladdr->ipaddr.u8[i * 2],
             lladdr->ipaddr.u8[i * 2 + 1]);
    }
    /* make it hardcoded... */
    lladdr->state = ADDR_AUTOCONF;

    printf("%02x%02x\n", lladdr->ipaddr.u8[14], lladdr->ipaddr.u8[15]);
  }
#elif NETSTACK_CONF_WITH_IPV4
  process_start(&tcpip_process, NULL);
#endif

  autostart_start(autostart_processes);

  while(1) {
    int retval, usec;
    retval = process_run();
    usec = retval ? 1 : 1000;
	usleep(usec);
    etimer_request_poll();
  }

  return 0;
}

