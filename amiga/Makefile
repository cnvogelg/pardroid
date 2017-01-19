BASE_DIR=..
# config
CONFIG?=configs/amiga68000.config
include $(BASE_DIR)/scripts/gencfg.mk
# common defs
include scripts/common-defs.mk
# toolchain
include scripts/toolchain-$(CONFIG_ARCH).mk

# paths
INCLUDES=src src/arch src/$(ARCH_DIR) src/$(MACH_DIR) src/base src/parbox
VPATH=$(INCLUDES)

BASE_SRCS = debug.c
PARIO_SRCS = pario.c pario_irq.s
PARBOX_SRCS = proto_low.s proto.c
TIMER_SRCS = timer.c

# test-pario
TEST_PARIO_SRCS = test-pario.c $(PARIO_SRCS) $(BASE_SRCS)
$(eval $(call make-program,test-pario,$(TEST_PARIO_SRCS)))

# test-timer
TEST_TIMER_SRCS = test-timer.c $(TIMER_SRCS) $(BASE_SRCS)
$(eval $(call make-program,test-timer,$(TEST_TIMER_SRCS)))

# test-proto
TEST_PARIO_SRCS = test-proto.c $(PARIO_SRCS) $(PARBOX_SRCS) $(TIMER_SRCS) $(BASE_SRCS)
$(eval $(call make-program,test-proto,$(TEST_PARIO_SRCS)))

# rules
include scripts/common-rules.mk
