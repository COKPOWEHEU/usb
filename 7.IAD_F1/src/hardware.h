#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#define RLED  B,5,1,GPIO_PP50
#define YLED  B,6,1,GPIO_PP50
#define GLED  B,7,1,GPIO_PP50
#define USB_DP	A,12,1,GPIO_HIZ

#define UART_TX A,9, GPIO_APP50
#define UART_RX A,10,GPIO_HIZ

#define USBR	B,2,0,GPIO_PP50
#define RESET   C,13,0,GPIO_PP50
#define BOOT0   A,8,1,GPIO_PP50
#define DTR		B,15,0,GPIO_PP50

#define jtag_disable() do{\
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;\
  AFIO->MAPR = (AFIO->MAPR &~AFIO_MAPR_SWJ_CFG) | AFIO_MAPR_SWJ_CFG_JTAGDISABLE;\
}while(0)

#include "pinmacro.h"
  
//------ USB -------------------
//PMA_size = 512
//EP0_size = 8

//Custom HID
// /dev/ttyACM0 - TTY
#define ENDP_TTY_IN		1
#define ENDP_TTY_OUT	1
#define ENDP_TTY_SIZE	32 //->64
// /dev/ttyACM1 - PROGR
#define ENDP_PROG_IN	2
#define ENDP_PROG_OUT	2
#define ENDP_PROG_SIZE	32 //->64
//UART1+2 - interrupt endpoint (placeholder)
#define ENDP_TTY_CTL	3
#define ENDP_PROG_CTL	4
#define ENDP_CTL_SIZE	8
//MSD
#define ENDP_MSD_IN		5
#define ENDP_MSD_OUT	5
#define ENDP_MSD_SIZE	64 //->128

#define interface_tty	0, 2
#define interface_progr	2, 2
#define interface_msd	4, 1
#define interface_count	5

#define ifnum(x) _marg1(x)
#define ifcnt(x) _marg2(x)

#endif
