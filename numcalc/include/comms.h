#pragma once
#include <USBComposite.h>
#include <Wire.h>
#include <SPI.h>
//#include <Serial.h>


extern USBHID HID;
extern HIDKeyboard USB_keyboard;
extern USBMIDI USB_midi;
extern USBAUDIO USB_audio;

// SPIClass SPI_2(2);