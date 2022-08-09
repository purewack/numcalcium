#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"

bool scope_hold = 0; //hold display, dont update on 1
int16_t trig_lvl = 2400; //threshold for triggering 
uint16_t trig_pos = 0; //flag if signal passed thresh and n of sample in buf
uint32_t trig_duration = 0; //spl time spent front
uint8_t trig_timebase = 1; //x axis zoom level in sw, adding / taking divisions
uint8_t timebase_mode = 0; //current time base mode, determines srate
int16_t y_gain = 255; //zoom level of the y axis in sw
int8_t y_gain_ctrl = 0; //amplifier gain control, > 0 = more gain, < 0 = less gain
double frequency; //calculated freq
uint32_t freq_av; //average freq
uint8_t freq_avc; //average freq counter
double db_level = 0.0;
double rms_total = 0.0;

void mode_scope_set_amp(int8_t mode){
	y_gain_ctrl = mode;

	gpio_write_bit(GPIOB,15,mode == -1 ? 1 : 0);
	gpio_write_bit(GPIOB,14,mode == -2 ? 1 : 0);
	gpio_write_bit(GPIOB,13,mode == -3 ? 1 : 0);

	gpio_write_bit(GPIOB,12,mode == 1 ? 1 : 0);
	gpio_write_bit(GPIOB,6,mode == 2 ? 1 : 0);
	gpio_write_bit(GPIOB,7,mode == 3 ? 1 : 0);
}

void mode_scope_set_tbase(uint_8 tb){
 	timebase_mode = tb;	
}

void mode_scope_on_begin(){
    adc_block_init();
	gpio_set_mode(GPIOB,15,GPIO_OUTPUT_OD);// a/2
	gpio_set_mode(GPIOB,14,GPIO_OUTPUT_OD);// a/5
	gpio_set_mode(GPIOB,13,GPIO_OUTPUT_OD);// a/10

	gpio_set_mode(GPIOB,12,GPIO_OUTPUT_OD);// a*2
	gpio_set_mode(GPIOB,6,GPIO_OUTPUT_OD);// a*5
	gpio_set_mode(GPIOB,7,GPIO_OUTPUT_OD);// a*10
	
	mode_scope_set_amp(0);
    LOGL("scope: done on_begin");
}

void mode_scope_on_end(){
    adc_block_deinit();
}

void mode_scope_on_process(){
    if(io.bscan_down){
        if(io.bscan_down & (1<<K_X)) scope_hold = !scope_hold;
        
        if(io.bscan_down & (1<<K_F1)) stats.fmode = 1; //trig_timebase
        else if(io.bscan_down & (1<<K_F2)) stats.fmode = 2; //y gain
        else if(io.bscan_down & (1<<K_F3)) stats.fmode = 3; //trig_lvl

	if(io.bscan_down & (1<<K_1)) adc_set_srate_type(0);
	else if(io.bscan_down & (1<<K_0)) adc_set_srate_type(-1);
        io.bscan_down = 0;
    }
    else if(io.bscan_up){
        stats.fmode = 0;
        io.bscan_up = 0;
    }
    if(stats.fmode && (io.turns_left || io.turns_right)){
        int tt = io.turns_right - io.turns_left;
	if(stats.fmode == 3) trig_lvl = (tt+trig_lvl)%4096;
        io.turns_left = 0;
        io.turns_right = 0;
    }

	adc_block_get((uint16_t*)(shared_int32_1024),1024);
    while(IS_ADC_BUSY){}
    auto s16b = (uint16_t*)shared_int32_1024;   
    
    trig_pos = 0;
	rms_total = 0;
    for(int i=1; i<1024; i++){
		auto a = double(s16b[i])/2048.0;
		rms_total += sqrt(a*a);

		trig_duration++;
        if(s16b[i-1] > trig_lvl && s16b[i] <= trig_lvl){
			if(!trig_pos)	
            	trig_pos = i;
			
            freq_av += (adc_srate / trig_duration);  
			trig_duration = 0;

			freq_avc++;
			if(freq_avc == 10){
				frequency = double(freq_av) / double(freq_avc);
				freq_avc = 0;
				freq_av = 0;
			}
        }
    }
	db_level = 20*log10(rms_total / 1024.0);

    if(scope_hold) return;
    lcd_clear();
		
        for(int i=trig_pos; i<128; i++){
            int yy = s16b[i];
            yy >>= 6;
            lcd_drawVline(i-trig_pos,0,yy);
        }
		
		//time base divisions
        for(int i=0; i<8; i++){
			char cc = 0xff;
			lcd_drawTile(i*16,64-8,1,8,0,0,&cc,DRAWBITMAP_XOR);
        }

			
			//info background
			char p = 0xff;
			lcd_drawTile(0,0,128,8,0,0,&p,DRAWBITMAP_XOR);
			char str[32];

        if(stats.fmode){
            const char* pre = (stats.fmode == 1 ? "Timebase/" : (stats.fmode == 2 ? "Y-Scale" : "Thresh"));
            int val = (stats.fmode == 1 ? trig_timebase : (stats.fmode == 3 ? trig_lvl : y_gain));
            snprintf(str,32,"%s:%d",pre,val);
            lcd_drawString(0,0,sys_font,str);
        }
		else {
			snprintf(str,32,"F:%dHz SR:%dkHz DB:%d",int(frequency),adc_srate/1000, int(db_level));				
			lcd_drawString(0,0,sys_font,str);
		}

        if(stats.fmode == 3){
            char p=0x1;
            int ty = trig_lvl>>6;
            for(int i=0; i<128; i+=8){
                lcd_drawTile(i,ty,4,8,0,0,&p,DRAWBITMAP_XOR);
            }
        }
    lcd_update();

}
