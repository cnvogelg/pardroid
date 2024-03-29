cmake-setup:
	[ -d $(BIN_DIR) ] && cmake -B $(BIN_DIR) -S . \
		-DVERSION_TAG=$(VERSION_TAG) \
		-DMACHTAG=$(MACHTAG_ID) \
		-DMACH_$(MACH_UPPER)=ON \
		-DCMAKE_BUILD_TYPE=$(FLAVOR)

cmake-build: $(ROMINFO_FILE)
	$(MAKE) -C $(BIN_DIR)
