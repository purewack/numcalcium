#include "py/runtime.h"
#include "py/objstr.h"
#include "py/obj.h"
#include "py/stream.h"
#include "py/builtin.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "../font/gohu13.h"
#include "vt100.h"
#include "board.h"

uint8_t lineBuf[1024*2];
static SemaphoreHandle_t spi_semaphore;
static QueueHandle_t buffer_queue;

typedef struct {
    const uint8_t *buffer;
    int size;
    int x;
    int y;
    int width;
    int height;
} buffer_data_t;

// Utility functions
void driver_send_cmd(uint8_t cmd) {
    gpio_set_level(BOARD_PIN_LCD_DC, 0); // Command mode
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd
    };
    spi_device_transmit(lcdspi_handle, &t);
}

void driver_send_data(uint8_t data) {
    gpio_set_level(BOARD_PIN_LCD_DC, 1); // Data mode
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &data
    };
    spi_device_transmit(lcdspi_handle, &t);
}

void driver_start_pixel(){
    gpio_set_level(BOARD_PIN_LCD_DC, 1); // Data mode
}
void driver_send_pixel_data(void* pixel, uint32_t bits){
    spi_transaction_t t = {
        .length = bits,
        .tx_buffer = pixel
    };
    spi_device_transmit(lcdspi_handle, &t);
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
        .pin_bit_mask =  (1ULL << BOARD_PIN_LCD_DC) | (1ULL << BOARD_PIN_LCD_LED),
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&io_conf);
    
    driver_setup();
    driver_fill(0,0,X_SIZE,Y_SIZE, COL_BLACK);
    gpio_set_level(BOARD_PIN_LCD_LED, 1);  // Turn on backlight
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


bool driver_ansiIsLeft(const unsigned char* text){
	// if(text[0] == '\033' && text[1] == '['){
	// 	int digits = 0;
		
	// }
	return false;
}

bool driver_ansiIsErase(const unsigned char* text){
	if(text[0] == '\033'){
		return text[1] == '[' && text[2] == 'K';
	}
	return false;
}


void driver_print(const unsigned char* text, const uint32_t len, int16_t *col, int16_t *line, const uint16_t color, const uint16_t bg, const uint8_t scale){
	
    if(scale > 4) {
//        DEBUG_printf("scale too large %d",scale);
        return;
    }

    if(scale < 1) {
//        DEBUG_printf("scale too small %d",scale);
        return;
    }

    int i=0;
    for(i=0; i<len; i++){

        char c = text[i];

        if(c == '\n'){
            *line += 1;
			if(*line >= Y_CHAR/scale){
				*line = 0;
			}
            // if(LFCR)
            // 	*col = 0;
			driver_fill(0,*line * ((int)font_tall) * scale,X_SIZE,(int)font_tall * scale, bg);
            continue;
        }

        if(c == '\r'){
            *col = 0;
            continue;
        }

		if(driver_ansiIsErase(&text[i])) {
			i+=2;
			driver_fill(
				*col * ((int)font_wide * scale),
				*line * ((int)font_tall * scale), 
				X_SIZE,
				(int)font_tall * scale,
				bg
			);
			continue;
		}

        if(c == '\b'){
            *col -= 1;
			if(*col < 0){
				*col = X_CHAR/scale;
				*line -= 1;
				if(*line < 0){
					*line = Y_CHAR/scale;
				}
			}
			continue;
        }
		
		int xx = (*col  * font_wide * scale);
		int yy = (*line * font_tall * scale);
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

        uint32_t charStart = ch * font_wide;
        uint16_t *buf = (uint16_t*)lineBuf;

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

        driver_start_pixel();
        driver_send_pixel_data(buf,16 * font_wide * font_tall * scale * scale);
        driver_end_pixel();

		*col += 1;
		if(*col >= X_CHAR/scale){
			*col = 0;
			*line += 1;
			if(*line >= Y_CHAR/scale){
				*line = 0;
			}			
			driver_fill(0,*line * ((int)font_tall) * scale,X_SIZE,(int)font_tall * scale, bg);
		}
    
    }
    
}

static void driver_send_buffer(buffer_data_t buffer_data){
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

static void driver_buffer_task(void *pvParameters){
    buffer_data_t buffer_data;

    while(1){        
        if(xQueueReceive(buffer_queue, &buffer_data, portMAX_DELAY)) {
            driver_send_buffer(buffer_data);
            xSemaphoreGive(spi_semaphore);
        }
    }
}



typedef struct _lcd_obj_t {
    mp_obj_base_t base;
	bool new;
	uint16_t bg;
	uint16_t color;

    int16_t col;
    int16_t line;
    uint8_t scale;
    bool LFCR;
    
	bool rgbSwap;
    bool invert;
} lcd_obj_t;

static lcd_obj_t lcd_instance = {{&lcd_type}};


static mp_obj_t buffer(size_t n_args, const mp_obj_t *args) {

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[1], &bufinfo, MP_BUFFER_READ);
    
    buffer_data_t buffer_data;
    buffer_data.buffer = bufinfo.buf;
    buffer_data.size = bufinfo.len;
    buffer_data.x = mp_obj_get_int(args[2]);
    buffer_data.y = mp_obj_get_int(args[3]);
    buffer_data.width = buffer_data.x + mp_obj_get_int(args[4]) - 1;
    buffer_data.height = buffer_data.y + mp_obj_get_int(args[5]) - 1;
    
    // Check if the semaphore is available
    if (xSemaphoreTake(spi_semaphore, 0) == pdTRUE) {
        xSemaphoreGive(spi_semaphore);
        
        if(n_args == 7 && mp_obj_is_true(args[6])){
            driver_send_buffer(buffer_data);
            return mp_const_none;
        }
        // No transfer is taking place, queue the buffer data
        xQueueSend(buffer_queue, &buffer_data, portMAX_DELAY);
    } else {
        // Transfer is taking place, block until it's done
        xQueueSend(buffer_queue, &buffer_data, portMAX_DELAY);
        xSemaphoreTake(spi_semaphore, portMAX_DELAY);
        xSemaphoreGive(spi_semaphore);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(buffer_obj, 6,7, buffer);



static mp_obj_t send_cmd(mp_obj_t self_in, mp_obj_t v) {
    const char c = mp_obj_get_int(v);
    driver_send_cmd(c);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(send_cmd_obj, send_cmd);

static mp_obj_t send_data(mp_obj_t self_in, mp_obj_t v) {
    const char c = mp_obj_get_int(v);
    driver_send_data(c);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(send_data_obj, send_data);

static mp_obj_t reset(mp_obj_t self_in) {
	driver_setup();
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(reset_obj, reset);

static mp_obj_t clear(mp_obj_t self_in) {
    lcd_obj_t *self = &lcd_instance;
    driver_fill(0,0,X_SIZE,Y_SIZE, self->bg);
    self->col = 0;
    self->line = 0;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(clear_obj, clear);

static mp_obj_t fill(size_t n_args, const mp_obj_t *args) {
	// lcd_obj_t* self = MP_OBJ_TO_PTR(args[0]);
    driver_fill(mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]),mp_obj_get_int(args[4]),mp_obj_get_int(args[5]));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR(fill_obj, 6, fill);

static mp_obj_t plot(size_t n_args, const mp_obj_t *args) {
	// lcd_obj_t* self = MP_OBJ_TO_PTR(args[0]);
    driver_pixel(mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR(plot_obj, 4, plot);


static mp_obj_t cursor(size_t n_args, const mp_obj_t *args) {
    lcd_obj_t *self = &lcd_instance;
	if(n_args > 1)
    	self->col = mp_obj_get_int(args[1]);
	if(n_args > 2)
    	self->line = mp_obj_get_int(args[2]);

	mp_obj_t tuple[2];
	tuple[0] = mp_obj_new_int(self->col);
	tuple[1] = mp_obj_new_int(self->line);
	if(n_args == 1)
    return mp_obj_new_tuple(2, tuple);
	else
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cursor_obj, 1, 3, cursor);

static mp_obj_t print(size_t n_args, const mp_obj_t *args) {
    lcd_obj_t *self = &lcd_instance;
//    DEBUG_printf("LCD %d, %d \n",self->scale,self->color);

    mp_check_self(mp_obj_is_str_or_bytes(args[1]));
    GET_STR_DATA_LEN(args[1], c_text, c_text_len);
    driver_print(c_text,c_text_len, &self->col, &self->line, self->color, self->bg, self->scale);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR(print_text_obj, 2, print);


static mp_obj_t options(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
	static const mp_arg_t allowed_args[] = {
        { MP_QSTR_self,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_foreground, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_background, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_rgbSwap, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1 } },
        { MP_QSTR_scale, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_invert, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
    lcd_obj_t *self = &lcd_instance;
	
	if((int)(args[1].u_int) >= 0) {self->color = args[1].u_int;}
	if((int)(args[2].u_int) >= 0) {self->bg = args[2].u_int;}
	if((int)(args[3].u_int) >= 0) {
		self->rgbSwap = args[3].u_int;
		driver_send_cmd(0x36);  // Memory data access control (MADCTL)
		driver_send_data(0x60 | (self->rgbSwap ? 0x8 : 0)); // Row/column swap, RGB order
	}
	if((int)(args[4].u_int) >= 1) {self->scale = args[4].u_int;}
	if((int)(args[5].u_int) >= 0) {
		self->invert = args[5].u_int;
		if(self->invert){
			driver_send_cmd(0x20);  // Inversion on (for proper colors)
		}
		else{    
			driver_send_cmd(0x21);
		}
	}

    mp_obj_t current_options = mp_obj_new_dict(0);
    mp_obj_dict_store(current_options, MP_OBJ_NEW_QSTR(MP_QSTR_background), mp_obj_new_int(self->bg));
    mp_obj_dict_store(current_options, MP_OBJ_NEW_QSTR(MP_QSTR_foreground), mp_obj_new_int(self->color));
    mp_obj_dict_store(current_options, MP_OBJ_NEW_QSTR(MP_QSTR_scale), mp_obj_new_int(self->scale));
    mp_obj_dict_store(current_options, MP_OBJ_NEW_QSTR(MP_QSTR_invert), mp_obj_new_int(self->invert));
    return current_options;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(options_obj, 1, options);



static mp_uint_t lcd_stream_write(mp_obj_t self_in, const void *buf, mp_uint_t size, int *errcode) {
    lcd_obj_t *self = &lcd_instance;
	driver_print((const unsigned char *)buf,size, &self->col, &self->line, self->color, self->bg, self->scale);
    return size; 
}

static mp_uint_t lcd_stream_read(mp_obj_t self_in, void *buf, mp_uint_t size, int *errcode) {
    return 0; 
}

static mp_uint_t lcd_stream_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    return 0; 
}


static mp_obj_t lcd_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    if (!lcd_instance.new) {
        
        driver_init();

        lcd_instance.base.type = type;
        lcd_instance.scale = 1;
        lcd_instance.color = COL_WHITE;
        lcd_instance.bg = COL_BLACK;
        lcd_instance.new = true;

        // Create the semaphore and buffer queue
        spi_semaphore = xSemaphoreCreateBinary();
        xSemaphoreGive(spi_semaphore);
        buffer_queue = xQueueCreate(1, sizeof(buffer_data_t));

        // Create the SPI task
        xTaskCreatePinnedToCore(driver_buffer_task, "driver_buffer_task", 4096, NULL, 1, NULL, 1);
    }
    return (mp_obj_t)&lcd_instance;
}


#define BMP_FILE_HEADER_SIZE 14

// Convert 24-bit BGR to 16-bit RGB565
static inline uint16_t bgr_to_rgb565(uint8_t b, uint8_t g, uint8_t r) {
    uint16_t rgb565 = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
    return ((rgb565 & 0xff)<<8) | (rgb565>>8);
}

// In-place nearest neighbor scaling
static void scale_image_in_place(uint16_t *fb_pixels, int width, int height, int scale) {
    int dst_width = width * scale;
    for (int y = height - 1; y >= 0; y--) {
        for (int x = width - 1; x >= 0; x--) {
            uint16_t color = fb_pixels[y * width + x];
            for (int sy = 0; sy < scale; sy++) {
                for (int sx = 0; sx < scale; sx++) {
                    fb_pixels[((y * scale) + sy) * dst_width + ((x * scale) + sx)] = color;
                }
            }
        }
    }
}

static mp_obj_t parse_bmp(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_file, ARG_fb, ARG_scale };
    mp_arg_t allowed_args[] = {
        { MP_QSTR_self, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_file, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_fb, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_scale, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
    };

    mp_arg_val_t arg_vals[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, arg_vals);

    mp_obj_t file_obj = arg_vals[ARG_file].u_obj;
    mp_obj_t fb_obj = arg_vals[ARG_fb].u_obj;
    int scale = arg_vals[ARG_scale].u_int;

    if (scale < 1) {
        mp_raise_ValueError("Scale must be >= 1");
    }

    mp_buffer_info_t fb_buf;
    uint16_t *fb_pixels = NULL;
    if (fb_obj != MP_OBJ_NULL) {
        mp_get_buffer_raise(fb_obj, &fb_buf, MP_BUFFER_WRITE);
        if (fb_buf.len < 2) {
            mp_raise_ValueError("Framebuffer too small");
        }
        fb_pixels = (uint16_t *)fb_buf.buf;
    }

    mp_obj_t read_meth = mp_load_attr(file_obj, MP_QSTR_read);
    mp_obj_t seek_meth = mp_load_attr(file_obj, MP_QSTR_seek);
    
    // Seek to beginning and read BMP file header
    mp_obj_t seek_args[2] = { MP_OBJ_NEW_SMALL_INT(0), MP_OBJ_NEW_SMALL_INT(0) };
    mp_call_function_n_kw(seek_meth, 2, 0, seek_args);
    
    mp_obj_t file_reader = mp_call_function_n_kw(read_meth, 1, 0, (mp_obj_t[]){ MP_OBJ_NEW_SMALL_INT(BMP_FILE_HEADER_SIZE) });
    mp_buffer_info_t file_header_buf;
    mp_get_buffer_raise(file_reader, &file_header_buf, MP_BUFFER_READ);
    uint8_t *file_header_data = (uint8_t *)file_header_buf.buf;
    
    if (file_header_buf.len < BMP_FILE_HEADER_SIZE || file_header_data[0] != 'B' || file_header_data[1] != 'M') {
        mp_raise_ValueError("Invalid BMP file");
    }
    
    uint32_t dib_header_size = 0;
    
    file_reader = mp_call_function_n_kw(read_meth, 1, 0, (mp_obj_t[]){ MP_OBJ_NEW_SMALL_INT(1) });
    mp_buffer_info_t dib_header;
    mp_get_buffer_raise(file_reader, &dib_header, MP_BUFFER_READ);
    dib_header_size = (uint32_t)((uint32_t *)dib_header.buf)[0];
    DEBUG_printf("DIB size %d\n", dib_header_size);

    file_reader = mp_call_function_n_kw(read_meth, 1, 0, (mp_obj_t[]){ MP_OBJ_NEW_SMALL_INT(dib_header_size - 1) });
    mp_get_buffer_raise(file_reader, &dib_header, MP_BUFFER_READ);
    uint8_t *dib_header_data = (uint8_t *)dib_header.buf;

    uint32_t pixel_offset = *(uint32_t*)(file_header_data + 10);
    int32_t width = *(int32_t*)(dib_header_data + 4 - 1);
    int32_t height = *(int32_t*)(dib_header_data + 8 - 1);
    uint16_t bpp = *(uint16_t*)(dib_header_data + 14 - 1);
    
    if (bpp != 1 && bpp != 4 && bpp != 8 && bpp != 16 && bpp != 24) {
        mp_raise_ValueError("Only 1-bit, 4-bit, 8-bit, 16-bit, and 24-bit BMP supported");
    }
    
    int row_size = ((width * bpp + 31) / 32) * 4;
    int abs_height = height < 0 ? -height : height;
    int flipped = height > 0;

    DEBUG_printf("Details %d %d %d %d Rows %d %d Scale %d\n", width, height, flipped, bpp, row_size, abs_height,scale);
            
    mp_obj_t img_dict = mp_obj_new_dict(0);
    mp_obj_dict_store(img_dict, mp_obj_new_str("width", strlen("width")), mp_obj_new_int(width * scale)); 
    mp_obj_dict_store(img_dict, mp_obj_new_str("height", strlen("height")), mp_obj_new_int(height * scale));
    mp_obj_dict_store(img_dict, mp_obj_new_str("bpp", strlen("bpp")), mp_obj_new_int(bpp));

    if (fb_pixels == NULL) {
        uint32_t size = scale * scale * width * abs_height * sizeof(uint16_t);
        fb_pixels = m_malloc(size);
        fb_buf.buf = fb_pixels;
        fb_buf.len = size;
        mp_obj_dict_store(img_dict, mp_obj_new_str("buffer", strlen("buffer")), mp_obj_new_bytearray_by_ref(size, fb_pixels));
    }
    
    if (bpp == 1 || bpp == 4 || bpp == 8) {
        int palette_size = (bpp == 1) ? 2 * 4 : (bpp == 4) ? 16 * 4 : 256 * 4;
        
        mp_obj_t seek_palette_args[2] = { MP_OBJ_NEW_SMALL_INT(BMP_FILE_HEADER_SIZE + dib_header_size), MP_OBJ_NEW_SMALL_INT(0) };
        mp_call_function_n_kw(seek_meth, 2, 0, seek_palette_args);

        mp_obj_t palette_data = mp_call_function_n_kw(read_meth, 1, 0, (mp_obj_t[]){ MP_OBJ_NEW_SMALL_INT(palette_size) });
        mp_buffer_info_t palette_buf;
        mp_get_buffer_raise(palette_data, &palette_buf, MP_BUFFER_READ);
        uint8_t *palette = (uint8_t *)palette_buf.buf;
        
        DEBUG_printf("Palette colors %d %p, Pixel offset %d\n", palette_buf.len / 4, palette_buf.buf, pixel_offset);
            
        mp_obj_t seek_pixel_args[2] = { MP_OBJ_NEW_SMALL_INT(pixel_offset), MP_OBJ_NEW_SMALL_INT(0) };
        mp_call_function_n_kw(seek_meth, 2, 0, seek_pixel_args);

        for (int y = 0; y < abs_height; y++) {
            mp_obj_t row_data = mp_call_function_n_kw(read_meth, 1, 0, (mp_obj_t[]){ MP_OBJ_NEW_SMALL_INT(row_size) });
            mp_buffer_info_t row_info;
            mp_get_buffer_raise(row_data, &row_info, MP_BUFFER_READ);
            uint8_t *row_buf = (uint8_t *)row_info.buf;
            
            if (row_info.len == 0) {
                DEBUG_printf("Failed to read row data\n");
                mp_raise_ValueError("Failed to read row data");
            }
            
            if (row_info.len < row_size) {
                mp_raise_ValueError("Unexpected end of file");
            }
            
            int dst_y = flipped ? (abs_height - 1 - y) : y;
            if (bpp == 1) {
                for (int x = 0; x < width; x += 8) {
                    uint8_t byte = row_buf[x / 8];
                    for (int bit = 0; bit < 8 && x + bit < width; bit++) {
                        uint8_t index = (byte >> (7 - bit)) & 1;
                        uint8_t b = palette[index * 4 + 0];
                        uint8_t g = palette[index * 4 + 1];
                        uint8_t r = palette[index * 4 + 2];
                        fb_pixels[dst_y * width + x + bit] = bgr_to_rgb565(b, g, r);
                    }
                }
            } else if (bpp == 4) {
                for (int x = 0; x < width; x += 2) {
                    if (x / 2 >= row_info.len) {
                        mp_raise_ValueError("Unexpected end of row data");
                    }
                    uint8_t byte = row_buf[x / 2];
                    uint8_t index1 = byte >> 4;
                    uint8_t index2 = byte & 0x0F;
                    uint8_t b1 = palette[index1 * 4 + 0];
                    uint8_t g1 = palette[index1 * 4 + 1];
                    uint8_t r1 = palette[index1 * 4 + 2];
                    uint8_t b2 = palette[index2 * 4 + 0];
                    uint8_t g2 = palette[index2 * 4 + 1];
                    uint8_t r2 = palette[index2 * 4 + 2];
                    fb_pixels[dst_y * width + x] = bgr_to_rgb565(b1, g1, r1);
                    if (x + 1 < width) {
                        fb_pixels[dst_y * width + x + 1] = bgr_to_rgb565(b2, g2, r2);
                    }
                }
            } else if (bpp == 8) {
                for (int x = 0; x < width; x++) {
                    uint8_t index = row_buf[x];
                    uint8_t b = palette[index * 4 + 0];
                    uint8_t g = palette[index * 4 + 1];
                    uint8_t r = palette[index * 4 + 2];
                    fb_pixels[dst_y * width + x] = bgr_to_rgb565(b, g, r);
                }
            }
        }
    } else if (bpp == 16) {
        mp_obj_t seek_pixel_args[2] = { MP_OBJ_NEW_SMALL_INT(pixel_offset), MP_OBJ_NEW_SMALL_INT(0) };
        mp_call_function_n_kw(seek_meth, 2, 0, seek_pixel_args);
        
        for (int y = 0; y < abs_height; y++) {
            mp_obj_t row_data = mp_call_function_n_kw(read_meth, 1, 0, (mp_obj_t[]){ MP_OBJ_NEW_SMALL_INT(row_size) });
            mp_buffer_info_t row_info;
            mp_get_buffer_raise(row_data, &row_info, MP_BUFFER_READ);
            uint8_t *row_buf = (uint8_t *)row_info.buf;
            
            DEBUG_printf("ROW %d %p\n", row_info.len, row_info.buf);
            if (row_info.len == 0) {
                DEBUG_printf("Failed to read row data\n");
                mp_raise_ValueError("Failed to read row data");
            }
            
            if (row_info.len < row_size) {
                mp_raise_ValueError("Unexpected end of file");
            }
            
            int dst_y = flipped ? (abs_height - 1 - y) : y;
            for (int x = 0; x < width; x++) {
                uint16_t pixel = *(uint16_t*)(row_buf + x * 2);
                fb_pixels[dst_y * width + x] = pixel;
            }
        }
    } else if (bpp == 24) {
        mp_obj_t seek_pixel_args[2] = { MP_OBJ_NEW_SMALL_INT(pixel_offset), MP_OBJ_NEW_SMALL_INT(0) };
        mp_call_function_n_kw(seek_meth, 2, 0, seek_pixel_args);
        
        for (int y = 0; y < abs_height; y++) {
            mp_obj_t row_data = mp_call_function_n_kw(read_meth, 1, 0, (mp_obj_t[]){ MP_OBJ_NEW_SMALL_INT(row_size) });
            mp_buffer_info_t row_info;
            mp_get_buffer_raise(row_data, &row_info, MP_BUFFER_READ);
            uint8_t *row_buf = (uint8_t *)row_info.buf;
            
            DEBUG_printf("ROW %d %p\n", row_info.len, row_info.buf);
            if (row_info.len == 0) {
                DEBUG_printf("Failed to read row data\n");
                mp_raise_ValueError("Failed to read row data");
            }
            
            if (row_info.len < row_size) {
                mp_raise_ValueError("Unexpected end of file");
            }
            
            int dst_y = flipped ? (abs_height - 1 - y) : y;
            for (int x = 0; x < width; x++) {
                uint8_t b = row_buf[x * 3 + 0];
                uint8_t g = row_buf[x * 3 + 1];
                uint8_t r = row_buf[x * 3 + 2];
                fb_pixels[dst_y * width + x] = bgr_to_rgb565(b, g, r);
            }
        }
    }

    // Scale the image in place if needed
    if (scale > 1) {
        int dst_width = width * scale;
        int dst_height = abs_height * scale;
        int fb_size = dst_width * dst_height * sizeof(uint16_t);
        
        if (fb_buf.len < fb_size) {
            DEBUG_printf("%d %d\n",fb_buf.len , fb_size);
            mp_raise_ValueError("Framebuffer too small for scaled image");
        }
        
        scale_image_in_place(fb_pixels, width, abs_height, scale);
    }
    
    if (fb_obj == MP_OBJ_NULL) {
        return img_dict;
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(parse_bmp_obj, 2, parse_bmp);

static const mp_rom_map_elem_t lcd_module_locals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_Terminal) },

    { MP_ROM_QSTR(MP_QSTR_BLACK), MP_ROM_INT(COL_BLACK) },
    { MP_ROM_QSTR(MP_QSTR_WHITE), MP_ROM_INT(COL_WHITE) },
    { MP_ROM_QSTR(MP_QSTR_RED), MP_ROM_INT(COL_RED) },
    { MP_ROM_QSTR(MP_QSTR_GREEN), MP_ROM_INT(COL_GREEN) },
    { MP_ROM_QSTR(MP_QSTR_BLUE), MP_ROM_INT(COL_BLUE) },
    { MP_ROM_QSTR(MP_QSTR_PURPLE), MP_ROM_INT(COL_PURPLE) },
    { MP_ROM_QSTR(MP_QSTR_YELLOW), MP_ROM_INT(COL_YELLOW) },
    { MP_ROM_QSTR(MP_QSTR_CYAN), MP_ROM_INT(COL_CYAN) },
    
    { MP_ROM_QSTR(MP_QSTR__sendcmd), MP_ROM_PTR(&send_cmd_obj) },
    { MP_ROM_QSTR(MP_QSTR__senddata), MP_ROM_PTR(&send_data_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&clear_obj) },
    { MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&fill_obj) },
    { MP_ROM_QSTR(MP_QSTR_plot), MP_ROM_PTR(&plot_obj) },
    { MP_ROM_QSTR(MP_QSTR_cursor), MP_ROM_PTR(&cursor_obj) },
    { MP_ROM_QSTR(MP_QSTR_print), MP_ROM_PTR(&print_text_obj) },
    { MP_ROM_QSTR(MP_QSTR_buffer), MP_ROM_PTR(&buffer_obj) },
    { MP_ROM_QSTR(MP_QSTR_options), MP_ROM_PTR(&options_obj)  }, 
    { MP_ROM_QSTR(MP_QSTR_parseBMP), MP_ROM_PTR(&parse_bmp_obj)  }, 
};
static MP_DEFINE_CONST_DICT(lcd_module_locals, lcd_module_locals_table);

static const mp_stream_p_t lcd_stream_p = {
    .write = lcd_stream_write,
    .read = lcd_stream_read,
	.ioctl = lcd_stream_ioctl,
    .is_text = false,
};

MP_DEFINE_CONST_OBJ_TYPE(
    lcd_type,
    MP_QSTR_Terminal,
    MP_TYPE_FLAG_ITER_IS_STREAM,
	make_new, lcd_make_new,
	locals_dict, &lcd_module_locals,
	protocol, &lcd_stream_p
);
