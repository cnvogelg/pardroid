#ifndef MACHTAG_H
#define MACHTAG_H

#define MT_ARCH_BUILD(a)        (a << 12)
#define MT_MCU_BUILD(m)         (m << 8)
#define MT_MACH_BUILD(m)        (m << 4)
#define MT_EXTRA_BUILD(m)       (m)
#define MT_BUILD(a,b,c)         (a | b | c)
#define MT_BUILDE(a,b,c,d)      (a | b | c | d)

#define MT_ARCH_MASK            0xf000
#define MT_MCU_MASK             0x0f00
#define MT_MACH_MASK            0x00f0
#define MT_EXTRA_MASK           0x000f

// archs
#define MT_ARCH_AVR             MT_ARCH_BUILD(1)
#define MT_ARCH_MK20            MT_ARCH_BUILD(2)

// mcus
// avr
#define MT_MCU_ATMEGA328        MT_MCU_BUILD(1)
#define MT_MCU_ATMEGA32         MT_MCU_BUILD(2)
#define MT_MCU_ATMEGA32U4       MT_MCU_BUILD(3)
// mk20
#define MT_MCU_MK20DX256        MT_MCU_BUILD(1)

// machines
// avr
#define MT_MACH_ARDUNANO        MT_MACH_BUILD(1)
#define MT_MACH_AVRNETIO        MT_MACH_BUILD(2)
#define MT_MACH_TEENSY20        MT_MACH_BUILD(3)
// mk20
#define MT_MACH_TEENSY32        MT_MACH_BUILD(1)

// tags
// avr
#define MACHTAG_AVR_ATMEGA328_ARDUNANO     MT_BUILD(MT_ARCH_AVR, MT_MCU_ATMEGA328, MT_MACH_ARDUNANO)
#define MACHTAG_AVR_ATMEGA328_ARDUNANO_1   MT_BUILDE(MT_ARCH_AVR, MT_MCU_ATMEGA328, MT_MACH_ARDUNANO, 1)
#define MACHTAG_AVR_ATMEGA32_AVRNETIO      MT_BUILD(MT_ARCH_AVR, MT_MCU_ATMEGA32, MT_MACH_AVRNETIO)
#define MACHTAG_AVR_ATMEGA32U4_TEENSY20    MT_BUILD(MT_ARCH_AVR, MT_MCU_ATMEGA32U4, MT_MACH_TEENSY20)
// mk20
#define MACHTAG_MK20_MK20DX256_TEENSY32    MT_BUILD(MT_ARCH_MK20, MT_MCU_MK20DX256, MT_MACH_TEENSY32)

/* functions */
extern void machtag_decode(u16 mt, rom_pchar *arch, rom_pchar *mcu, rom_pchar *mach, u08 *extra);

#endif
