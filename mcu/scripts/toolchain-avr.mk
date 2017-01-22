CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm
SIZE = avr-size

CFLAGS_ARCH := -mmcu=$(CONFIG_MCU) -DF_CPU=$(CONFIG_MCU_FREQ)
CFLAGS_ARCH += -Os
CFLAGS_ARCH += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -mcall-prologues

ASFLAGS_ARCH := -mmcu=$(CONFIG_MCU) -DF_CPU=$(CONFIG_MCU_FREQ)
ASFLAGS_ARCH += -Wa,-gstabs -x assembler-with-cpp
