include $(CONFIG_MAKE_FILE)

# ----- dirs -----
BIN_DIR=$(BUILD_DIR)
OBJ_DIR=$(BUILD_DIR)/obj
DEP_DIR=$(OBJ_DIR)

# ----- tool flags -----
# setup CFLAGS
CFLAGS_COMMON = -std=c99 -fno-common -Wall -Werror -Wstrict-prototypes
CFLAGS_COMMON += -Wall -Werror -Wstrict-prototypes
CFLAGS_COMMON += -Wno-error=unused-variable
CFLAGS_COMMON += -ffunction-sections -fdata-sections

CFLAGS_LST = -Wa,-adhlns=$(OBJ_DIR)/$(notdir $(<:%.c=%.lst))
CFLAGS_DEPS = -MMD
CFLAGS_INCLUDES = -I$(BUILD_DIR) $(patsubst %,-I%,$(INCLUDES))
CFLAGS_DEFINES = -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR)

CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_INCLUDES) $(CFLAGS_ARCH) $(CFLAGS_LST) \
	$(CFLAGS_DEPS) $(CFLAGS_DEFINES)

# setup ASFLAGS
ASFLAGS_LST = -Wa,-amdhlns=$(OBJ_DIR)/$(notdir $(<:%.S=%.lst))

ASFLAGS = $(ASFLAGS_COMMON) $(CFLAGS_INCLUDES) $(ASFLAGS_ARCH) $(ASFLAGS_LST) $(CFLAGS_DEPS)

LDFLAGS = -Wl,-Map=$$(@:%.elf=%.map),--cref
LDFLAGS += -lm -lc
LDFLAGS += -Wl,--gc-sections
LDFLAGS += $(LDFLAGS_ARCH)

# ----- firmware -----
# list of firmwares, created by make-firmware
FIRMWARES :=
DIST_FILES :=
PBL_FILES :=

# map-src-to-tgt
# $1 = src files
map-src-to-tgt = $(patsubst %.c,$(OBJ_DIR)/%.o,$(filter %.c,$1)) \
				 $(patsubst %.S,$(OBJ_DIR)/%.o,$(filter %.S,$1))

map-bin = $(patsubst %,$(BIN_DIR)/%,$1)
map-dist = $(patsubst %,$(DIST_DIR)/%,$(notdir $1))

# make-program rules
# $1 = program name
# $2 = srcs for program
# $3 = max rom size
# $4 = extra ld flags
define make-firmware
FIRMWARES += $1
FILES_$1 += $(call map-bin,$1.lss $1.sym $1.elf)

.PHONY: $1-sym $1-code $1-check

$1: $$(FILES_$1) $1-check
	@echo "$1 done"

$1-sym: $(call map-bin,$1.sym_size)
	$(H)cat $$<

$1-code: $(call map-bin,$1.code_size)
	$(H)cat $$<

$1-check: $(call map-bin,$1.elf)
	$(H)$(SIZE) -A $$< | scripts/checksize.py $3 $(CONFIG_MAX_RAM) $1

$1-prog: $(call map-bin,$1.hex) $1-check
	$(call prog-firmware,$$<,$$(<F))

$(BIN_DIR)/$1.elf: $(call map-src-to-tgt,$2)
	@echo "  LD   $$(@F)"
	$(H)$(CC) $(CFLAGS) $$^ -o $$@ $(LDFLAGS) $4
endef

# make-pbl
# $1 = program name
define make-pbl
PBL_FILES += $(call map-dist,$1$(DIST_TAG).pbl)
FILES_$1 += $(call map-bin,$1.pbl)
endef

# dist-pbl
# $1 = program name
define dist-pbl
DIST_FILES += $(call map-dist,$1$(DIST_TAG).pbl)

$(DIST_DIR)/$1$(DIST_TAG).pbl: $(BIN_DIR)/$1.pbl
	@echo "  DIST  $$(@F)"
	$(H)cp $$< $$@
endef

# dist-hex
# $1 = program name
define dist-hex
DIST_FILES += $(call map-dist,$1$(DIST_TAG).hex)

$(DIST_DIR)/$1$(DIST_TAG).hex: $(BIN_DIR)/$1.hex
	@echo "  DIST  $$(@F)"
	$(H)cp $$< $$@
endef

# get-fw-id
# derive firmware id from file name
# $1 = file name *.img or *.pbl
define get-fw-id
$(FW_ID_$(notdir $(basename $1)))
endef

# get-fw-ver
# derive firmware version from file name
# $1 = file name *.img or *.pbl
define get-fw-ver
$(FW_VER_$(notdir $(basename $1)))
endef

create_dir = $(shell test -d $1 || mkdir -p $1)

# create dirs
ifeq "$(filter clean clean-all,$(MAKECMDGOALS))" ""
create_obj_dir := $(call create_dir,$(OBJ_DIR))
create_bin_dir := $(call create_dir,$(BIN_DIR))
endif

# dist dir creation
ifneq "$(filter install dist dist-all,$(MAKECMDGOALS))" ""
dist_dir := $(call create_dir,$(DIST_DIR))
endif

