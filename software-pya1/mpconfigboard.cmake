set(IDF_TARGET esp32s3)

set(SDKCONFIG_DEFAULTS
    boards/sdkconfig.base
    ${SDKCONFIG_IDF_VERSION_SPECIFIC}
    ${MICROPY_BOARD_DIR}/sdkconfig.board
    boards/sdkconfig.ble
    boards/sdkconfig.spiram_sx
    boards/sdkconfig.240mhz
    boards/sdkconfig.spiram_oct
    boards/sdkconfig.riscv_ulp
)

set(MICROPY_SOURCE_BOARD
    ${MICROPY_SOURCE_BOARD}
    ${MICROPY_BOARD_DIR}/cmodules/modmpy.c
    ${MICROPY_BOARD_DIR}/cmodules/modboard.c
    ${MICROPY_BOARD_DIR}/cmodules/board_sdcard.c
    ${MICROPY_BOARD_DIR}/cmodules/board_dac.c
    ${MICROPY_BOARD_DIR}/cmodules/board_lcd.c
    ${MICROPY_BOARD_DIR}/cmodules/lcd_bmp.c
    ${MICROPY_BOARD_DIR}/cmodules/lcd_driver.c
    ${MICROPY_BOARD_DIR}/cmodules/font.c
    ${MICROPY_BOARD_DIR}/cmodules/font_hex_codes.c
)

set(MICROPY_FROZEN_MANIFEST ${MICROPY_BOARD_DIR}/manifest.py)
