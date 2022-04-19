#ifndef H_COMMS
#define H_COMMS
#include <USBComposite.h>
#include <Wire.h>
#include <SPI.h>
//#include <Serial.h>

// void midiNoteOffHandle(int ch, int note, int vel);
// void midiNoteOnHandle(int ch, int note, int vel);
// class USBMIDI_Recv: public USBMIDI;
// USBMIDI_Recv usbm;
USBHID HID;
HIDKeyboard Keyboard(HID);

// SPIClass SPI_2(2);
#endif