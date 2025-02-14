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


static mp_obj_t test(size_t n_args, const mp_obj_t *args) {
    mp_obj_t* callback = args[0];
    
    mp_call_function_n_kw(args[0], 0, 0, NULL);

    int srate = 32000;
    int outputs[2];
    outputs[0] = 20;
    outputs[1] = 21;

    if(n_args  >= 2){
        size_t len_outputs;
        mp_obj_t *items_outputs;
        mp_obj_get_array(args[1], &len_outputs, &items_outputs);
        if(len_outputs > 2){
            mp_raise_ValueError("too many outputs defined");
        }
        if(len_outputs < 1){
            mp_raise_ValueError("not enough outputs defined");
        }
        for(int i=0; i< len_outputs; i++){
            if(items_outputs[i] == mp_const_none)
                outputs[i] = -1;
            else
                outputs[i] = mp_obj_get_int(items_outputs[i]);
        }
    }
    if(n_args == 3){
        srate = mp_obj_get_int(args[2]);
    }

    mp_obj_t current_options = mp_obj_new_dict(0);

    mp_obj_dict_store(current_options, MP_OBJ_NEW_QSTR(MP_QSTR_srate), mp_obj_new_int(srate));
    
    mp_obj_t outputs_obj[2];
    outputs_obj[0] = mp_obj_new_int(outputs[0]);
    outputs_obj[1] = mp_obj_new_int(outputs[1]);
    mp_obj_dict_store(current_options, MP_OBJ_NEW_QSTR(MP_QSTR_outputs), mp_obj_new_tuple(2, outputs_obj));
    return current_options;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(test_obj,1,3,test);

// This is the entry point and is called when the module is imported
mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw, mp_obj_t *args) {
    MP_DYNRUNTIME_INIT_ENTRY

    mp_store_global(MP_QSTR_test, MP_OBJ_FROM_PTR(&test_obj));

    // This must be last, it restores the globals dict
    MP_DYNRUNTIME_INIT_EXIT
}
