#define	_COMMAND_NS_MOP_C_

#include "command.h"

#if RPL_WITH_NON_STORING

#include "net/rpl/rpl-ns.h"

int get_route_list(FILE *fp, char *buf, int buflen)
{
  rpl_ns_node_t *r;
  char ipstr[128], parent[128];
  int n=0, wr_comma=0;
  uip_ipaddr_t ip;

  for(r = rpl_ns_node_head(); r != NULL;
      r = rpl_ns_node_next(r)) 
  {
    rpl_ns_get_node_global_addr(&ip, r);
    uip_ipaddr_to_str(&ip, ipstr, sizeof(ipstr));
    rpl_ns_get_node_global_addr(&ip, r->parent);
    uip_ipaddr_to_str(&ip, parent, sizeof(parent));
    if(wr_comma) {
      ADD2BUF(fp, ",");
    }
    wr_comma=1;
    ADD2BUF(fp, "{ \"prefix\": \"%s\", \"pref_len\": \"%d\", \"parent\": \"%s\" }\n", 
        ipstr, 128, parent);
    if(n > buflen-100) {
      n += snprintf(buf+n, buflen-n, "[TRUNC]");
      break;
    }
  }
  return n;
}

int cmd_rtsize(uint16_t id, char *buf, int buflen)
{
  return snprintf(buf, buflen, "%d", rpl_ns_num_nodes());
}

#endif
