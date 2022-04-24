#pragma once
#include "util.h"

struct vnum_t{
    bool input;
    bool dot;
    double result;
    int mantissa;
    int expo;
    double d_mantissa;
    double d_expo;
    sarray_t<char> rep;
};
extern vnum_t keypad_num;

void startInputNumber();
double getInputNumberResult();
char* getInputNumberRep();
void numberInputKey(int i);
void numberInputBackspace();