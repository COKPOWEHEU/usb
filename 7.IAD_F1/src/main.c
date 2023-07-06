#include <stm32f1xx.h>
#include "hardware.h"
#include "pinmacro.h"
#include "usb_lib.h"
#include "clock.h"
#include "systick.h"

void __attribute__((weak)) _init(void){}
void __attribute__((weak)) SystemInit(void){}

void sleep(uint32_t time){
  while(time--)asm volatile("nop");
}

int main(void){
  SysClockMax();
  systick_init();
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;

  GPIO_config(RLED); GPIO_config(GLED);
  GPIO_config(USBR); GPIO_config(RESET); GPIO_config(BOOT0);
  
  GPO_OFF(RLED); GPO_OFF(GLED);
  GPO_OFF(RESET); GPO_OFF(BOOT0); GPO_OFF(USBR);
  
  USB_setup();
  __enable_irq();

  while(1){
    usb_class_poll();
  }
}
