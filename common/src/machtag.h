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

// mcus
#define MT_MCU_ATMEGA328        MT_MCU_BUILD(1)

// machines
#define MT_MACH_ARDUNANO        MT_MACH_BUILD(1)

// tags
#define MACHTAG_AVR_ATMEGA328_ARDUNANO     MT_BUILD(MT_ARCH_AVR, MT_MCU_ATMEGA328, MT_MACH_ARDUNANO)
#define MACHTAG_AVR_ATMEGA328_ARDUNANO_1   MT_BUILDE(MT_ARCH_AVR, MT_MCU_ATMEGA328, MT_MACH_ARDUNANO, 1)

/* functions */
extern void machtag_decode(u16 mt, rom_pchar *arch, rom_pchar *mcu, rom_pchar *mach, u08 *extra);

#endif
