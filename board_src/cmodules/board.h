#ifndef BOARD_H
#define BOARD_H

#include "py/mphal.h"
#include "py/obj.h"

#include "driver/spi_master.h"
#include "driver/sdspi_host.h"

#define BOARD_SPI_SLOT_INTERNAL SPI3_HOST

extern sdspi_dev_handle_t sdspi_handle;
extern spi_device_handle_t lcdspi_handle;


static const spi_bus_config_t spi_bus_defaults = {
    // #if CONFIG_IDF_TARGET_ESP32_S3
    .miso_io_num = GPIO_NUM_41,
    .mosi_io_num = GPIO_NUM_40,
    .sclk_io_num = GPIO_NUM_39,
    // #endif
    .data2_io_num = GPIO_NUM_NC,
    .data3_io_num = GPIO_NUM_NC,
    .data4_io_num = GPIO_NUM_NC,
    .data5_io_num = GPIO_NUM_NC,
    .data6_io_num = GPIO_NUM_NC,
    .data7_io_num = GPIO_NUM_NC,
    .max_transfer_sz = 4000,
    .flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_SCLK | SPICOMMON_BUSFLAG_MISO | SPICOMMON_BUSFLAG_MOSI,
    .intr_flags = 0,
};

static const sdspi_device_config_t sd_dev_defaults = {
    .host_id = BOARD_SPI_SLOT_INTERNAL,
    .gpio_cs = GPIO_NUM_38,
    .gpio_cd = SDSPI_SLOT_NO_CD,
    .gpio_wp = SDSPI_SLOT_NO_WP,
    .gpio_int = SDSPI_SLOT_NO_INT,
};

static const  spi_device_interface_config_t lcd_dev_defaults = {
    .clock_speed_hz = 20000000,
    .mode = 0,
    .spics_io_num = GPIO_NUM_42,
    .queue_size = 7,
};

extern const mp_obj_type_t board_sdcard_type;
extern const mp_obj_type_t lcd_type;

#endif
