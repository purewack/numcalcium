#include "py/runtime.h"
#include "py/objstr.h"
#include "py/obj.h"
#include "py/stream.h"
#include "py/builtin.h"

#include "modvt100.h"
#include "board.h"

settings_t lcd;

extern uint8_t fonttiny_tall;
extern uint8_t fonttiny_wide;
extern uint16_t fonttiny_count;
extern uint8_t fonttiny_data [576];

uint8_t lineBuf[1024*2];

bool ansiIsLeft(const unsigned char* text){
	if(lcd.ignoreEscapes) return false;
	// if(text[0] == '\033' && text[1] == '['){
	// 	int digits = 0;
		
	// }
	return false;
}

bool ansiIsErase(const unsigned char* text){
	if(lcd.ignoreEscapes) return false;
	if(text[0] == '\033'){
		return text[1] == '[' && text[2] == 'K';
	}
	return false;
}

// Utility functions
void driver_send_cmd(uint8_t cmd) {
    gpio_set_level(TFT_DC, 0); // Command mode
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd
    };
    spi_device_transmit(lcdspi_handle, &t);
}

void driver_send_data(uint8_t data) {
    gpio_set_level(TFT_DC, 1); // Data mode
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &data
    };
    spi_device_transmit(lcdspi_handle, &t);
}

void driver_start_pixel(){
    gpio_set_level(TFT_DC, 1); // Data mode
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
    
    if(lcd.invert)
    driver_send_cmd(0x20);  // Inversion on (for proper colors)
    else    
    driver_send_cmd(0x21); 

    driver_send_cmd(0x13);  // Normal display mode on
    driver_send_cmd(0x29);  // Display on
}

void lcd_reset() {
    lcd.color = COL_BLACK;
    lcd.x = 0;
    lcd.y = 0;
    lcd.line = 0;
    lcd.col = 0;
    lcd.scale = 2;
    lcd.LFCR = 0;
	lcd.largeLF = 1;
	lcd.underlineLF = 1;
	lcd.ignoreEscapes = 0;
    lcd.invert = 0;
    driver_setup();
}


void lcd_updateNewlineEnd(){
	lcd.color = COL_BLACK;
	driver_fill(0,lcd.line * ((int)fonttiny_tall) * lcd.scale,X_SIZE,(int)fonttiny_tall * lcd.scale * (lcd.largeLF ? 2 : 1));
	
	if(!lcd.underlineLF) return;
	lcd.color = COL_WHITE;
	driver_fill(0,fonttiny_tall* lcd.scale + lcd.line * lcd.scale * ((int)fonttiny_tall),X_SIZE,1);
}


void lcd_init() {
    // Configure backlight, CS, DC, and Reset pins
    gpio_config_t io_conf = {
        .pin_bit_mask =  (1ULL << TFT_DC) | (1ULL << TFT_BL),
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&io_conf);
    
    // // SPI setup
    // spi_bus_config_t buscfg = {
    //     .miso_io_num = -1,
    //     .mosi_io_num = TFT_MOSI,
    //     .sclk_io_num = TFT_SCLK,
    //     .quadwp_io_num = -1,
    //     .quadhd_io_num = -1,
    // };
    // spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);

    lcd_reset();
    driver_fill(0,0,X_SIZE,Y_SIZE);
	lcd.std = true;
    gpio_set_level(TFT_BL, 1);  // Turn on backlight
}

void driver_fill(int16_t x, int16_t y, int16_t w, int16_t h) {
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
	
	uint32_t c0 = (lcd.color & 0xFF);
	uint32_t c1 = (lcd.color & 0xFF00) >> 8;

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

void driver_pixel() {
    int xx = lcd.x;
    int xw = lcd.x;
    int yy = lcd.y;
    int yh = lcd.y;
    driver_send_cmd(0x2A); 
    driver_send_data((xx & 0x100) >> 8); driver_send_data(xx & 0xff); 
    driver_send_data((xw & 0x100) >> 8); driver_send_data(xw & 0xff); 

    driver_send_cmd(0x2B); 
    driver_send_data(0x00); driver_send_data(yy + Y_OFFSET);
    driver_send_data(0x00); driver_send_data(yh + Y_OFFSET); 

    driver_send_cmd(0x2C);
    driver_send_data(lcd.color >> 8); 
    driver_send_data(lcd.color & 0xFF);
}

void lcd_print(const unsigned char* text, uint32_t len) {
   
    int i=0;
    for(i=0; i<len; i++){

        char c = text[i];

		if(!lcd.ignoreEscapes){
        if(c == '\n'){
            lcd.line += 1;
			if(lcd.line >= Y_CHAR/lcd.scale){
				lcd.line = 0;
			}
            if(lcd.LFCR)
            	lcd.col = 0;
			lcd_updateNewlineEnd();
            continue;
        }

        if(c == '\r'){
            lcd.col = 0;
            continue;
        }

		if(ansiIsErase(&text[i])) {
			i+=2;
			lcd.color = COL_BLACK;
			driver_fill(
				lcd.col * ((int)fonttiny_wide * lcd.scale),
				lcd.line * ((int)fonttiny_tall * lcd.scale), 
				X_SIZE,
				(int)fonttiny_tall * lcd.scale
			);
			continue;
		}

        if(c == '\b'){
            lcd.col -= 1;
			if(lcd.col < 0){
				lcd.col = X_CHAR/lcd.scale;
				lcd.line -= 1;
				if(lcd.line < 0){
					lcd.line = Y_CHAR/lcd.scale;
				}
			}
			continue;
        }
		}
		
		int xx = (lcd.col  * fonttiny_wide * lcd.scale);
		int yy = (lcd.line * fonttiny_tall * lcd.scale);
        int xw = xx + (fonttiny_wide * lcd.scale) - 1;
        int yh = yy + (fonttiny_tall * lcd.scale) - 1;

        driver_send_cmd(0x2A); 
        driver_send_data((xx & 0x100) >> 8); driver_send_data(xx & 0xff); 
        driver_send_data((xw & 0x100) >> 8); driver_send_data(xw & 0xff); 

        driver_send_cmd(0x2B); 
        driver_send_data(0x00); driver_send_data(yy + Y_OFFSET);
        driver_send_data(0x00); driver_send_data(yh + Y_OFFSET); 

        driver_send_cmd(0x2C);
        
//        if(c >= 'a' && c <= 'z') c -= 32; //no caps allowed
        char ch = (c < ' ' || c > 126) ? 0 : (c-' '+1);

        uint32_t charStart = ch * fonttiny_wide;
        uint16_t *buf = (uint16_t*)lineBuf;

		for(int xx=0; xx<fonttiny_wide; xx++){
			for(int yy=0; yy<fonttiny_tall; yy++){
				uint16_t color = COL_BLACK;
				if(fonttiny_data[xx + charStart] & (1<<yy))
					color = COL_WHITE;
				
				int ws = fonttiny_wide * lcd.scale;
				int sx = (xx * lcd.scale);
				int sy = (yy * ws * lcd.scale);
				for(int iy=0; iy<lcd.scale; iy++){
					for(int ix=0; ix<lcd.scale; ix++){
						buf[sx+ix + sy+(iy*ws)] = color;
					}
				}
			}
		}

        driver_start_pixel();
        driver_send_pixel_data(buf,16 * fonttiny_wide * fonttiny_tall * lcd.scale * lcd.scale);
        driver_end_pixel();

		lcd.col += 1;
		if(lcd.col >= X_CHAR/lcd.scale){
			lcd.col = 0;
			lcd.line += 1;
			if(lcd.line >= Y_CHAR/lcd.scale){
				lcd.line = 0;
			}
			lcd_updateNewlineEnd();
		}
    
    }
    
}

static mp_obj_t buffer(mp_obj_t w, mp_obj_t h, mp_obj_t pixels) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(pixels, &bufinfo, MP_BUFFER_READ);

	const int ww = mp_obj_get_int(w);
	const int hh = mp_obj_get_int(h);

	int xx = (lcd.x);
	int yy = (lcd.y);
    int xw = xx + ww - 1;
    int yh = yy + hh - 1;
    
    DEBUG_printf("buffer stats: %d %d %d %d %d\n",xx,yy,xw,yh,bufinfo.len);
    
    driver_send_cmd(0x2A); 
    driver_send_data((xx & 0x100) >> 8); driver_send_data(xx & 0xff); 
    driver_send_data((xw & 0x100) >> 8); driver_send_data(xw & 0xff); 

    driver_send_cmd(0x2B); 
    driver_send_data(0x00); driver_send_data(yy + Y_OFFSET);
    driver_send_data(0x00); driver_send_data(yh + Y_OFFSET);
 
	driver_send_cmd(0x2C); 

	driver_start_pixel();
    int size = bufinfo.len;
    int xferred = 0;
    do{
        int count = size - xferred;
        if(count > 1024) count = 1024;
        driver_send_pixel_data(bufinfo.buf + xferred,8*count);
        xferred += count;
    }while(xferred != size);
	driver_end_pixel();

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(buffer_obj, buffer);



static mp_obj_t send_cmd(mp_obj_t v) {
    const char c = mp_obj_get_int(v);
    driver_send_cmd(c);
    return mp_const_none;
}
static mp_obj_t send_data(mp_obj_t v) {
    const char c = mp_obj_get_int(v);
    driver_send_data(c);
    return mp_const_none;
}
static mp_obj_t reset() {
    lcd_reset();
    return mp_const_none;
}
static mp_obj_t clear() {
    lcd.x = 0;
    lcd.y = 0;
    lcd.line = 0;
    lcd.col = 0;
    lcd.color = COL_BLACK;
    driver_fill(0,0,X_SIZE,Y_SIZE);
    return mp_const_none;
}
static mp_obj_t fill(mp_obj_t w, mp_obj_t h) {
    const unsigned int ww = mp_obj_get_int(w);
    const unsigned int hh = mp_obj_get_int(h);
    driver_fill(lcd.x, lcd.y, ww,hh);
    return mp_const_none;
}
static mp_obj_t plot() {
    driver_pixel();
    return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_1(send_cmd_obj, send_cmd);
static MP_DEFINE_CONST_FUN_OBJ_1(send_data_obj, send_data);
static MP_DEFINE_CONST_FUN_OBJ_0(reset_obj, reset);
static MP_DEFINE_CONST_FUN_OBJ_0(clear_obj, clear);
static MP_DEFINE_CONST_FUN_OBJ_2(fill_obj, fill);
static MP_DEFINE_CONST_FUN_OBJ_0(plot_obj, plot);

static mp_obj_t cursor(mp_obj_t x, mp_obj_t y) {
    lcd.x = mp_obj_get_int(x);
    lcd.y = mp_obj_get_int(y);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(cursor_obj, cursor);

static mp_obj_t caret(mp_obj_t x, mp_obj_t y) {
    lcd.col = mp_obj_get_int(x);
    lcd.line = mp_obj_get_int(y);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(caret_obj, caret);

static mp_obj_t color(mp_obj_t col) {
    lcd.color = mp_obj_get_int(col);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(color_obj, color);


static mp_obj_t print(mp_obj_t text) {
    int uu = lcd.underlineLF;
    lcd.underlineLF = false;
    mp_check_self(mp_obj_is_str_or_bytes(text));
    GET_STR_DATA_LEN(text, c_text, c_text_len);
    lcd_print(c_text,c_text_len);
    lcd.underlineLF = uu;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(print_obj, print);



static mp_obj_t console(mp_obj_t v) {
    lcd.std = mp_obj_get_int(v);
	printf("LCD mode %d",lcd.std);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(console_obj, console);

static mp_obj_t options(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
		{ MP_QSTR_LFCR, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_largeLF, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_underlineLF, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_escapes, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_rgbSwap, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_scale, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
	
	if(args[0].u_int >= 0) lcd.LFCR = args[0].u_int;
	if(args[1].u_int >= 0) lcd.largeLF = args[1].u_int;
	if(args[2].u_int >= 0) lcd.underlineLF = args[2].u_int;
	if(args[3].u_int >= 0) lcd.ignoreEscapes = !args[3].u_int;
	if(args[4].u_int >= 0) lcd.rgbSwap = !args[4].u_int;
	if(args[5].u_int >= 1) lcd.scale = args[5].u_int;
    
    
    driver_send_cmd(0x36);  // Memory data access control (MADCTL)
    driver_send_data(0x60 | (lcd.rgbSwap ? 0x8 : 0)); // Row/column swap, RGB order

    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(options_obj, 0, options);



typedef struct _lcd_stream_obj_t {
    mp_obj_base_t base;
} lcd_stream_obj_t;

static mp_obj_t lcd_stream_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    lcd_stream_obj_t *self = mp_obj_malloc(lcd_stream_obj_t, type);
	
    return MP_OBJ_FROM_PTR(self);
}

static const mp_rom_map_elem_t lcd_stream_locals_dict_table[] = {
};
static MP_DEFINE_CONST_DICT(lcd_stream_locals_dict, lcd_stream_locals_dict_table);


static mp_uint_t lcd_stream_write(mp_obj_t self_in, const void *buf, mp_uint_t size, int *errcode) {
    lcd_print((const unsigned char *)buf,size);
    return size; 
}

static mp_uint_t lcd_stream_read(mp_obj_t self_in, void *buf, mp_uint_t size, int *errcode) {
    return 0; 
}

static mp_uint_t lcd_stream_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    return 0; 
}

static const mp_stream_p_t lcd_stream_p = {
    .write = lcd_stream_write,
    .read = lcd_stream_read,
	.ioctl = lcd_stream_ioctl,
    .is_text = false,
};

static MP_DEFINE_CONST_OBJ_TYPE(
    lcd_stream_type,
    MP_QSTR_Stream,
    MP_TYPE_FLAG_ITER_IS_STREAM,
	make_new, lcd_stream_make_new,
	locals_dict, &lcd_stream_locals_dict,
	protocol, &lcd_stream_p
);



static const mp_rom_map_elem_t lcd_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_terminal) },

    { MP_ROM_QSTR(MP_QSTR_BLACK), MP_ROM_INT(COL_BLACK) },
    { MP_ROM_QSTR(MP_QSTR_WHITE), MP_ROM_INT(COL_WHITE) },
    { MP_ROM_QSTR(MP_QSTR_RED), MP_ROM_INT(COL_RED) },
    { MP_ROM_QSTR(MP_QSTR_GREEN), MP_ROM_INT(COL_GREEN) },
    { MP_ROM_QSTR(MP_QSTR_BLUE), MP_ROM_INT(COL_BLUE) },
    { MP_ROM_QSTR(MP_QSTR_PURPLE), MP_ROM_INT(COL_PURPLE) },
    { MP_ROM_QSTR(MP_QSTR_YELLOW), MP_ROM_INT(COL_YELLOW) },
    { MP_ROM_QSTR(MP_QSTR_CYAN), MP_ROM_INT(COL_CYAN) },
    
    { MP_ROM_QSTR(MP_QSTR_cmd), MP_ROM_PTR(&send_cmd_obj) },
    { MP_ROM_QSTR(MP_QSTR_data), MP_ROM_PTR(&send_data_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&clear_obj) },
    { MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&fill_obj) },
    { MP_ROM_QSTR(MP_QSTR_plot), MP_ROM_PTR(&plot_obj) },
    { MP_ROM_QSTR(MP_QSTR_color), MP_ROM_PTR(&color_obj) },
    { MP_ROM_QSTR(MP_QSTR_cursor), MP_ROM_PTR(&cursor_obj) },
    { MP_ROM_QSTR(MP_QSTR_caret), MP_ROM_PTR(&caret_obj) },
    { MP_ROM_QSTR(MP_QSTR_print), MP_ROM_PTR(&print_obj) },
    { MP_ROM_QSTR(MP_QSTR_buffer), MP_ROM_PTR(&buffer_obj) },
    { MP_ROM_QSTR(MP_QSTR_console), MP_ROM_PTR(&console_obj) },
    { MP_ROM_QSTR(MP_QSTR_options), (mp_obj_t)&options_obj  }, 
	{ MP_ROM_QSTR(MP_QSTR_Stream), MP_ROM_PTR(&lcd_stream_type) },
};
static MP_DEFINE_CONST_DICT(lcd_module_globals, lcd_module_globals_table);

const mp_obj_module_t lcd_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&lcd_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_terminal, lcd_module);





uint8_t fonttiny_tall = 8;
uint8_t fonttiny_wide = 6;
uint16_t fonttiny_count = 96;
uint8_t fonttiny_data [576] = {
	0x7f,
	0x55,
	0x6b,
	0x55,
	0x6b,
	0x7f,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x5e,
	0x5e,
	0x0,
	0x0,
	0x0,
	0x6,
	0x0,
	0x6,
	0x0,
	0x0,
	0x14,
	0x3e,
	0x14,
	0x3e,
	0x14,
	0x0,
	0x4,
	0x2a,
	0x7f,
	0x2a,
	0x10,
	0x0,
	0x0,
	0x26,
	0x16,
	0x8,
	0x34,
	0x32,
	0x0,
	0x34,
	0x4a,
	0x54,
	0x20,
	0x50,
	0x0,
	0x0,
	0x6,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x3c,
	0x42,
	0x0,
	0x0,
	0x0,
	0x0,
	0x42,
	0x3c,
	0x0,
	0x0,
	0x0,
	0xa,
	0x4,
	0xa,
	0x0,
	0x0,
	0x8,
	0x8,
	0x3e,
	0x8,
	0x8,
	0x0,
	0x0,
	0x0,
	0x8,
	0x6,
	0x0,
	0x0,
	0x0,
	0x8,
	0x8,
	0x8,
	0x8,
	0x0,
	0x0,
	0x0,
	0x40,
	0x0,
	0x0,
	0x0,
	0x0,
	0x60,
	0x18,
	0x6,
	0x0,
	0x0,
	0x0,
	0x3c,
	0x42,
	0x42,
	0x3c,
	0x0,
	0x0,
	0x0,
	0x44,
	0x7e,
	0x40,
	0x0,
	0x0,
	0x44,
	0x62,
	0x52,
	0x4c,
	0x0,
	0x0,
	0x24,
	0x42,
	0x52,
	0x2c,
	0x0,
	0x0,
	0x1e,
	0x50,
	0x7c,
	0x40,
	0x0,
	0x0,
	0x4e,
	0x52,
	0x52,
	0x22,
	0x0,
	0x0,
	0x3c,
	0x52,
	0x52,
	0x20,
	0x0,
	0x0,
	0x2,
	0x42,
	0x32,
	0xe,
	0x0,
	0x0,
	0x2c,
	0x52,
	0x52,
	0x2c,
	0x0,
	0x0,
	0xc,
	0x52,
	0x52,
	0x3c,
	0x0,
	0x0,
	0x0,
	0x0,
	0x14,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x34,
	0x0,
	0x0,
	0x0,
	0x0,
	0x8,
	0x14,
	0x22,
	0x0,
	0x0,
	0x14,
	0x14,
	0x14,
	0x14,
	0x0,
	0x0,
	0x22,
	0x14,
	0x8,
	0x0,
	0x0,
	0x0,
	0x4,
	0x52,
	0x52,
	0xc,
	0x0,
	0x3c,
	0x42,
	0x99,
	0x99,
	0x92,
	0x1c,
	0x0,
	0x7c,
	0x12,
	0x12,
	0x7c,
	0x0,
	0x0,
	0x7e,
	0x4a,
	0x4a,
	0x34,
	0x0,
	0x0,
	0x3c,
	0x42,
	0x42,
	0x42,
	0x0,
	0x0,
	0x7e,
	0x42,
	0x42,
	0x3c,
	0x0,
	0x0,
	0x7e,
	0x52,
	0x52,
	0x42,
	0x0,
	0x0,
	0x7e,
	0x12,
	0x12,
	0x2,
	0x0,
	0x0,
	0x3c,
	0x42,
	0x52,
	0x34,
	0x0,
	0x0,
	0x7e,
	0x10,
	0x10,
	0x7e,
	0x0,
	0x0,
	0x0,
	0x42,
	0x7e,
	0x42,
	0x0,
	0x0,
	0x20,
	0x42,
	0x3e,
	0x2,
	0x0,
	0x0,
	0x7e,
	0x10,
	0x28,
	0x46,
	0x0,
	0x0,
	0x7e,
	0x40,
	0x40,
	0x40,
	0x0,
	0x0,
	0x7e,
	0x4,
	0x8,
	0x4,
	0x7e,
	0x0,
	0x7e,
	0x4,
	0x8,
	0x10,
	0x7e,
	0x0,
	0x3c,
	0x42,
	0x42,
	0x3c,
	0x0,
	0x0,
	0x7e,
	0x12,
	0x12,
	0xc,
	0x0,
	0x0,
	0x1c,
	0x22,
	0x22,
	0x7c,
	0x0,
	0x0,
	0x7e,
	0x12,
	0x12,
	0x6c,
	0x0,
	0x0,
	0xc,
	0x52,
	0x52,
	0x20,
	0x0,
	0x0,
	0x2,
	0x2,
	0x7e,
	0x2,
	0x2,
	0x0,
	0x3e,
	0x40,
	0x40,
	0x3e,
	0x0,
	0x0,
	0x1e,
	0x60,
	0x60,
	0x1e,
	0x0,
	0x0,
	0x3e,
	0x40,
	0x78,
	0x40,
	0x3e,
	0x0,
	0x42,
	0x24,
	0x18,
	0x24,
	0x42,
	0x0,
	0x6,
	0x8,
	0x70,
	0x8,
	0x6,
	0x0,
	0x42,
	0x62,
	0x52,
	0x4a,
	0x46,
	0x0,
	0x0,
	0x7e,
	0x42,
	0x0,
	0x0,
	0x0,
	0x6,
	0x18,
	0x60,
	0x0,
	0x0,
	0x0,
	0x0,
	0x42,
	0x7e,
	0x0,
	0x0,
	0x0,
	0xc,
	0x2,
	0xc,
	0x0,
	0x0,
	0x40,
	0x40,
	0x40,
	0x40,
	0x40,
	0x40,
	0x0,
	0x2,
	0x4,
	0x8,
	0x0,
	0x0,
	0x0,
	0x0,
	0x70,
	0x28,
	0x78,
	0x0,
	0x0,
	0x0,
	0x7c,
	0x48,
	0x30,
	0x0,
	0x0,
	0x0,
	0x30,
	0x48,
	0x48,
	0x0,
	0x0,
	0x0,
	0x30,
	0x48,
	0x7c,
	0x0,
	0x0,
	0x0,
	0x38,
	0x68,
	0x50,
	0x0,
	0x0,
	0x0,
	0x10,
	0x78,
	0x14,
	0x0,
	0x0,
	0x0,
	0x8,
	0x54,
	0x38,
	0x0,
	0x0,
	0x0,
	0x7c,
	0x10,
	0x60,
	0x0,
	0x0,
	0x0,
	0x0,
	0x74,
	0x0,
	0x0,
	0x0,
	0x40,
	0x80,
	0x74,
	0x0,
	0x0,
	0x0,
	0x0,
	0x7c,
	0x10,
	0x68,
	0x0,
	0x0,
	0x0,
	0x3c,
	0x40,
	0x0,
	0x0,
	0x70,
	0x8,
	0x10,
	0x8,
	0x70,
	0x0,
	0x0,
	0x70,
	0x8,
	0x8,
	0x70,
	0x0,
	0x0,
	0x30,
	0x48,
	0x48,
	0x30,
	0x0,
	0x0,
	0x0,
	0xf0,
	0x28,
	0x10,
	0x0,
	0x0,
	0x10,
	0x28,
	0xf0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x78,
	0x8,
	0x0,
	0x0,
	0x0,
	0x0,
	0x58,
	0x68,
	0x0,
	0x0,
	0x0,
	0x0,
	0x3c,
	0x48,
	0x0,
	0x0,
	0x0,
	0x0,
	0x78,
	0x40,
	0x78,
	0x0,
	0x0,
	0x38,
	0x40,
	0x38,
	0x0,
	0x0,
	0x38,
	0x40,
	0x20,
	0x40,
	0x38,
	0x0,
	0x0,
	0x0,
	0x68,
	0x10,
	0x68,
	0x0,
	0x0,
	0x98,
	0xa0,
	0x78,
	0x0,
	0x0,
	0x0,
	0x48,
	0x68,
	0x58,
	0x0,
	0x0,
	0x0,
	0x10,
	0x7c,
	0x82,
	0x0,
	0x0,
	0x0,
	0x0,
	0xfe,
	0x0,
	0x0,
	0x0,
	0x0,
	0x82,
	0x7c,
	0x10,
	0x0,
	0x0,
	0x0,
	0x10,
	0x8,
	0x10,
	0x8,
	0x0 
};
