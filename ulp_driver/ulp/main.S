/*
System IO driver, runs all the time

Pseudo code:

int b_scan;
int pwm_ratio;

void main(){
	int p_scan = 0;
	int scan_val = 1;
	int pwm_time = 0;

	while(1){
		if(isMainCpuSleeping()){
			if(readWakeButton())
				wakeUp()
			continue;
		}

		//else

		setPinsAsOutputs();
		shiftOut(scan_val)
		setPinsAsInputs();
		p_scan |= (readPinValues() << (scan_val * 3))

		scan_val = (scan_val<<1) & 0xff;

		if(scan_val == 0){
			scan_val = 1;
			b_scan = p_scan;
			p_scan = 0;
		}

		if(pwm_time & 0xff === 0){
			pwm_time |= 255;
			pwmPinFlip()
		}
	}
}

input banking:
each bank has 3 bits
7 banks for 21 total inputs

0b gg fff eee ddd ccc bbb aaa
LSB = button 1
MSB = button N-1
*/


#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/soc_ulp.h"

    .bss

    .text

	.equ pin_CK	, 	9
	.equ pin_D 	,	8
	.equ pin_T 	,	4
	.equ pin_OE	, 	17


/* public vars */
	.global b_scan
b_scan:
	.long 0

	.global b_up
b_up:
	.long 0	

	.global b_down
b_down:
	.long 0

/* private vars */
shift_val:
	.long 0
p_scan:
	.long 0

/* void main() */
    .global entry
entry:

//setPinsAsOutputs()
	WRITE_RTC_REG(RTC_GPIO_ENABLE_W1TS_REG, RTC_GPIO_ENABLE_W1TS_S + pin_CK, 1, 1)
	WRITE_RTC_REG(RTC_GPIO_ENABLE_W1TS_REG, RTC_GPIO_ENABLE_W1TS_S + pin_D , 1, 1)

// set OE low
	WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + pin_OE,1, 1)


//load shift_val address for shifting out
	move	R0, shift_val 		
	ld		R1, R0, 0		//R1 = shift_val 
// outShiftVal() 
	jump 	out_shift_val
out_shift_val_ret:


// set OE high
	WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + pin_OE,1, 1)


// setPinsAsInputs()
	WRITE_RTC_REG(RTC_GPIO_ENABLE_W1TC_REG, RTC_GPIO_ENABLE_W1TC_S + pin_CK, 1, 1)
	WRITE_RTC_REG(RTC_GPIO_ENABLE_W1TC_REG, RTC_GPIO_ENABLE_W1TC_S + pin_D , 1, 1)
	

// readPinValues()
	READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S, 17);
	//R0 = io

	move	R3, 0	   	    // R3 = 0
		
		//bank bit 0
		move	R1, 1			// R1 = 1
		lsh		R2, R1, pin_CK 	// R2 = R1 << pin_CK === R2 = (1<<pin_CK)
		and		R1, R0, R2	    // R1 = R0 & R2      === io &= (1<<pin_CK)
		//      									 : R1 = (io & (1<<pin_ck))
		move	R2, pin_CK	   	// R2 = pin_CK
		rsh		R1, R1, R2     	// R1 =>> R2 		 === R1 = R1 >> (pin_CK)
		//											 : R1 = (io & (1<<pin_ck)) >> (pin_CK)
		or     R3, R3, R1		// R3 |= R1		     === R3 |= (io & (1<<pin_ck)) >> (pin_CK)
		
		//bank bit 1
		move	R1, 1			
		lsh		R2, R1, pin_D 	
		and		R1, R0, R2	    
		//
		move	R2, pin_D	   	
		rsh		R1, R1, R2     	
		lsh		R1, R1, 1		//shift to second bank bit position
		//
		or     R3, R3, R1	
		
		//bank bit 2
		move	R1, 1			
		lsh		R2, R1, pin_T 	
		and		R1, R0, R2	    
		//
		move	R2, pin_T	   
		rsh		R1, R1, R2     	
		lsh		R1, R1, 2		//shift to third bank bit position
		//
		or     R3, R3, R1	
	
	//=== R3 = ((pin_T<<2) | (pin_D<<1) | pin_CK)

	//R1 = (scan_val-1) * 3
	move	R1, shift_val 
	ld		R0, R1, 0
	sub		R0, R0, 1
	add		R1, R0, R0		
	add     R1, R1, R0
	
	//R3 =<< R1 === ((scan_val-1) * 3)
	lsh		R3,	R3, R1 

	//p_scan |= R3
	move    R1, p_scan
	ld		R0, R1, 0
	or		R3, R3, R0	//R3 |= p_scan
	st		R3, R1, 0	//p_scan = R3

// shift_val = (shift_val<<1) % 0xff 
	move	R1, shift_val 
	ld		R0, R1, 0 		//R0 = shift_val
	lsh		R0, R0, 1 		//R0 =<< 1
	and		R0, R0, 0xff	//R0 &= 0xff
	
	// if(R0 > 0) goto save_shiftval
	jumpr	save_shiftval, 0, GT	
save_shiftval_else:

	//b_scan = R3 (p_scan)
	move	R1, b_scan 
	st		R3, R1, 0 

	//p_scan = 0
	move	R1, p_scan 
	move	R3, 0
	st		R3, R1, 0 

	// shift_val = 1
	move	R1, shift_val 
	ld		R0, R1, 0
	move	R0, 1

save_shiftval:
	st		R0, R1, 0


// return 
    halt

// void outShiftVal(val): shift out value of R1 on pins 
out_shift_val:
	stage_rst


out_shift_val_loop:
	// if(stage <= 8) start loop 
	jumps	out_shift_bit_begin, 8, LT
	// else 
	// End of loop 
	jump	out_shift_val_ret

out_shift_bit_begin:
	and 	R0, R1, 1		// R0 = R1 & 0x1 

	// if(R0 > 0) goto set_d_high 
	jumpr	set_d_high, 1, EQ
	// else 
else_set_d_low:
	WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + pin_D,1, 1)
	jump 	out_shift_bit_end
set_d_high:
	WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + pin_D,1, 1)

out_shift_bit_end:
	rsh		R1, R1, 1		// R1 = R1 >> 1
	
// set CK low
//	WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + pin_CK,1, 1)
// set CK high
	WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + pin_CK,1, 1)
// set CK low
	WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + pin_CK,1, 1)

	stage_inc 1
	jump	out_shift_val_loop

