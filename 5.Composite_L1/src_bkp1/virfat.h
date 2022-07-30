#ifndef __VIRFAT_H__
#define __VIRFAT_H__
#include <inttypes.h>

#define VIRFAT_DATE_DD_MM_YYYY	28, 7, 2022 //date of create / last access
#define VIRFAT_VOLNAME		"USB_TEST   " //disk Label (sometimes ignored by OS)

void virfat_init();
char virfat_ep0_in(config_pack_t *req, void **data, uint16_t *size);
char virfat_ep0_out(config_pack_t *req, uint16_t offset, uint16_t rx_size);
void virfat_poll();

extern volatile uint8_t virfat_hse_enabled;
extern volatile uint16_t virfat_mic_freq_Hz;

#endif
