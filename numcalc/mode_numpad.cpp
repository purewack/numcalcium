
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

volatile int numpad_inactive_time;
#define INACTIVE_TIME_LIMIT 400

void mode_numpad_on_begin(){
    Serial.println("USB HID Setup");
    USBComposite.setProductId(0x0031);
    HID.begin(HID_KEYBOARD);
    Keyboard.begin();
    delay(1000);
    Serial.println("USB HID Done");
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
        numpad_inactive_time = 0;
        return;
    }
    if(i == K_F2) {
        stats.fmode = 1; 
        numpad_inactive_time = 0;
        return;
    }
    if(stats.fmode == 2){
        return;
    }
    int ii = i + stats.fmode*20;
    char key = numpad_keys[ii];
    if(key) Keyboard.release(key);
}

void mode_numpad_on_work(){
    if(numpad_inactive_time == 0){
        digitalWrite(LCD_LIGHT, 1);
    }
    if(numpad_inactive_time == INACTIVE_TIME_LIMIT){
        digitalWrite(LCD_LIGHT, 0);
    }
    if(numpad_inactive_time < INACTIVE_TIME_LIMIT){
        numpad_inactive_time++;
    }
}

void mode_numpad_on_gfx(){
    u8g2.setCursor(16, 32);
    if(stats.fmode == 0) u8g2.print(">Stock Numpad");
    else u8g2.print(">Directional Keys");
}