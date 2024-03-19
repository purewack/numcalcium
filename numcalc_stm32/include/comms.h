#pragma once
#include <USBComposite.h>
#include <Wire.h>
// #include <Wire_slave.h>
#include <SPI.h>
//#include <Serial.h>

#define USB_CONN_START 1
#define USB_CONN_END -1

extern USBHID HID;
extern HIDKeyboard USB_keyboard;
extern USBMIDI USB_midi;
extern USBAUDIO USB_audio;
extern SPIClass SPI_2;