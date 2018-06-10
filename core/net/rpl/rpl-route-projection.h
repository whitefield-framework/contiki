/**
 * \file
 *         RPL Route Projection
 *
 * \author Rahul Jadhav <nyrahul@gmail.com>
 */


#ifndef RPL_ROUTE_PROJECT_H_
#define RPL_ROUTE_PROJECT_H_

#include "rpl-conf.h"

int project_dao(uip_ipaddr_t *dstip, uip_ipaddr_t *tgtip, 
        uip_ipaddr_t *via_arr, int via_cnt, uint8_t lifetime);

#endif /* RPL_ROUTE_PROJECT_H_ */
