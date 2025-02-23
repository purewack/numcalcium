#include "py/builtin.h"
#include "py/runtime.h"
#include "py/obj.h"
#include "py/mphal.h"

#include "driver/gptimer.h"
#include "driver/sdm.h"
#include "driver/gpio.h"
#include "esp_intr_alloc.h"

#include <math.h>
#include "pins.h"
#include "board.h"


typedef struct _sdm_obj_t {
    mp_obj_base_t base;
	bool new; 
    mp_obj_t callback;
    int8_t *buffer_ptr;
    int8_t *buffer_ptr_near;
    int8_t *buffer_ptr_far;
    uint8_t buf_dual;
    uint16_t buf_size;
    uint16_t buf_counter;
    gptimer_handle_t _timer_handle;
    sdm_channel_handle_t channels[2];
    int pins[2];
    uint8_t channel_count;
} sdm_obj_t;

static bool IRAM_ATTR buffer_sample_feed_unibuf_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    sdm_obj_t *self = (sdm_obj_t*)user_ctx;

    sdm_channel_set_pulse_density(self->channels[0], self->buffer_ptr[self->buf_counter++]);
    if(self->channel_count > 1){
    sdm_channel_set_pulse_density(self->channels[1], self->buffer_ptr[self->buf_counter++]);
    }

    if (self->buf_counter == self->buf_size) {
        self->buf_counter = 0;    
        if (self->callback != mp_const_none && mp_obj_is_callable(self->callback)) {
            mp_sched_schedule(self->callback, MP_OBJ_NEW_SMALL_INT(1));
        }
        if(self->buf_dual){
            self->buffer_ptr = self->buffer_ptr_far;
        }
        //refill request from n/2->n
    }
    else if (self->buf_counter == self->buf_size>>1) {        
        if (self->callback != mp_const_none && mp_obj_is_callable(self->callback)) {
            mp_sched_schedule(self->callback, MP_OBJ_NEW_SMALL_INT(0));
        }
        if(self->buf_dual){
            self->buffer_ptr = self->buffer_ptr_near;
        }
        //refill request from 0->n/2
    }

    return false;
}


static mp_obj_t sdm_deinit(mp_obj_t self_in) {
    sdm_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->_timer_handle) {
        DEBUG_printf("stop timer\n");
        check_esp_err(gptimer_stop(self->_timer_handle));  // Stop the timer first
        DEBUG_printf("disable timer\n");
        check_esp_err(gptimer_disable(self->_timer_handle));  // Disable timer first
        DEBUG_printf("deleting of timer\n");
        check_esp_err(gptimer_del_timer(self->_timer_handle));
        self->_timer_handle = NULL;
    }

    DEBUG_printf("deleting of channels\n");
    for (int i = 0; i < 2; i++) {
        if (self->channels[i] != NULL) {
            DEBUG_printf("disable channel %d\n",self->channels[i]);
            check_esp_err(sdm_channel_disable(self->channels[i]));
            DEBUG_printf("deleting channel %d\n",self->channels[i]);
            check_esp_err(sdm_del_channel(self->channels[i]));
            self->channels[i] = NULL;
        }
        if(self->channels[i] != -1)
            gpio_reset_pin(self->pins[i]);
    }
    self->channel_count = 0;  // Reset count
    self->callback = mp_const_none;

    DEBUG_printf("deinit sigma-delta\n");
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(sdm_deinit_obj, sdm_deinit);


static mp_obj_t sdm_pause(mp_obj_t self_in) {
    sdm_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if(self->channel_count){
        for(int i=0; i< self->channel_count; i++){
            check_esp_err(sdm_channel_disable(self->channels[i]));
        }
        check_esp_err(gptimer_stop(self->_timer_handle));
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(sdm_pause_obj, sdm_pause);

static mp_obj_t sdm_resume(mp_obj_t self_in) {
    sdm_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if(self->channel_count){
        for(int i=0; i< self->channel_count; i++){
            check_esp_err(sdm_channel_enable(self->channels[i]));
        }
        check_esp_err(gptimer_start(self->_timer_handle));
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(sdm_resume_obj, sdm_resume);


static mp_obj_t sdm_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 2, 5, true);
    sdm_obj_t *self = mp_obj_malloc_with_finaliser(sdm_obj_t, &sdm_type);
    
    mp_buffer_info_t bufinfo[2];
    
    if(mp_obj_is_type(args[0], &mp_type_tuple)){
        self->buf_dual = 1;
        size_t n_items;
        mp_obj_t *items;
        mp_obj_tuple_get(args[0], &n_items, &items);
        if(n_items != 2){
            mp_raise_ValueError("bytearray tuple can only contian 2 entries");
        }
        mp_get_buffer_raise(items[0], &bufinfo[0], MP_BUFFER_RW);
        mp_get_buffer_raise(items[1], &bufinfo[1], MP_BUFFER_RW);
        if(bufinfo[0].len != bufinfo[1].len){
            mp_raise_ValueError("buffers must have equal size");
        }
        DEBUG_printf("dual buffers\n");
    }
    else{
        self->buf_dual = 0;
        mp_get_buffer_raise(args[0], &bufinfo[0], MP_BUFFER_RW);
        DEBUG_printf("single buffer\n");
    }

    mp_obj_t callback = args[1];
    if (!mp_obj_is_callable(callback)) {
        mp_raise_ValueError("callback must be callable");
    }

    int srate = 32000;
    int trate = srate * 300;
    int outputs[2];
    outputs[0] = BOARD_PIN_A_OUT_L;
    outputs[1] = BOARD_PIN_A_OUT_R;

    if(n_args >= 3){
        size_t len_outputs;
        mp_obj_t *items_outputs;
        mp_obj_get_array(args[2], &len_outputs, &items_outputs);
        if(len_outputs > 2){
            mp_raise_ValueError("Too many outputs defined");
        }
        if(len_outputs < 1){
            mp_raise_ValueError("Not enough outputs defined");
        }
        for(int i=0; i< len_outputs; i++){
            if(items_outputs[i] == mp_const_none)
                outputs[i] = -1;
            else if(!mp_obj_is_int(items_outputs[i])){
                outputs[i] = mp_hal_get_pin_obj(items_outputs[i]);
            }
            else
                outputs[i] = mp_obj_get_int(items_outputs[i]);
        }
        if(outputs[0] == outputs[1]){
            mp_raise_ValueError("Pins cannot refer to the same pin");
        }
    }
    if(n_args >= 4){
        srate = mp_obj_get_int(args[3]);
        trate = srate * 300;
    }
    if(n_args >= 5){
        trate = mp_obj_get_int(args[4]);
    }

    self->pins[0] = -1;
    self->pins[1] = -1;
    self->channel_count = 0;
    for (int i = 0; i < 2; i++) {
        if (outputs[i] == -1) continue;
        int pin = outputs[i];
        self->pins[self->channel_count] = pin;

        sdm_channel_handle_t chan = NULL;
        sdm_config_t config = {
            .clk_src = GPTIMER_CLK_SRC_XTAL,
            .sample_rate_hz = trate,
            .gpio_num = pin,
        };
        DEBUG_printf("ch: %d\n", pin);

        esp_err_t err = sdm_new_channel(&config, &chan);
        if (err != ESP_OK) {
            mp_raise_OSError(err);  // Raise error if allocation fails
        }

        DEBUG_printf("ch enable %d\n", pin);
        check_esp_err(sdm_channel_enable(chan));
        self->channels[self->channel_count++] = chan;
        DEBUG_printf("Channel configured to use pin: %d\n", pin);
    }
    DEBUG_printf("Channel config done, active channels %d\n",self->channel_count);
    
    /* Allocate GPTimer handle */
    gptimer_handle_t timer_handle;
    gptimer_config_t timer_cfg = {
        .clk_src = GPTIMER_CLK_SRC_XTAL,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = srate,
    };
    DEBUG_printf("Timer setup\n");
    check_esp_err(gptimer_new_timer(&timer_cfg, &timer_handle));
    
    /* Set the timer alarm configuration */
    gptimer_alarm_config_t alarm_cfg = {
        .alarm_count = 1,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };
    
    DEBUG_printf("Alarm setup");
    check_esp_err(gptimer_set_alarm_action(timer_handle, &alarm_cfg));

    /* Register the alarm callback */
   
    DEBUG_printf("Callback setup");
    gptimer_event_callbacks_t cbs = {
        .on_alarm = buffer_sample_feed_unibuf_cb,
    };
    check_esp_err(gptimer_register_event_callbacks(timer_handle, &cbs, self));
    check_esp_err(gptimer_enable(timer_handle));
    check_esp_err(gptimer_start(timer_handle));
    
    if(self->buf_dual)
        self->buffer_ptr_far = (int8_t *)bufinfo[1].buf;
    else
        self->buffer_ptr_far = (int8_t *)bufinfo[0].buf;
    self->buffer_ptr_near = (int8_t *)bufinfo[0].buf;
    self->buffer_ptr = self->buffer_ptr_near;
    self->buf_size = bufinfo[0].len;
    self->callback = callback;
    self->_timer_handle = timer_handle;
    DEBUG_printf("Started sigma-delta audio\n");

    return MP_OBJ_FROM_PTR(self);
}


static const mp_rom_map_elem_t sigma_delta_locals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&sdm_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&sdm_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_pause), MP_ROM_PTR(&sdm_pause_obj) },
    { MP_ROM_QSTR(MP_QSTR_resume), MP_ROM_PTR(&sdm_resume_obj) },
};
static MP_DEFINE_CONST_DICT(sigma_delta_locals, sigma_delta_locals_table);


MP_DEFINE_CONST_OBJ_TYPE(
    sdm_type,
    MP_QSTR_SigmaDelta,
    MP_TYPE_FLAG_NONE,
	make_new, sdm_make_new,
	locals_dict, &sigma_delta_locals
);
