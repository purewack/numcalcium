# Root Makefile for building MicroPython with a custom board definition

VARIANT   := pya1
FONT := gohu13
FONT_CHAR_COUNT = 96

VERSION_NUMCALC := "A1"

PORT ?=

# runtime ids
CURRENT_UID := $(shell id -u)
CURRENT_GID := $(shell id -g)

export CURRENT_UID
export CURRENT_GID

MPY_DIR_REL 		:= micropython/ports/esp32
BOARD_DIR_REL 		:= software-$(VARIANT)
BOARD_DIR 			:= $(CURDIR)/$(BOARD_DIR_REL)
DOCKER_BOARD_DIR 	:= /project/$(BOARD_DIR_REL)
DOCKER_MPY_DIR 		:= /project/$(MPY_DIR_REL)

IDF_VERSION := espressif/idf:v5.5.1
DOCKER_CMD := docker run --rm --privileged -v $(CURDIR):/project -w /project -u $(CURRENT_UID) -e HOME=/tmp $(IDF_VERSION)

# main target
.PHONY: all ulp pins font flash clean fullclean

all: gen-version
	$(DOCKER_CMD) make -C $(DOCKER_MPY_DIR) BOARD_DIR=$(DOCKER_BOARD_DIR) PORT=$(PORT) BOARD=$(VARIANT)

firmware-no-freeze: 
	$(DOCKER_CMD) make -C $(DOCKER_MPY_DIR) BOARD_DIR=$(DOCKER_BOARD_DIR) PORT=$(PORT) BOARD=$(VARIANT) BOARD_VARIANT=no_freeze

generated: gen-ulp gen-pins

gen-ulp: gen-dir
	cd micropython-ulp-compiler && make SOURCES="../software-pya1/ulp/io.c" PRE_CMD="cd micropython-ulp-compiler" IDF_VERSION="$(IDF_VERSION)" DOCKER_CMD="$(DOCKER_CMD)" \
OUTPUT_DIR="${DOCKER_BOARD_DIR}/generated" \
NAME="__ulpio"
	cp $(BOARD_DIR)/generated/__ulpio.py $(BOARD_DIR)/modules/__ulpio.py

gen-pins: gen-dir
	python3 $(BOARD_DIR)/generators/pins.py $(BOARD_DIR)/cmodules/pins.h  $(BOARD_DIR)/pins.csv $(BOARD_DIR)/generated/pins.py
    
gen-font: gen-dir
	mkdir -p $(BOARD_DIR)/fonts
	python3 $(BOARD_DIR)/generators/font.py  $(BOARD_DIR)/fonts/$(FONT) $(BOARD_DIR)/generated $(FONT_CHAR_COUNT)
	for file in $(BOARD_DIR)/generated/font_*.py; do \
		mpy-cross -march=xtensawin "$$file"; \
	done

gen-dir:
	mkdir -p $(BOARD_DIR)/generated

gen-version:
	mkdir -p $(BOARD_DIR)/modules-build
	python3 $(BOARD_DIR)/generators/version.py $(VERSION_NUMCALC) $(BOARD_DIR)/modules-build/__numcalcium_version.py

deploy:
	$(DOCKER_CMD) make -C $(DOCKER_MPY_DIR) BOARD_DIR=$(DOCKER_BOARD_DIR) PORT=$(PORT) BOARD=$(VARIANT) deploy

deploy-no-freeze:
	$(DOCKER_CMD) make -C $(DOCKER_MPY_DIR) BOARD_DIR=$(DOCKER_BOARD_DIR) PORT=$(PORT) BOARD=$(VARIANT)-no_freeze deploy

erase: 
	$(DOCKER_CMD) make -C $(DOCKER_MPY_DIR) BOARD_DIR=$(DOCKER_BOARD_DIR) PORT=$(PORT) BOARD=$(VARIANT) erase

clean:
	rm -rf ${CURDIR}/$(MPY_DIR_REL)/build-$(VARIANT) 
	rm -rf ${CURDIR}/$(MPY_DIR_REL)/build-$(VARIANT)-no_freeze

fullclean: clean
	rm -rf ${CURDIR}/software-$(VARIANT)/generated
	rm -rf ${CURDIR}/micropython-ulp-compiler/build
	rm -rf ${CURDIR}/micropython-ulp-compiler/.cache
	rm -rf ${CURDIR}/micropython-ulp-compiler/ulp-compiler/build

distribute: clean all
	mkdir -p $(CURDIR)/dist
	cp ${CURDIR}/$(MPY_DIR_REL)/build-$(VARIANT)/firmware.bin \
${CURDIR}/dist/numcalcium-$(VERSION_NUMCALC)-`cd $(BOARD_DIR)/generated && python3 -c 'import __versioning; print(__versioning._build)'`.bin

# mount local dirs for dev mode for py files

dev-modules:
	mpremote mount $(BOARD_DIR) run $(BOARD_DIR)/dev-modules.py

dev-generated:
	mpremote mount $(BOARD_DIR)/generated


# test scripts
test-display:
	mpremote mount tests run tests/display.py

test-display-double:
	mpremote run tests/display_doublebuf.py

test-io:
	mpremote run tests/inputs.py

test-lights:
	mpremote run tests/lights.py

test-sd:
	mpremote run tests/sd_display.py

test-audio:
	mpremote mount tests/headphones run tests/headphones/stream.py

test-shorts:
	mpremote run tests/shorts.py
