BASE_DIR=..
# config
CONFIG?=configs/amiga68000.config
include $(BASE_DIR)/scripts/gencfg.mk
# common defs
include scripts/common-defs.mk
# toolchain
include scripts/toolchain-$(CONFIG_ARCH).mk

# paths
INCLUDES=src src/arch src/$(ARCH_DIR) src/$(MACH_DIR) src/base src/parbox src/pablo
INCLUDES+=../common/src
VPATH=$(INCLUDES)

BASE_SRCS = debug.c
PARIO_SRCS = pario.c pario_irq.s
PROTO_SRCS = proto_low.s proto.c
TIMER_SRCS = timer.c

PARBOX_SRCS = $(BASE_SRCS) $(PARIO_SRCS) $(PROTO_SRCS) $(TIMER_SRCS)
PARBOX_SRCS += parbox.c

# test-pario
TEST_PARIO_SRCS = test-pario.c $(PARIO_SRCS) $(BASE_SRCS)
$(eval $(call make-program,test-pario,$(TEST_PARIO_SRCS)))

# test-timer
TEST_TIMER_SRCS = test-timer.c $(TIMER_SRCS) $(BASE_SRCS)
$(eval $(call make-program,test-timer,$(TEST_TIMER_SRCS)))

# test-proto
TEST_PARIO_SRCS = test-proto.c $(PARIO_SRCS) $(PROTO_SRCS) $(TIMER_SRCS) $(BASE_SRCS)
$(eval $(call make-program,test-proto,$(TEST_PARIO_SRCS)))

# parbox-test
PARBOX_TEST_SRCS = parbox-test.c $(PARBOX_SRCS)
$(eval $(call make-program,parbox-test,$(PARBOX_TEST_SRCS)))

# pablo
PABLO_SRCS = pablo.c pblfile.c bootloader.c machtag.c $(PARBOX_SRCS)
$(eval $(call make-program,pablo,$(PABLO_SRCS)))

# rules
include scripts/common-rules.mk
