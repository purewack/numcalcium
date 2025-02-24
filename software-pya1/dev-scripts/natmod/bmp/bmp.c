#include "py/dynruntime.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BMP_HEADER_SIZE 54

// Convert 24-bit BGR to 16-bit RGB565
static inline uint16_t bgr_to_rgb565(uint8_t b, uint8_t g, uint8_t r) {
    uint16_t rgb565 = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
    return ((rgb565 & 0xff)<<8) | (rgb565>>8);
}

static mp_obj_t bmp_load_rgb565(mp_obj_t file_obj, mp_obj_t fb_obj) {
    mp_buffer_info_t fb_buf;
    mp_get_buffer_raise(fb_obj, &fb_buf, MP_BUFFER_WRITE);
    
    if (fb_buf.len < 2) {
        mp_raise_ValueError("Framebuffer too small");
    }
    
    mp_obj_t read_meth = mp_load_attr(file_obj, MP_QSTR_read);
    mp_obj_t seek_meth = mp_load_attr(file_obj, MP_QSTR_seek);
    
    // Seek to beginning and read BMP header
    mp_obj_t seek_args[2] = { MP_OBJ_NEW_SMALL_INT(0), MP_OBJ_NEW_SMALL_INT(0) };
    mp_call_function_n_kw(seek_meth, 2, 0, seek_args);
    
    mp_obj_t header = mp_call_function_n_kw(read_meth, 1, 0, (mp_obj_t[]){ MP_OBJ_NEW_SMALL_INT(BMP_HEADER_SIZE) });
    mp_buffer_info_t header_buf;
    mp_get_buffer_raise(header, &header_buf, MP_BUFFER_READ);
    uint8_t *header_data = (uint8_t *)header_buf.buf;
    
    if (header_buf.len < BMP_HEADER_SIZE || header_data[0] != 'B' || header_data[1] != 'M') {
        mp_raise_ValueError("Invalid BMP file");
    }
    
    int32_t width = *(int32_t*)(header_data + 18);
    int32_t height = *(int32_t*)(header_data + 22);
    uint32_t pixel_offset = *(uint32_t*)(header_data + 10);
    uint16_t bpp = *(uint16_t*)(header_data + 28);
    
    if (bpp != 8 && bpp != 24) {
        mp_raise_ValueError("Only 8-bit and 24-bit BMP supported");
    }
    
    int row_size = ((width * (bpp / 8) + 3) & ~3);
    int abs_height = height < 0 ? -height : height;
    int flipped = height > 0;
    
    uint16_t *fb_pixels = (uint16_t *)fb_buf.buf;
    
    if (bpp == 8) {
        mp_obj_t seek_palette_args[2] = { MP_OBJ_NEW_SMALL_INT(54), MP_OBJ_NEW_SMALL_INT(0) };
        mp_call_function_n_kw(seek_meth, 2, 0, seek_palette_args);
        
        mp_obj_t palette_data = mp_call_function_n_kw(read_meth, 1, 0, (mp_obj_t[]){ MP_OBJ_NEW_SMALL_INT(1024) });
        mp_buffer_info_t palette_buf;
        mp_get_buffer_raise(palette_data, &palette_buf, MP_BUFFER_READ);
        uint8_t *palette = (uint8_t *)palette_buf.buf;
        
        mp_obj_t seek_pixel_args[2] = { MP_OBJ_NEW_SMALL_INT(pixel_offset), MP_OBJ_NEW_SMALL_INT(0) };
        mp_call_function_n_kw(seek_meth, 2, 0, seek_pixel_args);
        
        for (int y = 0; y < abs_height; y++) {
            mp_obj_t row_data = mp_call_function_n_kw(read_meth, 1, 0, (mp_obj_t[]){ MP_OBJ_NEW_SMALL_INT(row_size) });
            mp_buffer_info_t row_info;
            mp_get_buffer_raise(row_data, &row_info, MP_BUFFER_READ);
            uint8_t *row_buf = (uint8_t *)row_info.buf;
            
            if (row_info.len < row_size) {
                mp_raise_ValueError("Unexpected end of file");
            }
            
            int dst_y = flipped ? (abs_height - 1 - y) : y;
            for (int x = 0; x < width; x++) {
                uint8_t index = row_buf[x];
                uint8_t b = palette[index * 4 + 0];
                uint8_t g = palette[index * 4 + 1];
                uint8_t r = palette[index * 4 + 2];
                fb_pixels[dst_y * width + x] = bgr_to_rgb565(b, g, r);
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
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(bmp_load_rgb565_obj, bmp_load_rgb565);

// This is the entry point and is called when the module is imported
mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw, mp_obj_t *args) {
    MP_DYNRUNTIME_INIT_ENTRY

    mp_store_global(MP_QSTR_load_rgb565, MP_OBJ_FROM_PTR(&bmp_load_rgb565_obj));
    // This must be last, it restores the globals dict
    MP_DYNRUNTIME_INIT_EXIT
}

