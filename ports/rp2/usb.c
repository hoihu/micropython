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

#include "py/ringbuf.h"
#include "extmod/misc.h"
#include "shared/runtime/interrupt_char.h"
#include "modmachine.h"
#include "mphalport.h"

#if MICROPY_HW_ENABLE_USBDEV

extern uint32_t tud_cdc_n_available       (uint8_t itf);
extern void tud_cdc_rx_cb(uint8_t itf);
extern int32_t  tud_cdc_n_read_char       (uint8_t itf);
uint32_t tud_cdc_n_read            (uint8_t itf, void* buffer, uint32_t bufsize);

STATIC uint8_t cdc_1_array[MICROPY_HW_STDIN_BUFFER_LEN];
#if MICROPY_HW_USB_CDC_NUM >= 2
STATIC uint8_t cdc_2_array[MICROPY_HW_STDIN_BUFFER_LEN];
#endif
#if MICROPY_HW_USB_CDC_NUM >= 3
STATIC uint8_t cdc_3_array[MICROPY_HW_STDIN_BUFFER_LEN];
#endif

ringbuf_t cdc_ringbuffers[] = {
    {.buf=cdc_1_array, .size=sizeof(cdc_1_array), .iget=0, .iput=0},
    #if MICROPY_HW_USB_CDC_NUM >= 2
    {.buf=cdc_2_array, .size=sizeof(cdc_2_array), .iget=0, .iput=0},
    #endif
    #if MICROPY_HW_USB_CDC_NUM >= 3
    {.buf=cdc_3_array, .size=sizeof(cdc_3_array), .iget=0, .iput=0},
    #endif
};

uint8_t cdc_itf_pending; // keep track of cdc interfaces which need attention to poll

void poll_cdc_interfaces(void) {
    // any CDC interfaces left to poll?
    if (cdc_itf_pending) {
        for (uint8_t itf_nr=0; itf_nr < MICROPY_HW_USB_CDC_NUM; itf_nr++){
            if (ringbuf_free(&cdc_ringbuffers[itf_nr])) {
                if (cdc_itf_pending & (1 << itf_nr)) {
                    tud_cdc_rx_cb(itf_nr);
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
    ringbuf_t cdc_ringbuf = cdc_ringbuffers[itf-1];
    cdc_itf_pending &= ~(1 << itf);
    for (int bytes_avail = tud_cdc_n_available(itf); bytes_avail > 0; --bytes_avail) {
        if (ringbuf_free(&cdc_ringbuf)) {
            int data_char;
            tud_cdc_n_read(itf, &data_char, 1);
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
#endif