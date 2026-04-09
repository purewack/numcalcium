#ifndef LCD_BMP_H
#define LCD_BMP_H

#include "py/runtime.h"
#include "py/objstr.h"
#include "py/obj.h"
#include "py/stream.h"
#include "py/builtin.h"
#include <string.h>

#define BMP_FILE_HEADER_SIZE 14

mp_obj_t parse_bmp(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args);

void integer_scale_plot(int dst_x0, int dst_y0, int src_height, int src_width, int canvas_h, int canvas_w, const uint8_t* src_buf, uint16_t* canvas_buf, int scale);
void float_scale_plot(int dst_x0, int dst_y0, int src_height, int src_width, int dst_height, int dst_width, int canvas_high, int canvas_wide, const uint8_t* src_buf, uint16_t* canvas_buf, float scale, bool dither);
    
#endif //LCD_BMP_H

