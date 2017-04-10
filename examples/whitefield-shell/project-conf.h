#ifndef _PROJECT_CONF_H_
#define _PROJECT_CONF_H_

#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     wfmac_driver

#ifndef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nullrdc_driver
#endif /* NETSTACK_CONF_RDC */

#undef NETSTACK_CONF_RADIO
#define NETSTACK_CONF_RADIO   wfradio_driver

#ifndef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154
#endif /* NETSTACK_CONF_FRAMER */

#endif // _PROJECT_CONF_H_
