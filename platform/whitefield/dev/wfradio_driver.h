#ifndef _WFRADIO_DRIVER_H_
#define _WFRADIO_DRIVER_H_

#include "contiki.h"
#include "dev/radio.h"

PROCESS_NAME(wfradio_process);
extern const struct radio_driver wfradio_driver;

/**
 * Set radio channel.
 */
void
radio_set_channel(int channel);

/**
 * Set transmission power of transceiver.
 *
 * \param p The power of the transceiver, between 1 and 100.
 */
void
radio_set_txpower(unsigned char p);

/**
 * The signal strength of the last received packet
 */
int
radio_signal_strength_last(void);

/**
 * This current signal strength.
 */
int
radio_signal_strength_current(void);

/**
 * Link quality indicator of last received packet.
 */
int
radio_LQI(void);

#endif // _WFRADIO_DRIVER_H_
