#include "font.h"
#include "font_hex_codes.h"
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
    driver_fill(0,0,X_SIZE,Y_SIZE, 0,NULL);
}

void driver_fill(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, uint16_t* canvas){
    if(x >= X_SIZE) return;
    if(y >= Y_SIZE) return;
    if(x < 0) {
        w -= x;
        x = 0;
    }
    if(y < 0) {
        h -= y;
        y = 0;
    }
	if(w > X_SIZE) w = X_SIZE;
	if(h > Y_SIZE) h = Y_SIZE;

    int xx = x;
    int xw = x + w;
    int yy = y;
    int yh = y + h;

    if(canvas){
        for(int iy=yy; iy<yh; iy++){
            for(int ix=xx; ix<xw; ix++){
                driver_pixel(ix,iy,color,canvas);
            }
        }
        return;
    }

    xw -= 1;
    yh -= 1;

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

void driver_pixel(int16_t x, int16_t y, uint16_t color, uint16_t* canvas) {
    if(canvas){
        uint32_t c0 = (color & 0xFF);
        uint32_t c1 = (color & 0xFF00) >> 8;

        int ii = (x + y*X_SIZE);
        canvas[ii] = c1 + (c0<<8);
        return;
    }

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


bool driver_ansiIsLeft(const unsigned char* text, int *ii, int *count) {
    if (text[0] == '\033') { 
        int index = 1; 
        int chars = 0;

        if (text[index] == '[') { 
            index++;

            if (!(text[index] >= '0' && text[index] <= '9')) {
                return false;
            }

            while (text[index] >= '0' && text[index] <= '9') {
                chars = chars * 10 + (text[index] - '0');
                index++;
            }

            if (text[index] == 'D') {
                *ii = index + 1;  
                *count = chars;   
                return true;
            }
        }
    }
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


void driver_print_font_value(unsigned char ch, uint16_t *buf, uint8_t scale, uint16_t bg, uint16_t color, int font_wide, int font_tall, uint8_t *font_buf, int x_off, int y_off){

    uint8_t bytes_per_height = (font_tall/8+1);
    uint32_t charStart = ch * font_wide;

    uint16_t ws = font_wide * scale;
    uint16_t lws = ws;

    if(x_off != -1 && y_off != -1){
        color = ((color & 0xFF)<<8) + ((color&0xFF00)>>8);
        lws = X_SIZE;
    }
    else{
        x_off = 0;
        y_off = 0;
    }
    
    for(int xx=0; xx<font_wide; xx++){
        for(int yy=0; yy<font_tall; yy++){
            uint16_t cc = bg;
            uint8_t b = font_buf[(xx + charStart)*bytes_per_height + (yy/8)];
            if(b & (1<<(yy%8)))
                cc = color;
            
            uint16_t sx = (xx * scale);
            uint16_t sy = (yy * lws * scale);
            for(int iy=0; iy<scale; iy++){
                for(int ix=0; ix<scale; ix++)
                    buf[sx+ix+x_off + (y_off*lws)+sy+(iy*lws)] = cc;
            }
        }
    }
}


// for printing hex numbers for invisible ascii characters
void driver_print_escape_value(unsigned char ch, uint16_t *buf, int fws, int fts, int x_off, int y_off){
 
    const int font_divide_bound = (FONT_TALL_HEX_CODES>>1);
    const int hw = FONT_WIDE_HEX_CODES;
    const int hh = FONT_TALL_HEX_CODES;
    int lws = fws;
    uint16_t color = 0xeefd;
    if(x_off != -1 && y_off != -1){
        color = 0xfdee;
        lws = X_SIZE;
    }
    else{
        x_off = 0;
        y_off = 0;
    }

    for(uint16_t xx=0; xx<fws; xx++){
        for(uint16_t yy=0; yy<fts; yy++){
            uint16_t cc = color;

            if(xx < hw && yy < hh){
                uint16_t a = (yy >= font_divide_bound) ? ch&0xf : (ch >> 4);
                uint16_t v = xx + (hw*(a%8));
                uint16_t hline = font_data_hex_codes[v]; //((d[(v*2) + 1]) << 0x8) | d[(v*2)];
                hline = hline >> (a > 8 ? font_divide_bound :  0);
                if(hline & (1<<(yy % font_divide_bound)))
                    cc = 0;
            }
            
            buf[(xx+x_off) + (yy+y_off)*lws] = cc;
        }
    }
}

void driver_print(const unsigned char* text, const uint32_t len, float *col, float *line, const uint16_t _color, const uint16_t _bg, const uint8_t scale, const bool autoWrap, font_t* font, uint16_t* canvas, const uint16_t cvW, const uint16_t cvH){
	
    if(scale > 4) {
//        //DEBUG_printf("scale too large %d",scale);
        return;
    }

    if(scale < 1) {
//        //DEBUG_printf("scale too small %d",scale);
        return;
    }

    uint16_t bg = (_bg>>8) | (_bg&0xff)<<8;
    uint16_t color = (_color>>8) | (_color&0xff)<<8;

    int xsize = canvas ? cvW : X_SIZE;
    int ysize = canvas ? cvH : Y_SIZE;
    int xchar = xsize / FONT_WIDE;
    int ychar = ysize / FONT_TALL;
    uint8_t font_wide = FONT_WIDE;
    uint8_t font_tall = FONT_TALL;
    uint8_t *font_buf = (uint8_t*)font_data;
    // uint8_t font_count = FONT_COUNT;
    if(font){
        xchar = xsize / font->xfWide;
        ychar = ysize / font->xfTall;
        font_wide = font->xfWide;
        font_tall = font->xfTall;
        font_buf = font->xfData;
        // font_count = font->xfCount;
    }


    float fts = (float)(font_tall * scale);
    float fws = (float)(font_wide * scale);

    if(*col >= xchar/scale && !autoWrap){
        return;
    }
    if(*line >= ychar/scale && !autoWrap){
        return;
    }

    int i=0;
    for(i=0; i<len; i++){

        unsigned char c = text[i];

        if(c == '\n'){
            *line += 1.f;
			if(*line >= (float)(ychar/scale)){
				*line = 0;
			}
            // if(LFCR)
            // 	*col = 0;
			driver_fill(
                0,
                (int16_t)(*line * fts),
                xsize,
                (uint16_t)fts, 
                _bg, canvas);
            continue;
        }

        if(c == '\r'){
            *col = 0;
            continue;
        }

		if(driver_ansiIsErase(&text[i],&i)) {
			driver_fill(
				(int16_t)(*col * fws),
				(int16_t)(*line * fts), 
				xsize,
				(uint16_t)fts,
				_bg, canvas
			);
			continue;
		}

        int back = 0;
        if(c == '\b' || driver_ansiIsLeft(&text[i],&i,&back)){
            *col -= back ? (float)back : 1.f;
			if(*col < 0){
				*col = (float)(xchar/scale);
				*line -= 1.f;
				if(*line < 0){
					*line = (float)(ychar/scale);
				}
			}
			continue;
        }
		
        char ch = (c < ' ' || c > 126) ? 0 : (c-' '+1);
		int xx = (int)(*col  * fws);
		int yy = (int)(*line * fts);
        int xw = xx + ((int)fws) - 1;
        int yh = yy + ((int)fts) - 1;
        uint16_t *buf = canvas;

        if(!canvas){
        driver_send_cmd(0x2A); 
        driver_send_data((xx & 0x100) >> 8); driver_send_data(xx & 0xff); 
        driver_send_data((xw & 0x100) >> 8); driver_send_data(xw & 0xff); 

        driver_send_cmd(0x2B); 
        driver_send_data(0x00); driver_send_data(yy + Y_OFFSET);
        driver_send_data(0x00); driver_send_data(yh + Y_OFFSET); 

        driver_send_cmd(0x2C);
        
        buf = (uint16_t*)lineBuf;
        xx = -1;
        yy = -1;
        }

        //print escape character code
        if(!ch){
            driver_print_escape_value((unsigned char )c,buf,(int)fws,(int)fts, xx,yy);
        }
        //print character from font map
        else{
            driver_print_font_value(ch,buf,scale, bg,color, font_wide, font_tall, font_buf, xx,yy);
        }
        
        if(!canvas){
        driver_start_pixel();
        driver_send_pixel_data(buf,16 * font_wide * fts * scale);
        driver_end_pixel();
        }

		*col += 1;
		if(*col >= xchar/scale){
            if(!autoWrap){
                return;
            }
			*col = 0;
			*line += 1.f;
			if(*line >= (float)(ychar/scale)){
				*line = 0;
			}			
			driver_fill(
                0,
                (int16_t)(*line * ((float)fts))
                ,xsize,
                (uint16_t)fts,
                _bg, canvas);
		}
    
    }
    
}

void driver_send_buffer(buffer_data_t buffer_data){
    int ww = buffer_data.x + buffer_data.width - 1;
    int hh = buffer_data.y + buffer_data.height - 1;
    driver_send_cmd(0x2A); 
    driver_send_data((buffer_data.x & 0x100) >> 8); driver_send_data(buffer_data.x & 0xff); 
    driver_send_data((ww & 0x100) >> 8); driver_send_data(ww & 0xff); 

    driver_send_cmd(0x2B); 
    driver_send_data(0x00); driver_send_data(buffer_data.y + Y_OFFSET);
    driver_send_data(0x00); driver_send_data(hh + Y_OFFSET);
 
    driver_send_cmd(0x2C); 

    driver_start_pixel();

    if(buffer_data.partial){
        for(int tx_y=0; tx_y<buffer_data.height; tx_y++){
            const void* adr = buffer_data.buffer + (tx_y * X_SIZE * sizeof(uint16_t));
            uint32_t sz = 8 * buffer_data.width * sizeof(uint16_t);
            // DEBUG_printf("line(%d) @%p for sz%d \n",tx_y,adr,sz);
    
            driver_send_pixel_data(
                adr,
                sz
            );
        }
    }
    else{
        int size = buffer_data.size;
        int xferred = 0;
        int limit = 32000;
        do{
            int count = size - xferred;
            if(count > limit) count = limit;
            driver_send_pixel_data(buffer_data.buffer + xferred,8*count);
            xferred += count;
        }
        while(xferred != size);
    }
    driver_end_pixel();
}
