
#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"

void mode_midi_on_begin(){
    
}

void mode_midi_on_end(){

}

void mode_midi_on_press(int i){
    Serial.println("midi off");
    USB_midi.sendNoteOn(0,60+i,127);
}

void mode_midi_on_release(int i){
    Serial.println("midi off");
    USB_midi.sendNoteOff(0,60+i,127);
}

// void mode_midi_on_gfx(){
   
// }