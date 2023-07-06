#ifndef __UART_H__
#define __UART_H__

#if 1==0
ver.2021.06.20

модуль для работы с UART в stm32f103

макронастройки:
  UART1_EN, UART2_EN, UART3_EN - какие UARTы включать
  USART - если используется только один UART, достаточно указать его номер. Его же потом можно использовать в других макросах
  F_APB1, F_APB2 - тактовая частота соответствующей шины
  
макросы и функции:
  void UART_init(int num, uint32_t baud)
  
  void UART_putc(int num, uint8_t data)
  void UART_write(int num, uint8_t data, uint8_t len)
  void UART_puts(int num, char* str)
  uint8_t UART_tx_count(int num)
  
  uint8_t UART_getc(int num)
  void UART_read(int num, uint8_t data, uint8_t len)
  void UART_gets(int num, char* str, uint8_t len)
  uint8_t UART_scan(int num)
  uint8_t UART_rx_count(int num)
где
  num - номер UART (1 - 3)
  data - байт данных для приема / передачи
  str - строка для приема / передачи
  len - (максимальное) количество байт для приема / передачи
#endif
//TODO: проверить чтение
//TODO: настроить ремап портов

//#include <stm32f10x.h>
#include "pinmacro.h"

#ifndef UART_SIZE_PWR
  #define UART_SIZE_PWR 6
#endif

#if (USART==1)
  #define UART1_EN
#elif (USART==2)
  #define UART2_EN
#elif (USART==3)
  #define UART3_EN
#endif

#if (!defined(UART1_EN)) && (!defined(UART2_EN)) && (!defined(UART3_EN))
  #error define either UART1_EN of UART2_EN or UART3_EN
#endif

#ifndef UART1_REMAP
  #define UART1_TX A,9 ,1,GPIO_APP50
  #define UART1_RX A,10,1,GPIO_HIZ
#else
  #define UART1_TX B,6 ,1,GPIO_APP50
  #define UART1_RX B,7 ,1,GPIO_HIZ
#endif

#define UART2_TX A,2 ,1,GPIO_APP50
#define UART2_RX A,3 ,1,GPIO_HIZ

#define UART3_TX B,10,1,GPIO_APP50
#define UART3_RX B,11,1,GPIO_HIZ

#define UART_SIZE (1<<UART_SIZE_PWR)
#define UART_MASK (UART_SIZE-1)

typedef volatile struct{
  volatile uint8_t st,en;
  volatile uint8_t arr[UART_SIZE];
}uart_buffer;

uint8_t uart_buf_size(uart_buffer *buf){
  return ((buf->st - buf->en) & UART_MASK);
}

uint8_t uart_buf_read(uart_buffer *buf){
  uint8_t res;
  if(uart_buf_size(buf) == 0)return 0;
  res = buf->arr[buf->st];
  buf->st++;
  buf->st &= UART_MASK;
  return res;
}

void uart_buf_write(uart_buffer *buf, uint8_t dat){
  if(uart_buf_size(buf)!=1){
    buf->arr[buf->en]=dat;
    buf->en++; buf->en &= UART_MASK;
  }
}

#define UART_PIN(num,dir) UART##num##_##dir
#define UART_buf(num,dir) uart##num##_##dir
#define UART(num) USART##num
#define UART_IRQ(num) USART##num##_IRQn

#define _UART_rx_count(num)	((UART_SIZE - uart_buf_size(&UART_buf(num,rx))) & UART_MASK)
#define _UART_tx_count(num)  (UART_MASK - uart_buf_size(&UART_buf(num,tx)))
#define _UART_getc(num)  	uart_buf_read(&UART_buf(num,rx))
#define _UART_scan(num)      (UART_buf(num,rx).arr[UART_buf(num,rx).st])
#define _UART_write(num, data, len) UART##num##_write(data, len)
#define _UART_puts(num, str) UART##num##_puts(str)
#define _UART_read(num, data, len) UART##num##_read(data, len)
#define _UART_gets(num, str, len) UART##num##_gets(str, len)

#define UART_rx_count(num)	_UART_rx_count(num)
#define UART_tx_count(num)  _UART_tx_count(num)
#define UART_getc(num)  	_UART_getc(num)
#define UART_scan(num)      _UART_scan(num)
#define UART_write(num, data, len) _UART_write(num, data, len)
#define UART_puts(num, str) _UART_puts(num, str)
#define UART_read(num, data, len) _UART_read(num, data, len)
#define UART_gets(num, str, len) _UART_gets(num, str, len)

#define UART_putc(num, data) do{\
  uart_buf_write(&UART_buf(num,tx), data);\
  UART(num)->CR1 |= USART_CR1_TXEIE;\
}while(0)


#define UART_speed(num, baud) do{ \
  if(num == 1){ \
    USART1->BRR = F_APB2 / (baud); \
  }else{ \
    UART(num)->BRR = F_APB1 / (baud);\
  } \
}while(0)

#define UART_init(num, baud) do{\
  GPIO_config( UART_PIN(num,RX) ); \
  GPIO_config( UART_PIN(num,TX) ); \
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;\
  if(num == 1){\
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;\
  }else if(num == 2){ \
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;\
  }else{ \
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;\
  }\
  UART_speed(num, baud); \
  UART(num)->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;\
  UART(num)->CR2 = 0;\
  UART(num)->CR3 = 0;\
  UART(num)->GTPR = 0;\
  UART_buf(num,rx).st=0; UART_buf(num,rx).en=0; UART_buf(num,tx).st=0; UART_buf(num,tx).en=0;\
  NVIC_EnableIRQ( UART_IRQ(num) );\
}while(0)

///////////////////////////////////////////////////////////////////////////////////////////////
//             UART_1
///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef UART1_EN
static uart_buffer uart1_rx;
static uart_buffer uart1_tx;

void USART1_IRQHandler(void){
  if( USART1->SR & USART_SR_RXNE ){
    uint8_t temp = USART1->DR;
    uart_buf_write(&uart1_rx, temp);
  }else if( USART1->SR & USART_SR_TXE ){
    if(uart_buf_size(&uart1_tx) != 0)USART1->DR = uart_buf_read(&uart1_tx);
      else USART1->CR1 &=~ USART_CR1_TXEIE;
  }
}

void UART1_write(uint8_t *data, uint8_t len){
  while(len--)UART_putc(1, *(data++));
}

void UART1_puts(char *str){
  while(str[0] != 0)UART_putc(1, *(str++));
}

void UART1_read(uint8_t *data, uint8_t len){
  while(len--){
    while(UART_rx_count(1) == 0){}
    *(data++) = UART_getc(1);
  }
}

void UART1_gets(char *str, uint8_t len){
  while(len--){
    while(UART_rx_count(1) == 0){}
    str[0] = UART_getc(1);
    if(str[0] == 0 || str[0] == 13)break;
    str++;
  }
  if(str[0] != 0){
    if(len < 3)str[0] = 0;
      else{ str[0] = 0x0A; str[1] = 0x0D; str[2] = 0; }
  }
}
#endif


///////////////////////////////////////////////////////////////////////////////////////////////
//             UART_2
///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef UART2_EN
static uart_buffer uart2_rx;
static uart_buffer uart2_tx;

void USART2_IRQHandler(void){
  if( USART2->SR & USART_SR_RXNE ){
    uint8_t temp = USART2->DR;
    uart_buf_write(&uart2_rx, temp);
  }else if( USART2->SR & USART_SR_TXE ){
    if(uart_buf_size(&uart2_tx) != 0)USART2->DR = uart_buf_read(&uart2_tx);
      else USART2->CR1 &=~ USART_CR1_TXEIE;
  }
}

void UART2_write(uint8_t *data, uint8_t len){
  while(len--)UART_putc(2, *(data++));
}

void UART2_puts(char *str){
  while(str[0] != 0)UART_putc(2, *(str++));
}

void UART2_read(uint8_t *data, uint8_t len){
  while(len--){
    while(UART_rx_count(2) == 0){}
    *(data++) = UART_getc(2);
  }
}

void UART2_gets(char *str, uint8_t len){
  while(len--){
    while(UART_rx_count(2) == 0){}
    str[0] = UART_getc(2);
    if(str[0] == 0 || str[0] == 13)break;
    str++;
  }
  if(str[0] != 0){
    if(len < 3)str[0] = 0;
      else{ str[0] = 0x0A; str[1] = 0x0D; str[2] = 0; }
  }
}
#endif


///////////////////////////////////////////////////////////////////////////////////////////////
//             UART_3
///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef UART3_EN
static uart_buffer uart3_rx;
static uart_buffer uart3_tx;

void USART3_IRQHandler(void){
  if( USART3->SR & USART_SR_RXNE ){
    uint8_t temp = USART3->DR;
    uart_buf_write(&uart3_rx, temp);
  }else if( USART3->SR & USART_SR_TXE ){
    if(uart_buf_size(&uart3_tx) != 0)USART3->DR = uart_buf_read(&uart3_tx);
      else USART3->CR1 &=~ USART_CR1_TXEIE;
  }
}

void UART3_write(uint8_t *data, uint8_t len){
  while(len--)UART_putc(3, *(data++));
}

void UART3_puts(char *str){
  while(str[0] != 0)UART_putc(3, *(str++));
}

void UART_read(uint8_t *data, uint8_t len){
  while(len--){
    while(UART_rx_count(3) == 0){}
    *(data++) = UART_getc(3);
  }
}

void UART3_gets(char *str, uint8_t len){
  while(len--){
    while(UART_rx_count(3) == 0){}
    str[0] = UART_getc(3);
    if(str[0] == 0 || str[0] == 13)break;
    str++;
  }
  if(str[0] != 0){
    if(len < 3)str[0] = 0;
      else{ str[0] = 0x0A; str[1] = 0x0D; str[2] = 0; }
  }
}
#endif


#endif
