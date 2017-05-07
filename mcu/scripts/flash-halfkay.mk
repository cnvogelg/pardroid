# halfkay is the teensy bootloader

HALFKAY_CLI ?= teensy_loader_cli
HALFKAY_FLAGS = -v -w --mcu=$(CONFIG_HALFKAY_MCU)

# prog rule for firmware
# $1 = firmware.hex file
# $2 = short name
define prog-firmware
	@echo "  PROG  $2"
	$(H)$(HALFKAY_CLI) $(HALFKAY_FLAGS) $1
endef
