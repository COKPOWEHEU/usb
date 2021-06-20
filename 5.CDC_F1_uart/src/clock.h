#ifndef __STM32F1xx_CLOCK__
#define __STM32F1xx_CLOCK__

#define SCME_OK   0
#define SCME_HSE  1
#define SCME_PLL  2
#define SCME_SW   3

#define F_APB1 36000000
#define F_APB2 72000000
#define F_CPU 72000000
//F_HSE = 8 MHz
int8_t SysClockMax(){
  int i;
  uint32_t tmp;
  RCC->CR &=~RCC_CR_HSEON;
  RCC->CR &=~RCC_CR_HSEBYP;
  RCC->CR |= RCC_CR_HSEON;
  for(i=0;i<0x0FFF;i++){
    if(RCC->CR & RCC_CR_HSERDY){i=0x1FFF; break;}
  }
  if(i != 0x1FFF)return SCME_HSE; //can not start HSE
  FLASH->ACR &=~FLASH_ACR_PRFTBE;
  FLASH->ACR |= FLASH_ACR_PRFTBE;

  FLASH->ACR &=~FLASH_ACR_LATENCY;
  FLASH->ACR |= FLASH_ACR_LATENCY_2;
  
  tmp = RCC->CFGR;
  tmp &=~(RCC_CFGR_PLLMULL  | RCC_CFGR_PLLSRC);
  tmp |= (RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLSRC);
  //PLLCLK = F_HSE * 9 = 72 MHz
  //we will configure CFGR_SW to PLL --> SYSCLK = PLLCLK = 72 MHz
  tmp &=~RCC_CFGR_HPRE;
  tmp |= RCC_CFGR_HPRE_DIV1;
  //AHB prescaler = 1 --> HCLK = SYSCLK/HPRE = 72 MHz
  tmp &=~RCC_CFGR_PPRE2;
  tmp |= RCC_CFGR_PPRE2_DIV1;
  //APB2 prescaler = 1 --> PCLK2 = HCLK/PPRE2 = 72 MHz
  tmp &=~RCC_CFGR_PPRE1;
  tmp |= RCC_CFGR_PPRE1_DIV2;
  //APB1 prescaler = 2 --> PCLK1 = HCLK/PPRE1 = 36 MHz
  RCC->CFGR = tmp;
  
  RCC->CR |= RCC_CR_PLLON;
  for(i=0;i<0x0FFF;i++){
    if(RCC->CR & RCC_CR_PLLRDY){i=0x1FFF; break;}
  }
  if(i != 0x1FFF)return SCME_PLL; //can not enable PLL
  tmp = RCC->CFGR;
  tmp &=~RCC_CFGR_SW;
  tmp |= RCC_CFGR_SW_PLL;
  RCC->CFGR = tmp;
  for(i=0;i<0x0FFF;i++){
    if((RCC->CFGR & RCC_CFGR_SWS)==RCC_CFGR_SWS_PLL){i=0x1FFF; break;}
  }
  if(i != 0x1FFF)return SCME_SW;
  return SCME_OK;
}

#endif
