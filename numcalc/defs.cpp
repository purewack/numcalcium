#include "include/sys.h"
#include "include/comms.h"

//conflicting miso pin being pinmoded to input after ::sendBuffer()
U8G2_ST7565_ERC12864_ALT_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ LCD_CK, /* data=*/ LCD_MOSI, /* cs=*/ LCD_CS, /* dc=*/ LCD_DC, /* reset=*/ LCD_RST);
//U8G2_ST7565_ERC12864_ALT_F_4W_SW_SPI u8g2(U8G2_R0, /* cs=*/ LCD_CS, /* dc=*/ LCD_DC, /* reset=*/ LCD_RST);
USBHID HID;
HIDKeyboard USB_keyboard(HID);
USBMIDI USB_midi;
USBAUDIO USB_audio;

SPIClass SPI_2(2);

hw_t hw = {
    {0},
    0,0,0,0,0,0,0,0,
    {ROW_A, ROW_B, ROW_C, ROW_D, ROW_E, ROW_F},
    {SEG_A, SEG_B, SEG_C, SEG_D}
};

stats_t stats = {
    nullptr,
    {0},
    0,0,0,0,0,
    nullptr
};
