#pragma once
#include "util.h"

struct vnum_t{
    int8_t m_dc;
    int8_t e_dc;
    uint32_t e_int;
    uint32_t m_int;
    double result;
};

extern vnum_t keypad_num;
extern bool keypad_num_inputting;
extern bool keypad_num_dot;

void beginNumberInput(vnum_t &n);
void endNumberInput(vnum_t &n);
void numberInputKey(vnum_t& n, uint32_t i);
bool numberInputBackspace(vnum_t &n);
void print_vnum(vnum_t& n, int& x, const int y);