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

#endif //LCD_BMP_H

