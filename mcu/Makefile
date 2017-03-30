BASE_DIR=..

# config setup
CONFIG?=configs/pbnano.config
FLASH_CONFIG?=configs/flash.config
EXTRA_CONFIG=$(FLASH_CONFIG) $(USER_CONFIG)
ifeq "$(DEBUG)" "1"
	dummy:=$(info debug enabled)
	EXTRA_CONFIG+=configs/debug.config
endif
ALL_CONFIGS=configs/pbnano.config configs/pbcrasbe.config configs/avrnetio.config
ALL_CONFIGS+=configs/teensy20.config

include $(BASE_DIR)/scripts/gencfg.mk
# version
include ../version.mk
# common defs
include scripts/common-defs.mk
# toolchain
include scripts/toolchain-$(CONFIG_ARCH).mk
# flash rules
include scripts/flash-$(CONFIG_FLASH_TOOL).mk

# machtag
MACHTAG=$(BASE_DIR)/scripts/machtag.py
MT_VALS:=$(shell $(MACHTAG) -vd $(CONFIG_ARCH) $(CONFIG_MCU) $(CONFIG_MACH) $(CONFIG_MACH_VARIANT))
MACHTAG_ID=$(word 1,$(MT_VALS))
MACHTAG_SYMBOL=$(word 2,$(MT_VALS))
CFLAGS_DEFINES += -DMACHTAG=$(MACHTAG_SYMBOL)

# generate version tag
VERSION_TAG=$(shell scripts/vertag.py $(VERSION_MAJOR) $(VERSION_MINOR))
CFLAGS_DEFINES += -DVERSION_TAG=$(VERSION_TAG)

# dist tag
DIST_TAG=$(CONFIG_BASE)-$(VERSION_MAJOR).$(VERSION_MINOR)

# sources
BASE_SRCS = uart.c uartutil.c system.c pablo.c
PROTO_SRCS = proto_low.S proto.c reg_ro.c

INCLUDES=src src/arch src/$(ARCH_DIR) src/$(MACH_DIR) src/base src/parbox
INCLUDES += ../common/src
VPATH=$(INCLUDES)

# test-fw
TEST_SRCS = $(BASE_SRCS) $(PROTO_SRCS) test-main.c machtag.c pend.c
$(eval $(call make-firmware,test-fw,$(TEST_SRCS),$(CONFIG_MAX_ROM)))
$(eval $(call make-pablo,test-fw))

ifdef CONFIG_BOOTLOADER
# bootloader
BOOTLOADER_SRCS = $(BASE_SRCS) $(PROTO_SRCS) bootloader.c flash.c
$(eval $(call make-firmware,bootloader,$(BOOTLOADER_SRCS),$(CONFIG_BOOTLOADER_SIZE),$(LDFLAGS_BOOTLOADER)))
$(eval $(call make-bootloader,bootloader))
endif

DEFAULT_FIRMWARE=test-fw

# rules
include scripts/common-rules.mk
