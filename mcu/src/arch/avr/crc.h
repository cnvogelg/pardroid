#ifndef CRC_H
#define CRC_H

#include <util/crc16.h>
#define crc_xmodem_update(crc, data) _crc_xmodem_update(crc, data)

extern uint8_t crc7(const uint8_t* data, uint8_t n);

#endif
