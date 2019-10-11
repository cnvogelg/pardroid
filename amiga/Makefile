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
INCLUDES+=src/pamela src/engine src/pablo
INCLUDES+=src/test src/mini-test src/bench
INCLUDES+=../common/src
INCLUDES+=src/device/shared src/device/pamela src/device/test
VPATH=$(INCLUDES)

BASE_SRCS = debug.c
PARIO_SRCS = pario.c pario_irq.s
PROTO_SRCS = proto_low.s proto.c
TIMER_SRCS = timer.c
TEST_SRCS = test.c
BENCH_SRCS = bench.c bench_main.c

PAMELA_SRCS = $(BASE_SRCS) $(PARIO_SRCS) $(PROTO_SRCS) $(TIMER_SRCS)
PAMELA_SRCS += pamela.c

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
# pb-test-pamela
PB_TEST_PAMELA_SRCS = pb-test-pamela.c $(PAMELA_SRCS) $(TEST_SRCS) tests_pamela.c
$(eval $(call make-program,pb-test-pamela,$(PB_TEST_PAMELA_SRCS)))
$(eval $(call dist-program,pb-test-pamela))

# ----- bench
# pb-bench-proto
PB_BENCH_PAMELA_SRCS = pb-bench-pamela.c $(PAMELA_SRCS) $(BENCH_SRCS)
$(eval $(call make-program,pb-bench-pamela,$(PB_BENCH_PAMELA_SRCS)))
$(eval $(call dist-program,pb-bench-pamela))

# ----- tools
# pablo
PABLO_SRCS = pablo.c pblfile.c bootloader.c machtag.c fwid.c $(PAMELA_SRCS)
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
# pamela.device
PAMELA_DEV_SRCS = pamela_dev.c
$(eval $(call make-device,pamela.device,$(PAMELA_DEV_SRCS)))
$(eval $(call dist-device,pamela.device))

# pamela device test
PAMDEV_TEST_SRCS = pamdev_test.c
$(eval $(call make-program,pamdev_test,$(PAMDEV_TEST_SRCS)))
$(eval $(call dist-program,pamdev_test))

# rules
include scripts/common-rules.mk
