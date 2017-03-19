CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm

HOST_CC = gcc

CFLAGS_ARCH := -mmcu=$(CONFIG_MCU) -DF_CPU=$(CONFIG_MCU_FREQ)
CFLAGS_ARCH += -Os
CFLAGS_ARCH += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS_ARCH += -fno-split-wide-types -mrelax

ASFLAGS_ARCH := -mmcu=$(CONFIG_MCU) -DF_CPU=$(CONFIG_MCU_FREQ)
ASFLAGS_ARCH += -Wa,-gstabs -x assembler-with-cpp

LDFLAGS_BOOTLOADER := -Wl,--section-start=.text=$(CONFIG_BOOTLOADER_ADDR)
LDFLAGS_BOOTLOADER += -Wl,--relax -nostartfiles -nostdlib
