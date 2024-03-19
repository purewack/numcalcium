#include "include/audio.h"
#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"
#include "libintdsp/_source/init.c"
#include "libintdsp/_source/nodes.c"

int edit_focus = 1;
int edit_cursor = 0;
int a_gain = 100;
int b_gain = 100;
double a_freq = 440.0;
double b_freq = 1000.0;
double a_phase = 0.0;
double b_phase = 0.0;

int new_params = 0;

void mode_audio_on_begin(){
    soft_i2s_init();
    
    if(!osca) {
        libintdsp_init(&gg,[](int16_t ph){
            return int16_t(32766.0 * sin(2.0 * 3.1415 * double(ph)/double(LUT_COUNT)));
        });
        osca = (osc_t*)(new_osc(&gg,"S")->processor);
        oscb = (osc_t*)(new_osc(&gg,"S")->processor);
    }
    set_osc_freq(osca,4400,31250);
    set_osc_freq(oscb,10000,31250);
    
    lcd_clear();
    drawTitle();
    lcd_update();
    new_params = 1;
}

void mode_audio_on_end(){
    soft_i2s_deinit();
}

void mode_audio_on_process(){
    if(io.bscan_down){
        int a = 0;
        if(io.bscan_down == (1<<K_F1))
            a = 1;
        if(io.bscan_down == (1<<K_F2))
            a = 2;
        if(io.bscan_down == (1<<K_F3))
            a = 3;

        if(io.bscan_down == (1<<K_1))
            edit_focus = 1;
        if(io.bscan_down == (1<<K_2))
            edit_focus = 2;

        edit_cursor = (edit_cursor == a) ? 0 : a;

        io.bscan_down = 0;
        new_params = 1;
    }
    if(io.turns_left || io.turns_right){
        if(edit_cursor == 1 && edit_focus == 1){
            a_freq += double(io.turns_right);//*0.1;
            a_freq -= double(io.turns_left);//*0.1;
            if(a_freq < 0) a_gain = 0;
            if(a_freq > 16000.0) a_gain = 16000.0;

            set_osc_freq(osca,int(a_freq*10),31250);
        }
        else if(edit_cursor == 2 && edit_focus == 1){
            a_gain += io.turns_right;
            a_gain -= io.turns_left;
            if(a_gain < 0) a_gain = 0;
            if(a_gain > 255) a_gain = 255;
        }
        else if(edit_cursor == 3 && edit_focus == 1){
            if(osca->table == sint) osca->table = sawt;
            else osca->table = sint;
        }

        if(edit_cursor == 1 && edit_focus == 2){
            b_freq += double(io.turns_right);//*0.1;
            b_freq -= double(io.turns_left);//*0.1;
            if(b_freq < 0) b_gain = 0;
            if(b_freq > 16000.0) b_gain = 16000.0;

            set_osc_freq(oscb,int(b_freq*10),31250);
        }
        else if(edit_cursor == 2 && edit_focus == 2){
            b_gain += io.turns_right;
            b_gain -= io.turns_left;
            if(b_gain < 0) b_gain = 0;
            if(b_gain > 255) b_gain = 255;
        }
        else if(edit_cursor == 3 && edit_focus == 2){
            if(oscb->table == sint) oscb->table = sawt;
            else oscb->table = sint;
        }

        io.turns_left = 0;
        io.turns_right = 0;
        new_params = 1;
    }

    if(new_params){
        clearProgGFX();
            char str[32];
            
            snprintf(str,32,"[CH 1]");
            lcd_drawString(0,16,sys_font,str);
            snprintf(str,32,"[CH 2]");
            lcd_drawString(64,16,sys_font,str);

            snprintf(str,32,"%dHz",int(a_freq));
            lcd_drawString(0,24,sys_font,str);
            snprintf(str,32,"%dHz",int(b_freq));
            lcd_drawString(64,24,sys_font,str);
            
            //int a_gain_db = int(20.0*log10(double(a_gain)/256.0));
            snprintf(str,32,"%d/256",a_gain);
            lcd_drawString(0,32,sys_font,str);
            snprintf(str,32,"%d/256",b_gain);
            lcd_drawString(64,32,sys_font,str);

            snprintf(str,32,"%s",(osca->table == sint) ? "Sin" : "Saw");
            lcd_drawString(0,32+8,sys_font,str);
            snprintf(str,32,"%s",(oscb->table == sint) ? "Sin" : "Saw");
            lcd_drawString(64,32+8,sys_font,str);

            if(edit_focus){
                char pat = 0xff;
                lcd_drawTile(64*(edit_focus-1),16 + 8*edit_cursor,64,8,0,0,&pat,DRAWBITMAP_XOR);
            }
        updateProgGFX();
        new_params = 0;
    }

    if(abuf.req){ 
        int s = 0;
        int e = abuf.buf_len>>1;
        if(abuf.req == 2){
        s = abuf.buf_len>>1;
        e = abuf.buf_len;
        }
        abuf.req = 0;

        for(int i=s; i<e; i+=2){
            proc_osc((void*)(osca));
            proc_osc((void*)(oscb));
            int spl = (osca->io->out*a_gain);
            if(spl > 8388096) spl = 8388096;
            if(spl < -8388096) spl = -8388096;
            abuf.buf[i] = spl>>8;
            spl = (oscb->io->out*b_gain);
            if(spl > 8388096) spl = 8388096;
            if(spl < -8388096) spl = -8388096;
            abuf.buf[i+1] = spl>>8;
        }
    }
}
