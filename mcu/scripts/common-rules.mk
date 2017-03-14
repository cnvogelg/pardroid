# all rule
all: $(FIRMWARES)

sym: $(patsubst %,%-sym,$(FIRMWARES))

code: $(patsubst %,%-code,$(FIRMWARES))

check: $(patsubst %,%-check,$(FIRMWARES))

prog: $(DEFAULT_FIRMWARE)-prog

pablo: $(DEFAULT_FIRMWARE)-pablo

clean:
	$(H)rm -rf $(BUILD_DIR)

# final hex (flash) file from elf
%.hex: %.elf
	@echo "  HEX  $(@F)"
	$(H)$(OBJCOPY) -O $(CONFIG_FLASH_FORMAT) -j .data -j .text $< $@

# final hex (flash) file from elf
%.bin: %.elf
	@echo "  BIN  $(@F)"
	$(H)$(OBJCOPY) -O binary -j .data -j .text $< $@

# generate pablo flash image
%.pbl: %.bin
	@echo "  PBL  $(@F)"
	$(H)scripts/pblgen.py $< $(CONFIG_MAX_ROM) $(MACH_TAG) $(VERSION_TAG) $@

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
ifneq "$(MAKECMDGOALS)" "clean"
-include $(shell mkdir -p $(DEP_DIR) 2>/dev/null) $(wildcard $(DEP_DIR)/*.d)
endif
