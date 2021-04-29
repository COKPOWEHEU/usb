#ifndef __PINMACRO_H__
#define __PINMACRO_H__

//last modified 2019.09.20 - add PM_BITMASK

#if 1==0
  #define LED A,1,1,GPIO_PP_VS //PA0, GPIO_push-pull, very high speed, active level high (1)
  GPIO_config( LED ); //configure
  
  GPI_PUP( LED ); //enable pull-up
  GPI_PDN( LED ); //enable pull-down
  GPI_HIZ( LED ); //disable pull-up / pull-down
  
  GPO_ON( LED ); //switch ON
  GPO_OFF( LED ); //switch OFF
  GPO_T( LED ); //toggle
  
  GPI_ON( LED ) //logical level on LED is active (high in this example)
  GPI_OFF( LED ) //logical level on LED is inactive (low in this example)
  
  PM_BITMASK( ADC1->SQR5, ADC_SQR5_SQ1, channel ); //in variable [reg] clear bits by [mask] and set by [val]
#endif

#define PM_BITMASK( reg, mask, val ) do{\
    reg = (reg &~ mask) | ((mask &~(mask<<1))*val); \
  }while(0)


#define GPIO_MODER_MASK 0b11
#define GPIO_INP  0b00
#define GPIO_PP   0b01
#define GPIO_ALT  0b10
#define GPIO_ANA  0b11

#define GPIO_LS   0b0000
#define GPIO_MS   0b0100
#define GPIO_HS   0b1000
#define GPIO_VS   0b1100

#define GPIO_OD   0x0100

#define GPIO_INP_LS (GPIO_INP | GPIO_LS)
#define GPIO_INP_MS (GPIO_INP | GPIO_MS)
#define GPIO_INP_HS (GPIO_INP | GPIO_HS)
#define GPIO_INP_VS (GPIO_INP | GPIO_VS)

#define GPIO_PP_LS (GPIO_PP | GPIO_LS)
#define GPIO_PP_MS (GPIO_PP | GPIO_MS)
#define GPIO_PP_HS (GPIO_PP | GPIO_HS)
#define GPIO_PP_VS (GPIO_PP | GPIO_VS)

#define GPIO_ALT_LS (GPIO_ALT | GPIO_LS)
#define GPIO_ALT_MS (GPIO_ALT | GPIO_MS)
#define GPIO_ALT_HS (GPIO_ALT | GPIO_HS)
#define GPIO_ALT_VS (GPIO_ALT | GPIO_VS)

#define GPIO_ANA_LS (GPIO_ANA | GPIO_LS)
#define GPIO_ANA_MS (GPIO_ANA | GPIO_MS)
#define GPIO_ANA_HS (GPIO_ANA | GPIO_HS)
#define GPIO_ANA_VS (GPIO_ANA | GPIO_VS)

//костыль к замечательным CMSIS от ST, где в даташите AFRH:AFRL, а в файле AFR[2]
#define AFRH AFR[1]
#define AFRL AFR[0]


#define concat2(a,b,...)  a##b
#define concat3(a,b,c,...)  a#b#c

#define _marg1(a,...)  a
#define _marg2(a,b,...)  b
#define _marg3(a,b,c,...)  c
#define _marg4(a,b,c,d,...)  d
#define marg1(x) _marg1(x)
#define marg2(x) _marg2(x)
#define marg3(x) _marg3(x)
#define marg4(x) _marg4(x)
#define _GPIO(port) GPIO##port
#define GPIO(x) _GPIO(x)

#define GPIO_config(port) \
  do{\
    uint32_t temp = GPIO(_marg1(port))->MODER; \
    temp &=~ (0b11 << (_marg2(port)*2)); \
    temp |= ((_marg4(port) & 0b11) << (_marg2(port)*2)); \
    GPIO(_marg1(port))->MODER = temp; \
    \
    temp = GPIO(_marg1(port))->OSPEEDR; \
    temp &=~ (0b11 << (_marg2(port)*2)); \
    temp |= (((_marg4(port)>>2) & 0b11) << (_marg2(port)*2)); \
    GPIO(_marg1(port))->OSPEEDR = temp; \
    \
    if(_marg4(port) & GPIO_OD)GPIO(_marg1(port))->OTYPER |= (1<<_marg2(port));\
    \
    if((_marg4(port) & GPIO_MODER_MASK) == GPIO_ALT){\
      if(_marg2(port) > 7){ \
        temp = GPIO(_marg1(port))->AFRH; \
        temp &=~ ((AF_MASK>>4) << (((_marg2(port)-8)&7)*4)); \
        temp |= (((_marg4(port) & AF_MASK)>>4) << (((_marg2(port)-8)&7)*4)); \
        GPIO(_marg1(port))->AFRH = temp; \
      }else{ \
        temp = GPIO(_marg1(port))->AFRL; \
        temp &=~ ((AF_MASK>>4) << ((_marg2(port)&7)*4)); \
        temp |= (((_marg4(port) & AF_MASK)>>4) << ((_marg2(port)&7)*4)); \
        GPIO(_marg1(port))->AFRL = temp; \
      } \
    }\
  }while(0)
  
#define GPIO_manual(port, mode) \
  do{\
    uint32_t temp = GPIO(_marg1(port))->MODER; \
    temp &=~ (0b11 << (_marg2(port)*2)); \
    temp |= ((mode & 0b11) << (_marg2(port)*2)); \
    GPIO(_marg1(port))->MODER = temp; \
    \
    temp = GPIO(_marg1(port))->OSPEEDR; \
    temp &=~ (0b11 << (_marg2(port)*2)); \
    temp |= (((mode>>2) & 0b11) << (_marg2(port)*2)); \
    GPIO(_marg1(port))->OSPEEDR = temp; \
    \
    if(mode & GPIO_OD)GPIO(_marg1(port))->OTYPER |= (1<<_marg2(port));\
    \
    if((mode & GPIO_MODER_MASK) == GPIO_ALT){\
      if(_marg2(port) > 7){ \
        temp = GPIO(_marg1(port))->AFRH; \
        temp &=~ ((AF_MASK>>4) << (((_marg2(port)-8)&7)*4)); \
        temp |= (((mode & AF_MASK)>>4) << (((_marg2(port)-8)&7)*4)); \
        GPIO(_marg1(port))->AFRH = temp; \
      }else{ \
        temp = GPIO(_marg1(port))->AFRL; \
        temp &=~ ((AF_MASK>>4) << ((_marg2(port)&7)*4)); \
        temp |= (((mode & AF_MASK)>>4) << ((_marg2(port)&7)*4)); \
        GPIO(_marg1(port))->AFRL = temp; \
      } \
    }\
  }while(0)
  
#define GPI_PUP(port)\
  do{\
    uint32_t temp = GPIO(_marg1(port))->PUPDR; \
    temp &=~ (0b11 << (_marg2(port)*2)); \
    temp |=  (0b01 << (_marg2(port)*2)); \
    GPIO(_marg1(port))->PUPDR = temp; \
  }while(0)
  
#define GPI_PDN(port)\
  do{\
    uint32_t temp = GPIO(_marg1(port))->PUPDR; \
    temp &=~ (0b11 << (_marg2(port)*2)); \
    temp |=  (0b10 << (_marg2(port)*2)); \
    GPIO(_marg1(port))->PUPDR = temp; \
  }while(0)
  
#define GPI_HIZ(port)\
  do{\
    uint32_t temp = GPIO(_marg1(port))->PUPDR; \
    temp &=~ (0b11 << (_marg2(port)*2)); \
    GPIO(_marg1(port))->PUPDR = temp; \
  }while(0)

#define GPO_0(port) \
  do{ \
    GPIO(_marg1(port))->BSRR = (1<<_marg2(port)<<16); \
  }while(0)

#define GPO_1(port) \
  do{ \
    GPIO(_marg1(port))->BSRR = (1<<_marg2(port)); \
  }while(0)
  
#define GPO_ON(port) \
  do{ \
    GPIO(_marg1(port))->BSRR = (1<<_marg2(port)<<((1-_marg3(port))*16)); \
  }while(0)
  
#define GPO_OFF(port) \
  do{ \
    GPIO(_marg1(port))->BSRR = (1<<_marg2(port)<<((_marg3(port))*16)); \
  }while(0)
  
#define GPO_T(port) \
  do{ \
    if(GPIO(_marg1(port))->ODR & (1<<_marg2(port))){ \
      GPIO(_marg1(port))->BSRR = (1<<_marg2(port)<<16); \
    }else{ \
      GPIO(_marg1(port))->BSRR = (1<<_marg2(port)); \
    } \
    /*GPIO(_marg1(port))->ODR ^= (1<<_marg2(port));*/ \
  }while(0)
  
#define GPI_ON(port)  ((GPIO(_marg1(port))->IDR & (1<<_marg2(port)))==(_marg3(port)<<_marg2(port)))
#define GPI_OFF(port) ((GPIO(_marg1(port))->IDR & (1<<_marg2(port)))!=(_marg3(port)<<_marg2(port)))

//***********************************************************************************************
//***********************************************************************************************
//**   Alternative functions
//***********************************************************************************************
//***********************************************************************************************

#define AF_MASK 0xF0

#define AF_SYSTEM 0x00 //MCO, JTAG, ...
#define AF_TIM2 0x10
#define AF_TIM3 0x20
#define AF_TIM4 0x20
#define AF_TIM5 0x20
#define AF_TIM9 0x30
#define AF_TIM10 0x30
#define AF_TIM10 0x30
#define AF_I2C1 0x40
#define AF_I2C2 0x40
#define AF_SPI1 0x50
#define AF_SPI2 0x50
#define AF_SPI3 0x60
#define AF_UART1 0x70
#define AF_UART2 0x70
#define AF_UART3 0x70
#define AF_UART4 0x80
#define AF_UART5 0x80
//#define AF_9 0x90 //NOT AVAIBLE (tim12-14, CAN, ...)
#define AF_USB 0xA0
#define AF_LCD 0xB0
#define AF_FSMC 0xC0
//#define AF_13 0xD0 //NOT AVAIBLE (DCMI)
#define AF_RI 0xE0
#define AF_EVENTOUT 0xF0
  
  

#endif
