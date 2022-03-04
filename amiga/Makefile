BASE_DIR=..
# config
DEFAULT_CONFIG=m68k-cia
CONFIG?=configs/$(DEFAULT_CONFIG).config
COMPILER?=vbcc
include $(BASE_DIR)/scripts/gencfg.mk
# common defs
include scripts/common-defs.mk
# toolchain
include scripts/toolchain-$(CONFIG_ARCH)-$(COMPILER).mk
include scripts/toolchain-$(CONFIG_ARCH).mk
# version
include ../version.mk

ALL_CONFIGS=configs/m68k-cia.config
ALL_COMPILERS=vbcc gcc

# paths
INCLUDES=src src/arch src/$(ARCH_DIR) src/$(MACH_DIR) src/base
INCLUDES+=src/proto src/pamela src/pablo
INCLUDES+=src/test src/mini-test src/bench
INCLUDES+=../common/src include
INCLUDES+=src/device/shared src/device/pamela src/device/test
VPATH=$(INCLUDES)

# base
BASE_SRCS = debug.c
TIMER_SRCS = timer.c
PARIO_SRCS = $(PARIO_$(CONFIG_MACH)_SRCS)
PARIO_cia_SRCS = pario.c pario_irq.s

# pario+proto
PROTO_cia_SRCS = proto_low.s proto_low_cfg.c
PROTO_ENV_SRCS = proto_env.c $(TIMER_SRCS) $(PARIO_SRCS)
PROTO_ATOM_SRCS = $(PROTO_$(CONFIG_MACH)_SRCS) $(PROTO_ENV_SRCS) proto_atom.c
PROTO_DEV_SRCS = $(PROTO_ATOM_SRCS) proto_dev.c
PROTO_IO_SRCS = $(PROTO_DEV_SRCS) proto_io.c
PROTO_BOOT_SRCS = $(PROTO_DEV_SRCS) proto_boot.c

# pamela
PAMELA_SRCS = $(PROTO_IO_SRCS) pamela.c pamela_err.c pamela_chn.c

# test+bench
TEST_SRCS = test.c test-buffer.c
BENCH_SRCS = bench.c bench_main.c

# ---- mini tests
# minitest-pario
MINITEST_PARIO_SRCS = minitest-pario.c $(PARIO_SRCS) $(BASE_SRCS)
$(eval $(call make-program,minitest-pario,$(MINITEST_PARIO_SRCS)))

# minitest-timer
MINITEST_TIMER_SRCS = minitest-timer.c $(TIMER_SRCS) $(BASE_SRCS)
$(eval $(call make-program,minitest-timer,$(MINITEST_TIMER_SRCS)))

# minitest-udp
MINITEST_UDP_SRCS = minitest-udp.c $(BASE_SRCS)
$(eval $(call make-program,minitest-udp,$(MINITEST_UDP_SRCS)))

# minitest-proto-env
MINITEST_PROTO_ENV_SRCS = minitest-proto-env.c $(PROTO_ENV_SRCS) $(BASE_SRCS)
$(eval $(call make-program,minitest-proto-env,$(MINITEST_PROTO_ENV_SRCS)))

# minitest-proto-atom
MINITEST_PROTO_ATOM_SRCS = minitest-proto-atom.c $(PROTO_ATOM_SRCS) $(BASE_SRCS)
$(eval $(call make-program,minitest-proto-atom,$(MINITEST_PROTO_ATOM_SRCS)))

# minitest-proto-dev
MINITEST_PROTO_DEV_SRCS = minitest-proto-dev.c $(PROTO_DEV_SRCS) $(BASE_SRCS)
$(eval $(call make-program,minitest-proto-dev,$(MINITEST_PROTO_DEV_SRCS)))

# minitest-proto-io
MINITEST_PROTO_IO_SRCS = minitest-proto-io.c $(PROTO_IO_SRCS) $(BASE_SRCS)
$(eval $(call make-program,minitest-proto-io,$(MINITEST_PROTO_IO_SRCS)))

# minitest-proto-boot
MINITEST_PROTO_BOOT_SRCS = minitest-proto-boot.c $(PROTO_BOOT_SRCS) $(BASE_SRCS)
$(eval $(call make-program,minitest-proto-boot,$(MINITEST_PROTO_BOOT_SRCS)))

# minitest-pamela
MINITEST_PAMELA_SRCS = minitest-pamela.c $(PAMELA_SRCS) $(BASE_SRCS)
$(eval $(call make-program,minitest-pamela,$(MINITEST_PAMELA_SRCS)))

# ----- tests
# test-proto-atom
TEST_PROTO_ATOM_SRCS = test-proto-atom.c $(PROTO_ATOM_SRCS) $(TEST_SRCS)
TEST_PROTO_ATOM_SRCS += proto-atom-testsuite.c
TEST_PROTO_ATOM_SRCS += proto-env-testsuite.c
$(eval $(call make-program,test-proto-atom,$(TEST_PROTO_ATOM_SRCS)))
$(eval $(call dist-program,test-proto-atom))

# test-proto-dev
TEST_PROTO_DEV_SRCS = test-proto-dev.c $(PROTO_DEV_SRCS) $(TEST_SRCS)
TEST_PROTO_DEV_SRCS += proto-dev-testsuite.c
$(eval $(call make-program,test-proto-dev,$(TEST_PROTO_DEV_SRCS)))
$(eval $(call dist-program,test-proto-dev))

# test-proto-io
TEST_PROTO_IO_SRCS = test-proto-io.c $(PROTO_IO_SRCS) $(TEST_SRCS)
TEST_PROTO_IO_SRCS += proto-io-testsuite.c
$(eval $(call make-program,test-proto-io,$(TEST_PROTO_IO_SRCS)))
$(eval $(call dist-program,test-proto-io))

# test-proto-boot
TEST_PROTO_BOOT_SRCS = test-proto-boot.c $(PROTO_BOOT_SRCS) $(TEST_SRCS)
TEST_PROTO_BOOT_SRCS += proto-boot-testsuite.c
$(eval $(call make-program,test-proto-boot,$(TEST_PROTO_BOOT_SRCS)))
$(eval $(call dist-program,test-proto-boot))

# ----- tools
# pablo
PABLO_SRCS = pablo.c pblfile.c bootloader.c machtag.c fwid.c $(PROTO_BOOT_SRCS)
$(eval $(call make-program,pablo,$(PABLO_SRCS)))
$(eval $(call dist-program,pablo))

# disabled for now
ifeq "a" "b"

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

# pario simulator
ifeq "$(CONFIG_MACH)" "net"
PARIO_SIM_SRCS = pario_sim.c sim_msg.c sim_pkt.c udp.c timer.c
$(eval $(call make-program,pario_sim,$(PARIO_SIM_SRCS)))
$(eval $(call dist-program,pario_sim))

PARIO_SIM_TEST_SRCS = pario_sim_test.c sim_msg.c
$(eval $(call make-program,pario_sim_test,$(PARIO_SIM_TEST_SRCS)))
$(eval $(call dist-program,pario_sim_test))

SIM_PKT_TEST_SRCS = sim_pkt_test.c sim_pkt.c udp.c timer.c
$(eval $(call make-program,sim_pkt_test,$(SIM_PKT_TEST_SRCS)))
$(eval $(call dist-program,sim_pkt_test))
endif

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

endif

# rules
include scripts/common-rules.mk

gcc:
	$(MAKE) COMPILER=gcc
