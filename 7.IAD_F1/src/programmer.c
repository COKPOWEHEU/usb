#include "programmer.h"
#define F_APB1 36000000
#define F_APB2 72000000
#define UART_SIZE_PWR 8
#define USART 1
#include "uart.h"
#include "systick.h"

#ifndef NULL
  #define NULL ((void*)0)
#endif

#define CDC_SEND_ENCAPSULATED 0x00
#define CDC_GET_ENCAPSULATED  0x01
#define CDC_SET_COMM_FEATURE  0x02
#define CDC_GET_COMM_FEATURE  0x03
#define CDC_CLR_COMM_FEATURE  0x04
#define CDC_SET_LINE_CODING   0x20
#define CDC_GET_LINE_CODING   0x21
#define CDC_SET_CTRL_LINES    0x22
#define CDC_SEND_BREAK        0x23

struct cdc_linecoding{
  uint32_t baudrate;
  uint8_t stopbits; //0=1bit, 1=1.5bits, 2=2bits
  uint8_t parity; //0=none, 1=odd, 2=even, 3=mark (WTF?), 4=space (WTF?)
  uint8_t wordsize; //length of data word: 5,6,7,8 or 16 bits
}__attribute__((packed));

USB_ALIGN volatile struct cdc_linecoding tty_cfg = {
  .baudrate = 9600,
  .stopbits = 1,
  .parity = 0,
  .wordsize = 8,
};
USB_ALIGN volatile struct cdc_linecoding progr_cfg = {
  .baudrate = 9600,
  .stopbits = 1,
  .parity = 0,
  .wordsize = 8,
};

#define TTYM_NORMAL	0
#define TTYM_PROGR	1
#define TTYM_CTRL	2
volatile char ttymode = TTYM_NORMAL;
volatile uint32_t timeout_ms = 0;
#define SPEED_CTRL	50
#define TIMEOUT_MS 	10000
#define timeout_reset() do{timeout_ms = systick_ms() + TIMEOUT_MS;}while(0)

void uart_ctrl( uint8_t *buf, int size);

char programmer_ep0_in(config_pack_t *req, void **data, uint16_t *size){
  if( (req->bmRequestType & 0x7F) == (USB_REQ_CLASS | USB_REQ_INTERFACE) ){
    if( req->bRequest == CDC_GET_LINE_CODING ){
      if( req->wIndex == ifnum(interface_tty) ){
        *data = (void*)&tty_cfg;
        *size = sizeof(tty_cfg);
        return 1;
      }else if( req->wIndex == ifnum(interface_progr) ){
        *data = (void*)&progr_cfg;
        *size = sizeof(progr_cfg);
        return 1;
      }
    }
  }
  return 0;
}

void uart_cfg( volatile struct cdc_linecoding *cfg ){
  UART_speed( USART, cfg->baudrate );
  //stopbits
  
  //parity
  if(cfg->parity == 0){ //none
    USART1->CR1 &=~USART_CR1_PCE;
    USART1->CR1 &=~USART_CR1_M;
  }else if(cfg->parity == 1){ //odd
    USART1->CR1 |= USART_CR1_M;
    USART1->CR1 |= USART_CR1_PCE;
    USART1->CR1 |= USART_CR1_PS;
  }else if(cfg->parity == 2){ //even
    USART1->CR1 |= USART_CR1_M;
    USART1->CR1 |= USART_CR1_PCE;
    USART1->CR1 &=~USART_CR1_PS;
  }
}

char programmer_ep0_out(config_pack_t *req, uint16_t offset, uint16_t rx_size){
  if( (req->bmRequestType & 0x7F) == (USB_REQ_CLASS | USB_REQ_INTERFACE) ){
    if( req->bRequest == CDC_SET_LINE_CODING ){
      if(rx_size == 0)return 1;
      if( req->wIndex == ifnum(interface_tty) ){
        usb_ep_read(0, (void*)&tty_cfg);
        if(ttymode == TTYM_NORMAL){
          uart_cfg( &tty_cfg );
        }
        return 1;
      }else if( req->wIndex == ifnum(interface_progr) ){
        usb_ep_read(0, (void*)&progr_cfg);
        if( progr_cfg.baudrate == SPEED_CTRL ){
          ttymode = TTYM_CTRL;
          timeout_reset();
        }else if( ttymode == TTYM_CTRL ){
          ttymode = TTYM_PROGR;
          uart_cfg( &progr_cfg );
          timeout_reset();
        }
        return 1;
      }
    }else if( req->bRequest == CDC_SET_CTRL_LINES ){
      if( req->wValue & (1<<0) )GPO_ON(DTR); else GPO_OFF(DTR);
      //wValue bits:
      //  7-2 : reserved
      //  1   : RTS
      //  0   : DTR
    }
  }
  return 0;
}

static void cdc_out(uint8_t epnum){
  USB_ALIGN uint8_t buf[ENDP_TTY_SIZE];
  int size;
  if(ttymode == TTYM_NORMAL){
    epnum = ENDP_TTY_OUT;
    if(usb_ep_ready( ENDP_PROG_OUT ))usb_ep_read( ENDP_PROG_OUT, (void*)buf );
    if(!usb_ep_ready(ENDP_TTY_OUT ))return;
  }else{
    epnum = ENDP_PROG_OUT;
    if(usb_ep_ready( ENDP_TTY_OUT ))usb_ep_read( ENDP_TTY_OUT, (void*)buf );
    if(!usb_ep_ready(ENDP_PROG_OUT))return;
    timeout_reset();
  }
  
  if(ttymode != TTYM_CTRL){
    if( UART_tx_count( USART ) > (ENDP_TTY_SIZE + 10) ){
      size = usb_ep_read( epnum, (void*)buf );
      UART_write( USART, buf, size );
    }
  }else{
    size = usb_ep_read( ENDP_PROG_OUT, (void*)buf );
    uart_ctrl( buf, size );
  }
}

static char initflag = 0;

void programmer_init(){
  UART_init(USART, 9600);
  //UART_puts(USART, "Test\n\r");
  ttymode = TTYM_NORMAL;
  usb_ep_init( ENDP_TTY_CTL  | 0x80, USB_ENDP_INTR, ENDP_CTL_SIZE,  NULL );
  usb_ep_init( ENDP_TTY_IN   | 0x80, USB_ENDP_BULK, ENDP_TTY_SIZE,  NULL );
  usb_ep_init( ENDP_TTY_OUT,         USB_ENDP_BULK, ENDP_TTY_SIZE,  NULL );

  usb_ep_init( ENDP_PROG_CTL | 0x80, USB_ENDP_INTR, ENDP_CTL_SIZE,  NULL );
  usb_ep_init( ENDP_PROG_IN  | 0x80, USB_ENDP_BULK, ENDP_PROG_SIZE, NULL );
  usb_ep_init( ENDP_PROG_OUT,        USB_ENDP_BULK, ENDP_PROG_SIZE, NULL );
  initflag = 1;
}

void programmer_poll(){
  if(!initflag)return;
  USB_ALIGN uint8_t buf[ENDP_TTY_SIZE];
  int size;
  
  size = UART_rx_count( USART );
  if(size > 0){
    if( size > ENDP_TTY_SIZE )size = ENDP_TTY_SIZE;
    
    if( ttymode == TTYM_NORMAL ){
      if(usb_ep_ready( ENDP_TTY_IN | 0x80 )){
        UART_read( USART, buf, size );
        usb_ep_write( ENDP_TTY_IN | 0x80, (uint16_t*)buf, size );
      }
    }else if( ttymode == TTYM_PROGR ){
      if(usb_ep_ready( ENDP_PROG_IN | 0x80 )){
        UART_read( USART, buf, size);
        usb_ep_write( ENDP_PROG_IN | 0x80, (uint16_t*)buf, size );
      }
    }else{ //if ttymode==TTYM_CTRL - ignore
      UART_read( USART, buf, size );
    }
  }
  
  cdc_out( ENDP_TTY_OUT );
  
  //таймаут
  if( ttymode != TTYM_NORMAL ){
    if( systick_ms() > timeout_ms ){
      ttymode = TTYM_NORMAL;
      uart_cfg( &tty_cfg );
    }
  }
  
  //mode
  if( ttymode == TTYM_NORMAL ){		//green
    GPO_ON(GLED); GPO_OFF(RLED);
  }else if( ttymode == TTYM_PROGR ){//red
    GPO_ON(RLED); GPO_OFF(GLED);
  }else{							//green+red
    GPO_ON(RLED); GPO_ON(GLED);
  }
}

/*
#undef RESET
#define RESET RLED
#undef BOOT0
#define BOOT0 GLED
//*/
#define CTRL_RST	(1<<0)
#define CTRL_BOOT0	(1<<1)
#define CTRL_USB	(1<<2)
//control sequence
void uart_ctrl( uint8_t *buf, int size){
  //GPO_T( GLED );
  uint8_t val = 0;
  if( GPI_ON(RESET) )val |= CTRL_RST;
  if( GPI_ON(BOOT0) )val |= CTRL_BOOT0;
  if( GPI_ON(USBR ) )val |= CTRL_USB;
  for(int i=0; i<size; i++){
    if(buf[i] == 'z')timeout_ms = 0;
    if(buf[i] == 'R')val |= CTRL_RST;
    if(buf[i] == 'r')val &=~CTRL_RST;
    if(buf[i] == 'B')val |= CTRL_BOOT0;
    if(buf[i] == 'b')val &=~CTRL_BOOT0;
    if(buf[i] == 'U')val |= CTRL_USB;
    if(buf[i] == 'u')val &=~CTRL_USB;
  }
  if( val & CTRL_RST )	GPO_ON(RESET); else GPO_OFF(RESET);
  if( val & CTRL_BOOT0)	GPO_ON(BOOT0); else GPO_OFF(BOOT0);
  if( val & CTRL_USB)	GPO_ON(USBR);  else GPO_OFF(USBR);
}
