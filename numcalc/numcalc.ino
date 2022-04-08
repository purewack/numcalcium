
//CMODE:
//numpad -> F1 = normal, F2 = Arrows, F3 = phone ascii
//calculator -> F1 = basic, F2 = sci, F3 = comp
//midi -> F1 = launchpad, F2 = seq usb, F3 = seq uart midi
//comms monitor -> F1 = UART, F2 = SPI, F3 = I2C
//GPIO -> F1 = debounced sw, F2 = clock bits program like cpld, F3 = PWM channels
//PWM audio -> F1 = osc, F2 scope, F3 nodes
//ws28xx ?

//current stats:
//rom 40% 26.2kb / 64kb 
//ram 63% 13kb / 20.4kb r= 7.5kb

#include <MapleFreeRTOS900.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>

#define DEBUG
#include "include/defs.h"
#include "include/modes.h"
#include "include/comms_usb.h"

U8G2_ST7565_ERC12864_ALT_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ LCD_CS, /* dc=*/ LCD_DC, /* reset=*/ LCD_RST);

Modes cmode = Modes::m_numpad;

typedef struct t_menu_item{
  char* text;
  uint8_t has_sub_menu;
  t_menu* next;
  void (*on_action)(void);
};

typedef struct t_menu {
  t_menu_item* items;
  uint8_t items_c;
  uint8_t items_i;
  uint8_t visible;
} cmenu;

typedef struct t_io {
  //uint8_t target[4];
  uint8_t state = 0;
  uint8_t old_state = 0;
  uint8_t dt;
};

t_io io[21];
uint8_t enc_a, enc_b,enc_a_old,enc_b_old;
int8_t enc_turns, enc_turns_old;
uint8_t ok, oko;

uint8_t k = 0;
const uint8_t rows[] = {ROW_A, ROW_B, ROW_C, ROW_D, ROW_E, ROW_F};
const uint8_t cols[] = {SEG_A, SEG_B, SEG_C, SEG_D};

void sys_on_ok(){

}
void sys_on_nav(int8_t r){
  //r > 0 = right
  //r < 0 = left
}
void sys_on_press(uint8_t i){

}
void sys_on_release(uint8_t i){
  
}

void vTaskWorker(void* params){
  while(1){
    vTaskDelay(100);
  }
}

void vTaskScreen(void* params){


  u8g2.begin();
  u8g2.setContrast(80);

  uint8_t i = 0;
  while(1){
    vTaskDelay(100);
    u8g2.clearBuffer();					// clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
    u8g2.drawStr(i%30,i%30,"Hello World!");	// write something to the internal memory
    u8g2.sendBuffer();
    i++;	
  }
}

void vTaskKeyMux(void* params){
  while(1){
    ok = digitalRead(B_OK);
    if(ok != oko){
      oko = ok;
      if(!ok) continue;

      //if(cmenu != 1) cmenu = 1;
      Serial.println("ok");
      sys_on_ok();
    }
    
    for(int r=0; r<5; r++){
      digitalWrite(rows[r],HIGH);
      for(int c=0; c<4; c++){
        int I = c + (r*4);
        if(io[I].target[cmode]){
          io[I].state = digitalRead(cols[c]);

          if(io[I].state && !io[I].old_state){
            io[I].dt = 0;
            sys_on_press(I);
          }
          else if(!io[I].state && io[I].old_state){
            sys_on_release(I);
          }
          
          if(io[I].state != io[I].old_state)
            io[I].old_state = io[I].state;

          if(io[I].state) io[I].dt += 1;
        }
      }
      digitalWrite(rows[r],LOW);
    }

    digitalWrite(ROW_F, HIGH);
    
      enc_a_old = enc_a;
      enc_b_old = enc_b;
      enc_a = digitalRead(SEG_A);
      enc_b = digitalRead(SEG_B);
      //if(cmenu == 0) goto enc_clean;

      if(enc_a && !enc_b && !enc_a_old && !enc_b_old){
  #ifdef DEBUG
        Serial.println("LEFT turn");
  #endif
        sys_on_nav(-1);
      }
      else if(!enc_a && enc_b && !enc_a_old && !enc_b_old){    
  #ifdef DEBUG
        Serial.println("RIGHT turn");
  #endif
        sys_on_nav(1);
      }
    digitalWrite(ROW_F, LOW);

    vTaskDelay(10);
    //delay(10);
  }
}

void setup(){
  disableDebugPorts();
  USBComposite.setProductId(0x0031);
  HID.begin(HID_KEYBOARD);
  Serial.begin(9600);
  Wire.begin();

  // io[13].target[0] = KEY_UP_ARROW;
  // io[8].target[0] = KEY_LEFT_ARROW;
  // io[9].target[0] = KEY_DOWN_ARROW;
  // io[10].target[0] = KEY_RIGHT_ARROW;
  // io[5].target[0] = KEY_UP_ARROW;
  // io[0].target[0] = KEY_LEFT_ARROW;
  // io[1].target[0] = KEY_DOWN_ARROW;
  // io[2].target[0] = KEY_RIGHT_ARROW;
  cmode = 0;

  for(int i=0; i<4; i++)
    pinMode(cols[i], INPUT_PULLDOWN);

  for(int i=0; i<6; i++)
    pinMode(rows[i], OUTPUT);

  pinMode(SYS_PDOWN, OUTPUT);
  digitalWrite(SYS_PDOWN, LOW);

  pinMode(B_OK,INPUT_PULLDOWN);
  
  pinMode(LCD_LIGHT, OUTPUT);
  // for(int j=0; j<256; j++){
  //   delayMicroseconds(j);
  //   digitalWrite(LCD_LIGHT, HIGH);
  //   delayMicroseconds(256-j);
  //   digitalWrite(LCD_LIGHT, LOW);
  // }
  digitalWrite(LCD_LIGHT, HIGH);
  
  delay(2000);
  Serial.println("Starting scheduler");
  xTaskCreate(vTaskKeyMux,"key_mux",configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY,NULL);
  xTaskCreate(vTaskWorker,"worker",configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY,NULL);
  xTaskCreate(vTaskScreen,"screen",configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY,NULL);
  vTaskStartScheduler();
  //vTaskKeyMux(nullptr);
}

void loop(){ }
