
#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"

void mode_comms_on_begin(){
#ifndef DEBUG
    Serial.begin()
#endif
    SPI2.begin();
    Wire.begin();
}

void mode_comms_on_end(){
#ifndef DEBUG
    Serial.end();
#endif
    SPI2.end();
    Wire.end();
}

void mode_comms_on_press(int i){

}

void mode_comms_on_release(int i){
    
}

void mode_comms_on_gfx(){

}