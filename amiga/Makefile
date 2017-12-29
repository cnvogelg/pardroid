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
INCLUDES+=src/pamela src/engine src/pablo
INCLUDES+=src/test src/mini-test src/bench
INCLUDES+=../common/src
VPATH=$(INCLUDES)

BASE_SRCS = debug.c
PARIO_SRCS = pario.c pario_irq.s
PROTO_SRCS = proto_low.s proto.c
TIMER_SRCS = timer.c
WORKER_SRCS = worker.c
TEST_SRCS = test.c
BENCH_SRCS = bench.c

PAMELA_SRCS = $(BASE_SRCS) $(PARIO_SRCS) $(PROTO_SRCS) $(TIMER_SRCS) $(WORKER_SRCS)
PAMELA_SRCS += pamela.c reg.c offset.c status.c
ENGINE_SRCS += engine.c request.c channel.c

# ---- mini tests
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
TEST_ENGINE_SRCS = test-engine.c $(PAMELA_SRCS) $(ENGINE_SRCS)
$(eval $(call make-program,test-engine,$(TEST_ENGINE_SRCS)))

# ----- tests
# pb-test-pamela
PB_TEST_PAMELA_SRCS = pb-test-pamela.c $(PAMELA_SRCS) $(TEST_SRCS) tests_proto.c
$(eval $(call make-program,pb-test-pamela,$(PB_TEST_PAMELA_SRCS)))

# ----- bench
# pb-bench-proto
PB_BENCH_PAMELA_SRCS = pb-bench-pamela.c $(PAMELA_SRCS) $(BENCH_SRCS)
$(eval $(call make-program,pb-bench-pamela,$(PB_BENCH_PAMELA_SRCS)))

# ----- tools
# parbox-engine
PARBOX_ENGINE_SRCS = parbox-engine.c $(PAMELA_SRCS) $(ENGINE_SRCS)
$(eval $(call make-program,parbox-engine,$(PARBOX_ENGINE_SRCS)))

# parbox-client
PARBOX_CLIENT_SRCS = parbox-client.c $(PAMELA_SRCS) $(ENGINE_SRCS)
$(eval $(call make-program,parbox-client,$(PARBOX_CLIENT_SRCS)))

# pablo
PABLO_SRCS = pablo.c pblfile.c bootloader.c machtag.c fwid.c $(PAMELA_SRCS)
$(eval $(call make-program,pablo,$(PABLO_SRCS)))

# bootstrap: crunched pablo
PROGRAMS += bootstrap
bootstrap: $(BIN_DIR)/bootstrap
$(BIN_DIR)/bootstrap: $(BIN_DIR)/pablo
	@echo "  BOOTSTRAP "
	$(H)shrinkler -h $< $@ >/dev/null
	@stat -f '%z bytes' $@

# rules
include scripts/common-rules.mk
