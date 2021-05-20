#include "usb_lib.h"
#include <wchar.h>

//TODO: debug
#include "hardware.h"
//#define UART_SIZE_PWR 8
//#include "uart.h"

#define ENDP_CTL_NUM 2
#define ENDP_CTL_SIZE 8

#define ENDP_DATA_NUM 1
#define ENDP_DATA_SIZE 64


#define STD_DESCR_LANG 0
#define STD_DESCR_VEND 1
#define STD_DESCR_PROD 2
#define STD_DESCR_SN   3
#define STD_DESCR_CONF 4
#define STD_DESCR_IF   5

#if 0
#define USB_VID 0x0483 //STmicroelectronics, чтобы под виндой можно было пользоваться существующим драйвером
#define USB_PID 0x5740 //STmicroelectronics
#else
#define USB_VID 0x03EB //AVR LUFA
#define USB_PID 0x2044 //AVR LUFA
#endif

static const uint8_t USB_DeviceDescriptor[] = {
  18,     // bLength
  0x01,   // bDescriptorType - Device descriptor
  USB_U16(0x0200), // bcdUSB
  2,//0,   // bDevice Class
  2,//0,   // bDevice SubClass
  0,   // bDevice Protocol
  USB_EP0_BUFSZ,   // bMaxPacketSize0
  USB_U16( USB_VID ), // idVendor
  USB_U16( USB_PID ), // idProduct
  USB_U16( 1 ), // bcdDevice_Ver
  STD_DESCR_VEND,   // iManufacturer
  STD_DESCR_PROD,   // iProduct
  STD_DESCR_SN,   // iSerialNumber
  1    // bNumConfigurations
};

static const uint8_t USB_DeviceQualifierDescriptor[] = {
  10,     //bLength
  0x06,   // bDescriptorType - Device qualifier
  USB_U16(0x0200), // bcdUSB
  0,   // bDeviceClass
  0,   // bDeviceSubClass
  0,   // bDeviceProtocol
  USB_EP0_BUFSZ,   // bMaxPacketSize0
  1,   // bNumConfigurations
  0x00    // Reserved
};

static const uint8_t USB_ConfigDescriptor[] = {
  0x09, // bLength: Configuration Descriptor size
  2,    //bDescriptorType: Configuration
  USB_U16(67), //wTotalLength
  2, // bNumInterfaces
  1, // bConfigurationValue: Configuration value
  0, // iConfiguration: Index of string descriptor describing the configuration
  0x80, // bmAttributes: bus powered
  0x32, // MaxPower 100 mA
  
  //*****************  CDC control interface **************
  0x09, // bLength: Interface Descriptor size
  0x04, // bDescriptorType:
  0x00, // bInterfaceNumber: Number of current Interface
  0x00, // bAlternateSetting: Alternate setting
  0x01, // bNumEndpoints
  0x02, // bInterfaceClass
  0x02, // bInterfaceSubClass
  0x00, // bInterfaceProtocol
  0,    // iInterface:
  //******* CDC functional descriptors ******************
      //Header
      0x05, //bLength
      0x24, //bDescriptorType = CS_INTERFACE
      0x00, //bDescriptorSubType = Header
      USB_U16( 0x0110 ), //bcd CDC
      //0x10, //bcdCDC - spec release number
      //0x01,
      //Call mamagement
      0x05, //bLength
      0x24, //bDescriptorType = CS_INTERFACE
      0x01, //bDescriptorSubType = Call management
      0x00, //bmCapabilities = D0+D1 //TODO:WTF?
      0x01, //bDataInterface = 1
      //ACM
      0x04, //bLength
      0x24, //bDescriptorType = CS_INTERFACE
      0x02, //bDescriptorSubType = Abstract Control Management
      0x02, //bmCapabilities
        //7-4 : reserved
        //3   : supports 'Network connection'
        //2   : supports 'Send break'
        //1   : supports 'Set line coding', 'set control line state', 'get line coding', 'serial state'
        //0   : supports 'Set comm feature', 'clear comm feature', 'get comm feature'
      //Union
      0x05, //bLength
      0x24, //bDescriptorType = CS_INTERFACE
      0x06, //bDescriptorSubType = Union
      0x00, //bMasterInterface = Communication class interface
      0x01, //bSlaveInterface0 = Data Class Interface
    //Endpoints
    0x07, //bLength
    0x05, //bDescriptorType = Endpoint
    ENDP_CTL_NUM | 0x80, //endpoint address
    0x03, //endpoint type = interrupt
    USB_U16( ENDP_CTL_SIZE ), //size
    0x10, //bInterval
  //*****************  CDC data interface **************
  0x09, //bLength
  0x04, //bDescriptorType
  0x01, //bInterfaceNumber
  0x00, //bAlternateSetting
  0x02, //bNumEndpoints
  0x0A, //bInterfaceClass = CDC
  0,//TODO WTF 0x02, //bInterfaceSubClass
  0x00, //bInterfaceProtocol
  0x00, //iInterace
    //Endpoints
    0x07, //bLength
    0x05, //bDescriptorType = Endpoint
    ENDP_DATA_NUM, // endpoint address
    0x02, //endpoint type = Bulk
    USB_U16( ENDP_DATA_SIZE ), //size
    0x00, //bInterval (ignored)
    //
    0x07, //bLength
    0x05, //bDescriptorType = Endpoint
    ENDP_DATA_NUM | 0x80, //endpoint address
    0x02, //endpoint type = Bulk
    USB_U16( ENDP_DATA_SIZE ), //size
    0x00, //bInterval (ignored)

};

USB_STRING(USB_StringLangDescriptor, u"\x0409"); //lang US
#if 0
USB_STRING(USB_StringManufacturingDescriptor, u"STMicroelectronics");
USB_STRING(USB_StringProdDescriptor, u"STM32 Virtual ComPort");
USB_STRING(USB_StringSerialDescriptor, u"00000000001A");
#else
USB_STRING(USB_StringManufacturingDescriptor, u"COKPOWEHEU");
USB_STRING(USB_StringProdDescriptor, u"USB test CDC");
USB_STRING(USB_StringSerialDescriptor, u"1");
#endif

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

volatile struct cdc_linecoding linecoding = {
  .baudrate = 9600,
  .stopbits = 0,
  .parity = 0,
  .wordsize = 8,
};

void usb_class_ep0_in(config_pack_t *req, void **data, uint16_t *size){
  if( (req->bmRequestType & 0x7F) == (USB_REQ_CLASS | USB_REQ_INTERFACE) ){
    if( req->bRequest == CDC_GET_LINE_CODING ){
      *data = &linecoding;
      *size = sizeof(linecoding);
    }
  }
}

void usb_class_ep0_out(config_pack_t *req){
  if( (req->bmRequestType & 0x7F) == (USB_REQ_CLASS | USB_REQ_INTERFACE) ){
    if( req->bRequest == CDC_SET_LINE_CODING ){
      if( ! usb_ep_ready(0) )return;
      usb_ep_read(0, &linecoding);
      
      if(linecoding.baudrate == 9600)GPO_ON(GLED); else GPO_OFF(GLED);
      if(linecoding.baudrate == 115200)GPO_ON(RLED); else GPO_OFF(RLED);
    }else if( req->bRequest == CDC_SET_CTRL_LINES ){
      //wValue bits:
      //  7-2 : reserved
      //  1   : RTS
      //  0   : DTR
    }
  }
}
#if 1
struct intr_serial_state_s{
  uint8_t bmRequestType;
  uint8_t bNotification;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
  union{
    uint16_t rawdata;
    struct{
      uint16_t bDCD:1;
      uint16_t bDSR:1;
      uint16_t bBreak:1;
      uint16_t bRingSignal:1;
      uint16_t bFramingErr:1;
      uint16_t bParityErr:1;
      uint16_t bOverrun:1;
      uint16_t reserved:9;
    };
  };
}__attribute__((packed));

struct intr_serial_state_s intr_serial_state = {
  .bmRequestType = 0xA1,
  .bNotification = 0x20,
  .wValue = 0,
  .wIndex = 0,
  .wLength = 2,
  .rawdata = 0x0003,
};

uint8_t *intr_buf = NULL;
uint16_t intr_size = 0;
static void cdc_ctl(uint8_t epnum){
  //if(! usb_ep_ready( ENDP_CTL_NUM | 0x80 ) )return;
  uint16_t left = intr_size;
  if(intr_size == 0)return;
  if(left > ENDP_CTL_SIZE)left = ENDP_CTL_SIZE;
  usb_ep_write( ENDP_CTL_NUM | 0x80, intr_buf, left);
  intr_size -= left;
  intr_buf += left;
}
#else
static void cdc_ctl(uint8_t epnum){
}
#endif

static void cdc_data_out(uint8_t epnum){
  //GPO_T(GLED);
  uint8_t buf[ ENDP_DATA_SIZE ];
  int len = usb_ep_read( ENDP_DATA_NUM, buf);
  if(len == 0)return;
  if(buf[0] == 'a')GPO_ON(GLED);
  if(buf[0] == 's')GPO_OFF(GLED);
  if(buf[0] == 'd')usb_ep_write( ENDP_DATA_NUM | 0x80, buf, 1 );
}


static void cdc_data_in(uint8_t epnum){
  //TODO
}

void usb_class_disable(){
  GPO_OFF(GLED);
  GPO_OFF(RLED);
}

void usb_class_init(){
  usb_ep_init( ENDP_CTL_NUM | 0x80, USB_ENDP_INTR, ENDP_CTL_SIZE, cdc_ctl);
  usb_ep_init( ENDP_DATA_NUM, USB_ENDP_BULK, ENDP_DATA_SIZE, cdc_data_out);
  usb_ep_init( ENDP_DATA_NUM | 0x80, USB_ENDP_BULK, ENDP_DATA_SIZE, cdc_data_in);

}

void usb_class_poll(){
  if(GPI_ON( LBTN )){
    while( GPI_ON(LBTN) ){}
    intr_serial_state.bDSR = 1;
    intr_buf = (uint8_t*)&intr_serial_state;
    intr_size = sizeof(intr_serial_state);
    cdc_ctl( ENDP_CTL_NUM );
  }
  if(GPI_ON( JBTN )){
    while( GPI_ON(JBTN) ){}
    intr_serial_state.bDSR = 0;
    intr_buf = (uint8_t*)&intr_serial_state;
    intr_size = sizeof(intr_serial_state);
    cdc_ctl( ENDP_CTL_NUM );
  }
  /*if( GPI_ON( LBTN ) ){
    while( GPI_ON( LBTN) ){}
    uint8_t ch = 'a';
    usb_ep_write(ENDP_DATA_NUM, &ch, 1);
  }
  if( GPI_ON( JBTN) ){
    while( GPI_ON( JBTN )){}
    intr_buf = (uint8_t*)&intr_serial_state;
    intr_size = sizeof(intr_serial_state);
    cdc_ctl( ENDP_CTL_NUM );
  }*/
}

//---------- CDC -----------------
