#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define COL_RED    0xf800
#define COL_GREEN  0x07e0
#define COL_BLUE   0x001f
#define COL_BLACK  0x0000
#define COL_WHITE  0xffff

// Pin definitions
#define TFT_BL     46
#define TFT_CS     42
#define TFT_DC     45
#define TFT_MOSI   40
#define TFT_SCLK   39

#define Y_OFFSET    35
#define X_SIZE      320
#define Y_SIZE      170
#define X_CHAR      53
#define Y_CHAR      21

typedef struct settings_type {
    uint16_t color;
    int8_t x;
    int8_t y;
    int8_t col;
    int8_t line;
    uint8_t scale;
    bool largeLF;
    bool underlineLF;
    bool LFCR;
    bool std;
    bool ignoreEscapes;
    bool invert;
} settings_t;

extern settings_t lcd;

// Utility functions
void driver_send_cmd(uint8_t cmd);

void driver_send_data(uint8_t data);
void driver_fill(int16_t c, int16_t y, int16_t w, int16_t h);
void driver_pixel();

void lcd_reset();
void lcd_init();
void lcd_print(const unsigned char* text, uint32_t len);
