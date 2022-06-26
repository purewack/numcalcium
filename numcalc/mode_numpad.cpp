
#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"


const char numpad_keys[20*2] = {
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
  lcd_clear();
  drawTitle();
  lcd_update();
  io.bscan_down |= (1<<K_F1);

  disconnectUSB();
  delay(100);
  connectUSB();  //stats.cprog_sel = 1;
  delay(100);
  USBComposite.begin();
  //while(!USBComposite) delay(1);
}

void mode_numpad_on_end(){
  drawUSBStatus();
  lcd_updateSection(0,1,0,128);
}

double th = 0.0;
void mode_numpad_on_process(){
    if(USBComposite){
      for(int i=0; i<20; i++){
          if(io.bscan_down & (1<<i)) 
              USB_keyboard.press(numpad_keys[i + 20*stats.fmode]);
          if(io.bscan_up & (1<<i)) 
              USB_keyboard.release(numpad_keys[i + 20*stats.fmode]);
      }   

      if(io.bscan_down & (1<<K_F1)) {
        stats.fmode = 0;
        clearProgGFX();
          lcd_drawString(16,24, sys_font, "Default keys");
        updateProgGFX();
      }
      if(io.bscan_down & (1<<K_F2)) {
        stats.fmode = 1;
        clearProgGFX();
          lcd_drawString(16,24, sys_font, "BP | ^ | SP | DEL");
          lcd_drawString(16,32, sys_font, "<  | V | >  | RET");
        updateProgGFX();
      }

      if(io.bscan_down) {
        LOGL(io.bscan_down);
        io.bscan_down = 0;
      }
      
      if(io.bscan_up) {
        LOGL(io.bscan_up);
        io.bscan_up = 0;
      }
    }
    else{
      clearProgGFX();
      lcd_drawLine(64 + int(14.0*cos(-th)),32 + int(14.0*sin(-th)),64 + int(14.0*cos(-th-3.1415)),32 + int(14.0*sin(-th-3.1415)));
      lcd_drawLine(64 + int(20.0*cos(th+2.09)),32 + int(20.0*sin(th+2.09)),64 + int(20.0*cos(th+3.1415+2.09)),32 + int(20.0*sin(th+3.1415+2.09)));
      lcd_drawLine(64 + int(6.0*cos(th+4.18)),32 + int(6.0*sin(th+4.18)),64 + int(6.0*cos(th+3.1415+4.18)),32 + int(6.0*sin(th+3.1415+4.18)));
      updateProgGFX();
      th += 0.007;
      
    }

    if(USBComposite != stats.usb_state){
      stats.usb_state = USBComposite;
      lcd_clearSection(0,8,0,128,0);
      drawTitle();
      drawUSBStatus();
      lcd_updateSection(0,1,0,128);
      io.bscan_down |= (1<<K_F1);
    }

    delay(5);
}
