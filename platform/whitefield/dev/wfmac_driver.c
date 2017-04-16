#include "net/mac/nullmac.h"
#include "net/netstack.h"
#include "net/ip/uip.h"
#include "net/ip/tcpip.h"
#include "net/packetbuf.h"
#include "net/netstack.h"

#include <stdio.h>

/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
	printf("%s called\n", __FUNCTION__);
	fflush(NULL);
	NETSTACK_RDC.send(sent, ptr);
}
/*---------------------------------------------------------------------------*/
static void
packet_input(void)
{
	printf("%s called\n", __FUNCTION__);
	fflush(NULL);
	NETSTACK_LLSEC.input();
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return NETSTACK_RDC.on();
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  return NETSTACK_RDC.off(keep_radio_on);
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
}
/*---------------------------------------------------------------------------*/
const struct mac_driver wfmac_driver = {
  "wfmac",
  init,
  send_packet,
  packet_input,
  on,
  off,
  channel_check_interval,
};
/*---------------------------------------------------------------------------*/
