#include <stdio.h>
#include <stdlib.h>
#include <string.h>
unsigned long logpow(unsigned long x,unsigned long p){
  // logarithmic exponentiation ftw !
  unsigned long r = 1;
  while(p){
    if(p&1) r *=x;
    x*=x;
    p >>=1;      
  }
  return(r);
}

unsigned char crc7_syndrome_table[256] = {
    0x00, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x36, 0x3f,
    0x48, 0x41, 0x5a, 0x53, 0x6c, 0x65, 0x7e, 0x77,
    0x19, 0x10, 0x0b, 0x02, 0x3d, 0x34, 0x2f, 0x26,
    0x51, 0x58, 0x43, 0x4a, 0x75, 0x7c, 0x67, 0x6e,
    0x32, 0x3b, 0x20, 0x29, 0x16, 0x1f, 0x04, 0x0d,
    0x7a, 0x73, 0x68, 0x61, 0x5e, 0x57, 0x4c, 0x45,
    0x2b, 0x22, 0x39, 0x30, 0x0f, 0x06, 0x1d, 0x14,
    0x63, 0x6a, 0x71, 0x78, 0x47, 0x4e, 0x55, 0x5c,
    0x64, 0x6d, 0x76, 0x7f, 0x40, 0x49, 0x52, 0x5b,
    0x2c, 0x25, 0x3e, 0x37, 0x08, 0x01, 0x1a, 0x13,
    0x7d, 0x74, 0x6f, 0x66, 0x59, 0x50, 0x4b, 0x42,
    0x35, 0x3c, 0x27, 0x2e, 0x11, 0x18, 0x03, 0x0a,
    0x56, 0x5f, 0x44, 0x4d, 0x72, 0x7b, 0x60, 0x69,
    0x1e, 0x17, 0x0c, 0x05, 0x3a, 0x33, 0x28, 0x21,
    0x4f, 0x46, 0x5d, 0x54, 0x6b, 0x62, 0x79, 0x70,
    0x07, 0x0e, 0x15, 0x1c, 0x23, 0x2a, 0x31, 0x38,
    0x41, 0x48, 0x53, 0x5a, 0x65, 0x6c, 0x77, 0x7e,
    0x09, 0x00, 0x1b, 0x12, 0x2d, 0x24, 0x3f, 0x36,
    0x58, 0x51, 0x4a, 0x43, 0x7c, 0x75, 0x6e, 0x67,
    0x10, 0x19, 0x02, 0x0b, 0x34, 0x3d, 0x26, 0x2f,
    0x73, 0x7a, 0x61, 0x68, 0x57, 0x5e, 0x45, 0x4c,
    0x3b, 0x32, 0x29, 0x20, 0x1f, 0x16, 0x0d, 0x04,
    0x6a, 0x63, 0x78, 0x71, 0x4e, 0x47, 0x5c, 0x55,
    0x22, 0x2b, 0x30, 0x39, 0x06, 0x0f, 0x14, 0x1d,
    0x25, 0x2c, 0x37, 0x3e, 0x01, 0x08, 0x13, 0x1a,
    0x6d, 0x64, 0x7f, 0x76, 0x49, 0x40, 0x5b, 0x52,
    0x3c, 0x35, 0x2e, 0x27, 0x18, 0x11, 0x0a, 0x03,
    0x74, 0x7d, 0x66, 0x6f, 0x50, 0x59, 0x42, 0x4b,
    0x17, 0x1e, 0x05, 0x0c, 0x33, 0x3a, 0x21, 0x28,
    0x5f, 0x56, 0x4d, 0x44, 0x7b, 0x72, 0x69, 0x60,
    0x0e, 0x07, 0x1c, 0x15, 0x2a, 0x23, 0x38, 0x31,
    0x46, 0x4f, 0x54, 0x5d, 0x62, 0x6b, 0x70, 0x79
};

typedef struct __attribute__ ((packed)) {
  
  unsigned char version           :2;
  unsigned char r1/*RESERVED*/      :6;
  unsigned char TAAC              :8;
  unsigned char NSAC              :8;
  unsigned char TRAN_SPEED        :8;
  unsigned int  CCC               :12;
  unsigned char READ_BL_LEN       :4;
  unsigned char READ_BL_PARTIAL   :1;
  unsigned char WRITE_BLK_MISALIGN:1;
  unsigned char READ_BLK_MISALIGN :1;
  unsigned char DSR_IMP           :1;
  unsigned char r2/*RESERVED*/      :2;
  unsigned int  C_SIZE            :12;
  unsigned char VDD_R_CURR_MIN    :3;
  unsigned char VDD_R_CURR_MAX    :3;
  unsigned char VDD_W_CURR_MIN    :3;
  unsigned char VDD_W_CURR_MAX    :3;
  unsigned char C_SIZE_MULT       :3;
  unsigned char ERASE_BLK_EN      :1;
  unsigned char SECTOR_SIZE       :7;
  unsigned char WP_GRP_SIZE       :7;
  unsigned char WP_GRP_ENABLE     :1;
  unsigned char r3/*RESERVED*/      :2;
  unsigned char R2W_FACTOR        :3;
  unsigned char WRITE_BL_LEN      :4;
  unsigned char WRITE_BL_PARTIAL  :1;
  unsigned char r4/*RESERVED*/      :5;
  unsigned char FILE_FORMAT_GRP   :1;
  unsigned char COPY              :1;
  unsigned char PERM_WRITE_PROTECT:1;
  unsigned char TMP_WRITE_PROTECT :1;
  unsigned char FILE_FORMAT       :2;
  unsigned char r5/*RESERVED*/      :2;
  unsigned char CRC               :7;
  unsigned char r6/*RESERVED*/      :1;

} SD_Card_CSD_Struct_t;

char crc7(char* buff,int size){
unsigned char crc = 0;
  unsigned int i;
  for(i=0;i<size;i++)
    crc = crc7_syndrome_table[(crc << 1) ^ buff[i]];
  return(crc);
}

int main(){

  SD_Card_CSD_Struct_t* mcsd = (SD_Card_CSD_Struct_t*)malloc(sizeof(SD_Card_CSD_Struct_t));


  //unsigned char buff[16]={38,0,50,95,90,131,174,254,251,207,255,146,128,64,223};
  unsigned char buff[16]={0x00,0x26,0x00,0x32,0x5f,0x5a,0x83,0xae,0xfe,0xfb,0xcf,0xff,0x92,0x80,0x40,0xdf};
  unsigned char buff2[16]={0x32,0x00,0x26,0x00,0xae,0x83,0x5a,0x5f,0xff,0xcf,0xfb,0xfe,0xdf,0x40,0x80,0x92};

  //unsigned char buff[16]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

  memcpy(mcsd,buff,16);

  printf("crc 0x%x 0x%x\n",(crc7(buff,15)<<1) & 1,crc7(buff2,15));

    //{0,0,0,0,0,90,131,174,254,251,207};
  unsigned long MMC_CardSize;
  unsigned int  mmc_C_SIZE,mmc_READ_BL_LEN,mmc_C_SIZE_MULT;

  int i,j;

  unsigned int c=(buff[8]>>6)|(buff[7]<<2)|((buff[6]&0x3)<<10);
  unsigned int l=(buff[5]& 0xf);
  unsigned int m=(buff[9] & 0x3 )<<1 | (buff[10]&(0x80)>>7);

  printf(" str sz:%d\n",mcsd->C_SIZE );
  memcpy(mcsd,buff2,16);
  printf(" str sz:%d\n",mcsd->C_SIZE );
  /*
  mmc_C_SIZE =c;
  mmc_READ_BL_LEN=l;
  mmc_C_SIZE_MULT=m;
  

  printf("c :0x%x %d\n",c,c);
  printf("l :0x%x %d\n",l,l);
  printf("m :0x%x %d\n",m,m);
  printf("%ld\n",(unsigned long)((1+c)*logpow(2,m+2)*logpow(2,l)));
  
  MMC_CardSize = (mmc_C_SIZE + 1);
  // power function with base 2 is better with a loop
  // i = (pow(2,mmc_C_SIZE_MULT+2)+0.5);
  for(i = 2,j=mmc_C_SIZE_MULT+2; j>1; j--)
    i <<= 1;
  MMC_CardSize *= i;
  // power function with base 2 is better with a loop
  //i = (pow(2,mmc_READ_BL_LEN)+0.5);
  for(i = 2,j=mmc_READ_BL_LEN; j>1; j--)
    i <<= 1;
  MMC_CardSize *= i;

   printf("calc mmc %ld\n",MMC_CardSize);
  //CSD.READ_BL_LEN = (buff[5]& 0xf);
  // CSD.C_SIZE_MULT = (buff[9] & 0x3 )<<1 | (buff[10]&(0x80)>>7);*/

}
