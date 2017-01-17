CC=vc
AS=vasmm68k_mot

CFLAGS_ARCH = -c99 -cpu=$(CONFIG_MCU)
LDFLAGS_ARCH = -cpu=$(CONFIG_MCU) -lvc -lamiga
ASFLAGS_ARCH = -cpu=$(CONFIG_MCU)
