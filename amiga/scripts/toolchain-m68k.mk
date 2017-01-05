CC=vc
AS=vasmm68k_mot

CFLAGS_ARCH = -c99 -cpu=$(CONFIG_MCU)
LDFLAGS_ARCH = -cpu=$(CONFIG_MCU) -lvcs -lamiga
ASFLAGS_ARCH = -quiet -Fhunk -m$(CONFIG_MCU)
