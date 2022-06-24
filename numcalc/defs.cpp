#include "include/sys.h"
#include "include/comms.h"
#include "include/util.h"
#include "include/number.h"

#include "numcalcium-base/hw.cpp"
#include "numcalcium-base/fonttiny.cpp"

// USBHID HID;
// HIDKeyboard USB_keyboard(HID);
// USBMIDI USB_midi;
// USBAUDIO USB_audio;

// SPIClass SPI_2(2);
font_t sys_font = {
    fonttiny_tall,
    fonttiny_wide,
    (void*)fonttiny_data,
    fonttiny_data_count
};

stats_t stats = {
    nullptr,
    {0},
    0,0,0,0,0,0,0
};

double computeNumber(vnum_t &n){
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
        if(i == key_map[j]) {
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
        
        if(i == K_DOT && mdc == 0){
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
    
    
    
    if(i == K_DOT) {
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
    // LOGL("}\n");

    font_t fnt = {
      fonttiny_tall,
      fonttiny_wide,
      (void*)fonttiny_data,
      fonttiny_data_count
    };

    for(int c=0; c<cc; c++){
        if(c == 1 && n.dot) {
            lcd_drawChar(x,y,fnt,'.');
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
                lcd_drawChar(x,y,fnt,c);
            
            x+=6;
        }
    } 

}