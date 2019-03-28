include version.mk

.PHONY: docs

DISTFILES := README.md
PROJECT := parbox

REVSION := $(shell git log -1 --pretty=format:%h)
DATE := $(shell date '+%Y%m%d')
DIST_NAME := $(PROJECT)-$(VERSION)
SNAP_NAME := $(PROJECT)-pre$(VERSION)-$(REVSION)-$(DATE)
PWD = $(shell pwd)
SHOW_CMD = open

help:
	@echo "clean   clean release/snapshot"
	@echo "dist    build release"
	@echo "snap    build snapshot"
	@echo "init    install python packages"
	@echo "docs    create html docs"
	@echo "show    show documents"

init:
	pip3 install --upgrade pip
	pip3 install sphinx

docs:
	[ -d build/docs/html ] || mkdir -p build/docs/html
	sphinx-build -b html docs build/docs/html

show: docs
	$(SHOW_CMD) build/docs/html/index.html

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
	@$(MAKE) -C amiga dist-all DIST_BASE_DIR=$(PWD)/$(PACK_NAME)/amiga
	# mcu
	@mkdir -p $(PACK_NAME)/mcu
	@$(MAKE) -C mcu dist-all DIST_BASE_DIR=$(PWD)/$(PACK_NAME)/mcu
	# other files
	@cp -a $(DISTFILES) $(PACK_NAME)
	@zip -r $(PACK_NAME).zip $(PACK_NAME)
	@rm -rf $(PACK_NAME)
