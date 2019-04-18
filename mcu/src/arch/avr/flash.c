#include <avr/boot.h>
#include <avr/pgmspace.h>

#include "autoconf.h"
#include "types.h"

#if (SPM_PAGESIZE <= 255)
typedef uint16_t pagesize_t;
#else
typedef uint8_t pagesize_t;
#endif

void flash_program_page(u32 addr, const u08 *data)
{
  u16 address = (u16)addr;
  eeprom_busy_wait();
  boot_page_erase(address);
  boot_spm_busy_wait ();
  const uint8_t *buf = data;
  for (pagesize_t i=0; i<SPM_PAGESIZE; i+=2) {
    uint16_t w = *buf++;
    w += (*buf++) << 8;
    boot_page_fill(address + i, w);
  }
  boot_page_write(address);
  boot_spm_busy_wait();
  boot_rww_enable();
}

void flash_read_page(u32 addr, u08 *data)
{
  u16 address = (u16)addr;
  for (pagesize_t i=0; i<SPM_PAGESIZE; i++) {
    *data++ = pgm_read_byte(address++);
  }
}
