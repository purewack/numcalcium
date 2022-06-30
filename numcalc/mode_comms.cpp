#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"



void mode_comms_on_begin(){
	LOGL("begin comms");
    //Wire.begin();
    LOGL("wire begin");
	lcd_clear();
	lcd_drawString(0,32-8,sys_font,"ZZZ");
	lcd_drawString(8,32-3,sys_font,"XXX");
	lcd_drawString(16,32,sys_font,"YYY");
	lcd_update();
}

void mode_comms_on_end(){
    //Wire.end();
    LOGL("wire end");
}

void mode_comms_on_process(){
	return;    
byte error, address;
    int nDevices;
  
    Serial.println("Scanning...");
  
    nDevices = 0;
    for(address = 1; address < 127; address++ ) 
    {
      // The i2c_scanner uses the return value of
      // the Write.endTransmisstion to see if
      // a device did acknowledge to the address.
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
  
      if (error == 0)
      {
        Serial.print("I2C device found at address 0x");
        if (address<16) 
          Serial.print("0");
        Serial.print(address,HEX);
        Serial.println("  !");
  
        nDevices++;
      }
      else if (error==4) 
      {
        Serial.print("Unknown error at address 0x");
        if (address<16) 
          Serial.print("0");
        Serial.println(address,HEX);
      }    
    }
    if (nDevices == 0)
      Serial.println("No I2C devices found\n");
    else
      Serial.println("done\n");
  
    delay(500);           // wait 5 seconds for next scan
}
