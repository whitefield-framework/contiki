#define	_COMMAND_C_

#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "net/packetbuf.h"
#include "net/netstack.h"
#include "net/rpl/rpl-private.h"
#include "net/ip/uip.h"

int cmd_rpl_stats(uint16_t id, char *buf, int buflen)
{
	return snprintf(buf, buflen, "mem_ovrflw=%d,loc_rep=%d,glo_rep=%d,"
			 "malformed_msgs=%d,resets=%d,prnt_sw=%d,"
			 "fwd_err=%d,loop_err=%d,loop_warn=%d,root_rep=%d",
			 rpl_stats.mem_overflows, rpl_stats.local_repairs, rpl_stats.global_repairs,
			 rpl_stats.malformed_msgs, rpl_stats.resets, rpl_stats.parent_switch,
			 rpl_stats.forward_errors, rpl_stats.loop_errors, rpl_stats.loop_warnings, rpl_stats.root_repairs);
}

int cmd_node_osname(uint16_t id, char *buf, int buflen)
{
	return snprintf(buf, buflen, "contiki");
}

int cmd_ipv6_stats(uint16_t id, char *buf, int buflen)
{
	return snprintf(buf, buflen, "ipv6 rcv=%d,sent=%d,fwded=%d,drop=%d,"
		"ver_len_err=%d,"
		"fragerr=%d,chkerr=%d,protoerr=%d",
		uip_stat.ip.recv, uip_stat.ip.sent, uip_stat.ip.forwarded, uip_stat.ip.drop, 
		(uip_stat.ip.vhlerr + uip_stat.ip.hblenerr + uip_stat.ip.lblenerr), 
		uip_stat.ip.fragerr, uip_stat.ip.chkerr, uip_stat.ip.protoerr);
}

int cmd_icmp_stats(uint16_t id, char *buf, int buflen)
{
	return snprintf(buf, buflen, "icmpv6 rcv=%d,sent=%d,drop=%d,typeerr=%d,chkerr=%d",
		uip_stat.icmp.recv, uip_stat.icmp.sent, uip_stat.icmp.drop, 
		uip_stat.icmp.typeerr, uip_stat.icmp.chkerr);
}

int cmd_udp_stats(uint16_t id, char *buf, int buflen)
{
#if UIP_CONF_UDP
	return snprintf(buf, buflen, "udp rcv=%d,sent=%d,drop=%d,chkerr=%d",
		uip_stat.udp.recv, uip_stat.udp.sent, uip_stat.udp.drop, uip_stat.udp.chkerr);
#else
	return snprintf(buf, buflen, "UDP_NOT_ENABLED");
#endif
}

int cmd_tcp_stats(uint16_t id, char *buf, int buflen)
{
#if UIP_CONF_TCP
	return snprintf(buf, buflen, "tcp rcv=%d,sent=%d,drop=%d,chkerr=%d,"
		"ackerr=%d,rst=%d,rexmit=%d,syndrop=%d,synrst=%d",
		uip_stat.tcp.recv, uip_stat.tcp.sent, uip_stat.tcp.drop, uip_stat.tcp.chkerr,
		uip_stat.tcp.ackerr, uip_stat.tcp.rst, uip_stat.tcp.rexmit, 
		uip_stat.tcp.syndrop, uip_stat.tcp.synrst);
#else
	return snprintf(buf, buflen, "TCP_NOT_ENABLED");
#endif
}

int cmd_nd6_stats(uint16_t id, char *buf, int buflen)
{
	return snprintf(buf, buflen, "nd6 rcv=%d,sent=%d,drop=%d",
		uip_stat.nd6.recv, uip_stat.nd6.sent, uip_stat.nd6.drop);
}

#include "stackline/sl_command.c"

