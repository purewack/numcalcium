/* ULP-RISC-V example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   This code runs on ULP-RISC-V  coprocessor
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "ulp_riscv.h"
#include "ulp_riscv_gpio.h"
#include "ulp_riscv_utils.h"


static bool gpio_level = false;
int counter = 0;

int main (void)
{   
    counter++;
    ulp_riscv_gpio_init(2);
    ulp_riscv_gpio_output_enable(2);
    for(int i=0; i<64; i++){
        ulp_riscv_gpio_output_level(2,1);
        ulp_riscv_gpio_output_level(2,0);
    }
    
    if(counter == 500){
        counter = 0;
        ulp_riscv_wakeup_main_processor();    
    }

    return 0;
}
