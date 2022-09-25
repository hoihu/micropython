#ifndef MICROPY_INCLUDED_RP2_USB_H
#define MICROPY_INCLUDED_RP2_USB_H
#include "py/runtime.h"

void poll_cdc_interfaces(void);
void tud_cdc_rx_cb(uint8_t itf);
#endif
