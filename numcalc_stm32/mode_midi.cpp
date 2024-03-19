#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"

int midi_base = 0; //c3
extern double th;
int midi_cc = -1;
char midi_ccs[16];
bool midi_serial = false;

void mode_midi_draw_base();

void mode_midi_send_noteon(char note){
  if(midi_serial){
    char bytes[3] = {0x90, 0x00, 0x7f};
    bytes[1] = midi_base+note;
    Serial.write(bytes,3);
  }
  else if(USBComposite){
    USB_midi.sendNoteOn(0,midi_base+note,127);
  }
}
void mode_midi_send_noteoff(char note){
  if(midi_serial){
    char bytes[3] = {0x80, 0x00, 0x00};
    bytes[1] = midi_base+note;
    Serial.write(bytes,3);
  }
  else if(USBComposite){
    USB_midi.sendNoteOff(0,midi_base+note,0);
  }
}
void mode_midi_send_cc(unsigned char cc,unsigned char val){
  if(midi_serial){
    unsigned char bytes[6];
    bytes[0] = 0xB0;
    bytes[1] = cc+16;
    bytes[2] = 0;
    bytes[3] = 0xB0;
    bytes[4] = cc+32+16;
    bytes[5] = val;
    Serial.write(bytes,6);
  }
  else if(USBComposite){
    USB_midi.sendControlChange(0,cc,val);
  }
}

void mode_midi_on_begin(){
  lcd_clear();
  drawTitle();
  lcd_update();

  if(!USBComposite){
    disconnectUSB();
    delay_us(100000);
    connectUSB();  //stats.cprog_sel = 1;
    delay_us(100000);
    USBComposite.begin();
  }


#ifdef DEBUG
  Serial.end();
#endif
  Serial.begin(31250);
  midi_serial = true;
  midi_base = 36;
  mode_midi_draw_base();
}

void mode_midi_on_end(){

  Serial.end();
#ifdef DEBUG
  Serial.begin(115200);
#endif

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

    lcd_drawString(10,64-20,sys_font,midi_serial ? "[MIDI -> Serial]" : "[MIDI -> USB]");
    updateProgGFX();
}

void mode_midi_on_process(){

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
    else if(io.bscan_down & (1<<K_X)){
      midi_serial = !midi_serial;
      mode_midi_draw_base();
    }

    else{ 
      for(int i=0; i<20; i++){
          if(midi_cc == -1){
              if(io.bscan_down & (1<<i)) 
                  mode_midi_send_noteon(i);
              if(io.bscan_up & (1<<i)) 
                  mode_midi_send_noteoff(i);
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
      mode_midi_send_cc(midi_cc,vv);
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

    

    if(USBComposite != stats.usb_state){
      stats.usb_state = USBComposite;
      lcd_clearSection(0,8,0,128,0);
      drawTitle();
      drawUSBStatus();
      lcd_updateSection(0,1,0,128);
      io.bscan_down |= (1<<K_F1);
    }

    delay_us(5000);
}
