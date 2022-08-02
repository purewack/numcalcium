#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"

bool scope_hold = 0; //hold display, dont update on 1
int16_t trig_lvl = 2400; //threshold for triggering 
uint16_t trig_pos = 0; //flag if signal passed thresh and n of sample in buf
uint32_t trig_duration = 0; //spl time spent front
uint8_t trig_timebase = 1; 
int16_t y_gain = 255;
double frequency; //calculated freq

void mode_scope_on_begin(){
    adc_block_init();
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
        io.bscan_down = 0;
    }
    else if(io.bscan_up){
        stats.fmode = 0;
        io.bscan_up = 0;
    }
    if(stats.fmode && (io.turns_left || io.turns_right)){
        int tt = io.turns_right - io.turns_left;
	if(stats.fmode == 3) trig_lvl += tt;
    }

	adc_block_get((uint16_t*)(shared_int32_1024),1024);
    while(IS_ADC_BUSY){}
    auto s16b = (uint16_t*)shared_int32_1024;   
    
    trig_pos = 0;
    for(int i=1; i<1024; i++){
		trid_duration++;
        if(s16b[i-1] > trig_lvl && s16b[i] <= trig_lvl){
			if(!trig_pos)	
            	trig_pos = i;
			
            freq_av += (adc_srate / trig_duration);  
			trig_duration = 0;

			freq_avc++;
			if(freq_avc == 10){
				freq = double(freq_av) / double(freq_avc);
				freq_avc = 0;
				freq_av = 0;
			}
        }
    }

    if(scope_hold) return;
    lcd_clear();
        for(int i=trig_pos; i<128; i++){
            int yy = s16b[i];
            yy >>= 6;
            lcd_drawVline(i-trig_pos,0,yy);
        }
			
			//info background
			char p = 0xff;
			lcd_drawTile(0,0,128,8,0,0,&p,DRAWBITMAP_XOR);
			char str[32];

        if(stats.fmode){
            const char* pre = (stats.fmode == 1 ? "Timebase/" : (stats.fmode == 2 ? "Y-Scale" : "Thresh"));
            int val = (stats.fmode == 1 ? trig_timebase : (stats.fmode == 2 ? trig_lvl : y_gain));
            snprintf(str,32,"%s:%d",pre,val);
            lcd_drawString(0,0,sys_font,str);
        }
		else {
			snprintf(str,32,"F:%dHz SR:%dHz",int(freq),adc_srate);				
			lcd_drawString(0,0,sys_font,str);
		}

        if(stats.fmode == 1){
            char p=0x1;
            int ty = trig_lvl>>6;
            for(int i=0; i<128; i+=8){
                lcd_drawTile(i,ty,4,8,0,0,&p,DRAWBITMAP_XOR);
            }
        }
    lcd_update();

}
