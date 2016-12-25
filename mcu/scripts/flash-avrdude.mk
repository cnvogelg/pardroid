ifeq "$(OS)" "Darwin"
LDR_PORT ?= $(shell ls /dev/cu.usbserial-* | tail -n 1)
else
LDR_PORT ?= $(shell ls /dev/ttyUSB* | tail -n 1)
endif

# commands
AVRDUDE ?= avrdude
AVRDUDE_WRITE_FLASH  = -U flash:w:$(TARGET_HEX)
AVRDUDE_WRITE_FUSE   = -U lfuse:w:$(CONFIG_AVRDUDE_LFUSE):m -U hfuse:w:$(CONFIG_AVRDUDE_HFUSE):m

# combine flags
AVRDUDE_LDR_FLAGS += -p $(CONFIG_AVRDUDE_MCU) -c $(CONFIG_AVRDUDE_PROGRAMMER)
ifdef AVRDUDE_DEBUG
AVRDUDE_LDR_FLAGS += -v -v -v -v
endif

check_prog:
	$(AVRDUDE) $(AVRDUDE_LDR_FLAGS) -U signature:r:sig.txt:h
	@echo -n " device signature: "
	@cat sig.txt
	@rm -f sig.txt

prog: $(TARGET_HEX) size
	@echo "  --- programming flash ---"
	$(AVRDUDE) $(AVRDUDE_LDR_FLAGS) $(AVRDUDE_WRITE_FLASH)

read_fuse:
	$(AVRDUDE) $(AVRDUDE_LDR_FLAGS) -U lfuse:r:lfuse.txt:h -U hfuse:r:hfuse.txt:h
	@echo -n " lfuse: "
	@cat lfuse.txt
	@echo -n " hfuse: "
	@cat hfuse.txt
	@rm -f lfuse.txt hfuse.txt
