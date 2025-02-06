# Root Makefile for building MicroPython with a custom board definition

# Path to the MicroPython submodule
MPY_DIR := micropython

# Path to the ESP32 port
ESP32_PORT_DIR := $(MPY_DIR)/ports/esp32

# Path to your custom board definition
VARIANT   := pya1
BOARD_DIR := $(CURDIR)/$(VARIANT)

# Default target
.PHONY: all
all:
	$(MAKE) -C $(ESP32_PORT_DIR) BOARD_DIR=$(BOARD_DIR) BOARD=$(notdir $(BOARD_DIR))

# Clean target
.PHONY: clean
clean:
	$(MAKE) -C $(ESP32_PORT_DIR) BOARD_DIR=$(BOARD_DIR) BOARD=$(notdir $(BOARD_DIR)) clean

# Flash target (optional: specify PORT if needed)
.PHONY: flash
flash:
	$(MAKE) -C $(ESP32_PORT_DIR) BOARD_DIR=$(BOARD_DIR) BOARD=$(notdir $(BOARD_DIR)) deploy

# Monitor target
.PHONY: fresh
fresh:
	rm -rf $(ESP32_PORT_DIR)/build-$(VARIANT)/frozen_content.c $(ESP32_PORT_DIR)/build-$(VARIANT)/genhdr $(ESP32_PORT_DIR)/build-$(VARIANT)/frozen-mpy

# Monitor target
.PHONY: monitor
monitor:
	$(MAKE) -C $(ESP32_PORT_DIR) BOARD_DIR=$(BOARD_DIR) BOARD=$(notdir $(BOARD_DIR)) monitor

