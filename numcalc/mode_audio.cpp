#include "include/audio.h"
#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"
struct sig_gen_t
{
    double f;
    double a;
    int shape;
    int16_t table[512];
} sig_gen;

void mode_audio_on_begin(){

}

void mode_audio_on_process_end(){

}

void mode_audio_on_process(){
    if(abuf.req){

    }
}