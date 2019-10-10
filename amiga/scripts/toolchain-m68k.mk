# common m68k toolchain

AS=vasmm68k_mot
CRUNCHER=shrinkler >/dev/null -h -9
LHA=lha a -0
SIZE=m68k-amigaos-size

NDK_DIR ?= $(HOME)/projects/amidev/ndk_3.9
NDK_INC = $(NDK_DIR)/include/include_h
NDK_LIB = $(NDK_DIR)/include/linker_libs
NDK_INC_ASM = $(NDK_DIR)/include/include_i

ASFLAGS_ARCH = -Fhunk -quiet -phxass -m$(CONFIG_MCU) -I$(NDK_INC_ASM)

PRG_SRCS = dosmain.c
DEV_SRCS = device.c unit.c devworker.c xsysbase.s

LIBS = $(LIBS_ARCH) $(LIBS_$(FLAVOR))
