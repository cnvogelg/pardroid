BASE_DIR=..
# config
CONFIG?=configs/ardunano.config
include $(BASE_DIR)/scripts/gencfg.mk
# common defs
include scripts/common-defs.mk
# toolchain
include scripts/toolchain-$(CONFIG_ARCH).mk
# flash rules
include scripts/flash-$(CONFIG_FLASH_TOOL).mk

# sources
BASE_SRCS = uart.c util.c uartutil.c mach.c
PARBOX_SRCS = proto_low.S proto.c

INCLUDES=src src/arch src/$(ARCH_DIR) src/$(MACH_DIR) src/base src/parbox
VPATH=$(INCLUDES)

# test-fw
TEST_SRCS = $(BASE_SRCS) $(PARBOX_SRCS) test-main.c
$(eval $(call make-firmware,test-fw,$(TEST_SRCS)))

DEFAULT_FIRMWARE=test-fw

# rules
include scripts/common-rules.mk
