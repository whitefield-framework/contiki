#include <stdio.h>
#include <string.h>

#include "contiki.h"

#include "net/packetbuf.h"
#include "net/netstack.h"

#include "dev/radio.h"
#include "wfradio_driver.h"
#include "commline/commline.h"

PROCESS(wfradio_process, "whitefield radio process");
/*---------------------------------------------------------------------------*/
static void
set_send_on_cca(uint8_t enable)
{
}
/*---------------------------------------------------------------------------*/
static void
set_frame_filtering(int enable)
{
}
/*---------------------------------------------------------------------------*/
static void
set_auto_ack(int enable)
{
}
/*---------------------------------------------------------------------------*/
static void
set_poll_mode(int enable)
{
}
/*---------------------------------------------------------------------------*/
void
radio_set_channel(int channel)
{
}
/*---------------------------------------------------------------------------*/
void
radio_set_txpower(unsigned char power)
{
}
/*---------------------------------------------------------------------------*/
int
radio_signal_strength_last(void)
{
	return 1;
}
/*---------------------------------------------------------------------------*/
int
radio_signal_strength_current(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
int
radio_LQI(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
radio_on(void)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
radio_off(void)
{
  return 0;
}

extern void mac_handle_ack(msg_buf_t *mbuf);
extern void sl_handle_cmd(msg_buf_t *mbuf);
/*---------------------------------------------------------------------------*/
extern uint16_t gNodeID;
static int radio_read(void *inbuf, unsigned short bufsize)
{
	int ret;
	linkaddr_t addr;
	DEFINE_MBUF(mbuf);

	ret = cl_recvfrom_q(MTYPE(STACKLINE, gNodeID), mbuf, sizeof(mbuf_buf), CL_FLAG_NOWAIT);
	if(mbuf->len == 0) {
		return 0;
	}
	INFO("RECV ret:%d src:%x dst:%x len:%d flags:%x\n", 
		ret, mbuf->src_id, mbuf->dst_id, mbuf->len, mbuf->flags);
	if(mbuf->len > bufsize) {
		ERROR("How can mbuflen(%d) be greater than bufsize:%d?!\n", mbuf->len, bufsize);
		return 0;
	}
	if(mbuf->flags & MBUF_IS_CMD) {
#if 1
		//Hack Alert!! Notice that the foll condn will never be true
		//This condition had to be added so that cmd_* function are linked in the 
		//binary. Linker decides that cmd_* functions are never called from 
		//any other place and optimizes out! But I need these functions to be 
		//loaded dynamically using dlsym(). Using this check i sort of fool linker 
		//into believing that the function is called, but the wrapping cond will 
		//never be true.
		if(mbuf->len == 0xdead && mbuf->len == 0xc0de) {
			extern int cmd_rtsize(uint16_t, char *, int);
			cmd_rtsize(0xbabe, NULL, 0xcafe);
		}
#endif
		sl_handle_cmd(mbuf);
		return 0;
	}
	memcpy(inbuf, mbuf->buf, mbuf->len);
	cl_get_id2longaddr(mbuf->src_id, addr.u8, sizeof(addr.u8));
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &addr);
	cl_get_id2longaddr(mbuf->dst_id, addr.u8, sizeof(addr.u8));
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &addr);
	packetbuf_set_attr(PACKETBUF_ATTR_RSSI, mbuf->info.sig.rssi);
	packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, mbuf->info.sig.lqi);
	if(mbuf->flags & MBUF_IS_ACK) {
		mac_handle_ack(mbuf);
		return 0;
	}
#if 0
	static int rcvd_cnt=0;
	rcvd_cnt++;
	PRINT_HEX(mbuf->buf, mbuf->len, "rcvd %d, len=%d\n", rcvd_cnt, mbuf->len);
#endif
	return mbuf->len;
}
/*---------------------------------------------------------------------------*/
static int channel_clear(void)
{
	return 1;
}
/*---------------------------------------------------------------------------*/
static int radio_send(const void *payload, unsigned short payload_len)
{
	return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
static int prepare_packet(const void *data, unsigned short len)
{
	return len;
}
/*---------------------------------------------------------------------------*/
static int transmit_packet(unsigned short len)
{
	int ret = RADIO_TX_ERR;
	return ret;
}
/*---------------------------------------------------------------------------*/
static int receiving_packet(void)
{
	return 0;
}
/*---------------------------------------------------------------------------*/
static int pending_packet(void)
{
	return 1;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(wfradio_process, ev, data)
{
	int len;

	PROCESS_BEGIN();

	while(1) {
		PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
		packetbuf_clear();
		len = radio_read(packetbuf_dataptr(), PACKETBUF_SIZE);
		if(len > 0) {
			packetbuf_set_datalen(len);
			NETSTACK_MAC.input();
		}
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static int init(void)
{
	process_start(&wfradio_process, NULL);
	return 1;
}
/*---------------------------------------------------------------------------*/
static radio_result_t get_value(radio_param_t param, radio_value_t *value)
{
	switch(param) {
		case RADIO_PARAM_RX_MODE:
			*value = 0;
			return RADIO_RESULT_OK;
		case RADIO_PARAM_TX_MODE:
			*value = 0;
			return RADIO_RESULT_OK;
		case RADIO_PARAM_LAST_RSSI:
			return RADIO_RESULT_OK;
		case RADIO_PARAM_LAST_LINK_QUALITY:
			return RADIO_RESULT_OK;
		default:
			return RADIO_RESULT_NOT_SUPPORTED;
	}
}
/*---------------------------------------------------------------------------*/
static radio_result_t set_value(radio_param_t param, radio_value_t value)
{
	switch(param) {
		case RADIO_PARAM_RX_MODE:
			if(value & ~(RADIO_RX_MODE_ADDRESS_FILTER |
						RADIO_RX_MODE_AUTOACK | RADIO_RX_MODE_POLL_MODE)) {
				return RADIO_RESULT_INVALID_VALUE;
			}

			/* Only disabling is acceptable for RADIO_RX_MODE_ADDRESS_FILTER */
			if ((value & RADIO_RX_MODE_ADDRESS_FILTER) != 0) {
				return RADIO_RESULT_NOT_SUPPORTED;
			}
			set_frame_filtering((value & RADIO_RX_MODE_ADDRESS_FILTER) != 0);

			/* Only disabling is acceptable for RADIO_RX_MODE_AUTOACK */
			if ((value & RADIO_RX_MODE_ADDRESS_FILTER) != 0) {
				return RADIO_RESULT_NOT_SUPPORTED;
			}
			set_auto_ack((value & RADIO_RX_MODE_AUTOACK) != 0);

			set_poll_mode((value & RADIO_RX_MODE_POLL_MODE) != 0);
			return RADIO_RESULT_OK;
		case RADIO_PARAM_TX_MODE:
			if(value & ~(RADIO_TX_MODE_SEND_ON_CCA)) {
				return RADIO_RESULT_INVALID_VALUE;
			}
			set_send_on_cca((value & RADIO_TX_MODE_SEND_ON_CCA) != 0);
			return RADIO_RESULT_OK;
		case RADIO_PARAM_CHANNEL:
			if(value < 11 || value > 26) {
				return RADIO_RESULT_INVALID_VALUE;
			}
			radio_set_channel(value);
			return RADIO_RESULT_OK;
		default:
			return RADIO_RESULT_NOT_SUPPORTED;
	}
}
/*---------------------------------------------------------------------------*/
static radio_result_t get_object(radio_param_t param, void *dest, size_t size)
{
	return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static radio_result_t set_object(radio_param_t param, const void *src, size_t size)
{
	return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
const struct radio_driver wfradio_driver =
{
	init,
	prepare_packet,
	transmit_packet,
	radio_send,
	radio_read,
	channel_clear,
	receiving_packet,
	pending_packet,
	radio_on,
	radio_off,
	get_value,
	set_value,
	get_object,
	set_object
};
/*---------------------------------------------------------------------------*/
