all: $(PROGRAMS)

clean:
	$(H)rm -rf $(BUILD_DIR)

# compile
$(OBJ_DIR)/%.o : %.c
	@echo "  CC   $<"
	$(H)$(CC) -c $(CFLAGS) $< -o $@

# assemble
$(OBJ_DIR)/%.o : %.s
	@echo "  ASM  $<"
	$(H)$(CC) -c $(ASFLAGS) $< -o $@
