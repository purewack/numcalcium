#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define COL_RED    0xf800
#define COL_GREEN  0x07e0
#define COL_BLUE   0x001f
#define COL_PURPLE 0xf81f
#define COL_YELLOW 0xffe0
#define COL_CYAN   0x07ff
#define COL_BLACK  0x0000
#define COL_WHITE  0xffff

// Pin definitions
#define TFT_BL     46
#define TFT_CS     42
#define TFT_DC     45
#define TFT_MOSI   40
#define TFT_SCLK   39

//lcd config 
#define Y_OFFSET    35
#define X_SIZE      320
#define Y_SIZE      170
#define X_CHAR      (X_SIZE/font_wide)
#define Y_CHAR      (Y_SIZE/font_tall)

// Utility functions for direct use
void driver_send_cmd(uint8_t cmd);
void driver_send_data(uint8_t data);

void driver_fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void driver_pixel(uint16_t x, uint16_t y, uint16_t color);

void lcd_reset();
void driver_init();

void driver_print(const unsigned char* text, const uint32_t len, int16_t *col, int16_t *line, const uint16_t color, const uint16_t bg, const uint8_t scale);
