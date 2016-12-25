CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm
SIZE = avr-size

CFLAGS_ARCH = -mmcu=$(CONFIG_MCU) -DF_CPU=$(CONFIG_MCU_FREQ)
