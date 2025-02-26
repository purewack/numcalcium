//#include "py/compile.h"
//#include "py/runtime.h"
//#include "py/stream.h"
//#include "py/objstr.h"
//#include "py/persistentcode.h"
//#include "extmod/vfs.h"
//
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


/*
#include "py/compile.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/objstr.h"
#include "py/persistentcode.h"
#include "extmod/vfs.h"

 Function to load and execute an MPY file from EEPROM
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

 Define the module
const mp_obj_module_t executor_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&executor_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_mpy, executor_module);
*/

#include "py/obj.h"
#include "py/stream.h"
#include "py/reader.h"
#include "py/runtime.h"
#include "py/persistentcode.h"
#include "py/compile.h"
#include "py/parse.h"
#include "py/builtin.h"

typedef struct _mpy_streamer_t {
    mp_stream_p_t* stream_p;
    mp_obj_t stream;
    int pos;
    int len;
    size_t readsize;
    byte *buf;
} mpy_streamer_t;

static mp_uint_t mpy_stream_readbyte(void *data) {
    mpy_streamer_t *rs = (mpy_streamer_t *)data;

    // Check if we need to refill the buffer
    if (rs->pos >= rs->len) {
        int err;
        rs->pos = 0;
        rs->len = rs->stream_p->read(rs->stream, rs->buf, rs->readsize, &err);
        if (rs->len == MP_STREAM_ERROR) {
            mp_raise_OSError(err);
        }
        if (rs->len == 0) {
            DEBUG_printf("eof\n\r");
            return MP_READER_EOF; // End of stream
        }
        DEBUG_printf("refill\n\r");
    }

    // Return the next byte from the buffer
    byte rbyte = rs->buf[rs->pos++];
    DEBUG_printf("read %d\n\r",rbyte);
    return rbyte;
}

static void mpy_stream_close(void *data) {
    DEBUG_printf("close req\n");
    mpy_streamer_t *rs = (mpy_streamer_t *)data;
    mp_stream_close(rs->stream);
    m_del_obj(mpy_streamer_t, rs);
    DEBUG_printf("closed\n\r");
}

static mp_obj_t load_stream(size_t n_args, const mp_obj_t *args) {
    mp_reader_t reader;
    
    mpy_streamer_t *rs = m_new_obj(mpy_streamer_t);
    rs->stream = args[0];
    rs->stream_p =  mp_get_stream_raise(rs->stream, MP_STREAM_OP_READ);
    rs->pos = 0;
    rs->len = 0;
    rs->readsize = n_args == 2 ? mp_obj_get_int(args[1]) : 1024;
    size_t bufpointer_size = rs->readsize;
    byte *bufpointer = rs->buf = m_new(byte, bufpointer_size);
    
    reader.data = rs;
    reader.readbyte = mpy_stream_readbyte;
    reader.close = mpy_stream_close;

//    reader.readbyte((void*)reader.data);
//    while(reader.readbyte((void*)reader.data) != MP_READER_EOF){};

    
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
    
    DEBUG_printf("ended\n\r");
    m_del(byte, bufpointer, bufpointer_size);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(load_stream_obj,1,2, load_stream);


static mp_obj_t load_mem(mp_obj_t buffer_in) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buffer_in, &bufinfo, MP_BUFFER_READ);

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        // Execute the given .mpy data.
        mp_module_context_t *ctx = m_new_obj(mp_module_context_t);
        ctx->module.globals = mp_globals_get();
        mp_compiled_module_t cm;
        cm.context = ctx;
        mp_raw_code_load_mem(bufinfo.buf, bufinfo.len, &cm);
        mp_obj_t f = mp_make_function_from_proto_fun(cm.rc, ctx, MP_OBJ_NULL);
        mp_call_function_0(f);
        nlr_pop();
    } else {
        // Uncaught exception: print it out.
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
    }

    return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_1(load_mem_obj, load_mem);

static const mp_rom_map_elem_t mpy_loader_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_mpy) },
    { MP_ROM_QSTR(MP_QSTR_from_stream), MP_ROM_PTR(&load_stream_obj) },
    { MP_ROM_QSTR(MP_QSTR_from_memory), MP_ROM_PTR(&load_mem_obj) }
};

static MP_DEFINE_CONST_DICT(mpy_loader_module_globals, mpy_loader_module_globals_table);

const mp_obj_module_t mpy_loader_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mpy_loader_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_mpy, mpy_loader_module);

