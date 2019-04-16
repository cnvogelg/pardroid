#include <util/crc16.h>
#include <avr/pgmspace.h>

#include "autoconf.h"
#include "types.h"
#include "pablo.h"

u16 pablo_get_mach_tag(void)
{
  u16 addr = CONFIG_MAX_ROM - 4;
  return pgm_read_word(addr);
}

u16 pablo_get_rom_version(void)
{
  u16 addr = CONFIG_MAX_ROM - 6;
  return pgm_read_word(addr);
}

u16 pablo_get_rom_fw_id(void)
{
  u16 addr = CONFIG_MAX_ROM - 8;
  return pgm_read_word(addr);
}

u16 pablo_get_rom_crc(void)
{
  u16 addr = CONFIG_MAX_ROM - 2;
  return pgm_read_word(addr);
}

u16 pablo_check_rom_crc(void)
{
  u16 crc = 0xffff;
  u16 addr = 0;
  while(addr < CONFIG_MAX_ROM) {
    crc = _crc_ccitt_update(crc, pgm_read_byte(addr++));
  }
  return crc;
}
