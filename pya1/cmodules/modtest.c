#include "py/obj.h"
#include "py/runtime.h"


// This structure represents Timer instance objects.
typedef struct _testclass_obj_t {
    mp_obj_base_t base;
} testclass_obj_t;

static mp_obj_t testclass_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    testclass_obj_t *self = mp_obj_malloc(testclass_obj_t, type);
	
    return MP_OBJ_FROM_PTR(self);
}

static const mp_rom_map_elem_t testclass_locals_dict_table[] = {
};
static MP_DEFINE_CONST_DICT(testclass_locals_dict, testclass_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    testclass_type,
    MP_QSTR_TestClass,
    MP_TYPE_FLAG_NONE,
	make_new, testclass_make_new,
    locals_dict, &testclass_locals_dict
);




// This structure represents Timer instance objects.
typedef struct _testsingle_obj_t {
    mp_obj_base_t base;
    int v;
} testsingle_obj_t;


const mp_obj_type_t testsingle_type;

static testsingle_obj_t *testsingle_instance = NULL;

static mp_obj_t testsingle_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    if (testsingle_instance == NULL) {
        testsingle_instance = m_new_obj(testsingle_obj_t);
        testsingle_instance->base.type = type;
        DEBUG_printf("new\n");
    }
    return MP_OBJ_FROM_PTR(testsingle_instance);
}

static mp_obj_t testsingle_val(mp_obj_t self_in, mp_obj_t v_in) {
    testsingle_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int a = mp_obj_get_int(v_in);
    if(a > 0) self->v = a;
    return mp_obj_new_int(self->v);
}
static MP_DEFINE_CONST_FUN_OBJ_2(testsingle_val_obj, testsingle_val);


static const mp_rom_map_elem_t testsignle_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_val), MP_ROM_PTR(&testsingle_val_obj) },
};
static MP_DEFINE_CONST_DICT(testsignle_locals_dict, testsignle_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    testsingle_type,
    MP_QSTR_TestSingle,
    MP_TYPE_FLAG_NONE,
	make_new, testsingle_make_new,
    locals_dict, &testsignle_locals_dict
);


// A simple example module
static mp_obj_t modtest_hello(void) {
    printf("Hello from modtest!\n");
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(modtest_hello_obj, modtest_hello);

static const mp_rom_map_elem_t modtest_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_test) },
    { MP_ROM_QSTR(MP_QSTR_hello), MP_ROM_PTR(&modtest_hello_obj) },
    { MP_ROM_QSTR(MP_QSTR_TestClass), MP_ROM_PTR(&testclass_type) },
    { MP_ROM_QSTR(MP_QSTR_TestSingle), MP_ROM_PTR(&testsingle_type) },
};

static MP_DEFINE_CONST_DICT(modtest_globals, modtest_globals_table);

const mp_obj_module_t modtest_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&modtest_globals,
};

// Register the module to make it available in Python
MP_REGISTER_MODULE(MP_QSTR_modtest, modtest_user_cmodule);
