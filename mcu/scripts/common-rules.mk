# all rule
all: info $(FIRMWARES)

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
	@echo "  IMG  $(@F)  id=$(call get-fw-id,$(@F))  ver=$(call get-fw-ver,$(@F))"
	$(H)scripts/pblgen.py $< $(CONFIG_MAX_ROM) $(MACHTAG_ID) $(call get-fw-id,$(@F)) $(call get-fw-ver,$(@F)) $@

# generate pablo flash image
%.pbl: %.img
	@echo "  PBL  $(@F)  id=$(call get-fw-id,$(@F))  ver=$(call get-fw-ver,$(@F))"
	$(H)scripts/pblfile.py $< $(CONFIG_MAX_ROM) $(MACHTAG_ID) $(call get-fw-id,$(@F)) $(call get-fw-ver,$(@F)) $@

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
	$(H)$(NM) --size-sort --print-size $< | egrep ' [bBdD] ' > $@

# code size
%.code_size: %.elf
	@echo "  CSI  $(@F)"
	$(H)$(NM) --size-sort --print-size $< | egrep ' [tT] ' > $@

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
