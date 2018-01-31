# read config at get build dir
ALL_CONFIG_FILES=$(CONFIG) $(EXTRA_CONFIG)
GENCONFIG=$(BASE_DIR)/scripts/genconfig.py
CONFIG_BASE=$(notdir $(basename $(CONFIG)))
CONF_VALS:=$(shell $(GENCONFIG) -bam $(ALL_CONFIG_FILES))
BUILD_TAG=$(word 1,$(CONF_VALS))
ARCH_DIR=$(word 2,$(CONF_VALS))
MACH_DIR=$(word 3,$(CONF_VALS))

# invalid config - abort
ifeq "$(BUILD_TAG)" "INVALID"
.PHONY: invalid_conf
invalid_conf:
	@$(GENCONFIG) $(ALL_CONFIG_FILES)
endif

# flavor setup
FLAVOR?=debug
ALL_FLAVORS=debug release

BUILD_BASE_DIR?=BUILD
BUILD_DIR:=$(BUILD_BASE_DIR)/$(FLAVOR)/$(CONFIG_BASE)

DIST_BASE_DIR?=DIST
DIST_DIR:=$(DIST_BASE_DIR)/$(FLAVOR)/$(CONFIG_BASE)

# dist tag
#DIST_TAG=-$(CONFIG_BASE)-$(VERSION_MAJOR).$(VERSION_MINOR)

# derived config files
CONFIG_H_FILE=$(BUILD_DIR)/autoconf.h
CONFIG_MAKE_FILE=$(BUILD_DIR)/conf.mk

# all config files
CONFIG_FILES=$(CONFIG_H_FILE) $(CONFIG_MAKE_FILE)

# create build dir
ifeq "$(filter clean clean-all,$(MAKECMDGOALS))" ""
create_build_dir := $(shell test -d $(BUILD_DIR) || mkdir -p $(BUILD_DIR))
create_config_h := $(shell test -f $(CONFIG_H_FILE) || $(GENCONFIG) -c $(CONFIG_H_FILE) $(ALL_CONFIG_FILES))
create_config_make := $(shell test -f $(CONFIG_MAKE_FILE) || $(GENCONFIG) -k $(CONFIG_MAKE_FILE) $(ALL_CONFIG_FILES))
endif

# toggle verbose
ifndef VERBOSE
H=@
export H
endif

# default rule
all:

# re-generate configs
$(CONFIG_H_FILE): $(ALL_CONFIG_FILES) $(GENCONFIG)
	$(H)$(GENCONFIG) -c $@ $(ALL_CONFIG_FILES)

$(CONFIG_MAKE_FILE): $(ALL_CONFIG_FILES) $(GENCONFIG)
	$(H)$(GENCONFIG) -k $@ $(ALL_CONFIG_FILES)

include $(CONFIG_MAKE_FILE)
