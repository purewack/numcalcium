#include <py/mpconfig.h>
#include "py/runtime.h"
#include "py/mphal.h"

#include "driver/spi_master.h"
#include "driver/sdspi_host.h"

#include "cmodules/board.h"
#include "cmodules/modvt100.h"

sdspi_dev_handle_t sdspi_handle;
spi_device_handle_t lcdspi_handle;


void NUMCALCIUM_board_init(){
    check_esp_err(spi_bus_initialize(BOARD_SPI_SLOT_INTERNAL, &spi_bus_defaults, SPI_DMA_CH_AUTO));

    spi_bus_add_device(BOARD_SPI_SLOT_INTERNAL, &lcd_dev_defaults, &lcdspi_handle);
    lcd_init();

    // DEBUG_printf("  Setting up SPI slot configuration");
    sdspi_device_config_t dev_config = sd_dev_defaults;
    // DEBUG_printf("  Calling sdspi_host_init_device()");
    esp_err_t ret = sdspi_host_init_device(&dev_config, &sdspi_handle);
    if (ret != ESP_OK) {
        // spi_bus_free(spi_host_id);
        return;
        check_esp_err(ret);
    }
}
