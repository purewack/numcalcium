#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"

int midi_base = 0; //c3
extern double th;

void mode_midi_on_begin(){
  lcd_clear();
  drawTitle();
  lcd_update();
  io.bscan_down |= (1<<K_F1);

  if(!USBComposite){
    disconnectUSB();
    delay_us(100000);
    connectUSB();  //stats.cprog_sel = 1;
    delay_us(100000);
    USBComposite.begin();
  }
}

void mode_midi_on_end(){
  disconnectUSB();
  USBComposite.end();
  drawUSBStatus();
  lcd_updateSection(0,1,0,128);
}

void mode_midi_draw_base(){
    clearProgGFX();
      char str[32];
    snprintf(str,32,"MIDI Base: %d",midi_base);
    lcd_drawString(20,20,sys_font,str);
    snprintf(str,32,"  -      +");
    lcd_drawString(0,64-8,sys_font,str);
    updateProgGFX();
}

void mode_midi_on_process(){
    if(USBComposite){
      if(io.bscan_down & (1<<K_F1)) {
        midi_base -= 1;
        mode_midi_draw_base();
      }
      else if(io.bscan_down & (1<<K_F2)) {
        midi_base += 1;
        mode_midi_draw_base();
      }
      else{ 
        for(int i=0; i<20; i++){
            if(io.bscan_down & (1<<i)) 
                USB_midi.sendNoteOn(0,midi_base+i,127);
            if(io.bscan_up & (1<<i)) 
                USB_midi.sendNoteOff(0,midi_base+i,0);
        }  
      }
      
      if(io.bscan_down) {
        io.bscan_down = 0;
      }
      if(io.bscan_up) {
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
      midi_base = 48;
    }

    delay_us(5000);
}
