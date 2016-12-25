all: $(TARGET)

clean:
	$(H)rm -rf $(BUILD_DIR)

$(TARGET): $(OBJDIR) $(OBJ) $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).lss $(BUILD_DIR)/$(TARGET).sym

$(OBJDIR):
	$(H)mkdir -p $(OBJDIR)

$(BUILD_DIR):
	$(H)mkdir -p $(BUILD_DIR)

# final hex (flash) file from elf
%.hex: %.elf
	@echo "  HEX  $@"
	$(H)$(OBJCOPY) -O $(FORMAT) -j .data -j .text $< $@

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
	$(H)$(CC) -c $(CFLAGS) $(CFLAGS_LOCAL) $< -o $@

# include dependencies
-include $(shell mkdir -p $(DEPDIR) 2>/dev/null) $(wildcard $(DEPDIR)/*.d)
