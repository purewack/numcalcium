#include "py/dynruntime.h"

// Function to read from the stream and print contents to REPL
static void print_stream_contents(mp_obj_t stream) {
    const mp_stream_p_t *stream_p = mp_get_stream(stream);
    int err;
    byte buf[64];
    mp_uint_t out_sz;
    while ((out_sz = stream_p->read(stream, buf, 4, &err)) > 0) {
        mp_printf(&mp_plat_print, "[data size (%d)]\n", out_sz);
        mp_printf(&mp_plat_print, "%.*s", out_sz, buf);
    }
    if (out_sz == MP_STREAM_ERROR) {
        mp_raise_OSError(err);
    }
}

// Define the 'test' function which takes a file object as an argument
static mp_obj_t my_module_test(mp_obj_t file_obj) {
    print_stream_contents(file_obj);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(my_module_test_obj, my_module_test);

// This is the entry point and is called when the module is imported
mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw, mp_obj_t *args) {
    MP_DYNRUNTIME_INIT_ENTRY

    // Make the function available in the module's namespace
    mp_store_global(MP_QSTR_use, MP_OBJ_FROM_PTR(&my_module_test_obj));

    // This must be last, it restores the globals dict
    MP_DYNRUNTIME_INIT_EXIT
}
