#define	_COMMAND_C_

#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "net/packetbuf.h"
#include "net/netstack.h"
#include "net/rpl/rpl-private.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6-route.h"

extern void LOCK(void);
extern void UNLOCK(void);

int uip_ipaddr_to_str(const uip_ipaddr_t *addr, char *buf, int buflen)
{
#if NETSTACK_CONF_WITH_IPV6
	uint16_t a;
	unsigned int i;
	int f, n =0;
#endif /* NETSTACK_CONF_WITH_IPV6 */
	if(addr == NULL) {
		return snprintf(buf, buflen, "[NONE]");
	}
#if NETSTACK_CONF_WITH_IPV6
	for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
		a = (addr->u8[i] << 8) + addr->u8[i + 1];
		if(a == 0 && f >= 0) {
			if(f++ == 0) {
				n += snprintf(buf+n, buflen-n, "::");
			}
		} else {
			if(f > 0) {
				f = -1;
			} else if(i > 0) {
				n += snprintf(buf+n, buflen-n, ":");
			}
			n += snprintf(buf+n, buflen-n,"%x", a);
		}
	}
	return n;
#else /* NETSTACK_CONF_WITH_IPV6 */
	return snprintf(buf, buflen, "%u.%u.%u.%u", 
			addr->u8[0], addr->u8[1], addr->u8[2], addr->u8[3]);
#endif /* NETSTACK_CONF_WITH_IPV6 */
}

int cmd_def_route(uint16_t id, char *buf, int buflen)
{
	int n;
	LOCK();
	n = uip_ipaddr_to_str(uip_ds6_defrt_choose(), buf, buflen);
	UNLOCK();
	return n;
}

int get_route_list(char *buf, int buflen)
{
	uip_ds6_route_t *r;
	char ipstr[128], nhop[128];
	int n=0;

	for(r = uip_ds6_route_head();
			r != NULL;
			r = uip_ds6_route_next(r)) {
		uip_ipaddr_to_str(&r->ipaddr, ipstr, sizeof(ipstr));
		uip_ipaddr_to_str(uip_ds6_route_nexthop(r), nhop, sizeof(nhop));
		n += snprintf(buf+n, buflen-n, "%s/%d -- %s\n", ipstr, r->length, nhop); 
		if(n > buflen-100) {
			n += snprintf(buf+n, buflen-n, "[TRUNC]");
			break;
		}
	}
	if(!n) {
		return snprintf(buf, buflen, "[NONE]");
	}
	return n;
}

int cmd_rtsize(uint16_t id, char *buf, int buflen)
{
	int n;
	LOCK();
	n = snprintf(buf, buflen, "%d", uip_ds6_route_num_routes());
	UNLOCK();
	return n;
}

int cmd_route_table(uint16_t id, char *buf, int buflen)
{
	int n;
	LOCK();
	n = get_route_list(buf, buflen);
	UNLOCK();
	return n;
}

int cmd_rpl_stats(uint16_t id, char *buf, int buflen)
{
	int n;
	LOCK();
	n = snprintf(buf, buflen, "mem_ovrflw=%d,loc_rep=%d,glo_rep=%d,"
			"malformed_msgs=%d,resets=%d,prnt_sw=%d,"
			"fwd_err=%d,loop_err=%d,loop_warn=%d,root_rep=%d",
			rpl_stats.mem_overflows, rpl_stats.local_repairs, rpl_stats.global_repairs,
			rpl_stats.malformed_msgs, rpl_stats.resets, rpl_stats.parent_switch,
			rpl_stats.forward_errors, rpl_stats.loop_errors, rpl_stats.loop_warnings, rpl_stats.root_repairs);
	UNLOCK();
	return n;
}

int cmd_node_osname(uint16_t id, char *buf, int buflen)
{
	return snprintf(buf, buflen, "contiki");
}

int cmd_ipv6_stats(uint16_t id, char *buf, int buflen)
{
	int n;
	LOCK();
	n = snprintf(buf, buflen, "ipv6 rcv=%d,sent=%d,fwded=%d,drop=%d,"
			"ver_len_err=%d,"
			"fragerr=%d,chkerr=%d,protoerr=%d",
			uip_stat.ip.recv, uip_stat.ip.sent, uip_stat.ip.forwarded, uip_stat.ip.drop, 
			(uip_stat.ip.vhlerr + uip_stat.ip.hblenerr + uip_stat.ip.lblenerr), 
			uip_stat.ip.fragerr, uip_stat.ip.chkerr, uip_stat.ip.protoerr);
	UNLOCK();
	return n;
}

int cmd_icmp_stats(uint16_t id, char *buf, int buflen)
{
	int n;
	LOCK();
	n = snprintf(buf, buflen, "icmpv6 rcv=%d,sent=%d,drop=%d,typeerr=%d,chkerr=%d",
			uip_stat.icmp.recv, uip_stat.icmp.sent, uip_stat.icmp.drop, 
			uip_stat.icmp.typeerr, uip_stat.icmp.chkerr);
	UNLOCK();
	return n;
}

int cmd_udp_stats(uint16_t id, char *buf, int buflen)
{
#if UIP_CONF_UDP
	int n;
	LOCK();
	n = snprintf(buf, buflen, "udp rcv=%d,sent=%d,drop=%d,chkerr=%d",
			uip_stat.udp.recv, uip_stat.udp.sent, uip_stat.udp.drop, uip_stat.udp.chkerr);
	UNLOCK();
	return n;
#else
	return snprintf(buf, buflen, "UDP_NOT_ENABLED");
#endif
}

int cmd_tcp_stats(uint16_t id, char *buf, int buflen)
{
#if UIP_CONF_TCP
	int n;
	LOCK();
	n = snprintf(buf, buflen, "tcp rcv=%d,sent=%d,drop=%d,chkerr=%d,"
			"ackerr=%d,rst=%d,rexmit=%d,syndrop=%d,synrst=%d",
			uip_stat.tcp.recv, uip_stat.tcp.sent, uip_stat.tcp.drop, uip_stat.tcp.chkerr,
			uip_stat.tcp.ackerr, uip_stat.tcp.rst, uip_stat.tcp.rexmit, 
			uip_stat.tcp.syndrop, uip_stat.tcp.synrst);
	UNLOCK();
	return n;
#else
	return snprintf(buf, buflen, "TCP_NOT_ENABLED");
#endif
}

int cmd_nd6_stats(uint16_t id, char *buf, int buflen)
{
	int n;
	LOCK();
	n = snprintf(buf, buflen, "nd6 rcv=%d,sent=%d,drop=%d",
			uip_stat.nd6.recv, uip_stat.nd6.sent, uip_stat.nd6.drop);
	UNLOCK();
	return n;
}

#include "stackline/sl_command.c"

