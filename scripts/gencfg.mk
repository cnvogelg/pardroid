# read config at get build dir
GENCONFIG=$(BASE_DIR)/scripts/genconfig.py
CONF_VALS:=$(shell $(GENCONFIG) -bam $(CONFIG))
BUILD_TAG=$(word 1,$(CONF_VALS))
ARCH_DIR=$(word 2,$(CONF_VALS))
MACH_DIR=$(word 3,$(CONF_VALS))

# invalid config - abort
ifeq "$(BUILD_TAG)" "INVALID"
.PHONY: invalid_conf
invalid_conf:
	@$(GENCONFIG) $(CONFIG)
endif

BUILD_BASE_DIR?=BUILD
BUILD_DIR=$(BUILD_BASE_DIR)/$(BUILD_TAG)

export BASE_DIR
export BUILD_TAG
export BUILD_DIR
export ARCH_DIR
export MACH_DIR

# derived config files
CONFIG_H_FILE=$(BUILD_DIR)/autoconf.h
export CONFIG_H_FILE
CONFIG_MAKE_FILE=$(BUILD_DIR)/conf.mk
export CONFIG_MAKE_FILE

# all config files
CONFIG_FILES=$(CONFIG_H_FILE) $(CONFIG_MAKE_FILE)

ifndef VERBOSE
H=@
export H
endif

all: $(CONFIG_FILES)
	$(H)$(MAKE) -f main.mk all

.DEFAULT: $(CONFIG_FILES)
	$(H)$(MAKE) -f main.mk $@

# generate configs
$(CONFIG_H_FILE): $(BUILD_DIR) $(CONFIG) $(GENCONFIG)
	$(H)$(GENCONFIG) -c $@ $(CONFIG)

$(CONFIG_MAKE_FILE): $(BUILD_DIR) $(CONFIG) $(GENCONFIG)
	$(H)$(GENCONFIG) -k $@ $(CONFIG)

$(BUILD_DIR):
	$(H)mkdir -p $(BUILD_DIR)
