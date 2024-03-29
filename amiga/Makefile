BASE_DIR=..
# config
DEFAULT_CONFIG=m68k-cia
CONFIG?=configs/$(DEFAULT_CONFIG).config
EXTRA_CONFIG=$(wildcard configs/$(FLAVOR).config)
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
INCLUDES+=../common/include include
INCLUDES+=src/device src/pamdev src/pamlib src/tools src/paloma
VPATH=$(INCLUDES) ../common/src

# base
BASE_SRCS = debug.c worker.c
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
PAMELA_ENGINE_SRCS = $(PAMELA_SRCS) pamela_engine.c pamela_req.c pamela_sock.c

# pamlib
PAMLIB_SRCS = pamlib.c pamela_err.c
PAMLIB_REQ_SRCS = $(PAMLIB_SRCS) pamlib_req.c

# paloma
PALOMA_SRCS = paloma_lib.c

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

# minitest-worker
MINITEST_WORKER_SRCS = minitest-worker.c $(BASE_SRCS)
$(eval $(call make-program,minitest-worker,$(MINITEST_WORKER_SRCS)))

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

# minitest-pamela-engine
MINITEST_PAMELA_ENGINE_SRCS = minitest-pamela-engine.c $(PAMELA_ENGINE_SRCS) $(BASE_SRCS)
$(eval $(call make-program,minitest-pamela-engine,$(MINITEST_PAMELA_ENGINE_SRCS)))

# minitest-pamdev
MINITEST_PAMDEV_SRCS = minitest-pamdev.c $(BASE_SRCS)
$(eval $(call make-program,minitest-pamdev,$(MINITEST_PAMDEV_SRCS)))

# minitest-pamlib
MINITEST_PAMLIB_SRCS = minitest-pamlib.c $(BASE_SRCS) $(PAMLIB_SRCS)
$(eval $(call make-program,minitest-pamlib,$(MINITEST_PAMLIB_SRCS)))

# minitest-pamlib_req
MINITEST_PAMLIB_REQ_SRCS = minitest-pamlib_req.c $(BASE_SRCS) $(PAMLIB_REQ_SRCS)
$(eval $(call make-program,minitest-pamlib_req,$(MINITEST_PAMLIB_REQ_SRCS)))

# minitest-paloma
MINITEST_PALOMA_SRCS = minitest-paloma.c $(BASE_SRCS) $(PAMLIB_SRCS) $(PALOMA_SRCS)
$(eval $(call make-program,minitest-paloma,$(MINITEST_PALOMA_SRCS)))

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

# test-pamela
TEST_PAMELA_SRCS = test-pamela.c $(PAMELA_SRCS) $(TEST_SRCS)
TEST_PAMELA_SRCS += pamela-testsuite.c
$(eval $(call make-program,test-pamela,$(TEST_PAMELA_SRCS)))
$(eval $(call dist-program,test-pamela))

# test-pamela-engine
TEST_PAMELA_ENGINE_SRCS = test-pamela-engine.c $(PAMELA_ENGINE_SRCS) $(TEST_SRCS)
TEST_PAMELA_ENGINE_SRCS += pamela-engine-testsuite.c
$(eval $(call make-program,test-pamela-engine,$(TEST_PAMELA_ENGINE_SRCS)))
$(eval $(call dist-program,test-pamela-engine))

# test-pamlib
TEST_PAMLIB_SRCS = test-pamlib.c $(PAMLIB_SRCS) $(TEST_SRCS)
TEST_PAMLIB_SRCS += pamlib-testsuite.c
$(eval $(call make-program,test-pamlib,$(TEST_PAMLIB_SRCS)))
$(eval $(call dist-program,test-pamlib))

# test-pamlib_req
TEST_PAMLIB_REQ_SRCS = test-pamlib_req.c $(PAMLIB_REQ_SRCS) $(TEST_SRCS)
TEST_PAMLIB_REQ_SRCS += pamlib_req-testsuite.c
$(eval $(call make-program,test-pamlib_req,$(TEST_PAMLIB_REQ_SRCS)))
$(eval $(call dist-program,test-pamlib_req))

# test-paloma
TEST_PALOMA_SRCS = test-paloma.c $(PAMLIB_SRCS) $(PALOMA_SRCS) $(TEST_SRCS)
TEST_PALOMA_SRCS += paloma-testsuite.c
$(eval $(call make-program,test-paloma,$(TEST_PALOMA_SRCS)))
$(eval $(call dist-program,test-paloma))

# ----- tools
# pablo
PABLO_SRCS = pablo.c pblfile.c bootloader.c machtag.c fwid.c $(PROTO_BOOT_SRCS)
$(eval $(call make-program,pablo,$(PABLO_SRCS)))
$(eval $(call dist-program,pablo))

# pam-info
PAM_INFO_SRCS = pam-info.c $(PAMLIB_SRCS)
$(eval $(call make-program,pam-info,$(PAM_INFO_SRCS)))
$(eval $(call dist-program,pam-info))

# ----- devices
# pamela.device
PAMELA_DEV_SRCS = pamela_dev.c $(PAMELA_ENGINE_SRCS)
$(eval $(call make-device,pamela.device,$(PAMELA_DEV_SRCS)))
$(eval $(call dist-device,pamela.device))

# ----- bench
# bench-proto-io
BENCH_PROTO_IO_SRCS = bench-proto-io.c $(PROTO_IO_SRCS) $(BENCH_SRCS)
$(eval $(call make-program,bench-proto-io,$(BENCH_PROTO_IO_SRCS)))
$(eval $(call dist-program,bench-proto-io))

# bench-pamela
BENCH_PAMELA_SRCS = bench-pamela.c $(PAMELA_SRCS) $(BENCH_SRCS)
$(eval $(call make-program,bench-pamela,$(BENCH_PAMELA_SRCS)))
$(eval $(call dist-program,bench-pamela))

# disabled for now
ifeq "a" "b"

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

endif

# rules
include scripts/common-rules.mk

gcc:
	$(MAKE) COMPILER=gcc
