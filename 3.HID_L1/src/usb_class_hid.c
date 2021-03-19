#include "usb_lib.h"
#include <wchar.h>
#include "hardware.h"

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

#define HIDCLASS_HID          3
#define HIDSUBCLASS_NONE      0
#define HIDSUBCLASS_BOOT      1
#define HIDPROTOCOL_NONE      0
#define HIDPROTOCOL_KEYBOARD  1
#define HIDPROTOCOL_MOUSE     2

static const uint8_t USB_DeviceDescriptor[] = {
  ARRLEN1(
  bLENGTH,     // bLength
  USB_DESCR_DEVICE,   // bDescriptorType - Device descriptor
  USB_U16(0x0200), // bcdUSB
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

static const uint8_t USB_DeviceQualifierDescriptor[] = {
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

static const uint8_t USB_HIDDescriptor[] = {
    0x05, 0x01, // Usage Page (Generic Desktop)           
    0x09, 0x02, // Usage (Mouse)                          
    0xA1, 0x01, // Collection (Application)               
    0x09, 0x01, //  Usage (Pointer)                       
    0xA1, 0x00, //  Collection (Physical)                 
    0x85, 0x01, //   Report ID
    0x05, 0x09, //      Usage Page (Buttons)              
    0x19, 0x01, //      Usage Minimum (01)                
    0x29, 0x03, //      Usage Maximum (03)                
    0x15, 0x00, //      Logical Minimum (0)               
    0x25, 0x01, //      Logical Maximum (0)               
    0x95, 0x03, //      Report Count (3)                  
    0x75, 0x01, //      Report Size (1)                   
    0x81, 0x02, //      Input (Data, Variable, Absolute)  
    0x95, 0x01, //      Report Count (1)                  
    0x75, 0x05, //      Report Size (5)                   
    0x81, 0x01, //      Input (Constant)    ;5 bit padding
    0x05, 0x01, //      Usage Page (Generic Desktop)      
    0x09, 0x30, //      Usage (X)                         
    0x09, 0x31, //      Usage (Y)                         
    0x15, 0x81, //      Logical Minimum (-127)            
    0x25, 0x7F, //      Logical Maximum (127)             
    0x75, 0x08, //      Report Size (8)                   
    0x95, 0x02, //      Report Count (2)                  
    0x81, 0x06, //      Input (Data, Variable, Relative)  
    0xC0, 0xC0, // End Collection,End Collection          
//
    0x09, 0x06, //		Usage (Keyboard)        
    0xA1, 0x01, //		Collection (Application)
    0x85, 0x02, //   Report ID
    0x05, 0x07, //  	Usage (Key codes)                 
    0x19, 0xE0, //      Usage Minimum (224)               
    0x29, 0xE7, //      Usage Maximum (231)               
    0x15, 0x00, //      Logical Minimum (0)               
    0x25, 0x01, //      Logical Maximum (1)               
    0x75, 0x01, //      Report Size (1)                   
    0x95, 0x08, //      Report Count (8)                  
    0x81, 0x02, //      Input (Data, Variable, Absolute)  
    0x95, 0x01, //      Report Count (1)                  
    0x75, 0x08, //      Report Size (8)                   
    0x81, 0x01, //      Input (Constant)    ;5 bit padding
    0x95, 0x05, //      Report Count (5)                  
    0x75, 0x01, //      Report Size (1)                   
    0x05, 0x08, //      Usage Page (Page# for LEDs)       
    0x19, 0x01, //      Usage Minimum (01)                
    0x29, 0x05, //      Usage Maximum (05)                
    0x91, 0x02, //      Output (Data, Variable, Absolute) 
    0x95, 0x01, //      Report Count (1)                  
    0x75, 0x03, //      Report Size (3)                   
    0x91, 0x01, //      Output (Constant)                 
    0x95, 0x06, //      Report Count (1)                  
    0x75, 0x08, //      Report Size (3)                   
    0x15, 0x00, //      Logical Minimum (0)               
    0x25, 0x65, //      Logical Maximum (101)             
    0x05, 0x07, //  	Usage (Key codes)                 
    0x19, 0x00, //      Usage Minimum (00)                
    0x29, 0x65, //      Usage Maximum (101)               
    0x81, 0x00, //      Input (Data, Array)               
    0xC0        // 		End Collection,End Collection     
};

static const uint8_t USB_ConfigDescriptor[] = {
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
    1, // bNumEndpoints
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
    INTR_NUM | 0x80, //bEdnpointAddress
    USB_ENDP_INTR, //bmAttributes
    USB_U16( INTR_SIZE ), //MaxPacketSize
    100, //bInterval
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

uint8_t kbd_buf[9] = {2, 0, 0, 0, 0, 0, 0, 0, 0};

static void intr_req(uint8_t epnum){
  if(kbd_buf[3] == 0)return;
  kbd_buf[3] = 0;
  usb_ep_write(INTR_NUM | 0x80, kbd_buf, sizeof(kbd_buf));
}

void usb_class_init(){
  usb_ep_init( INTR_NUM | 0x80, USB_ENDP_INTR, INTR_SIZE, intr_req);
}

/*
 * MOUSE
 * buf[0]: 1 - report ID
 * buf[1]: bit2 - middle button, bit1 - right, bit0 - left
 * buf[2]: move X
 * buf[3]: move Y
 * buf[4]: wheel
 */

/*
 * Keyboard buffer:
 * buf[0]: 2 - report ID
 * buf[1]: MOD - клавиши модификаторы
 * buf[2]: reserved
 * buf[3]..buf[8] - keycodes 1..6
 */
void usb_class_poll(){
  uint8_t data[8];
  if(GPI_ON( LBTN )){
    while( GPI_ON(LBTN) ){}
    data[0] = 1;
    data[1] = 0x00;
    data[2] = (uint8_t)100;
    data[3] = (uint8_t)100;
    data[4] = 0;
    usb_ep_write(INTR_NUM | 0x80, data, 5);
  }
  if(GPI_ON( JBTN )){
    while( GPI_ON(JBTN) ){}
    kbd_buf[3] = 10;
    usb_ep_write(INTR_NUM | 0x80, kbd_buf, sizeof(kbd_buf));
  }
}
