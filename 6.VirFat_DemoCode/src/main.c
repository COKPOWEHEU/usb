#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>

#pragma pack(push, 1)
typedef struct __attribute__((__packed__)){
  uint8_t JmpBoot[3];  //{0xEB, 0x3C, 0x90}
  uint8_t OEMname[8];  //имя программы, создавшей ФС
  uint16_t BytesPerSec;//512
  uint8_t SecPerClust; //1
  uint16_t SecReserved;//1
  uint8_t NumFats;     //1
  uint16_t RootEntCnt; //кратно 16
  uint16_t TotSect;    //от 0x1000 до 0x10000
  uint8_t DriveType;   //0xF8 - HDD, 0xF0 - FDD
  uint16_t fatsize;    //(TotSect / BytesPerSec / SecPerClust * 2)
  uint16_t SecPerTrak; //0x0020 ?
  uint16_t NumHeads;   //0x0040 ?
  uint32_t SecHidden;  //0
  uint32_t TotSect32;  //0
  
  uint8_t DriveNum;    //0x80 ?
  uint8_t NTErrFlag;   //0
  uint8_t BootSig;     //0x29 ?
  uint32_t VolID;      //по сути, UUID раздела, берем любое
  char VolName[11];    //имя раздела (часте6нько игнорируется операционками)
  char FSName[8];      //"FAT16   " и только так!
  uint8_t reserved[448];
  uint16_t EOS_AA55;   //контрольное значение 0xAA55 (в little-endian)
}fat_pbr_t;
fat_pbr_t pbr;

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

uint32_t totsect, sectsize, clustsize;
uint32_t fatstart, fatsize, nfats;
uint32_t rootstart, rootsize;
uint32_t datastart = 0;
void pbr_decode(FILE *pf, char verbose){
  datastart = 0;
  if(pf == NULL)return;
  fseek(pf, 0, SEEK_SET);
  fread(&pbr, sizeof(pbr), 1, pf);
  
  totsect = pbr.TotSect;
  if(totsect == 0)totsect = pbr.TotSect32;
  
  if(totsect < 0x0FFF){printf("Error: FS is FAT12\n"); return;}
  if(totsect > 0xFFFF){printf("Error: FS if FAT32\n"); return;}
  
  sectsize = pbr.BytesPerSec;
  clustsize = sectsize * pbr.SecPerClust;
  //fat
  fatstart = pbr.SecReserved;
  nfats = pbr.NumFats;
  fatsize = pbr.fatsize;
  //root
  rootstart = fatstart + fatsize*nfats;
  rootsize = pbr.RootEntCnt / (sectsize / 32);
  //data
  datastart = rootstart + rootsize;
  if(verbose){
    printf("OEM name = [%8.8s]\n", pbr.OEMname);
    printf("Sector size = %i bytes\n", sectsize);
    printf("Cluster size = %i sectors = %i bytes\n", pbr.SecPerClust, clustsize);
    printf("reserved = %i sectors\n", pbr.SecReserved);
    printf("hidden = %i sectors\n", pbr.SecHidden);
    printf("Max files in root dir = %i\n", pbr.RootEntCnt);
    printf("Total size = %i sectors = %i bytes\n", totsect, totsect*sectsize);
    printf("Drive type = %.2X (%s)\n", (uint8_t)pbr.DriveType, (pbr.DriveType==0xF8 ? "HDD" : "FDD"));
    printf("Volume ID = %.8X\n", pbr.VolID);
    printf("Volume name = [%11.11s]\n", pbr.VolName);
    printf("FS name = [%8.8s]\n", pbr.FSName);
    printf("AA55 = %.4X\n", pbr.EOS_AA55);
    printf("---\n");
    printf("%i FATs: [%.4X]/[%.8X] (%i sectors / %i bytes)\n", pbr.NumFats, fatstart, fatstart*sectsize, fatsize, fatsize*sectsize);
    printf("Root dir: [%.4X]/[%.8X] (%i sectors / %i bytes)\n", rootstart, rootstart*sectsize, rootsize, rootsize*sectsize);
    printf("Data: [%.4X]/[%.8X]\n", datastart, datastart*sectsize);
    printf("---\n");
  }
}

void read_file(FILE *pf, uint32_t clust1){
  if(datastart == 0)return;
  if(pf == NULL)return;
  char data[ clustsize ];
  printf("Read file %i => %.8X\n", clust1, datastart*sectsize + (clust1-2)*clustsize);
  fseek(pf, datastart*sectsize + (clust1-2)*clustsize, SEEK_SET);
  fread(data, clustsize, 1, pf);
  
  //find last non-0 byte
  uint16_t lastdata = clustsize;
  for(lastdata=clustsize-1; lastdata>0; lastdata--)if(data[lastdata]!=0)break;
    
  for(int j=0; j<clustsize; j+=16){
    for(int i=0; i<16; i++)printf("%.2X ", (uint8_t)data[i+j]);
    printf("| ");
    for(int i=0; i<16; i++){
      if(isprint(data[i+j]))printf("%c", data[i+j]); else printf(" ");
    }
    printf("\n");
    if(j+16>=lastdata)break;
  }
  
  uint16_t clust_next;
  clust_next = fatstart*sectsize + clust1*2;
  fseek(pf, clust_next, SEEK_SET);
  fread(&clust_next, sizeof(clust_next), 1, pf);
  if((clust_next != 0) && (clust_next < 0xFFF6)){
    printf("next cluster %i\n", clust_next);
  }else{
    printf("next cluster: NONE\n");
  }
}
void read_dir(FILE *pf, int32_t clust1){
  if(datastart == 0)return;
  if(pf == NULL)return;
  uint32_t addr;
  if(clust1 > 0){
    addr = datastart*sectsize + (clust1-2)*clustsize;
  }else{ //special cast for Root dir
    addr = (rootstart + (-clust1) )*sectsize;
  }
  dir_elem file[ clustsize / sizeof(dir_elem) ];
  printf("Dir file %i => %.8X\n", clust1, addr);
  fseek(pf, addr, SEEK_SET);
  fread(file, clustsize, 1, pf);
  
  for(int i=0; i<sizeof(file)/sizeof(file[0]); i++){
    if(file[i].name[0] == 0)break;
    if(file[i].dir_attr == 0x0F){
      printf("LFN   ");
    }else{
      if(file[i].dir_attr & 0x01)printf("r"); else printf(" "); //read-only
      if(file[i].dir_attr & 0x02)printf("h"); else printf(" "); //hidden
      if(file[i].dir_attr & 0x04)printf("s"); else printf(" "); //system
      if(file[i].dir_attr & 0x08)printf("V"); else printf(" "); //special file VolumeID
      if(file[i].dir_attr & 0x10)printf("/"); else printf(" "); //directory
      if(file[i].dir_attr & 0x20)printf("a"); else printf(" "); //archive
    }
    printf(" ");
    printf("[%8.8s.%3.3s] (%i bytes) from %.4X%.4X\n", file[i].name, &(file[i].name[8]), file[i].size, file[i].cluster1_HI, file[i].cluster1_LO);
  }
  if(addr > 0){
    uint16_t clust_next;
    clust_next = fatstart*sectsize + clust1*2;
    fseek(pf, clust_next, SEEK_SET);
    fread(&clust_next, sizeof(clust_next), 1, pf);
    if((clust_next != 0) && (clust_next < 0xFFF6)){
      printf("next cluster %i\n", clust_next);
    }else{
      printf("next cluster: NONE\n");
    }
  }else{ //special cast for Root dir
    if((-clust1) < rootsize){
      printf("next cluster %i\n", clust1-1);
    }else{
      printf("next cluster: NONE\n");
    }
  }
}

int main(int argc, char **argv){
  FILE *pf = NULL;
  char *name = "fat16.img";
  for(int i=1; i<argc; i++){
    if((strcmp(argv[i], "-h")==0) || (strcmp(argv[i], "--help")==0)){
      printf("Usage: %s <file>\n", argv[0]);
      printf("  default <file> if [%s]\n", name);
      return 0;
    }else{
      name = argv[i];
    }
  }
  pf = fopen(name, "rb");
  if(pf == NULL){
    printf("Can not open file [%s]\n", name);
    return 0;
  }
  pbr_decode(pf, 0);
  char key = 0;
  int32_t clust;
  while(key != 'q'){
    printf("p          \t: show PBR\n");
    printf("d <cluster>\t: read directory by <cluster> (zero or negative values = root dir)\n");
    printf("f <cluster>\t: read file by <cluster>\n");
    printf("q          \t: quit\n");
    do{
      scanf("%c", &key);
    }while(isspace(key));
    
    if(key == 'p'){
      pbr_decode(pf, 1);
    }else if((key == 'd') || (key == 'f') ){
      scanf("%"SCNi32, &clust);
      if(key == 'd'){
        read_dir(pf, clust);
      }else{
        read_file(pf, clust);
      }
    }
  }
  fclose(pf);
}
