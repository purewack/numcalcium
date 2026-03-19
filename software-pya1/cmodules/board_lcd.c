#include "py/runtime.h"
#include "py/objstr.h"
#include "py/obj.h"
#include "py/stream.h"
#include "py/builtin.h"
#include "py/objstr.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "vt100.h"
#include "lcd_bmp.h"
#include "board.h"
#include "font.h"

typedef struct _lcd_obj_t {
    mp_obj_base_t base;
	bool new;
	uint16_t bg;
	uint16_t color;

    float col;
    float line;
    uint8_t scale;
    bool LFCR;
    bool addLFCR;
	bool rgbSwap;
    bool invert;
    bool autoWrap;

    bool extFont;
    font_t* font;
    uint16_t* canvas_buf;
} lcd_obj_t;

const mp_obj_type_t lcd_type;
static lcd_obj_t lcd_instance = {{&lcd_type}};

static mp_obj_t buffer(size_t n_args, const mp_obj_t *args) {

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[1], &bufinfo, MP_BUFFER_READ);
    
    buffer_data_t buffer_data;
    buffer_data.buffer = bufinfo.buf;
    buffer_data.size = bufinfo.len;
    buffer_data.x = mp_obj_get_int(args[2]);
    buffer_data.y = mp_obj_get_int(args[3]);
    buffer_data.width = mp_obj_get_int(args[4]);
    buffer_data.height = mp_obj_get_int(args[5]);
    buffer_data.partial = n_args == 7 ? mp_obj_get_int(args[6]) : 0;
    
    // DEBUG_printf("buf (%d,%d) @ (%d,%d) pp:%d \n",buffer_data.x,buffer_data.y, buffer_data.width, buffer_data.height, buffer_data.partial);
    driver_send_buffer(buffer_data);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(buffer_obj, 6,7, buffer);


static mp_obj_t set_canvas(mp_obj_t self_in, mp_obj_t buf_in) {

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_RW);
    
    lcd_obj_t *self = &lcd_instance;
    self->canvas_buf = (uint16_t*)bufinfo.buf;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(set_canvas_obj, set_canvas);


static mp_obj_t unset_canvas(mp_obj_t self_in) {
    lcd_obj_t *self = &lcd_instance;
    if(self->canvas_buf){
        // free(self->canvas_buf);
        self->canvas_buf = NULL;
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(unset_canvas_obj, unset_canvas);


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
    driver_fill(0,0,X_SIZE,Y_SIZE, self->bg, self->canvas_buf);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(clear_obj, clear);

static mp_obj_t fill(size_t n_args, const mp_obj_t *args) {
    lcd_obj_t *self = &lcd_instance;
    driver_fill(mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]),mp_obj_get_int(args[4]),mp_obj_get_int(args[5]),self->canvas_buf);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR(fill_obj, 6, fill);

static mp_obj_t plot(size_t n_args, const mp_obj_t *args) {
    lcd_obj_t *self = &lcd_instance;
    driver_pixel(mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]),self->canvas_buf);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR(plot_obj, 4, plot);


static mp_obj_t cursor(size_t n_args, const mp_obj_t *args) {
    lcd_obj_t *self = &lcd_instance;
	if(n_args > 1)
    	self->col = mp_obj_is_int(args[1]) ? (float)mp_obj_get_int(args[1]) : mp_obj_get_float(args[1]);
	if(n_args > 2)
    	self->line = mp_obj_is_int(args[2]) ? (float)mp_obj_get_int(args[2]) : mp_obj_get_float(args[2]);

    if(n_args == 1){
    	mp_obj_t tuple[2];
    	tuple[0] = mp_obj_new_float(self->col);
    	tuple[1] = mp_obj_new_float(self->line);
        return mp_obj_new_tuple(2, tuple);
    }
	else
	   return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cursor_obj, 1, 3, cursor);


static mp_obj_t options(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
	static const mp_arg_t allowed_args[] = {
        { MP_QSTR_self,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_foreground, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_background, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_rgbSwap, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1 } },
        { MP_QSTR_scale, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_invert, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_addLFCR, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_autoWrap, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
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
    if((int)(args[6].u_int) >= 0) {self->addLFCR = args[6].u_int;}
    if((int)(args[7].u_int) >= 0) {self->autoWrap  = args[7].u_int;}

    mp_obj_t current_options = mp_obj_new_dict(0);
    mp_obj_dict_store(current_options, MP_OBJ_NEW_QSTR(MP_QSTR_background), mp_obj_new_int(self->bg));
    mp_obj_dict_store(current_options, MP_OBJ_NEW_QSTR(MP_QSTR_foreground), mp_obj_new_int(self->color));
    mp_obj_dict_store(current_options, MP_OBJ_NEW_QSTR(MP_QSTR_scale), mp_obj_new_int(self->scale));
    mp_obj_dict_store(current_options, MP_OBJ_NEW_QSTR(MP_QSTR_invert), mp_obj_new_int(self->invert));
    mp_obj_dict_store(current_options, MP_OBJ_NEW_QSTR(MP_QSTR_addLFCR), mp_obj_new_int(self->addLFCR));
    mp_obj_dict_store(current_options, MP_OBJ_NEW_QSTR(MP_QSTR_rgbSwap), mp_obj_new_int(self->rgbSwap));
    mp_obj_dict_store(current_options, MP_OBJ_NEW_QSTR(MP_QSTR_autoWrap), mp_obj_new_int(self->autoWrap));
    return current_options;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(options_obj, 1, options);

static mp_obj_t unloadFont(mp_obj_t self_in) {
    lcd_obj_t *self = &lcd_instance;
    self->font = NULL;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(unload_font_obj, unloadFont);

static mp_obj_t loadFont(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_self,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_width,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_height,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_data,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_count,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    lcd_obj_t *self = &lcd_instance;
    
    uint8_t cWidth = args[1].u_int;
    uint8_t cHeight = args[2].u_int;
    uint8_t cCount = args[4].u_int;
    
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[3].u_obj, &bufinfo, MP_BUFFER_RW);
    
    
    font_t *new_font = (font_t *)malloc(sizeof(font_t));
    if (new_font == NULL) {
        mp_raise_OSError(MP_ENOMEM);
        return mp_const_none;
    }
    
    uint8_t *font_buf = (uint8_t *)bufinfo.buf;
    
    new_font->xfWide = cWidth;
    new_font->xfTall = cHeight;
    new_font->xfData = font_buf;
    new_font->xfCount = cCount;

    if (self->extFont && self->font != NULL) {
        if (self->font->xfData != NULL) {
            free(self->font->xfData);
        }
        free(self->font);
    }
    
    // Set new font
    self->font = new_font;
    self->extFont = true;
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(load_font_obj, 5, loadFont);

// Custom print handler for LCD like the built-in print()
static void lcd_print_strn(void *data, const char *str, size_t len) {
    (void)data;  
    lcd_obj_t *self = &lcd_instance;
    driver_print((const unsigned char*)str, len, &self->col, &self->line, self->color, self->bg, self->scale, self->autoWrap, self->font, self->canvas_buf);
}

static const mp_print_t lcd_printer = {NULL, lcd_print_strn};

static void my_obj_print_helper(const mp_print_t *print, mp_obj_t o_in, mp_print_kind_t kind) {
    mp_cstack_check();

    #ifndef NDEBUG
    if (o_in == MP_OBJ_NULL) {
        mp_print_str(print, "(nil)");
        return;
    }
    #endif

    const mp_obj_type_t *type = mp_obj_get_type(o_in);

    // Check if the object is a string
    if (mp_obj_is_str(o_in)) {
        kind = PRINT_STR;  // Ensure it prints as a plain string
    }

    if (MP_OBJ_TYPE_HAS_SLOT(type, print)) {
        MP_OBJ_TYPE_GET_SLOT(type, print)(print, o_in, kind);
    } else {
        mp_printf(print, "<%q>", type->name);
    }
}
static mp_obj_t lcd_print(size_t n_args, const mp_obj_t *args) {
    lcd_obj_t *self = &lcd_instance;
   
    for (size_t i = 1; i < n_args; i++) {
        my_obj_print_helper(&lcd_printer, args[i], PRINT_REPR);
        // Add space between arguments (except the last one)
        if (i < n_args - 1) {
            driver_print((const unsigned char*)" ", 1, &self->col, &self->line, self->color, self->bg, self->scale, self->autoWrap, self->font, self->canvas_buf);
        }
    }

    // Print newline at the end (if required)
    if(n_args > 2 && self->addLFCR && self->autoWrap)
        driver_print((const unsigned char*)"\n\r", 2, &self->col, &self->line, self->color, self->bg, self->scale, true, self->font, self->canvas_buf);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(lcd_print_obj, 1, MP_OBJ_FUN_ARGS_MAX, lcd_print);


static mp_uint_t lcd_stream_write(mp_obj_t self_in, const void *buf, mp_uint_t size, int *errcode) {
    lcd_obj_t *self = &lcd_instance;
    driver_print((const unsigned char *)buf, size, &self->col, &self->line, self->color, self->bg, self->scale, self->autoWrap, self->font, self->canvas_buf);
    return size; 
}

static mp_uint_t lcd_stream_read(mp_obj_t self_in, void *buf, mp_uint_t size, int *errcode) {
    return 0; 
}

static mp_uint_t lcd_stream_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    return 0; 
}


static mp_obj_t lcd_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    if (!lcd_instance.new) {
        
        driver_init();

        lcd_instance.base.type = type;
        lcd_instance.scale = 1;
        lcd_instance.color = 0xffff;
        lcd_instance.bg = 0;
        lcd_instance.addLFCR = true;
        lcd_instance.autoWrap = true;
        lcd_instance.new = true;
        lcd_instance.font = NULL;

    }
    return (mp_obj_t)&lcd_instance;
}

// static mp_obj_t lcd_deinit(const mp_obj_t self_in){
    
//     return mp_const_none;
// }


static MP_DEFINE_CONST_FUN_OBJ_KW(parse_bmp_obj, 2, parse_bmp);

static const mp_rom_map_elem_t lcd_module_locals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_Terminal) },

    // { MP_ROM_QSTR(MP_QSTR_BLACK), MP_ROM_INT(COL_BLACK) },
    // { MP_ROM_QSTR(MP_QSTR_WHITE), MP_ROM_INT(COL_WHITE) },
    // { MP_ROM_QSTR(MP_QSTR_RED), MP_ROM_INT(COL_RED) },
    // { MP_ROM_QSTR(MP_QSTR_GREEN), MP_ROM_INT(COL_GREEN) },
    // { MP_ROM_QSTR(MP_QSTR_BLUE), MP_ROM_INT(COL_BLUE) },
    // { MP_ROM_QSTR(MP_QSTR_PURPLE), MP_ROM_INT(COL_PURPLE) },
    // { MP_ROM_QSTR(MP_QSTR_YELLOW), MP_ROM_INT(COL_YELLOW) },
    // { MP_ROM_QSTR(MP_QSTR_CYAN), MP_ROM_INT(COL_CYAN) },
    { MP_ROM_QSTR(MP_QSTR_FONT_H), MP_ROM_INT(FONT_TALL) },
    { MP_ROM_QSTR(MP_QSTR_FONT_W), MP_ROM_INT(FONT_WIDE) },
    { MP_ROM_QSTR(MP_QSTR_FONT_NAME), MP_OBJ_NEW_QSTR(FONT_NAME_Q) },
    { MP_ROM_QSTR(MP_QSTR__CHARS_Y), MP_ROM_INT(Y_CHAR) },
    { MP_ROM_QSTR(MP_QSTR__CHARS_X), MP_ROM_INT(X_CHAR) },
    // { MP_ROM_QSTR(MP_QSTR_WIDTH), MP_ROM_INT(X_SIZE) },
    // { MP_ROM_QSTR(MP_QSTR_HEIGHT), MP_ROM_INT(Y_SIZE) },
    
    { MP_ROM_QSTR(MP_QSTR__sendcmd), MP_ROM_PTR(&send_cmd_obj) },
    { MP_ROM_QSTR(MP_QSTR__senddata), MP_ROM_PTR(&send_data_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&clear_obj) },
    { MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&fill_obj) },
    { MP_ROM_QSTR(MP_QSTR_plot), MP_ROM_PTR(&plot_obj) },
    { MP_ROM_QSTR(MP_QSTR_cursor), MP_ROM_PTR(&cursor_obj) },
    { MP_ROM_QSTR(MP_QSTR_print), MP_ROM_PTR(&lcd_print_obj) },
    { MP_ROM_QSTR(MP_QSTR_buffer), MP_ROM_PTR(&buffer_obj) },
    { MP_ROM_QSTR(MP_QSTR__set_canvas), MP_ROM_PTR(&set_canvas_obj) },
    { MP_ROM_QSTR(MP_QSTR__unset_canvas), MP_ROM_PTR(&unset_canvas_obj) },
    { MP_ROM_QSTR(MP_QSTR_options), MP_ROM_PTR(&options_obj)  }, 
    { MP_ROM_QSTR(MP_QSTR_parseBMP), MP_ROM_PTR(&parse_bmp_obj)  }, 
    { MP_ROM_QSTR(MP_QSTR_loadFont), MP_ROM_PTR(&load_font_obj)  }, 
    { MP_ROM_QSTR(MP_QSTR_unloadFont), MP_ROM_PTR(&unload_font_obj)  }, 
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
