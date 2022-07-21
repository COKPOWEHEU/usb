#include "usb_lib.h"
#include <wchar.h>
#include "hardware.h"

extern const uint8_t my_res_start[]   asm(FATSRC "_start");
extern const uint8_t my_res_end[]     asm(FATSRC "_end");

#define ENDP_NUM  1
#define ENDP_SIZE 64
#define MSC_MEDIA_PACKET     512

#define STD_DESCR_LANG 0
#define STD_DESCR_VEND 1
#define STD_DESCR_PROD 2
#define STD_DESCR_SN   3
#define STD_DESCR_CONF 4
#define STD_DESCR_IF   5

#define USB_VID 0x0bda
#define USB_PID 0x0129
//#define USB_VID 0x16C0
//#define USB_PID 0x05DF

#define MSDCLASS_MSD          0x08
#define MSDSUBCLASS_SCSI      0x06
#define MSDPROTOCOL_BULKONLY  0x50

void scsi_reset();
void scsi_command();

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
    2, // bNumEndpoints
    MSDCLASS_MSD, // bInterfaceClass: 
    MSDSUBCLASS_SCSI, // bInterfaceSubClass: 
    MSDPROTOCOL_BULKONLY, // bInterfaceProtocol: 
    0x00, // iInterface
  )
  ARRLEN1(
    bLENGTH, //bLength
    USB_DESCR_ENDPOINT, //bDescriptorType
    ENDP_NUM | 0x80,  //Endpoint address
    USB_ENDP_BULK,   //Bulk endpoint type
    USB_U16(ENDP_SIZE), //endpoint size
    0x00,   //Polling interval in milliseconds (ignored)
  )
  ARRLEN1(
    bLENGTH, //bLength
    USB_DESCR_ENDPOINT, //bDescriptorType
    ENDP_NUM,  //Endpoint address
    USB_ENDP_BULK,   //Bulk endpoint type
    USB_U16(ENDP_SIZE), //endpoint size
    0x00,   //Polling interval in milliseconds (ignored)
  )
  )
};


USB_STRING(USB_StringLangDescriptor, u"\x0409"); //lang US
USB_STRING(USB_StringManufacturingDescriptor, u"COKPOWEHEU"); //Vendor
USB_STRING(USB_StringProdDescriptor, u"USB MSD"); //Product
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


#define USBCLASS_MSC_GET_MAX_LUN  0xFE
#define USBCLASS_MSC_RESET        0xFF

uint8_t maxlun = 15; //15; //больше 2 (maxlun=1) последние винды не умеют

uint8_t rambuf[1024*29];

struct{
  uint32_t capacity;
  const uint8_t *buf;
}storage[] = {
  [0] = {
    .capacity = 0,
    .buf = my_res_start,
  },
  /*[1] = {
    .capacity = 0,
    .buf = my_res_start,
  },*/
  [1] = {
    .capacity = sizeof(rambuf),
    .buf = rambuf
  },
  [2] = {
    .capacity = 0,
    .buf = my_res_start,
  },
  [3] = {
    .capacity = 0,
    .buf = my_res_start,
  },
  [4] = {
    .capacity = 0,
    .buf = my_res_start,
  },
  [5] = {
    .capacity = 0,
    .buf = my_res_start,
  },
  [6] = {
    .capacity = 0,
    .buf = my_res_start,
  },
  [7] = {
    .capacity = 0,
    .buf = my_res_start,
  },
  [8] = {
    .capacity = 0,
    .buf = my_res_start,
  },
  [9] = {
    .capacity = 0,
    .buf = my_res_start,
  },
  [10] = {
    .capacity = 0,
    .buf = my_res_start,
  },
  [11] = {
    .capacity = 0,
    .buf = my_res_start,
  },
  [12] = {
    .capacity = 0,
    .buf = my_res_start,
  },
  [13] = {
    .capacity = 0,
    .buf = my_res_start,
  },
  [14] = {
    .capacity = 0,
    .buf = my_res_start,
  },
  [15] = {
    .capacity = 0,
    .buf = my_res_start,
  },
};

char usb_class_ep0_in(config_pack_t *req, void **data, uint16_t *size){
  if(req->bRequest == USBCLASS_MSC_RESET){
    scsi_reset();
  }
  if(req->bRequest == USBCLASS_MSC_GET_MAX_LUN){ //MSC_GET_MAX_LUN
    *data = &maxlun;
    *size = 1;
  }
  return 0;
}


#define CSW_STATUS_SUCCESS		0
#define CSW_STATUS_FAILED		1
#define CSW_STATUS_PHASE_ERROR		2
// The sense codes
enum sbc_sense_key {
	SBC_SENSE_KEY_NO_SENSE		= 0x00,
	SBC_SENSE_KEY_RECOVERED_ERROR	= 0x01,
	SBC_SENSE_KEY_NOT_READY		= 0x02,
	SBC_SENSE_KEY_MEDIUM_ERROR	= 0x03,
	SBC_SENSE_KEY_HARDWARE_ERROR	= 0x04,
	SBC_SENSE_KEY_ILLEGAL_REQUEST	= 0x05,
	SBC_SENSE_KEY_UNIT_ATTENTION	= 0x06,
	SBC_SENSE_KEY_DATA_PROTECT	= 0x07,
	SBC_SENSE_KEY_BLANK_CHECK	= 0x08,
	SBC_SENSE_KEY_VENDOR_SPECIFIC	= 0x09,
	SBC_SENSE_KEY_COPY_ABORTED	= 0x0A,
	SBC_SENSE_KEY_ABORTED_COMMAND	= 0x0B,
	SBC_SENSE_KEY_VOLUME_OVERFLOW	= 0x0D,
	SBC_SENSE_KEY_MISCOMPARE	= 0x0E
};

enum sbc_asc {
	SBC_ASC_NO_ADDITIONAL_SENSE_INFORMATION	= 0x00,
	SBC_ASC_PERIPHERAL_DEVICE_WRITE_FAULT	= 0x03,
	SBC_ASC_LOGICAL_UNIT_NOT_READY		= 0x04,
	SBC_ASC_UNRECOVERED_READ_ERROR		= 0x11,
	SBC_ASC_INVALID_COMMAND_OPERATION_CODE	= 0x20,
	SBC_ASC_LBA_OUT_OF_RANGE		= 0x21,
	SBC_ASC_INVALID_FIELD_IN_CDB		= 0x24,
	SBC_ASC_WRITE_PROTECTED			= 0x27,
	SBC_ASC_NOT_READY_TO_READY_CHANGE	= 0x28,
	SBC_ASC_FORMAT_ERROR			= 0x31,
	SBC_ASC_MEDIUM_NOT_PRESENT		= 0x3A
};

enum sbc_ascq {
	SBC_ASCQ_NA				= 0x00,
	SBC_ASCQ_FORMAT_COMMAND_FAILED		= 0x01,
	SBC_ASCQ_INITIALIZING_COMMAND_REQUIRED	= 0x02,
	SBC_ASCQ_OPERATION_IN_PROGRESS		= 0x07
};

struct usb_msc_cbw{
  uint32_t dSignature;
  uint32_t dTag;
  uint32_t dDataLength;
  uint8_t  bmFlags;
  uint8_t  bLUN;
  uint8_t  bCBLength;
  uint8_t  CB[16];
}__attribute__((packed));
typedef struct usb_msc_cbw usb_msc_cbw_t;

struct usb_msc_csw{
  uint32_t dSignature;
  uint32_t dTag;
  uint32_t dDataResidue;
  uint8_t  bStatus;
}__attribute__((packed));
typedef struct usb_msc_csw usb_msc_csw_t;

struct usb_msc_sense{
	uint8_t key;
	uint8_t asc;
	uint8_t ascq;
}__attribute__((packed));
typedef struct usb_msc_sense usb_msc_sense_t;

usb_msc_cbw_t msc_cbw;
uint8_t msc_cbw_count = 0;
usb_msc_csw_t msc_csw = {
  .dSignature = 0x53425355, //волшебное чиселко
};
uint8_t msc_csw_count = 0;
usb_msc_sense_t msc_sense = {0,0,0};

static uint32_t bytestowrite = 0;
static uint32_t bytestoread = 0;
static uint32_t bytescount = 0;
static uint8_t buffer[MSC_MEDIA_PACKET];

uint32_t start_lba;
uint16_t block_count;
uint16_t cur_count = 0;

static void msc_ep1_in(uint8_t epnum);

static void msc_ep1_out(uint8_t epnum){
  int left = sizeof(usb_msc_cbw_t) - msc_cbw_count;
  if(left > 0){ //чтение команды
    int sz = usb_ep_read(ENDP_NUM, (uint16_t*)&(((uint8_t*)&msc_cbw)[msc_cbw_count]) );
    msc_cbw_count += sz;
    if(msc_cbw_count == sizeof(usb_msc_cbw_t)){ //команда прочитана полностью
      scsi_command();
    }else return;
  }else if(bytescount < bytestoread){ //если разнести условие, произойдет повторное чтение буфера EP1_OUT, который был прочитан раньше, но size не сброшен (все равно этим железо занимается)
    uint8_t lun = msc_cbw.bLUN;
    int sz;
    if(lun == 0){
      sz = usb_ep_read(ENDP_NUM, (uint16_t*)&buffer[0]);
    }else{
      sz = usb_ep_read(ENDP_NUM, (uint16_t*)&rambuf[ start_lba*512 + bytescount ]);
      cur_count += sz;
    }
    bytescount += sz;
  }
  if(bytescount < bytestoread)return;
  msc_ep1_in(ENDP_NUM | 0x80);
}

static void msc_ep1_in(uint8_t epnum){
  if(! usb_ep_ready(epnum) )return;
  
  if(bytescount < bytestowrite){
    uint32_t left = bytestowrite - bytescount;
    if(left > ENDP_SIZE)left = ENDP_SIZE;
    if(block_count == 0){
      usb_ep_write(ENDP_NUM, &buffer[bytescount], left);
    }else{
      uint8_t lun = msc_cbw.bLUN;
      usb_ep_write(ENDP_NUM, &storage[lun].buf[start_lba*512 + bytescount], left);
      cur_count += left;
    }
    bytescount += left;
  }else{
    int32_t left = sizeof(msc_csw) - msc_csw_count;
    if(left > 0){
      if(left > ENDP_SIZE)left = ENDP_SIZE;
      usb_ep_write(ENDP_NUM, (uint8_t*)&(((uint8_t*)&msc_csw)[msc_csw_count]), left);
      msc_csw_count += left;
    }else if(left == 0){
      msc_cbw_count = 0;
      msc_csw_count = 0;
      bytestoread = 0;
      bytestowrite = 0;
      bytescount = 0;
      
      block_count = 0;
      cur_count = 0;
    }
  }
}

void usb_class_init(){
  usb_ep_init(ENDP_NUM,        USB_EP_BULK, ENDP_SIZE, msc_ep1_out);
  usb_ep_init(ENDP_NUM | 0x80, USB_EP_BULK, ENDP_SIZE, msc_ep1_in);
  storage[0].capacity = (my_res_end - my_res_start);
  for(int i=2;i<=maxlun; i++)storage[i].capacity = storage[0].capacity;
  for(uint16_t i=0; i<512; i++){rambuf[i] = storage[0].buf[i];}
}


//SCSI

// Implemented SCSI Commands
#define SCSI_TEST_UNIT_READY	0x00
#define SCSI_REQUEST_SENSE	0x03
#define SCSI_FORMAT_UNIT	0x04
#define SCSI_READ_6		0x08
#define SCSI_WRITE_6		0x0A
#define SCSI_INQUIRY		0x12
#define SCSI_MODE_SENSE_6	0x1A
#define SCSI_SEND_DIAGNOSTIC	0x1D
#define SCSI_READ_CAPACITY	0x25
#define SCSI_READ_10		0x28
#define SCSI_WRITE_10		0x2A

#define SCSI_MMC_START_STOP_UNIT	0x1B
#define SCSI_MMC_PREVENT_ALLOW_REMOVAL	0x1E
#define SCSI_MMC_READ_FORMAT_CAPACITY 0x23 //винда очень любит этот запрос, а коррекно обрабатывать ответ "не поддерживаю" не умеет

typedef struct{
  uint8_t opcode;
  uint8_t cdb_info1;
  uint32_t block_address;
  uint8_t cdb_info2;
  uint16_t length;
  uint8_t control;
}scsi_cbw_10_t;

inline void scsi_reset(){
  //TODO
}

static const uint8_t inquiry_response[36] = {
  0x00,	// Byte 0: Peripheral Qualifier = 0, Peripheral Device Type = 0
  0x80,	// Byte 1: RMB = 1, Reserved = 0
  0x04,	// Byte 2: Version = 0
  0x02,	// Byte 3: Obsolete = 0, NormACA = 0, HiSup = 0, Response Data Format = 2
  0x1F,	// Byte 4: Additional Length (n-4) = 31 + 4
  0x00,	// Byte 5: SCCS = 0, ACC = 0, TPGS = 0, 3PC = 0, Reserved = 0, Protect = 0
  0x00,	// Byte 6: BQue = 0, EncServ = 0, VS = 0, MultiP = 0, MChngr = 0, Obsolete = 0, Addr16 = 0
  0x00,	// Byte 7: Obsolete = 0, Wbus16 = 0, Sync = 0, Linked = 0, CmdQue = 0, VS = 0
  'C', 'O', 'K', 'P', ' ', ' ', ' ', ' ', //Vendor (8 bytes)
  'M', 'S', 'D', ' ', ' ', ' ', ' ', ' ', //Product (16 bytes)
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  '0', '.', '0', '1' //Version (4 bytes)
};

#define LENGTH_INQUIRY_PAGE00		 7
const uint8_t  inquiry_page00_data[] = {//7						
  0x00,		
  0x00, 
  0x00, 
  (LENGTH_INQUIRY_PAGE00 - 4),
  0x00, 
  0x80, 
  0x83 
};  

void scsi_inquiry(){
  if( msc_cbw.CB[1] & 0x01){ //EVPD is set. Для Win10 это важно. Для Linux или WinXP - нет
    for(uint16_t i=0; i<sizeof(inquiry_page00_data); i++)buffer[i] = ((uint8_t*)inquiry_page00_data)[i];
    bytestowrite = sizeof(inquiry_page00_data);
  }else{
    for(uint16_t i=0; i<sizeof(inquiry_response); i++)buffer[i] = ((uint8_t*)inquiry_response)[i];
    bytestowrite = sizeof(inquiry_response);
  }
  if( bytestowrite < msc_cbw.dDataLength ) bytestowrite = msc_cbw.dDataLength;
  
  msc_csw.dDataResidue = msc_cbw.dDataLength - bytestowrite;
  msc_sense.key = SBC_SENSE_KEY_NO_SENSE;
  msc_sense.asc = SBC_ASC_NO_ADDITIONAL_SENSE_INFORMATION;
  msc_sense.ascq =SBC_ASCQ_NA;
}

static const uint8_t sense_response[18] = {
  0x70,	// Byte 0: VALID = 0, Response Code = 112
  0x00,	// Byte 1: Obsolete = 0
  0x00,	// Byte 2: Filemark = 0, EOM = 0, ILI = 0, Reserved = 0, Sense Key = 0
  	// Byte 3 - Byte 6: Information = 0
  0, 0, 0, 0,
  0x0a,	// Byte 7: Additional Sense Length = 10
  	// Byte 8 - Byte 11: Command Specific Info = 0
  0, 0, 0, 0,
  0x00,	// Byte 12: Additional Sense Code (ASC) = 0
  0x00,	// Byte 13: Additional Sense Code Qualifier (ASCQ) = 0
  0x00,	// Byte 14: Field Replaceable Unit Code (FRUC) = 0
  0x00,	// Byte 15: SKSV = 0, SenseKeySpecific[0] = 0
  0x00,	// Byte 16: SenseKeySpecific[0] = 0
  0x00	// Byte 17: SenseKeySpecific[0] = 0
};
void scsi_request_sense(){
  //memcpy(buffer, sense_response, 18);
  for(uint8_t i=0; i<sizeof(sense_response); i++)buffer[i] = sense_response[i];
  buffer[2] = msc_sense.key; //0x05
  buffer[12]= msc_sense.asc; //0x20
  buffer[13]= msc_sense.ascq; //0x00
  bytestowrite = 18;
}

void scsi_test_unit_ready(){
  //return OK
  msc_csw.dDataResidue = 0;
  msc_sense.key = SBC_SENSE_KEY_NO_SENSE;
  msc_sense.asc = SBC_ASC_NO_ADDITIONAL_SENSE_INFORMATION;
  msc_sense.ascq =SBC_ASCQ_NA;
}

void scsi_read_capacity(){
  uint8_t lun = msc_cbw.bLUN;
  uint32_t last_lba = storage[lun].capacity / 512 - 1;
  
  buffer[0] = (last_lba >> 24) & 0xFF;
  buffer[1] = (last_lba >> 16) & 0xFF;
  buffer[2] = (last_lba >>  8) & 0xFF;
  buffer[3] = (last_lba >>  0) & 0xFF;
  
  buffer[4] = 0;
  buffer[5] = 0;
  buffer[6] = 2;
  buffer[7] = 0;
  
  bytestowrite = 8;

  msc_csw.dDataResidue = 0;
  msc_sense.key = SBC_SENSE_KEY_NO_SENSE;
  msc_sense.asc = SBC_ASC_NO_ADDITIONAL_SENSE_INFORMATION;
  msc_sense.ascq =SBC_ASCQ_NA;
}

void scsi_mode_sense_6(){
  buffer[0] = 3;
  buffer[1] = 0;
  if(msc_cbw.bLUN != 1){ //LUN1 = ram buffer, read-write. Other LUNs = rom buffer, read-only
    buffer[2] = (1<<7);
  }else{
    buffer[2] = 0;
  }
  buffer[3] = 0;
  bytestowrite = 4;
  msc_csw.dDataResidue = msc_cbw.dDataLength-4;
}

void scsi_read_10(){
  //uint32_t 
  start_lba = (msc_cbw.CB[2] << 24) | (msc_cbw.CB[3] << 16) | (msc_cbw.CB[4] << 8) | (msc_cbw.CB[5] << 0);
  //uint16_t 
  block_count = (msc_cbw.CB[7] << 8) | (msc_cbw.CB[8] << 0);
  cur_count = 0;
  
  bytestowrite = ((uint32_t)block_count) << 9;
  
  msc_sense.key = SBC_SENSE_KEY_NO_SENSE;
  msc_sense.asc = SBC_ASC_NO_ADDITIONAL_SENSE_INFORMATION;
  msc_sense.ascq =SBC_ASCQ_NA;
}

void scsi_write_10(){
  start_lba = (msc_cbw.CB[2] << 24) | (msc_cbw.CB[3] << 16) | (msc_cbw.CB[4] << 8) | (msc_cbw.CB[5] << 0);
  block_count = (msc_cbw.CB[7] << 8) | (msc_cbw.CB[8] << 0);
  
  uint8_t lun = msc_cbw.bLUN;
  cur_count = 0;
  
  if(lun != 1){
    msc_csw.bStatus = CSW_STATUS_FAILED;
    msc_sense.key = SBC_SENSE_KEY_MEDIUM_ERROR;
    msc_sense.asc = SBC_ASC_WRITE_PROTECTED;
    msc_sense.ascq =SBC_ASCQ_NA;
    return;
  }
  
  bytestoread = ((uint32_t)block_count) << 9;
  
  msc_sense.key = SBC_SENSE_KEY_NO_SENSE;
  msc_sense.asc = SBC_ASC_NO_ADDITIONAL_SENSE_INFORMATION;
  msc_sense.ascq =SBC_ASCQ_NA;
}

void scsi_mmc_read_fmt_cap(){
  //uint8_t lun = msc_cbw.bLUN;
  
  buffer[0] = buffer[1] = buffer[2] = 0; //reserved
  buffer[3] = 8*(maxlun+1);//size

  uint32_t lunsize;
  uint32_t repsize = 4;
  for(uint8_t i=0; i<=maxlun; i++){
    lunsize = storage[i].capacity / 512;
    buffer[repsize+0] = (lunsize >> 24) & 0xFF;
    buffer[repsize+1] = (lunsize >> 16) & 0xFF;
    buffer[repsize+2] = (lunsize >>  8) & 0xFF;
    buffer[repsize+3] = (lunsize >>  0) & 0xFF;
    buffer[repsize+4] = 0;
    buffer[repsize+5] = 0; // \.
    buffer[repsize+6] = 2; //  } Block length = 512 bytes
    buffer[repsize+7] = 0; // /.
    repsize += 8;
    if(repsize >= sizeof(buffer)){repsize = sizeof(buffer); break;}
  }
  bytestowrite = repsize;
  
  msc_sense.key = SBC_SENSE_KEY_NO_SENSE;
  msc_sense.asc = SBC_ASC_NO_ADDITIONAL_SENSE_INFORMATION;
  msc_sense.ascq =SBC_ASCQ_NA;
}

void scsi_command(){
  msc_csw.dTag = msc_cbw.dTag;
  msc_csw.bStatus = CSW_STATUS_SUCCESS;
  msc_csw.dDataResidue = 0;
  
  bytestoread = 0;
  bytestowrite = 0;
  bytescount = 0;
  
  switch( msc_cbw.CB[0] ){
    case SCSI_INQUIRY:
      scsi_inquiry();
      break;
    case SCSI_TEST_UNIT_READY:
      scsi_test_unit_ready();
      break;
    case SCSI_READ_CAPACITY:
      scsi_read_capacity();
      break;
    case SCSI_REQUEST_SENSE:
      scsi_request_sense();
      break;
    case SCSI_MODE_SENSE_6:
      scsi_mode_sense_6();
      break;
    case SCSI_READ_10:
      scsi_read_10();
      break;
    case SCSI_WRITE_10:
      scsi_write_10();
      break;
    case SCSI_MMC_READ_FORMAT_CAPACITY:
      scsi_mmc_read_fmt_cap();
      break;
    case SCSI_MMC_START_STOP_UNIT:
    case SCSI_MMC_PREVENT_ALLOW_REMOVAL:
      break;
    default:
      msc_csw.bStatus = CSW_STATUS_FAILED;
      msc_sense.key = SBC_SENSE_KEY_ILLEGAL_REQUEST;
      msc_sense.asc = SBC_ASC_INVALID_COMMAND_OPERATION_CODE;
      msc_sense.ascq = SBC_ASCQ_NA;
      //
      break;
  }
}

