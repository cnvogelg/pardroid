all: $(PROGRAMS)

clean:
	$(H)rm -rf $(BUILD_DIR)

clean-all:
	$(H)rm -rf $(BUILD_BASE_DIR)

# distribution
dist: $(DIST_FILES)

# compile
$(OBJ_DIR)/%.o : %.c
	@echo "  CC   $<"
	$(H)$(CC) -c $(CFLAGS) $< -o $@

# assemble
$(OBJ_DIR)/%.o : %.s
	@echo "  ASM  $<"
	$(H)$(CC) -c $(ASFLAGS) $< -o $@
