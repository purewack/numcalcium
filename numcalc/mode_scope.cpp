#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"

bool scope_hold; //hold display, dont update on 1
int16_t trig_lvl; //threshold for triggering 
uint16_t trig_pos; //flag if signal passed thresh and n of sample in buf
uint32_t trig_duration; //spl time spent front
uint8_t trig_filter;

uint8_t x_zoom; //x axis zoom level in sw, adding / taking divisions
uint8_t y_zoom; //zoom level of the y axis in sw
int8_t x_timebase; //current time base mode, determines srate 0 == 32khz 1ms/div
int8_t y_gain_ctrl; //amplifier gain control, > 0 = more gain, < 0 = less gain
uint32_t us_per_div;//per div microseconds
uint32_t uv_per_div;//per div microvolts
int8_t y_scroll;
uint8_t x_scroll;

double frequency; //calculated freq
uint32_t freq_av; //average freq
uint8_t freq_avc; //average freq counter

double db_ref;
double db_level; //db signal level of frame
double rms_total; //rms total for average 

uint8_t dt_cursor; //time base cursor
uint8_t dy_cursor; //time base cursor
uint8_t details_view; //show hide ui
uint8_t info_view;

#define SCOPE_FMODE_TB 1
#define SCOPE_FMODE_XZOOM 2
#define SCOPE_FMODE_XSCROLL 3
#define SCOPE_FMODE_YAMP 4
#define SCOPE_FMODE_YZOOM 5
#define SCOPE_FMODE_YSCROLL 6
#define SCOPE_FMODE_THRESH 7
#define SCOPE_FMODE_XDELTA 8
#define SCOPE_FMODE_YDELTA 9

/* 
    p = position, either x or y coord based on hoz
    start = line start,
    len = line len from start,
    hor = is line horizontal
    pattern = line pattern for drawTile
*/
void mode_scope_line_dash(int p, int start, int len, int hor, int pattern){
    char pat= pattern == 0 ? (!hor ? 0x0f : 0x1) : pattern;    
    for(int i=start; i<start+len; i+=8){
        lcd_drawTile(
            hor ? i : p, //x
            !hor ? i : p, //y
            hor ? 4 : 1,
            8,0,0,&pat,
            DRAWBITMAP_XOR
        );
    }
}

void mode_scope_set_amp(int8_t mode){
    if(mode > 3) return;
    if(mode < -2) return;
	y_gain_ctrl = mode;

	gpio_write_bit(GPIOB,15,mode == -1 ? 1 : 0);
	gpio_write_bit(GPIOB,14,mode == -2 ? 1 : 0);
	gpio_write_bit(GPIOB,13,mode == -3 ? 1 : 0);

	gpio_write_bit(GPIOB,12,mode == 1 ? 1 : 0);
	gpio_write_bit(GPIOB,6,mode == 2 ? 1 : 0);
	gpio_write_bit(GPIOB,7,mode == 3 ? 1 : 0);
}

void mode_scope_set_tbase(int8_t tb){
    if(tb < 0) return;
    if(tb > 14) return;
	
    uint32_t sr = 32;
    if(tb == 14){
        sr = 820512;
        adc_set_srate(-1);
    }
    else{
        for(int j=0; j<tb; j++){
            if(j%3 == 1) {
                sr*=5; sr/=2;
            }
            else 
                sr*=2;
        }
        adc_set_srate(sr);
    }
    x_timebase = tb;
    us_per_div = (2000000/sr);// 2 * 1s in us / srate
    us_per_div *= (128/8); //screen w * 8 divs
}

void mode_scope_on_begin(){
    scope_hold = 0; //hold display, dont update on 1
    trig_duration = 0;
    trig_lvl = 32<<6; //threshold for triggering 
    trig_filter = 3;

    x_zoom = 0; //x axis zoom level in sw, adding / taking divisions
    x_timebase = 0; //current time base mode, determines srate
    y_zoom = 0; //zoom level of the y axis in sw
    y_gain_ctrl = 0; //amplifier gain control, > 0 = more gain, < 0 = less gain
    
    y_scroll = 0;
    x_scroll = 0;

    db_ref = 3.3;
    freq_avc = 0; //average freq counter
    
    dy_cursor = 0;
    dt_cursor = 0; //cursor for measuring time selection
    details_view = 1; //show or hide ui
    info_view = 1; //0 no info, 1 == signal freq db, 


    adc_block_init();
	gpio_set_mode(GPIOB,15,GPIO_OUTPUT_OD);// a/10

	gpio_set_mode(GPIOB,14,GPIO_OUTPUT_OD);//
	gpio_set_mode(GPIOB,13,GPIO_OUTPUT_OD);//
	gpio_set_mode(GPIOB,12,GPIO_OUTPUT_OD);// a*2
	gpio_set_mode(GPIOB,6,GPIO_OUTPUT_OD);// a*5
	gpio_set_mode(GPIOB,7,GPIO_OUTPUT_OD);// a*10
	
	mode_scope_set_amp(0);
    mode_scope_set_tbase(10);

    //disabled as pre-amp boards are expected to generate their own supply rails
    ////clock for negative supply rail
    // timer_pause(TIMER4);
    // timer_set_prescaler(TIMER4, 48-1);
    // timer_set_compare(TIMER4, TIMER_CH1, 50-1);
    // timer_set_reload(TIMER4, 100-1);
    // timer_cc_enable(TIMER4, TIMER_CH1);
    // timer_resume(TIMER4);
    // gpio_set_mode(GPIOB,6,GPIO_AF_OUTPUT_PP);

    LOGL("scope: done on_begin");
}

void mode_scope_on_end(){
    adc_block_deinit();
}

void mode_scope_on_process(){
    //button ctrl
    if(io.bscan_down){
        if(io.bscan_down & (1<<K_X)){
            if(stats.fmode) stats.fmode = 0;
            else details_view = !details_view;
        }
        if(io.bscan_down & (1<<K_R)) scope_hold = !scope_hold;
        
        if(io.bscan_down & (1<<K_F1) && stats.fmode == SCOPE_FMODE_XZOOM) stats.fmode = SCOPE_FMODE_XSCROLL; 
        else if(io.bscan_down & (1<<K_F1) && stats.fmode == SCOPE_FMODE_TB) stats.fmode = SCOPE_FMODE_XZOOM;
        else if(io.bscan_down & (1<<K_F1)) stats.fmode = SCOPE_FMODE_TB;
        else if(io.bscan_down & (1<<K_F2) && stats.fmode == SCOPE_FMODE_YZOOM) stats.fmode = SCOPE_FMODE_YSCROLL;
        else if(io.bscan_down & (1<<K_F2) && stats.fmode == SCOPE_FMODE_YAMP) stats.fmode = SCOPE_FMODE_YZOOM;
        else if(io.bscan_down & (1<<K_F2) ) stats.fmode = SCOPE_FMODE_YAMP;
        else if(io.bscan_down & (1<<K_F3) && stats.fmode == SCOPE_FMODE_THRESH) stats.fmode = SCOPE_FMODE_XDELTA; 
        else if(io.bscan_down & (1<<K_F3) && stats.fmode == SCOPE_FMODE_XDELTA) stats.fmode = SCOPE_FMODE_YDELTA;
        else if(io.bscan_down & (1<<K_F3)) stats.fmode = SCOPE_FMODE_THRESH;
        
        if(stats.fmode) details_view = 1;
        io.bscan_down = 0;
    }


    if((io.turns_left || io.turns_right)){
        int tt = io.turns_right - io.turns_left;
        if(stats.fmode == SCOPE_FMODE_TB)
            mode_scope_set_tbase(x_timebase+tt);
        else if(stats.fmode == SCOPE_FMODE_YAMP){
            mode_scope_set_amp(y_gain_ctrl+tt);
        }
        else if(stats.fmode == SCOPE_FMODE_YZOOM){
            y_zoom += tt;
            y_zoom %= 64;
        }
        else if(stats.fmode == SCOPE_FMODE_XZOOM){
            x_zoom += tt;
            x_zoom %= 16;
        }
        else if(stats.fmode == SCOPE_FMODE_YSCROLL){
            y_scroll += tt;
            y_scroll %= 2048;
        }
        else if(stats.fmode == SCOPE_FMODE_XSCROLL){
            x_scroll += tt;
            x_scroll %= 128;
        }
        else if(stats.fmode == SCOPE_FMODE_THRESH) {
            if(io.bstate & (1<<K_Y)) tt*=25;
            trig_lvl -= tt;
            trig_lvl %= 4096;
        }
        else if(stats.fmode == SCOPE_FMODE_XDELTA){ 
            dt_cursor += tt;
            dt_cursor %= 128;
        }
        else if(stats.fmode == SCOPE_FMODE_YDELTA) {
            dy_cursor += tt;
            dy_cursor %= 32;
        }

        io.turns_left = 0;
        io.turns_right = 0;
    }

    const int spl_count = 512;
    auto s16b = (int16_t*)shared_int32_1024;  
    //signal acquisition, busy wait
    if(!scope_hold){
        timer_pause(TIMER3);
	    adc_block_get((uint16_t*)s16b,spl_count);
        while(IS_ADC_BUSY){};
        timer_resume(TIMER3);
    }

    //signal conditioning 
    trig_pos = 0;
    trig_duration = 0;
	rms_total = 0;
    int spl = 0;
    const int tt = trig_lvl;
    for(int i=1; i<spl_count; i++){
        spl = s16b[i];
		auto a = double(spl-2048)/2048.0;
		rms_total += sqrt(a*a)*2;

		trig_duration++;
        auto tr_spl_a = s16b[i-1]>>trig_filter;
        auto tr_spl_b = s16b[i]>>trig_filter;
        tr_spl_a <<= trig_filter;
        tr_spl_b <<= trig_filter;
        if(tr_spl_a < tt && tr_spl_b >= tt){
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
	db_level = 20.0*log10((rms_total/double(spl_count)));

    lcd_clear();
		
        //plot data
        auto tr = trig_pos*(x_zoom+1);
        for(int i=tr; i<128+tr; i++){
            int yy = s16b[(i/(x_zoom+1))+x_scroll]-2048;
            yy *= -(y_zoom+1);
            yy >>= 6;
            yy += 32;
            yy += y_scroll;
            lcd_drawVline(i-tr,0,yy);
        }
		
        //info background
        if(details_view){
            char p = 0xff;
            lcd_drawTile(0,0,128,8,0,0,&p,DRAWBITMAP_XOR);
            char str[32];

            if(stats.fmode == SCOPE_FMODE_TB){
                if(us_per_div < 1000)
                    snprintf(str,32,"T/Div: %dus",us_per_div);
                else
                    snprintf(str,32,"T/Div: %dms",us_per_div/1000);
                lcd_drawString(0,0,sys_font,str);
            }
            else if(stats.fmode == SCOPE_FMODE_XSCROLL || stats.fmode == SCOPE_FMODE_XZOOM){
                snprintf(str,32,"X*%d X->%d",x_zoom+1, x_scroll);
                lcd_drawString(0,0,sys_font,str);
            }
            else if(stats.fmode == SCOPE_FMODE_YAMP){
                snprintf(str,32,"Y_AMP:%d",y_gain_ctrl);
                lcd_drawString(0,0,sys_font,str);
            }
            else if(stats.fmode == SCOPE_FMODE_YSCROLL || stats.fmode == SCOPE_FMODE_YZOOM){
                snprintf(str,32,"Y*%d Y^%d",y_zoom+1, y_scroll);
                lcd_drawString(0,0,sys_font,str);
            }
            else if(stats.fmode == SCOPE_FMODE_THRESH){
                snprintf(str,32,"Thresh:%d%% (%d)",(trig_lvl>>6)*100 / 64,trig_lvl);
                lcd_drawString(0,0,sys_font,str);
            }
            else if(stats.fmode == SCOPE_FMODE_XDELTA || stats.fmode == SCOPE_FMODE_YDELTA){
                auto dtt = (dt_cursor*us_per_div*8/2)/128;
                const char* dtt_unit = "us"; 
                snprintf(str,32,"DX:%d%s DY:%d",dtt,dtt_unit,dy_cursor);
                lcd_drawString(0,0,sys_font,str);
            }
            else {
                snprintf(str,32,"%ddB f:%dHz",int(db_level), int(frequency));				
                lcd_drawString(0,0,sys_font,str);
            }

            //if(details_view == 2){
                p=0x1;    
                lcd_drawTile(0,32,128,1,0,0,&p,DRAWBITMAP_XOR); 
            //}

            //thresh indicator
            if(stats.fmode == SCOPE_FMODE_THRESH){
                auto tt = trig_lvl-2048;
                tt *= y_zoom;
                tt >>= 6;
                tt += 2048;
                mode_scope_line_dash(tt,0,128,1,0);
            }
            //delta indicator
            if(stats.fmode == SCOPE_FMODE_YDELTA || stats.fmode == SCOPE_FMODE_XDELTA){
                mode_scope_line_dash(dy_cursor,0,128,1,0);
                mode_scope_line_dash(64-dy_cursor,0,128,1,0);
                mode_scope_line_dash(dt_cursor,0,64,0,0);
            }
            else{
                //time base divisions
                for(int x=0; x<8; x++){
                    mode_scope_line_dash(x*16, 8,64-8, 0,0x55);
                }
            }
        }

        if(scope_hold){
            lcd_drawChar(128-6,0,sys_font,'H');
        }

    lcd_update();

}
