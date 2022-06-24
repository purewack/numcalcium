
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
    KEY_BACKSPACE,KEY_UP_ARROW,' ',KEY_DELETE,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0
};


void mode_numpad_on_begin(){
    // stats.gfx_text[0] = "1.test";
    // stats.gfx_text[1] = "2.2";
    // stats.gfx_text[2] = ">3.test asdf";
    // stats.gfx_text_count = 3;
}

void mode_numpad_on_end(){

}

void mode_numpad_on_process(){

    if(stats.usb_state){
        for(int i=0; i<20; i++){
            if(io.bscan_down & (1<<i)) 
                USB_keyboard.press(numpad_keys[i + 20*stats.fmode]);
            if(io.bscan_up & (1<<i)) 
                USB_keyboard.release(numpad_keys[i + 20*stats.fmode]);
        }
        lcd_drawString(110,0, sys_font, "USB");
        if(stats.fmode == 0){
            lcd_drawString(16,24, sys_font, "Default keys");
        }
        else{
            lcd_drawString(16,24, sys_font, "BP | ^ | SP | DEL");
            lcd_drawString(16,32, sys_font, "<  | V | >  | RET");
        }     
    }
    else{
        lcd_drawString(16,32, sys_font, "Connecting...");
    }

    if(io.bscan_down & (1<<K_F1)) stats.fmode = 0;
    if(io.bscan_down & (1<<K_F2)) stats.fmode = 1;
    if(io.bscan_down) io.bscan_down = 0;
    if(io.bscan_up) io.bscan_up = 0;

    
    if(!stats.usb_state){
        USBComposite.clear();
        USBComposite.setProductId(0x0031);
        HID.registerComponent();
        HID.setReportDescriptor(HID_KEYBOARD); 
        USB_midi.registerComponent();
        bool usb = USBComposite.begin();
        if(!usb) LOGL("usb begin failed");
        else LOGL("usb begin succ");
        stats.usb_state = usb;
    }
}