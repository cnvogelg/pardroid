BASE_DIR=..
# config
CONFIG?=configs/ardunano.config
include $(BASE_DIR)/scripts/gencfg.mk
# version
include ../version.mk
# common defs
include scripts/common-defs.mk
# toolchain
include scripts/toolchain-$(CONFIG_ARCH).mk
# flash rules
include scripts/flash-$(CONFIG_FLASH_TOOL).mk

# generate machine tag
MACH_TAG=$(shell scripts/machtag.py $(CONFIG_ARCH_ID) $(CONFIG_MCU_ID) $(CONFIG_MACH_ID))
CFLAGS_DEFINES += -DMACH_TAG=$(MACH_TAG)
# generate version tag
VERSION_TAG=$(shell scripts/vertag.py $(VERSION_MAJOR) $(VERSION_MINOR))
CFLAGS_DEFINES += -DVERSION_TAG=$(VERSION_TAG)

# sources
BASE_SRCS = uart.c uartutil.c mach.c pablo.c
PROTO_SRCS = proto_low.S proto.c reg_ro.c

INCLUDES=src src/arch src/$(ARCH_DIR) src/$(MACH_DIR) src/base src/parbox
INCLUDES += ../common/src
VPATH=$(INCLUDES)

# test-fw
TEST_SRCS = $(BASE_SRCS) $(PROTO_SRCS) test-main.c
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
