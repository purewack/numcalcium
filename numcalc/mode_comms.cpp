
#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"

// int comms_new_bytes = 0;
// void mode_comms_on_begin(){
// // #ifndef DEBUG
// //     Serial1.begin(9600);
// // #endif
// //     SPI_2.begin();
// //     Wire.begin();    
//     u8g2log.begin(32, 7, u8log_buffer);	// connect to hud, assign buffer
//     u8g2log.print("\f");
//     u8g2log.print(">");
//     stats.gfx_refresh = 1;
//     stats.gfx_log = 1;
//     comms_new_bytes = 1;
//  }

// void mode_comms_on_end(){
    
// // #ifndef DEBUG
// //     Serial1.end();
// // #endif
// //     SPI_2.end();
// //     Wire.end();
//     //u8g2log.end();
// }

// int mode_comms_on_press(int i){
//     return 1;
// }

// int mode_comms_on_release(int i){
//     return 1;
// }

// void mode_comms_on_loop(unsigned long dt){
//     if(Serial.available()){
//         while(Serial.available() > 0){
//             char c = Serial.read();
            
//             if(c == '\n') 
//                 u8g2log.print("\n>");
//             else
//                 u8g2log.print(c);
            
//         }    
//         comms_new_bytes = 1;
//         stats.gfx_refresh = 1;
//     }
// }

// void mode_comms_on_gfx(){
//     if(!comms_new_bytes) return;
//     hud.setFont(LCD_LOG_FONT);	
//     hud.drawLog(0, 16, u8g2log);
//     comms_new_bytes = 0;
//     LOGL("gfx");
// }

// TwoWire WIRE2 (2,I2C_FAST_MODE);
// #define Wire WIRE2

void mode_comms_on_begin(){
    Wire.begin();
    LOGL("wire begin");
}

void mode_comms_on_end(){
    Wire.end();
    LOGL("wire end");
}

void mode_comms_on_process(){
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