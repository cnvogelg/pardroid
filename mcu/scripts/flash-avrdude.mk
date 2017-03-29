# flash-avrdude.mk
# rules for flashing with avrdude

# detect (single) serial port on system
OS := $(shell uname -s)
ifeq "$(OS)" "Darwin"
SERIAL_PORT ?= $(shell ls /dev/cu.usbserial-* | tail -n 1)
else
SERIAL_PORT ?= $(shell ls /dev/ttyUSB* | tail -n 1)
endif

# avrdude binary
AVRDUDE ?= avrdude

# write flash: $1 = hex file
AVRDUDE_WRITE_FLASH  = -U flash:w:$1

# set fuses
ifneq "$(CONFIG_AVRDUDE_LFUSE)" ""
AVRDUDE_WRITE_FUSE += -U lfuse:w:$(CONFIG_AVRDUDE_LFUSE):m
endif
ifneq "$(CONFIG_AVRDUDE_HFUSE)" ""
AVRDUDE_WRITE_FUSE += -U hfuse:w:$(CONFIG_AVRDUDE_HFUSE):m
endif
ifneq "$(CONFIG_AVRDUDE_EFUSE)" ""
AVRDUDE_WRITE_FUSE += -U efuse:w:$(CONFIG_AVRDUDE_EFUSE):m
endif

# lock byte
ifneq "$(CONFIG_AVRDUDE_LOCK_BYTE)" ""
AVRDUDE_LOCK_BOOTLOADER = -U lock:w:$(CONFIG_AVRDUDE_LOCK_BYTE):m
endif

# setup base parameters of avrdude:
# device (required)
AVRDUDE_FLAGS += -p $(CONFIG_AVRDUDE_MCU)
# programmer (required)
AVRDUDE_FLAGS += -c $(CONFIG_AVRDUDE_PROGRAMMER)
# port (optional)
ifeq "$(CONFIG_AVRDUDE_PORT)" "auto"
AVRDUDE_FLAGS += -P $(SERIAL_PORT)
else
ifneq "$(CONFIG_AVRDUDE_PORT)" ""
AVRDUDE_FLAGS += -P $(CONFIG_AVRDUDE_PORT)
endif
endif
# baud (optional)
ifneq "$(CONFIG_AVRDUDE_BAUD)" ""
AVRDUDE_FLAGS += -b $(CONFIG_AVRDUDE_BAUD)
endif
# debug (optional)
ifeq "$(CONFIG_AVRDUDE_DEBUG)" "1"
AVRDUDE_FLAGS += -v -v -v -v
else
AVRDUDE_QUIET = -q
endif

read-sig:
	@echo "device signature: "
	$(H)$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_QUIET) -U signature:r:-:h

read-fuses:
	@echo "fuses: low, high, extra"
	$(H)$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_QUIET) -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h

read-flash:
	$(H)$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:r:flash.img:r

write-fuses:
	$(H)$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FUSE)

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
