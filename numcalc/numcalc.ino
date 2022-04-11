
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


//conflicting miso pin being pinmoded to input after ::sendBuffer()
//U8G2_ST7565_ERC12864_ALT_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ LCD_CS, /* dc=*/ LCD_DC, /* reset=*/ LCD_RST);
//U8G2_ST7565_64128N_F_4W_HW u8g2(U8G2_R0, /* cs=*/ LCD_CS, /* dc=*/ LCD_DC, /* reset=*/ LCD_RST);

Modes cmode = Modes::m_numpad;
typedef struct t_menu;
typedef struct t_menu_item;

typedef struct t_menu_item{
  char* text;
  uint8_t has_sub_menu;
  t_menu* next;
};

typedef struct t_menu {
  t_menu_item* items;
  uint8_t items_c;
  uint8_t items_i;
  uint8_t visible;  
  
  void (*on_ok)(void);
  void (*on_nav)(int);
  void (*on_press)(int);
  void (*on_release)(int);
} cmenu[6];
int ci = 0;


void vTaskWorker(void* params){
  while(1){
    vTaskDelay(100);
  }
}

void vTaskScreen(void* params){

  // u8g2.begin();
  // u8g2.setContrast(82);
  // u8g2.setFlipMode(1);
  // u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font

  // SPI.begin();
  //   pinMode(LCD_LIGHT, OUTPUT);
  //   digitalWrite(LCD_LIGHT, HIGH);
  
  // vTaskDelay(2000);


  uint8_t i = 0;
  while(1){
    vTaskDelay(100);
    // u8g2.clearBuffer();					// clear the internal memory
    // u8g2.drawStr(i%30,i%30,"Hello World!");	// write something to the internal memory
    // u8g2.sendBuffer();
    //SPI.transfer(0xff);
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
      cmenu[ci].on_ok();
    }
    
    for(int r=0; r<5; r++){
      digitalWrite(rows[r],HIGH);
      for(int c=0; c<4; c++){
        int I = c + (r*4);
        //if(io[I].target[cmode]){
          io[I].state = digitalRead(cols[c]);

          if(io[I].state && !io[I].old_state){
            io[I].dt = 0;
            cmenu[ci].on_press(I);
          }
          else if(!io[I].state && io[I].old_state){
            cmenu[ci].on_release(I);
          }
          
          if(io[I].state != io[I].old_state)
            io[I].old_state = io[I].state;

          if(io[I].state) io[I].dt += 1;
//        }
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
        cmenu[ci].on_nav(-1);
      }
      else if(!enc_a && enc_b && !enc_a_old && !enc_b_old){    
  #ifdef DEBUG
        Serial.println("RIGHT turn");
  #endif
        cmenu[ci].on_nav(1);
      }
    digitalWrite(ROW_F, LOW);

    vTaskDelay(10);
    //delay(10);
  }
}

void setup(){
  disableDebugPorts();
  // USBComposite.setProductId(0x0031);
  // HID.begin(HID_KEYBOARD);
  Serial.begin(9600);

  for(int i=0; i<4; i++)
    pinMode(cols[i], INPUT_PULLDOWN);

  for(int i=0; i<6; i++)
    pinMode(rows[i], OUTPUT);

  pinMode(SYS_PDOWN, OUTPUT);
  digitalWrite(SYS_PDOWN, LOW);

  pinMode(B_OK,INPUT_PULLDOWN);
  
  delay(1000);
  Serial.println("Starting scheduler");
  xTaskCreate(vTaskKeyMux,"key_mux",configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY,NULL);
  xTaskCreate(vTaskWorker,"worker",configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY,NULL);
  xTaskCreate(vTaskScreen,"screen",configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY,NULL);
  vTaskStartScheduler();
}

void loop(){ }
