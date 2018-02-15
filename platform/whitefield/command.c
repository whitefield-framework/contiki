#define	_COMMAND_C_

#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "net/packetbuf.h"
#include "net/netstack.h"
#include "net/rpl/rpl-private.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6-route.h"

#define INFO    printf
#define ERROR   printf

//extern void LOCK(void);
//extern void UNLOCK(void);
#define ADD2BUF(FP, ...) \
    if(FP) {\
        fprintf(FP, __VA_ARGS__);\
    } else {\
        n += snprintf(buf+n, buflen-n, __VA_ARGS__); \
    }

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
	return uip_ipaddr_to_str(uip_ds6_defrt_choose(), buf, buflen);
}

int get_route_list(FILE *fp, char *buf, int buflen)
{
	uip_ds6_route_t *r;
	char ipstr[128], nhop[128];
	int n=0, wr_comma=0;

	for(r = uip_ds6_route_head();
			r != NULL;
			r = uip_ds6_route_next(r)) {
		uip_ipaddr_to_str(&r->ipaddr, ipstr, sizeof(ipstr));
		uip_ipaddr_to_str(uip_ds6_route_nexthop(r), nhop, sizeof(nhop));
		if(wr_comma) {
			ADD2BUF(fp, ",");
		}
        wr_comma=1;
		ADD2BUF(fp, "{ \"prefix\": \"%s\", \"pref_len\": \"%d\", \"next_hop\": \"%s\" }\n", 
			ipstr, r->length, nhop);
		if(n > buflen-100) {
			n += snprintf(buf+n, buflen-n, "[TRUNC]");
			break;
		}
	}
	return n;
}

int cmd_rtsize(uint16_t id, char *buf, int buflen)
{
	return snprintf(buf, buflen, "%d", uip_ds6_route_num_routes());
}

int cmd_route_table(uint16_t id, char *buf, int buflen)
{
	int n=0;
  FILE *fp=NULL;

  if(buf && buf[0]) {
      fp = fopen(buf, "wt");
      if(!fp) {
          ADD2BUF(fp, "cmd_route_table: COULD NOT WRITE TO FILE:<%s>\n", buf);
          ERROR("cmd_route_table: COULD NOT WRITE TO FILE:<%s>\n", buf);
          return n;
      }
  }
  INFO("cmd_route_table buf:[%s]\n",buf);
	//LOCK();
	ADD2BUF(fp, "{ \"route_table\": {\n");
	ADD2BUF(fp, "\t\"routes\": [\n");
	n += get_route_list(fp, buf+n, buflen-n);
	ADD2BUF(fp, "]\n}}");
	//UNLOCK();
  if(fp) {
    fclose(fp);
    ADD2BUF(NULL, "SUCCESS");
  }
	return n;
}

int cmd_rpl_stats(uint16_t id, char *buf, int buflen)
{
	int n=0;
	//LOCK();
	ADD2BUF(NULL, "{ \"rpl_stats\": {\n");
	ADD2BUF(NULL, "\t\"mem_ovrflw\": \"%d\",\n", rpl_stats.mem_overflows);
	ADD2BUF(NULL, "\t\"local_repairs\": \"%d\",\n", rpl_stats.local_repairs);
	ADD2BUF(NULL, "\t\"global_repairs\": \"%d\",\n", rpl_stats.global_repairs);
	ADD2BUF(NULL, "\t\"malformed_msgs\": \"%d\",\n", rpl_stats.malformed_msgs);
	ADD2BUF(NULL, "\t\"resets\": \"%d\",\n", rpl_stats.resets);
	ADD2BUF(NULL, "\t\"parent_switch\": \"%d\",\n", rpl_stats.parent_switch);
	ADD2BUF(NULL, "\t\"fwd_errors\": \"%d\",\n", rpl_stats.forward_errors);
	ADD2BUF(NULL, "\t\"loop_errors\": \"%d\",\n", rpl_stats.loop_errors);
	ADD2BUF(NULL, "\t\"loop_warns\": \"%d\",\n", rpl_stats.loop_warnings);
	ADD2BUF(NULL, "\t\"root_repairs\": \"%d\",\n", rpl_stats.root_repairs);
	ADD2BUF(NULL, "\t\"dio_sent_mcast\": \"%d\",\n", rpl_stats.dio_sent_m);
	ADD2BUF(NULL, "\t\"dio_sent_ucast\": \"%d\",\n", rpl_stats.dio_sent_u);
	ADD2BUF(NULL, "\t\"dio_rcvd\": \"%d\",\n", rpl_stats.dio_recvd);
	ADD2BUF(NULL, "\t\"dao_sent\": \"%d\",\n", rpl_stats.dao_sent);
	ADD2BUF(NULL, "\t\"dao_rcvd\": \"%d\",\n", rpl_stats.dao_recvd);
	ADD2BUF(NULL, "\t\"dao_fwded\": \"%d\",\n", rpl_stats.dao_forwarded);
	ADD2BUF(NULL, "\t\"npdao_sent\": \"%d\",\n", rpl_stats.npdao_sent);
	ADD2BUF(NULL, "\t\"npdao_rcvd\": \"%d\",\n", rpl_stats.npdao_recvd);
	ADD2BUF(NULL, "\t\"npdao_fwded\": \"%d\",\n", rpl_stats.npdao_forwarded);
	ADD2BUF(NULL, "\t\"dco_sent\": \"%d\",\n", rpl_stats.dco_sent);
	ADD2BUF(NULL, "\t\"dco_rcvd\": \"%d\",\n", rpl_stats.dco_recvd);
	ADD2BUF(NULL, "\t\"dco_fwded\": \"%d\",\n", rpl_stats.dco_forwarded);
	ADD2BUF(NULL, "\t\"dco_ign\": \"%d\"\n", rpl_stats.dco_ignored);
	ADD2BUF(NULL, "}\n}");
#if 0
	n = snprintf(buf, buflen, "mem_ovrflw=%d,loc_rep=%d,glo_rep=%d,"
			"malformed_msgs=%d,resets=%d,prnt_sw=%d,"
			"fwd_err=%d,loop_err=%d,loop_warn=%d,root_rep=%d",
			rpl_stats.mem_overflows, rpl_stats.local_repairs, rpl_stats.global_repairs,
			rpl_stats.malformed_msgs, rpl_stats.resets, rpl_stats.parent_switch,
			rpl_stats.forward_errors, rpl_stats.loop_errors, rpl_stats.loop_warnings, rpl_stats.root_repairs);
#endif
	//UNLOCK();
	return n;
}

int cmd_node_osname(uint16_t id, char *buf, int buflen)
{
	int n=0;
	ADD2BUF(NULL, "{\"os\": \"contiki\"}");
	return n;
}

int cmd_ipv6_stats(uint16_t id, char *buf, int buflen)
{
	int n=0;
	//LOCK();
	ADD2BUF(NULL, "{ \"ipv6_stats\": {\n");
	ADD2BUF(NULL, "\t\"rcvd\": \"%d\",\n", uip_stat.ip.recv);
	ADD2BUF(NULL, "\t\"sent\": \"%d\",\n", uip_stat.ip.sent);
	ADD2BUF(NULL, "\t\"fwded\": \"%d\",\n", uip_stat.ip.forwarded);
	ADD2BUF(NULL, "\t\"drop\": \"%d\",\n", uip_stat.ip.drop);
	ADD2BUF(NULL, "\t\"drop\": \"%d\",\n", uip_stat.ip.drop);
	ADD2BUF(NULL, "\t\"ver_len_err\": \"%d\",\n", 
		uip_stat.ip.vhlerr + uip_stat.ip.hblenerr + uip_stat.ip.lblenerr);
	ADD2BUF(NULL, "\t\"fragerr\": \"%d\",\n", uip_stat.ip.fragerr);
	ADD2BUF(NULL, "\t\"chkerr\": \"%d\",\n", uip_stat.ip.chkerr);
	ADD2BUF(NULL, "\t\"protoerr\": \"%d\"\n", uip_stat.ip.protoerr);
	ADD2BUF(NULL, "}\n}");
#if 0
	n = snprintf(buf, buflen, "ipv6 rcv=%d,sent=%d,fwded=%d,drop=%d,"
			"ver_len_err=%d,"
			"fragerr=%d,chkerr=%d,protoerr=%d",
			uip_stat.ip.recv, uip_stat.ip.sent, uip_stat.ip.forwarded, uip_stat.ip.drop, 
			(uip_stat.ip.vhlerr + uip_stat.ip.hblenerr + uip_stat.ip.lblenerr), 
			uip_stat.ip.fragerr, uip_stat.ip.chkerr, uip_stat.ip.protoerr);
	//UNLOCK();
#endif
	return n;
}

int cmd_icmp_stats(uint16_t id, char *buf, int buflen)
{
	int n=0;
	ADD2BUF(NULL, "{ \"icmp_stats\": {\n");
	ADD2BUF(NULL, "\t\"rcvd\": \"%d\",\n", uip_stat.icmp.recv);
	ADD2BUF(NULL, "\t\"sent\": \"%d\",\n", uip_stat.icmp.sent);
	ADD2BUF(NULL, "\t\"drop\": \"%d\",\n", uip_stat.icmp.drop);
	ADD2BUF(NULL, "\t\"typeerr\": \"%d\",\n", uip_stat.icmp.typeerr);
	ADD2BUF(NULL, "\t\"chkerr\": \"%d\"\n", uip_stat.icmp.chkerr);
	ADD2BUF(NULL, "}\n}");
#if 0
	n = snprintf(buf, buflen, "icmpv6 rcv=%d,sent=%d,drop=%d,typeerr=%d,chkerr=%d",
			uip_stat.icmp.recv, uip_stat.icmp.sent, uip_stat.icmp.drop, 
			uip_stat.icmp.typeerr, uip_stat.icmp.chkerr);
#endif
	return n;
}

int cmd_udp_stats(uint16_t id, char *buf, int buflen)
{
#if UIP_CONF_UDP
	int n=0;
	ADD2BUF(NULL, "{ \"udp_stats\": {\n");
	ADD2BUF(NULL, "\t\"rcvd\": \"%d\",\n", uip_stat.udp.recv);
	ADD2BUF(NULL, "\t\"sent\": \"%d\",\n", uip_stat.udp.sent);
	ADD2BUF(NULL, "\t\"drop\": \"%d\",\n", uip_stat.udp.drop);
	ADD2BUF(NULL, "\t\"chkerr\": \"%d\"\n", uip_stat.udp.chkerr);
	ADD2BUF(NULL, "}\n}");
	return n;
#else
	return snprintf(buf, buflen, "UDP_NOT_ENABLED");
#endif
}

int cmd_tcp_stats(uint16_t id, char *buf, int buflen)
{
#if UIP_CONF_TCP
	int n=0;
	ADD2BUF(NULL, "{ \"tcp_stats\": {\n");
	ADD2BUF(NULL, "\t\"rcvd\": \"%d\",\n", uip_stat.tcp.recv);
	ADD2BUF(NULL, "\t\"sent\": \"%d\",\n", uip_stat.tcp.sent);
	ADD2BUF(NULL, "\t\"drop\": \"%d\",\n", uip_stat.tcp.drop);
	ADD2BUF(NULL, "\t\"chkerr\": \"%d\",\n", uip_stat.tcp.chkerr);
	ADD2BUF(NULL, "\t\"ackerr\": \"%d\",\n", uip_stat.tcp.ackerr);
	ADD2BUF(NULL, "\t\"rst\": \"%d\",\n", uip_stat.tcp.rst);
	ADD2BUF(NULL, "\t\"rexmit\": \"%d\",\n", uip_stat.tcp.rexmit);
	ADD2BUF(NULL, "\t\"syndrop\": \"%d\",\n", uip_stat.tcp.syndrop);
	ADD2BUF(NULL, "\t\"synrst\": \"%d\"\n", uip_stat.tcp.synrst);
	ADD2BUF(NULL, "}\n}");
	return n;
#else
	return snprintf(buf, buflen, "TCP_NOT_ENABLED");
#endif
}

int cmd_nd6_stats(uint16_t id, char *buf, int buflen)
{
	int n=0;
	ADD2BUF(NULL, "{ \"nd6_stats\": {\n");
	ADD2BUF(NULL, "\t\"rcvd\": \"%d\",\n", uip_stat.nd6.recv);
	ADD2BUF(NULL, "\t\"sent\": \"%d\",\n", uip_stat.nd6.sent);
	ADD2BUF(NULL, "\t\"drop\": \"%d\"\n", uip_stat.nd6.drop);
	ADD2BUF(NULL, "}\n}");
	return n;
}

int cmd_config_info(uint16_t id, char *buf, int buflen)
{
	int n=0;
	ADD2BUF(NULL, "{ \"config\": {\n");
	ADD2BUF(NULL, "\t\"rttable_maxsz\": \"%d\",\n", UIP_DS6_ROUTE_NB);
	ADD2BUF(NULL, "\t\"nbrtable_maxsz\": \"%d\"\n", NBR_TABLE_MAX_NEIGHBORS);
	ADD2BUF(NULL, "}\n}");
	return n;
}

