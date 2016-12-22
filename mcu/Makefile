# read config at get build dir
CONFIG?=configs/ardunano.config
GENCONFIG=../scripts/genconfig.py
BUILD_BASE_DIR=BUILD
CONF_VALS:=$(shell $(GENCONFIG) -bam -B $(BUILD_BASE_DIR) $(CONFIG))
BUILD_DIR=$(word 1,$(CONF_VALS))
ARCH_DIR=$(word 2,$(CONF_VALS))
MACH_DIR=$(word 3,$(CONF_VALS))
ifeq "$(BUILD_DIR)" "INVALID"
.PHONY: invalid_conf
invalid_conf:
	@$(GENCONFIG) $(CONFIG)
endif

export ARCH_DIR
export MACH_DIR

# derived config files
CONFIG_H_FILE=$(BUILD_DIR)/autoconf.h
CONFIG_MAKE_FILE=$(BUILD_DIR)/conf.make
CONFIG_FILES=$(CONFIG_H_FILE) $(CONFIG_MAKE_FILE)

ifndef VERBOSE
H=@
endif

# --- rules ---

all: $(CONFIG_FILES)
	$(H)make -C src BUILD_DIR=../$(BUILD_DIR)

clean:
	$(H)rm -rf $(BUILD_DIR)

# generate configs
$(CONFIG_H_FILE): $(BUILD_DIR) $(CONFIG) $(GENCONFIG)
	$(H)$(GENCONFIG) -c $@ $(CONFIG)

$(CONFIG_MAKE_FILE): $(BUILD_DIR) $(CONFIG) $(GENCONFIG)
	$(H)$(GENCONFIG) -k $@ $(CONFIG)

$(BUILD_DIR):
	$(H)mkdir -p $(BUILD_DIR)
