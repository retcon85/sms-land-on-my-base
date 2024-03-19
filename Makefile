# Forgive me for I know not what I do.
SHELL = /bin/bash

# OPTIONS YOU MIGHT NEED TO CHANGE:
# change this to true to build in PSGlib (for music & sound in your game)
USEPSGLIB := true
ASSETSOPTS :=

# by default output will take the name of the folder we're in
PROJECTNAME := $(notdir $(CURDIR))
TARGETDIR := ./build/
SOURCEDIR := ./src/
ASSETSDIR := ./assets/
TARGETEXT := sms
SOURCEEXT := c
HEADEREXT := h
# the default entrypoint (where main function is defined), which must come first in the linker
ENTRYPOINT := main
SMSLIB_DIR := /usr/local/share/sdcc/lib/sms
SMSINC_DIR := /usr/local/share/sdcc/include/sms
ifeq ("$(USEPSGLIB)", "true")
PSGLIB := $(SMSLIB_DIR)/PSGlib.rel
PSGMACRO := -DUSEPSGLIB
endif

TARGET := $(TARGETDIR)$(PROJECTNAME).$(TARGETEXT)
SOURCES = $(wildcard $(SOURCEDIR)*.$(SOURCEEXT))
OBJECTS = $(patsubst $(SOURCEDIR)%.$(SOURCEEXT),$(TARGETDIR)%.rel,$(SOURCES))
HEADERS := $(SOURCEDIR)*.$(HEADEREXT)

# asset bundling
ASSETS := $(wildcard $(ASSETSDIR)*)
ASSETSBINTARGET := $(patsubst $(ASSETSDIR)%.bin,$(TARGETDIR)$(ASSETSDIR)%.bin,$(filter %.bin, $(ASSETS)))
ASSETSPSGTARGET := $(patsubst $(ASSETSDIR)%.psg,$(TARGETDIR)$(ASSETSDIR)%.psg,$(filter %.psg, $(ASSETS)))
ASSETSVGMTARGET := $(patsubst $(ASSETSDIR)%.vgm,$(TARGETDIR)$(ASSETSDIR)%.psg,$(filter %.vgm, $(ASSETS)))
ASSETSTILESTARGET := $(patsubst $(ASSETSDIR)%.bmp,$(TARGETDIR)$(ASSETSDIR)%.tiles,$(filter %.bmp, $(ASSETS)))
ASSETSPALETTETARGET := $(patsubst $(ASSETSDIR)%.bmp,$(TARGETDIR)$(ASSETSDIR)%.palette,$(filter %.bmp, $(ASSETS)))
# patterns included in $(BUNDLEDASSETS) will be bundled; others will be ignored
BUNDLEDASSETS = $(ASSETSBINTARGET) $(ASSETSPSGTARGET) $(ASSETSVGMTARGET) $(ASSETSTILESTARGET) $(ASSETSPALETTETARGET)
# the file src/assets.genererated.h will be automatically generated to index all bundled assets in code
ASSETSHEADER := $(SOURCEDIR)assets.generated.$(HEADEREXT)
BANKSOURCES = $(wildcard $(TARGETDIR)bank*.c)
BANKGENERATEDSOURCES = $(patsubst $(TARGETDIR)bank%.c,$(SOURCEDIR)bank%.generated.c,$(BANKSOURCES))

# customize this list to explicitly specify the order of linking
MAINS := $(TARGETDIR)$(ENTRYPOINT).rel

# main build target
build: $(TARGETDIR) assets $(TARGET)

# create the build folder if it doesn't exist
$(TARGETDIR):
	mkdir -p $(TARGETDIR)

# link stage - generally runs once to create a single output
$(TARGETDIR)%.ihx: $(OBJECTS)
	sdcc -L$(SMSLIB_DIR) -o$@ -mz80 --no-std-crt0 --data-loc 0xC000 -Wl-b_BANK3=0x8000 -Wl-b_BANK2=0x8000 $(SMSLIB_DIR)/crt0_sms.rel $(MAINS) $(filter-out $(MAINS),$(OBJECTS)) SMSlib.lib $(PSGLIB) || rm $@

# compile stage - generally runs once per .c file found in the source folder
$(TARGETDIR)%.rel: $(SOURCEDIR)%.$(SOURCEEXT) $(ASSETSHEADER) $(HEADERS)
	sdcc -I$(SMSINC_DIR) --opt-code-speed -c -mz80 -o$(TARGETDIR) --peep-file $(SMSLIB_DIR)/peep-rules.txt $(PSGMACRO) $<

# packing stage - generally runs once to create a single output
$(TARGET): $(TARGETDIR)$(ENTRYPOINT).ihx
	ihx2sms $< $(TARGET)

.PHONY: assets
assets: $(TARGETDIR) $(TARGETDIR)$(ASSETSDIR) $(BUNDLEDASSETS) $(ASSETSHEADER)

$(SOURCEDIR)bank%.generated.c: $(TARGETDIR)bank%.c $(ASSETSHEADER)
	mv $< $@

# automatically bundle selected assets using assets2banks
$(ASSETSHEADER): $(TARGETDIR)$(ASSETSDIR) $(BUNDLEDASSETS) $(ASSETSDIR)assets2banks.cfg
	echo '// This file is automatically generated - do not modify!' > $(ASSETSHEADER)
	-cp  $(ASSETSDIR)assets2banks.cfg $(TARGETDIR)$(ASSETSDIR)
	cd $(TARGETDIR) && assets2banks $(ASSETSDIR) $(ASSETSOPTS) --allowsplitting --singleheader=../$(ASSETSHEADER)
	for f in $(TARGETDIR)bank*.c; do \
		b=`basename $$f`; \
		t="$(SOURCEDIR)$${b/.c/.generated.c}"; \
		echo '// This file is automatically generated - do not modify!' > $$t; \
		echo "#pragma constseg $${b/.c/}" >> $$t; echo "" >> $$t; \
		cat $$f >> $$t; \
	done

# include any existing .psg file in assets folder in bundled assets
$(TARGETDIR)$(ASSETSDIR)%.psg: $(ASSETSDIR)%.psg
	cp $< $@
# include any existing .tiles file in assets folder in bundled assets
$(TARGETDIR)$(ASSETSDIR)%.tiles: $(ASSETSDIR)%.tiles
	cp $< $@
# include any existing .palette file in assets folder in bundled assets
$(TARGETDIR)$(ASSETSDIR)%.palette: $(ASSETSDIR)%.palette
	cp $< $@
# include any existing .bin file in assets folder in bundled assets
$(TARGETDIR)$(ASSETSDIR)%.bin: $(ASSETSDIR)%.bin
	cp $< $@

# convert a .bmp file in assets folder to .tiles and .palette files in bundled assets
$(TARGETDIR)$(ASSETSDIR)%.tiles: $(ASSETSDIR)%.bmp
	img2tiles $< -obin --no-palette > $@
$(TARGETDIR)$(ASSETSDIR)%.palette: $(ASSETSDIR)%.bmp
	img2tiles $< -obin --no-tiles > $@

# convert a .vgm file in assets folder to .psg file in bundled assets
$(TARGETDIR)$(ASSETSDIR)%.psg: $(ASSETSDIR)%.vgm
	vgm2psg $< $@
	retcon-audio psg $@ $@

# create bundled assets folder under build as staging area
$(TARGETDIR)$(ASSETSDIR):
	mkdir -p $(TARGETDIR)$(ASSETSDIR)

# make clean to remove the build folder and all generated files
.PHONY: clean
clean:
	rm -rf $(TARGETDIR)
	-rm -r */*.generated.*

# intermediates never get deleted
.SECONDARY: