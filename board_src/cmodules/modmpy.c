//#include "py/compile.h"
//#include "py/runtime.h"
//#include "py/stream.h"
//#include "py/objstr.h"
//#include "py/persistentcode.h"
//#include "extmod/vfs.h"
//
//static mp_obj_t execute_mpy(mp_obj_t buffer_in) {
//    mp_buffer_info_t bufinfo;
//    mp_get_buffer_raise(buffer_in, &bufinfo, MP_BUFFER_READ);
//
//    nlr_buf_t nlr;
//    if (nlr_push(&nlr) == 0) {
//        // Execute the given .mpy data.
//        mp_module_context_t *ctx = m_new_obj(mp_module_context_t);
//        ctx->module.globals = mp_globals_get();
//        mp_compiled_module_t cm;
//        cm.context = ctx;
//        mp_raw_code_load_mem(bufinfo.buf, bufinfo.len, &cm);
//        mp_obj_t f = mp_make_function_from_proto_fun(cm.rc, ctx, MP_OBJ_NULL);
//        mp_call_function_0(f);
//        nlr_pop();
//    } else {
//        // Uncaught exception: print it out.
//        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
//    }
//
//    return mp_const_none;
//}
//
//static MP_DEFINE_CONST_FUN_OBJ_1(execute_mpy_obj, execute_mpy);
//
//static const mp_rom_map_elem_t executor_module_globals_table[] = {
//    { MP_ROM_QSTR(MP_QSTR_exec_buffer), MP_ROM_PTR(&execute_mpy_obj) },
//};
//
//static MP_DEFINE_CONST_DICT(executor_module_globals, executor_module_globals_table);
//
// Define the module
//const mp_obj_module_t executor_module = {
//    .base = { &mp_type_module },
//    .globals = (mp_obj_dict_t *)&executor_module_globals,
//};
//
//MP_REGISTER_MODULE(MP_QSTR_mpy, executor_module);
//


#include "py/compile.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/objstr.h"
#include "py/persistentcode.h"
#include "extmod/vfs.h"

// Function to load and execute an MPY file from EEPROM
static mp_obj_t execute_mpy(mp_obj_t stream_in) {
     uint32_t start_address = mp_obj_get_int(start_obj);
    uint32_t size = mp_obj_get_int(size_obj);

    // Allocate buffer for reading entire MPY file
    uint8_t *buffer = m_new(uint8_t, size);

    // Read entire MPY file from EEPROM into buffer
    size_t bytes_read = eeprom_read(start_address, buffer, size);
    if (bytes_read != size) {
        m_del(uint8_t, buffer, size);
        mp_raise_msg(&mp_type_RuntimeError, "Failed to read complete MPY file from EEPROM");
    }

    // Create a reader object for the buffer
    mp_reader_t reader;
    reader.data = buffer;
    reader.readbyte = [](void *data) -> int {
        static size_t index = 0;
        uint8_t *buf = (uint8_t *)data;
        if (index < size) {
            return buf[index++];
        }
        return -1; // End of buffer
    };
    reader.close = [](void *data) {
        // Free the buffer when done
        m_del(uint8_t, buffer, size);
    };


    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        // Execute the given .mpy data.
        mp_module_context_t *ctx = m_new_obj(mp_module_context_t);
        ctx->module.globals = mp_globals_get();
        mp_compiled_module_t cm;
        cm.context = ctx;
        mp_raw_code_load(&reader, &cm);
        mp_obj_t f = mp_make_function_from_proto_fun(cm.rc, ctx, MP_OBJ_NULL);
        mp_call_function_0(f);
        nlr_pop();
    } else {
        // Uncaught exception: print it out.
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
    }

    return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_1(execute_mpy_obj, execute_mpy);

static const mp_rom_map_elem_t executor_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR_exec_buffer), MP_ROM_PTR(&execute_mpy_obj) },
};

static MP_DEFINE_CONST_DICT(executor_module_globals, executor_module_globals_table);

// Define the module
const mp_obj_module_t executor_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&executor_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_mpy, executor_module);

