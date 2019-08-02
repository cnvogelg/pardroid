CC=m68k-amigaos-gcc
CFLAGS_ARCH = -noixemul -m$(CONFIG_MCU) -Werror
LDFLAGS_ARCH = -noixemul -m$(CONFIG_MCU)
