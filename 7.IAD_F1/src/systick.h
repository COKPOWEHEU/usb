#ifndef _SYSTICK_H_
#define _SYSTICK_H_

#include <inttypes.h>
extern volatile uint32_t systick_internal;
inline uint32_t systick_ms(){return systick_internal;}

void systick_init();

#endif
