#ifndef _PROGRAMMER_H_
#define _PROGRAMMER_H_

#include "usb_lib.h"
#include "hardware.h"

char programmer_ep0_in(config_pack_t *req, void **data, uint16_t *size);
char programmer_ep0_out(config_pack_t *req, uint16_t offset, uint16_t rx_size);
void programmer_init();
void programmer_poll();

#endif
