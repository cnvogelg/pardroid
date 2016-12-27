all: $(TARGET)

clean:
	$(H)rm -rf $(BUILD_DIR)

$(TARGET): $(OBJDIR) $(OBJ) $(TARGET_ELF) $(TARGET_LSS) $(TARGET_SYM)

$(OBJDIR):
	$(H)mkdir -p $(OBJDIR)

$(BUILD_DIR):
	$(H)mkdir -p $(BUILD_DIR)

# size checking
size: size_code size_data

size_code: $(TARGET_ELF)
	$(H)SIZE=`$(SIZE) -C $< | grep Program | awk '{ print $$2 }'` ; \
	if [ $$SIZE -gt $(CONFIG_MAX_ROM) ] ; then \
		echo "  $$SIZE >  $(CONFIG_MAX_ROM) bytes: code TOO LARGE" ; exit 1 ; \
	else \
		echo "  $$SIZE <= $(CONFIG_MAX_ROM) bytes: code ok" ; \
	fi

size_data: $(TARGET_ELF)
	$(H)SIZE=`$(SIZE) -C $< | fgrep Data | awk '{ print $$2 }'` ; \
	if [ $$SIZE -gt $(CONFIG_MAX_RAM) ] ; then \
		echo "  $$SIZE >  $(CONFIG_MAX_RAM) bytes: sram TOO LARGE" ; exit 1 ; \
	else \
		echo "  $$SIZE <= $(CONFIG_MAX_RAM) bytes: sram ok" ; \
	fi

size_symbols: $(TARGET_ELF)
	$(H)$(NM) --size-sort --print-size $< | egrep ' [bBdD] '


# final hex (flash) file from elf
%.hex: %.elf
	@echo "  HEX  $@"
	$(H)$(OBJCOPY) -O $(CONFIG_FLASH_FORMAT) -j .data -j .text $< $@

# finale eeprom file from elf
%.eep: %.elf
	@echo "  EEP  $@"
	$(H)$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# extended listing file
%.lss: %.elf
	@echo "  LSS  $@"
	$(H)$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo "  SYM  $@"
	$(H)$(NM) -n $< > $@

# link
%.elf: $(OBJ)
	@echo "  LD   $@"
	$(H)$(CC) $(CFLAGS) $(OBJ) --output $@ $(LDFLAGS)

# compile
$(OBJDIR)/%.o : %.c
	@echo "  CC   $<"
	$(H)$(CC) -c $(CFLAGS) $< -o $@

# assemble
$(OBJDIR)/%.o : %.$(ASM_SUFFIX)
	@echo "  ASM  $<"
	$(H)$(CC) -c $(ASFLAGS) $< -o $@

# include dependencies
-include $(shell mkdir -p $(DEPDIR) 2>/dev/null) $(wildcard $(DEPDIR)/*.d)
