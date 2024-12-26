/* ULP riscv DS18B20 1wire temperature sensor example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include "esp_sleep.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"
#include "soc/rtc_periph.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "ulp_riscv.h"
#include "ulp_main.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");
static void init_ulp_program(void)
{
    esp_err_t err = ulp_riscv_load_binary(ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start));
    ESP_ERROR_CHECK(err);

    ulp_set_wakeup_period(0, 1000);

    err = ulp_riscv_run();
    ESP_ERROR_CHECK(err);
}

void app_main(void)
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    if (cause != ESP_SLEEP_WAKEUP_ULP) {
        init_ulp_program();
        rtc_gpio_init(16);
        rtc_gpio_set_direction(16,RTC_GPIO_MODE_OUTPUT_OD);
        ulp_set_wakeup_period(0, 1000);
    }

    if (cause == ESP_SLEEP_WAKEUP_ULP) {
        ulp_set_wakeup_period(0, 1000);
        
    }
    
    rtc_gpio_hold_dis(16);
    rtc_gpio_set_level(16,1);

    vTaskDelay(100);
    printf("HELLO! %d\n",cause);
    vTaskDelay(100);

    while(1){ 
        printf("ULP io: %ld[%ld]{%ld} (%ld)\n", ulp_bscan, ulp_bdown, ulp_bup, ulp_turns);
        vTaskDelay(10);
        if(ulp_bdown & 4){
            ulp_set_wakeup_period(0, 100000);
            rtc_gpio_set_level(16,0);
            rtc_gpio_hold_en(16);
            ESP_ERROR_CHECK( esp_sleep_enable_ulp_wakeup());
            esp_deep_sleep_start();
        }
    }
}

