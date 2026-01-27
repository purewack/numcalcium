# Root Makefile for building MicroPython with a custom board definition

VARIANT   := pya1
FONT := gohu13

PORT ?=

# runtime ids
CURRENT_UID := $(shell id -u)
CURRENT_GID := $(shell id -g)

export CURRENT_UID
export CURRENT_GID

BOARD_DIR_REL := software-$(VARIANT)
BOARD_DIR := $(CURDIR)/$(BOARD_DIR_REL)
DOCKER_BOARD_DIR := /project/$(BOARD_DIR_REL)
DOCKER_MPY_DIR := /project/micropython/ports/esp32

IDF_VERSION := espressif/idf:v5.5.1
DOCKER_CMD := docker run --rm --privileged -v $(CURDIR):/project -w /project -u $(CURRENT_UID) -e HOME=/tmp $(IDF_VERSION)

# main target
.PHONY: all ulp pins font flash clean fullclean

all: 
	$(DOCKER_CMD) make -C $(DOCKER_MPY_DIR) BOARD_DIR=$(DOCKER_BOARD_DIR) PORT=$(PORT) BOARD=$(VARIANT)

firmware-no-freeze: 
	$(DOCKER_CMD) make -C $(DOCKER_MPY_DIR) BOARD_DIR=$(DOCKER_BOARD_DIR) PORT=$(PORT) BOARD=$(VARIANT) BOARD_VARIANT=no_freeze

gen-ulp: gen-dir
	cd micropython-ulp-compiler && make SOURCES="../software-pya1/ulp/io.c" PRE_CMD="cd micropython-ulp-compiler" IDF_VERSION="$(IDF_VERSION)" DOCKER_CMD="$(DOCKER_CMD)" \
OUTPUT_DIR="${DOCKER_BOARD_DIR}/generated" \
NAME="__ulpio"
	cp $(BOARD_DIR)/generated/__ulpio.py $(BOARD_DIR)/modules/__ulpio.py

gen-pins: gen-dir
	python3 $(BOARD_DIR)/generators/generate_pins.py $(BOARD_DIR)/cmodules/pins.h  $(BOARD_DIR)/pins.csv $(BOARD_DIR)/generated/pins.py
    
gen-font: gen-dir
	mkdir -p $(BOARD_DIR)/fonts
	python3 $(BOARD_DIR)/generators/generate_font.py  $(BOARD_DIR)/fonts/$(FONT) $(BOARD_DIR)/fonts

gen-dir:
	mkdir -p $(BOARD_DIR)/generated

deploy:
	$(DOCKER_CMD) make -C $(DOCKER_MPY_DIR) BOARD_DIR=$(DOCKER_BOARD_DIR) PORT=$(PORT) BOARD=$(VARIANT) deploy

deploy-no-freeze:
	$(DOCKER_CMD) make -C $(DOCKER_MPY_DIR) BOARD_DIR=$(DOCKER_BOARD_DIR) PORT=$(PORT) BOARD=$(VARIANT)-no_freeze deploy

erase: 
	$(DOCKER_CMD) make -C $(DOCKER_MPY_DIR) BOARD_DIR=$(DOCKER_BOARD_DIR) PORT=$(PORT) BOARD=$(VARIANT) erase

clean:
	rm -rf ${CURDIR}/micropython/ports/esp32/build-$(VARIANT) 
	rm -rf ${CURDIR}/micropython/ports/esp32/build-$(VARIANT)-no_freeze
	rm -rf ${CURDIR}/micropython-ulp-compiler/build
	rm -rf ${CURDIR}/micropython-ulp-compiler/.cache
	rm -rf ${CURDIR}/micropython-ulp-compiler/ulp-compiler/build

fullclean: clean
	rm -rf ${CURDIR}/software-$(VARIANT)/generated


# mount local dirs for dev mode for py files
dev:
	mpremote mount ../numcalcium-software run ./main.py

dev-modules:
	mpremote mount $(BOARD_DIR)/modules

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
