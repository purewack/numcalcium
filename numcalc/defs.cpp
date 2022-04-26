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



vnum_t keypad_num{0};
bool keypad_num_inputting = false;

void enterNumberInput(vnum_t &n){
    if(n.m_dc > 0) n.m_dc *= -1;
    if(n.e_dc > 0) n.e_dc *= -1;
}

void endNumberInput(vnum_t &n){
    if(n.e_dc < 0) n.e_dc *= -1;
    if(n.m_dc < 0) n.m_dc *= -1;
    auto e = double(n.e_int);
    auto m = double(n.m_int);
    auto mm = m / pow(10.0,n.m_dc);
    n.result = e;
    n.result += mm;
}

void numberInputKey(vnum_t& n, uint32_t i){
    if(i == K_DOT && n.m_dc == 0) {
        n.e_dc *= -1;
        if(n.e_dc == 0) n.e_dc = 1;
        n.m_dc = -1;
        return;
    }

    if(n.m_dc == -1&& n.m_int == 0 && i != K_0) n.m_dc = 0;
    //if(n.e_dc == 1 && n.e_int == 0) n.e_dc = 0;

    const uint32_t key_map[10] = {
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

    int ii = 0;
    char cc = '0';
    for(int j=0; j<10; j++){
        if(i == key_map[j]) {
            ii = j;
            cc += j;
        }
    }

    if(n.e_dc <= 0){
        n.e_int *= 10.0;
        n.e_int += ii;
        n.e_dc -= 1;
    }
    else{
        n.m_int *= 10.0;
        n.m_int += ii;
        n.m_dc -= 1;
    }

}

void numberInputBackspace(vnum_t &n){

    if(n.m_dc < 0){
        n.m_dc++;
        n.m_int /= 10;
        if(n.m_int == 0 && n.e_dc > 0) 
            n.e_dc *= -1;
    }
    else{
        if(n.e_dc == 0) return;
        n.e_dc++; 
        n.e_int /= 10;
    }

}


void print_vnum(vnum_t& n, int &x, const int y){
    
    int cc = n.m_dc ? 2 : 1;
    Serial.println('{');
    Serial.println(n.e_dc);
    Serial.println(n.m_dc);
    Serial.println(n.e_int);
    Serial.println(n.m_int);
    Serial.println("}\n");
   
    for(int c=0; c<cc; c++){
        auto nnn = c == 0 ? n.e_int : n.m_int;
        auto dcc = c == 0 ? n.e_dc : n.m_dc;
        if(dcc < 0) dcc *= -1;
        for(int j=0; j<dcc; j++){
            int a = nnn;
            int b = nnn;
            int t = dcc-j-1;
            
            for(int i=0; i<t+1; i++)
                a /= 10;
            a *= 10;
            for(int i=0; i<t; i++)
                b /= 10;
            
            char c = char(b)-char(a);
            c += '0';

            u8g2.setCursor(x,y);
            u8g2.print(c);
            x+=6;
        }

        if(c == 0 && n.m_dc != 0) {
            u8g2.setCursor(x,y);
            u8g2.print('.');
            x+=6;
        }
    } 

}