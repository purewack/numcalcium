#include "py/obj.h"
#include "py/runtime.h"
#include "ulp_io_numcalcium.h"
#include "ulp_riscv.h"

// A simple example module
static mp_obj_t hwio_downKeys(size_t n_args, const mp_obj_t *args) {
    mp_int_t r = ulp_bdown;
    if(n_args == 1){
        mp_int_t mask = mp_obj_get_int(args[0]);
        ulp_bdown &= (~mask);
    }
    return mp_obj_new_int(r);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(hwio_downKeys_obj, 0, 1, hwio_downKeys);

static const mp_rom_map_elem_t hwio_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_hwio) },
    { MP_ROM_QSTR(MP_QSTR_downKeys), MP_ROM_PTR(&hwio_downKeys_obj) },
};

static MP_DEFINE_CONST_DICT(hwio_globals, hwio_globals_table);

const mp_obj_module_t hwio_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&hwio_globals,
};

// Register the module to make it available in Python
MP_REGISTER_MODULE(MP_QSTR_hwio, hwio_user_cmodule);
