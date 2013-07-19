#ifndef _MMCLIB_H
#define _MMCLIB_H


// macro defines
#define HIGH(a) ((a>>8)&0xFF)               // high byte from word
#define LOW(a)  (a&0xFF)                    // low byte from word

#ifndef DUMMY_CHAR
#define DUMMY_CHAR 0xFF
#endif

// Tokens (necessary  because at NPO/IDLE (and CS active) only 0xff is on the data/command line)
#define MMC_START_DATA_BLOCK_TOKEN          0xfe   // Data token start byte, Start Single Block Read
#define MMC_START_DATA_MULTIPLE_BLOCK_READ  0xfe   // Data token start byte, Start Multiple Block Read
#define MMC_START_DATA_BLOCK_WRITE          0xfe   // Data token start byte, Start Single Block Write
#define MMC_START_DATA_MULTIPLE_BLOCK_WRITE 0xfc   // Data token start byte, Start Multiple Block Write
#define MMC_STOP_DATA_MULTIPLE_BLOCK_WRITE  0xfd   // Data toke stop byte, Stop Multiple Block Write


// an affirmative R1 response (no errors)
#define MMC_R1_RESPONSE_OK                 0x00
#define MMC_R1_RESPONSE_IDLE               0x01
#define MMC_R1_RESPONSE_ERROR_ERRST        0x02 /*Erase rst*/
#define MMC_R1_RESPONSE_ERROR_ILLCMD       0x04 /*Illegal command*/
#define MMC_R1_RESPONSE_ERROR_CRC          0x08 /*CRC error*/
#define MMC_R1_RESPONSE_ERROR_ERZSEQ       0x10 /*Erase Seq Error*/
#define MMC_R1_RESPONSE_ERROR_ADDRSS       0x20 /*Address Error*/
#define MMC_R1_RESPONSE_ERROR_PARAM        0x40 /*Address Error*/
#define MMC_R1_RESPONSE_DUMMY              0xff

#define MMC_R2_RESPONSE_ERROR_CRDLCKD      0x01 /*Card locked*/
#define MMC_R2_RESPONSE_ERROR_WPESLUF      0x02 /*Write Protect Erase Skip, Lock/Unlock Failed*/
#define MMC_R2_RESPONSE_ERROR_UNKNOWN      0x04 /* Unspecified error*/
#define MMC_R2_RESPONSE_ERROR_CCONTRL      0x08 /*card controller error*/
#define MMC_R2_RESPONSE_ERROR_CECC         0x10 /*ECC*/
#define MMC_R2_RESPONSE_ERROR_WPVTION      0x20 /*Write protect violation*/
#define MMC_R2_RESPONSE_ERROR_ERZPRM       0x40 /*Erase param*/
#define MMC_R2_RESPONSE_ERROR_OVRWRT       0x80 /*out of range*/

// error/success codes
#define MMC_SUCCESS           0x01
#define MMC_BLOCK_SET_ERROR   0x02
#define MMC_RESPONSE_ERROR    0x03
#define MMC_DATA_TOKEN_ERROR  0x04
#define MMC_INIT_ERROR        0x05
#define MMC_CRC_ERROR         0x10
#define MMC_WRITE_ERROR       0x11
#define MMC_OTHER_ERROR       0x12
#define MMC_TIMEOUT_ERROR     0xFF


// commands: first bit 0 (start bit), second 1 (transmission bit); CMD-number + 0ffsett 0x40
// commands + response: sd simplified layer p85
// responses def : p96
// Responses : 
//R1 0
//R2 1
//R7 2
//R1b 3

#define MMC_RESPONSE_TYPE_R1 0
#define MMC_RESPONSE_TYPE_R2 1
#define MMC_RESPONSE_TYPE_R3 2
#define MMC_RESPONSE_TYPE_R7 3
#define MMC_RESPONSE_TYPE_R1b 4

#define MMC_RESPONSE_TYPE_R1_LEN 1
#define MMC_RESPONSE_TYPE_R2_LEN 2
#define MMC_RESPONSE_TYPE_R3_LEN 5
#define MMC_RESPONSE_TYPE_R7_LEN 5
#define MMC_RESPONSE_TYPE_R1b_LEN 1

 
#define MMC_GO_IDLE_STATE          0x0     //CMD0
#define MMC_GO_IDLE_STATE_RESPTYPE MMC_RESPONSE_TYPE_R1

#define MMC_SEND_OP_COND_SDV1      0x1     //CMD1
#define MMC_SEND_OP_COND_SDV1_RESPTYPE MMC_RESPONSE_TYPE_R1

#define MMC_SEND_OP_COND_SDV2      0x29     //ACMD41
#define MMC_SEND_OP_COND_SDV2_RESPTYPE MMC_RESPONSE_TYPE_R1

#define MMC_SEND_IF_COND           0x8     //CMD8
#define MMC_SEND_IF_COND_RESPTYPE  MMC_RESPONSE_TYPE_R7

#define MMC_READ_CSD               0x9     //CMD9
#define MMC_READ_CSD_RESPTYPE      MMC_RESPONSE_TYPE_R1

#define MMC_READ_CID               0xa     //CMD10
#define MMC_READ_CID_RESPTYPE      MMC_RESPONSE_TYPE_R1

#define MMC_STOP_TRANSMISSION      0xc     //CMD12
#define MMC_STOP_TRANSMISSION_RESPTYPE     MMC_RESPONSE_TYPE_R1b

#define MMC_SEND_STATUS            0xd     //CMD13
#define MMC_SEND_STATUS_RESPTYPE      MMC_RESPONSE_TYPE_R1

#define MMC_SET_BLOCKLEN           0x10     //CMD16 Set block length for next read/write
#define MMC_SET_BLOCKLEN_RESPTYPE      MMC_RESPONSE_TYPE_R1

#define MMC_READ_SINGLE_BLOCK      0x11     //CMD17 Read block from memory
#define MMC_READ_SINGLE_BLOCK_RESPTYPE      MMC_RESPONSE_TYPE_R1

#define MMC_READ_MULTIPLE_BLOCK    0x12     //CMD18
#define MMC_READ_MULTIPLE_BLOCK_RESPTYPE      MMC_RESPONSE_TYPE_R1

// SHOULD NOT BE USED IN SPI MODE PREFER STOP TRANSMISSION 
// WHEN YOU HAVE WHAT YOU WANT
#define MMC_READ_WRITE_SET_BLCK_CNT 0x17   //CMD 23 - CNT for cmd18 & 25
#define MMC_READ_WRITE_SET_BLCK_CNT_RESPTYPE      MMC_RESPONSE_TYPE_R1
// YOU HAVE BEEN WARNED

#define MMC_WRITE_SINGLE_BLOCK            0x18     //CMD24
#define MMC_WRITE_SINGLE_BLOCK_RESPTYPE      MMC_RESPONSE_TYPE_R1

#define MMC_WRITE_MULTIPLE_BLOCK   0x19     //CMD25
#define MMC_WRITE_MULTIPLE_BLOCK_RESPTYPE      MMC_RESPONSE_TYPE_R1

#define MMC_WRITE_CSD              0x1b     //CMD27 PROGRAM_CSD
#define MMC_WRITE_CSD_RESPTYPE      MMC_RESPONSE_TYPE_R1

#define MMC_SET_WRITE_PROT         0x1c     //CMD28
#define MMC_SET_WRITE_PROT_RESPTYPE     MMC_RESPONSE_TYPE_R1b

#define MMC_CLR_WRITE_PROT         0x1d     //CMD29
#define MMC_CLR_WRITE_PROT_RESPTYPE     MMC_RESPONSE_TYPE_R1b

#define MMC_SEND_WRITE_PROT        0x1e     //CMD30
#define MMC_SEND_WRITE_PROT_RESPTYPE      MMC_RESPONSE_TYPE_R1

#define MMC_TAG_SECTOR_START       0x20     //CMD32
#define MMC_TAG_SECTOR_START_RESPTYPE       MMC_RESPONSE_TYPE_R1

#define MMC_TAG_SECTOR_END         0x21     //CMD33
#define MMC_TAG_SECTOR_END_RESPTYPE        MMC_RESPONSE_TYPE_R1
/*
#define MMC_UNTAG_SECTOR           0x22     //CMD34
#define MMC_UNTAG_SECTOR

#define MMC_TAG_EREASE_GROUP_START 0x23     //CMD35
#define MMC_TAG_EREASE_GROUP_START

#define MMC_TAG_EREASE_GROUP_END   0x24     //CMD36
#define MMC_TAG_EREASE_GROUP_END

#define MMC_UNTAG_EREASE_GROUP     0x25     //CMD37
#define MMC_UNTAG_EREASE_GROUP
*/
#define MMC_ERASE                 0x26     //CMD38
#define MMC_ERASE_RESPTYPE        MMC_RESPONSE_TYPE_R1b


#define MMC_READ_OCR               0x3A     //CMD58
#define MMC_READ_OCR_RESPTYPE      MMC_RESPONSE_TYPE_R3

#define MMC_CRC_ONOFF               0x3b    //CMD59
#define MMC_READ_OCR_RESPTYPE      MMC_RESPONSE_TYPE_R3

#define SUPPORT_PARTIAL_R 0x1
#define SUPPORT_PARTIAL_W 0x2
typedef struct __attribute__ ((packed)) {
  
  unsigned char version           :2;
  unsigned char TAAC              :8;
  unsigned char NSAC              :8;
  unsigned char TRAN_SPEED        :8;
  unsigned int  CCC               :12;
  unsigned char READ_BL_LEN       :4;
  unsigned char READ_BL_PARTIAL   :1;
  unsigned char WRITE_BLK_MISALIGN:1;
  unsigned char READ_BLK_MISALIGN :1;
  unsigned char DSR_IMP           :1;
  unsigned int C_SIZE            :16;
  unsigned char VDD_R_CURR_MIN    :3;
  unsigned char VDD_R_CURR_MAX    :3;
  unsigned char VDD_W_CURR_MIN    :3;
  unsigned char VDD_W_CURR_MAX    :3;
  unsigned char C_SIZE_MULT       :3;
  unsigned char ERASE_BLK_EN      :1;
  unsigned char SECTOR_SIZE       :7;
  unsigned char WP_GRP_SIZE       :7;
  unsigned char WP_GRP_ENABLE     :1;
  unsigned char R2W_FACTOR        :3;
  unsigned char WRITE_BL_LEN      :4;
  unsigned char WRITE_BL_PARTIAL  :1;
  unsigned char FILE_FORMAT_GRP   :1;
  unsigned char COPY              :1;
  unsigned char PERM_WRITE_PROTECT:1;
  unsigned char TMP_WRITE_PROTECT :1;
  unsigned char FILE_FORMAT       :2;
} SD_Card_CSD_Struct_t;


typedef struct __attribute__ ((packed)) { //
  unsigned char MID :8;
  unsigned int OID :16;
  unsigned char* Prodname;
  unsigned char ProdRev :8;
  unsigned long SerialNbr :32;
  unsigned int MDT :12;

}SD_Card_CID_Struct_t;

   
#ifndef MINIMYZE
typedef struct __attribute__ ((packed)) { //p112

  unsigned int /*Reserved*/ :15;
  unsigned char v27 :1;
  unsigned char v28 :1;
  unsigned char v29 :1;
  unsigned char v30 :1;
  unsigned char v31 :1;
  unsigned char v32 :1;
  unsigned char v33 :1;
  unsigned char v34 :1;
  unsigned char v35 :1;
  unsigned char v18 :1;
  unsigned char UHS2 :1;
  unsigned char CCS :1;

  unsigned char powerup :1;

}SD_Card_OCR_Struct_t;
#endif


typedef struct __attribute__ ((packed)){
  unsigned char bootable;
  unsigned char chs_start[3];
  unsigned char type;
  unsigned char chs_stop[3];
  unsigned long offset;
  unsigned long size;
}  PartitionEntry_struct_t;

typedef struct __attribute__ ((packed)){
  PartitionEntry_struct_t PartitionEntries[4];
  unsigned char ActivePartitionNumber;
} Partition_table_struct_t;

typedef struct __attribute__ ((packed)){
  char version;
  char HCorXC;
  unsigned long size;
  char partial;
#ifndef MINIMYZE
  SD_Card_CSD_Struct_t* csd_p;
  SD_Card_CID_Struct_t* cid_p;
  SD_Card_OCR_Struct_t* ocr_p;
  Partition_table_struct_t* Partition_table;
#else
  unsigned char * Prodname;
  unsigned long SC_Card1stpart_offset ;
#endif
  //Partition_table_struct_t* Partition_table;
} SD_Card_struct_t;

// check if MMC card is present
char mmcPing(void);
char mmcSendCommand(unsigned char cmd,unsigned long data,unsigned char crc,char cmd_responsetype);

char mmcLibInit(unsigned char div0,unsigned char div1);
unsigned long logpow(unsigned long x,unsigned long p);
SD_Card_struct_t* getSDCardInfo();
char mmcReadBlock(unsigned char* pBuffer,unsigned long addr,int size);
char mmcWriteBlock(unsigned char* pBuffer,unsigned long addr,int size);
char mmcContinuousWriteStart(unsigned long addr);
char mmcContinuousWriteStop();
char mmcContinuousWriteBuff(unsigned char* pBuffer,int size);
#endif /* _MMCLIB_H */
