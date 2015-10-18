#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "py/nlr.h"
#include "py/lexer.h"
#include "py/parse.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/stackctrl.h"
#include "py/gc.h"


#include "gccollect.h"
#include "systick.h"
#include "readline.h"
#include "pyexec.h"
// #include "i2c.h"
// #include "spi.h"
// #include "uart.h"
#include "timer.h"
// #include "led.h"
#include "pin.h"
// #include "extint.h"
// #include "usrsw.h"
#include "usb.h"
#include "rtc.h"
#include "rng.h"
#include MICROPY_HAL_H

#include "stm32_hal_legacy.h"

extern void SystemClock_Config(void);
extern void pendsv_init(void);
extern void toogle_debug(void);

void storage_irq_handler() {}

mp_import_stat_t mp_import_stat(const char *path) {
    return MP_IMPORT_STAT_NO_EXIST;
}

STATIC mp_obj_t pyb_main(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_opt, MP_ARG_INT, {.u_int = 0} }
    };

    if (MP_OBJ_IS_STR(pos_args[0])) {
        MP_STATE_PORT(pyb_config_main) = pos_args[0];

        // parse args
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
        MP_STATE_VM(mp_optimise_value) = args[0].u_int;
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(pyb_main_obj, 1, pyb_main);

static char heap[2048];

int main(int argc, char **argv) {

    // uint32_t tickstart;
    // Stack limit should be less than real stack size, so we have a chance
    // to recover from limit hit.  (Limit is measured in bytes.)
    mp_stack_set_limit((char*)&_ram_end - (char*)&_heap_end - 1024);

    HAL_Init();

    // enable GPIO clocks
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();

    #if defined(MICROPY_BOARD_EARLY_INIT)
    MICROPY_BOARD_EARLY_INIT();
    #endif

    // set the system clock to be HSE
    SystemClock_Config();

    // tickstart = HAL_GetTick();
    // /* Wait till HSE is disabled */
    // while( tickstart + 20 > HAL_GetTick()) {
        // toogle_debug();
    // }
    // basic sub-system init
    pendsv_init();
    // TIM3 is set-up for the USB CDC interface
    timer_tim3_init();

    int first_soft_reset = true;

// soft_reset:

#if MICROPY_HW_ENABLE_RTC
    if (first_soft_reset) {
        rtc_init();
    }
#endif

    // GC init
    gc_init(&_heap_start, &_heap_end);

    // Micro Python init
    mp_init();
    readline_init0();
    // pin_init0();
    // extint_init0();
    timer_init0();
    // uart_init0();

    pyb_usb_init0();

    // init USB device to default setting if it was not already configured
    if (!(pyb_usb_flags & PYB_USB_FLAG_USB_MODE_CALLED)) {
        pyb_usb_dev_init_cdc(USBD_VID, USBD_PID_CDC_MSC);
    }

    while (1);


    #if MICROPY_ENABLE_GC
    gc_init(heap, heap + sizeof(heap));
    #endif
    mp_init();
    #if MICROPY_REPL_EVENT_DRIVEN
    pyexec_event_repl_init();
    for (;;) {
        int c = mp_hal_stdin_rx_chr();
        if (pyexec_event_repl_process_char(c)) {
            break;
        }
    }
    #else
    pyexec_friendly_repl();
    #endif
    //do_str("print('hello world!', list(x+1 for x in range(10)), end='eol\\n')", MP_PARSE_SINGLE_INPUT);
    //do_str("for i in range(10):\r\n  print(i)", MP_PARSE_FILE_INPUT);
    mp_deinit();
    return 0;
}

// void gc_collect(void) {
//     // WARNING: This gc_collect implementation doesn't try to get root
//     // pointers from CPU registers, and thus may function incorrectly.
//     void *dummy;
//     gc_collect_start();
//     gc_collect_root(&dummy, ((mp_uint_t)stack_top - (mp_uint_t)&dummy) / sizeof(mp_uint_t));
//     gc_collect_end();
//     gc_dump_info();
// }

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    return NULL;
}


// mp_import_stat_t mp_import_stat(const char *path) {
//     return MP_IMPORT_STAT_NO_EXIST;
// }

mp_obj_t mp_builtin_open(uint n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);

void nlr_jump_fail(void *val) {
}

void NORETURN __fatal_error(const char *msg) {
    while (1);
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif

#if !MICROPY_MIN_USE_STDOUT
void _start(void) {main(0, NULL);}
#endif
