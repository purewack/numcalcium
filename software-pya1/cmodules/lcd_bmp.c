#include "lcd_bmp.h"

// Convert 24-bit BGR to 16-bit RGB565
static inline uint16_t bgr_to_rgb565(uint8_t b, uint8_t g, uint8_t r) {
    uint16_t rgb565 = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
    return ((rgb565 & 0xff)<<8) | (rgb565>>8);
}

// In-place nearest neighbor scaling
static void scale_bmp_in_place(uint16_t *fb_pixels, int width, int height, int scale) {
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

mp_obj_t parse_bmp(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
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
        mp_raise_ValueError(MP_ERROR_TEXT("Scale must be >= 1"));
    }

    mp_buffer_info_t fb_buf;
    uint16_t *fb_pixels = NULL;
    if (fb_obj != MP_OBJ_NULL) {
        mp_get_buffer_raise(fb_obj, &fb_buf, MP_BUFFER_WRITE);
        if (fb_buf.len < 2) {
            mp_raise_ValueError(MP_ERROR_TEXT("Framebuffer too small"));
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
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid BMP file"));
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
        mp_raise_ValueError(MP_ERROR_TEXT("Only 1-bit, 4-bit, 8-bit, 16-bit, and 24-bit BMP supported"));
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
                mp_raise_ValueError(MP_ERROR_TEXT("Failed to read row data"));
            }
            
            if (row_info.len < row_size) {
                mp_raise_ValueError(MP_ERROR_TEXT("Unexpected end of file"));
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
                        mp_raise_ValueError(MP_ERROR_TEXT("Unexpected end of row data"));
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
                mp_raise_ValueError(MP_ERROR_TEXT("Failed to read row data"));
            }
            
            if (row_info.len < row_size) {
                mp_raise_ValueError(MP_ERROR_TEXT("Unexpected end of file"));
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
                mp_raise_ValueError(MP_ERROR_TEXT("Failed to read row data"));
            }
            
            if (row_info.len < row_size) {
                mp_raise_ValueError(MP_ERROR_TEXT("Unexpected end of file"));
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
            mp_raise_ValueError(MP_ERROR_TEXT("Framebuffer too small for scaled image"));
        }
        
        scale_bmp_in_place(fb_pixels, width, abs_height, scale);
    }
    
    if (fb_obj == MP_OBJ_NULL) {
        return img_dict;
    }

    return mp_const_none;
}
