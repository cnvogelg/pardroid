#ifndef CRC_H
#define CRC_H

extern uint16_t crc_xmodem_update(uint16_t crc, uint8_t data);
extern uint8_t crc7(const uint8_t* data, uint8_t n);

#endif
