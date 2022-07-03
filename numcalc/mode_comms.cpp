#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"

int i2c_step = 0;
#define I2C_RES 0
#define I2C_SCAN 1
#define I2C_WRITE 2
int i2c_addr_cursor = 0;
int i2c_addr_count = 0;
int i2c_addr_table[4];
int i2c_payload_count = 0;
int i2c_payload_ptr = 0;
byte i2c_payload[16];
int i2c_payload_ptr_offset = 0;

bool comms_setup = 0;
int imode = 0;
int digit_ptr = 0;

void mode_comms_on_begin(){
  if(!comms_setup)
    Wire.begin();
  comms_setup = 1;
	lcd_clear();
  drawTitle();
  lcd_update();
  i2c_step = I2C_SCAN;
  io.bscan_down |= 1;
}

void mode_comms_on_end(){
  if(comms_setup) 
    Wire.end();
  comms_setup = 0;
}

void mode_comms_on_process(){
  char str[32]; 
  delay_us(10000);
  if(stats.fmode == 0 ){
    if(!(io.bscan_down || io.turns_left || io.turns_right)) return;

i2c_task: 
    switch(i2c_step){
      case I2C_RES:
      
        if(io.bscan_down & (1<<K_P)) {
          i2c_step = I2C_WRITE;
          io.bscan_down = 0;
          goto i2c_task;
        }
        else if(io.bscan_down & (1<<K_R)){
          i2c_step = I2C_SCAN;
          io.bscan_down = 0;
          goto i2c_task;
        }
        else if(io.turns_left){
          io.turns_left = 0;
          i2c_addr_cursor = ((i2c_addr_cursor-1)&0x7f);
        }
        else if(io.turns_right){
          io.turns_right = 0;
          i2c_addr_cursor = ((i2c_addr_cursor+1)&0x7f);
        }

        clearProgGFX();
        snprintf(str,32,"I2C: %d [%d]{0x%X}",i2c_addr_count,1+i2c_addr_cursor,1+i2c_addr_cursor);
        lcd_drawString(0,16,sys_font,str);
        for(int i=0; i<4; i++){
          for(int x=0; x<32; x++){
            if(i2c_addr_table[i] & (1<<x))
              lcd_drawRectSize(x*4,32+i*8,3,3);
            else
              lcd_drawHline(x*4,32+i*8,3);

            if(x + i*32 == i2c_addr_cursor){
              uint8_t box[5] = {0x3f,0x3f,0x3f,0x3f,0x3f};
              lcd_drawTile(x*4,31+i*8,3,5,0,box,DRAWBITMAP_XOR);
            }
          }
        }
        updateProgGFX();
      break;

      case I2C_SCAN:
        clearProgGFX();
        lcd_drawString(32,24,sys_font,"I2C Scan...");
        updateProgGFX();
        delay_us(500000);
        byte error, address;
        i2c_addr_count = 0;
        i2c_addr_table[0] = 0;
        i2c_addr_table[1] = 0;
        i2c_addr_table[2] = 0;
        i2c_addr_table[3] = 0;
        for(address = 0; address < 127; address++ ) 
        {
          Wire.beginTransmission(address+1);
          error = Wire.endTransmission(true);
      
          if (error == 0)
          {
            i2c_addr_count++;
            i2c_addr_table[(address>>5)] |= (1<<(address&0x1f));
          }
        }
        i2c_step = I2C_RES;
        goto i2c_task;
      break;

      case I2C_WRITE:
        
        if ( io.bscan_down&0x70000 ){
          int i = io.bscan_down>>16;
          io.bscan_down = 0;
          digit_ptr = 0;
          if(imode != i)
            imode = i;
          else
            imode = 0;
        }
        else if ((io.bscan_down & 0x7772) && imode == 1){
          uint32_t i = ((io.bscan_down&0x70)>>3); //123
          i |= ((io.bscan_down&0x000700)>>4); //456
          i |= ((io.bscan_down&0x007000)>>5); //789
          i |= ((io.bscan_down&0x000002)>>1); //0
          LOGL(i);
          for(int j=0; j<10; j++){
            if(i == (1<<j)){
              i=j; 
              break;
            }
          }

          uint8_t a = (i2c_payload[i2c_payload_ptr]); //org
          if(digit_ptr == 0){
            a /= 10;
            a *= 10;
            i2c_payload[i2c_payload_ptr] = a+i;
          }
          else if(digit_ptr == 1){
            uint8_t b = ((a / 100)*10 + i)*10; //xNx
            uint8_t c = a - ((a / 10)*10); //..x
            i2c_payload[i2c_payload_ptr] = b+c;
          }
          else if(digit_ptr == 2){
            uint8_t b = i*100; //N..
            uint8_t c = a - ((a / 100)*100); //.xx
            i2c_payload[i2c_payload_ptr] = b+c;
          }

          digit_ptr = (digit_ptr+1) % 3;

          io.bscan_down = 0;
        }
        else if ((io.bscan_down & 0x8FFFE) && imode == 2){
          uint32_t i = ((io.bscan_down&0x70)>>3); //123
          i |= ((io.bscan_down&0x000700)>>4); //456
          i |= ((io.bscan_down&0x007000)>>5); //789
          i |= ((io.bscan_down&0x000002)>>1); //0
          i |= ((io.bscan_down&0x00000C)<<8); //AB
          i |= ((io.bscan_down&0x000080)<<5); //C
          i |= ((io.bscan_down&0x00800)<<2); //D
          i |= ((io.bscan_down&0x08000)>>1); //E
          i |= ((io.bscan_down&0x80000)>>4); //F
          LOGL(i);
          for(int j=0; j<16; j++){
            if(i == (1<<j)){
              i=j; 
              break;
            }
          }

          i2c_payload[i2c_payload_ptr] &= 0xf0>>(digit_ptr*4);
          i2c_payload[i2c_payload_ptr] |= i<<(digit_ptr*4);
        
          digit_ptr = (!digit_ptr) & 0x1;

          io.bscan_down = 0;
        }
        else if ((io.bscan_down & 0x3772) && imode == 4){
            if(io.bscan_down == (1<<K_0)) 
              i2c_payload[i2c_payload_ptr] = 0;
            else{
              byte i = ((io.bscan_down&0x70)>>4) | ((io.bscan_down&0x700)>>5)| ((io.bscan_down&0x3000)>>6);
              i2c_payload[i2c_payload_ptr] ^= i;
            }
          io.bscan_down = 0;
        }
        else if(io.bscan_down == (1<<K_P)) {
          clearProgGFX();
          updateProgGFX();
          Wire.beginTransmission(i2c_addr_cursor+1);
          Wire.write(i2c_payload,i2c_payload_count);
          Wire.endTransmission(true);
          io.bscan_down = 0;
          delay_us(100000);
        }
        else if(io.bscan_down == (1<<K_X)){
          i2c_payload_count = 0;
          i2c_payload_ptr = 0;
          io.bscan_down = 0;
        }
        else if(io.bscan_down == (1<<K_R)){
          i2c_step = I2C_RES;
          io.bscan_down = 0;
          goto i2c_task;
        }
        else if(io.turns_left){
          io.turns_left = 0;
          i2c_payload[i2c_payload_ptr] = ((i2c_payload[i2c_payload_ptr]-1)&0xff);
        }
        else if(io.turns_right){
          io.turns_right = 0;
          i2c_payload[i2c_payload_ptr] = ((i2c_payload[i2c_payload_ptr]+1)&0xff);
        }
        else if(io.bscan_down == (1<<K_N)) {
          if(i2c_payload_count < 16) {
            i2c_payload_count++;
            i2c_payload_ptr = i2c_payload_count;
            i2c_payload[i2c_payload_count] = i2c_payload[i2c_payload_count-1];
          }
          io.bscan_down = 0;
          delay_us(100000);
        }
        else if(io.bscan_down == (1<<K_D)) {
          if(i2c_payload_count > 0) {
            i2c_payload_count--;
            i2c_payload_ptr = i2c_payload_count;
          }
          io.bscan_down = 0;
          delay_us(100000);
        }

        clearProgGFX();
          // snprintf(str,32,"(P)ost; (R)et; %s","...");
          // lcd_drawString(0,16,sys_font,str);
          snprintf(str,32,"Dest @ [%d]{0x%X}",1+i2c_addr_cursor,1+i2c_addr_cursor);
          lcd_drawString(0,16,sys_font,str);
          lcd_drawHline(0,24,128);

          for(int i=0; i<i2c_payload_count; i++){
            char d = i2c_payload[i];
            snprintf(str,32,"0x%s%X",d < 16 ? "0" : "",d);
            lcd_drawString((i%4)*5*6,24+8*(i>>2),sys_font,str);
          }

          char d = i2c_payload[i2c_payload_ptr];
          char bin_str[10];
          bin_str[8] = 0;
          for(int i=0; i<8; i++)
            bin_str[7-i] = d & (1<<i) ? '1' : '0';

          snprintf(str,32,"%3d  0x%s%X  0b%s",d,d < 16 ? "0" : "",d,bin_str);
          lcd_drawString(0,64-8,sys_font,str);

          // if(imode)
          //   lcd_drawHline(imode*2*6 + 4*6*imode,64-10,4*6);
          
        updateProgGFX();

      break;
    }
    
  }
}
