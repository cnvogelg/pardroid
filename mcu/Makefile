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

# generated bootstrap code
BOOTSTRAP_CODE = ../amiga/BUILD/amiga68000/bootstrap
BOOTSTRAP_H = $(BUILD_DIR)/bootstrap.h
BOOTSTRAP_C = $(BUILD_DIR)/bootstrap.c
BOOTSTRAP_KNOK = $(BUILD_DIR)/bootstrap.knok
BOOTSTRAP_GEN = scripts/genbootstrap.py
# generated bootrexx code
BOOTREXX_CODE = ../amiga/src/bootstrap/boot.rexx
BOOTREXX_H = $(BUILD_DIR)/bootrexx.h
BOOTREXX_C = $(BUILD_DIR)/bootrexx.c

# base sources
COMMON_SRCS = machtag.c fwid.c
BASE_SRCS = timer.c strobe.c spi.c crc.c $(BASE_ARCH_SRCS)
BASE_SRCS += uart.c uartutil.c system.c pablo.c rominfo.c mem.c
PARAM_SRCS = param.c param_def.c crc.c
# pamela
PROTO_SRCS = $(PROTO_ARCH_SRCS)
PROTO_SRCS += proto.c reg.c action.c func.c
PROTO_EXT_SRCS = base_reg.c status.c tables.c offset.c
ifdef CONFIG_BOOTSTRAP
PROTO_EXT_SRCS += bootstrap.c bootrexx.c
endif
PROTO_EXT_SRCS += knok.c
PAMELA_SRCS = $(COMMON_SRCS) $(BASE_SRCS) $(PROTO_SRCS) $(PROTO_EXT_SRCS)

# handler
HANDLER_SRCS = handler.c handler_reg.c handler_regl.c msgio.c
HANDLER_SRCS += hnd_echo.c hnd_null.c

# drivers
DRIVER_SRCS = driver.c blk_null.c
ifdef CONFIG_DRIVER_ENC28J60
DRIVER_SRCS += enc28j60.c eth_enc.c
DRIVER_INCLUDES += src/driver/enc28j60
endif
ifdef CONFIG_DRIVER_SDCARD
DRIVER_SRCS += sdcard.c blk_sdraw.c
DRIVER_INCLUDES += src/driver/sdcard
endif
ifdef CONFIG_DRIVER_WIZNET
DRIVER_SRCS += wiznet_low.c wiznet.c wiz_io.c
DRIVER_INCLUDES += src/driver/wiznet
endif
ifdef CONFIG_DRIVER_UFAT
DRIVER_SRCS += ufat.c
DRIVER_INCLUDES += src/driver/ufat
endif

# common includes
INCLUDES := src
INCLUDES += src/arch src/$(ARCH_DIR) src/$(ARCH_DIR)/bsp src/arch/common
INCLUDES += src/$(MACH_DIR)
INCLUDES += src/base
INCLUDES += src/pamela src/handler src/driver src/test src/param
INCLUDES += ../common/src
INCLUDES += $(DRIVER_INCLUDES)
VPATH=$(INCLUDES) $(BUILD_DIR)

# ----- test firmware images -----
# test-base
TEST_BASE_SRCS = $(COMMON_SRCS) $(BASE_SRCS) $(PROTO_ARCH_SRCS)
TEST_BASE_SRCS += test-base.c
FW_ID_test-base-fw = 0x8001
FW_VER_test-base-fw = $(VERSION_TAG)
$(eval $(call dist-hex,test-base-fw))
$(eval $(call make-firmware,test-base-fw,$(TEST_BASE_SRCS),$(CONFIG_MAX_ROM)))

# test-param
TEST_PARAM_SRCS = $(COMMON_SRCS) $(BASE_SRCS) $(PROTO_ARCH_SRCS) $(PARAM_SRCS)
TEST_PARAM_SRCS += test-param.c
FW_ID_test-param-fw = 0x8008
FW_VER_test-param-fw = $(VERSION_TAG)
$(eval $(call dist-hex,test-param-fw))
$(eval $(call make-firmware,test-param-fw,$(TEST_PARAM_SRCS),$(CONFIG_MAX_ROM)))

# test-enc28j60
ifdef CONFIG_DRIVER_ENC28J60
TEST_ENC28J60_SRCS = $(COMMON_SRCS) $(BASE_SRCS)
TEST_ENC28J60_SRCS += test-enc28j60.c enc28j60.c
FW_ID_test-enc28j60-fw = 0x8006
FW_VER_test-enc28j60-fw = $(VERSION_TAG)
$(eval $(call dist-hex,test-enc28j60-fw))
$(eval $(call make-firmware,test-enc28j60-fw,$(TEST_ENC28J60_SRCS),$(CONFIG_MAX_ROM)))
endif

# test-sdcard
ifdef CONFIG_DRIVER_SDCARD
TEST_SDCARD_SRCS = $(COMMON_SRCS) $(BASE_SRCS)
TEST_SDCARD_SRCS += test-sdcard.c sdcard.c
FW_ID_test-sdcard-fw = 0x8004
FW_VER_test-scard-fw = $(VERSION_TAG)
$(eval $(call dist-hex,test-sdcard-fw))
$(eval $(call make-firmware,test-sdcard-fw,$(TEST_SDCARD_SRCS),$(CONFIG_MAX_ROM)))
endif

# test-wiznet
ifdef CONFIG_DRIVER_WIZNET
TEST_WIZNET_SRCS = $(COMMON_SRCS) $(BASE_SRCS)
TEST_WIZNET_SRCS += test-wiznet.c wiznet.c wiznet_low.c wiz_io.c
FW_ID_test-wiznet-fw = 0x8005
FW_VER_test-wiznet-fw = $(VERSION_TAG)
$(eval $(call dist-hex,test-wiznet-fw))
$(eval $(call make-firmware,test-wiznet-fw,$(TEST_WIZNET_SRCS),$(CONFIG_MAX_ROM)))
endif

# test-ufat
ifdef CONFIG_DRIVER_UFAT
TEST_UFAT_SRCS = $(COMMON_SRCS) $(BASE_SRCS)
TEST_UFAT_SRCS += test-ufat.c ufat.c sdcard.c
FW_ID_test-ufat-fw = 0x8008
FW_VER_test-ufat-fw = $(VERSION_TAG)
$(eval $(call dist-hex,test-ufat-fw))
$(eval $(call make-firmware,test-ufat-fw,$(TEST_UFAT_SRCS),$(CONFIG_MAX_ROM)))
endif

# test-net
TEST_NET_SRCS = $(PAMELA_SRCS) $(PARAM_SRCS)
TEST_NET_SRCS += test-net.c driver.c
TEST_NET_SRCS += $(DRIVER_SRCS)
FW_ID_test-net-fw = 0x8007
FW_VER_test-net-fw = $(VERSION_TAG)
$(eval $(call dist-hex,test-net-fw))
$(eval $(call make-firmware,test-net-fw,$(TEST_NET_SRCS),$(CONFIG_MAX_ROM)))

# test-pamela
TEST_PAMELA_SRCS = $(PAMELA_SRCS)
TEST_PAMELA_SRCS += test-pamela.c
FW_ID_test-pamela-fw = 0x8002
FW_VER_test-pamela-fw = $(VERSION_TAG)
$(eval $(call dist-pbl,test-pamela-fw))
$(eval $(call dist-hex,test-pamela-fw))
$(eval $(call make-firmware,test-pamela-fw,$(TEST_PAMELA_SRCS),$(CONFIG_MAX_ROM)))

# test-handler
TEST_HANDLER_SRCS = $(PAMELA_SRCS)
TEST_HANDLER_SRCS += $(HANDLER_SRCS) $(DRIVER_SRCS)
TEST_HANDLER_SRCS += test-handler.c
FW_ID_test-handler-fw = 0x8003
FW_VER_test-handler-fw = $(VERSION_TAG)
$(eval $(call dist-pbl,test-handler-fw))
$(eval $(call dist-hex,test-handler-fw))
$(eval $(call make-firmware,test-handler-fw,$(TEST_HANDLER_SRCS),$(CONFIG_MAX_ROM)))

ifndef DEBUG
ifdef CONFIG_BOOTLOADER
# bootloader
BOOTLOADER_SRCS = $(BASE_SRCS) proto.c reg.c action.c func.c bootloader.c flash.c
BOOTLOADER_SRCS += $(BOOTLOADER_ARCH_SRCS)
$(eval $(call dist-hex,bootloader))
$(eval $(call make-firmware,bootloader,$(BOOTLOADER_SRCS),$(CONFIG_BOOTLOADER_SIZE),$(LDFLAGS_BOOTLOADER)))
endif
endif

DEFAULT_FIRMWARE=test-proto-fw

# rules
include scripts/common-rules.mk

# bootstrap rules
$(BOOTSTRAP_H) $(BOOTSTRAP_C): $(BOOTSTRAP_GEN) $(BOOTSTRAP_CODE)
	@echo "  BOOTSTRAP from $(BOOTSTRAP_CODE)"
	$(H)$(BOOTSTRAP_GEN) $(BOOTSTRAP_CODE) $(BOOTSTRAP_C) $(BOOTSTRAP_H) \
		bootstrap $(BOOTSTRAP_KNOK)

$(BOOTREXX_H) $(BOOTREXX_C): $(BOOTSTRAP_GEN) $(BOOTREXX_CODE)
	@echo "  BOOTREXX from $(BOOTREXX_CODE)"
	$(H)$(BOOTSTRAP_GEN) $(BOOTREXX_CODE) $(BOOTREXX_C) $(BOOTREXX_H) \
		bootrexx
