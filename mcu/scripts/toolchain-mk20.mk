COMPILER_PREFIX = arm-none-eabi-
CC = $(COMPILER_PREFIX)gcc
OBJCOPY = $(COMPILER_PREFIX)objcopy
OBJDUMP = $(COMPILER_PREFIX)objdump
SIZE = $(COMPILER_PREFIX)size
NM = $(COMPILER_PREFIX)nm

UPPER_MCU := $(shell echo $(CONFIG_MCU) | tr a-z A-Z)
MCU_LD := src/$(ARCH_DIR)/bsp/$(CONFIG_MCU).ld

CFLAGS_ARCH := -mcpu=cortex-m4 -mthumb
CFLAGS_ARCH += -D__$(UPPER_MCU)__ -DF_CPU=$(CONFIG_MCU_FREQ)
CFLAGS_ARCH += -Os

LDFLAGS_ARCH := -Os -Wl,--defsym=__rtc_localtime=0
LDFLAGS_ARCH += --specs=nano.specs -mcpu=cortex-m4 -mthumb -T$(MCU_LD)

BASE_ARCH_SRCS = mk20dx128.c eeprom.c led.c
PROTO_ATOM_ARCH_SRCS = proto_low.c
