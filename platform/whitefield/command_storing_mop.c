#define	_COMMAND_SM_MOP_C_

#include "command.h"

#if !RPL_WITH_NON_STORING

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

#endif
