#ifndef __VIRFAT_H__
#define __VIRFAT_H__
#include <inttypes.h>

#define VIRFAT_VOLNAME		"Programmer "

#if 0==1
//////////////////////////////////////////////////////////////
//  User-defined settings, variables and callbacks of FAT ////
//////////////////////////////////////////////////////////////
#define VIRFAT_READONLY // (#define / not define)
#define VIRFAT_TIME_CALLBACK // (#define / not define)
#define VIRFAT_DATE_DD_MM_YYYY	1, 3, 2022 //date of create / last access
#define VIRFAT_TIME_HH_MM_SS	0, 0, 0 //time of create / last access
#define VIRFAT_VOLID		0xFC561629 //disk UUID 
#define VIRFAT_VOLNAME		"VIRTUAL_FAT" //disk Label (sometimes ignored by OS)

//optional setings
#define VIRFAT_JMPBOOT		{0xEB, 0x3C, 0x90} //I dont know why but this data is important...
#define VIRFAT_OEMNAME		"virfat  " //FAT internal name of program created FS

//sequence of files in Root directory
static const virfat_file_t virfat_rootdir[] = {...};

typedef struct{
  char *name;					//file name (in DOS format, upper register, 8.3)
  virfat_callback file_read;	//callback Host reading sector from Device
  virfat_callback file_write;	//callback Host writing sector to Device
  uint16_t size;				//file size
}virfat_file_t;

//example function of reading file from USB storage (callback function):
//  buf - buffer to read (1 sector = 512 bytes)
//  addr - number of reading sector (0 ... )
//  idx - file index (in virfat_rootdir array)
void demo_log_read(uint8_t *buf, uint32_t addr, uint16_t file_idx){ buf = data[addr*512]; }

//example function of writing file to USB storage (callback function):
//  buf - buffer to write (1 sector = 512 bytes)
//  addr - number of writing sector (0 ... )
//  idx - file index (in virfat_rootdir array)
void demo_log_read(uint8_t *buf, uint32_t addr, uint16_t file_idx){ data[arr*512] = buf; }

//callback function to syncronize time by reading 'last access/write time)
void virfat_time_callback(uint16_t date, uint16_t time){...}

//last sector in storage
#define VIRFAT_SECTOR_LAST (N)

typedef struct virfat_date_t; - bit structure to encode/decode FAT date
typedef struct virfat_time_t; - bit structure to encode/decode FAT time

//////////////////////////////////////////////////////////////
//  System function called by USB driver /////////////////////
//////////////////////////////////////////////////////////////

uint32_t virfat_getsize(); //storage size (in sectors)
void virfat_read(uint8_t *buf, uint32_t addr); //read 1 sector by address (addr) from storate
void virfat_write(uint8_t *buf, uint32_t addr);//write 1 sector by address (addr) to storage
#endif


//#define VIRFAT_READONLY
#ifndef VIRFAT_DATE_DD_MM_YYYY
  #define VIRFAT_DATE_DD_MM_YYYY	1, 3, 2022
#endif
#ifndef VIRFAT_TIME_HH_MM_SS
  #define VIRFAT_TIME_HH_MM_SS	0, 0, 0
#endif
#ifndef VIRFAT_VOLID
  #define VIRFAT_VOLID		0xFC561629
#endif
#ifndef VIRFAT_JMPBOOT
  #define VIRFAT_JMPBOOT		{0xEB, 0x3C, 0x90}
#endif
#ifndef VIRFAT_OEMNAME
  #define VIRFAT_OEMNAME		"virfat  "
#endif
#define VIRFAT_FSNAME		"FAT16   "

//////////////////////////////////////////////////////////////
//  display firmware of this device as .bin file /////////////
//////////////////////////////////////////////////////////////
extern const uint8_t *flash_end     asm("_etext");
extern const uint8_t *ram_end       asm("_edata");
#define FLASH_SIZE	40960 //размер прошивки. Взят с запасом, в компил-тайме его не вычислить
#include "../build/src_zip.h"

void firmware_read(uint8_t *buf, uint32_t addr, uint16_t file_idx){
  const uint8_t *flash = (uint8_t*)0x08000000;
  for(int i=0; i<512; i++)buf[i] = flash[i + addr*512];
}

//  Example makefile
const char make_file[] =
"test_stflash:\n"
"	stty -F /dev/tty_STFLASH_0 300\n"
"	stty -F /dev/tty_STFLASH_0 50\n"
"	echo 'RBU' > /dev/tty_STFLASH_0\n"
"	echo 'rBU' > /dev/tty_STFLASH_0\n"
"	sleep 1\n"
"	stm32flash -r read.bin -S 0x08000000:32768 /dev/tty_STFLASH_0\n"
"	sleep 1\n"
"	echo 'RbU' > /dev/tty_STFLASH_0\n"
"	sleep 1\n"
"	echo 'rbuz' > /dev/tty_STFLASH_0\n"
"test_arduino:\n"
"	stty -F /dev/tty_STFLASH_0 9600\n"
"	stty -F /dev/tty_STFLASH_0 50\n"
"	avrdude -c arduino -p atmega8 -P /dev/tty_STFLASH_0 -b 115200 -Uflash:r:/dev/null:i\n"
"	stty -F /dev/tty_STFLASH_0 50\n"
"	echo 'z' > /dev/tty_STFLASH_0\n"
;
//  Example udev rules file
const char rules_file[] =
"# /etc/udev/rules.d/98-cokp_serial.rules\n"
"\n"
"SUBSYSTEM==\"tty\", ATTRS{manufacturer}==\"COKPOWEHEU\" ENV{CONNECTED_COKP}=\"yes\"\n"
"ENV{CONNECTED_COKP}==\"yes\", SUBSYSTEM==\"tty\", ATTRS{interface}==\"?*\", PROGRAM=\"/bin/bash -c \\\"ls /dev | grep tty_$attr{interface}_ | wc -l \\\"\", SYMLINK+=\"tty_$attr{interface}_%c\"\n"
;
// Hardware info file
#define _STR(x) #x
#define STR(x) _STR(x)
const char hardware_file[] =
"stm32f103\r\n"
"\tUART:\r\n"
"\t\tTx = P" STR(marg1(UART_TX)) STR(marg2(UART_TX)) "\r\n"
"\t\tRx = P" STR(marg1(UART_RX)) STR(marg2(UART_RX)) "\r\n"
"\t\tDTR= P" STR(marg1(DTR)) STR(marg2(DTR)) "\r\n"
"\tReset_out = P" STR(marg1(RESET)) STR(marg2(RESET)) "\r\n"
"\tBoot0_out = P" STR(marg1(BOOT0)) STR(marg2(BOOT0)) "\r\n"
"\tUSBR (USB relay) = P" STR(marg1(USBR)) STR(marg2(USBR)) "\r\n"
"\r\n"
"\tLEDS:\r\n"
"\t\tGreen (debug mode): P" STR(marg1(GLED)) STR(marg2(GLED)) "\r\n"
"\t\tRed (programming mode): P" STR(marg1(RLED)) STR(marg2(RLED)) "\r\n"
"\t\tRed + Green (change Reset, Boot0, USBR pins status): Red + Green\r\n"
"\r\n"
"Connect UART of this programmer to UART of target microcontroller, Reset_out pin to Reset and Boot0_out to BOOT0. If needed connect USBR to relay disconnecting D+,D- of target. Edit or copy udev rules according example.\r\n"
"Then run 'make test_stflash' or 'make test_arduino' to test connection. You also may use this MAKEFILE as example to burn firmwaries\r\n"
;

const char lufa_driver_file[] =
";************************************************************\r\n"
"; Windows USB CDC ACM Setup File\r\n"
"; Copyright (c) 2000 Microsoft Corporation\r\n"
";************************************************************\r\n"
"\r\n"
"[DefaultInstall]\r\n"
"CopyINF=\"LUFA_CDC.INF\"\r\n"
"\r\n"
"[Version]\r\n"
"Signature=\"$Windows NT$\"\r\n"
"Class=Ports\r\n"
"ClassGuid={4D36E978-E325-11CE-BFC1-08002BE10318}\r\n"
"Provider=%MFGNAME%\r\n"
"DriverVer=7/1/2012,10.0.0.0\r\n"
"\r\n"
"[Manufacturer]\r\n"
"%MFGNAME%=DeviceList, NTx86, NTamd64, NTia64\r\n"
"\r\n"
"[SourceDisksNames]\r\n"
"\r\n"
"[SourceDisksFiles]\r\n"
"\r\n"
"[DestinationDirs]\r\n"
"DefaultDestDir=12\r\n"
"\r\n"
"[DriverInstall]\r\n"
"Include=mdmcpq.inf\r\n"
"CopyFiles=FakeModemCopyFileSection\r\n"
"AddReg=DriverInstall.AddReg\r\n"
"\r\n"
"[DriverInstall.Services]\r\n"
"Include=mdmcpq.inf\r\n"
"AddService=usbser, 0x00000002, LowerFilter_Service_Inst\r\n"
"\r\n"
"[DriverInstall.AddReg]\r\n"
"HKR,,EnumPropPages32,,\"msports.dll,SerialPortPropPageProvider\"\r\n"
"\r\n"
";------------------------------------------------------------------------------\r\n"
";  Vendor and Product ID Definitions\r\n"
";------------------------------------------------------------------------------\r\n"
"; When developing your USB device, the VID and PID used in the PC side\r\n"
"; application program and the firmware on the microcontroller must match.\r\n"
"; Modify the below line to use your VID and PID.  Use the format as shown below.\r\n"
"; Note: One INF file can be used for multiple devices with different VID and PIDs.\r\n"
"; For each supported device, append \",USB\\VID_xxxx&PID_yyyy\" to the end of the line.\r\n"
";------------------------------------------------------------------------------\r\n"
"[DeviceList]\r\n"
"%DESCRIPTION%=DriverInstall, USB\\VID_16C0&PID_05DF\r\n"
"\r\n"
"[DeviceList.NTx86]\r\n"
"%DESCRIPTION%=DriverInstall, USB\\VID_16C0&PID_05DF\r\n"
"\r\n"
"[DeviceList.NTamd64]\r\n"
"%DESCRIPTION%=DriverInstall, USB\\VID_16C0&PID_05DF\r\n"
"\r\n"
"[DeviceList.NTia64]\r\n"
"%DESCRIPTION%=DriverInstall, USB\\VID_16C0&PID_05DF\r\n"
"\r\n"
";------------------------------------------------------------------------------\r\n"
";  String Definitions\r\n"
";------------------------------------------------------------------------------\r\n"
";Modify these strings to customize your device\r\n"
";------------------------------------------------------------------------------\r\n"
"[Strings]\r\n"
"MFGNAME=\"http://www.lufa-lib.org\"\r\n"
"DESCRIPTION=\"LUFA CDC-ACM Virtual Serial Port\"\r\n"
;

void file_read(uint8_t *buf, uint32_t addr, uint16_t file_idx){
  const char *txt = (void*)0;
  int sz = 0;
  if(file_idx == 1){
    txt = make_file;
    sz = sizeof(make_file);
  }else if(file_idx == 2){
    txt = rules_file;
    sz = sizeof(rules_file);
  }else if(file_idx == 3){
    txt = hardware_file;
    sz = sizeof(hardware_file);
  }else if(file_idx == 4){
    txt = lufa_driver_file;
    sz = sizeof(lufa_driver_file);
  }else if(file_idx == 5){
    txt = (char*)src_zip;
    sz = sizeof(src_zip);
  }
  addr *= 512;
  int en = 512;
  if(addr + 512 > sz)en = (sz - addr);
  for(int i=0; i<en; i++)buf[i] = txt[addr + i];
  for(int i=en; i<512; i++)buf[i] = ' ';
}

//////////////////////////////////////////////////////////////
//  Dummy file read/write function
//////////////////////////////////////////////////////////////
void virfat_file_dummy(uint8_t *buf, uint32_t addr, uint16_t file_idx){
  for(uint16_t i=0; i<512; i++)buf[i] = ' ';
}

//////////////////////////////////////////////////////////////
typedef void(*virfat_callback)(uint8_t *buf, uint32_t addr, uint16_t file_idx);
typedef struct{
  char *name;
  virfat_callback file_read;
  virfat_callback file_write;
  uint16_t size;
}virfat_file_t;

static const virfat_file_t virfat_rootdir[] = {
  {
    .name = "firmwarebin",
    .file_read = firmware_read,
    .file_write = virfat_file_dummy,
    .size = (FLASH_SIZE+511) / 512,
  },
  {
    .name = "makefile   ",
    .file_read = file_read,
    .file_write = virfat_file_dummy,
    .size = (sizeof(make_file)+511) / 512,
  },
  {
    .name = "RULES   TXT",
    .file_read = file_read,
    .file_write = virfat_file_dummy,
    .size = (sizeof(rules_file)+511) / 512,
  },
  {
    .name = "HARDWARETXT",
    .file_read = file_read,
    .file_write = virfat_file_dummy,
    .size = (sizeof(hardware_file)+511)/512,
  },
  {
    .name = "LUFA_CDCINF",
    .file_read = file_read,
    .file_write = virfat_file_dummy,
    .size = (sizeof(lufa_driver_file)+511)/512,
  },
  {
    .name = "SRC     ZIP",
    .file_read = file_read,
    .file_write = virfat_file_dummy,
    .size = (sizeof(src_zip)+511)/512,
  },
};

#define VIRFAT_FILES_TOTAL	( sizeof(virfat_rootdir) / sizeof(virfat_file_t) )
#ifdef VIRFAT_VOLNAME
  #define VIRFAT_ROOTENT (( VIRFAT_FILES_TOTAL + 15 + 1 ) &~ 15)
#else
  #define VIRFAT_ROOTENT (( VIRFAT_FILES_TOTAL + 15) &~ 15)
#endif

#define virfat_pbrstart 0
#define virfat_fatstart 1
static uint16_t virfat_rootstart;
#define virfat_datastart (virfat_rootstart + (VIRFAT_ROOTENT / 16))

#define _FAT_DATE(day, month, year) ( ((((year)-1980) & 127)<<9) | (((month)&15)<<5) | ((day) & 31) )
#define FAT_DATE(x) _FAT_DATE(x)
#define _FAT_TIME(hh, mm, ss) ( (((hh) & 31)<<11) | (((mm) & 63)<<5) | (((ss)/2) & 31) )
#define FAT_TIME(x) _FAT_TIME(x)
typedef struct{
  uint16_t day:5;
  uint16_t month:4;
  uint16_t year_1980:7;
}virfat_date_t;
typedef struct{
  uint16_t sec2:5;
  uint16_t min:6;
  uint16_t hour:5;
}virfat_time_t;

#pragma pack(push, 1)
typedef struct __attribute__((__packed__)){
  uint8_t JmpBoot[3]; //ignore
  uint8_t OEMname[8];
  uint16_t BytesPerSec; //512
  uint8_t SecPerClust; //1
  uint16_t SecReserved; //1
  uint8_t NumFats; //1
  uint16_t RootEntCnt; //32 (кратно 32)
  uint16_t TotSect; //0x2000 (от 0x0FFF - это fat12)
  uint8_t DriveType; //F8 - HDD
  uint16_t fatsize; //0x0020 (TotSect / BytesPerSec / SecPerClust * 2)
  uint16_t SecPerTrak; //0x0020 ?
  uint16_t NumHeads; //0x0040
  uint32_t SecHidden; //0
  uint32_t TotSect32; //0
  
  uint8_t DriveNum; //0x80 ?
  uint8_t NTErrFlag; //0
  uint8_t BootSig; //0x29 ?
  uint32_t VolID; //0xFC561629?
  char VolName[11]; //строка
  char FSName[8]; //"FAT16   "
  //reserved[]
  //[0x01FE] = 0x55
  //[0x01FF] = 0xAA
}virfat_pbr_t;
static const virfat_pbr_t virfat_pbr = {
  .JmpBoot = VIRFAT_JMPBOOT,
  .OEMname = VIRFAT_OEMNAME,
  .BytesPerSec = 512,
  .SecPerClust = 1,
  .SecReserved = 1,
  .NumFats = 1,
  .RootEntCnt = VIRFAT_ROOTENT,
  //.TotSect = VIRFAT_TOTSECT,
  .DriveType = 0xF8,
  //.fatsize = VIRFAT_FAT_SIZE, //(TotSect / BytesPerSec / SecPerClust * 2)
  .SecPerTrak = 0x0020, //?
  .NumHeads = 0x0040, //?
  .SecHidden = 0,
  .TotSect32 = 0,
  .DriveNum = 0x80, //?
  .NTErrFlag = 0, //?
  .BootSig = 0x29,//0, //?
  .VolID = VIRFAT_VOLID,
#ifdef VIRFAT_VOLNAME
  .VolName = VIRFAT_VOLNAME,
#else
  .VolName = "___________",
#endif
  .FSName = VIRFAT_FSNAME,
};
#pragma pack(pop)

static uint16_t virfat_cur_date = FAT_DATE( VIRFAT_DATE_DD_MM_YYYY );
static uint16_t virfat_cur_time = FAT_TIME( VIRFAT_TIME_HH_MM_SS );

#define CLUST_BROKEN 0xFFF7

#pragma pack(push, 1)
typedef struct __attribute__((__packed__)) {
  char name[11];
  uint8_t dir_attr;
  uint8_t NTattr; //ignore
  uint8_t create_time_10ms;
  uint16_t create_time_s;
  uint16_t creae_date;
  uint16_t acc_date;
  uint16_t cluster1_HI;
  uint16_t write_time;
  uint16_t write_date;
  uint16_t cluster1_LO;
  uint32_t size;
}dir_elem;
#pragma pack(pop)
#define VIRFAT_FLAG_RO		0x01
#define VIRFAT_FLAG_HIDDEN	0x02
#define VIRFAT_FLAG_SYSTEM	0x04
#define VIRFAT_FLAG_VOLID	0x08
#define VIRFAT_FLAG_DIR		0x10
#define VIRFAT_FLAG_ARCHIVE	0x20

uint32_t virfat_getsize();
/////////////////////////////////////////////////////
/////// Read  ///////////////////////////////////////
/////////////////////////////////////////////////////
static inline void _virfat_read_pbr(uint8_t *buf, uint32_t addr){
  uint16_t i;
  for(i=0; i<sizeof(virfat_pbr); i++){
    buf[i] = ((uint8_t*)&virfat_pbr)[i];
  }
  ((virfat_pbr_t*)buf)->TotSect = virfat_getsize();
  ((virfat_pbr_t*)buf)->fatsize = (virfat_rootstart - virfat_fatstart);
  for(; i< 0x01FE; i++)buf[i] = 0;
  buf[0x01FE] = 0x55;
  buf[0x01FF] = 0xAA;
}
static inline void _virfat_read_fat(uint8_t *buf, uint32_t addr){
  uint16_t *fat = (uint16_t*)buf;
  uint16_t *fat_end = (uint16_t*)(buf + 512);
  uint16_t sec = (addr-1)*(512/2);
  if(addr == 1){
    fat[0] = 0xFFF8;
    fat[1] = 0xFFFF;
    fat[2] = 0x0000;
    fat += 3;
    sec += 3;
  }
  sec -=3; //3 sectors reserved
  sec += 1; //file addressing from (start+1) to (end)
  uint16_t next_file_addr = 0;
  for(uint16_t i=0; i<VIRFAT_FILES_TOTAL; i++){
    next_file_addr += virfat_rootdir[i].size;
    for(;sec < next_file_addr; sec++){
      fat[0] = sec + 3;
      fat++;
      if(fat == fat_end)return;
    }
    if( sec == next_file_addr){
      sec++;
      fat[0] = 0xFFFF;
      fat++;
      if(fat == fat_end)return;
    }
  }
  
  for(;fat < fat_end; fat++){
    fat[0] = CLUST_BROKEN;
  }
}

static inline void _virfat_read_root(uint8_t *buf, uint32_t addr){
  uint16_t file_addr = 3;
  uint16_t i;
  dir_elem *elem = (dir_elem*)buf;
  
  i = (addr - virfat_rootstart) * 16;
  for(uint16_t j=0; j<i; j++){
    file_addr += virfat_rootdir[j].size;
  }
  
#warning Error detected on CH32V203. TODO: test on STM32!
  //elem += i;
  uint16_t cnt = 0;
  
#ifdef VIRFAT_VOLNAME
  if(i == 0){
    for(uint16_t j=0; j<11; j++)elem[0].name[j] = VIRFAT_VOLNAME[j];
    elem[0].dir_attr = VIRFAT_FLAG_VOLID;
    elem[0].acc_date = elem[0].write_date = virfat_cur_date;
    elem[0].write_time = virfat_cur_time;
    elem[0].cluster1_HI = 0;
    elem[0].cluster1_LO = 0;
    elem[0].size = 0;
    cnt++;
    elem++;
    //i++;
    i--;
  }
  //i--;
  i++;
#endif
  for(; i<VIRFAT_FILES_TOTAL; i++, cnt++){
    if(cnt == 16)return;
    for(uint16_t j=0; j<sizeof(dir_elem); j++){ ((uint8_t*)elem)[j] = 0; }
    elem[0].dir_attr = VIRFAT_FLAG_ARCHIVE;
    //if write operations does not supports -> mark file as read-only
    if(virfat_rootdir[i].file_write == virfat_file_dummy)elem[0].dir_attr |= VIRFAT_FLAG_RO;
    
    elem[0].creae_date = FAT_DATE( VIRFAT_DATE_DD_MM_YYYY );
    elem[0].acc_date = elem[0].write_date = virfat_cur_date;
    elem[0].write_time = virfat_cur_time;
    elem[0].cluster1_HI = 0;
    elem[0].size = (uint32_t)512 * virfat_rootdir[i].size;
    
    for(uint16_t j=0; j<11; j++)elem[0].name[j] = virfat_rootdir[i].name[j];
    elem[0].cluster1_LO = file_addr;
    file_addr += virfat_rootdir[i].size;
    elem++;
  }
  for(; i<VIRFAT_ROOTENT; i++, cnt++){
    if(cnt == 16)return;
    for(uint16_t j=0; j<sizeof(dir_elem); j++){ ((uint8_t*)elem)[j] = 0; }
    for(uint16_t j=0; j<11; j++)elem[0].name[j] = 0;
    elem[0].cluster1_HI = 0xFFFF;
    elem[0].cluster1_LO = 0xFFFF;
    elem[0].size = 0;
    elem++;
  }
}
static inline void _virfat_read_data(uint8_t *buf, uint32_t addr){
  uint16_t file_addr = 1;
  addr = (addr - virfat_datastart);
  for(uint16_t j=0; j<VIRFAT_FILES_TOTAL; j++){
    if( (addr >= file_addr) && (addr < (file_addr + virfat_rootdir[j].size)) ){
      virfat_rootdir[j].file_read(buf, addr - file_addr, j);
      return;
    }
    file_addr += virfat_rootdir[j].size;
  }
  for(uint16_t i=0; i<512; i++){
    *(buf++) = 0xFF;
  }
}

/////////////////////////////////////////////////////
/////// Write  //////////////////////////////////////
/////////////////////////////////////////////////////
#define _virfat_write_pbr(buf,  addr)
#define _virfat_write_fat(buf,  addr)
//static uint16_t virfat_cur_date = FAT_DATE( VIRFAT_DATE_DD_MM_YYYY );
//static uint16_t virfat_cur_time = FAT_TIME( VIRFAT_TIME_HH_MM_SS );
#ifndef VIRFAT_TIME_CALLBACK
  #define  _virfat_write_root(buf, addr)
#else
static inline void _virfat_write_root(uint8_t *buf, uint32_t addr){
  uint16_t date, time, change_flag = 0;
  dir_elem *elem = (dir_elem*)buf;
  for(uint16_t i=0; i<16; i++){
    if(elem[i].name[0] == 0)continue;
    date = elem[0].acc_date;
    if(elem[i].write_date > date)date = elem[i].write_date;
    if(date >= virfat_cur_date){
      if(date > virfat_cur_date)change_flag = 1;
      virfat_cur_date = date;
      time = elem[i].write_time;
      if(time > virfat_cur_time){
        virfat_cur_time = time;
        change_flag = 1;
      }
    }
  }
  if(change_flag)virfat_time_callback(virfat_cur_date, virfat_cur_time);
}
#endif

static inline void _virfat_write_data(uint8_t *buf, uint32_t addr){
  uint16_t file_addr = 1;
  addr = (addr - virfat_datastart);
  for(uint16_t j=0; j<VIRFAT_FILES_TOTAL; j++){
    if( (addr >= file_addr) && (addr < (file_addr + virfat_rootdir[j].size)) ){
      virfat_rootdir[j].file_write(buf, addr - file_addr, j);
      return;
    }
    file_addr += virfat_rootdir[j].size;
  }
}

void virfat_init(){
  uint32_t sectotal = 0;
  for(uint16_t i=0; i<VIRFAT_FILES_TOTAL; i++){
    sectotal += virfat_rootdir[i].size;
  }
  if(sectotal < 0x2000)sectotal = 0x2000;
  virfat_rootstart = virfat_fatstart + ((sectotal * 2 + 511) / 512);
}

uint32_t virfat_getsize(){return 512UL*(virfat_rootstart-virfat_fatstart)/2;}

void virfat_read(uint8_t *buf, uint32_t addr){
  if(addr < virfat_fatstart){
    _virfat_read_pbr( buf, addr);
  }else if(addr < virfat_rootstart){
    _virfat_read_fat( buf, addr);
  }else if(addr < virfat_datastart){
    _virfat_read_root(buf, addr);
  }else{
    _virfat_read_data(buf, addr);
  }
}

void virfat_write(uint8_t *buf, uint32_t addr){
  if(addr < virfat_fatstart){
    _virfat_write_pbr( buf, addr);
  }else if(addr < virfat_rootstart){
    _virfat_write_fat( buf, addr);
  }else if(addr < virfat_datastart){
    _virfat_write_root(buf, addr);
  }else{
    _virfat_write_data(buf, addr);
  }
}

#endif
