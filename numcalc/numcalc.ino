
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


#define DEBUG
#include "include/sys.h"
#include "include/modes.h"
#include "include/comms.h"
#include <EEPROM.h>
//#include <MapleFreeRTOS900.h>

void vTaskEndScheduler(){
}
void vTaskDelay(int d){
  delay(d);
}

void powerOff(int fade){
  uint16_t pp = 0;
  for(int i=0; i<P_COUNT; i++){
    if(&stats.progs[i] == stats.cprog) break;
    pp++;
  }
  EEPROM.write(0,pp);
  EEPROM.write(1,stats.fmode);
  delay(500);

  vTaskEndScheduler();
  u8g2.clearBuffer();
  u8g2.sendBuffer();
  if(fade) lcdFade(0);
  
  digitalWrite(SYS_PDOWN, HIGH);
}

void changeToProg(int i){
  if(&stats.progs[i] == stats.cprog) return;

  if(stats.cprog) 
    if(stats.cprog->onEnd) 
      stats.cprog->onEnd();

  stats.cprog = &stats.progs[i];

  stats.fmode = 0;
  stats.c_i = i;
  stats.gfx_log = 0;

  if(stats.cprog->onBegin) 
    stats.cprog->onBegin(); 
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
  stats.no_input_time = 0;
}

// void vTaskWorker(void* params){
//   while(1){
//     vTaskDelay(10);
//     if(stats.cprog->on_work)
//       stats.cprog->on_work();
//   }
// }

void vTaskScreen(void* params){
  //if(!digitalRead(LCD_LIGHT)) return;
//
  //while(1){
    // if(digitalRead(LCD_LIGHT))
    //   vTaskDelay(30);
    // else{
    //   //vTaskDelay(200);
    //   return;
    // }

    u8g2.clearBuffer();
    u8g2.setFont(LCD_DEF_FONT);

  
    if(stats.cprog_sel){
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
      
    }
    else{  
      u8g2.setFont(LCD_DEF_FONT);
      u8g2.setCursor(0,8);
      u8g2.print(stats.cprog->title);
      u8g2.drawHLine(0,9,128);
      u8g2.setCursor(0,64);

      if(stats.cprog->txt_f1){
        u8g2_uint_t sel = (stats.fmode == 0 ? U8G2_BTN_INV|U8G2_BTN_BW1 : U8G2_BTN_BW1 );
        u8g2.drawButtonUTF8(0,64, sel, 41, 0, 0, stats.cprog->txt_f1);
      }
      if(stats.cprog->txt_f2){
        u8g2_uint_t sel = (stats.fmode == 1 ? U8G2_BTN_INV|U8G2_BTN_BW1 : U8G2_BTN_BW1 );
        u8g2.drawButtonUTF8(42,64 , sel, 41, 0, 0, stats.cprog->txt_f2);
      }
      if(stats.cprog->txt_f3){
        u8g2_uint_t sel = (stats.fmode == 2 ? U8G2_BTN_INV|U8G2_BTN_BW1 : U8G2_BTN_BW1 );
        u8g2.drawButtonUTF8(84,64 , sel, 41, 0, 0, stats.cprog->txt_f3);
      }

      if(stats.cprog->onGfx)
        stats.cprog->onGfx();

      // if(stats.gfx_text_count){
      //   for(int i=0; i<7;i++){
      //     if(i > stats.gfx_text_count-1) break;
      //       u8g2.setCursor(0, 64-10-(9*i));
      //       u8g2.print(stats.gfx_text[stats.gfx_text_count-1-i]);
      //   }
      // }

    }

    u8g2.sendBuffer();
  // }
}

void vTaskKeyMux(void* params){
  unsigned long dt = 0;

  while(1){
    
    hw.ok = digitalRead(B_OK);
    if(hw.ok != hw.oko){
      hw.oko = hw.ok;
      stats.gfx_refresh |= 1;

      if(!hw.ok) continue;

      #ifdef DEBUG
      Serial.println("ok");
      #endif
      resetInactiveTime();

      if(!digitalRead(LCD_LIGHT)) continue;

      if(!stats.cprog_sel) {
        stats.cprog_sel = 1;
      }
      else {
        if(stats.c_i == -1) {
          powerOff(1);
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
            if(stats.cprog->onPress)
              stats.gfx_refresh |= stats.cprog->onPress(I);
          }
          else if(!hw.io[I].state && hw.io[I].old_state){
            if(stats.cprog->onRelease)
              stats.gfx_refresh |= stats.cprog->onRelease(I);
          }
          
          if(hw.io[I].state != hw.io[I].old_state){
            hw.io[I].old_state = hw.io[I].state;
            stats.no_input_time = 0;
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
        else if(stats.cprog->onNav)
          stats.cprog->onNav(-1);

        stats.gfx_refresh |= 1;
      }
      else if(!hw.enc_a && hw.enc_b && !hw.enc_a_old && !hw.enc_b_old){    
        #ifdef DEBUG
        Serial.println("RIGHT turn");
        #endif

        resetInactiveTime();
        if(stats.cprog_sel){
          stats.c_i = stats.c_i < P_COUNT-1 ? stats.c_i+1 : P_COUNT-1;
        }
        else if(stats.cprog->onNav)
          stats.cprog->onNav(1);

        stats.gfx_refresh |= 1;
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

    stats.no_input_time++;
    if(stats.cprog->no_input_lim){
      if(stats.no_input_time >= stats.cprog->no_input_lim) powerOff(0);
    }

    if(stats.cprog->onLoop && !stats.cprog_sel)
      stats.cprog->onLoop(dt);

    if(stats.gfx_refresh) vTaskScreen(nullptr);
    
    stats.gfx_refresh = 0;

    //if(!stats.cprog->onLoop || stats.cprog_sel)
      vTaskDelay(5);

    dt++;
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

  u8g2.begin();
  u8g2.setContrast(82);
  u8g2.setFlipMode(1);

  
  USBComposite.clear();
  USBComposite.setProductId(0x0031);
  HID.registerComponent();
  HID.setReportDescriptor(HID_KEYBOARD); 
  // USB_audio.setParameters(MIC_STEREO,32000);
  // USB_audio.registerComponent();
  USB_midi.registerComponent();
  bool usb = USBComposite.begin();
  if(!usb) Serial.println("usb begin failed");
  else Serial.println("usb begin succ");

  stats.progs[P_NUMPAD].onBegin = mode_numpad_on_begin;
  stats.progs[P_NUMPAD].onEnd = mode_numpad_on_end;
  stats.progs[P_NUMPAD].onPress = mode_numpad_on_press;
  stats.progs[P_NUMPAD].onRelease = mode_numpad_on_release;
  stats.progs[P_NUMPAD].title = "Keyboard";
  stats.progs[P_NUMPAD].txt_f1 = "123";
  stats.progs[P_NUMPAD].txt_f2 = "< ^ >";
  stats.progs[P_NUMPAD].txt_f3 = nullptr;
  stats.progs[P_NUMPAD].inactive_inc = 1;
  stats.progs[P_NUMPAD].inactive_lim = 800;
  stats.progs[P_NUMPAD].no_input_lim = 240000; //20mins

  stats.progs[P_CALC].onBegin = mode_calc_on_begin;
  stats.progs[P_CALC].onEnd = mode_calc_on_end;
  stats.progs[P_CALC].onPress = mode_calc_on_press;
  stats.progs[P_CALC].onRelease = mode_calc_on_release;
  stats.progs[P_CALC].onGfx = mode_calc_on_gfx;
  stats.progs[P_CALC].title = "Calculator";
  stats.progs[P_CALC].txt_f1 = "SCI";
  stats.progs[P_CALC].txt_f2 = "BIN";
  stats.progs[P_CALC].txt_f3 = nullptr;
  stats.progs[P_CALC].inactive_inc = 1;
  stats.progs[P_CALC].inactive_lim = 10000;
  stats.progs[P_CALC].no_input_lim = 240000; //20mins

  stats.progs[P_MIDI].onBegin = mode_midi_on_begin;
  stats.progs[P_MIDI].onEnd = mode_midi_on_end;
  stats.progs[P_MIDI].onPress = mode_midi_on_press;
  stats.progs[P_MIDI].onRelease = mode_midi_on_release;
  stats.progs[P_MIDI].title = "MIDI";
  stats.progs[P_MIDI].txt_f1 = nullptr;
  stats.progs[P_MIDI].txt_f2 = nullptr;
  stats.progs[P_MIDI].txt_f3 = nullptr;
  stats.progs[P_MIDI].inactive_inc = 1;
  stats.progs[P_MIDI].inactive_lim = 400;
  stats.progs[P_MIDI].no_input_lim = 240000; //20mins

  stats.progs[P_COMMS].onBegin = mode_comms_on_begin;
  stats.progs[P_COMMS].onEnd = mode_comms_on_end;
  stats.progs[P_COMMS].onPress = mode_comms_on_press;
  stats.progs[P_COMMS].onRelease = mode_comms_on_release;
  stats.progs[P_COMMS].onLoop = mode_comms_on_loop;
  stats.progs[P_COMMS].onGfx = mode_comms_on_gfx;
  stats.progs[P_COMMS].title = "Comms";
  stats.progs[P_COMMS].txt_f1 = "UART";
  stats.progs[P_COMMS].txt_f2 = "SPI";
  stats.progs[P_COMMS].txt_f3 = "I2C";
  stats.progs[P_COMMS].inactive_inc = 0;
  stats.progs[P_COMMS].inactive_lim = 800;

  uint16_t p = 0;
  uint16_t f = 0;
  EEPROM.read(0,&p);
  EEPROM.read(1,&f);
  if(p >= P_COUNT) p = 0;
  if(f > 2) f = 0;

  Serial.println("seq start");
  changeToProg(p);
  stats.fmode = f;

  Serial.println("sched start");

  // xTaskCreate(vTaskKeyMux,"key_mux",configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY,NULL);
  // //xTaskCreate(vTaskWorker,"worker",configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY,NULL);
  // xTaskCreate(vTaskScreen,"screen",configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY,NULL);
  // vTaskStartScheduler();
  stats.gfx_refresh = 1;
  vTaskKeyMux(nullptr);
}

void loop(){}
