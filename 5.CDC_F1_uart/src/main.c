#include <stm32f1xx.h>
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
  SysClockMax();
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPAEN;

  GPIO_config(RLED); GPIO_config(GLED); GPIO_config(YLED);
  //GPIO_config(BTN1); GPIO_config(BTN2);
  
  USB_setup();
  __enable_irq();

  while(1){
    usb_class_poll();
    //GPO_T(YLED);
    //sleep(1000000);
  }
}
