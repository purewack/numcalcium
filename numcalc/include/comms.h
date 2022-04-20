#pragma once
#include <USBComposite.h>
#include <Wire.h>
#include <Wire_slave.h>
#include <SPI.h>
//#include <Serial.h>


extern USBHID HID;
extern HIDKeyboard USB_keyboard;
extern USBMIDI USB_midi;
extern USBAUDIO USB_audio;
extern SPIClass SPI2(2);