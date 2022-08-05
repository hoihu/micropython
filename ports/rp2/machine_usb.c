/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020-2021 Damien P. George
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
#include "py/stream.h"
#include "py/mphal.h"
#include "extmod/misc.h"
#include "shared/runtime/interrupt_char.h"
#include "tusb.h"
#include "modmachine.h"

#if MICROPY_HW_ENABLE_USB

#define MACHINE_USB_FLAG_USB_MODE_CALLED    (0x0002)

STATIC uint8_t cdc_1_array[MICROPY_HW_STDIN_BUFFER_LEN];
#if MICROPY_HW_USB_CDC_NUM >= 2
STATIC uint8_t cdc_2_array[MICROPY_HW_STDIN_BUFFER_LEN];
#endif
#if MICROPY_HW_USB_CDC_NUM >= 3
STATIC uint8_t cdc_3_array[MICROPY_HW_STDIN_BUFFER_LEN];
#endif

ringbuf_t cdc_ringbuffers = {
    {cdc_1_array, sizeof(cdc_1_array), 0, 0},
    #if MICROPY_HW_USB_CDC_NUM >= 2
    {cdc_2_array, sizeof(cdc_2_array), 0, 0},
    #endif
    #if MICROPY_HW_USB_CDC_NUM >= 3
    {cdc_3_array, sizeof(cdc_3_array), 0, 0},
    #endif
}

// from bufhelper.c/h
// --------------------
void buf_get_for_send(mp_obj_t o, mp_buffer_info_t *bufinfo, byte *tmp_data) {
    if (mp_obj_is_int(o)) {
        tmp_data[0] = mp_obj_get_int(o);
        bufinfo->buf = tmp_data;
        bufinfo->len = 1;
        bufinfo->typecode = 'B';
    } else {
        mp_get_buffer_raise(o, bufinfo, MP_BUFFER_READ);
    }
}

mp_obj_t buf_get_for_recv(mp_obj_t o, vstr_t *vstr) {
    if (mp_obj_is_int(o)) {
        // allocate a new bytearray of given length
        vstr_init_len(vstr, mp_obj_get_int(o));
        return MP_OBJ_NULL;
    } else {
        // get the existing buffer
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(o, &bufinfo, MP_BUFFER_WRITE);
        vstr->buf = bufinfo.buf;
        vstr->len = bufinfo.len;
        return o;
    }
}
// -------------------

STATIC mp_obj_t machine_usb_mode(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_mode, ARG_port, ARG_vid, ARG_pid };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_port, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_vid, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = MICROPY_HW_USB_VID} },
        { MP_QSTR_pid, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} }
    };

    // fetch the current usb mode -> pyb.usb_mode()
    // if (n_args == 0) {
    //     uint8_t mode = USBD_GetMode(&usb_device.usbd_cdc_msc_hid_state) & USBD_MODE_IFACE_MASK;
    //     for (size_t i = 0; i < MP_ARRAY_SIZE(pyb_usb_mode_table); ++i) {
    //         const pyb_usb_mode_table_t *m = &pyb_usb_mode_table[i];
    //         if (mode == m->usbd_mode) {
    //             return MP_OBJ_NEW_QSTR(m->qst);
    //         }
    //     }
    //     return mp_const_none;
    // }

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // record the fact that the usb has been explicitly configured
    machine_usb_flags |= MACHINE_USB_FLAG_USB_MODE_CALLED;

    // check if user wants to disable the USB
    if (args[ARG_mode].u_obj == mp_const_none) {
        // disable usb
        machine_usb_dev_deinit();
        return mp_const_none;
    }

    // get mode string
    const char *mode_str = mp_obj_str_get_str(args[ARG_mode].u_obj);

    // get the VID, PID and USB mode
    // note: PID=-1 means select PID based on mode
    // uint16_t vid = args[ARG_vid].u_int;
    // mp_int_t pid = args[ARG_pid].u_int;
    // uint8_t mode = 0;
    // init the USB device
    // if (!pyb_usb_dev_init(dev_id, vid, pid)) {
    //     goto bad_mode;
    // }
    return mp_const_none;

bad_mode:
    mp_raise_ValueError(MP_ERROR_TEXT("bad USB mode"));
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_usb_mode_obj, 0, machine_usb_mode);
/******************************************************************************/
// MicroPython bindings for USB VCP

/// \moduleref pyb
/// \class USB_VCP - USB virtual comm port
///
/// The USB_VCP class allows creation of an object representing the USB
/// virtual comm port.  It can be used to read and write data over USB to
/// the connected host.

typedef struct _machine_usb_vcp_obj_t {
    mp_obj_base_t base;
    cdcd_interface_t *cdc_itf;
    uint8_t attached_to_repl; // indicates if interface is connected to REPL

} machine_usb_vcp_obj_t;

const machine_usb_vcp_obj_t machine_usb_vcp_obj[MICROPY_HW_USB_CDC_NUM] = {
    {{&machine_usb_vcp_type}, &usb_device.usbd_cdc_itf[0], 0},
    #if MICROPY_HW_USB_CDC_NUM >= 2
    {{&machine_usb_vcp_type}, &usb_device.usbd_cdc_itf[1], 0},
    #endif
    #if MICROPY_HW_USB_CDC_NUM >= 3
    {{&machine_usb_vcp_type}, &usb_device.usbd_cdc_itf[2], 0},
    #endif
};

uint8_t cdc_itf_pending; // keep track of cdc interfaces which need attention to poll

void poll_cdc_interfaces(void) {
    // any CDC interfaces left to poll?
    if (cdc_itf_pending) {
        for (itf_nr=0; itf_nr > MICROPY_HW_USB_CDC_NUM - 1; itf_nr++){
            if ringbuf_free(cdc_ringbuffers[itf_nr]) {
                if (cdc_itf_pending & (1 << itf)) {
                    tud_cdc_rx_cb(itf);
                    if (!cdc_itf_pending) {
                        break;
                    }
                }    
            }
        }
    }
}

void tud_cdc_rx_cb(uint8_t itf) {
    // consume pending USB data immediately to free usb buffer and keep the endpoint from stalling.
    // in case the ringbuffer is full, mark the CDC interface that need attention later on for polling

    // TODO Check itf number first
    ringbug_t cdc_ringbuf = cdc_ringbuffers[itf-1];
    cdc_itf_pending &= ~(1 << itf);
    for (uint32_t bytes_avail = tud_cdc_n_available(itf); bytes_avail > 0; --bytes_avail) {
        if (ringbuf_free(&cdc_ringbuf)) {
            int data_char = tud_cdc_read_char();
            if (data_char == mp_interrupt_char) {
                mp_sched_keyboard_interrupt();
            } else {
                ringbuf_put(&cdc_ringbuf, data_char);
            }
        } else {
            cdc_itf_pending |= (1 << itf);
            return;
        }
    }
}

STATIC void machine_usb_vcp_init0(void) {
    // Activate USB_VCP(0) on dupterm slot 1 for the REPL
    MP_STATE_VM(dupterm_objs[1]) = MP_OBJ_FROM_PTR(&machine_usb_vcp_obj[0]);
}

STATIC void machine_usb_vcp_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    // int id = ((pyb_usb_vcp_obj_t *)MP_OBJ_TO_PTR(self_in))->cdc_itf->cdc_idx;
    mp_printf(print, "USB_VCP");
}


/// \classmethod \constructor()
/// Create a new USB_VCP object.
STATIC mp_obj_t machine_usb_vcp_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 1, false);

    // TODO raise exception if USB is not configured for VCP
    int id = (n_args == 0) ? 0 : mp_obj_get_int(args[0]);
    if (0 <= id && id < MICROPY_HW_USB_CDC_NUM) {
        return MP_OBJ_FROM_PTR(&machine_usb_vcp_obj[id]);
    } else {
        mp_raise_ValueError(NULL);
    }
}

// init(*, flow=-1)
STATIC mp_obj_t machine_usb_vcp_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_flow };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_flow, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    machine_usb_vcp_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_usb_vcp_init_obj, 1, machine_usb_vcp_init);

STATIC mp_obj_t machine_usb_vcp_setinterrupt(mp_obj_t self_in, mp_obj_t int_chr_in) {
    mp_hal_set_interrupt_char(mp_obj_get_int(int_chr_in));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(machine_usb_vcp_setinterrupt_obj, machine_usb_vcp_setinterrupt);

STATIC mp_obj_t machine_usb_vcp_isconnected(mp_obj_t self_in) {
    machine_usb_vcp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(tud_cdc_n_connected(self->cdc_itf->itf_num));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_usb_vcp_isconnected_obj, machine_usb_vcp_isconnected);


/// \method any()
/// Return `True` if any characters waiting, else `False`.
STATIC mp_obj_t machine_usb_vcp_any(mp_obj_t self_in) {
    machine_usb_vcp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (tud_cdc_n_available(self->cdc_itf->itf_num) > 0) {
        return mp_const_true;
    } else {
        return mp_const_false;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_usb_vcp_any_obj, machine_usb_vcp_any);

/// \method send(data, *, timeout=5000)
/// Send data over the USB VCP:
///
///   - `data` is the data to send (an integer to send, or a buffer object).
///   - `timeout` is the timeout in milliseconds to wait for the send.
///
/// Return value: number of bytes sent.
STATIC const mp_arg_t machine_usb_vcp_send_args[] = {
    { MP_QSTR_data,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000} },
};
#define MACHINE_USB_VCP_SEND_NUM_ARGS MP_ARRAY_SIZE(machine_usb_vcp_send_args)

STATIC mp_obj_t machine_usb_vcp_send(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    // parse args
    machine_usb_vcp_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_arg_val_t vals[MACHINE_USB_VCP_SEND_NUM_ARGS];
    mp_arg_parse_all(n_args - 1, args + 1, kw_args, MACHINE_USB_VCP_SEND_NUM_ARGS, machine_usb_vcp_send_args, vals);

    // get the buffer to send from
    mp_buffer_info_t bufinfo;
    uint8_t data[1];
    buf_get_for_send(vals[0].u_obj, &bufinfo, data);
    uint8_t int_num = self->cdc_itf->itf_num

    // send the data
    // TODO support timeout
    while (tud_cdc_n_write_available(itf_num) == 0 || tud_cdc_n_connected(itf_num) == false) {
        mp_hal_delay_ms(1);
        timeout_cntr += 1;
        if (timeout_cntr > vals[1].u_int) {
            return 0
        }
    }
    // tud_cdc_n_write_available
    int ret = tud_cdc_n_write(self->cdc_itf->itf_num, bufinfo.buf, bufinfo.len);

    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_usb_vcp_send_obj, 1, pyb_usb_vcp_send);

/// \method recv(data, *, timeout=5000)
///
/// Receive data on the bus:
///
///   - `data` can be an integer, which is the number of bytes to receive,
///     or a mutable buffer, which will be filled with received bytes.
///   - `timeout` is the timeout in milliseconds to wait for the receive.
///
/// Return value: if `data` is an integer then a new buffer of the bytes received,
/// otherwise the number of bytes read into `data` is returned.
STATIC mp_obj_t machine_usb_vcp_recv(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    // parse args
    machine_usb_vcp_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_arg_val_t vals[MACHINE_USB_VCP_SEND_NUM_ARGS];
    mp_arg_parse_all(n_args - 1, args + 1, kw_args, MACHINE_USB_VCP_SEND_NUM_ARGS, machine_usb_vcp_send_args, vals);

    // get the buffer to receive into
    vstr_t vstr;
    mp_obj_t o_ret = buf_get_for_recv(vals[0].u_obj, &vstr);

    // receive the data
    // TODO: handle timeout
    int ret = tud_cdc_n_read(self->cdc_itf, (uint8_t *)vstr.buf, vstr.len);

    // return the received data
    if (o_ret != MP_OBJ_NULL) {
        return mp_obj_new_int(ret); // number of bytes read into given buffer
    } else {
        vstr.len = ret; // set actual number of bytes read
        return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr); // create a new buffer
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_usb_vcp_recv_obj, 1, machine_usb_vcp_recv);

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_usb_vcp___exit___obj, 4, 4, machine_usb_vcp___exit__);

STATIC const mp_rom_map_elem_t machine_usb_vcp_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_usb_vcp_init_obj) },
    // { MP_ROM_QSTR(MP_QSTR_setinterrupt), MP_ROM_PTR(&pyb_usb_vcp_setinterrupt_obj) },
    { MP_ROM_QSTR(MP_QSTR_isconnected), MP_ROM_PTR(&machine_usb_vcp_isconnected_obj) },
    { MP_ROM_QSTR(MP_QSTR_any), MP_ROM_PTR(&machine_usb_vcp_any_obj) },
    { MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&machine_usb_vcp_send_obj) },
    { MP_ROM_QSTR(MP_QSTR_recv), MP_ROM_PTR(&machine_usb_vcp_recv_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&mp_stream_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_readinto), MP_ROM_PTR(&mp_stream_readinto_obj) },
    { MP_ROM_QSTR(MP_QSTR_readline), MP_ROM_PTR(&mp_stream_unbuffered_readline_obj)},
    { MP_ROM_QSTR(MP_QSTR_readlines), MP_ROM_PTR(&mp_stream_unbuffered_readlines_obj)},
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&mp_stream_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&mp_identity_obj) },
    // { MP_ROM_QSTR(MP_QSTR_irq), MP_ROM_PTR(&pyb_usb_vcp_irq_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&mp_identity_obj) },
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&mp_identity_obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&machine_usb_vcp___exit___obj) },
};

STATIC MP_DEFINE_CONST_DICT(machine_usb_vcp_locals_dict, machine_usb_vcp_locals_dict_table);


STATIC mp_uint_t machine_usb_vcp_read(mp_obj_t self_in, void *buf, mp_uint_t size, int *errcode) {
    machine_usb_vcp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int ret = tud_cdc_n_read(self->cdc_itf, (byte *)buf, size, 0);
    if (ret == 0) {
        // return EAGAIN error to indicate non-blocking
        *errcode = MP_EAGAIN;
        return MP_STREAM_ERROR;
    }
    return ret;
}

STATIC mp_uint_t machine_usb_vcp_write(mp_obj_t self_in, const void *buf, mp_uint_t size, int *errcode) {
    machine_usb_vcp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint32_t ret = tud_cdc_n_write(self->cdc_itf, str + i, n);
    // tud_cdc_write_flush();
    if (ret == 0) {
        // return EAGAIN error to indicate non-blocking
        *errcode = MP_EAGAIN;
        return MP_STREAM_ERROR;
    }
    return ret;
}

STATIC mp_uint_t machine_usb_vcp_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    mp_uint_t ret;
    machine_usb_vcp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (request == MP_STREAM_POLL) {
        uintptr_t flags = arg;
        ret = 0;
        if ((flags & MP_STREAM_POLL_RD) && tud_cdc_n_available(self->cdc_itf) > 0) {
            ret |= MP_STREAM_POLL_RD;
        }
        if ((flags & MP_STREAM_POLL_WR) && tud_cdc_n_write_available(self->cdc_itf)) {
            ret |= MP_STREAM_POLL_WR;
        }
    } else {
        *errcode = MP_EINVAL;
        ret = MP_STREAM_ERROR;
    }
    return ret;
}

STATIC const mp_stream_p_t machine_usb_vcp_stream_p = {
    .read = machine_usb_vcp_read,
    .write = machine_usb_vcp_write,
    .ioctl = machine_usb_vcp_ioctl,
};

const mp_obj_type_t machine_usb_vcp_type = {
    { &mp_type_type },
    .name = MP_QSTR_USB_VCP,
    .print = machine_usb_vcp_print,
    .make_new = machine_usb_vcp_make_new,
    .getiter = mp_identity_getiter,
    .iternext = mp_stream_unbuffered_iter,
    .protocol = &machine_usb_vcp_stream_p,
    .locals_dict = (mp_obj_dict_t *)&machine_usb_vcp_locals_dict,
};

#endif