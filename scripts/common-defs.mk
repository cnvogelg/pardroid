include $(CONFIG_MAKE_FILE)

DEPDIR=$(BUILD_DIR)/deps
OBJDIR=$(BUILD_DIR)/obj

ASM_SUFFIX?=S
COBJ = $(patsubst %.c,$(OBJDIR)/%.o,$(CSRC))
AOBJ = $(patsubst %.$(ASM_SUFFIX),$(OBJDIR)/%.o,$(ASRC))
OBJ = $(COBJ) $(AOBJ)

# setup CFLAGS
CFLAGS_COMMON = -std=c99 -fno-common -Wall -Werror -Wstrict-prototypes
CFLAGS_COMMON += -Wall -Werror -Wstrict-prototypes

CFLAGS_INCLUDES = -I$(BUILD_DIR) $(patsubst %,-I%,$(INCLUDES))

CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_INCLUDES) $(CFLAGS_ARCH)

TARGET ?= target
