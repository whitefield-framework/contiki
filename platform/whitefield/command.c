#define	_COMMAND_C_

#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "net/packetbuf.h"
#include "net/netstack.h"
#include "net/rpl/rpl-private.h"

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

#include "stackline/sl_command.c"

