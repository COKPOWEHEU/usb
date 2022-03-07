#ifndef __VIRFAT_H__
#define __VIRFAT_H__
#include <inttypes.h>

#if 0==1
//////////////////////////////////////////////////////////////
//  User-defined settings, variables and callbacks of FAT ////
//////////////////////////////////////////////////////////////
#define VIRFAT_READONLY // (#define / not define)
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
  uint16_t addr_st;				//1st sector in FAT
  uint16_t addr_en;				//(last+1)sector in FAT (based on file size)
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
void virfat_time_callback(virfat_date_t *date, virfat_time_t *time){...}

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
#ifndef VIRFAT_VOLNAME
  #define VIRFAT_VOLNAME		"VIRTUAL_FAT"
#endif
#ifndef VIRFAT_JMPBOOT
  #define VIRFAT_JMPBOOT		{0xEB, 0x3C, 0x90}
#endif
#ifndef VIRFAT_OEMNAME
  #define VIRFAT_OEMNAME		"virfat  "
#endif
#define VIRFAT_FSNAME		"FAT16   "

//////////////////////////////////////////////////////////////
//  DEMO file controlling LEDs ///////////////////////////////
//////////////////////////////////////////////////////////////
void demo_led_read(uint8_t *buf, uint32_t addr, uint16_t file_idx){
  for(uint16_t i=1; i<512; i++)buf[i] = ' ';
  if(file_idx == 0){
    if( GPI_ON(RLED) )buf[0] = '1'; else buf[0] = '0';
  }else{
    if( GPI_ON(GLED) )buf[0] = '0'; else buf[0] = '0';
  }
}
void demo_led_write(uint8_t *buf, uint32_t addr, uint16_t file_idx){
  if(file_idx == 0){
    if(buf[0] == '1')GPO_ON(RLED); else GPO_OFF(RLED);
  }else{
    if(buf[0] == '1')GPO_ON(GLED); else GPO_OFF(GLED);
  }
}
//////////////////////////////////////////////////////////////
// DEMO l6og file ////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//0..11 => 12
void u32tobuf(uint32_t val, uint8_t *buf){
  buf[10] = (val % 10)+'0'; val /=10;
  for(uint8_t *tmp=buf+9; tmp>=buf; tmp--){
    if(val == 0)tmp[0] = ' '; else tmp[0] = (val % 10)+'0';
    val /= 10;
  }
  buf[11] = '\t';
}

struct demolog_data_t{
  uint16_t x;
  uint16_t y;
};
#define DEMO_DATASIZE	1000
const struct demolog_data_t demo_log[DEMO_DATASIZE] = {
  [0]   = {0,   0},
  [1]   = {1,   1},
  [10]  = {10,  10},
  [100] = {100, 100},
  [500] = {500, 500},
  [999] = {999, 999},
};
#define DEMO_HEADLEN 32
const char demo_head[ DEMO_HEADLEN ] = "Some header string\r\nx\t\t\ty\t\t\tz\r\n";
#define DEMO_DATALEN	(12*3 + 1) //37
#define DEMO_LINESINSEC	(512 / DEMO_DATALEN) //13
#define DEMO_LINESINHEAD (DEMO_HEADLEN / DEMO_DATALEN) //(sizeof(demo_head) / DEMO_DATALEN)

void demo_log_read(uint8_t *buf, uint32_t addr, uint16_t file_idx){
  uint16_t pos = 0;
  if(addr*512 < sizeof(demo_head)){
    char *head_start = (char*)&demo_head[addr*512];
    while(head_start[0]!=0){
      buf[pos] = head_start[0];
      pos++; head_start++;
      if(pos == 512)return;
    }
  }
  
  uint32_t st = addr * DEMO_LINESINSEC;
  uint32_t en = st + DEMO_LINESINSEC - DEMO_LINESINHEAD;
  if(st >= DEMO_LINESINHEAD){
    st -= DEMO_LINESINHEAD;
  }
  if(en > DEMO_DATASIZE)en = DEMO_DATASIZE;
  for(;st < en; st++){
    u32tobuf(st, &buf[pos]); pos+=12;
    u32tobuf(demo_log[st].x, &buf[pos]); pos+=12;
    u32tobuf(demo_log[st].y, &buf[pos]); pos+=12;
    buf[pos-1] = '\r';
    buf[pos] = '\n';
    pos++;
  }
  if(pos < 2)pos=2;
  for(pos -= 2; pos < 510; pos++)buf[pos] = ' ';
  buf[510] = '\r';
  buf[511] = '\n';
}
#define DEMO_LOG_SIZE ((DEMO_DATASIZE + DEMO_LINESINHEAD + (DEMO_LINESINSEC-1)) / DEMO_LINESINSEC)

//////////////////////////////////////////////////////////////
//  Dummy file read/write function
//////////////////////////////////////////////////////////////
void virfat_file_dummy_read(uint8_t *buf, uint32_t addr, uint16_t file_idx){
  for(uint16_t i=0; i<512; i++)buf[i] = 0;
}
void virfat_file_dummy_write(uint8_t *buf, uint32_t addr, uint16_t file_idx){}

//////////////////////////////////////////////////////////////
typedef void(*virfat_callback)(uint8_t *buf, uint32_t addr, uint16_t file_idx);
typedef struct{
  char *name;
  virfat_callback file_read;
  virfat_callback file_write;
  uint16_t addr_st;
  uint16_t addr_en;
}virfat_file_t;

static const virfat_file_t virfat_rootdir[] = {
  {
    .name = "RLED    TXT",
    .file_read = demo_led_read,
    .file_write = demo_led_write,
    .addr_st = 0,
    .addr_en = 1,
  },
  {
    .name = "GLED    TXT",
    .file_read = demo_led_read,
    .file_write = demo_led_write,
    .addr_st = 1,
    .addr_en = 2,
  },
  {
    .name = "LOG     TXT",
    .file_read = demo_log_read,
    .file_write = virfat_file_dummy_write,
    .addr_st = 2,
    .addr_en = 2 + DEMO_LOG_SIZE,
  },
  {
    .name = "DUMMY   TXT",
    .file_read = virfat_file_dummy_read,
    .file_write = virfat_file_dummy_write,
    .addr_st = 2 + DEMO_LOG_SIZE,
    .addr_en = 2 + DEMO_LOG_SIZE + 1,
  },
};
#define VIRFAT_SECTOR_LAST	(2 + DEMO_LOG_SIZE + 2)

#define VIRFAT_FILES_TOTAL	( sizeof(virfat_rootdir) / sizeof(virfat_file_t) )
#if (VIRFAT_SECTOR_LAST < 0x2000)
  #define VIRFAT_TOTSECT 0x2000
#else
  #define VIRFAT_TOTSECT (((VIRFAT_SECTOR_LAST) + 511) &~ 511)
#endif
#define VIRFAT_ROOTENT (( VIRFAT_FILES_TOTAL + 15 ) &~ 15)
#define VIRFAT_FAT_SIZE (VIRFAT_TOTSECT / 512 / 1 * 2)

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

static inline void virfat_time_callback(virfat_date_t *date, virfat_time_t *time){}

#pragma pack(push, 1)
static const struct __attribute__((__packed__)){
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
}virfat_pbr = {
  .JmpBoot = VIRFAT_JMPBOOT,
  .OEMname = VIRFAT_OEMNAME,
  .BytesPerSec = 512,
  .SecPerClust = 1,
  .SecReserved = 1,
  .NumFats = 1,
  .RootEntCnt = VIRFAT_ROOTENT,
  .TotSect = VIRFAT_TOTSECT,
  .DriveType = 0xF8,
  .fatsize = VIRFAT_FAT_SIZE, //(TotSect / BytesPerSec / SecPerClust * 2)
  .SecPerTrak = 0x0020, //?
  .NumHeads = 0x0040, //?
  .SecHidden = 0,
  .TotSect32 = 0,
  .DriveNum = 0x80, //?
  .NTErrFlag = 0, //?
  .BootSig = 0x29,//0, //?
  .VolID = VIRFAT_VOLID,
  .VolName = VIRFAT_VOLNAME,
  .FSName = VIRFAT_FSNAME,
};
#pragma pack(pop)

#define VIRFAT_FAT_START	1
#define VIRFAT_ROOT_START	(VIRFAT_FAT_START + VIRFAT_FAT_SIZE)
#define VIRFAT_DATA_START	(VIRFAT_ROOT_START + VIRFAT_ROOTENT / 16)

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

/////////////////////////////////////////////////////
/////// Read  ///////////////////////////////////////
/////////////////////////////////////////////////////
static inline int _virfat_read_pbr(uint8_t *buf, uint32_t addr){
  if(addr >= 1)return 0;
  uint16_t i;
  for(i=0; i<sizeof(virfat_pbr); i++){
    buf[i] = ((uint8_t*)&virfat_pbr)[i];
  }
  for(; i< 0x01FE; i++)buf[i] = 0;
  buf[0x01FE] = 0x55;
  buf[0x01FF] = 0xAA;
  return 1;
}
static inline int _virfat_read_fat(uint8_t *buf, uint32_t addr){
  if(addr < 1)return 0;
  if(addr >= VIRFAT_ROOT_START)return 0;
  uint16_t *fat = (uint16_t*)buf;
  uint16_t *fat_end = (uint16_t*)(buf + 512);
  uint16_t sec = (addr-1)*(512/2);
  if(addr == 1){
    fat[0] = 0xFFF8;
    fat[1] = 0xFFFF;
    fat += 2;
    sec += 2;
  }
  sec -=2;
  for(; fat < fat_end; fat++, sec++){
    if( sec >= virfat_rootdir[VIRFAT_FILES_TOTAL-1].addr_en){
      fat[0] = CLUST_BROKEN;
    }else{
      for(uint16_t i=0; i<VIRFAT_FILES_TOTAL; i++){
        if( sec <  virfat_rootdir[i].addr_st )continue;
        if( sec >= virfat_rootdir[i].addr_en )continue;
        if( sec == (virfat_rootdir[i].addr_en - 1) ){fat[0] = 0xFFFF; break;}
        fat[0] = sec + 3;
        break;
      }
    }
  }
  return 1;
}
static inline int _virfat_read_root(uint8_t *buf, uint32_t addr){
  if(addr < VIRFAT_ROOT_START)return 0;
  if(addr >= VIRFAT_DATA_START)return 0;
  uint16_t i;
  dir_elem *elem = (dir_elem*)buf;
  
  i = (addr - VIRFAT_ROOT_START) * 16;
  elem += i;
  uint16_t cnt = 0;
  
  for(; i<VIRFAT_FILES_TOTAL; i++, cnt++){
    if(cnt == 16)return 1;
    for(uint16_t j=0; j<sizeof(dir_elem); j++){ ((uint8_t*)elem)[j] = 0; }
    elem[0].dir_attr = 0x20;
    elem[0].creae_date = FAT_DATE( VIRFAT_DATE_DD_MM_YYYY );
    elem[0].acc_date = elem[0].write_date = virfat_cur_date;
    elem[0].cluster1_HI = 0;
    elem[0].size = (uint32_t)512*(virfat_rootdir[i].addr_en - virfat_rootdir[i].addr_st);
    
    for(uint16_t j=0; j<11; j++)elem[0].name[j] = virfat_rootdir[i].name[j];
    elem[0].cluster1_LO = virfat_rootdir[i].addr_st + 2;
    elem++;
  }
  for(; i<VIRFAT_ROOTENT; i++, cnt++){
    if(cnt == 16)return 1;
    for(uint16_t j=0; j<sizeof(dir_elem); j++){ ((uint8_t*)elem)[j] = 0; }
    for(uint16_t j=0; j<11; j++)elem[0].name[j] = 0;
    elem[0].cluster1_HI = 0xFFFF;
    elem[0].cluster1_LO = 0xFFFF;
    elem[0].size = 0;
    elem++;
  }
  return 1;
}

static inline int _virfat_read_data(uint8_t *buf, uint32_t addr){
  if(addr < VIRFAT_DATA_START)return 0;
  
  addr = (addr - VIRFAT_DATA_START);
  for(uint16_t j=0; j<VIRFAT_FILES_TOTAL; j++){
    if( (addr >= virfat_rootdir[j].addr_st) && (addr < virfat_rootdir[j].addr_en) ){
      virfat_rootdir[j].file_read(buf, addr - virfat_rootdir[j].addr_st, j);
      return 1;
    }
  }
  //any data outside the calculated area are 'broken'. The Host can't change them
  for(uint16_t i=0; i<512; i++){
    *(buf++) = 0xFF;
  }
  return 1;
}
/////////////////////////////////////////////////////
/////// Write  //////////////////////////////////////
/////////////////////////////////////////////////////
static inline int _virfat_write_fat (uint8_t *buf, uint32_t addr){
  if(addr < 2)return 0;
  if(addr >= VIRFAT_ROOT_START)return 0;
  return 1;
}
static inline int _virfat_write_root(uint8_t *buf, uint32_t addr){
  if(addr < VIRFAT_ROOT_START)return 0;
  if(addr >= VIRFAT_DATA_START)return 0;
  uint16_t i;
  dir_elem *elem = (dir_elem*)buf;
  
  i = (addr - VIRFAT_ROOT_START) * 16;
  if(i >= VIRFAT_FILES_TOTAL)return 1; //these records are in 'root' dir but not emulated
  elem += i;
  
  for(uint16_t j=0; j<16; j++){
    if(i >= VIRFAT_FILES_TOTAL)return 1;
    virfat_cur_date = elem[0].acc_date;
    virfat_cur_time = elem[0].write_time;
    virfat_time_callback((virfat_date_t*)&(elem[0].acc_date), (virfat_time_t*)&(elem[0].write_time));
    //TODO: set date-time callback
    //virfat_rootdir[j].addr = elem[0].cluster1_LO;
    i++;
    elem++;
  }
  return 1;
}
static inline int _virfat_write_data(uint8_t *buf, uint32_t addr){
  if(addr < VIRFAT_DATA_START)return 0;
  if(addr < VIRFAT_DATA_START)return 0;
  
  addr = (addr - VIRFAT_DATA_START);
  for(uint16_t j=0; j<VIRFAT_FILES_TOTAL; j++){
    if( (addr >= virfat_rootdir[j].addr_st) && (addr < virfat_rootdir[j].addr_en) ){
      virfat_rootdir[j].file_write(buf, addr - virfat_rootdir[j].addr_st, j);
      return 1;
    }
  }
  return 1;
}

uint32_t virfat_getsize(){return VIRFAT_TOTSECT;}

void virfat_read(uint8_t *buf, uint32_t addr){
  if(_virfat_read_pbr( buf, addr) )return;
  if(_virfat_read_fat( buf, addr) )return;
  if(_virfat_read_root(buf, addr) )return;
  if(_virfat_read_data(buf, addr) )return;
}

void virfat_write(uint8_t *buf, uint32_t addr){
  if(_virfat_write_fat (buf, addr) )return;
  if(_virfat_write_root(buf, addr) )return;
  if(_virfat_write_data(buf, addr) )return;
}

//uint32_t cur_sect_addr = 0;
//uint8_t cur_sect[512];

#endif
