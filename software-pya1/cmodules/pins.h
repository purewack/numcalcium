#ifndef BOARD_PINS_H
#define BOARD_PINS_H


// py-a0
//#define BOARD_PIN_LCD_CS    42
//#define BOARD_PIN_SD_CS     38
//#define BOARD_PIN_LCD_DC    45

//#define BOARD_PIN_LEDS      47
//#define BOARD_PIN_LCD_LED   46
//#define BOARD_PIN_BUZZER    21
//#define BOARD_PIN_A_OUT_L   43
//#define BOARD_PIN_A_OUT_R   44

//Internal
#define BOARD_PIN_LCD_CS    42
#define BOARD_PIN_SD_CS     38
#define BOARD_PIN_LCD_DC    46

#define BOARD_PIN_LEDS      45
#define BOARD_PIN_LCD_LED   9

#define BOARD_PIN_BUZZER    10
#define BOARD_PIN_A_OUT_L   47
#define BOARD_PIN_A_OUT_R   48


#define BOARD_PIN_SLEEP_REQ 16
#define BOARD_PIN_VBAT_MON  15
#define BOARD_PIN_CHR_STATE 21


#define BOARD_PIN_CK        39
#define BOARD_PIN_MOSI      40
#define BOARD_PIN_MISO      41

#define BOARD_PIN_RX1       18
#define BOARD_PIN_TX1       17
#define BOARD_PIN_RX0       44
#define BOARD_PIN_TX0       43

//Port plug
#define BOARD_PIN_PORT1     1
#define BOARD_PIN_PORT2     2
#define BOARD_PIN_PORT3     3
#define BOARD_PIN_PORT4     4
#define BOARD_PIN_PORT5     5
#define BOARD_PIN_PORT6     6
#define BOARD_PIN_PORT7     7
#define BOARD_PIN_PORT8     8

#define BOARD_PIN_PORT9     BOARD_PIN_TX1
#define BOARD_PIN_PORT10    BOARD_PIN_RX1
#define BOARD_PIN_PORT11    BOARD_PIN_TX0
#define BOARD_PIN_PORT12    BOARD_PIN_RX0

#define BOARD_PIN_PORT_SCL  BOARD_PIN_RX0
#define BOARD_PIN_PORT_SDA  BOARD_PIN_TX0

// IO keypad + encoder
#define BOARD_PIN_IO_OE     11
#define BOARD_PIN_IO_CK     12
#define BOARD_PIN_IO_DD     13
#define BOARD_PIN_IO_TT     14
#define BOARD_PIN_B_OK      0

#endif
