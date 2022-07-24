#include "usb_lib.h"
#include <wchar.h>
#include "hardware.h"

#define ENDP_DATA_IN 2
#define ENDP_DATA_OUT 1
#define ENDP_DATA_SIZE 64

#define ENDP_CTL_NUM  3
#define ENDP_CTL_SIZE 8

#define STD_DESCR_LANG 0
#define STD_DESCR_VEND 1
#define STD_DESCR_PROD 2
#define STD_DESCR_SN   3

#define USB_VID 0x16C0
#define USB_PID 0x05DF

#define DEVCLASS_CDC          0x02
#define CDCSUBCLASS_ACM       0x02
#define CDCCLASS_DATA         0x0A
#define CDCPROTOCOL_UNDEF     0x00
#define CDCPROTOCOL_VENDOR    0xFF

USB_ALIGN static const uint8_t USB_DeviceDescriptor[] = {
  ARRLEN1(
  bLENGTH,     // bLength
  USB_DESCR_DEVICE,   // bDescriptorType - Device descriptor
  USB_U16(0x0110), // bcdUSB
  0,   // bDevice Class
  0,   // bDevice SubClass
  0,   // bDevice Protocol
  USB_EP0_BUFSZ,   // bMaxPacketSize0
  USB_U16( USB_VID ), // idVendor
  USB_U16( USB_PID ), // idProduct
  USB_U16( 1 ), // bcdDevice_Ver
  STD_DESCR_VEND,   // iManufacturer
  STD_DESCR_PROD,   // iProduct
  STD_DESCR_SN,   // iSerialNumber
  1    // bNumConfigurations
  )
};

USB_ALIGN static const uint8_t USB_DeviceQualifierDescriptor[] = {
  ARRLEN1(
  bLENGTH,     //bLength
  USB_DESCR_QUALIFIER,   // bDescriptorType - Device qualifier
  USB_U16(0x0200), // bcdUSB
  0,   // bDeviceClass
  0,   // bDeviceSubClass
  0,   // bDeviceProtocol
  USB_EP0_BUFSZ,   // bMaxPacketSize0
  1,   // bNumConfigurations
  0x00    // Reserved
  )
};

USB_ALIGN static const uint8_t USB_ConfigDescriptor[] = {
  ARRLEN34(
  ARRLEN1(
    bLENGTH, // bLength: Configuration Descriptor size
    USB_DESCR_CONFIG,    //bDescriptorType: Configuration
    wTOTALLENGTH, //wTotalLength
    2, // bNumInterfaces
    1, // bConfigurationValue: Configuration value
    0, // iConfiguration: Index of string descriptor describing the configuration
    0x80, // bmAttributes: bus powered
    0x32, // MaxPower 100 mA
  )
  ARRLEN1(//CDC control Interface
    bLENGTH, // bLength
    USB_DESCR_INTERFACE, // bDescriptorType
    0, // bInterfaceNumber
    0, // bAlternateSetting
    1, // bNumEndpoints
    DEVCLASS_CDC, // bInterfaceClass: 
    CDCSUBCLASS_ACM, // bInterfaceSubClass: 
    CDCPROTOCOL_UNDEF, // bInterfaceProtocol: 
    0x00, // iInterface
  )
    ARRLEN1(//Header
      bLENGTH, //bLength
      USB_DESCR_CS_INTERFACE, //bDescriptorType
      0, //bDescriptorSubType
      USB_U16(0x0110), //bcdCDC
    )
    ARRLEN1( //Call mamagement
      bLENGTH,
      USB_DESCR_CS_INTERFACE,
      0x01, //Call mamagement
      0x00, //TODO: bmCapabilities D0+D1
      1, //bDataInterface
    )
    ARRLEN1( //ACM
      bLENGTH,
      USB_DESCR_CS_INTERFACE,
      2, //bDescriptorSubType = Abstract Control Management
      0x02, //bmCapabilities
        //7-4 : reserved
        //3   : supports 'Network connection'
        //2   : supports 'Send break'
        //1   : supports 'Set line coding', 'set control line state', 'get line coding', 'serial state'
        //0   : supports 'Set comm feature', 'clear comm feature', 'get comm feature'
    )
    ARRLEN1( //Union
      bLENGTH,
      USB_DESCR_CS_INTERFACE,
      6, //bDescriptorSubType = Union
      0, //bMasterInterface = Communication class interface
      1, //bSlaveInterface0 = Data Class Interface
    )
    ARRLEN1(//Endpoint descriptor
      bLENGTH, //bLength
      USB_DESCR_ENDPOINT, //bDescriptorType
      ENDP_CTL_NUM | 0x80,
      USB_ENDP_INTR,
      USB_U16(ENDP_CTL_SIZE),
      100, //bInterval - частота опроса, для изохронных всегда 1
    )
    
  ARRLEN1( //CDC data interface
    bLENGTH,
    USB_DESCR_INTERFACE,
    1, //bInterfaceNumber
    0, //bAlternateSetting
    2, //bNumEndpoints
    CDCCLASS_DATA, //bInterfaceClass
    0, //bInterfaceSubClass
    0, //bInterfaceProtocol
    0, //iInterface
  )
    ARRLEN1(//Endpoint descriptor
      bLENGTH, //bLength
      USB_DESCR_ENDPOINT, //bDescriptorType
      ENDP_DATA_OUT,
      USB_ENDP_BULK,
      USB_U16(ENDP_DATA_SIZE),
      0, //bInterval - частота опроса, для изохронных всегда 1
    )
    ARRLEN1(//Endpoint descriptor
      bLENGTH, //bLength
      USB_DESCR_ENDPOINT, //bDescriptorType
      ENDP_DATA_IN | 0x80,
      USB_ENDP_BULK,
      USB_U16(ENDP_DATA_SIZE),
      0, //bInterval - частота опроса, для изохронных всегда 1
    )
  )
};

USB_STRING(USB_StringLangDescriptor, u"\x0409"); //lang US
USB_STRING(USB_StringManufacturingDescriptor, u"COKPOWEHEU"); //Vendor
USB_STRING(USB_StringProdDescriptor, u"USB CDC"); //Product
USB_STRING(USB_StringSerialDescriptor, u"1"); //Serial (BCD)

void usb_class_get_std_descr(uint16_t descr, const void **data, uint16_t *size){
  switch(descr & 0xFF00){
    case DEVICE_DESCRIPTOR:
      *data = &USB_DeviceDescriptor;
      *size = sizeof(USB_DeviceDescriptor);
      break;
    case CONFIGURATION_DESCRIPTOR:
      *data = &USB_ConfigDescriptor;
      *size = sizeof(USB_ConfigDescriptor);
      break;
    case DEVICE_QUALIFIER_DESCRIPTOR:
      *data = &USB_DeviceQualifierDescriptor;
      *size = USB_DeviceQualifierDescriptor[0];
      break;
    case STRING_DESCRIPTOR:
      switch(descr & 0xFF){
        case STD_DESCR_LANG:
          *data = &USB_StringLangDescriptor;
          break;
        case STD_DESCR_VEND:
          *data = &USB_StringManufacturingDescriptor;
          break;
        case STD_DESCR_PROD:
          *data = &USB_StringProdDescriptor;
          break;
        case STD_DESCR_SN:
          *data = &USB_StringSerialDescriptor;
          break;
        default:
          return;
      }
      *size = ((uint8_t*)*data)[0]; //data->bLength
      break;
    default:
      break;
  }
}

static void sleep(uint32_t time){
  while(time--)asm volatile("nop");
}


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

USB_ALIGN volatile struct cdc_linecoding linecoding = {
  .baudrate = 9600,
  .stopbits = 0,
  .parity = 0,
  .wordsize = 8,
};

char usb_class_ep0_in(config_pack_t *req, void **data, uint16_t *size){
  if( (req->bmRequestType & 0x7F) == (USB_REQ_CLASS | USB_REQ_INTERFACE) ){
    if( req->bRequest == CDC_GET_LINE_CODING ){
      *data = (void*)&linecoding;
      *size = sizeof(linecoding);
      return 1;
    }
  }
  return 0;
}

char usb_class_ep0_out(config_pack_t *req, uint16_t offset, uint16_t rx_size){
  if( (req->bmRequestType & 0x7F) == (USB_REQ_CLASS | USB_REQ_INTERFACE) ){
    if( req->bRequest == CDC_SET_LINE_CODING ){
      if(rx_size == 0)return 1;
      
      usb_ep_read(0, (uint16_t*)&linecoding);
      //if(linecoding.baudrate == 9600)GPO_ON(GLED); else GPO_OFF(GLED);
      //if(linecoding.baudrate == 115200)GPO_ON(RLED); else GPO_OFF(RLED);
      return 1;
    }else if( req->bRequest == CDC_SET_CTRL_LINES ){
      //wValue bits:
      //  7-2 : reserved
      //  1   : RTS
      //  0   : DTR
    }
  }
  return 0;
}

void ctl_callback(uint8_t epnum){
  
}

void data_out_callback(uint8_t epnum){
  USB_ALIGN uint8_t buf[ ENDP_DATA_SIZE ];
  int len = usb_ep_read_double( ENDP_DATA_OUT, (uint16_t*)buf);
  if(len == 0)return;
  if(buf[0] == 'a')GPO_ON(GLED);
  if(buf[0] == 's')GPO_OFF(GLED);
}

void fmtu8(uint8_t x, char *buf){
  buf[0] = x/100 + '0';
  buf[1] = (x/10)%10 + '0';
  buf[2] = x % 10 + '0';
  buf[3] = '\n';
  buf[4] = '\r';
  buf[5] = 0;
}

char test(){
  static uint8_t x = 0;
  USB_ALIGN uint8_t ch[6];
  fmtu8(x, (char*)ch);
  usb_ep_write_double( ENDP_DATA_IN | 0x80, (uint16_t*)ch, sizeof(ch)-1);
  x++;
  return 1;
}

void data_in_callback(uint8_t epnum){
  static uint8_t cnt = 0;
  if(cnt >= 2){cnt = 0; return;}
  test();
  cnt++;
}

void usb_class_init(){
  usb_ep_init( ENDP_CTL_NUM | 0x80, USB_ENDP_INTR, ENDP_CTL_SIZE, ctl_callback );
  
  usb_ep_init_double( ENDP_DATA_IN | 0x80, USB_ENDP_BULK, ENDP_DATA_SIZE, data_in_callback );
  usb_ep_init_double( ENDP_DATA_OUT, USB_ENDP_BULK, ENDP_DATA_SIZE, data_out_callback );
  
}

void usb_class_poll(){
  if(GPI_ON( LBTN )){
    while( GPI_ON(LBTN) ){}
  }
  if(GPI_ON( JBTN )){
    while( GPI_ON(JBTN) ){}
    USB_ALIGN uint8_t ch = 'a';
    usb_ep_write_double(ENDP_DATA_IN | 0x80, (uint16_t*)&ch, 1);
    GPO_T(RLED);
  }
  
  GPO_T(RLED);
  sleep(5000000);
}
