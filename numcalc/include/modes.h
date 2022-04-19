#ifndef H_MODES
#define H_MODES
//CMODE:
//numpad -> F1 = normal, F2 = Arrows, F3 = phone ascii
//calculator -> F1 = basic, F2 = sci, F3 = comp
//midi -> F1 = launchpad, F2 = seq usb, F3 = seq uart midi
//comms monitor -> F1 = UART, F2 = SPI, F3 = I2C
//GPIO -> F1 = debounced sw, F2 = clock bits program like cpld, F3 = PWM channels
//PWM audio -> F1 = osc, F2 scope, F3 nodes
//ws28xx ?

enum Modes {
    m_numpad, //normally numpad unless shift double pressed 
    m_calc_sci,
    m_calc_comp,
    m_midi,
    m_comms,
    m_gpio,
    m_pwm_audio,
    m_ws28xx
};

void mode_numpad_on_begin();
void mode_numpad_on_end();
void mode_numpad_on_press(int i);
void mode_numpad_on_release(int i);

#endif