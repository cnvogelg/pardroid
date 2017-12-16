#ifndef EEPROM_H
#define EEPROM_H

#include "kinetis.h"
#include <stdint.h>

extern void eeprom_initialize(void);

extern uint8_t eeprom_read_byte(const uint8_t *addr);
extern uint16_t eeprom_read_word(const uint16_t *addr);
extern uint32_t eeprom_read_dword(const uint32_t *addr);
extern void eeprom_read_block(void *buf, const void *addr, uint32_t len);

extern int eeprom_is_ready(void);

extern void eeprom_write_byte(uint8_t *addr, uint8_t value);
extern void eeprom_write_word(uint16_t *addr, uint16_t value);
extern void eeprom_write_dword(uint32_t *addr, uint32_t value);
extern void eeprom_write_block(const void *buf, void *addr, uint32_t len);

#if defined(__MK20DX128__) || defined(__MK20DX256__)
  #define E2END 0x7FF
#elif defined(__MK64FX512__) || defined(__MK66FX1M0__)
  #define E2END 0xFFF
#elif defined(__MKL26Z64__)
  #define E2END 0x7F
#else
  #define E2END 0
#endif

#endif
