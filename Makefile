# Root Makefile for building MicroPython with a custom board definition

# Path to your custom board definition
VARIANT   := pya1
BOARD_DIR := $(CURDIR)/software-$(VARIANT)

FONT := gohu13
FONT_W := 8
FONT_H := 13

IDF_VERSION := espressif/idf:v5.5.1


# runtime ids
CURRENT_UID := $(shell id -u)
CURRENT_GID := $(shell id -g)
CURRENT_PWD := $(shell pwd)

export CURRENT_UID
export CURRENT_GID

# main target
.PHONY: all ulp pins font flash clean fullclean

all: 
	docker run --rm -v $(BOARD_DIR)/..:/project -w /project -u $(CURRENT_UID) -e HOME=/tmp $(IDF_VERSION) make -C /project/micropython/ports/esp32 BOARD_DIR=/project/software-$(VARIANT) BOARD=$(VARIANT)

gen-ulp: gen-dir
	docker run --rm -v $(BOARD_DIR)/..:/project -w /project -u $(CURRENT_UID) -e HOME=/tmp $(IDF_VERSION) bash -c "cd /project/software-$(VARIANT)/ulp-compiler && idf.py build"
	python3 $(BOARD_DIR)/generators/generate_ulp.py $(BOARD_DIR)/ulp-compiler/build/esp-idf/main/ulp_main/ulp_main.bin  $(BOARD_DIR)/ulp-compiler/build/esp-idf/main/ulp_main/ulp_main.ld $(BOARD_DIR)/modules/__ulpio.py 
	cp $(BOARD_DIR)/ulp-compiler/build/esp-idf/main/ulp_main/ulp_main.bin  $(BOARD_DIR)/ulp-compiler/build/esp-idf/main/ulp_main/ulp_main.ld $(BOARD_DIR)/generated
 
gen-pins: gen-dir
	python3 $(BOARD_DIR)/make_pins.py $(BOARD_DIR)/cmodules/pins.h  $(BOARD_DIR)/generated/pins.csv $(BOARD_DIR)/generated/pins.py
	cp $(BOARD_DIR)/generated/pins.csv $(BOARD_DIR)/pins.csv
    
gen-font: gen-dir
	python3 $(BOARD_DIR)/make_font.py $(BOARD_DIR)/font/$(FONT) $(FONT_W) $(FONT_H)

gen-dir:
	mkdir -p $(BOARD_DIR)/generated

deploy:
	docker run --rm --privileged -v $(BOARD_DIR)/..:/project -w /project -u $(CURRENT_UID) -e HOME=/tmp $(IDF_VERSION) make -C /project/micropython/ports/esp32 BOARD_DIR=/project/software-$(VARIANT) BOARD=$(VARIANT) deploy

clean:
	rm -rf ${CURRENT_PWD}/micropython/ports/esp32/build-$(VARIANT) 
	rm -rf ${CURRENT_PWD}/software-$(VARIANT)/ulp-compiler/build

fullclean: clean
	rm -rf ${CURRENT_PWD}/software-$(VARIANT)/generated

erase: 
	esptool.py erase_flash

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
