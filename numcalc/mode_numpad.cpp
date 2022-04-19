
#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"

char numpad_keys[20*2] = {
    KEY_BACKSPACE,'0','.','=',
    '1','2','3','+',
    '4','5','6','-',
    '7','8','9','/',
    0,0,0,'*',

    KEY_LEFT_ARROW,KEY_DOWN_ARROW,KEY_RIGHT_ARROW,KEY_RETURN,
    ' ',KEY_UP_ARROW,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,KEY_BACKSPACE
};


void mode_numpad_on_begin(){
    USBComposite.setProductId(0x0031);
    HID.begin(HID_KEYBOARD);
    Keyboard.begin();
}

void mode_numpad_on_end(){
    HID.end();
    Keyboard.end();
}


void mode_numpad_on_press(int i){
    if(stats.fmode == 2){
        return;
    }
    int ii = i + stats.fmode*20;
    char key = numpad_keys[ii];
    if(key) Keyboard.press(key);
}

void mode_numpad_on_release(int i){
    if(i == K_F1) {
        stats.fmode = 0; 
        resetInactiveTime();
        return;
    }
    if(i == K_F2) {
        stats.fmode = 1; 
        resetInactiveTime();
        return;
    }
    if(stats.fmode == 2){
        return;
    }
    int ii = i + stats.fmode*20;
    char key = numpad_keys[ii];
    if(key) Keyboard.release(key);
}

void mode_numpad_on_gfx(){
    u8g2.setCursor(8, 32);
    if(stats.fmode == 0) u8g2.print("> Numpad");
    else u8g2.print("> Arrow Keys");
}