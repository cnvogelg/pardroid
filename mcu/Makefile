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

# avr configs
ALL_CONFIGS=configs/pbnano.config configs/pbcrasbe.config configs/avrnetio.config
ALL_CONFIGS+=configs/teensy20.config
# mk20 configs
ALL_CONFIGS+=configs/teensy32.config

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
dummy := $(info arch=$(CONFIG_ARCH) mach=$(CONFIG_MACH))

# generate version tag
VERSION_TAG=$(shell scripts/vertag.py $(VERSION_MAJOR) $(VERSION_MINOR))
CFLAGS_DEFINES += -DVERSION_TAG=$(VERSION_TAG)

# dist tag
DIST_TAG=$(CONFIG_BASE)-$(VERSION_MAJOR).$(VERSION_MINOR)

# base sources
COMMON_SRCS = machtag.c fwid.c
BASE_SRCS = timer.c $(BASE_ARCH_SRCS)
BASE_SRCS += uart.c uartutil.c system.c pablo.c rominfo.c
PROTO_SRCS = $(PROTO_ARCH_SRCS)
PROTO_SRCS += proto.c reg.c action.c func.c
PROTO_EXT_SRCS = base_reg.c status.c tables.c offset.c
ALL_BASE_SRCS = $(COMMON_SRCS) $(BASE_SRCS) $(PROTO_SRCS) $(PROTO_EXT_SRCS)

# handler
HANDLER_SRCS = handler.c handler_reg.c buffer.c msgio.c hnd_echo.c hnd_null.c

# drivers
DRIVER_SRCS = driver.c drv_null.c
ifdef CONFIG_SPI
DRIVER_SRCS += spi.c
endif
ifdef CONFIG_DRIVER_ENC28J60
DRIVER_SRCS += enc28j60.c drv_enc28j60.c
DRIVER_INCLUDES += src/driver/enc28j60
endif
ifdef CONFIG_DRIVER_SDCARD
DRIVER_SRCS += sdcard.c drv_sdcard.c
DRIVER_INCLUDES += src/driver/sdcard
endif

# common includes
INCLUDES := src
INCLUDES += src/arch src/$(ARCH_DIR) src/$(ARCH_DIR)/bsp src/arch/common
INCLUDES += src/$(MACH_DIR)
INCLUDES += src/base
INCLUDES += src/proto src/handler src/driver
INCLUDES += ../common/src
INCLUDES += $(DRIVER_INCLUDES)
VPATH=$(INCLUDES)

# test-base
TEST_BASE_SRCS = $(COMMON_SRCS) $(BASE_SRCS) $(PROTO_ARCH_SRCS)
TEST_BASE_SRCS += test-base.c
ifdef CONFIG_DRIVER_SDCARD
TEST_BASE_SRCS += spi.c sdcard.c
endif
FW_ID_test-base-fw = 0x8001
FW_VER_test-base-fw = $(VERSION_TAG)
$(eval $(call dist-pbl,test-base-fw))
$(eval $(call dist-hex,test-base-fw))
$(eval $(call make-firmware,test-base-fw,$(TEST_BASE_SRCS),$(CONFIG_MAX_ROM)))

# test-proto
TEST_PROTO_SRCS = $(ALL_BASE_SRCS)
TEST_PROTO_SRCS += test-proto.c
FW_ID_test-proto-fw = 0x8002
FW_VER_test-proto-fw = $(VERSION_TAG)
$(eval $(call dist-pbl,test-proto-fw))
$(eval $(call dist-hex,test-proto-fw))
$(eval $(call make-firmware,test-proto-fw,$(TEST_PROTO_SRCS),$(CONFIG_MAX_ROM)))

# test-handler
TEST_HANDLER_SRCS = $(ALL_BASE_SRCS)
TEST_HANDLER_SRCS += $(HANDLER_SRCS) $(DRIVER_SRCS)
TEST_HANDLER_SRCS += test-handler.c
FW_ID_test-handler-fw = 0x8003
FW_VER_test-handler-fw = $(VERSION_TAG)
$(eval $(call dist-pbl,test-handler-fw))
$(eval $(call dist-hex,test-handler-fw))
$(eval $(call make-firmware,test-handler-fw,$(TEST_HANDLER_SRCS),$(CONFIG_MAX_ROM)))

ifdef CONFIG_BOOTLOADER
# bootloader
BOOTLOADER_SRCS = $(BASE_SRCS) $(PROTO_SRCS) bootloader.c flash.c
$(eval $(call dist-hex,bootloader))
$(eval $(call make-firmware,bootloader,$(BOOTLOADER_SRCS),$(CONFIG_BOOTLOADER_SIZE),$(LDFLAGS_BOOTLOADER)))
endif

DEFAULT_FIRMWARE=test-proto-fw

# rules
include scripts/common-rules.mk
