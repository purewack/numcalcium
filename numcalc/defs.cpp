#include "include/sys.h"
#include "include/comms.h"
#include "include/util.h"
#include "include/number.h"

#include "numcalcium-base/hw.cpp"
#include "numcalcium-base/ftiny.cpp"

font_t sys_font = {
    ftiny_tall,
    ftiny_wide,
    (void*)ftiny_data,
    ftiny_count
};

stats_t stats;

// SPIClass SPI_2(2);

USBHID HID;
HIDKeyboard USB_keyboard(HID);
USBMIDI USB_midi;
USBAUDIO USB_audio;

void connectUSB(){
    gpio_set_mode(GPIOA, 13, GPIO_OUTPUT_PP);
    gpio_write_bit(GPIOA, 13, 1);
}

void disconnectUSB(){
    gpio_set_mode(GPIOA, 13, GPIO_OUTPUT_PP);
    gpio_write_bit(GPIOA, 13, 0);   
}

void drawUSBStatus(){
    if(USBComposite)
        lcd_drawString(110,0, sys_font, "USB");
}

void drawTitle(){
    lcd_drawString(0,0, sys_font, stats.cprog->title);
    lcd_drawHline(0,9,128);
}

void drawFooter(){
//     hud.setCursor(0,64);
//     if(stats.cprog->txt_f1){
//       u8g2_uint_t sel = (stats.fmode == 0 ? U8G2_BTN_INV|U8G2_BTN_BW1 : U8G2_BTN_BW1 );
//       hud.drawButtonUTF8(0,64, sel, 41, 0, 0, stats.cprog->txt_f1);
//     }
//     if(stats.cprog->txt_f2){
//       u8g2_uint_t sel = (stats.fmode == 1 ? U8G2_BTN_INV|U8G2_BTN_BW1 : U8G2_BTN_BW1 );
//       hud.drawButtonUTF8(42,64 , sel, 41, 0, 0, stats.cprog->txt_f2);
//     }
//     if(stats.cprog->txt_f3){
//       u8g2_uint_t sel = (stats.fmode == 2 ? U8G2_BTN_INV|U8G2_BTN_BW1 : U8G2_BTN_BW1 );
//       hud.drawButtonUTF8(84,64 , sel, 41, 0, 0, stats.cprog->txt_f3);
//     }

//     if(stats.cprog->onGfx)
//       stats.cprog->onGfx();

//     // if(stats.gfx_text_count){
//     //   for(int i=0; i<7;i++){
//     //     if(i > stats.gfx_text_count-1) break;
//     //       hud.setCursor(0, 64-10-(9*i));
//     //       hud.print(stats.gfx_text[stats.gfx_text_count-1-i]);
//     //   }
//     // }
}

void clearProgGFX(){
    lcd_clearSection(10,64-10,0,128,0);
}
void updateProgGFX(){
    lcd_updateSection(1,7,0,128);
}


double doubleFromNumber(vnum_t &n){
    auto e = double(n.e_int);
    auto m = double(n.m_int);
    auto mm = m / pow(10.0,n.m_dc);
    n.result = e + mm;
    return n.result;
}

void clearNumber(vnum_t &n){
    n.dot = false;
    n.m_dc = 0;
    n.e_dc = 0;
    n.e_int = 0;
    n.m_int = 0;
    n.result = 0.0;
}

vnum_t numberFromDouble(double dnum){
    vnum_t vnum = {0};
    clearNumber(vnum);
    vnum.result = dnum;

    if(dnum < 0.0){
        dnum *= -1.0;
    }
    
    vnum.e_int = uint64_t(dnum);
    auto eint = vnum.e_int;
    while(eint > 0){
        eint /= 10;
        vnum.e_dc++;
    }
    
    dnum -= double(vnum.e_int);
    if(dnum != 0.0) vnum.dot = true;
    for(int i=0; i<12; i++){
        dnum *= 10.0;
        vnum.m_int = uint64_t(dnum);
    }
    uint64_t mdc = 0;
    for(int i=0; i<12; i++){
        if(vnum.m_int & 0x1) {
            vnum.m_dc = 12-mdc;
            break;
        }
        mdc++;
        vnum.m_int /= 10;
    }
    return vnum;
}

void decimalInsert(uint64_t& n, uint16_t dc, int pos, int digit){
    int aa = n;
    for(int i=0; i<dc-pos+1; i++)
        aa /= 10;
        
    aa*=10;
    aa+=digit;
    
    for(int i=0; i<dc-pos+1; i++)
        aa *= 10;
    
    auto aaa = n;
    for(int i=0; i<dc-pos+1; i++)
        aaa /= 10;
    for(int i=0; i<dc-pos+1; i++)
        aaa *= 10;
    aaa = n - aaa;
    
    n = aa+aaa;
}

void decimalRemove(uint64_t& n, uint16_t dc, int pos){
    int aa = n;
    for(int i=0; i<dc-pos+1; i++)
        aa /= 10;
    for(int i=0; i<dc-pos; i++)
        aa *= 10;
    
    auto aaa = n;
    for(int i=0; i<dc-pos; i++)
        aaa /= 10;
    for(int i=0; i<dc-pos; i++)
        aaa *= 10;
    aaa = n - aaa;

    n = aa+aaa;
}


void numberInputKey(vnum_t& n, uint32_t i, int pos){
    if(i != K_DOT && !n.dot && n.e_dc == 16) return;
    if(n.dot && n.m_dc == 16) return;

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
        if(i == (1<<key_map[j])) {
            ii = j;
            cc += j;
        }
    }
    
    if(pos > 0){
        auto e = n.e_int;
        auto m = n.m_int;
        auto edc = n.e_dc;
        auto mdc = n.m_dc;
        auto dot = n.dot;
        int dp = n.dot ? edc+1 : -1;
        
        if(i == (1<<K_DOT) && mdc == 0){
            dot = true;
            if(pos > edc) pos = edc;
            
            int ee = e;
            for(int i=0; i<edc-pos; i++)
                ee /= 10;
            int eee = ee;
            for(int i=0; i<edc-pos; i++)
                eee *= 10;
            
            int mm = e-eee;
            e = ee;
            m = mm;
            mdc = edc-pos;
            edc = pos;
        }
        else if(pos <= edc){
            decimalInsert(e,edc,pos,ii);
            edc++;
        }
        else if(mdc > 0){
            pos -= edc+1;
            if(pos > mdc) pos = mdc;
            decimalInsert(m,mdc,pos,ii);
            mdc++;
        }
            
        
        n.dot = dot;
        n.e_int = e;
        n.m_int = m;
        n.e_dc = edc;
        n.m_dc = mdc;
        return;
    }
    
    
    
    if(i == (1<<K_DOT)) {
        if(n.dot) return;
        if(n.e_dc == 0) n.e_dc = 1;
        n.dot = true;
        return;
    }

    if(n.e_dc < 16 && !n.dot){
        n.e_int *= 10.0;
        n.e_int += ii;
        n.e_dc++;
    }
    else if(n.m_dc < 16){
        n.m_int *= 10.0;
        n.m_int += ii;
        n.m_dc++;
    }

}


bool numberInputBackspace(vnum_t& n, int pos){
    if(pos > 0){
        
        auto e = n.e_int;
        auto m = n.m_int;
        auto edc = n.e_dc;
        auto mdc = n.m_dc;
        auto dot = n.dot;
        int dp = n.dot ? edc+1 : -1;
        
        if(pos == dp && dp > -1){
            for(int i=0; i<mdc; i++)
                e *= 10;
                
            e += m;
            m = 0;
            edc = edc + mdc;
            mdc = 0;
            dot = false;
        }
        else{
            if(pos <= edc){
                decimalRemove(e,edc,pos);
                edc--;
            }
            else if(mdc > 0){
                pos -= edc+1;
                if(pos > mdc) pos = mdc;
                decimalRemove(m,mdc,pos);
                mdc--;
            }
            
            
        }
        
        n.dot = dot;
        n.e_int = e;
        n.m_int = m;
        n.e_dc = edc < 0 ? 0 : edc;
        n.m_dc = mdc < 0 ? 0 : mdc;
        LOGL(n.m_dc);
        LOGL(n.e_dc);
        return (n.m_dc == 0 && n.e_dc == 0);
    }
    
    if(n.m_dc > 0){
        n.m_dc--;
        n.m_int /= 10;
    }
    else{
        if(n.dot){
            n.dot = false;
            return false;
        }
        n.e_dc--; 
        n.e_int /= 10;
        if(n.e_dc == 0) {
            n.e_dc = 0;
            return true;
        }
    }
    return (n.e_dc == 0 && n.m_dc == 0);
}


int numberLength(vnum_t& n){
    int l = 0;
    l += (n.dot ? 1 : 0);
    l += n.e_dc;
    l += (n.m_dc == 0 && n.dot ? 1 : n.m_dc);
    return l;
}

void printNumber(vnum_t& n, int &x, const int y){

    if(n.e_dc == 0 && n.m_dc == 0) return;
    
    int cc = 2;//n.m_dc || keypad_num_dot ? 2 : 1;
    // LOGL('{');
    // LOGL(n.e_dc);
    // LOGL(n.m_dc);
    // LOGL(n.e_int);
    // LOGL(n.m_int);
    // LOGL("}\n")

    for(int c=0; c<cc; c++){
        if(c == 1 && n.dot) {
            lcd_drawChar(x,y,sys_font,'.');
            x+=6;
        }

        auto nnn = c == 0 ? n.e_int : n.m_int;
        auto dcc = c == 0 ? (n.dot && n.e_dc == 0 ? 1 : n.e_dc) : (n.dot && n.m_dc == 0 ? 1 : n.m_dc);
        for(int j=0; j<dcc; j++){
            uint64_t a = nnn;
            uint64_t b = nnn;
            int64_t t = dcc-j-1;
            
            for(int i=0; i<t+1; i++)
                a /= 10;
            a *= 10;
            for(int i=0; i<t; i++)
                b /= 10;

            char c = char(b)-char(a);
            c += '0';

            if(x>=0 || x<=128-6)
                lcd_drawChar(x,y,sys_font,c);
            
            x+=6;
        }
    } 

}
