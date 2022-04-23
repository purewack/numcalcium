
#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"


void mode_comms_on_begin(){
// #ifndef DEBUG
//     Serial1.begin(9600);
// #endif
//     SPI_2.begin();
//     Wire.begin();  
    u8g2.setFont(LCD_LOG_FONT);	// set the font for the terminal window
    u8g2log.print("\f");
    u8g2log.print(">");
    stats.gfx_refresh = 1;
    stats.gfx_log = 1;
 }

void mode_comms_on_end(){
    
// #ifndef DEBUG
//     Serial1.end();
// #endif
//     SPI_2.end();
//     Wire.end();
    //u8g2log.end();
}

int mode_comms_on_press(int i){
    return 1;
}

int mode_comms_on_release(int i){
    return 1;
}

void mode_comms_on_loop(unsigned long dt){
    if(Serial.available()){
        u8g2.setFont(LCD_LOG_FONT);	// set the font for the terminal window
        while(Serial.available() > 0){
            char c = Serial.read();
            
            if(c == '\n') 
                u8g2log.print("\n>");
            else
                u8g2log.print(c);
            
            //Serial.println(c);
        }    
        stats.gfx_refresh = 1;
    }
    

}