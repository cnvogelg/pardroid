OS := $(shell uname -s)
ifeq "$(OS)" "Darwin"
LDR_PORT ?= $(shell ls /dev/cu.usbserial-* | tail -n 1)
else
LDR_PORT ?= $(shell ls /dev/ttyUSB* | tail -n 1)
endif

# commands
AVRDUDE ?= avrdude
AVRDUDE_WRITE_FLASH  = -U flash:w:$1
AVRDUDE_WRITE_FUSE   = -U lfuse:w:$(CONFIG_AVRDUDE_LFUSE):m -U hfuse:w:$(CONFIG_AVRDUDE_HFUSE):m
AVRDUDE_LOCK_BOOTLOADER = -U lock:w:0x0F:m

# combine flags
AVRDUDE_FLAGS += -p $(CONFIG_AVRDUDE_MCU) -c $(CONFIG_AVRDUDE_PROGRAMMER)
ifeq "$(CONFIG_AVRDUDE_HAS_PORT)" "y"
AVRDUDE_FLAGS += -P $(LDR_PORT) -b $(CONFIG_AVRDUDE_BAUD)
endif
ifdef AVRDUDE_DEBUG
AVRDUDE_FLAGS += -v -v -v -v
endif

read-sig:
	$(H)$(AVRDUDE) $(AVRDUDE_FLAGS) -U signature:r:sig.txt:h
	@echo -n " device signature: "
	@cat sig.txt
	@rm -f sig.txt

read-fuse:
	$(H)$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:r:lfuse.txt:h -U hfuse:r:hfuse.txt:h -U efuse:r:efuse.txt:h
	@echo -n " lfuse: "
	@cat lfuse.txt
	@echo -n " hfuse: "
	@cat hfuse.txt
	@echo -n " efuse: "
	@cat efuse.txt
	@rm -f lfuse.txt hfuse.txt efuse.txt

read-flash:
	$(H)$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:r:flash.hex:i

# prog rule for firmware
# $1 = firmware.hex file
# $2 = short name
define prog-firmware
	@echo "  PROG  $2"
	$(H)$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)
endef

# prog rule for firmware
# $1 = firmware.hex file
# $2 = short name
define prog-bootloader
	@echo "  PROG  $2"
	$(H)$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH) $(AVRDUDE_LOCK_BOOTLOADER)
endef
