#include <stddef.h>
#define MICROPY_HW_BOARD_NAME               "NumCalcium PY/A1"
#define MICROPY_HW_MCU_NAME                 "ESP32S3"

#define MICROPY_HW_ENABLE_UART_REPL         (1)

#define MICROPY_HW_I2C0_SCL                 (10)
#define MICROPY_HW_I2C0_SDA                 (9)

#define MICROPY_DEBUG_PRINTERS              (1)

#define ESP_PREFER_RISCV_ULP                (1)