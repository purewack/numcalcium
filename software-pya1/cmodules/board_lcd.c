#include "py/runtime.h"
#include "py/objstr.h"
#include "py/obj.h"
#include "py/stream.h"
#include "py/builtin.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "vt100.h"
#include "lcd_bmp.h"
#include "board.h"

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
            xSemaphoreTake(spi_semaphore, portMAX_DELAY);
            driver_send_buffer(buffer_data);
            xSemaphoreGive(spi_semaphore);
            return mp_const_none;
        }
        // No transfer is taking place, queue the buffer data
        xQueueSend(buffer_queue, &buffer_data, portMAX_DELAY);
    } else {
        xSemaphoreTake(spi_semaphore, portMAX_DELAY);
        // Transfer is taking place, block until it's done
        xQueueSend(buffer_queue, &buffer_data, portMAX_DELAY);
        xSemaphoreGive(spi_semaphore);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(buffer_obj, 6,7, buffer);



static mp_obj_t send_cmd(mp_obj_t self_in, mp_obj_t v) {
    xSemaphoreTake(spi_semaphore, portMAX_DELAY);
    const char c = mp_obj_get_int(v);
    driver_send_cmd(c);
    xSemaphoreGive(spi_semaphore);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(send_cmd_obj, send_cmd);

static mp_obj_t send_data(mp_obj_t self_in, mp_obj_t v) {
    xSemaphoreTake(spi_semaphore, portMAX_DELAY);
    const char c = mp_obj_get_int(v);
    driver_send_data(c);
    xSemaphoreGive(spi_semaphore);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(send_data_obj, send_data);

static mp_obj_t reset(mp_obj_t self_in) {
    xSemaphoreTake(spi_semaphore, portMAX_DELAY);
	driver_setup();
    xSemaphoreGive(spi_semaphore);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(reset_obj, reset);

static mp_obj_t clear(mp_obj_t self_in) {
    xSemaphoreTake(spi_semaphore, portMAX_DELAY);
    lcd_obj_t *self = &lcd_instance;
    driver_fill(0,0,X_SIZE,Y_SIZE, self->bg);
    self->col = 0;
    self->line = 0;
    xSemaphoreGive(spi_semaphore);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(clear_obj, clear);

static mp_obj_t fill(size_t n_args, const mp_obj_t *args) {
    xSemaphoreTake(spi_semaphore, portMAX_DELAY);
    driver_fill(mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]),mp_obj_get_int(args[4]),mp_obj_get_int(args[5]));
    xSemaphoreGive(spi_semaphore);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR(fill_obj, 6, fill);

static mp_obj_t plot(size_t n_args, const mp_obj_t *args) {
    xSemaphoreTake(spi_semaphore, portMAX_DELAY);
    driver_pixel(mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]));
    xSemaphoreGive(spi_semaphore);
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

    xSemaphoreTake(spi_semaphore, portMAX_DELAY);
    mp_check_self(mp_obj_is_str_or_bytes(args[1]));
    GET_STR_DATA_LEN(args[1], c_text, c_text_len);
    driver_print(c_text,c_text_len, &self->col, &self->line, self->color, self->bg, self->scale);
    xSemaphoreGive(spi_semaphore);
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
	
    xSemaphoreTake(spi_semaphore, portMAX_DELAY);
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
    xSemaphoreGive(spi_semaphore);

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
    xSemaphoreTake(spi_semaphore, portMAX_DELAY);
    driver_print((const unsigned char *)buf,size, &self->col, &self->line, self->color, self->bg, self->scale);
    xSemaphoreGive(spi_semaphore);
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
        
        // Create the semaphore and buffer queue
        spi_semaphore = xSemaphoreCreateBinary();
        xSemaphoreGive(spi_semaphore);
        buffer_queue = xQueueCreate(1, sizeof(buffer_data_t));

        xSemaphoreTake(spi_semaphore, portMAX_DELAY);
        driver_init();
        xSemaphoreGive(spi_semaphore);

        lcd_instance.base.type = type;
        lcd_instance.scale = 1;
        lcd_instance.color = COL_WHITE;
        lcd_instance.bg = COL_BLACK;
        lcd_instance.new = true;

        // Create the SPI task
        xTaskCreatePinnedToCore(driver_buffer_task, "driver_buffer_task", 4096, NULL, 1, NULL, 1);
    }
    return (mp_obj_t)&lcd_instance;
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
