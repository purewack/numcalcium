#pragma once
#include "util.h"

struct vnum_t{
    uint32_t m_dc;
    uint32_t e_dc;
    uint32_t e_int;
    uint32_t m_int;
    bool dot;
    double result;
};

double computeNumber(vnum_t &n);
void clearNumber(vnum_t &n);
void numberInputKey(vnum_t& n, uint32_t i, int pos);
bool numberInputBackspace(vnum_t &n, int pos);
void printNumber(vnum_t& n, int &x, const int y);
int numberLength(vnum_t& n);