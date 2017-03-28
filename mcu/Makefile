BASE_DIR=..
# config
CONFIG?=configs/pbnano.config
ALL_CONFIGS=configs/pbnano.config configs/pbcrasbe.config
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
MACHTAG=$(shell echo MACHTAG_$(CONFIG_ARCH)_$(CONFIG_MCU)_$(CONFIG_MACH) | tr a-z A-Z)
CFLAGS_DEFINES += -DMACHTAG=$(MACHTAG)

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
TEST_SRCS = $(BASE_SRCS) $(PROTO_SRCS) test-main.c machtag.c
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
