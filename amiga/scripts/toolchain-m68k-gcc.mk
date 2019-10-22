CC=m68k-amigaos-gcc
CFLAGS_ARCH = -noixemul -m$(CONFIG_MCU) -Werror \
			  -mregparm=4 -fomit-frame-pointer -s -O3
LDFLAGS_ARCH = -noixemul -m$(CONFIG_MCU) -s
LDFLAGS_DEV = -nostdlib -nostartfiles
LIBS_ARCH = -lamiga
LIBS_debug = -ldebug -lc
