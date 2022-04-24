#include "include/sys.h"
#include "include/comms.h"
#include "include/util.h"
#include "include/number.h"

//conflicting miso pin being pinmoded to input after ::sendBuffer()
U8G2_ST7565_ERC12864_ALT_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ LCD_CK, /* data=*/ LCD_MOSI, /* cs=*/ LCD_CS, /* dc=*/ LCD_DC, /* reset=*/ LCD_RST);
//U8G2_ST7565_ERC12864_ALT_F_4W_SW_SPI u8g2(U8G2_R0, /* cs=*/ LCD_CS, /* dc=*/ LCD_DC, /* reset=*/ LCD_RST);
U8G2LOG u8g2log;
uint8_t u8log_buffer[32*7]; 

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
    0,0,0,0,0,0,0
};




vnum_t keypad_num = {
    false,
    false,
    0.0,
    0,0,
    0.0,0.0,
    {0}
};

static unsigned int keypad_num_lim = 50;
char keypad_num_buf[50];

void startInputNumber(){
    keypad_num.input = true;
    keypad_num.dot = false;
    keypad_num.result = 0.0;
    keypad_num.d_mantissa = 0.0;
    keypad_num.d_expo = 0.0;
    keypad_num.mantissa = 0;
    keypad_num.expo = 0;
    keypad_num.rep.buf = keypad_num_buf;
    keypad_num.rep.lim = keypad_num_lim;
    keypad_num.rep.count = 0;
    for(int i=0; i<keypad_num_lim; i++)
        keypad_num.rep.buf[i] = '\0';
}
double getInputNumberResult(){
    keypad_num.input = false;
    keypad_num.result = keypad_num.d_expo;
    keypad_num.result += keypad_num.d_mantissa / pow(10.0,keypad_num.mantissa);
    return keypad_num.result;
}
char* getInputNumberRep(){
    keypad_num.input = false;
    //getInputNumberResult();
    return keypad_num.rep.buf;
}

void numberInputKey(int i){
    if(i == K_DOT && !keypad_num.dot) {
        sarray_push(keypad_num.rep,'.');
        keypad_num.dot = true;
        return;
    }

    const int key_map[10] = {
        K_0,
        K_1,
        K_2,
        K_3,
        K_4,
        K_5,
        K_6,
        K_7,
        K_8,
        K_9,
    };
    
    int ii = -1;
    char cc = '0';
    for(int j=0; j<10; j++){
        if(i == key_map[j]) {
            ii = j;
            cc += j;
        }
    }
    if(ii == -1) return;

    sarray_push(keypad_num.rep,cc);
    
    if(!keypad_num.dot){
        keypad_num.d_expo *= 10.0;
        keypad_num.d_expo += double(ii);
        keypad_num.expo += 1;
    }
    else{
        keypad_num.d_mantissa *= 10.0;
        keypad_num.d_mantissa += double(ii);
        keypad_num.mantissa += 1;
    }

}

void numberInputBackspace(){

    if(keypad_num.dot){
        keypad_num.mantissa--;
        keypad_num.d_mantissa = floor(keypad_num.d_mantissa / 10.0);
        if(keypad_num.mantissa == 0) {
            keypad_num.dot = false; 
            keypad_num.rep.buf[keypad_num.rep.count-1] = '\0';
            keypad_num.rep.count--;
        }
        
    }
    else{
        if(keypad_num.expo == 0) return;
        keypad_num.expo--; 
        keypad_num.d_expo = floor(keypad_num.d_expo / 10.0);
    }
    
    keypad_num.rep.buf[keypad_num.rep.count-1] = '\0';
    keypad_num.rep.count--;
   
}