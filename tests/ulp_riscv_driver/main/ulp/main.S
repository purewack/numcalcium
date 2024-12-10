#include "sdkconfig.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/soc_ulp.h"
#include "soc/sens_reg.h"

    .bss

    .text

    // Define constants
    .equ GPIO_NUM, 2                // GPIO number to toggle
    .equ TOGGLE_COUNT, 64           // Number of toggles

	.global counter
counter:
	.long 0	

    .global entry                   // Entry point for ULP program
entry:

loop_start:
    // Check if toggle counter reaches TOGGLE_COUNT
    move r2, TOGGLE_COUNT           // Load toggle count into r2
    sub r2, r2, 1                  // Subtract r3 from r2
    jump loop_end, EQ               // If r2 == 0, exit loop

    // Set GPIO high
	WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + GPIO_NUM,1, 1)

    // Set GPIO low
	WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + GPIO_NUM,1, 1)

    // Increment toggle counter
    move    r3, counter   
    ld      r3, r3,0              
    add     r3, r3, 1
    move    r2, counter 
    st      r3, r2, 0

    // Loop back
    jump loop_start

loop_end:
    halt                            // End program

