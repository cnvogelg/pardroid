# all rule

# --- cmake ---
ifeq "$(CONFIG_MAKESYS)" "cmake"

all: info cmake-setup cmake-build

include scripts/cmake-rules.mk

# --- make ---
else

all: info $(FIRMWARES)

endif

help:
	@echo "Global Rules:"
	@echo
	@echo "  all         make all firmwares"
	@echo "  info        show current setup"
	@echo "  release     release build"
	@echo
	@echo "  clean       clean current firmware"
	@echo "  clean-all   clean build and dist dirs"
	@echo
	@echo "  install     install all files"
	@echo
	@echo "  dist        dist build of current firmware"
	@echo "  dist-all    dist build of all firmwares"
	@echo
	@echo "Build Firmware:"
	@echo
	@for a in $(sort $(FIRMWARES)) ; do echo "  $$a" ; done
	@echo
	@echo "  *-sym       show symbols of firmware"
	@echo "  *-code      show code size of firmware"
	@echo "  *-check     check size"
	@echo "  *-prog      program/flash firmware"
	@echo
	@echo "Flags:  make <flag>=<value>"
	@echo
	@echo "CONFIG:"
	@for a in $(sort $(ALL_CONFIGS)) ; do echo "  $$a" ; done
	@echo "FLAVOR:"
	@for a in $(sort $(ALL_FLAVORS)) ; do echo "  $$a" ; done

info:
	@echo "--- flavor=$(FLAVOR) arch=$(CONFIG_ARCH) mach=$(CONFIG_MACH) ---"

release:
	@$(MAKE) FLAVOR=release

sym: $(patsubst %,%-sym,$(FIRMWARES))

code: $(patsubst %,%-code,$(FIRMWARES))

check: $(patsubst %,%-check,$(FIRMWARES))

prog: info $(DEFAULT_FIRMWARE)-prog

prog-full: $(DEFAULT_FIRMWARE)-prog-full

pablo: $(DEFAULT_FIRMWARE)-pablo

clean:
	$(H)rm -rf $(BUILD_DIR)

clean-all:
	$(H)rm -rf $(BUILD_BASE_DIR) $(DIST_BASE_DIR)

# install files
INSTALL_DIR ?= ../install

install: $(PBL_FILES)
	@if [ ! -d "$(INSTALL_DIR)" ]; then \
		echo "No INSTALL_DIR='$(INSTALL_DIR)' found!" ; \
		exit 1 ; \
	fi
	@echo "  INSTALL  $(PBL_FILES)"
	@cp $(PBL_FILES) $(INSTALL_DIR)/

# distribution
dist: $(DIST_FILES)

# for all configs
all-configs:
	@for a in $(ALL_CONFIGS) ; do \
		echo "--- config=$$a ---" ; \
		$(MAKE) CONFIG=$$a || exit 1 ; \
	done

all-flavors:
	@for a in $(ALL_FLAVORS) ; do \
		echo "--- flavor=$$a ---" ; \
		$(MAKE) FLAVOR=$$a all-configs || exit 1 ; \
	done

dist-all:
	@for a in $(ALL_FLAVORS) ; do \
		echo "--- flavor=$$a ---" ; \
		for b in $(ALL_CONFIGS) ; do \
			echo "--- config=$$b ---" ; \
			$(MAKE) FLAVOR=$$a CONFIG=$$b dist || exit 1 ; \
		done \
	done

# final hex (flash) file from elf
%.hex: %.elf
	@echo "  HEX  $(@F)"
	$(H)$(OBJCOPY) -O $(CONFIG_FLASH_FORMAT) -j .data -j .text $< $@

# final hex (flash) file from elf
%.bin: %.elf
	@echo "  BIN  $(@F)"
	$(H)$(OBJCOPY) -O binary -j .data -j .text $< $@

# generate pablo flash image
%.img: %.bin
	@echo "  IMG  $(@F)"
	$(H)scripts/pblgen.py $< $(CONFIG_MAX_ROM) $@

# generate pablo flash image
%.pbl: %.img
	@echo "  PBL  $(@F)"
	$(H)scripts/pblfile.py $< $(CONFIG_MAX_ROM) $@

.PRECIOUS: %.img %.bin

# finale eeprom file from elf
%.eep: %.elf
	@echo "  EEP  $(@F)"
	$(H)$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# extended listing file
%.lss: %.elf
	@echo "  LSS  $(@F)"
	$(H)$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo "  SYM  $(@F)"
	$(H)$(NM) -n $< > $@

# symbol size
%.sym_size: %.elf
	@echo "  SYM  $(@F)"
	$(H)$(NM) --radix=d --size-sort --print-size $< | egrep ' [bBdD] ' > $@

# code size
%.code_size: %.elf
	@echo "  CSI  $(@F)"
	$(H)$(NM) --radix=d --size-sort --print-size $< | egrep ' [tT] ' > $@

# compile
$(OBJ_DIR)/%.o : %.c
	@echo "  CC   $<"
	$(H)$(CC) -c $(CFLAGS) $< -o $@

# assemble
$(OBJ_DIR)/%.o : %.S
	@echo "  ASM  $<"
	$(H)$(CC) -c $(ASFLAGS) $< -o $@

# include dependencies
ifeq "$(filter clean clean-all,$(MAKECMDGOALS))" ""
-include $(shell mkdir -p $(DEP_DIR) 2>/dev/null) $(wildcard $(DEP_DIR)/*.d)
endif
