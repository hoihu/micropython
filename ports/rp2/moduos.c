/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "py/runtime.h"
#include "machine_uart.h"
#include "rp2_usb.h"

extern const mp_obj_type_t machine_uart_type;
extern const mp_obj_type_t rp2_usb_vcp_type; 

extern void uart_attach_to_repl(machine_uart_obj_t *self, bool attached);
extern void usb_vcp_attach_to_repl(rp2_usb_vcp_obj_t *self, bool attached);


uint8_t rosc_random_u8(size_t cycles);

STATIC mp_obj_t mp_uos_urandom(mp_obj_t num) {
    mp_int_t n = mp_obj_get_int(num);
    vstr_t vstr;
    vstr_init_len(&vstr, n);
    for (int i = 0; i < n; i++) {
        vstr.buf[i] = rosc_random_u8(8);
    }
    return mp_obj_new_bytes_from_vstr(&vstr);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_uos_urandom_obj, mp_uos_urandom);

bool mp_uos_dupterm_is_builtin_stream(mp_const_obj_t stream) {
    const mp_obj_type_t *type = mp_obj_get_type(stream);
    return type == &machine_uart_type
           #if MICROPY_HW_ENABLE_USB
           || type == &pyb_usb_vcp_type
           #endif
    ;
}

void mp_uos_dupterm_stream_detached_attached(mp_obj_t stream_detached, mp_obj_t stream_attached) {
    if (mp_obj_get_type(stream_detached) == &machine_uart_type) {
        uart_attach_to_repl(MP_OBJ_TO_PTR(stream_detached), false);
    }
    #if MICROPY_HW_ENABLE_USBDEV
    if (mp_obj_get_type(stream_detached) == &rp2_usb_vcp_type) {
        usb_vcp_attach_to_repl(MP_OBJ_TO_PTR(stream_detached), false);
    }
    #endif

    if (mp_obj_get_type(stream_attached) == &machine_uart_type) {
        uart_attach_to_repl(MP_OBJ_TO_PTR(stream_attached), true);
    }
    #if MICROPY_HW_ENABLE_USBDEV
    if (mp_obj_get_type(stream_attached) == &rp2_usb_vcp_type) {
        usb_vcp_attach_to_repl(MP_OBJ_TO_PTR(stream_attached), true);
    }
    #endif
}