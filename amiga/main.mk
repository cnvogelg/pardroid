
# common defs
include scripts/common-defs.mk

# toolchain
include scripts/toolchain-$(CONFIG_ARCH).mk

# c sources
CSRC = parbox-test.c debug.c pario.c
ASRC = pario_irq.s

#ASRC += $(ASRC_MACH)

VPATH=src/$(ARCH_DIR) src/$(MACH_DIR) src/arch src
INCLUDES=src src/arch src/$(ARCH_DIR) src/$(MACH_DIR)

TARGET = parbox

# --- rules ---

include scripts/common-rules.mk
