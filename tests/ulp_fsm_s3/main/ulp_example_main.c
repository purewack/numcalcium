/* ULP riscv DS18B20 1wire temperature sensor example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <inttypes.h>
#include "esp_sleep.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"
#include "soc/rtc_periph.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "ulp.h"
#include "ulp_main.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");
static void init_ulp_program(void)
{
//    esp_err_t err = ulp_riscv_load_binary(ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start));

    esp_err_t err = ulp_load_binary(0, ulp_main_bin_start,
            (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));
    ESP_ERROR_CHECK(err);

    /* The first argument is the period index, which is not used by the ULP-RISC-V timer
     * The second argument is the period in microseconds, which gives a wakeup time period of: 20ms
     */
    ulp_set_wakeup_period(0, 100000);

    /* Start the program */
    err = ulp_run(&ulp_entry - RTC_SLOW_MEM);
    ESP_ERROR_CHECK(err);
}

void app_main(void)
{
    rtc_gpio_init(2);
    rtc_gpio_set_direction(2, RTC_GPIO_MODE_OUTPUT_ONLY);
//    rtc_gpio_pulldown_dis(2);
//    rtc_gpio_pullup_dis(2);
//    rtc_gpio_hold_en(2);
//    gpio_deep_sleep_hold_en();
    

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    /* not a wakeup from ULP, load the firmware */
    if (cause != ESP_SLEEP_WAKEUP_ULP) {
        printf("Not a ULP wakeup, initializing it! \n");
        init_ulp_program();
    }

    /* ULP Risc-V read and detected a change in GPIO_0, prints */
    if (cause == ESP_SLEEP_WAKEUP_ULP) {
        printf("ULP woke up the main CPU! \n");
//        printf("ULP-RISC-V read changes in GPIO_0 current is: %s \n",
//            (bool)(ulp_gpio_level_previous == 0) ? "Low" : "High" );

    }

    /* Go back to sleep, only the ULP Risc-V will run */
    printf("Entering in deep sleep\n\n");

    /* Small delay to ensure the messages are printed */
    vTaskDelay(100);

//    ESP_ERROR_CHECK( esp_sleep_enable_ulp_wakeup());
//    esp_deep_sleep_start();

    gpio_reset_pin(3);
    gpio_set_direction(3, GPIO_MODE_OUTPUT);
    
    int level = 0;
    while(1){ 
        printf("ULP counter %ld\n",ulp_counter);
        vTaskDelay(100);
        gpio_set_level(3, level);
        level = !level;
    }
}

