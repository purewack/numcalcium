#pragma once

#include <libmaple/delay.h>
//CMODE:
//numpad -> F1 = normal, F2 = Arrows, F3 = phone ascii
//calculator -> F1 = basic, F2 = sci, F3 = comp
//midi -> F1 = launchpad, F2 = seq usb, F3 = seq uart midi
//comms monitor -> F1 = UART, F2 = SPI, F3 = I2C
//GPIO -> F1 = debounced sw, F2 = clock bits program like cpld, F3 = PWM channels
//PWM audio -> F1 = osc, F2 scope, F3 nodes
//ws28xx ?
#define P_COUNT 7

#define P_NUMPAD 0
void mode_numpad_on_begin();
void mode_numpad_on_end();
void mode_numpad_on_process();

#define P_CALC 1
void mode_calc_on_begin();
void mode_calc_on_end();
void mode_calc_on_process();

#define P_MIDI 2
void mode_midi_on_begin();
void mode_midi_on_end();
void mode_midi_on_process();

#define P_COMMS 3
void mode_comms_on_begin();
void mode_comms_on_end();
void mode_comms_on_process();

#define P_AUDIO 4
void mode_audio_on_begin();
void mode_audio_on_end();
void mode_audio_on_process();

#define P_GPIO 5
void mode_gpio_on_begin();
void mode_gpio_on_end();
void mode_gpio_on_process();

#define P_SCOPE 6
void mode_scope_on_begin();
void mode_scope_on_end();
void mode_scope_on_process();
