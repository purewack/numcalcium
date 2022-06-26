#include "numcalcium-base/hw.h"
#include "include/sys.h"
#include "include/modes.h"
#include "include/comms.h"
#include <EEPROM.h>

void powerOff(int fade){
  uint16_t pp = 0;
  for(int i=0; i<P_COUNT; i++){
    if(&stats.progs[i] == stats.cprog) break;
    pp++;
  }
  EEPROM.write(0,pp);
  EEPROM.write(1,stats.fmode);
  delay(500);

  lcd_clear();
  lcd_update();
  if(fade) lcd_fade(0);
  
  digitalWrite(SYS_PDOWN, HIGH);
}

void changeToProg(int i){
  if(&stats.progs[i] == stats.cprog) return;

  if(stats.cprog){ 
    if(stats.cprog->onEnd) 
      stats.cprog->onEnd();
  }

  stats.cprog = &stats.progs[i];

  stats.fmode = 0;
  stats.c_i = i;
  stats.gfx_log = 0;

  if(stats.cprog){
    if(stats.cprog->onBegin) 
    stats.cprog->onBegin(); 
  }
}

void resetInactiveTime(){
  stats.inactive_time = 0;
  stats.no_input_time = 0;
}


  


    // if(io.ok != oko){
    //   oko = io.ok;
    //   stats.gfx_refresh |= 1;

    //   if(!io.ok) continue;

    //   #ifdef DEBUG
    //   LOGL("ok");
    //   #endif
    //   resetInactiveTime();

    //   if(!stats.cprog_sel) {
    //     stats.cprog_sel = 1;
    //   }
    //   else {
    //     if(stats.c_i == -1) {
    //       powerOff(1);
    //     }
    //     changeToProg(stats.c_i);
    //     stats.cprog_sel = 0;
    //   }
    // }

    // if(stats.inactive_time == 0){
    //     digitalWrite(LCD_LIGHT, 1);
    // }
    // if(stats.inactive_time == stats.cprog->inactive_lim){
    //     if(stats.cprog_sel) {
    //       stats.cprog_sel = 0;
    //       stats.inactive_time = 0;
    //     }
    //     else
    //       digitalWrite(LCD_LIGHT, 0);

    // }
    
    // if(stats.inactive_time < stats.cprog->inactive_lim){
    //     stats.inactive_time += stats.cprog_sel ? 1 : stats.cprog->inactive_inc;
    // }

    // stats.no_input_time++;
    // if(stats.cprog->no_input_lim){
    //   if(stats.no_input_time >= stats.cprog->no_input_lim) powerOff(0);
    // }

    // if(stats.cprog->onLoop && !stats.cprog_sel)
    //   stats.cprog->onLoop(dt);

  

void setup(){
  Serial.begin(9600);

  stats.progs[P_NUMPAD].onBegin = mode_numpad_on_begin;
  stats.progs[P_NUMPAD].onEnd = mode_numpad_on_end;
  stats.progs[P_NUMPAD].onProcess = mode_numpad_on_process;
  stats.progs[P_NUMPAD].title = "Keyboard";
  stats.progs[P_NUMPAD].txt_f1 = "123";
  stats.progs[P_NUMPAD].txt_f2 = "< ^ >";
  stats.progs[P_NUMPAD].txt_f3 = nullptr;
  stats.progs[P_NUMPAD].inactive_inc = 1;
  stats.progs[P_NUMPAD].inactive_lim = 800;
  stats.progs[P_NUMPAD].no_input_lim = 240000; //20mins

  // stats.progs[P_CALC].onBegin = mode_calc_on_begin;
  // stats.progs[P_CALC].onEnd = mode_calc_on_end;
  // stats.progs[P_CALC].onPress = mode_calc_on_press;
  // stats.progs[P_CALC].onRelease = mode_calc_on_release;
  // stats.progs[P_CALC].onGfx = mode_calc_on_gfx;
  // stats.progs[P_CALC].onNav = mode_calc_on_nav;
  stats.progs[P_CALC].title = "Calculator";
  stats.progs[P_CALC].txt_f1 = "SCI";
  stats.progs[P_CALC].txt_f2 = "BIN";
  stats.progs[P_CALC].txt_f3 = nullptr;
  stats.progs[P_CALC].inactive_inc = 1;
  stats.progs[P_CALC].inactive_lim = 10000;
  stats.progs[P_CALC].no_input_lim = 240000; //20mins

  // stats.progs[P_MIDI].onBegin = mode_midi_on_begin;
  // stats.progs[P_MIDI].onEnd = mode_midi_on_end;
  // stats.progs[P_MIDI].onPress = mode_midi_on_press;
  // stats.progs[P_MIDI].onRelease = mode_midi_on_release;
  stats.progs[P_MIDI].title = "MIDI";
  stats.progs[P_MIDI].txt_f1 = nullptr;
  stats.progs[P_MIDI].txt_f2 = nullptr;
  stats.progs[P_MIDI].txt_f3 = nullptr;
  stats.progs[P_MIDI].inactive_inc = 1;
  stats.progs[P_MIDI].inactive_lim = 400;
  stats.progs[P_MIDI].no_input_lim = 240000; //20mins

  // stats.progs[P_COMMS].onBegin = mode_comms_on_begin;
  // stats.progs[P_COMMS].onEnd = mode_comms_on_end;
  // stats.progs[P_COMMS].onPress = mode_comms_on_press;
  // stats.progs[P_COMMS].onRelease = mode_comms_on_release;
  // stats.progs[P_COMMS].onLoop = mode_comms_on_loop;
  // stats.progs[P_COMMS].onGfx = mode_comms_on_gfx;
  stats.progs[P_COMMS].title = "Comms";
  stats.progs[P_COMMS].txt_f1 = "UART";
  stats.progs[P_COMMS].txt_f2 = "SPI";
  stats.progs[P_COMMS].txt_f3 = "I2C";
  stats.progs[P_COMMS].inactive_inc = 0;
  stats.progs[P_COMMS].inactive_lim = 800;

  base_init();
  io_mux_init();

  lcd_fade(1);
  

  USBComposite.clear();
  USBComposite.setProductId(0x0031);
  HID.registerComponent();
  HID.setReportDescriptor(HID_KEYBOARD); 
  USB_midi.registerComponent();

  

  // uint16_t p = 0;
  // uint16_t f = 0;
  // EEPROM.read(0,&p);
  // EEPROM.read(1,&f);
  // if(p >= P_COUNT) p = 0;
  // if(f > 2) f = 0;

  LOGL("seq start");
  changeToProg(0);
  stats.fmode = 0;
  LOGL("sched start");

}

void loop(){
  if(stats.cprog_sel){
    lcd_clear();
    lcd_drawString(0,0,sys_font,"~~Programs~~");
    lcd_drawHline(0,8,128);
    lcd_drawHline(0,10,128);
    if(stats.c_i > -1)
      lcd_drawString(120,14,sys_font,"^");

    int ii = 0;
    for(int i=(stats.c_i); i<(stats.c_i + 3); i++){
      if(i >= P_COUNT) continue;
      // u8g2_uint_t sel = (i == stats.c_i ? U8G2_BTN_INV|U8G2_BTN_BW1|U8G2_BTN_HCENTER : U8G2_BTN_BW1|U8G2_BTN_HCENTER);
      
      const char* text = (i==-1 ? "Power Off" : stats.progs[i].title);
      // hud.drawButtonUTF8(64,32 + (16*ii), sel, 128, 0, 0, text);
      lcd_drawString(16,16 + (16*ii), sys_font, text);
      if(i == stats.c_i) 
      {
        //lcd_drawRectSize(0,(16*ii), 126, 16);
        lcd_drawHline(16, 18 + 8 + (16*ii), 126-32);
      }
      ii++;
    }
    lcd_update();
  }
  else{  
    if(stats.cprog){
      if(stats.cprog->onProcess) 
      stats.cprog->onProcess();
    }
  }

}
