
#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"

void mode_comms_on_begin(){
// #ifndef DEBUG
//     Serial1.begin(9600);
// #endif
//     SPI_2.begin();
//     Wire.begin();
}

void mode_comms_on_end(){
// #ifndef DEBUG
//     Serial1.end();
// #endif
//     SPI_2.end();
//     Wire.end();
}

int mode_comms_on_press(int i){
    return 1;
}

int mode_comms_on_release(int i){
    return 1;
}

void mode_comms_on_loop(unsigned long dt){
    if(dt%100) Serial.println("on loop");
}