#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"

void mode_scope_on_begin(){
    lcd_clear();
    drawTitle();
    lcd_drawString(0,32,sys_font,"Hello");
    lcd_update();

    LOGL("adc init");
    adc_init(ADC1);
    ADC1->regs->CR2 |= ADC_CR2_TSVREFE;
    adc_set_extsel(ADC1,ADC_SWSTART);
    adc_set_sample_rate(ADC1, ADC_SMPR_7_5);
    adc_set_reg_seqlen(ADC1, 1);
    adc_enable(ADC1);
    adc_calibrate(ADC1);
    LOGL("on start scope fin ");
}

void mode_scope_on_end(){
    LOGL("on end scope");
}

void mode_scope_on_process(){
	LOGL("onproc");
	clearProgGFX();
	auto a = adc_read(ADC1,17);
	char str[32];
	double vv = 1.25*4096.0 / double(a);
	snprintf(str,32,"read:%d, mV[%d]",a,int(vv*1000.0));
	lcd_drawString(0,16,sys_font,str);
	updateProgGFX();
	delay_us(100000);
	LOGL("onproc end");
}
