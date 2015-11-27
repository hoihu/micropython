/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Martin Fischer
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

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "ringbuffer.h"

/**
 * simple ringbuffer handling that support putc/getc/is_full/is_empty functionality
 * may further be optmized by using 2'er complement masking instead of checking wraparound
 * conditions using if's (but the improvement is minimal).
 *
 * Example usage:
 *
 * uint8_t tx_buffer[256];
 * uint8_t* buf;
 * ringbuffer_t tx_ringbuffer;
 *
 * // initialize buffer first using "ringbuffer_init"
 * ringbuffer_init(&tx_ringbuffer, tx_buffer, sizeof(tx_buffer));
 * ringbuffer_putc(&tx_ringbuffer, "H")
 * ringbuffer_putc(&tx_ringbuffer, "i")
 * ringbuffer_getc(&tx_ringbuffer, buf);
 * ...
 */

void ringbuffer_init(ringbuffer_t* rbuffer, uint8_t* user_data, uint16_t len) {
    rbuffer->start = rbuffer->push_ptr = rbuffer->pop_ptr = user_data;
    rbuffer->sizeof_data = len;
    rbuffer->end = rbuffer->start + len - 1;
}

bool ringbuffer_is_full(ringbuffer_t* rbuffer) {
    return !ringbuffer_get_free_mem(rbuffer);
}

uint16_t ringbuffer_get_free_mem(ringbuffer_t* rbuffer) {
    if (rbuffer->push_ptr > rbuffer->pop_ptr) {
        return rbuffer->sizeof_data - (rbuffer->push_ptr - rbuffer->pop_ptr) - 1;
    } else {
        return rbuffer->pop_ptr - rbuffer->push_ptr - 1;
    }
}

uint16_t ringbuffer_get_used_mem(ringbuffer_t* rbuffer) {
    if (rbuffer->push_ptr > rbuffer->pop_ptr) {
        return rbuffer->push_ptr - rbuffer->pop_ptr;
    } else {
        return rbuffer->sizeof_data  - (rbuffer->pop_ptr - rbuffer->push_ptr);
    }
}

bool ringbuffer_putc(ringbuffer_t* rbuffer, uint8_t character) {
    *(rbuffer->push_ptr) = character;
    rbuffer->push_ptr++;
    if (rbuffer->push_ptr > rbuffer->end ) {
        rbuffer->push_ptr = rbuffer->start;
    }
    // check if buffer is full
    if (rbuffer->push_ptr == rbuffer->pop_ptr) {
        // move pop pointer one forward - overwrites oldest data
        rbuffer->pop_ptr++;
        if (rbuffer->pop_ptr  > rbuffer->end ) {
            rbuffer->pop_ptr = rbuffer->start;
        }
        // signal overwrite
        return false;
    }
    return true;
}

bool ringbuffer_getc(ringbuffer_t* rbuffer, uint8_t* ch) {
    if (ringbuffer_is_empty(rbuffer)) {
        return false;
    }
    *ch = *(rbuffer->pop_ptr);
    rbuffer->pop_ptr++;
    if (rbuffer->pop_ptr  > rbuffer->end ) {
        // wraparound
        rbuffer->pop_ptr = rbuffer->start;
    }
    return true;
}
