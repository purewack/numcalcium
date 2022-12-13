#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"

int midi_base = 0; //c3
extern double th;
int midi_cc = -1;
char midi_ccs[16];

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
    if(midi_cc == -1){
        snprintf(str,32,"MIDI Base: %d",midi_base);
        lcd_drawString(20,20,sys_font,str);
        snprintf(str,32,"  -      +      CC");
        lcd_drawString(0,64-8,sys_font,str);
    }
    else{
        snprintf(str,32,"MIDI CC: [%d] Val(%d)",midi_cc,midi_ccs[midi_cc]);
        lcd_drawString(0,20,sys_font,str);
        snprintf(str,32,"                Note");
        lcd_drawString(0,64-8,sys_font,str);
        lcd_drawRectSize(0,64-32,128,4);
        lcd_drawRectSize(0,64-31,midi_ccs[midi_cc],2);
    }
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
      else if(io.bscan_down & (1<<K_F3)){
        if(midi_cc != -1)
            midi_cc = -1;
        else
            midi_cc = 0;
        mode_midi_draw_base();
      }
      else{ 
        for(int i=0; i<20; i++){
            if(midi_cc == -1){
                if(io.bscan_down & (1<<i)) 
                    USB_midi.sendNoteOn(0,midi_base+i,127);
                if(io.bscan_up & (1<<i)) 
                    USB_midi.sendNoteOff(0,midi_base+i,0);
            }
            else{
                if(io.bscan_down & (1<<i)){
                    midi_cc = i; 
                    mode_midi_draw_base();
                }
            }
        }  
      }
      
      if(midi_cc != -1 && (io.turns_left || io.turns_right)){
        int turns = io.turns_right-io.turns_left;
        int vv = int(midi_ccs[midi_cc])+turns;
        if(vv < 0) vv = 0;
        if(vv > 127) vv = 127; 
        midi_ccs[midi_cc] = vv;
        USB_midi.sendControlChange(0,midi_cc,vv);
        io.turns_left = 0;
        io.turns_right = 0;
        mode_midi_draw_base();
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
