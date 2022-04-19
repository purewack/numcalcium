#pragma once
#include <USBComposite.h>
#include <Wire.h>
#include <SPI.h>
//#include <Serial.h>

// void midiNoteOffHandle(int ch, int note, int vel);
// void midiNoteOnHandle(int ch, int note, int vel);
// class USBMIDI_Recv: public USBMIDI;
// USBMIDI_Recv usbm;
extern USBHID HID;
extern HIDKeyboard Keyboard;

// SPIClass SPI_2(2);