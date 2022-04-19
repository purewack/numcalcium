
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



void changeToProg(int i){
  if(stats.cprog) stats.cprog->on_end();
  stats.cprog = &stats.progs[i];
  stats.cprog->on_begin(); 
  stats.fmode = 0;
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

void vTaskWorker(void* params){
  while(1){
    vTaskDelay(10);
    if(stats.cprog->on_work) stats.cprog->on_work();
  }
}

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

    if(stats.cprog->on_gfx) {
      u8g2.clearBuffer();
      u8g2.setCursor(0,15);
      u8g2.print(stats.cprog->title);
      u8g2.drawHLine(0,16,128);


      u8g2.setCursor(0,64);
      u8g2.print("NUM    DIR");
      u8g2.drawHLine(0,64-16,128);
      
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

//       vTaskEndScheduler();
// //      delay(1000);
//       lcdFade(0);
//       digitalWrite(SYS_PDOWN, HIGH);
//      
      if(stats.cprog->on_ok) stats.cprog->on_ok();
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
        if(stats.cprog->on_nav)
          stats.cprog->on_nav(-1);
      }
      else if(!hw.enc_a && hw.enc_b && !hw.enc_a_old && !hw.enc_b_old){    
  #ifdef DEBUG
        Serial.println("RIGHT turn");
  #endif
        if(stats.cprog->on_nav)
          stats.cprog->on_nav(1);
      }
    digitalWrite(ROW_F, LOW);

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

  stats.progs[0].on_begin = mode_numpad_on_begin;
  stats.progs[0].on_end = mode_numpad_on_end;
  stats.progs[0].on_press = mode_numpad_on_press;
  stats.progs[0].on_release = mode_numpad_on_release;
  stats.progs[0].on_gfx = mode_numpad_on_gfx;
  stats.progs[0].on_work = mode_numpad_on_work;
  stats.progs[0].title = "Numpad";
  changeToProg(0);

  xTaskCreate(vTaskKeyMux,"key_mux",configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY,NULL);
  xTaskCreate(vTaskWorker,"worker",configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY,NULL);
  xTaskCreate(vTaskScreen,"screen",configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY,NULL);
  vTaskStartScheduler();
}

void loop(){ }
