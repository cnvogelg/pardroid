# all rule
all: $(FIRMWARES)

size: $(patsubst %,%-size,$(FIRMWARES))

check: $(patsubst %,%-check,$(FIRMWARES))

prog: $(DEFAULT_FIRMWARE)-prog

clean:
	$(H)rm -rf $(BUILD_DIR)

# size checking
%.size_code: %.elf
	@echo "  SIZE $(@F)"
	$(H)$(SIZE) -C $< | grep Program | awk '{ print $$2 }' > $@

%.size_data: %.elf
	@echo "  SIZE $(@F)"
	$(H)$(SIZE) -C $< | grep Data | awk '{ print $$2 }' > $@

%.size_sym: %.elf
	@echo "  SIZE $(@F)"
	$(H)$(NM) --size-sort --print-size $< | egrep ' [bBdD] ' > $@


# final hex (flash) file from elf
%.hex: %.elf
	@echo "  HEX  $(@F)"
	$(H)$(OBJCOPY) -O $(CONFIG_FLASH_FORMAT) -j .data -j .text $< $@

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
