#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdint.h>

#define SERIAL_MAX_PAYLOAD_SIZE 512


#ifndef SERIAL_TX_PIN
#define SERIAL_TX_PIN 11
#endif

#ifndef SERIAL_RX_PIN
#define SERIAL_RX_PIN 12
#endif

#ifndef SERIAL_CTS_PIN
#define SERIAL_CTS_PIN 26
#endif

#ifndef SERIAL_RTS_PIN
#define SERIAL_RTS_PIN 27
#endif

void serial_init();

void serial_write_data(uint8_t c);

#endif
