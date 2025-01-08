#include "py/builtin.h"
#include "py/runtime.h"
#include "py/obj.h"

#include "driver/spi_master.h"
#include "soc/gpio_sig_map.h"
#include "soc/spi_pins.h"

#include "board.h"

// // A simple example module
// static mp_obj_t modboard_sd_info(const mp_obj_t in) {
//     machine_hw_spi_obj_t *spi = MP_OBJ_TO_PTR(in);
//     DEBUG_printf("SPI %d %d %d\n",spi->mosi,spi->miso,spi->sck);
//     return mp_const_none;
// }
// static MP_DEFINE_CONST_FUN_OBJ_1(modboard_sd_info_obj, modboard_sd_info);

static const mp_rom_map_elem_t board_module_globals_table[] = {
    // { MP_ROM_QSTR(MP_QSTR_spi_info), MP_ROM_PTR(&modboard_sd_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_SD), MP_ROM_PTR(&board_sdcard_type) },
};
static MP_DEFINE_CONST_DICT(board_module_globals, board_module_globals_table);


const mp_obj_module_t mp_module_board = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&board_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_board, mp_module_board);
