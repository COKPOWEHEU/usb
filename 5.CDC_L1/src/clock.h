#ifndef __STM32L1xx_CLOCK__
#define __STM32L1xx_CLOCK__

// F_HSE = 8 MHz
// F_LSE = 32768 Hz
// F_HSI ~ 16 MHz
// F_LSI ~ 37 kHz

#define F_APB1 32000000
#define F_APB2 32000000
#define F_CPU  32000000

//system clock
#define CLOCK_HSI 0
#define CLOCK_HSE 1
#define CLOCK_FAIL_MSI -1
#define CLOCK_FAIL_HSI -2
#define CLOCK_FAIL_PLL -3

//RTC clock
#define CLOCK_LSE 0x10 //bitmask
#define CLOCK_LSI 3
//#define CLOCK_HSE 1
#define CLOCK_FAIL_LSI -4

#define RTC_LSE 1

void init32(uint8_t reinit){
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  PWR->CR |= PWR_CR_DBP;
  
  if(!reinit){
    //if(RCC->CSR & RCC_CSR_RTCEN)return;
  }
  
  RCC->CSR |= RCC_CSR_RTCRST;
  RCC->CSR &=~RCC_CSR_RTCRST;
  
#if RTC_LSE
  RCC->CSR |= RCC_CSR_LSEON; while(!(RCC->CSR & RCC_CSR_LSERDY)){}
#else
  RCC->CSR |= RCC_CSR_LSION; while(!(RCC->CSR & RCC_CSR_LSIRDY)){}
#endif
  
  RCC->CSR &=~RCC_CSR_RTCSEL;
#if RTC_LSE
  RCC->CSR |= RCC_CSR_RTCSEL_LSE; // NOCLOCK / LSI / LSE / HSE
#else
  RCC->CSR |= RCC_CSR_RTCSEL_LSI; // NOCLOCK / LSI / LSE / HSE
#endif
  RCC->CSR |= RCC_CSR_RTCEN;
  
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;
  RTC->ISR |= RTC_ISR_INIT;
  while(!(RTC->ISR & RTC_ISR_INITF)){}
  //LSE
  RTC->PRER &=~(RTC_PRER_PREDIV_A | RTC_PRER_PREDIV_S);
#if RTC_LSE
  RTC->PRER |= 255;
  RTC->PRER |= (127 << 16);
#else
  RTC->PRER |= 249;
  RTC->PRER |= (127 << 16);
  //RTC->PRER = (127 << 16) | 249; //LSI
#endif
  //RTC->TR &=~RTC_TR_SU;
  //RTC->DR = 0;
  //RTC->DR &=~RTC_DR_DU;
  
  
  RTC->ISR &=~RTC_ISR_INIT;
  RTC->WPR = 0xFF;
}

int8_t clock_HS(int8_t quartz){
  uint16_t i;
  uint32_t tmp;
  
  //clock by MSI
  tmp = RCC->CFGR;
  tmp &=~RCC_CFGR_SW;
  tmp |= RCC_CFGR_SW_MSI; // MSI / HSI / HSE / PLL
  RCC->CFGR = tmp;
  for(i=0;i<0x0FFF;i++){
    if((RCC->CFGR & RCC_CFGR_SWS)==RCC_CFGR_SWS_MSI){i=0x1FFF; break;}
  }
  if(i != 0x1FFF)return CLOCK_FAIL_MSI;
  
  //disable HSE, PLL
  RCC->CR &=~ RCC_CR_HSEON;
  RCC->CR &=~ RCC_CR_HSION;
  RCC->CR &=~ RCC_CR_HSEBYP;
  RCC->CR &=~ RCC_CR_PLLON;
  RCC->CR &=~ RCC_CR_RTCPRE;
  RCC->CR |= RCC_CR_RTCPRE_1; //0b00=2 MHz ; 0b01=4MHz ; 0b10=8MHz ; 0b11=16MHz
  
  //if user wants to HSE try it
  if(quartz){
    RCC->CR |= RCC_CR_HSEON;
    for(i=0;i<0x0FFF;i++){
      if(RCC->CR & RCC_CR_HSERDY){i=0x1FFF; break;}
    }
    if(i != 0x1FFF)quartz = 0; //HSE fail; try to HSI
  }
  
  //if user wants to HSI or we cant start HSE - try to start HSI
  if(!quartz){
    RCC->CR |= RCC_CR_HSION;
    for(i=0;i<0x0FFF;i++){
      if(RCC->CR & RCC_CR_HSIRDY){i=0x1FFF; break;}
    }
    if(i != 0x1FFF)return CLOCK_FAIL_HSI;
  }
  
  // configure flash
  FLASH->ACR |= FLASH_ACR_ACC64;
  FLASH->ACR |= FLASH_ACR_PRFTEN;
  FLASH->ACR |= FLASH_ACR_LATENCY;

  
  tmp = RCC->CFGR;
  tmp &=~ (RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL | RCC_CFGR_PLLDIV);
  
  if(quartz){
    tmp |= RCC_CFGR_PLLSRC;  //PLL src = HSE
    tmp |= RCC_CFGR_PLLMUL12;//PLLCLK = F_HSE * 12 = 96 MHz
    tmp |= RCC_CFGR_PLLDIV3; //PLLCLK = F_HSE * 12 / 3 = 32 MHz
  }else{
    tmp |= RCC_CFGR_PLLMUL6; //PLLCLK = F_HSI * 6 = 96 MHz
    tmp |= RCC_CFGR_PLLDIV3; //PLLCLK = F_HSI * 6 / 3 = 32 MHz
  }
  
  tmp &=~RCC_CFGR_HPRE;
  tmp |= RCC_CFGR_HPRE_DIV1;
  //AHB = PLLCLK/HPRE = 32 MHz
  tmp &=~RCC_CFGR_PPRE2;
  tmp |= RCC_CFGR_PPRE2_DIV1;
  //APB2 = HCLK/PPRE2 = 32 MHz
  tmp &=~RCC_CFGR_PPRE1;
  tmp |= RCC_CFGR_PPRE1_DIV1;
  //APB1 = HCLK/PPRE1 = 32 MHz
  RCC->CFGR = tmp;
  
  RCC->CR |= RCC_CR_PLLON;
  for(i=0;i<0x0FFF;i++){
    if(RCC->CR & RCC_CR_PLLRDY){i=0x1FFF; break;}
  }
  if(i != 0x1FFF)return CLOCK_FAIL_PLL; //can not enable PLL
  
  tmp = RCC->CFGR;
  tmp &=~RCC_CFGR_SW;
  tmp |= RCC_CFGR_SW_PLL; // MSI / HSI / HSE / PLL
  RCC->CFGR = tmp;
  for(i=0;i<0x0FFF;i++){
    if((RCC->CFGR & RCC_CFGR_SWS)==RCC_CFGR_SWS_PLL){i=0x1FFF; break;}
  }
  if(i != 0x1FFF)return CLOCK_FAIL_PLL;
  
  if(quartz)return CLOCK_HSE;
    else return CLOCK_HSI;
}

void clock_MS(){
  uint16_t i;
  uint32_t tmp;
  
  //clock by MSI
  tmp = RCC->CFGR;
  tmp &=~RCC_CFGR_SW;
  tmp |= RCC_CFGR_SW_MSI; // MSI / HSI / HSE / PLL
  RCC->CFGR = tmp;
  for(i=0;i<0x0FFF;i++){
    if((RCC->CFGR & RCC_CFGR_SWS)==RCC_CFGR_SWS_MSI){i=0x1FFF; break;}
  }
  if(i != 0x1FFF)return;;
  
  //disable HSE, PLL
  RCC->CR &=~ RCC_CR_HSEON;
  RCC->CR &=~ RCC_CR_HSION;
  RCC->CR &=~ RCC_CR_HSEBYP;
  RCC->CR &=~ RCC_CR_PLLON;
}

#endif
