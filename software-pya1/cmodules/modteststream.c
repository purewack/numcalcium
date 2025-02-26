// Include MicroPython API.
#include "py/runtime.h"
#include "py/stream.h"

// This function will read from the stream.
static int my_read_stream(void *data) {
    mp_obj_t stream = data;
    const mp_stream_p_t *stream_p = mp_get_stream(stream);
    int err;
    byte c;
    mp_uint_t out_sz = stream_p->read(stream, &c, 1, &err);
    if (out_sz == MP_STREAM_ERROR) {
        mp_raise_OSError(err);
    }
    if (out_sz == 0) {
        mp_raise_type(&mp_type_EOFError);
    }
    return c;
}

// Initialize the stream for reading.
static bool my_init_read(mp_obj_t stream) {
    mp_get_stream_raise(stream, MP_STREAM_OP_READ);
    // Additional initialization code if needed
    return true;
}

// Define a structure to hold the module's state, including the stream object.
typedef struct _my_obj_t {
    mp_obj_base_t base;
    mp_obj_t stream;
} my_obj_t;

// Define the make_new function for the module.
static mp_obj_t my_module_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, false);
    my_obj_t *self = mp_obj_malloc(my_obj_t, type);
    self->stream = args[0];
    return MP_OBJ_FROM_PTR(self);
}

// Define the module's methods.
static mp_uint_t my_module_read(mp_obj_t o_in, void *buf, mp_uint_t size, int *errcode) {
    my_obj_t *self = MP_OBJ_TO_PTR(o_in);
    if (!my_init_read(self->stream)) {
        *errcode = MP_EINVAL;
        return MP_STREAM_ERROR;
    }
    const mp_stream_p_t *stream_p = mp_get_stream(self->stream);
    return stream_p->read(self->stream, buf, size, errcode);
}

// Define the stream protocol.
static const mp_stream_p_t my_stream_p = {
    .read = my_module_read,
    .write = NULL,  // Define write function if needed
    .ioctl = NULL,  // Define ioctl function if needed
    .is_text = false,
};

// Define the module type.
static const mp_rom_map_elem_t my_module_locals_dict_table[] = {
    // Add module methods here
};
static MP_DEFINE_CONST_DICT(my_module_locals_dict, my_module_locals_dict_table);

const mp_obj_type_t my_module_type = {
    { &mp_type_type },
    .name = MP_QSTR_MyModule,
    .make_new = my_module_make_new,
    .protocol = &my_stream_p,
    .locals_dict = (mp_obj_dict_t*)&my_module_locals_dict,
};

// Define all attributes of the module.
static const mp_rom_map_elem_t my_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_mymodule) },
    { MP_ROM_QSTR(MP_QSTR_MyModule), MP_ROM_PTR(&my_module_type) },
};
static MP_DEFINE_CONST_DICT(my_module_globals, my_module_globals_table);

// Define module object.
const mp_obj_module_t my_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&my_module_globals,
};

// Register the module to make it available in Python.
MP_REGISTER_MODULE(MP_QSTR_mymodule, my_user_cmodule);
