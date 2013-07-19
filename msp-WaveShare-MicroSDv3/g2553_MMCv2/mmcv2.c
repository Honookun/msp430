#include "mmcv2.h"
#include "SPI.h"

/*
  Private Function Declaration
*/

//int curr_crc16 = 0;
//#include "crc.h"
/* inline void updcrc16_str(unsigned char* data,int len){ */
/*   unsigned int crc_cnt=0; */
/*   while(crc_cnt<len) */
/*     curr_crc =  (curr_crc<<8) ^ crc16_table[(curr_crc >> 8) ^ data[crc_cnt++]]; */
/* } */

/* inline void updcrc16_char(unsigned char data){ */
/*   curr_crc =  (curr_crc<<8) ^ crc16_table[(curr_crc >> 8) ^ data]; */
/* } */



unsigned char command_response_buffer[16]; // max response = 5 bytes


SD_Card_struct_t SD_Card;
#ifndef MINIMYZE
SD_Card_CSD_Struct_t SD_Card_CSD;
SD_Card_CID_Struct_t SD_Card_CID;
SD_Card_OCR_Struct_t SD_Card_OCR;
Partition_table_struct_t SD_Card_Partition_table;
#endif
unsigned char Prodname[6];


long blocknbr;
int curr_block_sz=0;
int written_char;



char mmcCardInit(unsigned char Div0, unsigned char Div1);
char clockUntilQuery(unsigned char q);
void mmcReceiveDataBlock(unsigned char* buffer,unsigned int size);
char mmcReadPartitionTable();
SD_Card_struct_t* getSDCardInfo(){
  return(&SD_Card);
} 
char mmcLibInit(unsigned char Div0,unsigned char Div1)
{
  char i=0;
  char ret;
  MMC_PxOUT |= MMC_SIMO + MMC_UCLK;
  MMC_PxDIR |= MMC_SIMO + MMC_UCLK;
  // Chip Select
  
  MMC_CS_PxDIR |= MMC_CS;
  MMC_CS_PxOUT |= MMC_CS;
  // Card Detect Input
  MMC_CD_PxDIR &=  ~MMC_CD;
  MMC_PxSEL |= MMC_SIMO + MMC_SOMI + MMC_UCLK;
  MMC_PxSEL2 |= MMC_SIMO + MMC_SOMI + MMC_UCLK;

  if(mmcPing()){
    do{
      ret =  mmcCardInit(Div0,Div1) ;
    }while(i++<50 && ret != MMC_SUCCESS);
    if(i<50)
      return(MMC_SUCCESS);
    else
      return(ret);
  }else{
    return (MMC_INIT_ERROR);
  }
}

inline unsigned long logpow(unsigned long x,unsigned long p){
  // logarithmic exponentiation ftw !
  unsigned long r = 1;
  while(p){
    if(p&1) r *=x;
    x*=x;
    p >>=1;      
  }
  return(r);
}



void UnpackCIDv1(unsigned char* buff){
  unsigned char i;
#ifndef MINIMYZE
  SD_Card_CID.MID =  buff[0];
  SD_Card_CID.OID =  buff[1]<<8|buff[2];
#endif
  Prodname[5] = 0;
  for(i=0;i<5;i++)
    Prodname[i]=buff[3+i];
#ifndef MINIMYZE
  SD_Card_CID.Prodname = Prodname;
  SD_Card_CID.SerialNbr = ((long)buff[12]<<24)|((long)buff[11]<<16)|((long)buff[10]<<8)|buff[9];
  SD_Card_CID.MDT= (buff[13]&0xf)<<8|buff[14];
#endif
}


void UnpackCSD(unsigned char* buff){
#ifndef MINIMYZE
  SD_Card_CSD.version = (buff[0] & 0xc0 ) >> 6;
  SD_Card_CSD.TAAC = buff[1];
  SD_Card_CSD.NSAC = buff[2];
  SD_Card_CSD.TRAN_SPEED =  buff[3];
  SD_Card_CSD.CCC = buff[4]<<4|((buff[5]& 0xf0) >> 4);
  SD_Card_CSD.READ_BL_LEN = 
#else
    unsigned char READ_BL_LEN =
#endif 
(buff[5]& 0xf);

#ifndef MINIMYZE
  curr_block_sz= 1<<SD_Card_CSD.READ_BL_LEN;
  SD_Card_CSD.READ_BL_PARTIAL = buff[6]&0x80 >> 7;
  SD_Card_CSD.WRITE_BLK_MISALIGN = (buff[6]& 0x40)>>6;
  SD_Card_CSD.READ_BLK_MISALIGN = (buff[6]& 0x20)>>5;
  SD_Card_CSD.DSR_IMP = (buff[6]&010)>>4;
#endif
  if( (buff[0] & 0xc0 ) >> 6){
#ifndef MINIMYZE
    SD_Card.size=SD_Card_CSD.C_SIZE =
#else
      SD_Card.size=
#endif 
      (int) ((((long long)
       (
	(   (   (long)(buff[7] & 0x03f) ) <<16 )|
	(   (   (long)(buff[8]          ))<<8  )|
	(   (   (long) buff[9]          ))       
       )+1
	)<<19)/1000000);

  }else{

#ifndef MINIMYZE
    SD_Card_CSD.C_SIZE =
#else
    unsigned long C_SIZE=
#endif
      (buff[8]>>6)|(buff[7]<<2)|((buff[6]&0x3)<<10);


#ifndef MINIMYZE
    SD_Card_CSD.VDD_R_CURR_MIN = (buff[8] & 0x38 )>> 3;
    SD_Card_CSD.VDD_R_CURR_MAX = (buff[8] & 0x7 );
    SD_Card_CSD.VDD_W_CURR_MIN = (buff[9] & 0xe0 )>> 5;;
    SD_Card_CSD.VDD_W_CURR_MAX = (buff[9] & 0x1c )>> 2;
#endif

#ifndef MINIMYZE
   SD_Card_CSD.C_SIZE_MULT=
#else
    unsigned long C_SIZE_MULT=
#endif
     (buff[9] & 0x3 )<<1 | (buff[10]&(0x80)>>7);

#ifndef MINIMYZE
    SD_Card.size = (SD_Card_CSD.C_SIZE+1)*(logpow(2,SD_Card_CSD.C_SIZE_MULT+2)*logpow(2,SD_Card_CSD.READ_BL_LEN))>>20; //in MiB
#else
    SD_Card.size = (C_SIZE+1)*(logpow(2,C_SIZE_MULT+2)*logpow(2,READ_BL_LEN))>>20; //in MiB
#endif
  }
#ifndef MINIMYZE
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
   SD_Card.partial=0;
#endif
   if(buff[6]&0x80) SD_Card.partial |= SUPPORT_PARTIAL_R;
   if(buff[13]&0x20) SD_Card.partial |= SUPPORT_PARTIAL_W;
}

void UnpackOCR(unsigned char* buff){
#ifndef MINIMYZE
  SD_Card_OCR.v27 = buff[2] & 0x80 ? 1 : 0;
  SD_Card_OCR.v28 = buff[1] & 0x1 ? 1 : 0;
  SD_Card_OCR.v29 = buff[1] & 0x2 ? 1 : 0;
  SD_Card_OCR.v30 = buff[1] & 0x4 ? 1 : 0;
  SD_Card_OCR.v31 = buff[1] & 0x8 ? 1 : 0;
  SD_Card_OCR.v32 = buff[1] & 0x10 ? 1 : 0;
  SD_Card_OCR.v33 = buff[1] & 0x20 ? 1 : 0;
  SD_Card_OCR.v34 = buff[1] & 0x40 ? 1 : 0;
  SD_Card_OCR.v35 = buff[1] & 0x80 ? 1 : 0;
  SD_Card_OCR.v18 = buff[0] & 0x1 ? 1 : 0;
  SD_Card_OCR.UHS2 = buff[0] & 0x20 ? 1 : 0;
  SD_Card_OCR.CCS = buff[0] & 0x40 ? 1 : 0;
  SD_Card_OCR.powerup = buff[0] & 0x80 ? 1 : 0;
#endif
  SD_Card.HCorXC = buff[0] & 0x40 ? 1 : 0;
}


char mmcCardInit(unsigned char Div0,unsigned char Div1){
  unsigned char i=0;
  //  unsigned char RegBuffer[16];

  CS_HIGH();
  CS_HIGH();
  CS_HIGH();
  INITSPISetup(Div0,Div1);
  
  spiReadFrame(0,10);
  CS_LOW ();
  mmcSendCommand(MMC_GO_IDLE_STATE,0,0x95,MMC_GO_IDLE_STATE_RESPTYPE);
  if(command_response_buffer[0] & ~(MMC_R1_RESPONSE_IDLE))
    return(MMC_INIT_ERROR);


  mmcSendCommand(MMC_SEND_IF_COND,0x000001aa,0x87,MMC_SEND_IF_COND_RESPTYPE);
  // low voltage , 0xFA check value
   if(command_response_buffer[0] & ~(MMC_R1_RESPONSE_ERROR_ILLCMD | MMC_R1_RESPONSE_IDLE )){ //illegal command is managed
   
     return(MMC_INIT_ERROR);
   }
   
  if(command_response_buffer[0] & MMC_R1_RESPONSE_ERROR_ILLCMD){
    SD_Card.version = 1;
  }else{
    SD_Card.version = 2;
    if(command_response_buffer[3] !=1 || command_response_buffer[4] != 0xaa)
      // voltage & check value
       return(MMC_INIT_ERROR);
    
  }
  //mmcSendCommand(MMC_READ_OCR,0,0x5A,MMC_READ_OCR_RESPTYPE);
  //for(i=0;i<4;i++)
  //  *( ((unsigned char*)&SD_Card_OCR)+i)=*(command_response_buffer+4-i);
  //  not mandatory here
  //  Check thingies heres related to voltage changing & whatever
  // not used in msp430 : 3.3v /.
   
 


  while(command_response_buffer[0] & MMC_R1_RESPONSE_IDLE && i++<100 ) 
    mmcSendCommand(MMC_SEND_OP_COND_SDV1,0x40000000,0xdf,MMC_SEND_OP_COND_SDV1_RESPTYPE);

  if(i>=100)
    return MMC_TIMEOUT_ERROR;
    
  // waiting for sd to finish init process
  
 
  //mmcSendCommand(MMC_READ_CSD,0,0x5A,MMC_READ_CSD_RESPTYPE);
  //mmcReceiveDataBlock((unsigned char*)&SD_Card_CSD,16);
   
  
  i=0;
  while(i<4)command_response_buffer[i++]=0;

  mmcSendCommand(MMC_READ_CSD,0,0x5A,MMC_READ_CSD_RESPTYPE);
  if(command_response_buffer[0]){
    return(MMC_INIT_ERROR);
  }
  mmcReceiveDataBlock(command_response_buffer,16);
  //mmcReceiveDataBlock(RegBuffer,16);  
  UnpackCSD(command_response_buffer);
 
  

  mmcSendCommand(MMC_READ_CID,0,0x5A,MMC_READ_CID_RESPTYPE);
  if(command_response_buffer[0]){
    return(MMC_INIT_ERROR);
  }
  mmcReceiveDataBlock(command_response_buffer,16);  
  UnpackCIDv1(command_response_buffer);

 

  mmcSendCommand(MMC_SET_BLOCKLEN,512,0xff,MMC_SET_BLOCKLEN_RESPTYPE);
  if(command_response_buffer[0])
    return MMC_BLOCK_SET_ERROR;
  curr_block_sz = 512;

  mmcReadPartitionTable(command_response_buffer);
#ifndef MINIMYZE 
  SD_Card.csd_p=&SD_Card_CSD;
  SD_Card.cid_p=&SD_Card_CID;
  SD_Card.ocr_p=&SD_Card_OCR;
  SD_Card.Partition_table = &SD_Card_Partition_table;
#else
  SD_Card.Prodname = Prodname;
#endif
 

  do{
  mmcSendCommand(MMC_READ_OCR,0,0x5A,MMC_READ_OCR_RESPTYPE);
  }while(! (command_response_buffer[1]&0x80));

  if(command_response_buffer[0] & ~MMC_R1_RESPONSE_IDLE ){
    return(MMC_INIT_ERROR);
  }
  UnpackOCR(command_response_buffer+1);



  return(MMC_SUCCESS);
}

char mmcPing(void)
{
  if (!(MMC_CD_PxIN & MMC_CD))
    return (MMC_SUCCESS);
  else
    return (MMC_INIT_ERROR);
}



unsigned char PipelineControlResponse;
unsigned char PipelineControlQuery;

unsigned char clockUntilQueryPipelineControl(unsigned char c){
  if(c==PipelineControlQuery){ 
    //PipelineControlResponse=c;
    return 1;
  }
  return 0;
}

unsigned char clockUntilResponsePipelineControl(unsigned char c){
  if(c!=0xff){ // c != 0xff
    PipelineControlResponse=c;
    return 1;
  }
  return 0;
}
unsigned char clockUntilNotBusyPipelineControl(unsigned char c){
  if(c){ // c != 0x0
    PipelineControlResponse=c;
    return 1;
  }
  return 0;
}

char clockUntilQuery(unsigned char q){
  //PipelineControlResponse=0xff;
  PipelineControlQuery=q;
  setPipeliningControlRX(clockUntilQueryPipelineControl);
  spiReadFrame(0,1024);// clock for 1block busy
  if(PipelineControlResponse){
    //command_response_buffer[0]=PipelineControlResponse;
    return(MMC_SUCCESS);
  }else{
    return(MMC_TIMEOUT_ERROR);
  }
}

char clockUntilNotBusy(){
  PipelineControlResponse=0xff;
  setPipeliningControlRX(clockUntilNotBusyPipelineControl);
  spiReadFrame(0,513);// clock for 1block busy, will see later R1b
  if(PipelineControlResponse){
    command_response_buffer[0]=PipelineControlResponse;
    return(MMC_SUCCESS);
  }else{
    return(MMC_TIMEOUT_ERROR);
  }
}

char clockUntilResponse(){
  PipelineControlResponse=0xff;
  setPipeliningControlRX(clockUntilResponsePipelineControl);
  spiReadFrame(0,513); // blocksz+1
  if(~PipelineControlResponse){
    command_response_buffer[0]=PipelineControlResponse;
    return(MMC_SUCCESS);
  }else{
    return(MMC_TIMEOUT_ERROR);
  }
}
void mmcReceiveDataBlock(unsigned char* buffer,unsigned int size){
  clockUntilQuery(MMC_START_DATA_BLOCK_TOKEN);
  spiReadFrame(buffer,size);
  spiReadFrame(0,6);//ignore crc + clock timeout
}

char mmcSendCommand(unsigned char cmd,unsigned long data,unsigned char crc,char cmd_responsetype){
  
  unsigned char frame[6];
  int i=0;
  while(i<5)frame[i++]=0;
  /*i=0;
  while(i<4)command_response_buffer[i++]=0;
  */
  frame[0]=(cmd|0x40);
  if(data)
    //    *((unsigned long *)(frame+1))= ((data>>24)&0xff)|((data<<8)&0xff0000) |((data>>8)&0xff00) |((data<<24)&0xff000000);
    for(i=3;i>=0;i--) frame[4-i] = (data>>(8*i));

  frame[5]=crc;

  spiSendFrame(frame,6);
  if(clockUntilResponse() != MMC_SUCCESS)
    return(MMC_TIMEOUT_ERROR);
  switch(cmd_responsetype){
  case MMC_RESPONSE_TYPE_R2 :
    spiReadFrame(command_response_buffer+1,MMC_RESPONSE_TYPE_R2_LEN-1);
    break;
  case MMC_RESPONSE_TYPE_R3 :
    spiReadFrame(command_response_buffer+1,MMC_RESPONSE_TYPE_R3_LEN-1);
    break;
  case MMC_RESPONSE_TYPE_R7 :
    spiReadFrame(command_response_buffer+1,MMC_RESPONSE_TYPE_R7_LEN-1);
    break;
  case MMC_RESPONSE_TYPE_R1b:
    return(clockUntilNotBusy());
    break;
  }
  
  return(!command_response_buffer[0]);
}
#ifndef MINIMYZE 
char checkBlcSz512(){
  if(curr_block_sz != 512){
    mmcSendCommand(MMC_SET_BLOCKLEN,512,0xff,MMC_SET_BLOCKLEN_RESPTYPE);  
    if(command_response_buffer[0])
      return MMC_RESPONSE_ERROR;
    curr_block_sz=512;
  }
  return MMC_SUCCESS;
}
#endif

char mmcReadPartitionTable(){  
 
 

#ifndef MINIMYZE
   unsigned char i;
  unsigned char off;
  unsigned char BufferEntry[64];
  SD_Card_Partition_table.ActivePartitionNumber = 0;
  if(checkBlcSz512() != MMC_SUCCESS)
    return MMC_RESPONSE_ERROR;
#else
  unsigned char BufferEntry[12];
#endif
  /* mmcSendCommand(MMC_READ_SINGLE_BLOCK,0,0xff,MMC_READ_SINGLE_BLOCK_RESPTYPE);
  if(command_response_buffer[0])
    return MMC_RESPONSE_ERROR;
  clockUntilQuery(MMC_START_DATA_BLOCK_TOKEN);
  spiReadFrame(0,0x1be);// skip till entry*/

  
#ifndef MINIMYZE
  mmcReadBlock(BufferEntry,0x1be,64);
  for(i=0;i<4;i++){
    off=16*i;
    if(BufferEntry[off+1]|BufferEntry[off+2]|BufferEntry[off+3]){
      
      SD_Card_Partition_table.ActivePartitionNumber++;
      SD_Card_Partition_table.PartitionEntries[i].bootable = BufferEntry[off+0] == 0x80 ? 1:0;
      SD_Card_Partition_table.PartitionEntries[i].chs_start[0]=BufferEntry[off+3];
      SD_Card_Partition_table.PartitionEntries[i].chs_start[1]=BufferEntry[off+2];
      SD_Card_Partition_table.PartitionEntries[i].chs_start[2]=BufferEntry[off+1];
      SD_Card_Partition_table.PartitionEntries[i].type=BufferEntry[off+4];
      SD_Card_Partition_table.PartitionEntries[i].chs_stop[0]=BufferEntry[off+7];
      SD_Card_Partition_table.PartitionEntries[i].chs_stop[1]=BufferEntry[off+6];
      SD_Card_Partition_table.PartitionEntries[i].chs_stop[2]=BufferEntry[off+5];
      SD_Card_Partition_table.PartitionEntries[i].offset= 
	(long)BufferEntry[off+8]|(long)BufferEntry[off+9]<<8|(long)BufferEntry[off+10]<<16|(long)BufferEntry[off+11]<<24; /*to lil endian*/
      SD_Card_Partition_table.PartitionEntries[i].size= 
	(long)BufferEntry[off+12]|(long)BufferEntry[off+13]<<8|(long)BufferEntry[off+14]<<8|(long)BufferEntry[off+15]<<24; /*to lil endian*/
    }}
#else
      mmcReadBlock(BufferEntry,0x1be,12);
     SD_Card.SC_Card1stpart_offset = (long)BufferEntry[8]|(long)BufferEntry[9]<<8|(long)BufferEntry[10]<<16|(long)BufferEntry[11]<<24;
#endif  
  return MMC_SUCCESS;
};





char mmcContinuousWriteStart(unsigned long addr){
unsigned char DataBlockMark = MMC_START_DATA_BLOCK_WRITE;
  int preskip;
  blocknbr = addr >> 9;   // /512
  preskip = addr & 0x1ff; // %512
  mmcSendCommand(MMC_WRITE_SINGLE_BLOCK,SD_Card.HCorXC ? blocknbr : blocknbr*curr_block_sz,0xff,MMC_WRITE_SINGLE_BLOCK_RESPTYPE);
  if(command_response_buffer[0])
    return MMC_RESPONSE_ERROR;
  spiSendFrame(&DataBlockMark,1);
  spiReadFrameAltDummy(0,preskip);
  written_char=preskip;
  return MMC_SUCCESS;
}

char mmcContinuousWriteStop(){
  spiReadFrameAltDummy(0,curr_block_sz-written_char);
  spiReadFrame(0,1);//Dummy CRC
  if(clockUntilResponse() != MMC_SUCCESS)
    return(MMC_TIMEOUT_ERROR);
  command_response_buffer[0] &= 0xf; 
  if(command_response_buffer[0] == 0xb )
    return MMC_CRC_ERROR;
  if(command_response_buffer[0] == 0xd )
    return MMC_WRITE_ERROR;
  clockUntilNotBusy();
  return MMC_SUCCESS;
}

inline char mmcContinuousWriteBuff(unsigned char* pBuffer,int size){
  int left2write;
  char ret;
  if(written_char+size >= curr_block_sz){
    left2write = curr_block_sz-written_char;
    spiSendFrame(pBuffer,left2write);
    left2write=size-left2write;
    written_char=curr_block_sz;
    ret=mmcContinuousWriteStop();
    if(ret!=MMC_SUCCESS)
      return MMC_WRITE_ERROR;
    ret=mmcContinuousWriteStart( (blocknbr+1) * curr_block_sz );
    if(ret!=MMC_SUCCESS)
      return MMC_WRITE_ERROR;
    if(left2write){
      if(left2write==1)
	spiSendFrame(pBuffer+(size)-left2write,left2write); 
      else
	spiSendFrame(pBuffer+(size)-left2write,left2write+1); 
      // fix the fix : left2write+1 ??? why ??? weird spi shit here....
    }
    written_char=left2write;
  }else{   
    spiSendFrame(pBuffer,size);
    written_char+=size;
  }
  return MMC_SUCCESS;
}




char mmcWriteBlock(unsigned char* pBuffer,unsigned long addr,int size){
unsigned char DataBlockMark = MMC_START_DATA_BLOCK_WRITE;
  int preskip;

  //int i;

  
  if(size>512)
    return MMC_BLOCK_SET_ERROR;
#ifndef MINIMYZE 
  if(checkBlcSz512() != MMC_SUCCESS)
    return MMC_BLOCK_SET_ERROR;
#endif
  blocknbr = addr >> 9;   // /512
  preskip = addr & 0x1ff; // %512
  if(preskip+size>512)
    return MMC_BLOCK_SET_ERROR;
  mmcSendCommand(MMC_WRITE_SINGLE_BLOCK,SD_Card.HCorXC ? blocknbr : blocknbr*curr_block_sz,0xff,MMC_WRITE_SINGLE_BLOCK_RESPTYPE);
  if(command_response_buffer[0])
    return MMC_RESPONSE_ERROR;
  spiSendFrame(&DataBlockMark,1);
  //  updcrc_char(DataBlockMark);
  //  spiReadFrame(0,preskip); //will fill with 0xff
  spiReadFrameAltDummy(0,preskip); // fill with 0s
  //for(i=0;i<preskip;i++)
  //  updcrc_char(0xff); // 0xff
   spiSendFrame(pBuffer,size);
   // updcrc_str(pBuffer,size);
   //spiReadFrame(0,512-(preskip+size));
    spiReadFrameAltDummy(0,512-(preskip+size));
    //for(i=0;i<512-(preskip+size);i++)
    // updcrc_char(0xff);
    //spiSendFrame((unsigned char*)&curr_crc,2);
    spiReadFrameAltDummy(0xff,1);//Dummy CRC

    if(clockUntilResponse() != MMC_SUCCESS)
      return(MMC_TIMEOUT_ERROR);
    command_response_buffer[0] &= 0xf; 
    if(command_response_buffer[0] == 0xb )
      return MMC_CRC_ERROR;
    if(command_response_buffer[0] == 0xd )
      return MMC_WRITE_ERROR;
    clockUntilNotBusy();
    return MMC_SUCCESS;
}

char mmcReadBlock(unsigned char* pBuffer,unsigned long addr,int size){
  
 
  int preskip;
  long postskip=0;
  int crosssz;
  if(size>512)
    return MMC_BLOCK_SET_ERROR;
#ifndef MINIMYZE 
  if(checkBlcSz512() != MMC_SUCCESS)
    return MMC_BLOCK_SET_ERROR;
#endif  
  blocknbr = addr >> 9;   // /512
  preskip = addr & 0x1ff; // %512
  if(preskip+size>512){
    //
    // cross boundary read
    //
    crosssz = 512-preskip;
    mmcSendCommand(MMC_READ_SINGLE_BLOCK,SD_Card.HCorXC ? blocknbr : blocknbr*curr_block_sz,0xff,MMC_READ_SINGLE_BLOCK_RESPTYPE);
    //  ? new school, sector adress : old school cards, byte adress 
    if(command_response_buffer[0])
      return MMC_RESPONSE_ERROR;
    clockUntilQuery(MMC_START_DATA_BLOCK_TOKEN);
    spiReadFrame(0,preskip); // skip
    spiReadFrame(pBuffer,crosssz); // read up to boundary
    spiReadFrame(0,2);//skip crc
    blocknbr++;
    mmcSendCommand(MMC_READ_SINGLE_BLOCK,(SD_Card.HCorXC ? blocknbr : blocknbr*curr_block_sz),0xff,MMC_READ_SINGLE_BLOCK_RESPTYPE);
    //  ? new school, sector adress : old school cards, byte adress
   
    if(command_response_buffer[0])
      return MMC_DATA_TOKEN_ERROR;
    clockUntilQuery(MMC_START_DATA_BLOCK_TOKEN);
    spiReadFrame(pBuffer+crosssz,preskip+size-512);
    postskip=514-(preskip+size-512);// skip needed + crc
  }else{
    //
    //in boundary read
    //
    mmcSendCommand(MMC_READ_SINGLE_BLOCK,blocknbr,0xff,MMC_READ_SINGLE_BLOCK_RESPTYPE);
    if(command_response_buffer[0])
      return MMC_RESPONSE_ERROR;
    clockUntilQuery(MMC_START_DATA_BLOCK_TOKEN);
    spiReadFrame(0,preskip); // skip
    spiReadFrame(pBuffer,size);
    postskip=514-preskip+size; // skip needed + crc
    
  }
  spiReadFrame(0,postskip);
  return MMC_SUCCESS;
}








