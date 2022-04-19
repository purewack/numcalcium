#include "include/comms.h"
#include "include/sys.h"
#include "include/modes.h"

void mode_numpad_on_begin(){
    Serial.println("USB HID Setup");
    USBComposite.setProductId(0x0031);
    HID.begin(HID_KEYBOARD);
    Keyboard.begin();
    delay(1000);
    Serial.println("USB HID Done");
}

void mode_numpad_on_press(int i){
    Serial.println("press");
    switch(i){
        case K_Y:
            Keyboard.press(KEY_LEFT_ARROW);
        break; 
        
        case K_0:
            Keyboard.press(KEY_DOWN_ARROW);
        break;

        case K_DOT:
            Keyboard.press(KEY_RIGHT_ARROW);
        break;

        case K_2:
            Keyboard.press(KEY_UP_ARROW);
        break;
    }
}

void mode_numpad_on_release(int i){
    switch(i){
        case K_Y:
            Keyboard.release(KEY_LEFT_ARROW);
        break; 
        
        case K_0:
            Keyboard.release(KEY_DOWN_ARROW);
        break;

        case K_DOT:
            Keyboard.release(KEY_RIGHT_ARROW);
        break;

        case K_2:
            Keyboard.release(KEY_UP_ARROW);
        break;
    }}