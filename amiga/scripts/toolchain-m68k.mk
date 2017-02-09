CC=vc
AS=vasmm68k_mot

VBCC_TOOLCHAIN ?= /opt/m68k-amigaos/m68k-amigaos/vbcc/
MINSTART_OBJ = $(VBCC_TOOLCHAIN)/lib/minstart.o

CFLAGS_ARCH = -c99 -cpu=$(CONFIG_MCU) -sc
LDFLAGS_ARCH = -cpu=$(CONFIG_MCU) -sc -nostdlib -lamiga
ASFLAGS_ARCH = -cpu=$(CONFIG_MCU)

LDFLAGS_PRG = $(MINSTART_OBJ)
PRG_SRCS = dosmain.c
