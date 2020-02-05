BASE_DIR=..
# config
CONFIG?=configs/amiga68000.config
COMPILER?=vbcc
include $(BASE_DIR)/scripts/gencfg.mk
# common defs
include scripts/common-defs.mk
# toolchain
include scripts/toolchain-$(CONFIG_ARCH)-$(COMPILER).mk
include scripts/toolchain-$(CONFIG_ARCH).mk
# version
include ../version.mk

ALL_CONFIGS=configs/amiga68000.config
ALL_COMPILERS=vbcc gcc

# paths
INCLUDES=src src/arch src/$(ARCH_DIR) src/$(MACH_DIR) src/base
INCLUDES+=src/proto src/pamela src/pablo
INCLUDES+=src/test src/mini-test src/bench
INCLUDES+=../common/src include
INCLUDES+=src/device/shared src/device/pamela src/device/test
VPATH=$(INCLUDES)

BASE_SRCS = debug.c
PARIO_SRCS = pario.c pario_irq.s
PROTO_SRCS = proto_low.s proto_env.c proto.c
TIMER_SRCS = timer.c
TEST_SRCS = test.c test-buffer.c
BENCH_SRCS = bench.c bench_main.c

PROTO_ENV_SRCS = $(BASE_SRCS) $(PARIO_SRCS) $(PROTO_SRCS) $(TIMER_SRCS)
PAMELA_SRCS = channel.c channel_ops.c

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

# ----- tests
# pb-test-proto
PB_TEST_PROTO_SRCS = pb-test-proto.c $(PROTO_ENV_SRCS) $(TEST_SRCS) 
PB_TEST_PROTO_SRCS += proto-testsuite-msg.c proto-testsuite-event.c
PB_TEST_PROTO_SRCS += proto-testsuite-ext.c
PB_TEST_PROTO_SRCS += proto-testsuite.c
$(eval $(call make-program,pb-test-proto,$(PB_TEST_PROTO_SRCS)))
$(eval $(call dist-program,pb-test-proto))

# pb-test-pamela
PB_TEST_PAMELA_SRCS = pb-test-pamela.c $(PROTO_ENV_SRCS) $(PAMELA_SRCS) $(TEST_SRCS) 
PB_TEST_PAMELA_SRCS += pamela-testsuite.c
$(eval $(call make-program,pb-test-pamela,$(PB_TEST_PAMELA_SRCS)))
$(eval $(call dist-program,pb-test-pamela))

# ----- bench
# pb-bench-proto
PB_BENCH_PROTO_SRCS = pb-bench-proto.c $(PROTO_ENV_SRCS) $(BENCH_SRCS)
$(eval $(call make-program,pb-bench-proto,$(PB_BENCH_PROTO_SRCS)))
$(eval $(call dist-program,pb-bench-proto))

# ----- tools
# pablo
PABLO_SRCS = pablo.c pblfile.c bootloader.c machtag.c fwid.c $(PROTO_ENV_SRCS)
$(eval $(call make-program,pablo,$(PABLO_SRCS)))
$(eval $(call dist-program,pablo))

# ----- bootstrap
# boot.bin
BOOTSTRAP_SRCS = bootstrap.c $(PAMELA_SRCS)
$(eval $(call make-program,bootstrap,$(PABLO_SRCS)))
$(eval $(call crunch-program,boot.bin,bootstrap))
$(eval $(call dist-program,boot.bin))

# boot.run
BOOTRUN_FILES = pablo
$(eval $(call create-lha,boot.lha,$(BOOTRUN_FILES)))
$(eval $(call sfx-lha,boot.run,boot.lha))
$(eval $(call dist-program,boot.run))

# ----- devices
# common sources
PAMELA_DEV_SRCS = pamela_dev.c pamela_worker.c pam_engine.c
# pambox.device
PAMBOX_DEV_SRCS = pamela_boxdev.c pambox.c $(PAMELA_DEV_SRCS)
$(eval $(call make-device,pambox.device,$(PAMBOX_DEV_SRCS)))
$(eval $(call dist-device,pambox.device))
# pammet.device
PAMNET_DEV_SRCS = pamela_netdev.c pamnet.c $(PAMELA_DEV_SRCS)
$(eval $(call make-device,pamnet.device,$(PAMNET_DEV_SRCS)))
$(eval $(call dist-device,pamnet.device))

# pamela device test
PAMDEV_TEST_SRCS = pamdev_test.c
$(eval $(call make-program,pamdev_test,$(PAMDEV_TEST_SRCS)))
$(eval $(call dist-program,pamdev_test))

# rules
include scripts/common-rules.mk

gcc:
	$(MAKE) COMPILER=gcc
