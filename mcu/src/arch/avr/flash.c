#include <avr/boot.h>
#include <avr/pgmspace.h>

#include "autoconf.h"
#include "types.h"
#include "arch.h"

#if (SPM_PAGESIZE <= 255)
typedef uint16_t pagesize_t;
#else
typedef uint8_t pagesize_t;
#endif

void flash_program_page(flash_size_t addr, const u08 *data)
{
  eeprom_busy_wait();
  boot_page_erase(addr);
  boot_spm_busy_wait ();
  const uint8_t *buf = data;
  for (pagesize_t i=0; i<SPM_PAGESIZE; i+=2) {
    uint16_t w = *buf++;
    w += (*buf++) << 8;
    boot_page_fill(addr + i, w);
  }
  boot_page_write(addr);
  boot_spm_busy_wait();
  boot_rww_enable();
}

void flash_read_page(flash_size_t addr, u08 *data)
{
  for (pagesize_t i=0; i<SPM_PAGESIZE; i++) {
    *data++ = pgm_read_byte(addr++);
  }
}
