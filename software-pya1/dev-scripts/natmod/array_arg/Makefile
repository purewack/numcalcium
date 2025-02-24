# Location of top-level MicroPython directory
MPY_DIR = ../../micropython

# Name of module
MOD = array_nat

# Source files (.c or .py)
SRC = array.c

# Architecture to build for (x86, x64, armv7m, xtensa, xtensawin)
ARCH = xtensawin

# Include to get the rules for compiling and linking the module
include $(MPY_DIR)/py/dynruntime.mk

test:
	mpremote mount ./ run test.py
