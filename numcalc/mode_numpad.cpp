
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
    LOGL("numpad begin");
}

void mode_numpad_on_end(){

}


// int mode_numpad_on_press(int i){
//     if(stats.fmode == 2){
//         return 1;
//     }
//     int ii = i + stats.fmode*20;
//     char key = numpad_keys[ii];
//     if(key) USB_keyboard.press(key);
//     return 0;
// }

// int mode_numpad_on_release(int i){
//     if(i == K_F1) {
//         stats.fmode = 0; 
//         resetInactiveTime();
//         return 1;
//     }
//     if(i == K_F2) {
//         stats.fmode = 1; 
//         resetInactiveTime();
//         return 1;
//     }
//     if(stats.fmode == 2){
//         return 1;
//     }
//     int ii = i + stats.fmode*20;
//     char key = numpad_keys[ii];
//     if(key) USB_keyboard.release(key);
//     return 0;
// }

void mode_numpad_on_process(){

    if(io.bscan_down & (1<<K_F1)) stats.fmode = 0;
    if(io.bscan_down & (1<<K_F2)) stats.fmode = 1;
    if(io.bscan_down) io.bscan_down = 0;
    
    if(stats.fmode == 0){
        lcd_drawString(16,24, sys_font, "Default keys");
    }
    else{
        lcd_drawString(16,24, sys_font, "BP | ^ | SP | DEL");
        lcd_drawString(16,32, sys_font, "<  | V | >  | RET");
    }     
    
}