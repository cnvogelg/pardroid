BASE_DIR=..

# config setup
CONFIG?=configs/pbnano.config
ifeq "$(words $(wildcard $(CONFIG)))" "0"
	dummy:=$(error config not found: $(CONFIG))
endif
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
MACH_UPPER=$(shell echo "$(CONFIG_MACH)" | tr a-z A-Z)
CFLAGS_DEFINES += -DMACHTAG=$(MACHTAG_SYMBOL) -DMACH_$(MACH_UPPER)

# generate version tag
VERSION_TAG=$(shell scripts/vertag.py $(VERSION_MAJOR) $(VERSION_MINOR))
CFLAGS_DEFINES += -DVERSION_TAG=$(VERSION_TAG)

# dist tag
DIST_TAG=$(CONFIG_BASE)-$(VERSION_MAJOR).$(VERSION_MINOR)

# sources
BASE_SRCS = uart.c uartutil.c system.c pablo.c
PROTO_SRCS = proto_low.S proto.c reg.c action.c func.c
PROTO_EXT_SRCS = reg_def.c status.c tables.c offset.c
HANDLER_SRCS = handler.c buffer.c channel.c hnd_echo.c

INCLUDES=src src/arch src/$(ARCH_DIR) src/$(MACH_DIR) src/base src/parbox
INCLUDES += src/handler
INCLUDES += ../common/src
VPATH=$(INCLUDES)

# test-fw
TEST_SRCS = $(BASE_SRCS) $(PROTO_SRCS) $(PROTO_EXT_SRCS) $(HANDLER_SRCS)
TEST_SRCS += test-main.c machtag.c
$(eval $(call dist-pbl,test-fw))
$(eval $(call dist-hex,test-fw))
$(eval $(call make-firmware,test-fw,$(TEST_SRCS),$(CONFIG_MAX_ROM)))

ifdef CONFIG_BOOTLOADER
# bootloader
BOOTLOADER_SRCS = $(BASE_SRCS) $(PROTO_SRCS) bootloader.c flash.c
$(eval $(call dist-hex,bootloader))
$(eval $(call make-firmware,bootloader,$(BOOTLOADER_SRCS),$(CONFIG_BOOTLOADER_SIZE),$(LDFLAGS_BOOTLOADER)))
endif

DEFAULT_FIRMWARE=test-fw

# rules
include scripts/common-rules.mk
