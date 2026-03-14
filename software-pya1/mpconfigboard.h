#include <stddef.h>
#include "cmodules/pins.h"

#define MICROPY_HW_BOARD_NAME               "NumCalcium PY/A1"
#define MICROPY_HW_MCU_NAME                 "ESP32S3"

#define MICROPY_HW_ENABLE_UART_REPL         (1)

#define MICROPY_DEBUG_PRINTERS              (1)

#define MICROPY_HW_I2C0_SCL                 (BOARD_PIN_PORT_SCL)
#define MICROPY_HW_I2C0_SDA                 (BOARD_PIN_PORT_SDA)

#define ESP_PREFER_RISCV_ULP                (1)