BASE_DIR=..
# config
CONFIG?=configs/amiga68000.config
include $(BASE_DIR)/scripts/gencfg.mk
# common defs
include scripts/common-defs.mk
# toolchain
include scripts/toolchain-$(CONFIG_ARCH).mk

# paths
INCLUDES=src src/arch src/$(ARCH_DIR) src/$(MACH_DIR)
VPATH=$(INCLUDES)

# parbox-test
PARBOX_TEST_SRCS = parbox-test.c debug.c pario.c pario_irq.s
$(eval $(call make-program,parbox-test,$(PARBOX_TEST_SRCS)))

# rules
include scripts/common-rules.mk
