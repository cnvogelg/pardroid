#include "types.h"
#include "arch.h"
#include "machtag.h"

void machtag_decode(u16 mt, rom_pchar *arch, rom_pchar *mcu, rom_pchar *mach, u08 *extra)
{
  *arch = PSTR("?");
  *mcu = PSTR("?");
  *mach = PSTR("?");
  *extra = (u08)(mt & MT_EXTRA_MASK);

  switch(mt & MT_ARCH_MASK) {
    case MT_ARCH_AVR:
      *arch = PSTR("avr");
      /* AVR MCU */
      switch(mt & MT_MCU_MASK) {
        case MT_MCU_ATMEGA328:
          *mcu = PSTR("atmega328");
          break;
      }
      /* AVR MACH */
      switch(mt & MT_MACH_MASK) {
        case MT_MACH_ARDUNANO:
          *mach = PSTR("ardunano");
          break;
      }
      break;
  }
}
