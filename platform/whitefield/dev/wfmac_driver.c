#include "net/mac/nullmac.h"
#include "net/ip/uip.h"
#include "net/ip/tcpip.h"
#include "net/packetbuf.h"
#include "net/netstack.h"
#include "lib/random.h"

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

mac_callback_t g_mac_sent_cb;
/*---------------------------------------------------------------------------*/
static void send_packet(mac_callback_t sent, void *ptr)
{
//	int ret=MAC_TX_OK;
	DEFINE_MBUF(mbuf);

	if(!g_mac_sent_cb && sent) {
		g_mac_sent_cb = sent;
	} else if(g_mac_sent_cb && sent != g_mac_sent_cb) {
		ERROR("****** Didnt expect different MAC SENT CB ********\n");
		/*RJ: If this condn is hit means some additional code is required
		to manage the sent/ptr values ... have to maintain a queue and
		push the sent/ptr in every unicast case, so that when ACK is 
		rcvd, the sent/ptr are appropriately retrieved from queue */
		return;
	}

	mbuf->len = packetbuf_totlen();
	memcpy(mbuf->buf, packetbuf_hdrptr(), packetbuf_totlen());
	mbuf->src_id = gNodeID;
	mbuf->dst_id = lladdr_to_id((uip_lladdr_t*)packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
	INFO("src:%0x dst:%0x len:%d\n", mbuf->src_id, mbuf->dst_id, mbuf->len);
	if(CL_SUCCESS != cl_sendto_q(MTYPE(AIRLINE, CL_MGR_ID), mbuf, mbuf->len + sizeof(msg_buf_t))) {
//		ret=MAC_TX_ERR;
	}
//	mac_call_sent_callback(sent, ptr, ret, 1);
}

int get_tx_status(uint8_t wf_status, char *statstr, size_t len)
{
	switch(wf_status) {
		case WF_STATUS_ACK_OK:
			snprintf(statstr, len, "ACK_OK");
			return MAC_TX_OK;
		case WF_STATUS_NO_ACK:
			snprintf(statstr, len, "NO_ACK");
			return MAC_TX_NOACK;
		case WF_STATUS_ERR:
			snprintf(statstr, len, "TX_ERR");
			return MAC_TX_ERR;
		default:
			snprintf(statstr, len, "TX_FATAL");
			return MAC_TX_ERR_FATAL;
	}
	return 0;
}

void mac_handle_ack(msg_buf_t *mbuf)
{
	char statstr[32];
	int status;

	if(!g_mac_sent_cb) { 
		ERROR("How can mac sent cb is not set when ACK is rcvd!\n");
		return;
	}
	status = get_tx_status(mbuf->ack.status, statstr, sizeof(statstr));
	INFO("GOT AN ACK status:%s retries:%d\n", statstr, mbuf->ack.retries);
	mac_call_sent_callback(g_mac_sent_cb, NULL, status, mbuf->ack.retries);
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

