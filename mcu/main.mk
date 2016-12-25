
# common defs
include $(BASE_DIR)/scripts/common-defs.mk

# toolchain
include scripts/toolchain-$(CONFIG_ARCH).mk

# c sources
CSRC_ARCH=uart.c
CSRC_TARGET=util.c uartutil.c main.c
CSRC = $(CSRC_ARCH) $(CSRC_TARGET)

VPATH=src/$(ARCH_DIR) src/$(MACH_DIR) src/arch src

INCLUDES=src src/arch src/$(ARCH_DIR) src/$(MACH_DIR)

TARGET = parbox

# --- rules ---

include $(BASE_DIR)/scripts/common-rules.mk

# flash rules
include scripts/flash-$(CONFIG_FLASH_TOOL).mk
