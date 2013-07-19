#include <stdio.h>

typedef struct __attribute__ ((packed)) {  
  unsigned char version           :2;
  unsigned char /*RESERVED*/      :6;
  unsigned char TAAC              :8;
  unsigned char NSAC              :8;
  unsigned char TRAN_SPEED        :8;
  unsigned int  CCC               :12;
  unsigned char READ_BL_LEN       :4;
  unsigned char READ_BL_PARTIAL   :1;
  unsigned char WRITE_BLK_MISALIGN:1;
  unsigned char READ_BLK_MISALIGN :1;
  unsigned char DSR_IMP           :1;
  unsigned char /*RESERVED*/      :2;
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
  unsigned char /*RESERVED*/      :2;
  unsigned char R2W_FACTOR        :3;
  unsigned char WRITE_BL_LEN      :4;
  unsigned char WRITE_BL_PARTIAL  :1;
  unsigned char /*RESERVED*/      :5;
  unsigned char FILE_FORMAT_GRP   :1;
  unsigned char COPY              :1;
  unsigned char PERM_WRITE_PROTECT:1;
  unsigned char TMP_WRITE_PROTECT :1;
  unsigned char FILE_FORMAT       :2;
  unsigned char /*RESERVED*/      :2;
  unsigned char CRC               :7;
  unsigned char /*RESERVED*/      :1; 
} SD_Card_CSD_Struct_t;

SD_Card_CSD_Struct_t SD_Card_CSD;
char buffer[16] = {0x0e, 0x00, 0x32, 0x5b, 0x59, 0x00, 0x00, 0x1d, 0x17, 0x7f, 0x80, 0x0a, 0x40, 0x00, 0x8d, 0x81};
int curr_block_sz=0;

void UnpackCSDv1(unsigned char* buff){
  SD_Card_CSD.version = (buff[0] & 0xc0 ) >> 6;
  /*125:120 R*/
  /*119:112*/   SD_Card_CSD.TAAC = buff[1];
  /*111:104*/ SD_Card_CSD.NSAC = buff[2];
  /*103:96*/ SD_Card_CSD.TRAN_SPEED =  buff[3];
  /*95:84*/ SD_Card_CSD.CCC = buff[4]<<4|((buff[5]& 0xf0) >> 4);
  /*83:80*/ SD_Card_CSD.READ_BL_LEN = (buff[5]& 0xf);
  curr_block_sz= 1<<SD_Card_CSD.READ_BL_LEN;
  /*79*/ SD_Card_CSD.READ_BL_PARTIAL = buff[6]&0x80 >> 7;
  /*78*/ SD_Card_CSD.WRITE_BLK_MISALIGN = (buff[6]& 0x40)>>6;
  /*77*/ SD_Card_CSD.READ_BLK_MISALIGN = (buff[6]& 0x20)>>5;
  /*76*/ SD_Card_CSD.DSR_IMP = (buff[6]&010)>>4;
  /*75:74 R*/
  /*73:62*/SD_Card_CSD.C_SIZE = (buff[8]>>6)|(buff[7]<<2)|((buff[6]&0x3)<<10);
  /*61:59*/SD_Card_CSD.VDD_R_CURR_MIN = (buff[8] & 0x38 )>> 3;
  /*58:56*/ SD_Card_CSD.VDD_R_CURR_MAX = (buff[8] & 0x7 );
  /*55:53*/ SD_Card_CSD.VDD_W_CURR_MIN = (buff[9] & 0xe0 )>> 5;;
   SD_Card_CSD.VDD_W_CURR_MAX = (buff[9] & 0x1c )>> 2;
   SD_Card_CSD.C_SIZE_MULT = (buff[9] & 0x3 )<<1 | (buff[10]&(0x80)>>7);
   SD_Card_CSD.ERASE_BLK_EN = (buff[10]&0x40)>>6;
   SD_Card_CSD.SECTOR_SIZE = (buff[10]&0x3f)<<1|((buff[11]&0x80)>>7);
   SD_Card_CSD.WP_GRP_SIZE = (buff[11]&0x7f);
   SD_Card_CSD.WP_GRP_ENABLE = (buff[12]&0x80)>>7;
   SD_Card_CSD.R2W_FACTOR = (buff[12]&0x1c) >>2;
   SD_Card_CSD.WRITE_BL_LEN = (buff[12]&0x03)<<2 |(buff[13] & 0xc0) >> 6; ;
   SD_Card_CSD.WRITE_BL_PARTIAL=((buff[13]&0x20)>>5);
   SD_Card_CSD.FILE_FORMAT_GRP= ((buff[14]&0x80)>>7);
   SD_Card_CSD.COPY= ((buff[14]&0x40)>>6);
   SD_Card_CSD.PERM_WRITE_PROTECT=(buff[14] & 0x20)>>5;
   SD_Card_CSD.TMP_WRITE_PROTECT=(buff[14] & 0x10)>>4;
   SD_Card_CSD.FILE_FORMAT = (buff[14] & 0xc)>>2;
   /*7:1*/SD_Card_CSD.CRC = (buff[15] & 0xFE)>>1;
}


int main(){

  UnpackCSDv1(buffer);

printf("version\t\t\t %d\n",SD_Card_CSD.version);
printf("TAAC\t\t\t %d\n",SD_Card_CSD.TAAC);
printf("NSAC\t\t\t %d\n",SD_Card_CSD.NSAC);
printf("TRAN_SPEED\t\t %d\n",SD_Card_CSD.TRAN_SPEED);
printf("CCC\t\t\t %d\n",SD_Card_CSD.CCC);
printf("READ_BL_LEN\t\t %d\n",SD_Card_CSD.READ_BL_LEN);
printf("READ_BL_PARTIAL\t\t %d\n",SD_Card_CSD.READ_BL_PARTIAL);
printf("WRITE_BLK_MISALIGN\t %d\n",SD_Card_CSD.WRITE_BLK_MISALIGN);
printf("READ_BLK_MISALIGN\t %d\n",SD_Card_CSD.READ_BLK_MISALIGN);
printf("DSR_IMP\t\t\t %d\n",SD_Card_CSD.DSR_IMP);
printf("C_SIZE\t\t\t %d\n",SD_Card_CSD.C_SIZE);
printf("VDD_R_CURR_MIN\t\t %d\n",SD_Card_CSD.VDD_R_CURR_MIN);
printf("VDD_R_CURR_MAX\t\t %d\n",SD_Card_CSD.VDD_R_CURR_MAX);
printf("VDD_W_CURR_MIN\t\t %d\n",SD_Card_CSD.VDD_W_CURR_MIN);
printf("VDD_W_CURR_MAX\t\t %d\n",SD_Card_CSD.VDD_W_CURR_MAX);
printf("C_SIZE_MULT\t\t %d\n",SD_Card_CSD.C_SIZE_MULT);
printf("ERASE_BLK_EN\t\t %d\n",SD_Card_CSD.ERASE_BLK_EN);
printf("SECTOR_SIZE\t\t %d\n",SD_Card_CSD.SECTOR_SIZE);
printf("WP_GRP_SIZE\t\t %d\n",SD_Card_CSD.WP_GRP_SIZE);
printf("WP_GRP_ENABLE\t\t %d\n",SD_Card_CSD.WP_GRP_ENABLE);
 printf("R2W_FACTOR\t\t %d\n",SD_Card_CSD.R2W_FACTOR);
printf("WRITE_BL_LEN\t\t %d\n",SD_Card_CSD.WRITE_BL_LEN);
printf("WRITE_BL_PARTIAL\t %d\n",SD_Card_CSD.WRITE_BL_PARTIAL);
printf("FILE_FORMAT_GRP\t\t %d\n",SD_Card_CSD.FILE_FORMAT_GRP);
printf("COPY\t\t\t %d\n",SD_Card_CSD.COPY);
printf("PERM_WRITE_PROTECT\t %d\n",SD_Card_CSD.PERM_WRITE_PROTECT);
printf("TMP_WRITE_PROTECT\t %d\n",SD_Card_CSD.TMP_WRITE_PROTECT);
printf("FILE_FORMAT\t\t %d\n",SD_Card_CSD.FILE_FORMAT);
printf("CRC\t\t\t %d\n",SD_Card_CSD.CRC);
}
