/**
 * \file
 *         RPL Route Projection
 *
 * \author Rahul Jadhav <nyrahul@gmail.com>
 */

#include "net/rpl/rpl-conf.h"

#include "net/ip/uip.h"
#include "net/ip/tcpip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/rpl/rpl-private.h"
#include "net/rpl/rpl-ns.h"
#include "lib/list.h"
#include "lib/memb.h"

#if RPL_WITH_NON_STORING && RPL_ROUTE_PROJECTION

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#include <limits.h>
#include <string.h>

/* can define my own dao_sequence here, since BR only does route projection
 * and fortunately BR nevers sends DAO by itself for any other thing. */
static uint8_t dao_sequence = RPL_LOLLIPOP_INIT;

#define UIP_ICMP_PAYLOAD ((unsigned char *)&uip_buf[uip_l2_l3_icmp_hdr_len])
/*---------------------------------------------------------------------------*/
int project_dao(uip_ipaddr_t *dstip, uip_ipaddr_t *tgtip, uip_ipaddr_t *via_arr, int via_cnt, uint8_t lifetime)
{
  unsigned char *buffer;
  rpl_instance_t *instance;
  rpl_dag_t *dag;
  uint8_t prefixlen;
  int pos, i;
  
  if(!dstip || !tgtip || !via_arr || !via_cnt) {
    PRINTF("Sanity chk failed\n");
    return 1;
  }

  dag = rpl_get_any_dag();
  if(!dag) {
    PRINTF("Could not get any DAG\n");
    return 2;
  }
  instance = dag->instance;
  if(!instance) {
    PRINTF("Could not get any DAG-Instance\n");
    return 3;
  }

  buffer = UIP_ICMP_PAYLOAD;
  pos = 0;

  buffer[pos++] = instance->instance_id;
  buffer[pos] = 0;
  buffer[pos] |= RPL_DAO_K_FLAG;  //RJ: set K flag always
  ++pos;
  buffer[pos++] = 0; /* reserved */

  RPL_LOLLIPOP_INCREMENT(dao_sequence);
  buffer[pos++] = dao_sequence;

  /* create target subopt */
  prefixlen = sizeof(*tgtip) * CHAR_BIT;
  buffer[pos++] = RPL_OPTION_TARGET;
  buffer[pos++] = 2 + ((prefixlen + 7) / CHAR_BIT);
  buffer[pos++] = 0; /* reserved */
  buffer[pos++] = prefixlen;
  memcpy(buffer + pos, tgtip, (prefixlen + 7) / CHAR_BIT);
  pos += ((prefixlen + 7) / CHAR_BIT);

  buffer[pos++] = RPL_OPTION_VIA_INFORMATION;
  buffer[pos++] = (via_cnt * 16)+2; //16B per via ip + 2B (for path seq, path ltime)
  RPL_LOLLIPOP_INCREMENT(path_sequence);
  buffer[pos++] = path_sequence; /* path seq - ignored */
  buffer[pos++] = lifetime;
  for(i=0; i<via_cnt; i++) {
    memcpy(buffer + pos, &via_arr[i], 16);
    pos += 16;
  }

  PRINTF("RT_PROJECTION: Sending a %sDAO sz %d with dao_seq %u, path seq %u, lifetime %u, target ",
         lifetime == RPL_ZERO_LIFETIME ? "No-Path " : "", 
         pos, dao_sequence, path_sequence, lifetime);

  PRINT6ADDR(tgtip);
  PRINTF(" to ");
  PRINT6ADDR(dstip);
  PRINTF("\n");

  uip_icmp6_send(dstip, ICMP6_RPL, RPL_CODE_DAO, pos);

	if (0 == lifetime) {
		RPL_STAT(rpl_stats.npdao_sent++);
	} else {
		RPL_STAT(rpl_stats.dao_sent++);
	}
  return 0;
}
/*---------------------------------------------------------------------------*/

#endif /* RPL_WITH_NON_STORING && RPL_ROUTE_PROJECTION */
