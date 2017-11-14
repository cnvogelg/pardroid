
TEENSY_LOADER_CLI = teensy_loader_cli
TEENSY_LOADER_FLAGS = --mcu=$(CONFIG_MCU) -w -v

# prog rule for firmware
# $1 = firmware.hex file
# $2 = short name
define prog-firmware
	@echo "  PROG  $2"
	$(H)$(TEENSY_LOADER_CLI) $(TEENSY_LOADER_FLAGS) $1
endef
