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

#define RP2_USB_FLAG_USB_MODE_CALLED    (0x0002)

typedef struct _rp2_usb_vcp_obj_t {
    mp_obj_base_t base;
    uint8_t cdc_itf_nr;
    uint8_t attached_to_repl; // indicates if interface is connected to REPL
} rp2_usb_vcp_obj_t;

void usb_vcp_attach_to_repl(rp2_usb_vcp_obj_t *self, bool attached);

extern const mp_obj_type_t rp2_usb_vcp_type; 
extern mp_uint_t rp2_usb_flags;