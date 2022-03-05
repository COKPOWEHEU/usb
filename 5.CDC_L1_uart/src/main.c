#include <stm32l151xc.h>
#include "pinmacro.h"
#include "hardware.h"
#include "usb_lib.h"
#include "clock.h"

void __attribute__((weak)) _init(void){}
void __attribute__((weak)) SystemInit(void){}

void sleep(uint32_t time){
  while(time--)asm volatile("nop");
}

int main(void){
  clock_HS(1);
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN;
  GPIO_config(GLED); GPIO_config(RLED);
  GPIO_config(LBTN); GPIO_config(JBTN);
  
  USB_setup();
  __enable_irq();
  while (1){
    usb_class_poll();
    //sleep(100000);
  }
  return 0;
}

