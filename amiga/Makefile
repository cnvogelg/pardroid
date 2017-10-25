BASE_DIR=..
# config
CONFIG?=configs/amiga68000.config
include $(BASE_DIR)/scripts/gencfg.mk
# common defs
include scripts/common-defs.mk
# toolchain
include scripts/toolchain-$(CONFIG_ARCH).mk
# version
include ../version.mk

# dist tag
DIST_TAG=$(CONFIG_BASE)-$(VERSION_MAJOR).$(VERSION_MINOR)

# paths
INCLUDES=src src/arch src/$(ARCH_DIR) src/$(MACH_DIR) src/base
INCLUDES+=src/parbox src/engine src/pablo src/test
INCLUDES+=../common/src
VPATH=$(INCLUDES)

BASE_SRCS = debug.c
PARIO_SRCS = pario.c pario_irq.s
PROTO_SRCS = proto_low.s proto.c
TIMER_SRCS = timer.c
WORKER_SRCS = worker.c
TEST_SRCS = test.c

PARBOX_SRCS = $(BASE_SRCS) $(PARIO_SRCS) $(PROTO_SRCS) $(TIMER_SRCS) $(WORKER_SRCS)
PARBOX_SRCS += parbox.c reg.c offset.c engine.c request.c channel.c

# test-pario
TEST_PARIO_SRCS = test-pario.c $(PARIO_SRCS) $(BASE_SRCS)
$(eval $(call make-program,test-pario,$(TEST_PARIO_SRCS)))

# test-timer
TEST_TIMER_SRCS = test-timer.c $(TIMER_SRCS) $(BASE_SRCS)
$(eval $(call make-program,test-timer,$(TEST_TIMER_SRCS)))

# test-proto
TEST_PARIO_SRCS = test-proto.c $(PARIO_SRCS) $(PROTO_SRCS) $(TIMER_SRCS) $(BASE_SRCS)
$(eval $(call make-program,test-proto,$(TEST_PARIO_SRCS)))

# test-worker
TEST_WORKER_SRCS = test-worker.c $(WORKER_SRCS) $(BASE_SRCS)
$(eval $(call make-program,test-worker,$(TEST_WORKER_SRCS)))

# test-engine
TEST_ENGINE_SRCS = test-engine.c $(PARBOX_SRCS)
$(eval $(call make-program,test-engine,$(TEST_ENGINE_SRCS)))

# parbox-test
PARBOX_TEST_SRCS = parbox-test.c $(PARBOX_SRCS) $(TEST_SRCS) tests_proto.c
$(eval $(call make-program,parbox-test,$(PARBOX_TEST_SRCS)))

# parbox-engine
PARBOX_ENGINE_SRCS = parbox-engine.c $(PARBOX_SRCS)
$(eval $(call make-program,parbox-engine,$(PARBOX_ENGINE_SRCS)))

# parbox-client
PARBOX_CLIENT_SRCS = parbox-client.c $(PARBOX_SRCS)
$(eval $(call make-program,parbox-client,$(PARBOX_CLIENT_SRCS)))

# pablo
PABLO_SRCS = pablo.c pblfile.c bootloader.c machtag.c $(PARBOX_SRCS)
$(eval $(call make-program,pablo,$(PABLO_SRCS)))

# rules
include scripts/common-rules.mk
