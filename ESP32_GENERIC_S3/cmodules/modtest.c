#include "py/obj.h"
#include "py/runtime.h"

// A simple example module
static mp_obj_t modtest_hello(void) {
    printf("Hello from modtest!\n");
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(modtest_hello_obj, modtest_hello);

static const mp_rom_map_elem_t modtest_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_test) },
    { MP_ROM_QSTR(MP_QSTR_hello), MP_ROM_PTR(&modtest_hello_obj) },
};

static MP_DEFINE_CONST_DICT(modtest_globals, modtest_globals_table);

const mp_obj_module_t modtest_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&modtest_globals,
};

// Register the module to make it available in Python
MP_REGISTER_MODULE(MP_QSTR_modtest, modtest_user_cmodule);
