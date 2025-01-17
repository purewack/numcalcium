#include "py/runtime.h"
#include "py/objstr.h"
#include "py/obj.h"
#include "py/stream.h"
#include "py/builtin.h"

#include "vt100.h"
#include "board.h"

extern uint8_t font_tall;
extern uint8_t font_wide;
extern uint16_t font_count;
extern uint8_t font_data [576];

uint8_t lineBuf[1024*2];

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
    
    driver_send_cmd(0x21); 

    driver_send_cmd(0x13);  // Normal display mode on
    driver_send_cmd(0x29);  // Display on
}

void driver_init() {
    // Configure backlight, CS, DC, and Reset pins
    gpio_config_t io_conf = {
        .pin_bit_mask =  (1ULL << TFT_DC) | (1ULL << TFT_BL),
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&io_conf);
    
    driver_setup();
    driver_fill(0,0,X_SIZE,Y_SIZE, COL_BLACK);
    gpio_set_level(TFT_BL, 1);  // Turn on backlight
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
        DEBUG_printf("scale too large %d",scale);
        return;
    }

    if(scale < 1) {
        DEBUG_printf("scale too small %d",scale);
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

static lcd_obj_t *lcd_instance = NULL;

const mp_obj_type_t lcd_type;

// singleton object

// lcd_obj.bg = COL_BLACK;
// lcd_obj.color = COL_WHITE;
// lcd_obj.line = 0;
// lcd_obj.col = 0;
// lcd_obj.scale = 2;
// lcd_obj.LFCR = 0;
// lcd_obj.rgbSwap = 0;
// lcd_obj.invert = 0;
//static const lcd_obj_t lcd_obj = {{&lcd_type},COL_BLACK,COL_WHITE,0,0,2,0,0,0};


static mp_obj_t buffer(size_t n_args, const mp_obj_t *args) {
	// lcd_obj_t* self_in = MP_OBJ_TO_PTR(args[0]);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[1], &bufinfo, MP_BUFFER_READ);

	const int xx = mp_obj_get_int(args[2]);
	const int yy = mp_obj_get_int(args[3]);
    int xw = xx + mp_obj_get_int(args[4]) - 1;
    int yh = yy + mp_obj_get_int(args[5]) - 1;
    
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
static MP_DEFINE_CONST_FUN_OBJ_VAR(buffer_obj, 4, buffer);



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
    lcd_obj_t *self = lcd_instance;
    driver_fill(0,0,X_SIZE,Y_SIZE, self->bg);
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
    lcd_obj_t *self = lcd_instance;
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
    lcd_obj_t *self = lcd_instance;
    DEBUG_printf("LCD %d, %d \n",self->scale,self->color);

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
    
    lcd_obj_t *self = lcd_instance;
	
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
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(options_obj, 1, options);



static mp_uint_t lcd_stream_write(mp_obj_t self_in, const void *buf, mp_uint_t size, int *errcode) {
    lcd_obj_t *self = lcd_instance;
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
    if (lcd_instance == NULL) {
        driver_init();
        lcd_instance = m_new_obj(lcd_obj_t);
        lcd_instance->base.type = type;
        lcd_instance->scale = 2;
        lcd_instance->color = COL_WHITE;
        lcd_instance->bg = COL_BLACK;
        DEBUG_printf("new lcd\n");
    }
    return MP_OBJ_FROM_PTR(lcd_instance);
}


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
    
    { MP_ROM_QSTR(MP_QSTR_cmd), MP_ROM_PTR(&send_cmd_obj) },
    { MP_ROM_QSTR(MP_QSTR_data), MP_ROM_PTR(&send_data_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&clear_obj) },
    { MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&fill_obj) },
    { MP_ROM_QSTR(MP_QSTR_plot), MP_ROM_PTR(&plot_obj) },
    { MP_ROM_QSTR(MP_QSTR_cursor), MP_ROM_PTR(&cursor_obj) },
    { MP_ROM_QSTR(MP_QSTR_print), MP_ROM_PTR(&print_text_obj) },
    { MP_ROM_QSTR(MP_QSTR_buffer), MP_ROM_PTR(&buffer_obj) },
    { MP_ROM_QSTR(MP_QSTR_options), MP_ROM_PTR(&options_obj)  }, 
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


uint8_t font_tall = 8;
uint8_t font_wide = 6;
uint16_t font_count = 96;
uint8_t font_data [576] = {
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
