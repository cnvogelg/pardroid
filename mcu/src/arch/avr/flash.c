#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <util/crc16.h>

#include "autoconf.h"

#if (SPM_PAGESIZE <= 255)
typedef uint16_t pagesize_t;
#else
typedef uint8_t pagesize_t;
#endif

void flash_program_page(uint16_t address, const uint8_t *data)
{
  eeprom_busy_wait();
  boot_page_erase(address);
  boot_spm_busy_wait ();
  const uint16_t *buf = (const uint16_t *)data;
  for (pagesize_t i=0; i<SPM_PAGESIZE; i+=2) {
    uint16_t w = *buf++;
    w += (*buf++) << 8;
    boot_page_fill(address + i, w);
  }
  boot_page_write(address);
  boot_spm_busy_wait();
  boot_rww_enable();
}

void flash_read_page(uint16_t address, uint8_t *data)
{
  for (pagesize_t i=0; i<SPM_PAGESIZE; i++) {
    *data++ = pgm_read_byte(address++);
  }
}

uint16_t flash_check_crc(void)
{
  uint16_t crc = 0xffff;
  uint16_t addr = 0;
  while(addr < CONFIG_MAX_ROM) {
    crc = _crc_ccitt_update(crc, pgm_read_byte(addr++));
  }
  return crc;
}
