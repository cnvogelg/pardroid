include $(CONFIG_MAKE_FILE)

# dir config
OBJ_DIR=$(BUILD_DIR)/obj
BIN_DIR=$(BUILD_DIR)

# ----- toool config -----
# setup CFLAGS
CFLAGS_INCLUDES = -I$(BUILD_DIR) $(patsubst %,-I%,$(INCLUDES))
CFLAGS_DEBUG = -g

CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_INCLUDES) $(CFLAGS_ARCH) $(CFLAGS_DEBUG)

# setup ASFLAGS
ASFLAGS = $(ASFLAGS_COMMON) $(CFLAGS_INCLUDES) $(ASFLAGS_ARCH)

LDFLAGS_DEBUG = -g
LDFLAGS = $(LDFLAGS_COMMON) $(LDFLAGS_ARCH) $(LDFLAGS_DEBUG)

# ----- program generation -----
# list of programs, created by make-prgoram
PROGRAMS :=

# map-src-to-tgt
# $1 = src files
map-src-to-tgt = $(patsubst %.c,$(OBJ_DIR)/%.o,$(filter %.c,$1)) \
				 $(patsubst %.s,$(OBJ_DIR)/%.o,$(filter %.s,$1))

# make-program rules
# $1 = program name
# $2 = srcs for program
define make-program
PROGRAMS += $1

.PHONY: $1
$1: $(BIN_DIR)/$1

$(BIN_DIR)/$1: $(call map-src-to-tgt,$2 $(PRG_SRCS))
	@echo "  LD   $$(@F)"
	$(H)$(CC) $(LDFLAGS) $(LDFLAGS_PRG) -o $$@ $$+
endef

# create dirs
ifneq "$(MAKECMDGOALS)" "clean"
create_dir = $(shell test -d $1 || mkdir -p $1)
create_obj_dir := $(call create_dir,$(OBJ_DIR))
create_bin_dir := $(call create_dir,$(BIN_DIR))
endif
