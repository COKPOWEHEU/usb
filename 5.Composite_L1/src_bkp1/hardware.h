#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#define RLED B,8,1,GPIO_PP_VS
#define GLED B,9,1,GPIO_PP_VS
#define LBTN C,13,0,GPIO_INP_VS
#define JBTN B,5,0,GPIO_INP_VS

#include "pinmacro.h"

#define ENDP_VIRFAT_OUT	2
#define ENDP_VIRFAT_IN	1
#define ENDP_VIRFAT_SIZE 64



#endif
