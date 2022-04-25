#pragma once
//CMODE:
//numpad -> F1 = normal, F2 = Arrows, F3 = phone ascii
//calculator -> F1 = basic, F2 = sci, F3 = comp
//midi -> F1 = launchpad, F2 = seq usb, F3 = seq uart midi
//comms monitor -> F1 = UART, F2 = SPI, F3 = I2C
//GPIO -> F1 = debounced sw, F2 = clock bits program like cpld, F3 = PWM channels
//PWM audio -> F1 = osc, F2 scope, F3 nodes
//ws28xx ?
#define P_COUNT 4

#define P_NUMPAD 0
void mode_numpad_on_begin();
void mode_numpad_on_end();
int mode_numpad_on_press(int i);
int mode_numpad_on_release(int i);
void mode_numpad_on_gfx();

#define P_CALC 1
void mode_calc_on_begin();
void mode_calc_on_end();
int mode_calc_on_press(int i);
int mode_calc_on_release(int i);
void mode_calc_on_gfx();

#define P_MIDI 2
void mode_midi_on_begin();
void mode_midi_on_end();
int mode_midi_on_press(int i);
int mode_midi_on_release(int i);

#define P_COMMS 3
void mode_comms_on_begin();
void mode_comms_on_end();
int mode_comms_on_press(int i);
int mode_comms_on_release(int i);
void mode_comms_on_loop(unsigned long dt);
void mode_comms_on_gfx();
