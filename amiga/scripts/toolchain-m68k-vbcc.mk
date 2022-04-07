CC=vc +aos68km

VBCC_TARGET_AMIGAOS ?= $(VBCC)/targets/m68k-amigaos/
VBCC_INC = $(VBCC_TARGET_AMIGAOS)/include
VBCC_LIB = $(VBCC_TARGET_AMIGAOS)/lib

CFLAGS_ARCH = -c99 -cpu=$(CONFIG_MCU) -Os -+ -sc -I$(VBCC_INC) -I$(NDK_INC) -I$(NET_INC)
LDFLAGS_ARCH = -cpu=$(CONFIG_MCU) -sc -L$(VBCC_LIB) -L$(NDK_LIB)
LDFLAGS_DEV = -nostdlib -lvc
LIBS_debug = -ldebug 
LIBS_ARCH = -lamiga
