#include "include/comms.h"

void midiNoteOffHandle(int ch, int note, int vel){

}
void midiNoteOnHandle(int ch, int note, int vel){

}
class USBMIDI_Recv: public USBMIDI {
 virtual void handleNoteOff(unsigned int channel, unsigned int note, unsigned int velocity) {
    midiNoteOffHandle(channel,note,velocity);
 }
 virtual void handleNoteOn(unsigned int channel, unsigned int note, unsigned int velocity) {
    midiNoteOnHandle(channel,note,velocity);
  }
};
