all: $(TARGET)

clean:
	$(H)rm -rf $(BUILD_DIR)

$(TARGET): $(TARGET_BIN)

$(TARGET_BIN): $(OBJ)
	@echo "  LD   $@"
	$(H)$(CC) $(LDFLAGS) -o $@ $+

$(OBJ): $(OBJDIR)

$(OBJDIR):
	$(H)mkdir -p $(OBJDIR)

$(BUILD_DIR):
	$(H)mkdir -p $(BUILD_DIR)

# compile
$(OBJDIR)/%.o : %.c
	@echo "  CC   $<"
	$(H)$(CC) -c $(CFLAGS) $< -o $@

# assemble
$(OBJDIR)/%.o : %.$(ASM_SUFFIX)
	@echo "  ASM  $<"
	$(H)$(CC) -c $(ASFLAGS) $< -o $@
