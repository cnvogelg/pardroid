CC=vc +aos68km
AS=vasmm68k_mot
CRUNCHER=shrinkler >/dev/null -h -9
LHA=lha a -0

VBCC_TARGET_AMIGAOS ?= $(VBCC)/targets/m68k-amigaos/
VBCC_INC = $(VBCC_TARGET_AMIGAOS)/include
VBCC_LIB = $(VBCC_TARGET_AMIGAOS)/lib

NDK_DIR ?= $(HOME)/projects/amidev/ndk_3.9
NDK_INC = $(NDK_DIR)/include/include_h
NDK_LIB = $(NDK_DIR)/include/linker_libs
NDK_INC_ASM = $(NDK_DIR)/include/include_i

CFLAGS_ARCH = -c99 -cpu=$(CONFIG_MCU) -sc -I$(VBCC_INC) -I$(NDK_INC)
LDFLAGS_ARCH = -cpu=$(CONFIG_MCU) -sc -L$(VBCC_LIB) -L$(NDK_LIB) -lamiga
ASFLAGS_ARCH = -Fhunk -quiet -phxass -m$(CONFIG_MCU) -I$(NDK_INC_ASM)

PRG_SRCS = dosmain.c
