/* This example demonstrates the following features in a native module:
    - defining simple functions exposed to Python
    - defining local, helper C functions
    - defining constant integers and strings exposed to Python
    - getting and creating integer objects
    - creating Python lists
    - raising exceptions
    - allocating memory
    - BSS and constant data (rodata)
    - relocated pointers in rodata
*/

// Include the header file to get access to the MicroPython API
#include "py/dynruntime.h"
#include <stdint.h>

#define LUT_COUNT 256
const int16_t sint[256] = {
0,804,1607,2410,3211,4010,4807,5601,6392,7179,7961,8739,9511,10278,11038,11792,12539,13278,14009,14731,15445,16150,16845,17529,18203,18866,19518,20158,20786,21401,22004,22593,23169,23730,24278,24810,25328,25830,26317,26788,27243,27682,28104,28509,28897,29267,29620,29954,30271,30570,30850,31112,31355,31579,31784,31969,32136,32283,32411,32519,32608,32677,32726,32756,32766,32756,32726,32677,32608,32519,32411,32283,32136,31969,31784,31579,31355,31112,30850,30570,30271,29954,29620,29267,28897,28509,28104,27682,27243,26788,26317,25830,25328,24810,24278,23730,23169,22593,22004,21401,20786,20158,19518,18866,18203,17529,16845,16150,15445,14731,14009,13278,12539,11792,11038,10278,9511,8739,7961,7179,6392,5601,4807,4010,3211,2410,1607,804,0,-804,-1607,-2410,-3211,-4010,-4807,-5601,-6392,-7179,-7961,-8739,-9511,-10278,-11038,-11792,-12539,-13278,-14009,-14731,-15445,-16150,-16845,-17529,-18203,-18866,-19518,-20158,-20786,-21401,-22004,-22593,-23169,-23730,-24278,-24810,-25328,-25830,-26317,-26788,-27243,-27682,-28104,-28509,-28897,-29267,-29620,-29954,-30271,-30570,-30850,-31112,-31355,-31579,-31784,-31969,-32136,-32283,-32411,-32519,-32608,-32677,-32726,-32756,-32766,-32756,-32726,-32677,-32608,-32519,-32411,-32283,-32136,-31969,-31784,-31579,-31355,-31112,-30850,-30570,-30271,-29954,-29620,-29267,-28897,-28509,-28104,-27682,-27243,-26788,-26317,-25830,-25328,-24810,-24278,-23730,-23169,-22593,-22004,-21401,-20786,-20158,-19518,-18866,-18203,-17529,-16845,-16150,-15445,-14731,-14009,-13278,-12539,-11792,-11038,-10278,-9511,-8739,-7961,-7179,-6392,-5601,-4807,-4010,-3211,-2410,-1607,-804
};

int32_t gain,bias,out_spl;
uint32_t acc,phi;
int8_t *buf;
uint16_t buf_len;

void proc_osc(){
//  int16_t phi_dt, phi_l, phi_h;
//  int16_t s_l;
//  int16_t s_l, s_h;
//  int16_t intp;


//  int phi_dt = phi & 0xFF;
//  int phi_l = phi >> 8;
//  phi_h = (phi_l + 1);
//  phi_h = phi_h & 0xFF;

//  s_l = sint[phi_l];
//  s_h = sint[phi_h];
  
//  intp = lin_interp32(s_l,s_h,phi_dt,8);
//  out_spl = bias + (((intp)*gain)>>8);
  out_spl = sint[(phi>>8) & 0xff];

  phi += acc;
}

void set_osc_freq(uint32_t f_big, uint32_t srate){
  //256 = fixed int resolution for multiplication
  int32_t a = LUT_COUNT*256*f_big;
  acc = a / srate / 10;
}

// A function which computes Fibonacci numbers
static mp_obj_t process(mp_obj_t half_in) {
   
    int half = mp_obj_get_int(half_in);
    int s = 0;
    int e = buf_len>>1;
    if(half){
        s = e;
        e <<= 1;    
    }
    for(int i=s; i<e; i++){
        proc_osc();
        buf[i] = (out_spl>>9);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(process_obj, process);


// A function which computes Fibonacci numbers
static mp_obj_t buffer(mp_obj_t buffer_in) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buffer_in, &bufinfo, MP_BUFFER_RW);
    
    buf_len = bufinfo.len;
    buf = (int8_t*)bufinfo.buf;

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(buffer_obj, buffer);

static mp_obj_t set_freq(mp_obj_t f) {
    set_osc_freq(mp_obj_get_int(f) * 10, 32000);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(set_freq_obj, set_freq);


// This is the entry point and is called when the module is imported
mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw, mp_obj_t *args) {
    MP_DYNRUNTIME_INIT_ENTRY

    bias = 0;
    acc = 900;
    gain = 255;

    mp_store_global(MP_QSTR_buffer, MP_OBJ_FROM_PTR(&buffer_obj));
    mp_store_global(MP_QSTR_process, MP_OBJ_FROM_PTR(&process_obj));
    mp_store_global(MP_QSTR_freq, MP_OBJ_FROM_PTR(&set_freq_obj));

    // This must be last, it restores the globals dict
    MP_DYNRUNTIME_INIT_EXIT
}
