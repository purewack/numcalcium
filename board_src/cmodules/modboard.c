#include "py/builtin.h"
#include "py/runtime.h"
#include "py/obj.h"

#include "driver/spi_master.h"
#include "soc/gpio_sig_map.h"
#include "soc/spi_pins.h"

#include "board.h"

sdspi_dev_handle_t sdspi_handle;
spi_device_handle_t lcdspi_handle;

// // A simple example module
// static mp_obj_t modboard_sd_info(const mp_obj_t in) {
//     machine_hw_spi_obj_t *spi = MP_OBJ_TO_PTR(in);
//     DEBUG_printf("SPI %d %d %d\n",spi->mosi,spi->miso,spi->sck);
//     return mp_const_none;
// }
// static MP_DEFINE_CONST_FUN_OBJ_1(modboard_sd_info_obj, modboard_sd_info);

static mp_obj_t modboard_init(){

    esp_err_t bus = spi_bus_initialize(BOARD_SPI_SLOT_INTERNAL, &spi_bus_defaults, SPI_DMA_CH_AUTO);
    DEBUG_printf("bus %d\n",bus);

    if(bus == ESP_ERR_INVALID_STATE) return mp_const_none;
    else if(bus != 0) {
        check_esp_err(bus);
    }

    esp_err_t lcd_err = spi_bus_add_device(BOARD_SPI_SLOT_INTERNAL, &lcd_dev_defaults, &lcdspi_handle);
    DEBUG_printf("lcd-bus %d\n",lcd_err);

    sdspi_device_config_t dev_config = sd_dev_defaults;
    esp_err_t sd_err = sdspi_host_init_device(&dev_config, &sdspi_handle);
    DEBUG_printf("sd-bus %d\n",sd_err);
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(modboard_init_obj, modboard_init);

static const mp_rom_map_elem_t board_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&modboard_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_SD), MP_ROM_PTR(&board_sdcard_type) },
    { MP_ROM_QSTR(MP_QSTR_Terminal), MP_ROM_PTR(&lcd_type) },
    { MP_ROM_QSTR(MP_QSTR_DAC), MP_ROM_PTR(&board_dac_type) },
};
static MP_DEFINE_CONST_DICT(board_module_globals, board_module_globals_table);


const mp_obj_module_t mp_module_board = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&board_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_board, mp_module_board);
