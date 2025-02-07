#include "py/builtin.h"
#include "py/runtime.h"
#include "py/obj.h"
#include "py/mphal.h"

#include "driver/gptimer.h"
#include "driver/sdm.h"
#include "esp_intr_alloc.h"

#include <math.h>

typedef struct _dac_config {
    mp_obj_t callback;
    int8_t *buffer_ptr;
    uint16_t buf_size;
    uint16_t buf_counter;
    gptimer_handle_t _timer_handle;
    sdm_channel_handle_t channels[2];
    uint8_t channel_count;
} dac_config_t;

static dac_config_t dac_config = {mp_const_none};

static bool IRAM_ATTR buffer_sample_feed_unibuf_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    dac_config_t *conf = (dac_config_t*)user_ctx;

    sdm_channel_set_pulse_density(conf->channels[0], conf->buffer_ptr[conf->buf_counter++]);
    if(conf->channel_count > 1)
    sdm_channel_set_pulse_density(conf->channels[1], conf->buffer_ptr[conf->buf_counter++]);

    if (conf->buf_counter == conf->buf_size) {
        conf->buf_counter = 0;
        mp_sched_schedule(conf->callback, MP_OBJ_NEW_SMALL_INT(1));
        //refill request from n/2->n
    }
    else if (conf->buf_counter == conf->buf_size>>1) {
        mp_sched_schedule(conf->callback, MP_OBJ_NEW_SMALL_INT(0));
        //refill request from 0->n/2
    }

    return false;
}


static mp_obj_t sdm_deinit() {
    if(dac_config._timer_handle){
        gptimer_del_timer(dac_config._timer_handle);
        dac_config._timer_handle = NULL;

        for(int i=0; i<dac_config.channel_count; i++){
            check_esp_err(sdm_channel_disable(dac_config.channels[i]));
            check_esp_err(sdm_del_channel(dac_config.channels[i]));
            dac_config.channels[i] = NULL;
        }
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(sdm_deinit_obj, sdm_deinit);

// Start playback
static mp_obj_t sdm_init(size_t n_args, const mp_obj_t *args) {
    mp_obj_t* callback, buffer, outputs;
    
    if(n_args == 3){
        callback = args[0];
        buffer = args[1];
        outputs = args[2];
    }

    if (!mp_obj_is_callable(&callback)) {
        mp_raise_ValueError("callback must be callable");
    }
    if (!mp_obj_is_type(outputs, &mp_type_tuple)) {
        mp_raise_TypeError(MP_ERROR_TEXT("Expected a tuple for 1st argument"));
    }
    
    if(dac_config._timer_handle) return sdm_deinit();
    
    size_t len_outputs;
    mp_obj_t *items_outputs;
    mp_obj_get_array(&outputs, &len_outputs, &items_outputs);

    mp_buffer_info_t bufinfo;
    
    mp_get_buffer_raise(&buffer, &bufinfo, MP_BUFFER_RW);
    
    if (len_outputs < 1 || len_outputs > 2) {
        mp_raise_ValueError("Output numbers invalid");
    }
    
    dac_config.buffer_ptr = (int8_t *)bufinfo.buf;
    dac_config.buf_size = bufinfo.len;
    
    dac_config.channel_count = 0;

    for(int i=0; i<len_outputs; i++){
        if(items_outputs[i] == mp_const_none) continue;
        int pin = mp_obj_get_int(items_outputs[i]);
        sdm_channel_handle_t chan = NULL;
        sdm_config_t config = {
            .clk_src = GPTIMER_CLK_SRC_XTAL,
            .sample_rate_hz = 11 * 1000 * 1000,
            .gpio_num = pin,
        };
        check_esp_err(sdm_new_channel(&config, &chan));
        check_esp_err(sdm_channel_enable(chan));
        dac_config.channels[dac_config.channel_count++] = chan;
        DEBUG_printf("channel configured: %d\n",pin);
    }
    
    /* Allocate GPTimer handle */
    gptimer_handle_t timer_handle;
    gptimer_config_t timer_cfg = {
        .clk_src = GPTIMER_CLK_SRC_XTAL,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 32000,
    };
    check_esp_err(gptimer_new_timer(&timer_cfg, &timer_handle));
    
    /* Set the timer alarm configuration */
    gptimer_alarm_config_t alarm_cfg = {
        .alarm_count = 1,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };
    check_esp_err(gptimer_set_alarm_action(timer_handle, &alarm_cfg));

    /* Register the alarm callback */
    
    dac_config.callback = &callback;
    dac_config._timer_handle = timer_handle;

    gptimer_event_callbacks_t cbs = {
        .on_alarm = buffer_sample_feed_unibuf_cb,
    };
    check_esp_err(gptimer_register_event_callbacks(timer_handle, &cbs, &dac_config));
    check_esp_err(gptimer_enable(timer_handle));
    check_esp_err(gptimer_start(timer_handle));

    DEBUG_printf("Playing audio\n");

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(sdm_init_obj, 2,3, sdm_init);


static mp_obj_t sdm_pause() {
    if(dac_config.channel_count){
        for(int i=0; i<dac_config.channel_count; i++){
            check_esp_err(sdm_channel_disable(dac_config.channels[i]));
        }
        check_esp_err(gptimer_stop(dac_config._timer_handle));
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(sdm_pause_obj, sdm_pause);

static mp_obj_t sdm_resume() {
    if(dac_config.channel_count){
        for(int i=0; i<dac_config.channel_count; i++){
            check_esp_err(sdm_channel_enable(dac_config.channels[i]));
        }
        check_esp_err(gptimer_start(dac_config._timer_handle));
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(sdm_resume_obj, sdm_resume);


static const mp_rom_map_elem_t sigma_delta_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&sdm_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&sdm_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_pause), MP_ROM_PTR(&sdm_pause_obj) },
    { MP_ROM_QSTR(MP_QSTR_resume), MP_ROM_PTR(&sdm_resume_obj) },
};
static MP_DEFINE_CONST_DICT(sigma_delta_module_globals, sigma_delta_module_globals_table);

const mp_obj_module_t sigma_delta_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&sigma_delta_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_sigma_delta, sigma_delta_user_cmodule);
