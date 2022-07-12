#include "include/audio.h"
#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"
#include "libintdsp/_source/init.c"
#include "libintdsp/_source/nodes.c"

struct sig_gen_t
{
    double f;
    double a;
    int shape;
} sig_gen;

void mode_audio_on_begin(){
    soft_i2s_init();
    libintdsp_init(&gg,[](int16_t ph){
        return int16_t(32766.0 * sin(2.0 * 3.1415 * double(ph)/double(LUT_COUNT)));
    });
    
    if(!osc) osc = (osc_t*)(new_osc(&gg,"S")->processor);
    set_osc_freq(osc,4400,31250);
    
    lcd_clear();
    drawTitle();
    lcd_update();
}

void mode_audio_on_end(){
    soft_i2s_deinit();
}

void mode_audio_on_process(){
    if(abuf.req){ 
        int s = 0;
        int e = abuf.buf_len>>1;
        if(abuf.req == 2){
        s = abuf.buf_len>>1;
        e = abuf.buf_len;
        }
        abuf.req = 0;

        for(int i=s; i<e; i+=2){
            proc_osc((void*)(osc));
            int s = (osc->io->out*50)>>8;
            abuf.buf[i] = s;
            abuf.buf[i+1] = s;
        }
    }
}
