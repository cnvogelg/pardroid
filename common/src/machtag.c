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
        case MT_MCU_ATMEGA32:
          *mcu = PSTR("atmega32");
          break;
        case MT_MCU_ATMEGA32U4:
          *mcu = PSTR("atmega32u4");
          break;
      }
      /* AVR MACH */
      switch(mt & MT_MACH_MASK) {
        case MT_MACH_ARDUNANO:
          *mach = PSTR("ardunano");
          break;
        case MT_MACH_AVRNETIO:
          *mach = PSTR("avrnetio");
          break;
        case MT_MACH_TEENSY20:
          *mach = PSTR("teensy20");
          break;
      }
      break;
  }
}
