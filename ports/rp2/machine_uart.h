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
#include "py/mperrno.h"
#include "modmachine.h"

#include "hardware/irq.h"
#include "hardware/uart.h"
#include "hardware/regs/uart.h"

typedef struct _machine_uart_obj_t {
    mp_obj_base_t base;
    uart_inst_t *const uart;
    uint8_t uart_id;
    uint32_t baudrate;
    uint8_t bits;
    uart_parity_t parity;
    uint8_t stop;
    uint8_t tx;
    uint8_t rx;
    uint8_t cts;
    uint8_t rts;
    uint16_t timeout;       // timeout waiting for first char (in ms)
    uint16_t timeout_char;  // timeout waiting between chars (in ms)
    uint8_t invert;
    uint8_t flow;
    ringbuf_t read_buffer;
    mutex_t *read_mutex;
    ringbuf_t write_buffer;
    mutex_t *write_mutex;
    uint8_t attached_to_repl;
} machine_uart_obj_t;

extern void uart_attach_to_repl(machine_uart_obj_t *self, bool attached);
