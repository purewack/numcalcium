//#include <Arduino.h>
//#include <U8g2lib.h>
//#include <SPI.h>
//#include <Wire.h>
//#include <Serial.h>
////
//U8G2_ST7565_ERC12864_ALT_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ PA4, /* dc=*/ PA3, /* reset=*/ PA2);
//
//SPIClass SPI_2(2);


// static uint8_t encoderLookup[256*3]={	0x92,0x49,0x24,0x92,0x49,0x26,0x92,0x49,0x34,0x92,0x49,0x36,0x92,0x49,0xA4,0x92,0x49,0xA6,0x92,0x49,0xB4,0x92,0x49,0xB6,0x92,0x4D,0x24,0x92,0x4D,0x26,0x92,0x4D,0x34,0x92,0x4D,0x36,0x92,0x4D,0xA4,0x92,0x4D,0xA6,0x92,0x4D,0xB4,0x92,0x4D,0xB6,0x92,0x69,0x24,0x92,0x69,0x26,0x92,0x69,0x34,0x92,0x69,0x36,0x92,0x69,0xA4,0x92,0x69,0xA6,0x92,0x69,0xB4,0x92,0x69,0xB6,0x92,0x6D,0x24,0x92,0x6D,0x26,0x92,0x6D,0x34,0x92,0x6D,0x36,0x92,0x6D,0xA4,0x92,0x6D,0xA6,0x92,0x6D,0xB4,0x92,0x6D,0xB6,0x93,0x49,0x24,0x93,0x49,0x26,0x93,0x49,0x34,0x93,0x49,0x36,0x93,0x49,0xA4,0x93,0x49,0xA6,0x93,0x49,0xB4,0x93,0x49,0xB6,0x93,0x4D,0x24,0x93,0x4D,0x26,0x93,0x4D,0x34,0x93,0x4D,0x36,0x93,0x4D,0xA4,0x93,0x4D,0xA6,0x93,0x4D,0xB4,0x93,0x4D,0xB6,0x93,0x69,0x24,0x93,0x69,0x26,0x93,0x69,0x34,0x93,0x69,0x36,0x93,0x69,0xA4,0x93,0x69,0xA6,0x93,0x69,0xB4,0x93,0x69,0xB6,0x93,0x6D,0x24,0x93,0x6D,0x26,0x93,0x6D,0x34,0x93,0x6D,0x36,0x93,0x6D,0xA4,0x93,0x6D,0xA6,0x93,0x6D,0xB4,0x93,0x6D,0xB6,0x9A,0x49,0x24,0x9A,0x49,0x26,0x9A,0x49,0x34,0x9A,0x49,0x36,0x9A,0x49,0xA4,0x9A,0x49,0xA6,0x9A,0x49,0xB4,0x9A,0x49,0xB6,0x9A,0x4D,0x24,0x9A,0x4D,0x26,0x9A,0x4D,0x34,0x9A,0x4D,0x36,0x9A,0x4D,0xA4,0x9A,0x4D,0xA6,0x9A,0x4D,0xB4,0x9A,0x4D,0xB6,0x9A,0x69,0x24,0x9A,0x69,0x26,0x9A,0x69,0x34,0x9A,0x69,0x36,0x9A,0x69,0xA4,0x9A,0x69,\
// 											0xA6,0x9A,0x69,0xB4,0x9A,0x69,0xB6,0x9A,0x6D,0x24,0x9A,0x6D,0x26,0x9A,0x6D,0x34,0x9A,0x6D,0x36,0x9A,0x6D,0xA4,0x9A,0x6D,0xA6,0x9A,0x6D,0xB4,0x9A,0x6D,0xB6,0x9B,0x49,0x24,0x9B,0x49,0x26,0x9B,0x49,0x34,0x9B,0x49,0x36,0x9B,0x49,0xA4,0x9B,0x49,0xA6,0x9B,0x49,0xB4,0x9B,0x49,0xB6,0x9B,0x4D,0x24,0x9B,0x4D,0x26,0x9B,0x4D,0x34,0x9B,0x4D,0x36,0x9B,0x4D,0xA4,0x9B,0x4D,0xA6,0x9B,0x4D,0xB4,0x9B,0x4D,0xB6,0x9B,0x69,0x24,0x9B,0x69,0x26,0x9B,0x69,0x34,0x9B,0x69,0x36,0x9B,0x69,0xA4,0x9B,0x69,0xA6,0x9B,0x69,0xB4,0x9B,0x69,0xB6,0x9B,0x6D,0x24,0x9B,0x6D,0x26,0x9B,0x6D,0x34,0x9B,0x6D,0x36,0x9B,0x6D,0xA4,0x9B,0x6D,0xA6,0x9B,0x6D,0xB4,0x9B,0x6D,0xB6,0xD2,0x49,0x24,0xD2,0x49,0x26,0xD2,0x49,0x34,0xD2,0x49,0x36,0xD2,0x49,0xA4,0xD2,0x49,0xA6,0xD2,0x49,0xB4,0xD2,0x49,0xB6,0xD2,0x4D,0x24,0xD2,0x4D,0x26,0xD2,0x4D,0x34,0xD2,0x4D,0x36,0xD2,0x4D,0xA4,0xD2,0x4D,0xA6,0xD2,0x4D,0xB4,0xD2,0x4D,0xB6,0xD2,0x69,0x24,0xD2,0x69,0x26,0xD2,0x69,0x34,0xD2,0x69,0x36,0xD2,0x69,0xA4,0xD2,0x69,0xA6,0xD2,0x69,0xB4,0xD2,0x69,0xB6,0xD2,0x6D,0x24,0xD2,0x6D,0x26,0xD2,0x6D,0x34,0xD2,0x6D,0x36,0xD2,0x6D,0xA4,0xD2,0x6D,0xA6,0xD2,0x6D,0xB4,0xD2,0x6D,0xB6,0xD3,0x49,0x24,0xD3,0x49,0x26,0xD3,0x49,0x34,0xD3,0x49,0x36,0xD3,0x49,0xA4,0xD3,0x49,0xA6,0xD3,0x49,0xB4,0xD3,0x49,0xB6,0xD3,0x4D,0x24,0xD3,0x4D,0x26,0xD3,0x4D,0x34,0xD3,\
// 											0x4D,0x36,0xD3,0x4D,0xA4,0xD3,0x4D,0xA6,0xD3,0x4D,0xB4,0xD3,0x4D,0xB6,0xD3,0x69,0x24,0xD3,0x69,0x26,0xD3,0x69,0x34,0xD3,0x69,0x36,0xD3,0x69,0xA4,0xD3,0x69,0xA6,0xD3,0x69,0xB4,0xD3,0x69,0xB6,0xD3,0x6D,0x24,0xD3,0x6D,0x26,0xD3,0x6D,0x34,0xD3,0x6D,0x36,0xD3,0x6D,0xA4,0xD3,0x6D,0xA6,0xD3,0x6D,0xB4,0xD3,0x6D,0xB6,0xDA,0x49,0x24,0xDA,0x49,0x26,0xDA,0x49,0x34,0xDA,0x49,0x36,0xDA,0x49,0xA4,0xDA,0x49,0xA6,0xDA,0x49,0xB4,0xDA,0x49,0xB6,0xDA,0x4D,0x24,0xDA,0x4D,0x26,0xDA,0x4D,0x34,0xDA,0x4D,0x36,0xDA,0x4D,0xA4,0xDA,0x4D,0xA6,0xDA,0x4D,0xB4,0xDA,0x4D,0xB6,0xDA,0x69,0x24,0xDA,0x69,0x26,0xDA,0x69,0x34,0xDA,0x69,0x36,0xDA,0x69,0xA4,0xDA,0x69,0xA6,0xDA,0x69,0xB4,0xDA,0x69,0xB6,0xDA,0x6D,0x24,0xDA,0x6D,0x26,0xDA,0x6D,0x34,0xDA,0x6D,0x36,0xDA,0x6D,0xA4,0xDA,0x6D,0xA6,0xDA,0x6D,0xB4,0xDA,0x6D,0xB6,0xDB,0x49,0x24,0xDB,0x49,0x26,0xDB,0x49,0x34,0xDB,0x49,0x36,0xDB,0x49,0xA4,0xDB,0x49,0xA6,0xDB,0x49,0xB4,0xDB,0x49,0xB6,0xDB,0x4D,0x24,0xDB,0x4D,0x26,0xDB,0x4D,0x34,0xDB,0x4D,0x36,0xDB,0x4D,0xA4,0xDB,0x4D,0xA6,0xDB,0x4D,0xB4,0xDB,0x4D,0xB6,0xDB,0x69,0x24,0xDB,0x69,0x26,0xDB,0x69,0x34,0xDB,0x69,0x36,0xDB,0x69,0xA4,0xDB,0x69,0xA6,0xDB,0x69,0xB4,0xDB,0x69,0xB6,0xDB,0x6D,0x24,0xDB,0x6D,0x26,0xDB,0x6D,0x34,0xDB,0x6D,0x36,0xDB,0x6D,0xA4,0xDB,0x6D,0xA6,0xDB,0x6D,0xB4,0xDB,0x6D,0xB6};

// uint16_t
//   numLEDs,       // Number of RGB LEDs in strip
//   numBytes;      // Size of 'pixels' buffer

// uint8_t
//   *pixels,        // Holds the current LED color values, which the external API calls interact with 9 bytes per pixel + start + end empty bytes
//   *doubleBuffer, // Holds the start of the double buffer (1 buffer for async DMA transfer and one for the API interaction.
//   rOffset,       // Index of red byte within each 3- or 4-byte pixel
//   gOffset,       // Index of green byte
//   bOffset,       // Index of blue byte
//   wOffset;       // Index of white byte (same as rOffset if no white)
// uint32_t
//   endTime;       // Latch timing reference
  
// void ws_begin(uint8_t n){
//   if(doubleBuffer) 
//   {
// 	  free(doubleBuffer); 
//   }

//   numBytes = (n<<3) + n + 2; // 9 encoded bytes per pixel. 1 byte empty peamble to fix issue with WS_SPI_DEV MOSI and on byte at the end to clear down MOSI 
// 							// Note. (n<<3) +n is a fast way of doing n*9
//   if((doubleBuffer = (uint8_t *)malloc(numBytes*2)))
//   {
//     numLEDs = n;	 
// 	pixels = doubleBuffer;
// 	// Only need to init the part of the double buffer which will be interacted with by the API e.g. setPixelColor
// 	*pixels=0;//clear the preamble byte
// 	*(pixels+numBytes-1)=0;// clear the post send cleardown byte.
// 	ws_clear();// Set the encoded data to all encoded zeros 
//   } 
//   else 
//   {
//     numLEDs = numBytes = 0;
//   }

//   SPI_2.setClockDivider(SPI_CLOCK_DIV16);// need bit rate of 400nS but closest we can do @ 72Mhz is 444ns (which is within spec)
//   SPI_2.begin();
// }

// void ws_setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
//  {
//    uint8_t *bptr = pixels + (n<<3) + n +1;
//    uint8_t *tPtr = (uint8_t *)encoderLookup + g*2 + g;// need to index 3 x g into the lookup
   
//    *bptr++ = *tPtr++;
//    *bptr++ = *tPtr++;
//    *bptr++ = *tPtr++;

//    tPtr = (uint8_t *)encoderLookup + r*2 + r;
//    *bptr++ = *tPtr++;
//    *bptr++ = *tPtr++;
//    *bptr++ = *tPtr++;   
   
//    tPtr = (uint8_t *)encoderLookup + b*2 + b;
//    *bptr++ = *tPtr++;
//    *bptr++ = *tPtr++;
//    *bptr++ = *tPtr++;
//  }

// void ws_show(void) 
// {
//   SPI_2.dmaSendAsync(pixels,numBytes);// Start the DMA transfer of the current pixel buffer to the LEDs and return immediately.

//   // Need to copy the last / current buffer to the other half of the double buffer as most API code does not rebuild the entire contents
//   // from scratch. Often just a few pixels are changed e.g in a chaser effect
  
//   if (pixels==doubleBuffer)
//   {
// 	// pixels was using the first buffer
// 	pixels	= doubleBuffer+numBytes;  // set pixels to second buffer
// 	memcpy(pixels,doubleBuffer,numBytes);// copy first buffer to second buffer
//   }
//   else
//   {
// 	// pixels was using the second buffer	  
// 	pixels	= doubleBuffer;  // set pixels to first buffer
// 	memcpy(pixels,doubleBuffer+numBytes,numBytes);	 // copy second buffer to first buffer 
//   }	
// }

// void ws_clear() 
// {
// 	uint8_t * bptr= pixels+1;// Note first byte in the buffer is a preable and is always zero. hence the +1
// 	uint8_t *tPtr;

// 	for(int i=0;i< (numLEDs *3);i++)
// 	{
// 	   tPtr = (uint8_t *)encoderLookup;
//    	   *bptr++ = *tPtr++;
// 	   *bptr++ = *tPtr++;
// 	   *bptr++ = *tPtr++;	
// 	}
// }
//
//
//void setup(void) {
//  disableDebugPorts();
//  u8g2.begin();
//  u8g2.setContrast(80);
//  ws_begin(3);
//  
//}
//
//int i = 0 ;
//void loop(void) {
//  if(i&0x1){
//      int b = i<<2;
//      ws_setPixelColor(0,b,0,0);
//      ws_setPixelColor(1,0,b,0);
//      ws_setPixelColor(2,0,0,b);
//      ws_show();
//  }
//  
//  u8g2.clearBuffer();					// clear the internal memory
//  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
//  u8g2.drawStr(i%30,i%30,"Hello World!");	// write something to the internal memory
//  u8g2.sendBuffer();					// transfer internal memory to the display
//  i++;
//  delay(100);  
//}

//calculator -> F1 = basic, F2 = sci, F3 = comp
//numpad -> F1 = normal, F2 = Arrows, F3 = phone ascii
//midi -> F1 = launchpad, F2 = seq usb, F3 = seq uart midi
//comms monitor -> F1 = UART, F2 = SPI, F3 = I2C
//GPIO -> F1 = debounced sw, F2 = clock bits program like cpld, F3 = PWM channels
//PWM audio -> F1 = osc, F2 scope, F3 nodes
//script ?

#define SYS_PDOWN PB5
#define LCD_LIGHT PA6

#define SEG_A PB11
#define SEG_B PB10
#define SEG_C PB1
#define SEG_D PB0

#define ROW_A PA14
#define ROW_B PA15
#define ROW_C PB3
#define ROW_D PB4
#define ROW_E PB8
#define ROW_F PB9

#define B_OK PA8

#include <USBComposite.h>

USBHID HID;
HIDKeyboard Keyboard(HID);


typedef struct t_menu_item{
  char* text;
  void (*on_action)(void);
};

typedef struct t_menu {
  t_menu_item* items;
  uint8_t items_c;
  uint8_t items_i;
  uint8_t visible;
} cmenu;

typedef struct t_io {
  uint8_t target[4] = 0;
  uint8_t state = 0;
  uint8_t old_state = 0;
};

t_io io[20];
uint8_t enc_a, enc_b,enc_a_old,enc_b_old;
int8_t enc_turns, enc_turns_old;
uint8_t ok, oko;

uint8_t k = 0;
const uint8_t rows[5] = {ROW_A, ROW_B, ROW_C, ROW_D, ROW_E};
const uint8_t cols[4] = {SEG_A, SEG_B, SEG_C, SEG_D};

void setup(){
  disableDebugPorts();
  USBComposite.setProductId(0x0031);
  HID.begin(HID_KEYBOARD);
  Serial.begin(9600);

  io[13].target[0] = KEY_UP_ARROW;
  io[8].target[0] = KEY_LEFT_ARROW;
  io[9].target[0] = KEY_DOWN_ARROW;
  io[10].target[0] = KEY_RIGHT_ARROW;
  io[5].target[0] = KEY_UP_ARROW;
  io[0].target[0] = KEY_LEFT_ARROW;
  io[1].target[0] = KEY_DOWN_ARROW;
  io[2].target[0] = KEY_RIGHT_ARROW;

  for(int i=0; i<4; i++)
    pinMode(cols[i], INPUT_PULLDOWN);

  for(int i=0; i<6; i++)
    pinMode(rows[i], OUTPUT);

  pinMode(SYS_PDOWN, OUTPUT);
  digitalWrite(SYS_PDOWN, LOW);
  
  pinMode(LCD_LIGHT, OUTPUT);
  digitalWrite(LCD_LIGHT, HIGH);
  
  pinMode(B_OK,INPUT_PULLDOWN);
}

void loop(){
  ok = digitalRead(B_OK);
  if(ok != oko){
    oko = ok;
    if(!ok) return;

    if(cmenu != 1) cmenu = 1;

  }
  
  for(int r=0; r<5; r++){
    digitalWrite(rows[r],HIGH);
    for(int c=0; c<4; c++){
      int I = c + (r*4);
      if(io[I].target[cmode]){
        io[I].state = digitalRead(cols[c]);

        if(io[I].state && !io[I].old_state){
          Keyboard.press(io[I].target[cmode]);
        }
        else if(!io[I].state && io[I].old_state){
          Keyboard.release(io[I].target[cmode]);
        }
        
        if(io[I].state != io[I].old_state)
          io[I].old_state = io[I].state;
      }
    }
    digitalWrite(rows[r],LOW);
  }

  digitalWrite(ROW_F, HIGH);
  
    enc_a_old = enc_a;
    enc_b_old = enc_b;
    enc_a = digitalRead(SEG_A);
    enc_b = digitalRead(SEG_B);
    if(cmenu == 0) goto enc_clean;

    if(enc_a && !enc_b && !enc_a_old && !enc_b_old){
#ifdef DEBUG
      Serial.println("Right turn");
#endif
      
    }
    else if(!enc_a && enc_b && !enc_a_old && !enc_b_old){    
#ifdef DEBUG
      Serial.println("Left turn");
#endif
      
    }
  
enc_clean:
  digitalWrite(ROW_F, LOW);

  delay(5);
}
