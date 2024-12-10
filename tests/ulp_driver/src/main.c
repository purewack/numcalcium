#include <stdio.h>
#include "sdkconfig.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#include "esp_sleep.h"

#include "esp32/ulp.h"
#include "ulp_main.h"
#include "driver/rtc_io.h"
#include "soc/soc_ulp.h"   // for WRITE_RTC_REG
#include "soc/rtc_io_reg.h"  // for RTC_GPIO_*


extern const uint8_t _ulp_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t _ulp_end[]   asm("_binary_ulp_main_bin_end");

void ulp_init() {
  ESP_ERROR_CHECK( ulp_load_binary(0, _ulp_start, (_ulp_end-_ulp_start)/sizeof(ulp_entry)) );
}

void ulp_start() {
  // const ulp_insn_t program[] = {
  //   I_MOVI(R0, 32),         // R3 <- 16
  //   I_LD(R1, R0, 0),        // R0 <- RTC_SLOW_MEM[R3 + 0]
  //   I_ADDI(R2, R1, 2),     // R2 <- R0 + R1
  //   I_ST(R2, R0, 0),        // R2 -> RTC_SLOW_MEM[R2 + 2]
  //   I_HALT()
  // };
  // size_t load_addr = 0;
  // size_t size = sizeof(program)/sizeof(ulp_insn_t);
  // ulp_process_macros_and_load(load_addr, program, &size);
  // ulp_run(load_addr);
  ESP_ERROR_CHECK( ulp_run(&ulp_entry-RTC_SLOW_MEM) );
}


void app_main(void) {
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  if (cause != ESP_SLEEP_WAKEUP_ULP) {
    rtc_gpio_init(27); //OE
    rtc_gpio_init(32); //CK
    rtc_gpio_init(33); //D
    rtc_gpio_init(34); //T

    rtc_gpio_init(26); //deb
    rtc_gpio_set_direction(26, RTC_GPIO_MODE_OUTPUT_ONLY);

    rtc_gpio_set_direction(27, RTC_GPIO_MODE_OUTPUT_ONLY);
    rtc_gpio_set_direction(32, RTC_GPIO_MODE_INPUT_OUTPUT);
    rtc_gpio_set_direction(33, RTC_GPIO_MODE_INPUT_OUTPUT);
    rtc_gpio_set_direction(34, RTC_GPIO_MODE_INPUT_ONLY);
    
    rtc_gpio_isolate(12);
    rtc_gpio_isolate(15);
    
    ulp_set_wakeup_period(0, 10000);
    ulp_init();
    ulp_start();
  
    printf("ULP started %d\n", cause);
  }
  
  // printf("Going deep sleep %d\n", cause);
  
  // esp_deep_sleep_disable_rom_logging();
  // esp_sleep_enable_ulp_wakeup(); 
  // esp_deep_sleep_start();
	for(;;){
		vTaskDelay(pdMS_TO_TICKS(50));
		printf("bscan %ld\n",(ulp_b_scan & 0xffff));
	}

}
