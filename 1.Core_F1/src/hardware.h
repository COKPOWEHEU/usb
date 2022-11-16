#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#define RLED  B,7,1,GPIO_PP50
#define GLED  B,6,1,GPIO_PP50
#define YLED  B,5,1,GPIO_PP50
#define BTN1  A,8,0,GPIO_HIZ
#define BTN2  A,9,0,GPIO_HIZ //используется для программирования по UART
//#define USB_PULLUP A,10,1,GPIO_PP50 //так разведена плата
#define USB_DP	A,12,1,GPIO_HIZ

#include "pinmacro.h"

#endif
