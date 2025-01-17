set(IDF_TARGET esp32s3)

set(SDKCONFIG_DEFAULTS
    boards/sdkconfig.base
    ${SDKCONFIG_IDF_VERSION_SPECIFIC}
    ${MICROPY_BOARD_DIR}/sdkconfig.board
    boards/sdkconfig.usb
    boards/sdkconfig.ble
    boards/sdkconfig.spiram_sx
    boards/sdkconfig.240mhz
    boards/sdkconfig.spiram_oct
)

set(MICROPY_SOURCE_BOARD
    ${MICROPY_SOURCE_BOARD}
#    ${MICROPY_BOARD_DIR}/board_init.c
    ${MICROPY_BOARD_DIR}/cmodules/modtest.c
    ${MICROPY_BOARD_DIR}/cmodules/modboard.c
    ${MICROPY_BOARD_DIR}/cmodules/board_lcd.c
    ${MICROPY_BOARD_DIR}/cmodules/board_sdcard.c
)

set(MICROPY_FROZEN_MANIFEST ${MICROPY_BOARD_DIR}/manifest.py)

set(ulp_embedded_sources ${MICROPY_BOARD_DIR}/ulp/io.c)
