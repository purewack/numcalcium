# Root Makefile for building MicroPython with a custom board definition

# Path to your custom board definition
VARIANT   := pya1
BOARD_DIR := $(CURDIR)/software-$(VARIANT)

# Path to the MicroPython submodule
MPY_DIR := micropython

# Path to the ESP32 port
ESP32_PORT_DIR := $(MPY_DIR)/ports/esp32


# Default target
.PHONY: all
all:
	$(MAKE) -C $(ESP32_PORT_DIR) BOARD_DIR=$(BOARD_DIR) BOARD=$(notdir $(VARIANT))
	tput bel

# Update / customize pin definitions for the board
pins:
	python3 $(BOARD_DIR)/make_pins.py $(BOARD_DIR)/cmodules/pins.h  $(BOARD_DIR)/pins.csv $(BOARD_DIR)/modules/pins.py

# Clean target
.PHONY: clean
clean:
	$(MAKE) -C $(ESP32_PORT_DIR) BOARD_DIR=$(BOARD_DIR) BOARD=$(notdir $(VARIANT)) clean

# Flash target (optional: specify PORT if needed)
.PHONY: flash
flash:
	$(MAKE) -C $(ESP32_PORT_DIR) BOARD_DIR=$(BOARD_DIR) BOARD=$(notdir $(VARIANT)) deploy

# Monitor target
.PHONY: fresh
fresh:
	rm -rf $(ESP32_PORT_DIR)/build-$(VARIANT)/frozen_content.c $(ESP32_PORT_DIR)/build-$(VARIANT)/genhdr $(ESP32_PORT_DIR)/build-$(VARIANT)/frozen-mpy $(ESP32_PORT_DIR)/build-$(VARIANT)/pins.c
# Monitor target
.PHONY: monitor
monitor:
	$(MAKE) -C $(ESP32_PORT_DIR) BOARD_DIR=$(BOARD_DIR) BOARD=$(notdir $(BOARD_DIR)) monitor



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