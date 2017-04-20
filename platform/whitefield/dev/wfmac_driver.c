#include "net/mac/nullmac.h"
#include "net/ip/uip.h"
#include "net/ip/tcpip.h"
#include "net/packetbuf.h"
#include "net/netstack.h"
#include "lib/random.h"

#define	DEBUG 1
#define	DEBUG_PRINT 1
#include "net/net-debug.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "commline/commline.h"

extern uint16_t gNodeID;

uint16_t lladdr_to_id(uip_lladdr_t *ll)
{
	uint16_t nodeid;
	uint8_t *ptr=(uint8_t*)&nodeid;
	if(!(ll->addr[0]|ll->addr[1]|ll->addr[2]|ll->addr[3]|ll->addr[4]|ll->addr[5]|ll->addr[6]|ll->addr[7])) { 
		return 0xffff;
	}
	ptr[0] = ll->addr[7];
	ptr[1] = ll->addr[6];
	return nodeid;
}

/*---------------------------------------------------------------------------*/
static void send_packet(mac_callback_t sent, void *ptr)
{
	int ret=MAC_TX_OK;
	uint8_t buf[sizeof(msg_buf_t) + COMMLINE_MAX_BUF];
	msg_buf_t *mbuf = (msg_buf_t *)buf;

	mbuf->len = packetbuf_totlen();
	memcpy(mbuf->buf, packetbuf_hdrptr(), packetbuf_totlen());
	mbuf->src_id = gNodeID;
	mbuf->dst_id = lladdr_to_id((uip_lladdr_t*)packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
	INFO("src:%0x dst:%0x len:%d\n", mbuf->src_id, mbuf->dst_id, mbuf->len);
	if(CL_SUCCESS != cl_sendto_q(MTYPE(AIRLINE, 0), mbuf, mbuf->len + sizeof(msg_buf_t))) {
		ret=MAC_TX_ERR;
	}
	mac_call_sent_callback(sent, ptr, ret, 1);
}
/*---------------------------------------------------------------------------*/
static void packet_input(void)
{
	NETSTACK_NETWORK.input();
}
/*---------------------------------------------------------------------------*/
static int on(void)
{
	return 0;
}
/*---------------------------------------------------------------------------*/
static int off(int keep_radio_on)
{
	return 0;
}
/*---------------------------------------------------------------------------*/
static unsigned short channel_check_interval(void)
{
	return 0;
}
/*---------------------------------------------------------------------------*/
static void init(void)
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

