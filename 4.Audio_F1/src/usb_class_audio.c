#include "usb_lib.h"
#include <wchar.h>
#include "hardware.h"
#include "usb_audio.h"

#define ENDP_DATA_NUM 1
#define ENDP_IN_NUM   2
#define ENDP_DATA_SIZE 64

#define STD_DESCR_LANG 0
#define STD_DESCR_VEND 1
#define STD_DESCR_PROD 2
#define STD_DESCR_SN   3

#define USB_VID 0x16C0
#define USB_PID 0x05DF


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

#define F_SAMPLE 16000 //количество сэмплов в секунду

static const uint8_t USB_ConfigDescriptor[] = {
  ARRLEN34(
  ARRLEN1(
    bLENGTH, // bLength: Configuration Descriptor size
    USB_DESCR_CONFIG,    //bDescriptorType: Configuration
    wTOTALLENGTH, //wTotalLength
    3, // bNumInterfaces
    1, // bConfigurationValue: Configuration value
    0, // iConfiguration: Index of string descriptor describing the configuration
    0x80, // bmAttributes: bus powered
    0x32, // MaxPower 100 mA
  )
  ARRLEN1(//0: Audio control Interface
    bLENGTH, // bLength
    USB_DESCR_INTERFACE, // bDescriptorType
    0, // bInterfaceNumber
    0, // bAlternateSetting
    0, // bNumEndpoints (если испольуется Interrupt endpoint, может быть 1)
    USB_CLASS_AUDIO, // bInterfaceClass: 
    USB_SUBCLASS_AUDIOCONTROL, // bInterfaceSubClass: 
    0x00, // bInterfaceProtocol: 
    0x00, // iInterface
  )
    ARRLEN67(//AC interface
      ARRLEN1(//AC interface header
        bLENGTH, //bLength
        USB_DESCR_CS_INTERFACE, //bDescriptorType
        1, //bDescriptorSubType
        USB_U16(0x0100), //bcdADC //AudioDeviceClass серийный номер
        wTOTALLENGTH, //wTotalLength
        2, //bInCollection //количество интерфейсов в коллекции
        1, //bInterfaceNr(1), //массив (список) номеров интерфейсов в коллекции
        2,////bInterfaceNr(2), ...
      )
      ARRLEN1(//1. AC Input terminal
        bLENGTH, //bLength
        USB_DESCR_CS_INTERFACE, //bDescriptorType
        USBAUDIO_IF_TERM_IN, //bDescriptorSubType
        1, //bTerminalID
        USB_U16( USBAUDIO_TERMINAL_USB ),//USB_U16( USBAUDIO_TERMINAL_MIC ), //wTerminalTypeЧто это вообще такое (а вариантов немало!)
        0, //bAssocTerminal привязка выходного терминала для создания пары. Не используем
        1, //bNrChannels
        USB_U16( 0 ), //wChannelConfig //к чему именно подключены каналы
        0, //iChannelNames
        0, //iTerminal
      )
      ARRLEN1(//2. AC Input terminal
        bLENGTH, //bLength
        USB_DESCR_CS_INTERFACE, //bDescriptorType
        USBAUDIO_IF_TERM_IN, //bDescriptorSubType
        2, //bTerminalID
        USB_U16( USBAUDIO_TERMINAL_MIC ),//USB_U16( USBAUDIO_TERMINAL_MIC ), //wTerminalTypeЧто это вообще такое (а вариантов немало!)
        0, //bAssocTerminal привязка выходного терминала для создания пары. Не используем
        1, //bNrChannels
        USB_U16( 0 ), //wChannelConfig //к чему именно подключены каналы
        0, //iChannelNames
        0, //iTerminal
      )
      ARRLEN1(//5. AC Output Terminal
        bLENGTH, //bLength
        USB_DESCR_CS_INTERFACE, //bDescriptorType
        USBAUDIO_IF_TERM_OUT, //bDescriptorSubType
        5, //bTerminalID
        USB_U16( USBAUDIO_TERMINAL_SPEAKER ),//USB_U16( USBAUDIO_TERMINAL_USB ), //wTerminalType:speaker
        0, //bAssocTerminal
        3, //bSourceID  <-------------------------------------------
        0, //iTerminal
      )
      ARRLEN1(//6. AC Output Terminal
        bLENGTH, //bLength
        USB_DESCR_CS_INTERFACE, //bDescriptorType
        USBAUDIO_IF_TERM_OUT, //bDescriptorSubType
        6, //bTerminalID
        USB_U16( USBAUDIO_TERMINAL_USB ),//USB_U16( USBAUDIO_TERMINAL_USB ), //wTerminalType:speaker
        0, //bAssocTerminal
        4, //bSourceID  <-------------------------------------------
        0, //iTerminal
      )
      ARRLEN1(//3. AC Feature Unit
        bLENGTH, //bLength
        USB_DESCR_CS_INTERFACE, //bDescriptorType
        USBAUDIO_IF_FEATURE, //bDescriptorSubType
        3, //UnitID
        1, //bSourceID  <---------------------------------------------
        1, //bControlSize //размер одного элемента в массиве
        //bmaControls чем именно можно управлять
          USBAUDIO_FEATURE_MUTE, //Channel(0)
          USBAUDIO_FEATURE_NONE, //Channel(1) канал 1 - Mute
          //нужно описать оба канала?
        0, //iFeature
      )
      ARRLEN1(//4. AC Feature Unit
        bLENGTH, //bLength
        USB_DESCR_CS_INTERFACE, //bDescriptorType
        USBAUDIO_IF_FEATURE, //bDescriptorSubType
        4, //UnitID
        2, //bSourceID  <---------------------------------------------
        1, //bControlSize //размер одного элемента в массиве
        //bmaControls чем именно можно управлять
          USBAUDIO_FEATURE_MUTE, //Channel(0)
          USBAUDIO_FEATURE_NONE, //Channel(1) канал 1 - Mute
          //нужно описать оба канала?
        0, //iFeature
      )
    )
    
  ARRLEN1(//1 Audio Streaming Interface
    bLENGTH, //bLength
    USB_DESCR_INTERFACE, //bDescriptorType
    1, //bInterfaceNumber
    0, //bAlternateSetting
    0, //bNumEndpoints
    USB_CLASS_AUDIO, //bInterfaceClass
    USB_SUBCLASS_AUDIOSTREAMING, //bInterfaceSubClass
    0, //bInterfaceProtocol
    0, //iInterface
  )
  ARRLEN1(//1alt Audio Streaming Interface (alternative)
    bLENGTH, //bLength
    USB_DESCR_INTERFACE, //bDescriptorType
    1, //bInterfaceNumber
    1, //bAlternateSetting
    1, //bNumEndpoints
    USB_CLASS_AUDIO, //bInterfaceClass
    USB_SUBCLASS_AUDIOSTREAMING, //bInterfaceSubClass
    0, //bInterfaceProtocol
    0, //iInterface
  )
  ARRLEN1(//AS Interface
    bLENGTH, //bLength
    USB_DESCR_CS_INTERFACE, //bDescriptorType
    USBAUDIO_AS_GENERAL, //bDescriptorSubType
    6, //bTerminalLink  <----------------------------------------
    1, //bDelay //задержка, вносимая устройством (в единицах числа фреймов)
    USB_U16( USBAUDIO_FORMAT_PCM ), //wFormatTag=PCM, тип кодирования данных //TODO описать возможные типы
  )
  ARRLEN1(//AS Format Type 1
    bLENGTH, //bLength
    USB_DESCR_CS_INTERFACE, //bDescriptorType
    USBAUDIO_AS_FORMAT, //bDescriptorSubType
    1, //bFormatType
    1, //bNrChannels
    2, //bSubFrameSize //количество БАЙТОВ на отсчет (1-4)
    16, //bBitResolution //количество БИТОВ на отсчет (<= bSubFrameSize*8) //наверное, то-занимаемое в потоке место, а это - реальная разрешающая способность
    1, //bSamFreqType //количество поддерживаемых частот
    USB_AC24(F_SAMPLE), //tSamFreq //(6 байт!) массив диапазонов частот
  )
  ARRLEN1(//Endpoint descriptor
    bLENGTH, //bLength
    USB_DESCR_ENDPOINT, //bDescriptorType
    ENDP_IN_NUM | 0x80, 
    USB_ENDP_ISO, //Isochronous / Synch=none / usage=data
    USB_U16(ENDP_DATA_SIZE),
    1, //bInterval - частота опроса, для изохронных всегда 1
    0, //bRefresh - хз что это, сказано выставить в 0
    0, //bSynchAddress - адрес endpoint'а для синхронизации
  )
  ARRLEN1(//Isochronous endpoint descriptor
    bLENGTH, //bLength
    USB_DESCR_ENDP_ISO, //bDescriptorType
    1, //bDescriptorSubType
    0, //bmAttributes
    0, //bLockDelayUnits (undefned)
    USB_U16(0), //wLockDelay
  )
   
  ARRLEN1(//1 Audio Streaming Interface
    bLENGTH, //bLength
    USB_DESCR_INTERFACE, //bDescriptorType
    2, //bInterfaceNumber
    0, //bAlternateSetting
    0, //bNumEndpoints
    USB_CLASS_AUDIO, //bInterfaceClass
    USB_SUBCLASS_AUDIOSTREAMING, //bInterfaceSubClass
    0, //bInterfaceProtocol
    0, //iInterface
  )
  ARRLEN1(//1alt Audio Streaming Interface (alternative)
    bLENGTH, //bLength
    USB_DESCR_INTERFACE, //bDescriptorType
    2, //bInterfaceNumber
    1, //bAlternateSetting
    1, //bNumEndpoints
    USB_CLASS_AUDIO, //bInterfaceClass
    USB_SUBCLASS_AUDIOSTREAMING, //bInterfaceSubClass
    0, //bInterfaceProtocol
    0, //iInterface
  )
  ARRLEN1(//AS Interface
    bLENGTH, //bLength
    USB_DESCR_CS_INTERFACE, //bDescriptorType
    USBAUDIO_AS_GENERAL, //bDescriptorSubType
    1, //bTerminalLink  <----------------------------------------
    1, //bDelay //задержка, вносимая устройством (в единицах числа фреймов)
    USB_U16( USBAUDIO_FORMAT_PCM ), //wFormatTag=PCM, тип кодирования данных //TODO описать возможные типы
  )
  ARRLEN1(//AS Format Type 1
    bLENGTH, //bLength
    USB_DESCR_CS_INTERFACE, //bDescriptorType
    USBAUDIO_AS_FORMAT, //bDescriptorSubType
    1, //bFormatType
    1, //bNrChannels
    2, //bSubFrameSize //количество БАЙТОВ на отсчет (1-4)
    16, //bBitResolution //количество БИТОВ на отсчет (<= bSubFrameSize*8) //наверное, то-занимаемое в потоке место, а это - реальная разрешающая способность
    1, //bSamFreqType //количество поддерживаемых частот
    USB_AC24(F_SAMPLE), //tSamFreq //(6 байт!) массив диапазонов частот
  )
  ARRLEN1(//Endpoint descriptor
    bLENGTH, //bLength
    USB_DESCR_ENDPOINT, //bDescriptorType
    ENDP_DATA_NUM, 
    USB_ENDP_ISO, //Isochronous / Synch=none / usage=data
    USB_U16(ENDP_DATA_SIZE),
    1, //bInterval - частота опроса, для изохронных всегда 1
    0, //bRefresh - хз что это, сказано выставить в 0
    0, //bSynchAddress - адрес endpoint'а для синхронизации
  )
  ARRLEN1(//Isochronous endpoint descriptor
    bLENGTH, //bLength
    USB_DESCR_ENDP_ISO, //bDescriptorType
    1, //bDescriptorSubType
    0, //bmAttributes
    0, //bLockDelayUnits (undefned)
    USB_U16(0), //wLockDelay
  )
  )
};

USB_STRING(USB_StringLangDescriptor, u"\x0409"); //lang US
USB_STRING(USB_StringManufacturingDescriptor, u"COKPOWEHEU"); //Vendor
USB_STRING(USB_StringProdDescriptor, u"USB Audio"); //Product
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

uint8_t interface[3] = {0,0,0};

char usb_class_ep0_in(config_pack_t *req, void **data, uint16_t *size){
  
  return 0;
}
char usb_class_ep0_out(config_pack_t *req, uint16_t offset, uint16_t rx_size){
  if(req->bmRequestType == 0x01){
    if(req->bRequest == 0x0B){
      interface[req->wIndex] = req->wValue;
      usb_ep_write(0, NULL, 0);
      return 1;
    }
  }
  return 0;
}

int16_t dsin(uint8_t x){
  const int16_t arr[256] = {
    0, 785, 1570, 2354, 3136, 3917, 4695, 5470, 6242, 7011, 7775, 8534, 9289, 10037, 10780, 11516, 12245, 12967, 13681, 14387, 15084, 15772, 16451, 17119, 17778, 18425, 19062, 19687, 20300, 20901, 21489, 22065, 22627, 23175, 23710, 24230, 24736, 25227, 25702, 26162, 26607, 27035, 27447, 27842, 28221, 28583, 28927, 29254, 29564, 29855, 30129, 30384, 30622, 30840, 31041, 31222, 31385, 31528, 31653, 31759, 31845, 31913, 31961, 31990, 32000, 31990, 31961, 31913, 31845, 31759, 31653, 31528, 31385, 31222, 31041, 30840, 30622, 30384, 30129, 29855, 29564, 29254, 28927, 28583, 28221, 27842, 27447, 27035, 26607, 26162, 25702, 25227, 24736, 24230, 23710, 23175, 22627, 22065, 21489, 20901, 20300, 19687, 19062, 18425, 17778, 17119, 16451, 15772, 15084, 14387, 13681, 12967, 12245, 11516, 10780, 10037, 9289, 8534, 7775, 7011, 6242, 5470, 4695, 3917, 3136, 2354, 1570, 785, 0, -785, -1570, -2354, -3136, -3917, -4695, -5470, -6242, -7011, -7775, -8534, -9289, -10037, -10780, -11516, -12245, -12967, -13681, -14387, -15084, -15772, -16451, -17119, -17778, -18425, -19062, -19687, -20300, -20901, -21489, -22065, -22627, -23175, -23710, -24230, -24736, -25227, -25702, -26162, -26607, -27035, -27447, -27842, -28221, -28583, -28927, -29254, -29564, -29855, -30129, -30384, -30622, -30840, -31041, -31222, -31385, -31528, -31653, -31759, -31845, -31913, -31961, -31990, -32000, -31990, -31961, -31913, -31845, -31759, -31653, -31528, -31385, -31222, -31041, -30840, -30622, -30384, -30129, -29855, -29564, -29254, -28927, -28583, -28221, -27842, -27447, -27035, -26607, -26162, -25702, -25227, -24736, -24230, -23710, -23175, -22627, -22065, -21489, -20901, -20300, -19687, -19062, -18425, -17778, -17119, -16451, -15772, -15084, -14387, -13681, -12967, -12245, -11516, -10780, -10037, -9289, -8534, -7775, -7011, -6242, -5470, -4695, -3917, -3136, -2354, -1570, -785
  };
  return arr[x];
}

#define INTR_FREQ_HZ 1000
#define SAMPLE_COUNT ((F_SAMPLE / INTR_FREQ_HZ))
#define RES_FREQ_HZ 1000

void data_out_callback(uint8_t epnum){
  int cnt;
  int16_t buf[ENDP_DATA_SIZE/2];
  cnt = usb_ep_read_double(ENDP_DATA_NUM, (void*)buf);
  cnt /= 2;
  GPO_OFF(GLED);
  for(int i=0; i<cnt; i++){
    if(buf[i] > 10000)GPO_ON(GLED);
  }
}

volatile uint16_t count = 0;
void data_in_callback(uint8_t epnum){
  int16_t buf[ENDP_DATA_SIZE];
  uint8_t txsize = count;

  if(count > (ENDP_DATA_SIZE/2)){
    txsize = (ENDP_DATA_SIZE/2);
  }
  
  static int32_t cnt = 0;
  for(uint8_t i=0; i<txsize; i++){
    buf[i] = dsin( (uint32_t)cnt * RES_FREQ_HZ * 256 / F_SAMPLE );
    cnt++;
    if(cnt >= F_SAMPLE)cnt -= F_SAMPLE;
  }
  usb_ep_write_double(ENDP_IN_NUM, (void*)buf, txsize*2);

  count -= txsize;
}

void TIM4_IRQHandler(){
  TIM4->SR = ~TIM_SR_UIF;
  
  count += SAMPLE_COUNT;
  if(count > 0x8FFF){
    GPO_ON(RLED);
    count = 0;
  }
}

void usb_class_init(){
  usb_ep_init_double( ENDP_DATA_NUM, USB_ENDP_ISO, ENDP_DATA_SIZE, data_out_callback);
  usb_ep_init_double( ENDP_IN_NUM | 0x80, USB_ENDP_ISO, ENDP_DATA_SIZE, data_in_callback);
  
  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; //F_APB1=32MHz
  TIM4->PSC = 71; //1 MHz
  TIM4->ARR = (1000000 / INTR_FREQ_HZ)-1;
  TIM4->CR1 |= (TIM_CR1_ARPE);
  TIM4->DIER = (TIM_DIER_UIE);
  NVIC_EnableIRQ(TIM4_IRQn);
  TIM4->CR1 = (TIM_CR1_CEN);
}

void usb_class_poll(){
  //if(GPI_ON(LBTN)){GPO_OFF(RLED); GPO_OFF(GLED);}
}
