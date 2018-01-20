include version.mk

DISTFILES := README.md
PROJECT := parbox

REVSION := $(shell git log -1 --pretty=format:%h)
DATE := $(shell date '+%Y%m%d')
DIST_NAME := $(PROJECT)-$(VERSION)
SNAP_NAME := $(PROJECT)-pre$(VERSION)-$(REVSION)-$(DATE)
PWD = $(shell pwd)

help:
	@echo "clean   clean release/snapshot"
	@echo "dist    build release"
	@echo "snap    build snapshot"

clean:
	@rm -rf $(DIST_NAME) $(DIST_NAME).zip
	@rm -rf $(SNAP_NAME) $(SNAP_NAME).zip
	@rm -rf amiga/DIST amiga/BUILD
	@rm -rf mcu/DIST mcu/BUILD

dist:
	@$(MAKE) pack PACK_NAME=$(DIST_NAME)

snap:
	@$(MAKE) pack PACK_NAME=$(SNAP_NAME)

pack:
	@echo "packing $(PACK_NAME)"
	@rm -rf $(PACK_NAME) $(PACK_NAME).zip
	# amiga
	@mkdir -p $(PACK_NAME)/amiga
	@$(MAKE) -C amiga dist DIST_BASE_DIR=$(PWD)/$(PACK_NAME)/amiga
	# mcu
	@mkdir -p $(PACK_NAME)/mcu
	@$(MAKE) -C mcu dist-configs DIST_BASE_DIR=$(PWD)/$(PACK_NAME)/mcu
	# other files
	@cp -a $(DISTFILES) $(PACK_NAME)
	@zip -r $(PACK_NAME).zip $(PACK_NAME)
	@rm -rf $(PACK_NAME)
