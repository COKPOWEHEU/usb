#include "usb_lib.h"
#include <wchar.h>
#include "hardware.h"
#include "usb_hid.h"

#define INTR_NUM 1
#define INTR_SIZE 64

#define STD_DESCR_LANG 0
#define STD_DESCR_VEND 1
#define STD_DESCR_PROD 2
#define STD_DESCR_SN   3
#define STD_DESCR_CONF 4
#define STD_DESCR_IF   5

#define USB_VID 0x16C0
#define USB_PID 0x05DF

#define HIDREQ_GET_REPORT     1
#define HIDREQ_SET_REPORT     9


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

USB_ALIGN static const uint8_t USB_HIDDescriptor[] = {
  //keyboard
  USAGE_PAGE( USAGEPAGE_GENERIC ),//0x05, 0x01,
  USAGE( USAGE_KEYBOARD ), // 0x09, 0x06,
  COLLECTION( COLL_APPLICATION, // 0xA1, 0x01,
    REPORT_ID( 1 ), // 0x85, 0x01,
    USAGE_PAGE( USAGEPAGE_KEYBOARD ), // 0x05, 0x07,
    USAGE_MINMAX(224, 231), //0x19, 0xE0, 0x29, 0xE7,    
    LOGICAL_MINMAX(0, 1), //0x15, 0x00, 0x25, 0x01,
    REPORT_FMT(1, 8), //0x75, 0x01, 0x95, 0x08     
    INPUT_HID( HID_DATA | HID_VAR | HID_ABS ), // 0x81, 0x02,
     //reserved (???)
    REPORT_FMT(8, 1), // 0x75, 0x08, 0x95, 0x01,
    INPUT_HID(HID_CONST), // 0x81, 0x01,
              
    REPORT_FMT(1, 5),  // 0x75, 0x01, 0x95, 0x05,
    USAGE_PAGE( USAGEPAGE_LEDS ), // 0x05, 0x08,
    USAGE_MINMAX(1, 5), //0x19, 0x01, 0x29, 0x05,  
    OUTPUT_HID( HID_DATA | HID_VAR | HID_ABS ), // 0x91, 0x02,
    //выравнивание до 1 байта
    REPORT_FMT(3, 1), // 0x75, 0x03, 0x95, 0x01,
    OUTPUT_HID( HID_CONST ), // 0x91, 0x01,
    REPORT_FMT(8, 6),  // 0x75, 0x08, 0x95, 0x06,
    LOGICAL_MINMAX(0, 101), // 0x15, 0x00, 0x25, 0x65,         
    USAGE_PAGE( USAGEPAGE_KEYBOARD ), // 0x05, 0x07,
    USAGE_MINMAX(0, 101), // 0x19, 0x00, 0x29, 0x65,
    INPUT_HID( HID_DATA | HID_ARR ), // 0x81, 0x00,           
  )
  //touchscreen
  USAGE_PAGE( USAGEPAGE_DIGITIZER ), // 0x05, 0x0D,
  USAGE( USAGE_PEN ), // 0x09, 0x02,
  COLLECTION( COLL_APPLICATION, // 0xA1, 0x0x01,
    REPORT_ID( 2 ), //0x85, 0x02,
    USAGE( USAGE_FINGER ), // 0x09, 0x22,
    COLLECTION( COLL_PHISICAL, // 0xA1, 0x00,
      USAGE( USAGE_TOUCH ), // 0x09, 0x42,
      USAGE( USAGE_IN_RANGE ), // 0x09, 0x32,
      LOGICAL_MINMAX( 0, 1), // 0x15, 0x00, 0x25, 0x01,
      REPORT_FMT( 1, 2 ), // 0x75, 0x01, 0x95, 0x02,
      INPUT_HID( HID_VAR | HID_DATA | HID_ABS ), // 0x91, 0x02,
      REPORT_FMT( 1, 6 ), // 0x75, 0x01, 0x95, 0x06,
      INPUT_HID( HID_CONST ), // 0x81, 0x01,
                
      USAGE_PAGE( USAGEPAGE_GENERIC ), //0x05, 0x01,
      USAGE( USAGE_POINTER ), // 0x09, 0x01,
      COLLECTION( COLL_PHISICAL, // 0xA1, 0x00,         
        USAGE( USAGE_X ), // 0x09, 0x30,
        USAGE( USAGE_Y ), // 0x09, 0x31,
        LOGICAL_MINMAX16( 0, 10000 ), //0x16, 0x00, 0x00, 0x26, 0x10, 0x27,
        REPORT_FMT( 16, 2 ), // 0x75, 0x10, 0x95, 0x02,
        INPUT_HID( HID_VAR | HID_ABS | HID_DATA), // 0x91, 0x02,
      )
    )
  )
};

USB_ALIGN static const uint8_t USB_ConfigDescriptor[] = {
  ARRLEN34(
  ARRLEN1(
    bLENGTH, // bLength: Configuration Descriptor size
    USB_DESCR_CONFIG,    //bDescriptorType: Configuration
    wTOTALLENGTH, //wTotalLength
    1, // bNumInterfaces
    1, // bConfigurationValue: Configuration value
    0, // iConfiguration: Index of string descriptor describing the configuration
    0x80, // bmAttributes: bus powered
    0x32, // MaxPower 100 mA
  )
  ARRLEN1(
    bLENGTH, //bLength
    USB_DESCR_INTERFACE, //bDescriptorType
    0, //bInterfaceNumber
    0, // bAlternateSetting
    2, // bNumEndpoints
    HIDCLASS_HID, // bInterfaceClass: 
    HIDSUBCLASS_BOOT, // bInterfaceSubClass: 
    HIDPROTOCOL_KEYBOARD, // bInterfaceProtocol: 
    0x00, // iInterface
  )
  ARRLEN1(
    bLENGTH, //bLength
    USB_DESCR_HID, //bDescriptorType
    USB_U16(0x0110), //bcdHID
    0, //bCountryCode
    1, //bNumDescriptors
    USB_DESCR_HID_REPORT, //bDescriptorType
    USB_U16( sizeof(USB_HIDDescriptor) ), //wDescriptorLength
  )
  ARRLEN1(
    bLENGTH, //bLength
    USB_DESCR_ENDPOINT, //bDescriptorType
    INTR_NUM, //bEdnpointAddress
    USB_ENDP_INTR, //bmAttributes
    USB_U16( INTR_SIZE ), //MaxPacketSize
    10, //bInterval
  )
  ARRLEN1(
    bLENGTH, //bLength
    USB_DESCR_ENDPOINT, //bDescriptorType
    INTR_NUM | 0x80, //bEdnpointAddress
    USB_ENDP_INTR, //bmAttributes
    USB_U16( INTR_SIZE ), //MaxPacketSize
    10, //bInterval
  )
  )
};

USB_STRING(USB_StringLangDescriptor, u"\x0409"); //lang US
USB_STRING(USB_StringManufacturingDescriptor, u"COKPOWEHEU"); //Vendor
USB_STRING(USB_StringProdDescriptor, u"USB Keyboard + Mouse"); //Product
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

char usb_class_ep0_in(config_pack_t *req, void **data, uint16_t *size){
  if( req->bmRequestType == (USB_REQ_INTERFACE | 0x80) ){
    if( req->bRequest == GET_DESCRIPTOR ){
      if( req->wValue == HID_REPORT_DESCRIPTOR){
        *data = (void**)&USB_HIDDescriptor;
        *size = sizeof(USB_HIDDescriptor);
        return 1;
      }
    }
  }
  return 0;
}

static void intr_req(uint8_t epnum){
}

static void outp_req(uint8_t epnum){
  uint8_t buf[10];
  usb_ep_read(INTR_NUM, (void*)&buf);
  if(buf[1] & (1<<HID_KBDLED_CAPSLOCK))GPO_ON(RLED); else GPO_OFF(RLED);
  if(buf[1] & (1<<HID_KBDLED_NUMLOCK))GPO_ON(GLED); else GPO_OFF(GLED);
}

void usb_class_init(){
  usb_ep_init( INTR_NUM, USB_ENDP_INTR, INTR_SIZE, outp_req);
  usb_ep_init( INTR_NUM | 0x80, USB_ENDP_INTR, INTR_SIZE, intr_req);
}

struct{
  uint8_t report_id;
  union{
    uint8_t modifiers;
    struct{
      uint8_t lctrl:1;
      uint8_t lshift:1;
      uint8_t lalt:1;
      uint8_t lgui:1;
      uint8_t rctrl:1;
      uint8_t rshift:1;
      uint8_t ralt:1;
      uint8_t rgui:1;
    };
  };
  uint8_t reserved;
  uint8_t keys[6];
}USB_ALIGN __attribute__((packed)) report_kbd = {
  .report_id = 1,
  .modifiers = 0,
  .keys = {0,0,0,0,0,0},
};

struct{
  uint8_t report_id;
  union{
    uint8_t buttons;
    struct{
      uint8_t touch:1;
      uint8_t inrange:1;
      uint8_t reserved:6;
    };
  };
  uint16_t x;
  uint16_t y;
}USB_ALIGN __attribute__((packed)) report_tablet = {
  .report_id = 2,
  .buttons = 0,
  .x = 1000,
  .y = 1000,
};

void delay(uint32_t t){
  for(;t;t--)asm volatile("nop");
}

void usb_class_poll(){
  if(GPI_ON( LBTN )){
    while( GPI_ON(LBTN) ){}
    report_tablet.touch = 1;
    report_tablet.inrange = 1;
    report_tablet.x = 5000;
    report_tablet.y = 5000;
    usb_ep_write(INTR_NUM | 0x80, (void*)&report_tablet, sizeof(report_tablet));
    delay(1000000);
    report_tablet.buttons = 0;
    usb_ep_write(INTR_NUM | 0x80, (void*)&report_tablet, sizeof(report_tablet));
  }
  if(GPI_ON( JBTN )){
    while( GPI_ON(JBTN) ){}
    report_kbd.lshift = 1;
    report_kbd.keys[0] = 0x06;
    report_kbd.keys[1] = 0x12;
    report_kbd.keys[2] = 0x0E;
    report_kbd.keys[3] = 0x13;
    usb_ep_write(INTR_NUM | 0x80, (void*)&report_kbd, sizeof(report_kbd));
    delay(1000000);
    report_kbd.modifiers = report_kbd.keys[0] = report_kbd.keys[1] = report_kbd.keys[2] = report_kbd.keys[3] = 0;
    
    usb_ep_write(INTR_NUM | 0x80, (void*)&report_kbd, sizeof(report_kbd));
  }
}
