/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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

void ringbuffer_init(ringbuffer_t* rbuffer, uint8_t* user_data, uint16_t len) {
    rbuffer->start = rbuffer->push_ptr = rbuffer->pop_ptr = user_data;
    rbuffer->sizeof_data = len;
    rbuffer->end = rbuffer->start + len - 1;
}

bool ringbuffer_is_empty(ringbuffer_t* rbuffer) {
    return (rbuffer->push_ptr == rbuffer->pop_ptr);
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
        return false;
    }
    return true;
}

uint8_t ringbuffer_getc(ringbuffer_t* rbuffer) {
    uint8_t c;
    c = *(rbuffer->pop_ptr);
    rbuffer->pop_ptr++;
    if (rbuffer->pop_ptr  > rbuffer->end ) {
        // wraparound
        rbuffer->pop_ptr = rbuffer->start;
    }
    return c;
}
