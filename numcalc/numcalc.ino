
//CMODE:
//numpad -> F1 = normal, F2 = Arrows, F3 = phone ascii

//calculator -> F1 = basic, F2 = sci, F3 = comp
  //calc ops
  //(,)
  //+,-,/,*
  //^, log10, log2, loge, sqrt 
  //e, pi,
  //abs
  //(a)sin, (a)cos, (a)tan, (hyp)
  //
  //comp
  //(,)
  //+,-,/,* integers
  //<<,>>
  //&,|,!,^
  //&&,||,
  //%
  //bit input

//midi -> F1 = launchpad, F2 = seq usb, F3 = seq uart midi

//comms monitor -> F1 = UART, F2 = SPI, F3 = I2C

//GPIO -> F1 = debounced sw, F2 = clock bits program like cpld, F3 = PWM channels

//PWM audio -> F1 = osc, F2 scope, F3 nodes

//ws28xx ?



//current stats:
//rom 40% 26.2kb / 64kb 
//ram 63% 13kb / 20.4kb r= 7.5kb

#define DEBUG
#include "include/sys.h"
#include "include/modes.h"
#include "include/comms.h"
#include <EEPROM.h>


void changeToProg(int i){
  if(&stats.progs[i] == stats.cprog) return;

  if(stats.cprog) stats.cprog->on_end();
  stats.cprog = &stats.progs[i];
  stats.cprog->on_begin(); 
  stats.fmode = 0;
  stats.c_i = i;
}

void lcdFade(int in){
  for(int i=0; i<1000; i++){ 
    delayMicroseconds(1000-i);
    digitalWrite(LCD_LIGHT, in);
    delayMicroseconds(i);
    digitalWrite(LCD_LIGHT, !in);
  }
  digitalWrite(LCD_LIGHT, in);
}

void resetInactiveTime(){
  stats.inactive_time = 0;
}

// void vTaskWorker(void* params){
//   while(1){
//     vTaskDelay(10);
//     if(stats.cprog->on_work)
//       stats.cprog->on_work();
//   }
// }

void vTaskScreen(void* params){
//
  u8g2.begin();
  u8g2.setContrast(82);
  u8g2.setFlipMode(1);
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font

  uint8_t i = 0;
  while(1){
    if(digitalRead(LCD_LIGHT))
      vTaskDelay(30);
    else{
      vTaskDelay(200);
      continue; 
    }

    if(stats.cprog_sel){
      u8g2.clearBuffer();
      u8g2.setCursor(0,15);
      u8g2.print("~~Programs~~");
      u8g2.drawHLine(0,16,128);
      u8g2.drawHLine(0,18,128);
      if(stats.c_i > -1){
        u8g2.setCursor(120,14);
        u8g2.print("^");
      }

      int ii = 0;
      for(int i=(stats.c_i); i<(stats.c_i + 3); i++){
        if(i >= P_COUNT) continue;
        u8g2_uint_t sel = (i == stats.c_i ? U8G2_BTN_INV|U8G2_BTN_BW1|U8G2_BTN_HCENTER : U8G2_BTN_BW1|U8G2_BTN_HCENTER);
        
        const char* text = (i==-1 ? "Power Off" : stats.progs[i].title);
        u8g2.drawButtonUTF8(64,32 + (16*ii), sel, 128, 0, 0, text);
        ii++;
      }
      
      u8g2.sendBuffer();
    }
    else{
      u8g2.clearBuffer();
      u8g2.setCursor(0,8);
      u8g2.print(stats.cprog->title);
      u8g2.drawHLine(0,9,128);
      u8g2.setCursor(0,64);

      if(stats.cprog->footer){
        u8g2.print(stats.cprog->footer);
        u8g2.drawHLine(0,64-9,128);
      }

      stats.cprog->on_gfx();
      u8g2.sendBuffer();
    }
  }
}

void vTaskKeyMux(void* params){
  while(1){
    hw.ok = digitalRead(B_OK);
    if(hw.ok != hw.oko){
      hw.oko = hw.ok;
      if(!hw.ok) continue;

#ifdef DEBUG
      Serial.println("ok");
#endif
      resetInactiveTime();

      if(!stats.cprog_sel) {
        stats.cprog_sel = 1;
      }
      else {
        if(stats.c_i == -1) {
          //system shut down
          vTaskEndScheduler();
          u8g2.clearBuffer();
          u8g2.sendBuffer();
          lcdFade(0);
          int pp = 0;
          for(auto a : stats.progs){
            if(&a == stats.cprog) break;
            pp++;
          }
          EEPROM.write(0,pp);
          digitalWrite(SYS_PDOWN, HIGH);
        }
        changeToProg(stats.c_i);
        stats.cprog_sel = 0;
      }


    }
    
    for(int r=0; r<5; r++){
      digitalWrite(hw.rows[r],HIGH);
      for(int c=0; c<4; c++){
        int I = c + (r*4);
        //if(hw.io[I].target[cmode]){
          hw.io[I].state = digitalRead(hw.cols[c]);

          if(hw.io[I].state && !hw.io[I].old_state){
            hw.io[I].dt = 0;
            if(stats.cprog->on_press)
              stats.cprog->on_press(I);
          }
          else if(!hw.io[I].state && hw.io[I].old_state){
            if(stats.cprog->on_release)
              stats.cprog->on_release(I);
          }
          
          if(hw.io[I].state != hw.io[I].old_state){
            hw.io[I].old_state = hw.io[I].state;
              #ifdef DEBUG
                    Serial.println(I);
              #endif
          }
          if(hw.io[I].state) hw.io[I].dt += 1;
      }
      digitalWrite(hw.rows[r],LOW);
    }

    digitalWrite(ROW_F, HIGH);
    
      hw.enc_a_old = hw.enc_a;
      hw.enc_b_old = hw.enc_b;
      hw.enc_a = digitalRead(SEG_A);
      hw.enc_b = digitalRead(SEG_B);
      //if(stats.cprog == 0) goto enc_clean;

      if(hw.enc_a && !hw.enc_b && !hw.enc_a_old && !hw.enc_b_old){
  #ifdef DEBUG
        Serial.println("LEFT turn");
  #endif
        
        resetInactiveTime();
        if(stats.cprog_sel){
          stats.c_i = stats.c_i > 0 ? stats.c_i-1 : -1;
        }
        else if(stats.cprog->on_nav)
          stats.cprog->on_nav(-1);

      }
      else if(!hw.enc_a && hw.enc_b && !hw.enc_a_old && !hw.enc_b_old){    
  #ifdef DEBUG
        Serial.println("RIGHT turn");
  #endif

        resetInactiveTime();
        if(stats.cprog_sel){
          stats.c_i = stats.c_i < P_COUNT-1 ? stats.c_i+1 : P_COUNT-1;
        }
        else if(stats.cprog->on_nav)
          stats.cprog->on_nav(1);

      }
    digitalWrite(ROW_F, LOW);

    if(stats.inactive_time == 0){
        digitalWrite(LCD_LIGHT, 1);
    }
    if(stats.inactive_time == stats.cprog->inactive_lim){
        if(stats.cprog_sel) {
          stats.cprog_sel = 0;
          stats.inactive_time = 0;
        }
        else
          digitalWrite(LCD_LIGHT, 0);
    }
    if(stats.inactive_time < stats.cprog->inactive_lim){
        stats.inactive_time += stats.cprog_sel ? 1 : stats.cprog->inactive_inc;
    }

    vTaskDelay(5);
    //delay(10);
  }
}

void setup(){
  disableDebugPorts();
  pinMode(SYS_PDOWN, OUTPUT);
  digitalWrite(SYS_PDOWN, LOW);

  Serial.begin(9600);
  
  pinMode(B_OK,INPUT_PULLDOWN);
  for(int i=0; i<4; i++)
    pinMode(hw.cols[i], INPUT_PULLDOWN);
  for(int i=0; i<6; i++)
    pinMode(hw.rows[i], OUTPUT);
  
  pinMode(LCD_LIGHT, OUTPUT);
  lcdFade(1);

  USBComposite.clear();
  USBComposite.setProductId(0x0031);
  HID.setTXPacketSize(64);
  HID.registerComponent();
  HID.setReportDescriptor(HID_KEYBOARD); 
  // USB_audio.setParameters(MIC_STEREO,32000);
  // USB_audio.registerComponent();
  USB_midi.setTXPacketSize(64);
  USB_midi.registerComponent();
  bool usb = USBComposite.begin();
  if(!usb) Serial.println("usb begin failed");
  else Serial.println("usb begin succ");

  stats.progs[P_NUMPAD].on_begin = mode_numpad_on_begin;
  stats.progs[P_NUMPAD].on_end = mode_numpad_on_end;
  stats.progs[P_NUMPAD].on_press = mode_numpad_on_press;
  stats.progs[P_NUMPAD].on_release = mode_numpad_on_release;
  stats.progs[P_NUMPAD].on_gfx = mode_numpad_on_gfx;
  stats.progs[P_NUMPAD].title = "Numpad";
  stats.progs[P_NUMPAD].footer = "NUM      DIR         --";
  stats.progs[P_NUMPAD].inactive_inc = 1;
  stats.progs[P_NUMPAD].inactive_lim = 800;

  stats.progs[P_MIDI].on_begin = mode_midi_on_begin;
  stats.progs[P_MIDI].on_end = mode_midi_on_end;
  stats.progs[P_MIDI].on_press = mode_midi_on_press;
  stats.progs[P_MIDI].on_release = mode_midi_on_release;
  stats.progs[P_MIDI].on_gfx = mode_midi_on_gfx;
  stats.progs[P_MIDI].title = "MIDI";
  stats.progs[P_MIDI].footer = "--       --       --";
  stats.progs[P_MIDI].inactive_inc = 1;
  stats.progs[P_MIDI].inactive_lim = 400;

  stats.progs[P_COMMS].on_begin = mode_comms_on_begin;
  stats.progs[P_COMMS].on_end = mode_comms_on_end;
  stats.progs[P_COMMS].on_press = mode_comms_on_press;
  stats.progs[P_COMMS].on_release = mode_comms_on_release;
  stats.progs[P_COMMS].on_gfx = mode_comms_on_gfx;
  stats.progs[P_COMMS].title = "Comms";
  stats.progs[P_COMMS].footer = "UART      SPI       I2C";
  stats.progs[P_COMMS].inactive_inc = 0;
  stats.progs[P_COMMS].inactive_lim = 800;

  uint16_t p = 0;
  EEPROM.read(0,&p);
  if(p >= P_COUNT) p = 0;
  changeToProg(p);

  xTaskCreate(vTaskKeyMux,"key_mux",configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY,NULL);
  //xTaskCreate(vTaskWorker,"worker",configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY,NULL);
  xTaskCreate(vTaskScreen,"screen",configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY,NULL);
  vTaskStartScheduler();
}

void loop(){ }
