BASE_DIR=..

# config setup
CONFIG?=configs/teensy20.config
ifeq "$(words $(wildcard $(CONFIG)))" "0"
	dummy:=$(error config not found: $(CONFIG))
endif
FLASH_CONFIG?=configs/flash.config
EXTRA_CONFIG=$(FLASH_CONFIG) $(USER_CONFIG)
EXTRA_CONFIG+=$(wildcard configs/$(FLAVOR).config)

# avr configs
ALL_CONFIGS=configs/pbnano.config configs/pbcrasbe.config configs/avrnetio.config
ALL_CONFIGS+=configs/teensy20.config
# mk20 configs
ALL_CONFIGS+=configs/teensy32.config
# rp2 configs
ALL_CONFIGS+=configs/pico.config

include $(BASE_DIR)/scripts/gencfg.mk
# version
include ../version.mk
# common defs
include scripts/common-defs.mk

ifeq "$(CONFIG_MAKESYS)" ""
# toolchain
include scripts/toolchain-$(CONFIG_ARCH).mk
# flash rules
include scripts/flash-$(CONFIG_FLASH_TOOL).mk
endif

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

# generate rom info file
ROMINFO_FILE = $(BUILD_DIR)/rominfo_gen.h
ROMINFO=scripts/rominfo.py

# generated bootstrap code
BOOTSTRAP_CODE = ../amiga/BUILD/$(FLAVOR)-vbcc/m68k-cia/boot.bin
BOOTSTRAP_H = $(BUILD_DIR)/bootstrap.h
BOOTSTRAP_C = $(BUILD_DIR)/bootstrap.c
BOOTSTRAP_KNOK = $(BUILD_DIR)/bootstrap.knok
BOOTSTRAP_GEN = scripts/genbootstrap.py
# generated bootrexx code
BOOTREXX_CODE = ../amiga/src/bootstrap/boot.rexx
BOOTREXX_H = $(BUILD_DIR)/bootrexx.h
BOOTREXX_C = $(BUILD_DIR)/bootrexx.c

# base sources
BASE_SRCS = hw_system.c hw_timer.c hw_uart.c hw_i2c.c hw_spi.c
BASE_SRCS += $(BASE_ARCH_SRCS)
BASE_SRCS += strobe.c crc.c uartutil.c pablo.c rominfo.c #mem.c

# proto sources
PROTO_ATOM_SRCS = $(PROTO_ATOM_ARCH_SRCS) proto_atom.c
PROTO_DEV_SRCS = $(PROTO_ATOM_SRCS) knok.c proto_dev.c
PROTO_IO_SRCS = $(PROTO_DEV_SRCS) proto_io.c
PROTO_BOOT_SRCS = $(PROTO_BOOT_ARCH_SRCS) proto_atom.c proto_dev.c proto_boot.c

# pamela
PAMELA_SRCS = $(BASE_SRCS) $(PROTO_IO_SRCS)
PAMELA_SRCS += pamela.c pamela_io.c

# paloma
PALOMA_SRCS = handler.c handler_reg.c handler_regl.c msgio.c channel.c
PALOMA_SRCS += paloma.c

# handlers
HANDLER_SRCS += hnd_echo.c hnd_null.c handler_list.c

# drivers
DRIVER_SRCS = driver.c blk_null.c driver_list.c
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
ifdef CONFIG_TOOL_UFAT
TOOL_SRCS += ufat.c
TOOL_INCLUDES += src/tools/ufat
endif
DRIVER_SRCS += ssd1306.c display.c font8x16.c
DRIVER_INCLUDES += src/driver/display src/driver/display/ssd1306

# common includes
INCLUDES := src
INCLUDES += src/arch src/$(ARCH_DIR) src/$(ARCH_DIR)/bsp src/arch/common
INCLUDES += src/$(MACH_DIR)
INCLUDES += src/base src/proto
INCLUDES += src/pamela src/paloma src/handler src/driver src/test src/param
INCLUDES += ../common/src
INCLUDES += $(DRIVER_INCLUDES) $(TOOL_INCLUDES)
VPATH=$(INCLUDES) $(BUILD_DIR)

# ----- test firmware images -----
# test-base
TEST_BASE_SRCS = $(COMMON_SRCS) $(BASE_SRCS)
TEST_BASE_SRCS += test-base.c
$(eval $(call make-fw,test-base-fw,$(TEST_BASE_SRCS),$(CONFIG_MAX_ROM)))

# test-proto-atom
TEST_PROTO_ATOM_SRCS = $(COMMON_SRCS) $(BASE_SRCS) $(PROTO_ATOM_SRCS)
TEST_PROTO_ATOM_SRCS += test-proto-atom.c
$(eval $(call make-fw,test-proto-atom-fw,$(TEST_PROTO_ATOM_SRCS),$(CONFIG_MAX_ROM)))

# test-proto-dev
TEST_PROTO_DEV_SRCS = $(COMMON_SRCS) $(BASE_SRCS) $(PROTO_DEV_SRCS)
TEST_PROTO_DEV_SRCS += test-proto-dev.c
$(eval $(call make-fw,test-proto-dev-fw,$(TEST_PROTO_DEV_SRCS),$(CONFIG_MAX_ROM)))

# test-proto-io
TEST_PROTO_IO_SRCS = $(COMMON_SRCS) $(BASE_SRCS) $(PROTO_IO_SRCS)
TEST_PROTO_IO_SRCS += test-proto-io.c
$(eval $(call make-fw,test-proto-io-fw,$(TEST_PROTO_IO_SRCS),$(CONFIG_MAX_ROM)))

# test-proto-boot
TEST_PROTO_BOOT_SRCS = $(COMMON_SRCS) $(BASE_SRCS) $(PROTO_BOOT_SRCS)
TEST_PROTO_BOOT_SRCS += test-proto-boot.c
$(eval $(call make-fw,test-proto-boot-fw,$(TEST_PROTO_BOOT_SRCS),$(CONFIG_MAX_ROM)))

# test-sdcard
ifdef CONFIG_DRIVER_SDCARD
TEST_SDCARD_SRCS = $(COMMON_SRCS) $(BASE_SRCS)
TEST_SDCARD_SRCS += test-sdcard.c sdcard.c
$(eval $(call make-firmware,test-sdcard-fw,$(TEST_SDCARD_SRCS),$(CONFIG_MAX_ROM)))
endif

# disabled for now
ifeq "a" "b"

# test-param
TEST_PARAM_SRCS = $(COMMON_SRCS) $(BASE_SRCS) $(PROTO_ARCH_SRCS) $(PARAM_SRCS)
TEST_PARAM_SRCS += test-param.c
$(eval $(call make-firmware,test-param-fw,$(TEST_PARAM_SRCS),$(CONFIG_MAX_ROM)))

# test-enc28j60
ifdef CONFIG_DRIVER_ENC28J60
TEST_ENC28J60_SRCS = $(COMMON_SRCS) $(BASE_SRCS)
TEST_ENC28J60_SRCS += test-enc28j60.c enc28j60.c
$(eval $(call make-firmware,test-enc28j60-fw,$(TEST_ENC28J60_SRCS),$(CONFIG_MAX_ROM)))
endif

# test-wiznet
ifdef CONFIG_DRIVER_WIZNET
TEST_WIZNET_SRCS = $(COMMON_SRCS) $(BASE_SRCS)
TEST_WIZNET_SRCS += test-wiznet.c wiznet.c wiznet_low.c wiz_io.c
$(eval $(call make-firmware,test-wiznet-fw,$(TEST_WIZNET_SRCS),$(CONFIG_MAX_ROM)))
endif

# test-display
ifdef CONFIG_DRIVER_DISPLAY
TEST_DISPLAY_SRCS = $(COMMON_SRCS) $(BASE_SRCS)
TEST_DISPLAY_SRCS += test-display.c ssd1306.c display.c font6x8.c
$(eval $(call make-firmware,test-display-fw,$(TEST_DISPLAY_SRCS),$(CONFIG_MAX_ROM)))
endif

# test-ufat
ifdef CONFIG_TOOL_UFAT
TEST_UFAT_SRCS = $(COMMON_SRCS) $(BASE_SRCS)
TEST_UFAT_SRCS += test-ufat.c ufat.c sdcard.c
$(eval $(call make-firmware,test-ufat-fw,$(TEST_UFAT_SRCS),$(CONFIG_MAX_ROM)))
endif

# test-net
#TEST_NET_SRCS = $(PAMELA_SRCS) $(PARAM_SRCS)
#TEST_NET_SRCS += test-net.c driver.c
#TEST_NET_SRCS += $(DRIVER_SRCS)
#$(eval $(call make-firmware,test-net-fw,$(TEST_NET_SRCS),$(CONFIG_MAX_ROM)))

# test-paloma
#TEST_PALOMA_SRCS = $(PAMELA_SRCS) $(PALOMA_SRCS)
#TEST_PALOMA_SRCS += $(HANDLER_SRCS) $(DRIVER_SRCS)
#TEST_PALOMA_SRCS += test-paloma.c
#$(eval $(call dist-pbl,test-paloma-fw))
#$(eval $(call make-pbl,test-paloma-fw))
#$(eval $(call dist-hex,test-paloma-fw))
#$(eval $(call make-firmware,test-paloma-fw,$(TEST_PALOMA_SRCS),$(CONFIG_MAX_ROM)))

# disable
endif

# ----- main test suites -----
# test-pamela
TEST_PAMELA_SRCS = $(PAMELA_SRCS)
TEST_PAMELA_SRCS += test-pamela.c
$(eval $(call dist-pbl,test-pamela-fw))
$(eval $(call make-pbl,test-pamela-fw))
$(eval $(call dist-hex,test-pamela-fw))
$(eval $(call make-firmware,test-pamela-fw,$(TEST_PAMELA_SRCS),$(CONFIG_MAX_ROM)))

# ----- bootloader -----
#ifeq "$(FLAVOR)" "release"
ifdef CONFIG_BOOTLOADER
# bootloader
BOOTLOADER_SRCS = hw_uart.c hw_system.c pablo.c uartutil.c
BOOTLOADER_SRCS += bootbase.c bootloader.c flash.c
BOOTLOADER_SRCS += $(PROTO_BOOT_SRCS) $(BOOTLOADER_ARCH_SRCS)
$(eval $(call dist-hex,bootloader))
$(eval $(call make-firmware,bootloader,$(BOOTLOADER_SRCS),$(CONFIG_BOOTLOADER_SIZE),$(LDFLAGS_BOOTLOADER)))
endif
#endif

DEFAULT_FIRMWARE=test-proto-atom-fw

# ---------- rules ----------
include scripts/common-rules.mk

# ----- bootstrap rules -----
$(BOOTSTRAP_H) $(BOOTSTRAP_C): $(BOOTSTRAP_GEN) $(BOOTSTRAP_CODE)
	@echo "  BOOTSTRAP from $(BOOTSTRAP_CODE)"
	$(H)$(BOOTSTRAP_GEN) $(BOOTSTRAP_CODE) $(BOOTSTRAP_C) $(BOOTSTRAP_H) \
		bootstrap $(BOOTSTRAP_KNOK)

$(BOOTREXX_H) $(BOOTREXX_C): $(BOOTSTRAP_GEN) $(BOOTREXX_CODE)
	@echo "  BOOTREXX from $(BOOTREXX_CODE)"
	$(H)$(BOOTSTRAP_GEN) $(BOOTREXX_CODE) $(BOOTREXX_C) $(BOOTREXX_H) \
		bootrexx

# ----- rominfo rules -----
$(ROMINFO_FILE): $(ROMINFO)
	@echo "  ROMINFO gen $@"
	$(H)$(ROMINFO) -o $(ROMINFO_FILE) -v \
		$(CONFIG_ARCH) $(CONFIG_MCU) $(CONFIG_MACH) $(CONFIG_MACH_VARIANT) \
		ver:$(VERSION) $(VERSION_DATE) $(FLAVOR)

src/base/rominfo.c: $(ROMINFO_FILE)
