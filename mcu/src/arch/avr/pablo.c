#include <util/crc16.h>
#include <avr/pgmspace.h>

#include "autoconf.h"
#include "pablo.h"

uint16_t pablo_get_mach_tag(void)
{
  uint16_t addr = CONFIG_MAX_ROM - 4;
  return pgm_read_word(addr);
}

uint16_t pablo_get_rom_version(void)
{
  uint16_t addr = CONFIG_MAX_ROM - 6;
  return pgm_read_word(addr);
}

uint16_t pablo_check_rom_crc(void)
{
  uint16_t crc = 0xffff;
  uint16_t addr = 0;
  while(addr < CONFIG_MAX_ROM) {
    crc = _crc_ccitt_update(crc, pgm_read_byte(addr++));
  }
  return crc;
}
