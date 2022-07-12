#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"

int rep_count = 0;

void mode_gpio_on_begin(){
  gpio_set_mode(GPIOA,0,GPIO_OUTPUT_PP);
  gpio_set_mode(GPIOA,1,GPIO_OUTPUT_PP);

  io.turns_right |= 1;
}

void mode_gpio_on_end(){
  
}

void mode_gpio_on_process(){
  if(io.turns_left || io.turns_right){
    rep_count -= io.turns_left;
    rep_count += io.turns_right;
    if(rep_count < 0) rep_count = 0;
    io.turns_left = 0;
    io.turns_right = 0;
    
    lcd_clear();
      drawTitle();  
      char str[32]; 
      snprintf(str,32,"GP_A reps: %d",rep_count); 
      lcd_drawString(0,16,sys_font,str);
    lcd_update();
  }
  if(io.bscan_down){
    if(io.bscan_down & (1<<K_Y)){
      for(int i=0; i<rep_count; i++){
        gpio_write_bit(GPIOA,0,1);
        delay_us(250000);
        gpio_write_bit(GPIOA,0,0);
        delay_us(250000);
      }
    }
    if(io.bscan_down & (1<<K_0)){
      rep_count = 0;
      io.turns_right |= 1;
    }
    if(io.bscan_down & (1<<K_1)){
      gpio_write_bit(GPIOA,0,1);
    }
    if(io.bscan_down & (1<<K_2)){
      gpio_write_bit(GPIOA,1,1);
    }
    io.bscan_down = 0;
  }
  else if(io.bscan_up){
    if(io.bscan_up & (1<<K_1)){
      gpio_write_bit(GPIOA,0,0);
    }
    if(io.bscan_up & (1<<K_2)){
      gpio_write_bit(GPIOA,1,0);
    }
    io.bscan_up = 0;
  }
  delay_us(100000);
}
