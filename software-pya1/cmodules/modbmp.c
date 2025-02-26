#include "py/runtime.h"
#include "py/obj.h"
#include "py/objstr.h"
#include "py/binary.h"
#include "py/objarray.h"
#include "extmod/vfs.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h> // Add this at the top

#define BMP_HEADER_SIZE 54
#define BMP_PALETTE_SIZE 1024 // 256 colors * 4 bytes (BGRA)

// Convert 8-bit palette entry (BGR) to RGB565
static inline uint16_t bgr_to_rgb565(uint8_t b, uint8_t g, uint8_t r) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static mp_obj_t bmp_to_rgb565(mp_obj_t path_obj) {
    const char *path = mp_obj_str_get_str(path_obj);
    FILE *f = fopen(path, "rb");
    if (!f) {
        mp_raise_OSError(ENOENT);
    }

    uint8_t header[BMP_HEADER_SIZE];
    fread(header, 1, BMP_HEADER_SIZE, f);
    
    uint32_t width = *(uint32_t *)&header[18];
    int32_t height = *(int32_t *)&header[22];
    uint16_t bit_depth = *(uint16_t *)&header[28];
    uint32_t data_offset = *(uint32_t *)&header[10];

    bool top_down = height < 0;
    if (top_down) height = -height;

    size_t img_size = width * height * 2; // RGB565 (2 bytes per pixel)
    mp_obj_array_t *bytearray = m_new_obj(mp_obj_array_t);
    bytearray->base.type = &mp_type_bytearray;
    bytearray->typecode = 'B';
    bytearray->len = img_size;
    bytearray->items = m_new(uint8_t, img_size);

    fseek(f, data_offset, SEEK_SET);

    if (bit_depth == 8) {
        uint8_t palette[BMP_PALETTE_SIZE];
        fread(palette, 1, BMP_PALETTE_SIZE, f);
        uint8_t *buffer = m_new(uint8_t, width);

        for (int y = 0; y < height; y++) {
            int row = top_down ? y : (height - 1 - y);
            fread(buffer, 1, width, f);
            for (int x = 0; x < width; x++) {
                uint8_t index = buffer[x] * 4;
                uint16_t rgb565 = bgr_to_rgb565(palette[index], palette[index + 1], palette[index + 2]);
                ((uint16_t *)bytearray->items)[row * width + x] = rgb565;
            }
        }
        m_del(uint8_t, buffer, width);
    }
    else if (bit_depth == 24) {
        uint8_t *buffer = m_new(uint8_t, width * 3);
        for (int y = 0; y < height; y++) {
            int row = top_down ? y : (height - 1 - y);
            fread(buffer, 1, width * 3, f);
            for (int x = 0; x < width; x++) {
                uint8_t b = buffer[x * 3];
                uint8_t g = buffer[x * 3 + 1];
                uint8_t r = buffer[x * 3 + 2];
                ((uint16_t *)bytearray->items)[row * width + x] = bgr_to_rgb565(b, g, r);
            }
        }
        m_del(uint8_t, buffer, width * 3);
    } else {
        fclose(f);
        mp_raise_ValueError("Unsupported BMP format");
    }

    fclose(f);
    return MP_OBJ_FROM_PTR(bytearray);
}

static MP_DEFINE_CONST_FUN_OBJ_1(bmp_to_rgb565_obj, bmp_to_rgb565);

static const mp_rom_map_elem_t bmp_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR_bmp_to_rgb565), MP_ROM_PTR(&bmp_to_rgb565_obj) },
};

static MP_DEFINE_CONST_DICT(bmp_module_globals, bmp_module_globals_table);

const mp_obj_module_t bmp_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&bmp_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_modbmp, bmp_module);

