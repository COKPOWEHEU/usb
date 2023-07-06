#include <stm32f1xx.h>
#include "systick.h"

volatile uint32_t systick_internal = 0;

void systick_init(){
  //SysTick->LOAD = F_AHB / 8 / 1000;
  SysTick->LOAD = SysTick->CALIB;
  SysTick->CTRL = SysTick_CTRL_TICKINT | SysTick_CTRL_ENABLE;
}

void SysTick_Handler(){
  systick_internal++;
}
