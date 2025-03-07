#include "../font/gohu13.h"
#include "vt100.h"
#include "board.h"

uint8_t lineBuf[1024*2];
SemaphoreHandle_t spi_semaphore;
QueueHandle_t buffer_queue;


// Utility functions
void driver_send_cmd(uint8_t cmd) {
    gpio_set_level(BOARD_PIN_LCD_DC, 0); // Command mode
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd
    };
    spi_device_polling_transmit(lcdspi_handle, &t);
}

void driver_send_data(uint8_t data) {
    gpio_set_level(BOARD_PIN_LCD_DC, 1); // Data mode
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &data
    };
    spi_device_polling_transmit(lcdspi_handle, &t);
}

void driver_start_pixel(){
    gpio_set_level(BOARD_PIN_LCD_DC, 1); // Data mode
}
void driver_send_pixel_data(const void* pixel, uint32_t bits){
    spi_transaction_t t = {
        .length = bits,
        .tx_buffer = pixel
    };
    spi_device_polling_transmit(lcdspi_handle, &t);
}
void driver_end_pixel(){
}

void driver_setup(){
    driver_send_cmd(0x01);  // Software reset
    driver_send_cmd(0x11);  // Sleep out

    driver_send_cmd(0x36);  // Memory data access control (MADCTL)
    driver_send_data(0x60); // Row/column swap, RGB order

    driver_send_cmd(0x3A);  // Pixel format
    driver_send_data(0x05); // 16 bits per pixel
    
    driver_send_cmd(0x21); 

    driver_send_cmd(0x13);  // Normal display mode on
    driver_send_cmd(0x29);  // Display on
}

void driver_init() {
    // Configure backlight, CS, DC, and Reset pins
    gpio_config_t io_conf = {
        .pin_bit_mask =  (1ULL << BOARD_PIN_LCD_DC),
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&io_conf);
    
    driver_setup();
    driver_fill(0,0,X_SIZE,Y_SIZE, COL_BLACK);
}

void driver_fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color){
	if(w + x > X_SIZE) w -= x;
	if(h + y > Y_SIZE) h -= y;
	if(w > X_SIZE) w = X_SIZE;
	if(h > Y_SIZE) h = Y_SIZE;

    int xx = x;
    int xw = x + w - 1;
    int yy = y;
    int yh = y + h - 1;

    driver_send_cmd(0x2A); 
    driver_send_data((xx & 0x100) >> 8); driver_send_data(xx & 0xff); 
    driver_send_data((xw & 0x100) >> 8); driver_send_data(xw & 0xff); 

    driver_send_cmd(0x2B); 
    driver_send_data(0x00); driver_send_data(yy + Y_OFFSET);
    driver_send_data(0x00); driver_send_data(yh + Y_OFFSET); 

    driver_send_cmd(0x2C); 
	
	uint32_t c0 = (color & 0xFF);
	uint32_t c1 = (color & 0xFF00) >> 8;

    uint32_t xferred = 0;
    uint32_t size = w*h;
    driver_start_pixel();
    do{
        int count = size - xferred;
        uint8_t *buf = (uint8_t*)lineBuf;
        if(count > 1024) count = 1024;
        for (int i = xferred; i < count*2; i+=2) {
            buf[i] = c1;
            buf[i+1] = c0;
        }
        driver_send_pixel_data(lineBuf,count*16);
        xferred += count;
    }while(xferred != size);
    driver_end_pixel();
}

void driver_pixel(uint16_t x, uint16_t y, uint16_t color) {
    int xx = x;
    int xw = x;
    int yy = y;
    int yh = y;
    driver_send_cmd(0x2A); 
    driver_send_data((xx & 0x100) >> 8); driver_send_data(xx & 0xff); 
    driver_send_data((xw & 0x100) >> 8); driver_send_data(xw & 0xff); 

    driver_send_cmd(0x2B); 
    driver_send_data(0x00); driver_send_data(yy + Y_OFFSET);
    driver_send_data(0x00); driver_send_data(yh + Y_OFFSET); 

    driver_send_cmd(0x2C);
    driver_send_data(color >> 8); 
    driver_send_data(color & 0xFF);
}


bool driver_ansiIsLeft(const unsigned char* text, int *ii){
	// if(text[0] == '\033' && text[1] == '['){
	// 	int digits = 0;
		
	// }
	return false;
}

bool driver_ansiIsErase(const unsigned char* text, int *ii){
	if(text[0] == '\033'){
		bool erase = text[1] == '[' && text[2] == 'K';
        if(erase)
            *ii += 2;
        return erase;
	}
	return false;
}


void driver_print_font_value(char ch, uint16_t *buf, uint8_t scale, uint16_t bg, uint16_t color){
    uint32_t charStart = ch * font_wide;
            
    for(int xx=0; xx<font_wide; xx++){
        for(int yy=0; yy<font_tall; yy++){
            uint16_t cc = bg;
            if(font_data[xx + charStart] & (1<<yy))
                cc = color;
            
            int ws = font_wide * scale;
            int sx = (xx * scale);
            int sy = (yy * ws * scale);
            for(int iy=0; iy<scale; iy++){
                for(int ix=0; ix<scale; ix++){
                    buf[sx+ix + sy+(iy*ws)] = cc;
                }
            }
        }
    }
}

void driver_print_escape_value(char ch, uint16_t *buf, uint8_t scale){
    for(int nibble=0; nibble<2; nibble++){
        char value = nibble == 0 ? (ch&0xf0)>>4 : (ch&0xf);
        int xoff = (font_count-8)*font_wide + (value*font_wide)%(8*8);
        
        uint8_t ysize = font_tall>>1;
        uint8_t yoff = (value>8)*ysize;
        uint8_t ytarget = nibble ? ysize : 0;
        printf("nibble:%d, ys:%d, yoff:%d, xoff:%d v:%d\n\r",nibble,ysize,yoff,xoff,value);

        for(int xx=0; xx<font_wide; xx++){
            for(int yy=0; yy<ysize+1; yy++){
                uint16_t cc = 0;
                if(font_data[xx + xoff] & (1<<(yy + yoff)))
                    cc = 0xf83f;

                int ws = font_wide * scale;
                int sx = (xx * scale);
                int sy = ((ytarget+yy) * ws * scale);
                for(int iy=0; iy<scale; iy++){
                    for(int ix=0; ix<scale; ix++){
                        buf[sx+ix + sy+(iy*ws)] = cc;
                    }
                }
            }
        }
    }
}

void driver_print(const unsigned char* text, const uint32_t len, float *col, float *line, const uint16_t _color, const uint16_t _bg, const uint8_t scale){
	
    if(scale > 4) {
//        DEBUG_printf("scale too large %d",scale);
        return;
    }

    if(scale < 1) {
//        DEBUG_printf("scale too small %d",scale);
        return;
    }

    uint16_t bg = (_bg>>8) | (_bg&0xff)<<8;
    uint16_t color = (_color>>8) | (_color&0xff)<<8;

    int i=0;
    for(i=0; i<len; i++){

        char c = text[i];

        if(c == '\n'){
            *line += 1.f;
			if(*line >= (float)(Y_CHAR/scale)){
				*line = 0;
			}
            // if(LFCR)
            // 	*col = 0;
			driver_fill(
                0,
                (uint16_t)(*line * (float)(font_tall * scale)),
                X_SIZE,
                (uint16_t)font_tall * scale, 
                _bg);
            continue;
        }

        if(c == '\r'){
            *col = 0;
            continue;
        }

		if(driver_ansiIsErase(&text[i],&i)) {
			driver_fill(
				(uint16_t)(*col * ((float)font_wide * scale)),
				(uint16_t)(*line * ((float)font_tall * scale)), 
				X_SIZE,
				(int)font_tall * scale,
				_bg
			);
			continue;
		}

        // if(driver_ansiIsLeft(&text[i],&i)) {
        //     driver_fill(
        //         (uint16_t)(*col * ((float)font_wide * scale)),
        //         (uint16_t)(*line * ((float)font_tall * scale)), 
        //         X_SIZE,
        //         (int)font_tall * scale,
        //         _bg
        //     );
        //     continue;
        // }

        if(c == '\b'){
            *col -= 1.f;
			if(*col < 0){
				*col = (float)(X_CHAR/scale);
				*line -= 1.f;
				if(*line < 0){
					*line = (float)(Y_CHAR/scale);
				}
			}
			continue;
        }
		
		int xx = (int)(*col  * (float)(font_wide * scale));
		int yy = (int)(*line * (float)(font_tall * scale));
        int xw = xx + (font_wide * scale) - 1;
        int yh = yy + (font_tall * scale) - 1;

        driver_send_cmd(0x2A); 
        driver_send_data((xx & 0x100) >> 8); driver_send_data(xx & 0xff); 
        driver_send_data((xw & 0x100) >> 8); driver_send_data(xw & 0xff); 

        driver_send_cmd(0x2B); 
        driver_send_data(0x00); driver_send_data(yy + Y_OFFSET);
        driver_send_data(0x00); driver_send_data(yh + Y_OFFSET); 

        driver_send_cmd(0x2C);
        
       

//        if(c >= 'a' && c <= 'z') c -= 32; //no caps allowed
        char ch = (c < ' ' || c > 126) ? 0 : (c-' '+1);
        
        uint16_t *buf = (uint16_t*)lineBuf;

        //print escape character code
        if(!ch){
            driver_print_escape_value(c,buf,scale);
        }
        //print character from font map
        else{
            driver_print_font_value(ch,buf,scale, bg,color);
        }

        driver_start_pixel();
        driver_send_pixel_data(buf,16 * font_wide * font_tall * scale * scale);
        driver_end_pixel();

		*col += 1;
		if(*col >= X_CHAR/scale){
			*col = 0;
			*line += 1.f;
			if(*line >= (float)(Y_CHAR/scale)){
				*line = 0;
			}			
			driver_fill(
                0,
                (uint16_t)(*line * ((float)font_tall * scale))
                ,X_SIZE,
                (int)font_tall * scale,
                _bg);
		}
    
    }
    
}

void driver_send_buffer(buffer_data_t buffer_data){
    driver_send_cmd(0x2A); 
    driver_send_data((buffer_data.x & 0x100) >> 8); driver_send_data(buffer_data.x & 0xff); 
    driver_send_data((buffer_data.width & 0x100) >> 8); driver_send_data(buffer_data.width & 0xff); 

    driver_send_cmd(0x2B); 
    driver_send_data(0x00); driver_send_data(buffer_data.y + Y_OFFSET);
    driver_send_data(0x00); driver_send_data(buffer_data.height + Y_OFFSET);
 
    driver_send_cmd(0x2C); 

    driver_start_pixel();
    int size = buffer_data.size;// buffer_data.width * buffer_data.height * 2 * 8;
    int xferred = 0;
    int limit = 32000;
//            DEBUG_printf("buffer stats: %d %d %d %d %p\n",buffer_data.x,buffer_data.y,buffer_data.width,buffer_data.height,buffer_data.buffer);
    do{
        int count = size - xferred;
        if(count > limit) count = limit;
        driver_send_pixel_data(buffer_data.buffer + xferred,8*count);
        xferred += count;
    }while(xferred != size);
    driver_end_pixel();
}