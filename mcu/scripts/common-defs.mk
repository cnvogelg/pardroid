include $(CONFIG_MAKE_FILE)

# ----- dirs -----
BIN_DIR=$(BUILD_DIR)
OBJ_DIR=$(BUILD_DIR)/obj
DEP_DIR=$(OBJ_DIR)

# ----- tool flags -----
# setup CFLAGS
CFLAGS_COMMON = -std=c99 -fno-common -Wall -Werror -Wstrict-prototypes
CFLAGS_COMMON += -Wall -Werror -Wstrict-prototypes

CFLAGS_LST = -Wa,-adhlns=$(OBJ_DIR)/$(notdir $(<:%.c=%.lst))
CFLAGS_DEPS = -MMD
CFLAGS_INCLUDES = -I$(BUILD_DIR) $(patsubst %,-I%,$(INCLUDES))

CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_INCLUDES) $(CFLAGS_ARCH) $(CFLAGS_LST) $(CFLAGS_DEPS)

# setup ASFLAGS
ASFLAGS_LST = -Wa,-amdhlns=$(OBJ_DIR)/$(notdir $(<:%.$(ASM_SUFFIX)=%.lst))

ASFLAGS = $(ASFLAGS_COMMON) $(CFLAGS_INCLUDES) $(ASFLAGS_ARCH) $(ASFLAGS_LST) $(CFLAGS_DEPS)

# ----- firmware -----
# list of firmwares, created by make-firmware
FIRMWARES :=

# map-src-to-tgt
# $1 = src files
map-src-to-tgt = $(patsubst %.c,$(OBJ_DIR)/%.o,$(filter %.c,$1)) \
				 $(patsubst %.S,$(OBJ_DIR)/%.o,$(filter %.S,$1))

map-bin = $(patsubst %,$(BIN_DIR)/%,$1)

# size-check
# $1 = size file
# $2 = max value
# $3 = name to print
define size-check
SIZE=`cat $1` ; \
if [ $$$$SIZE -gt $2 ] ; then \
	echo "$3:  $$$$SIZE >  $2 bytes: TOO LARGE" ; exit 1 ; \
else \
	echo "$3:  $$$$SIZE <= $2 bytes: ok" ; \
fi
endef

# make-program rules
# $1 = program name
# $2 = srcs for program
define make-firmware
FIRMWARES += $1

.PHONY: $1 $1-size $1-check-code $1-check-data $1-check $1-prog

$1: $(call map-bin,$1.elf $1.hex $1.lss $1.sym)

$1-size: $(call map-bin,$1.size_sym $1.size_code $1.size_data)

$1-check-code: $(call map-bin,$1.size_code)
	$(H)$(call size-check,$$<,$(CONFIG_MAX_RAM),$$(<F))

$1-check-data: $(call map-bin,$1.size_data)
	$(H)$(call size-check,$$<,$(CONFIG_MAX_ROM),$$(<F))

$1-check: $1-check-code $1-check-data

$1-prog: $(call map-bin,$1.hex)
	$(call prog,$$<,$$(<F))

$(BIN_DIR)/$1.elf: $(call map-src-to-tgt,$2)
	@echo "  LD   $$(@F)"
	$(H)$(CC) $(CFLAGS) $$^ -o $$@ $(LDFLAGS)
endef

# create dirs
ifneq "$(MAKECMDGOALS)" "clean"
create_dir = $(shell test -d $1 || mkdir -p $1)
create_obj_dir := $(call create_dir,$(OBJ_DIR))
create_bin_dir := $(call create_dir,$(BIN_DIR))
endif
