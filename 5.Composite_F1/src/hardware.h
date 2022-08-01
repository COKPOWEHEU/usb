#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#define RLED  B,7,1,GPIO_PP50
#define GLED  B,6,1,GPIO_PP50
#define YLED  B,5,1,GPIO_PP50
#define BTN1  A,8,0,GPIO_HIZ
#define BTN2  A,9,0,GPIO_HIZ

#include "pinmacro.h"

#define ENDP_VIRFAT_OUT	2
#define ENDP_VIRFAT_IN	1
#define ENDP_VIRFAT_SIZE 64



#endif
