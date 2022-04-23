#include "include/sys.h"
#include "include/comms.h"
#include "include/util.h"

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

template <typename data_T>
void add_space(darray_t<data_T> &a){
    const int bss = a.chunk_size;
    if(a.buf_len == 0){
        a.buf = (data_T*)malloc(sizeof(data_T)*bss);
        a.buf_len = bss;
        a.count = 0;
    }
    else if(a.count == a.buf_len){
        //printf("increse buf size %d -> %d\n", a.buf_len, a.buf_len+bss);
        a.buf_len += bss;
        a.buf = (data_T*)realloc(a.buf, sizeof(data_T) * (a.buf_len));
    }
    //printf("add_space\n");
}

template <typename data_T> 
void darray_clear(darray_t<data_T> &a){
    if(a.buf) free(a.buf);
    a.buf_len = 0;
    a.count = 0;
}

template <typename data_T> 
void darray_push(darray_t<data_T> &a, data_T t){
    add_space(a);

    a.buf[a.count] = t;
    a.count++;
    //printf("pushed\n");
    return;
}
template <typename data_T> 
void darray_pop(darray_t<data_T> &a){
    if(a.count == 0) return;
    a.count--;
}

template <typename data_T> 
void darray_insert(darray_t<data_T> &a, data_T t, unsigned int i){
    if(i >= a.count) return darray_push(a,t);
    
    if(a.count+1 > a.buf_len) add_space(a);
    
    for(int j=a.count; j>i; j--){
        auto aa = a.buf[j-1];
        a.buf[j] = aa;
    }
    a.buf[i] = t;
    a.count++;
    
    //printf("inserted\n");
    return;
}
template <typename data_T> 
void darray_remove(darray_t<data_T> &a, int i){
    if(a.count == 0) return;
    for(int j=i; j<(a.count); j++){
        data_T aa = a.buf[j+1];
        a.buf[j] = aa;
    }
    a.count--;
}


