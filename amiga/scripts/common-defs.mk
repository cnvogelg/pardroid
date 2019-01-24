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

LHASFX_STUB = contrib/lhasfx/lhasfx.stub

# ----- program generation -----
# list of programs, created by make-prgoram
PROGRAMS :=
BIN_FILES :=
DIST_FILES :=

# map-src-to-tgt
# $1 = src files
map-src-to-tgt = $(patsubst %.c,$(OBJ_DIR)/%.o,$(filter %.c,$1)) \
				 $(patsubst %.s,$(OBJ_DIR)/%.o,$(filter %.s,$1))

map-dist = $(patsubst %,$(DIST_DIR)/%,$(notdir $1))

# make-program rules
# $1 = program name
# $2 = srcs for program
define make-program
PROGRAMS += $1
BIN_FILES += $(BIN_DIR)/$1

.PHONY: $1
$1: $(BIN_DIR)/$1

$(BIN_DIR)/$1: $(call map-src-to-tgt,$2 $(PRG_SRCS))
	@echo "  LD   $$(@F)"
	$(H)$(CC) $(LDFLAGS) $(LDFLAGS_PRG) -o $$@ $$+
endef

# dist-program
# $1 = program name
define dist-program
DIST_FILES += $(call map-dist,$1$(DIST_TAG))

$(DIST_DIR)/$1$(DIST_TAG): $(BIN_DIR)/$1
	@echo "  DIST $$(@F)"
	$(H)cp $$< $$@
endef

# crunch-program rules
# $1 = out program name
# $2 = in program name
define crunch-program
PROGRAMS += $1
BIN_FILES += $(BIN_DIR)/$1

.PHONY: $1
$1: $(BIN_DIR)/$1

$(BIN_DIR)/$1: $(BIN_DIR)/$2
	@echo "  CRUNCH  $$(@F)"
	$(H)$(CRUNCHER) $$< $$@
	@stat -f '  -> %z bytes' $$@
endef

# create-lha
# $1 = out lha file
# $2 = contents
define create-lha
PROGRAMS += $1
BIN_FILES += $(BIN_DIR)/$1

.PHONY: $1
$1: $(BIN_DIR)/$1

$(BIN_DIR)/$1: $(patsubst %,$(BIN_DIR)/%,$2)
	@echo "  LHA  $$(@F)"
	$(H)cd $(BIN_DIR) && $(LHA) $1 $2
	@stat -f '  -> %z bytes' $$@
endef

# sfx-lha
# $1 = out run file
# $2 = in lha file
define sfx-lha
PROGRAMS += $1
BIN_FILES += $(BIN_DIR)/$1

.PHONY: $1
$1: $(BIN_DIR)/$1

$(BIN_DIR)/$1: $(BIN_DIR)/$2
	@echo "  SFX  $$(@F)"
	$(H)cat $(LHASFX_STUB) $$< > $$@
endef

create_dir = $(shell test -d $1 || mkdir -p $1)

# create dirs
ifneq "$(MAKECMDGOALS)" "clean"
create_obj_dir := $(call create_dir,$(OBJ_DIR))
create_bin_dir := $(call create_dir,$(BIN_DIR))
endif

# dist dir creation
ifneq "$(filter install dist dist-all,$(MAKECMDGOALS))" ""
dist_dir := $(call create_dir,$(DIST_DIR))
endif
