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
BUILD_DIR:=$(BUILD_BASE_DIR)/$(BUILD_TAG)

# derived config files
CONFIG_H_FILE=$(BUILD_DIR)/autoconf.h
CONFIG_MAKE_FILE=$(BUILD_DIR)/conf.mk

# all config files
CONFIG_FILES=$(CONFIG_H_FILE) $(CONFIG_MAKE_FILE)

# create build dir
ifneq "$(MAKECMDGOALS)" "clean"
create_build_dir := $(shell test -d $(BUILD_DIR) || mkdir -p $(BUILD_DIR))
create_config_h := $(shell test -f $(CONFIG_H_FILE) || $(GENCONFIG) -c $(CONFIG_H_FILE) $(CONFIG))
create_config_make := $(shell test -f $(CONFIG_MAKE_FILE) || $(GENCONFIG) -k $(CONFIG_MAKE_FILE) $(CONFIG))
endif

# toggle verbose
ifndef VERBOSE
H=@
export H
endif

# default rule
all:

# re-generate configs
$(CONFIG_H_FILE): $(CONFIG) $(GENCONFIG)
	$(H)$(GENCONFIG) -c $@ $(CONFIG)

$(CONFIG_MAKE_FILE): $(CONFIG) $(GENCONFIG)
	$(H)$(GENCONFIG) -k $@ $(CONFIG)

include $(CONFIG_MAKE_FILE)
