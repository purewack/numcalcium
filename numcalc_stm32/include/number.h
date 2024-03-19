#pragma once
#include "util.h"

struct vnum_t{
    uint16_t m_dc;
    uint16_t e_dc;
    uint64_t e_int;
    uint64_t m_int;
    bool dot;
    double result;
};

void clearNumber(vnum_t &n);
double doubleFromNumber(vnum_t &n);
vnum_t numberFromDouble(double dnum);
void numberInputKey(vnum_t& n, uint32_t i, int pos);
bool numberInputBackspace(vnum_t &n, int pos);
void printNumber(vnum_t& n, int &x, const int y);
int numberLength(vnum_t& n);