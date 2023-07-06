#ifndef __USB_CLASS_MSD__
#define __USB_CLASS_MSD__

#include "usb_lib.h"
#include "hardware.h"

char msd_ep0_in(config_pack_t *req, void **data, uint16_t *size);
char msd_ep0_out(config_pack_t *req, uint16_t offset, uint16_t rx_size);
void msd_init();
void msd_poll();

#endif
