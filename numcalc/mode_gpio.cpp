#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"


int f = 0;
int gp_out;
int gp_in;
int gp_a_mv = 1;
int gp_b_mv = 2;

void mode_gpio_stats(){

  clearProgGFX();
    drawTitle();  
    char str[32]; 

    // snprintf(str,32,"  CS SCL CDA RX MO MI"); 
    // lcd_drawString(0,8*2,sys_font,str);

    // snprintf(str,32,"I/O %d %d %d %d %d %d",1,1,1,0,0,0); 
    // lcd_drawString(0,8*3,sys_font,str);

    adc_block_get((uint16_t*)(shared_int32_1024),128*4);
    while(IS_ADC_BUSY){}
    auto s16b = (uint16_t*)shared_int32_1024;

    for(int i=0; i<128; i++){
      int yy = s16b[i*4]>>5;
      lcd_drawVline(i,10,yy);
    }
    // snprintf(str,32,"GP_A: %d [%dmV]%d",f,(f*3300*gp_a_mv)/4096,gp_a_mv); 
    // lcd_drawString(0,8*4,sys_font,str);

    // // f = adc_read(ADC1,1);
    // // snprintf(str,32,"GP_B: %d [%dmV]%d",f,(f*3300*gp_b_mv)/4096,gp_b_mv); 
    // // lcd_drawString(0,8*5,sys_font,str);

    // snprintf(str,32,"PWM AR/PSC:%d/%d",TIMER1->regs.adv->ARR+1,TIMER1->regs.adv->PSC+1); 
    // lcd_drawString(0,8*6,sys_font,str);
    
    // snprintf(str,32,"DT:%d P:%dHz",TIMER1->regs.adv->CCR3,(48000000/(TIMER1->regs.adv->PSC+1))/(TIMER1->regs.adv->ARR+1)); 
    // lcd_drawString(0,8*7,sys_font,str);

    //char p = 0xff;
    //lcd_drawTile(0, 16 + 8*((stats.p_i+2)&0xf) ,128,8,0,0,&p,DRAWBITMAP_XOR);
  updateProgGFX();
}

void mode_gpio_on_begin(){
  adc_block_init();
  io.turns_right |= 1;
}

void mode_gpio_on_end(){
  
}

void mode_gpio_on_process(){
  if(io.bscan_down & (1<<K_X)){
    char p = 0xff;
    lcd_drawTile(0,0,128,8,0,0,&p,DRAWBITMAP_XOR);
    updateProgGFX();
    io.bscan_down = 0;
  }
  if(io.bstate == (1<<K_X)){
    delay_us(100000);
    return;
  }
  if((io.turns_left || io.turns_right)){
    stats.p_i += io.turns_right;
    stats.p_i -= io.turns_left;
    io.turns_left = 0;
    io.turns_right = 0;
    stats.p_i &= 0xf;
    stats.p_i %= 4;
    LOGL(stats.p_i);
  }
  mode_gpio_stats();
  //   rep_count -= io.turns_left;
  //   rep_count += io.turns_right;
  //   if(rep_count < 0) rep_count = 0;
  //   io.turns_left = 0;
  //   io.turns_right = 0;

  //   if(io.bscan_down & (1<<K_Y)){
  //     for(int i=0; i<rep_count; i++){
  //       gpio_write_bit(GPIOA,0,1);
  //       delay_us(250000);
  //       gpio_write_bit(GPIOA,0,0);
  //       delay_us(250000);
  //     }
  //   }
  //   if(io.bscan_down & (1<<K_0)){
  //     rep_count = 0;
  //     io.turns_right |= 1;
  //   }
  //   if(io.bscan_down & (1<<K_1)){
  //     gpio_write_bit(GPIOA,0,1);
  //   }
  //   if(io.bscan_down & (1<<K_2)){
  //     gpio_write_bit(GPIOA,1,1);
  //   }
  //   io.bscan_down = 0;
  //   if(io.bscan_up){
  //     if(io.bscan_up & (1<<K_1)){
  //       gpio_write_bit(GPIOA,0,0);
  //     }
  //     if(io.bscan_up & (1<<K_2)){
  //       gpio_write_bit(GPIOA,1,0);
  //     }
  //     io.bscan_up = 0;
  //   }
      
    
  // }
  
  delay_us(100000);
}
