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

#define PIN_CK 12
#define PIN_DD 13
#define PIN_LT 11
#define PIN_TT 14

int var_turns;
static unsigned int rscan;

static unsigned int pscan;
static unsigned int pscan_old;
unsigned int var_bscan;
unsigned int var_bdown;
unsigned int var_bup;
unsigned int var_bok;
unsigned int var_system_sleeping = 0;

void shiftOut(unsigned int v){

    ulp_riscv_gpio_input_disable(PIN_CK);
    ulp_riscv_gpio_input_disable(PIN_DD);
    ulp_riscv_gpio_output_enable(PIN_CK);
    ulp_riscv_gpio_output_enable(PIN_DD);

    ulp_riscv_gpio_output_level(PIN_CK, 0);
    ulp_riscv_gpio_output_level(PIN_DD, 0);
    
    ulp_riscv_gpio_output_level(PIN_LT, 0);
    for(int i=0; i<8; i++){
        ulp_riscv_gpio_output_level(PIN_DD, v & (1<<i));
        ulp_riscv_gpio_output_level(PIN_CK,1);
        ulp_riscv_gpio_output_level(PIN_CK,0);
    }
    ulp_riscv_gpio_output_level(PIN_LT, 1);
}

unsigned int readPins(){
    
    ulp_riscv_gpio_output_disable(PIN_CK);
    ulp_riscv_gpio_output_disable(PIN_DD);
    ulp_riscv_gpio_input_enable(PIN_CK);
    ulp_riscv_gpio_input_enable(PIN_DD);

    return (ulp_riscv_gpio_get_level(PIN_TT) << 2) 
        | (ulp_riscv_gpio_get_level(PIN_DD) << 1) 
        | (ulp_riscv_gpio_get_level(PIN_CK) << 0);
}


int main (void)
{   
    var_bok |= !ulp_riscv_gpio_get_level(0);
    if(var_system_sleeping){
        if(var_bok){
            while(!ulp_riscv_gpio_get_level(0)) {}
            ulp_riscv_wakeup_main_processor();
        }
        return 0;
    }

    ulp_riscv_gpio_init(PIN_CK);
    ulp_riscv_gpio_init(PIN_DD);
    ulp_riscv_gpio_init(PIN_LT);
    ulp_riscv_gpio_init(PIN_TT);
    ulp_riscv_gpio_input_enable(PIN_TT);
    ulp_riscv_gpio_pulldown(PIN_TT);
    ulp_riscv_gpio_pulldown(PIN_CK);
    ulp_riscv_gpio_pulldown(PIN_DD);

    ulp_riscv_gpio_output_enable(PIN_LT);
    
    pscan_old = pscan;
    pscan = 0;
    for(int jj=0; jj<7; jj++){
        shiftOut(1<<jj);
        pscan |= (readPins() << (jj * 3));
        
        shiftOut(1<<7);
        rscan = ((rscan<<2) | readPins()) & 0xF;
        if(rscan == 0b1011) {
            if(var_turns < 0) var_turns = 0;
            var_turns++;    
        }
        if(rscan == 0b0111) {
            if(var_turns > 0) var_turns = 0;
            var_turns--;    
        }
    }
    var_bup |= (pscan_old & (~pscan));
    var_bdown |= (pscan & (~pscan_old));   
    var_bscan = pscan; 

    ulp_riscv_gpio_input_disable(PIN_CK);
    ulp_riscv_gpio_input_disable(PIN_DD);
    ulp_riscv_gpio_input_disable(PIN_TT);
    ulp_riscv_gpio_input_disable(PIN_LT);
    ulp_riscv_gpio_output_disable(PIN_CK);
    ulp_riscv_gpio_output_disable(PIN_DD);
    ulp_riscv_gpio_output_disable(PIN_TT);
    ulp_riscv_gpio_output_disable(PIN_LT);
    ulp_riscv_gpio_deinit(PIN_CK);
    ulp_riscv_gpio_deinit(PIN_DD);
    ulp_riscv_gpio_deinit(PIN_LT);
    ulp_riscv_gpio_deinit(PIN_TT);
    
    return 0;
}
