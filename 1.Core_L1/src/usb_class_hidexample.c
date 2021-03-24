#include "usb_lib.h"
#include <wchar.h>

#include "hardware.h"

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

struct dataexchange_t{
  uint8_t rled;
  uint8_t gled;
}hid_data;

static const uint8_t USB_HIDDescriptor[] = {
    0x06, 0x00, 0xff,                   // USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                         // USAGE (Vendor Usage 1)
    0xa1, 0x01,                         // COLLECTION (Application)
    0x15, 0x00,                         //    LOGICAL_MINIMUM (0)  // min. значение для данных
    0x26, 0xff, 0x00,                   //    LOGICAL_MAXIMUM (255)// max. значение для данных, 255 тут не случайно, а чтобы уложиться в 1 байт
    0x75, 0x08,                         //    REPORT_SIZE (8)      // информация передается порциями, это размер одного "репорта" 8 бит
    0x95, sizeof(struct dataexchange_t),//    REPORT_COUNT         // количество порций
    0x09, 0x00,                         //    USAGE (Undefined)
    0xb2, 0x02, 0x01,                   //    FEATURE (Data,Var,Abs,Buf)
    0xc0                                // END_COLLECTION
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
    0, // bNumEndpoints
    HIDCLASS_HID, // bInterfaceClass: 
    HIDSUBCLASS_NONE, // bInterfaceSubClass: 
    HIDPROTOCOL_NONE, // bInterfaceProtocol: 
    0x00, // iInterface
  )
  ARRLEN1(
    bLENGTH, //bLength
    USB_DESCR_HID, //bDescriptorType
    USB_U16(0x0101), //bcdHID
    0, //bCountryCode
    1, //bNumDescriptors
    USB_DESCR_HID_REPORT, //bDescriptorType
    USB_U16( sizeof(USB_HIDDescriptor) ), //wDescriptorLength
  )
  )
};


USB_STRING(USB_StringLangDescriptor, u"\x0409"); //lang US
USB_STRING(USB_StringManufacturingDescriptor, u"COKPOWEHEU"); //Vendor
USB_STRING(USB_StringProdDescriptor, u"USB test"); //Product
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
  static uint8_t num = 0;
  if( req->bmRequestType == (USB_REQ_CLASS | USB_REQ_INTERFACE | 0x80) ){
    if( req->bRequest == HIDREQ_GET_REPORT ){
      
      if(GPI_ON(JBTN))hid_data.rled = 1; else hid_data.rled = 0;
      hid_data.gled = num;
      num++;
      *data = &hid_data;
      *size = sizeof(hid_data);
      return 1;
    }
  }
  return 0;
}

char usb_class_ep0_out(config_pack_t *req, uint16_t offset, uint16_t rx_size){
  if(rx_size == 0)return 1; //прочитан только запрос - не интересно
  //анализом запроса не заморачиваемя. Считаем, что ничего кроме нужного не придет
  if(rx_size != sizeof(hid_data))return 0; //защита если вдруг придет мусор
  usb_ep_read(0, (void*)&hid_data);
  
  if(hid_data.rled)GPO_ON(RLED); else GPO_OFF(RLED);
  if(hid_data.gled)GPO_ON(GLED); else GPO_OFF(GLED);
  
  return 1;
}

void usb_class_disconnect(){
  GPO_OFF(GLED);
  GPO_OFF(RLED);
}
