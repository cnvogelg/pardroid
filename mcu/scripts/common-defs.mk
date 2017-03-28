include $(CONFIG_MAKE_FILE)

# ----- dirs -----
BIN_DIR=$(BUILD_DIR)
OBJ_DIR=$(BUILD_DIR)/obj
DEP_DIR=$(OBJ_DIR)

# ----- tool flags -----
# setup CFLAGS
CFLAGS_COMMON = -std=c99 -fno-common -Wall -Werror -Wstrict-prototypes
CFLAGS_COMMON += -Wall -Werror -Wstrict-prototypes
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

# ----- firmware -----
# list of firmwares, created by make-firmware
FIRMWARES :=
DIST_FILES :=

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

.PHONY: $1-sym $1-code $1-check

$1-sym: $(call map-bin,$1.sym_size)
	$(H)cat $$<

$1-code: $(call map-bin,$1.code_size)
	$(H)cat $$<

$1-check: $(call map-bin,$1.elf)
	$(H)$(SIZE) -A $$< | scripts/checksize.py $3 $(CONFIG_MAX_RAM) $1

$(BIN_DIR)/$1.elf: $(call map-src-to-tgt,$2)
	@echo "  LD   $$(@F)"
	$(H)$(CC) $(CFLAGS) $$^ -o $$@ $(LDFLAGS) $4
endef

# make-pablo
# $1 = program name
define make-pablo
DIST_FILES += $(call map-dist,$1-$(DIST_TAG).pbl)

.PHONY: $1 $1-prog $1-prog-full

$1: $(call map-bin,$1.pbl $1.lss $1.sym) $1-check

$1-prog: $(call map-bin,$1.hex) $1-check
	$(call prog-firmware,$$<,$$(<F))

$1-prog-full: $(call map-bin,$1.img) $1-check
	$(call prog-firmware,$$<,$$(<F))

$1-dist: $(call map-bin,$1-$(CONFIG_BASE_NAME).hex)

$(DIST_DIR)/$1-$(DIST_TAG).pbl: $(BIN_DIR)/$1.pbl
	@echo "  DIST  $$(@F)"
	$(H)cp $$< $$@
endef

# make-bootloader
# $1 = program name
define make-bootloader
DIST_FILES += $(call map-dist,$1-$(DIST_TAG).hex)

.PHONY: $1 $1-prog

$1: $(call map-bin,$1.hex $1.lss $1.sym) $1-check

$1-prog: $(call map-bin,$1.hex) $1-check
	$(call prog-bootloader,$$<,$$(<F))

$(DIST_DIR)/$1-$(DIST_TAG).hex: $(BIN_DIR)/$1.hex
	@echo "  DIST  $$(@F)"
	$(H)cp $$< $$@
endef

# create dirs
ifeq "$(filter clean clean-all,$(MAKECMDGOALS))" ""
create_dir = $(shell test -d $1 || mkdir -p $1)
create_obj_dir := $(call create_dir,$(OBJ_DIR))
create_bin_dir := $(call create_dir,$(BIN_DIR))
endif

# dist dir creation
ifneq "$(filter dist dist-all,$(MAKECMDGOALS))" ""
dist_dir := $(call create_dir,$(DIST_DIR))
endif

