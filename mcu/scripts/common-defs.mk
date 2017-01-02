include $(CONFIG_MAKE_FILE)

OBJDIR=$(BUILD_DIR)/obj
DEPDIR=$(OBJDIR)

ASM_SUFFIX?=S
COBJ = $(patsubst %.c,$(OBJDIR)/%.o,$(CSRC))
AOBJ = $(patsubst %.$(ASM_SUFFIX),$(OBJDIR)/%.o,$(ASRC))
OBJ = $(AOBJ) $(COBJ)

# setup CFLAGS
CFLAGS_COMMON = -std=c99 -fno-common -Wall -Werror -Wstrict-prototypes
CFLAGS_COMMON += -Wall -Werror -Wstrict-prototypes

CFLAGS_LST = -Wa,-adhlns=$(OBJDIR)/$(notdir $(<:%.c=%.lst))
CFLAGS_DEPS = -MMD
CFLAGS_INCLUDES = -I$(BUILD_DIR) $(patsubst %,-I%,$(INCLUDES))

CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_INCLUDES) $(CFLAGS_ARCH) $(CFLAGS_LST) $(CFLAGS_DEPS)

# setup ASFLAGS
ASFLAGS_LST = -Wa,-amdhlns=$(OBJDIR)/$(notdir $(<:%.$(ASM_SUFFIX)=%.lst))

ASFLAGS = $(ASFLAGS_COMMON) $(CFLAGS_INCLUDES) $(ASFLAGS_ARCH) $(ASFLAGS_LST) $(CFLAGS_DEPS)

TARGET ?= target

TARGET_ELF = $(BUILD_DIR)/$(TARGET).elf
TARGET_HEX = $(BUILD_DIR)/$(TARGET).hex
TARGET_LSS = $(BUILD_DIR)/$(TARGET).lss
TARGET_SYM = $(BUILD_DIR)/$(TARGET).sym
