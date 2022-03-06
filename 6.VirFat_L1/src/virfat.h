#ifndef __VIRFAT_H__
#define __VIRFAT_H__

//#define VIRFAT_READONLY
#define VIRFAT_DATE_DD_MM_YYYY 1,3,2022
#define VIRFAT_FILES_TOTAL	3
#define VIRFAT_JMPBOOT		{0xEB, 0x3C, 0x90}
#define VIRFAT_OEMNAME		"virfat  "
#define VIRFAT_VOLID		0xFC561629
#define VIRFAT_VOLNAME		"Virtual FAT"
#define VIRFAT_FSNAME		"FAT16   "

#define VIRFAT_TOTSECT 0x2000
#define VIRFAT_ROOTENT (( VIRFAT_FILES_TOTAL + 15 ) &~ 15)
#define _FAT_DATE(day, month, year) ( ((((year)-1980) & 127)<<9) | (((month)&15)<<5) | ((day) & 31) )
#define FAT_DATE(x) _FAT_DATE(x)
//TODO: cc_date

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
  .fatsize = (VIRFAT_TOTSECT / 512 / 1 * 2), //(TotSect / BytesPerSec / SecPerClust * 2)
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
#define VIRFAT_ROOT_START	(VIRFAT_FAT_START + (VIRFAT_TOTSECT / 512 / 1 * 2))
#define VIRFAT_DATA_START	(VIRFAT_ROOT_START + VIRFAT_ROOTENT / 16)

typedef struct{
  char *name;
  uint16_t date;
  uint16_t addr;
  uint8_t sector[512];
}virfat_file_t;

static virfat_file_t virfat_rootdir[ VIRFAT_FILES_TOTAL ];

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
void _virfat_read_pbr(uint8_t *buf, uint32_t addr){
  if(addr >= 1)return;
  uint16_t i;
  for(i=0; i<sizeof(virfat_pbr); i++){
    buf[i] = ((uint8_t*)&virfat_pbr)[i];
  }
  for(; i< 0x01FE; i++)buf[i] = 0;
  buf[0x01FE] = 0x55;
  buf[0x01FF] = 0xAA;
}
void _virfat_read_fat(uint8_t *buf, uint32_t addr){
  if(addr < 1)return;
  if(addr >= VIRFAT_ROOT_START)return;
  uint16_t i;
  uint16_t *fat = (uint16_t*)buf;
  if(addr == 1){
    fat[0] = 0xFFF8;
    fat[1] = 0xFFFF;
    for(i=0; i<VIRFAT_FILES_TOTAL; i++)fat[i+2] = 0xFFFF;
    fat = fat + 2 + VIRFAT_FILES_TOTAL;
    for(i = i+2; i<(512/2); i++){
      *(fat++) = CLUST_BROKEN;
    }
    return;
  }
  for(i=0; i<(512/2); i++){
    *(fat++) = CLUST_BROKEN;
  }
}
void _virfat_read_root(uint8_t *buf, uint32_t addr){
  if(addr < VIRFAT_ROOT_START)return;
  if(addr >= VIRFAT_DATA_START)return;
  uint16_t i;
  dir_elem *elem = (dir_elem*)buf;
  
  i = (addr - VIRFAT_ROOT_START) * 16;
  elem += i;
  uint16_t cnt = 0;
  
  for(; i<VIRFAT_FILES_TOTAL; i++, cnt++){
    if(cnt == 16)return;
    for(uint16_t j=0; j<sizeof(dir_elem); j++){ ((uint8_t*)elem)[j] = 0; }
    elem[0].dir_attr = 0x20;
    elem[0].creae_date = FAT_DATE( VIRFAT_DATE_DD_MM_YYYY );
    elem[0].cluster1_HI = 0;
    elem[0].size = 512;
    
    for(uint16_t j=0; j<11; j++)elem[0].name[j] = virfat_rootdir[i].name[j];
    elem[0].cluster1_LO = virfat_rootdir[i].addr;
    elem[0].acc_date = elem[0].write_date = virfat_rootdir[i].date;
    elem++;
  }
  for(; i<VIRFAT_ROOTENT; i++, cnt++){
    if(cnt == 16)return;
    for(uint16_t j=0; j<sizeof(dir_elem); j++){ ((uint8_t*)elem)[j] = 0; }
    for(uint16_t j=0; j<11; j++)elem[0].name[j] = 0;
    elem[0].cluster1_HI = 0xFF;
    elem[0].cluster1_LO = 0xFF;
    elem[0].size = 0;
    elem++;
  }
}
void _virfat_read_data(uint8_t *buf, uint32_t addr){
  if(addr < VIRFAT_DATA_START)return;
  //addr = VIRFAT_DATA_START + (addr-2);
  addr = (addr - VIRFAT_DATA_START) + 2;
  for(uint16_t j=0; j<VIRFAT_FILES_TOTAL; j++){
    if( addr == virfat_rootdir[j].addr ){
      for(uint16_t i=0; i<512; i++){
        *(buf++) = virfat_rootdir[j].sector[i];
      }
      return;
    }
  }
  for(uint16_t i=0; i<512; i++){
    *(buf++) = (i % 20)+'A';
  }
}
/////////////////////////////////////////////////////
/////// Write  //////////////////////////////////////
/////////////////////////////////////////////////////
void _virfat_write_fat (uint8_t *buf, uint32_t addr){
  if(addr < 2)return;
  if(addr >= VIRFAT_ROOT_START)return;
  return;
}
void _virfat_write_root(uint8_t *buf, uint32_t addr){
  if(addr < VIRFAT_ROOT_START)return;
  if(addr >= VIRFAT_DATA_START)return;
  uint16_t i;
  dir_elem *elem = (dir_elem*)buf;
  
  i = (addr - VIRFAT_ROOT_START) * 16;
  if(i >= VIRFAT_FILES_TOTAL)return;
  elem += i;
  
  for(uint16_t j=0; j<16; j++){
    if(i >= VIRFAT_FILES_TOTAL)return;
    virfat_rootdir[j].date = elem[0].acc_date;
    virfat_rootdir[j].addr = elem[0].cluster1_LO;
    i++;
    elem++;
  }
}
void _virfat_write_data(uint8_t *buf, uint32_t addr){
  if(addr < VIRFAT_DATA_START)return;
}

uint32_t virfat_getsize(){return VIRFAT_TOTSECT;}

void virfat_init(){
  for(uint16_t i=0; i<VIRFAT_FILES_TOTAL; i++){
    virfat_rootdir[i].date = FAT_DATE( VIRFAT_DATE_DD_MM_YYYY );
    virfat_rootdir[i].addr = i+2;
    virfat_rootdir[i].sector[0] = i+'0';
    virfat_rootdir[i].sector[1] = virfat_rootdir[i].addr + '0';
    for(uint16_t j=2; j<512; j++)virfat_rootdir[i].sector[j] = (i % 10)+'A';
  }
  virfat_rootdir[0].name = "1AAAAAAATXT";
  virfat_rootdir[1].name = "2FILE2  TXT";
  virfat_rootdir[2].name = "3ANOTHERTXT";
}

void virfat_read(uint8_t *buf, uint32_t addr){
  _virfat_read_pbr( buf, addr);
  _virfat_read_fat( buf, addr);
  _virfat_read_root(buf, addr);
  _virfat_read_data(buf, addr);
}

void virfat_write(uint8_t *buf, uint32_t addr){
  _virfat_write_fat (buf, addr);
  _virfat_write_root(buf, addr);
  _virfat_write_data(buf, addr);
}

//uint32_t cur_sect_addr = 0;
//uint8_t cur_sect[512];

#endif
