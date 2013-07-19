#include <msp430.h>
#include "uart/uart.h"
#include "g2553_MMCv2/SPI.h"
#include "g2553_MMCv2/mmcv2.h"

unsigned char nmicnt;
void __attribute__((interrupt (NMI_VECTOR))) nmi_isr (void){
/* 
HERE = PANIC !
Shields up Mr Worf !
*/
  if(IFG1 & OFIFG){
    do {
      IFG1 &= ~OFIFG;
      for(nmicnt= 0xff;nmicnt>0;nmicnt--);
      P1OUT ^= 0x01;
      
    } while (IFG1 & OFIFG);
    IE1 |= OFIE;
    P1OUT &= BIT0;
  }
}

volatile char timeoutvar = 0;
void __attribute__((interrupt (TIMER0_A0_VECTOR))) TimerA0_a0_isr (void){
  timeoutvar++;
}


void __attribute__((interrupt (USCIAB0RX_VECTOR))) USCIAB0RX_isr (void){

  if(IFG2 & UCB0RXIFG){

    if(SPI_RXCallback(UCB0RXBUF)){
      IE2 &=  ~(UCB0RXIE|UCB0TXIE);
      __bic_SR_register_on_exit(LPM1_bits);
      }
  }
  if(IFG2 & UCA0RXIFG){
    while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
     UCA0TXBUF = UCA0RXBUF; // echo and set command global
  }
}

void __attribute__((interrupt (USCIAB0TX_VECTOR))) USCIAB0TX_isr (void){
  if(IFG2 & UCB0TXIFG){
    if(SPI_TXCallback()){
      IE2 &=  ~(UCB0RXIE|UCB0TXIE);
    __bic_SR_register_on_exit(LPM1_bits);
    }
}
}

void oscillatorFault(){ /*blinks in case of osc fault*/
  BCSCTL3 = LFXT1S_2; // DEFAULT : XTS = 0 -> uses VLO for LFXT1
  IFG1 &= ~OFIFG; // clear interrupt flg
  BCSCTL2 |= SELM_3|SELS; // MCLK = SMCLK = LFXT1 (see -2lines = VLO)
  
  __bis_status_register(SCG1+SCG0); 
  // stop MCLCK & SMCLK (settingthem vlo => cut power consumption)
  
  while(1){
    P1OUT ^= BIT0;
    __delay_cycles(500); // blink Red for problem
  }
}

void initClock8Mhz_SMCLK_D2(){
  int rep;
  P1OUT |= 0x01;

  DCOCTL  = CALDCO_8MHZ;
  BCSCTL1 = CALBC1_8MHZ;
  /* DEVICE DEPENDENT !!! 
     DCO running @16MHz => internal calibration
     BOTH LINE COUNTS : RSEL are in BCSCTL1 while subfreq in range for
     DCO is in DCOCTL
  */

  //  BCSCTL2 =   DIVS0; // 2 -> smlk 4mhz : later doesn't helps oscilo
  BCSCTL2 =   DIVS0;//2 -> smlk 4mhz

  BCSCTL1 |= XT2OFF;  /* ACLK = LFXTCLK =  32768hz xtal, /1 prescaler
			XT2 input is OFF
		     */
  BCSCTL3 = XCAP_3; /* internal XTAL caps (TI provided : 12.5pf)*/

  rep = 0;
  IFG1 &= ~OFIFG;
  while (IFG1 & OFIFG){ /* loop 10 times if after xtal not stab -> problem*/
    IFG1 &= ~OFIFG;
    __delay_cycles(10000);
    if(!(rep<10))
      oscillatorFault();
    rep++;
  }
  P1OUT = 0x00;
}


void printSDInfo(SD_Card_struct_t* CardInfo_p);
int main(){
  int blink=0;
  int blink2=0;
  unsigned int ret;
  
  

  // SD_Card_struct_t* Card_p;
  
  WDTCTL = WDTPW|WDTHOLD;
  initClock8Mhz_SMCLK_D2();
  IE1 |= OFIE;  // enable clock problem interrupt
  //Setup_uart(416,UCBRS2|UCBRS1,"* SDv3");
  Setup_uart(416,6,"* SDv3");

  /*fam guide p 435*/
  P1DIR|=BIT0;
  P2DIR|=BIT0|BIT2;
  P2OUT|=BIT0;

  // P2OUT&=~BIT2;

  // P1OUT|=BIT0;
  ret = mmcLibInit(29,0);

  if(ret == MMC_SUCCESS){

    printSDInfo(getSDCardInfo(10,0));

    blink2=1;


    //test writing partition table
    /* unsigned char pt[16] ={0x80,    0x02,    0x05,    0x07, */
    /* 			   0x0c,    0xff,    0xff,    0xff, */
    /* 			   0x00,    0x08,    0x00,    0x00, */
    /* 			   0x00,    0xe8,    0x3a,    0x00}; */
			   
    /* char ret = mmcWriteBlock(pt,0x1be,16); */
    /* uart_puts("W : 0x");uart_putnbrhex(ret);uart_CarRet(); */

   
    uart_CarRet();

#ifdef HARNESS

    // WARNING THIS WILL CRUSH
    // IN NON MINIMYZE COMPILE MODE ! 
    unsigned char buffer[128];
    unsigned int i;
    unsigned char schr[2]={0,1};
    char* jegeva = "JegeVa";
    unsigned long tmp =0;

    for(i=0;i<128;i++)
      buffer[i]=i; 
    uart_puts("Start : 0x");uart_putnbrhex(mmcContinuousWriteStart(0));uart_CarRet();

    ret = 0;
    tmp=0;

    uart_puts("CWrite: ");
    uart_puts("511x 'A' ");
    for(i=0;i<511;i++)
      tmp+=mmcContinuousWriteBuff(buffer+0x41,1);
    uart_putnbr(tmp);uart_putc(' ');uart_putnbr(i);
    ret+=tmp;

    uart_CarRet();

    i=3;tmp=0;
    uart_puts("CWrite: ");
    uart_putnbr(i);
    uart_puts("x128c ");
    for(;i>0;i--)     
      tmp+=mmcContinuousWriteBuff(buffer,128);
    uart_putnbr(tmp);;uart_putc(' ');uart_putnbr(3); ////////// 
 
    uart_CarRet();
    ret+=tmp;

    tmp=0;
    uart_puts("CWrite: 60x2c :");
    for(i=0;i<60;i++)
      tmp+=mmcContinuousWriteBuff(schr,2);
    
    ret+=tmp;
    uart_putnbr(tmp);;uart_putc(' ');uart_putnbr(i);

    uart_CarRet();

    tmp=0;
    uart_puts("CWrite: 2x'0' :");
    for(i=0;i<2;i++)
      tmp+=mmcContinuousWriteBuff(schr,1);
    uart_putnbr(tmp);;uart_putc(' ');uart_putnbr(i);
    ret+=tmp;
 
    uart_CarRet();

    tmp=0;
    uart_puts("CWrite: 1x'1' :");
    tmp+=mmcContinuousWriteBuff(schr+1,1);
    uart_putnbr(tmp);;uart_putc(' ');uart_putnbr(1);/////////////;
    ret+=tmp;

    uart_CarRet();

    tmp=0;
    uart_puts("CWrite: \""); uart_puts(jegeva);uart_puts("\": ");
    for(i=0;i<4;i++)
      tmp+=mmcContinuousWriteBuff((unsigned char*)jegeva,6);
    uart_putnbr(tmp);;uart_putc(' ');uart_putnbr(i);
    ret+=tmp;
     uart_CarRet();

    tmp=0;
    
    i=4;
    uart_puts("CWrite: ");
    uart_putnbr(i);
    uart_puts("x128c :");
    for(i=0;i<4;i++)   
      tmp+=mmcContinuousWriteBuff(buffer,128);
    uart_putnbr(tmp);;uart_putc(' ');uart_putnbr(i);//////////
 
   ;uart_CarRet();
    ret+=tmp;

    tmp=mmcContinuousWriteStop();
    uart_puts("Stop  : 0x");
    uart_putnbrhex(tmp);
    uart_CarRet();
    i=0;mmcContinuousWriteStart(0);
    tmp=0;
    TACTL = TASSEL_2|ID_3;//smclk /8
    TACCR0=0xf424; // .5s
    TA0CCTL0 |= CCIE;
    uart_puts("Strt Bench\n\r n*128 chars : R: ");
    TACTL |=MC_1; //TA0 start
    while(timeoutvar<8){
      mmcContinuousWriteBuff(buffer,128);
      tmp+=128;
    }; // 5Sec
    TACTL |=MC_0; //TA0 stop
    mmcContinuousWriteStop();
    uart_putnbr(tmp>>10); uart_puts(" kiB/s\n\r n* 1 char : R: ");
    tmp=0;
    TACTL = TASSEL_2|ID_3;//smclk /8
    TAR=0; TACCR0=0xf424; // .125s
    timeoutvar=0;
    mmcContinuousWriteStart(0);
    TACTL |=MC_1; //TA0 start
    while(timeoutvar<8){
      mmcContinuousWriteBuff(buffer,1);
      tmp++;
    }; 
    TACTL |=MC_0; //TA0 stop
    mmcContinuousWriteStop();
    uart_putnbr(tmp>>10); uart_puts(" kiB/s");

    uart_CarRet();
#endif
    }
    else{
      uart_puts("Lib Init Error : ");
      uart_putnbr(ret);uart_CarRet();
    }
  uart_sepline();

  CS_HIGH();
  while(1)
    {
    __delay_cycles(1000000);
    if(blink)
      P1OUT ^= BIT0;
    if(blink2)
      P2OUT ^= BIT2;
  }

}


void dispinfo(char* title,char* str,long n,char unit){
  uart_puts(title);
  uart_puts(": ");
  if(n) uart_putnbr(n);
  if(str) uart_puts(str);
  if(unit){uart_putc(' ');uart_putc(unit);}
  uart_CarRet();
}

void printSDInfo(SD_Card_struct_t* Card){

  
#ifndef MINIMYZE
  unsigned int tmp;
  unsigned char c=0;
  SD_Card_CSD_Struct_t* CSD;
  SD_Card_CID_Struct_t* CID;
  SD_Card_OCR_Struct_t* OCR;
  Partition_table_struct_t* PT;
  CSD=Card->csd_p;
  CID=Card->cid_p;
  OCR=Card->ocr_p;
  PT=Card->Partition_table;

  
  
  dispinfo("Name     ",(char*)CID->Prodname,0,0);

  //  dispinfo("Size     ",0,Card->size,'B');
  uart_puts("Size     : ");  uart_putnbr(Card->size);uart_puts(" MB"); uart_CarRet();
   //;,0,Card->size,'B');
  dispinfo("S/N      ",0,CID->SerialNbr,0);
  dispinfo("SDv      ",0,Card->version,0);
  dispinfo("CSDv     ",0,CSD->version ? 2:1,0);
 uart_puts("Cmd Clss : ");
 
  tmp = CSD->CCC;
  while(tmp){
    if(tmp&1){
      uart_putnbr(c);
      if(tmp>>1) uart_putc(',');
    }
    tmp>>=1;
    c++;
  }; uart_CarRet();
  tmp = 1<<CSD->READ_BL_LEN;
  dispinfo("Block    ",0,tmp,'B');
  dispinfo("Sector   ",0,tmp*((CSD->SECTOR_SIZE%10 | ((CSD->SECTOR_SIZE/10)<<4))+1),'B');  uart_puts("Partial  : ");

  if(Card->partial & SUPPORT_PARTIAL_R)  uart_putc('R');
  if(Card->partial & SUPPORT_PARTIAL_W)  uart_putc('W');
  uart_CarRet();
  uart_puts("UHS-2    : ");
  uart_putc('0'+OCR->UHS2);uart_CarRet();
  uart_puts("CCS      : ");
  uart_putc('0'+OCR->CCS);uart_CarRet();

  char* multinfo="1a1.2a1.3a1.5a2a2.5a3a3.5a4a4.5a5a5.5a6a7a8a";
  int tran_base[5]={0,100,1,10,100};
  char transtv[16]={1,10,12,13,15,20,25,30,35,40,45,50,55,60,70,80};
  int i;
uart_CarRet();  // uart_putnbr(sizeof(SD_Card_CSD_Struct_t)); uart_CarRet();
uart_puts("CSDv:"); uart_putnbr(CSD->version ? 2 : 1 );uart_CarRet();
uart_puts("TAAC :");
 switch(CSD->TAAC & 0x3){
 case 0:
 case 3:
 case 6:
     uart_putc('1');
   break;
 case 1:
 case 4:
 case 7:
     uart_puts("10");
   break;
 case 2:
 case 5:
     uart_puts("100");
   break;
 }
 if((CSD->TAAC & 0x3)<3){
   uart_puts("ns");
 }else if((CSD->TAAC & 0x3)<6){
    uart_puts("us");
 }else{
    uart_puts("ms");
 }
uart_puts(" x ");
// j=i=0;
 /* k=((CSD->TAAC & 0x38)>>3)-1;
    while(j < k){
    if( *(multinfo+i++) == 'a'){
    j++;
    }
    } */
 uart_putsNsep(multinfo,'a',((CSD->TAAC & 0x38)>>3)-1);
 uart_CarRet();

uart_puts("NSAC :");
uart_putnbr(CSD->NSAC);uart_CarRet();
uart_puts("TRAN_SPEED :");
 i =  tran_base[CSD->TRAN_SPEED & 0x7 ] *  transtv[(CSD->TRAN_SPEED & 0x38) >>3];
 uart_putnbr(i/10);uart_putc('.');uart_putnbr(i%10);CSD->TRAN_SPEED & 0x7 ? uart_puts("Mb/s") : uart_puts("kb/s") ;  uart_CarRet();
//uart_putnbr(CSD->TRAN_SPEED);uart_CarRet();
uart_puts("READ_BL_LEN :");
uart_putnbr(CSD->READ_BL_LEN);uart_CarRet();
uart_puts("READ_BL_PARTIAL :");
uart_putnbr(CSD->READ_BL_PARTIAL);uart_CarRet();
uart_puts("WRITE_BLK_MISALIGN :");
uart_putnbr(CSD->WRITE_BLK_MISALIGN);uart_CarRet();
uart_puts("READ_BLK_MISALIGN :");
uart_putnbr(CSD->READ_BLK_MISALIGN);uart_CarRet();
uart_puts("DSR_IMP :");
uart_putnbr(CSD->DSR_IMP);uart_CarRet();
uart_puts("C_SIZE :");
uart_putnbr(CSD->C_SIZE);uart_CarRet();
uart_puts("VDD_R_CURR_MIN :");
uart_putnbr(CSD->VDD_R_CURR_MIN);uart_CarRet();
uart_puts("VDD_R_CURR_MAX :");
uart_putnbr(CSD->VDD_R_CURR_MAX);uart_CarRet();
uart_puts("VDD_W_CURR_MIN :");
uart_putnbr(CSD->VDD_W_CURR_MIN);uart_CarRet();
uart_puts("VDD_W_CURR_MAX :");
uart_putnbr(CSD->VDD_W_CURR_MAX);uart_CarRet();
uart_puts("C_SIZE_MULT :");
uart_putnbr(CSD->C_SIZE_MULT);uart_CarRet();
uart_puts("ERASE_BLK_EN :");
uart_putnbr(CSD->ERASE_BLK_EN);uart_CarRet();
uart_puts("SECTOR_SIZE :");
uart_putnbr(CSD->SECTOR_SIZE);uart_CarRet();
uart_puts("WP_GRP_SIZE :");
uart_putnbr(CSD->WP_GRP_SIZE);uart_CarRet();
uart_puts("WP_GRP_ENABLE :");
uart_putnbr(CSD->WP_GRP_ENABLE);uart_CarRet();
uart_puts("R2W_FACTOR :");
uart_putnbr(CSD->R2W_FACTOR);uart_CarRet();
uart_puts("WRITE_BL_LEN :");
uart_putnbr(CSD->WRITE_BL_LEN);uart_CarRet();
uart_puts("WRITE_BL_PARTIAL :");
uart_putnbr(CSD->WRITE_BL_PARTIAL);uart_CarRet();
uart_puts("FILE_FORMAT_GRP :");
uart_putnbr(CSD->FILE_FORMAT_GRP);uart_CarRet();
uart_puts("COPY :");
uart_putnbr(CSD->COPY);uart_CarRet();
uart_puts("PERM_WRITE_PROTECT :");
uart_putnbr(CSD->PERM_WRITE_PROTECT);uart_CarRet();
uart_puts("TMP_WRITE_PROTECT :");
uart_putnbr(CSD->TMP_WRITE_PROTECT);uart_CarRet();
uart_puts("FILE_FORMAT :");
uart_putnbr(CSD->FILE_FORMAT);uart_CarRet();






  dispinfo("Parttions",0,0,'0'+PT->ActivePartitionNumber);
  for(tmp=0;tmp<4;tmp++){
    if(PT->PartitionEntries[tmp].chs_start[0]|PT->PartitionEntries[tmp].chs_start[1]|PT->PartitionEntries[tmp].chs_start[2]){
      dispinfo(" Parttion#",0,0,'0'+tmp);
      dispinfo("  Bootable ",0,0,PT->PartitionEntries[tmp].bootable ? 'y':'n');
     uart_puts("  CHSStart : ");
      uart_putnbrhex(PT->PartitionEntries[tmp].chs_start[0]); uart_putc(',');
      uart_putnbrhex(PT->PartitionEntries[tmp].chs_start[1]); uart_putc(',');
      uart_putnbrhex(PT->PartitionEntries[tmp].chs_start[2]); uart_CarRet();
    }
  }
#else
  dispinfo("Name",(char*)Card->Prodname,0,0);
  uart_puts("Size:");  uart_putnbr(Card->size);uart_puts(" MB"); uart_CarRet();
  dispinfo("SDv",0,Card->version,0);
  uart_puts("Partial:");
  if(Card->partial & SUPPORT_PARTIAL_R)  uart_putc('R');
  if(Card->partial & SUPPORT_PARTIAL_W)  uart_putc('W');uart_CarRet();
  dispinfo("Part0 Off.",0,Card->SC_Card1stpart_offset,0);
#endif
}


/* void printCSDInfo(SD_Card_CSD_Struct_t* CardInfo_p){ */
/*   char* multinfo="1a1.2a1.3a1.5a2a2.5a3a3.5a4a4.5a5a5.5a6a7a8a"; */
/*   int tran_base[5]={0,100,1,10,100}; */
/*   char transtv[16]={1,10,12,13,15,20,25,30,35,40,45,50,55,60,70,80}; */
/*   int i; */
/* uart_CarRet();  // uart_putnbr(sizeof(SD_Card_CSD_Struct_t)); uart_CarRet(); */
/* uart_puts("Version:"); uart_putnbr(CardInfo_p->version ? 2 : 1 );uart_CarRet(); */
/* uart_puts("TAAC :"); */
/*  switch(CardInfo_p->TAAC & 0x3){ */
/*  case 0: */
/*  case 3: */
/*  case 6: */
/*      uart_putc('1'); */
/*    break; */
/*  case 1: */
/*  case 4: */
/*  case 7: */
/*      uart_puts("10"); */
/*    break; */
/*  case 2: */
/*  case 5: */
/*      uart_puts("100"); */
/*    break; */
/*  } */
/*  if((CardInfo_p->TAAC & 0x3)<3){ */
/*    uart_puts("ns"); */
/*  }else if((CardInfo_p->TAAC & 0x3)<6){ */
/*     uart_puts("us"); */
/*  }else{ */
/*     uart_puts("ms"); */
/*  } */
/* uart_puts(" x "); */
/* // j=i=0; */
/*  /\* k=((CardInfo_p->TAAC & 0x38)>>3)-1; */
/*     while(j < k){ */
/*     if( *(multinfo+i++) == 'a'){ */
/*     j++; */
/*     } */
/*     } *\/ */
/*  uart_putsNsep(multinfo,'a',((CardInfo_p->TAAC & 0x38)>>3)-1); */
/*  uart_CarRet(); */

/* uart_puts("NSAC :"); */
/* uart_putnbr(CardInfo_p->NSAC);uart_CarRet(); */
/* uart_puts("TRAN_SPEED :"); */
/*  i =  tran_base[CardInfo_p->TRAN_SPEED & 0x7 ] *  transtv[(CardInfo_p->TRAN_SPEED & 0x38) >>3]; */
/*  uart_putnbr(i/10);uart_putc('.');uart_putnbr(i%10);CardInfo_p->TRAN_SPEED & 0x7 ? uart_puts("Mb/s") : uart_puts("kb/s") ;  uart_CarRet(); */
/* //uart_putnbr(CardInfo_p->TRAN_SPEED);uart_CarRet(); */
/* uart_puts("CCC :"); */
/* uart_putnbr(CardInfo_p->CCC);uart_CarRet(); */
/* uart_puts("READ_BL_LEN :"); */
/* uart_putnbr(CardInfo_p->READ_BL_LEN);uart_CarRet(); */
/* uart_puts("READ_BL_PARTIAL :"); */
/* uart_putnbr(CardInfo_p->READ_BL_PARTIAL);uart_CarRet(); */
/* uart_puts("WRITE_BLK_MISALIGN :"); */
/* uart_putnbr(CardInfo_p->WRITE_BLK_MISALIGN);uart_CarRet(); */
/* uart_puts("READ_BLK_MISALIGN :"); */
/* uart_putnbr(CardInfo_p->READ_BLK_MISALIGN);uart_CarRet(); */
/* uart_puts("DSR_IMP :"); */
/* uart_putnbr(CardInfo_p->DSR_IMP);uart_CarRet(); */
/* uart_puts("C_SIZE :"); */
/* uart_putnbr(CardInfo_p->C_SIZE);uart_CarRet(); */
/* uart_puts("VDD_R_CURR_MIN :"); */
/* uart_putnbr(CardInfo_p->VDD_R_CURR_MIN);uart_CarRet(); */
/* uart_puts("VDD_R_CURR_MAX :"); */
/* uart_putnbr(CardInfo_p->VDD_R_CURR_MAX);uart_CarRet(); */
/* uart_puts("VDD_W_CURR_MIN :"); */
/* uart_putnbr(CardInfo_p->VDD_W_CURR_MIN);uart_CarRet(); */
/* uart_puts("VDD_W_CURR_MAX :"); */
/* uart_putnbr(CardInfo_p->VDD_W_CURR_MAX);uart_CarRet(); */
/* uart_puts("C_SIZE_MULT :"); */
/* uart_putnbr(CardInfo_p->C_SIZE_MULT);uart_CarRet(); */
/* uart_puts("ERASE_BLK_EN :"); */
/* uart_putnbr(CardInfo_p->ERASE_BLK_EN);uart_CarRet(); */
/* uart_puts("SECTOR_SIZE :"); */
/* uart_putnbr(CardInfo_p->SECTOR_SIZE);uart_CarRet(); */
/* uart_puts("WP_GRP_SIZE :"); */
/* uart_putnbr(CardInfo_p->WP_GRP_SIZE);uart_CarRet(); */
/* uart_puts("WP_GRP_ENABLE :"); */
/* uart_putnbr(CardInfo_p->WP_GRP_ENABLE);uart_CarRet(); */
/* uart_puts("R2W_FACTOR :"); */
/* uart_putnbr(CardInfo_p->R2W_FACTOR);uart_CarRet(); */
/* uart_puts("WRITE_BL_LEN :"); */
/* uart_putnbr(CardInfo_p->WRITE_BL_LEN);uart_CarRet(); */
/* uart_puts("WRITE_BL_PARTIAL :"); */
/* uart_putnbr(CardInfo_p->WRITE_BL_PARTIAL);uart_CarRet(); */
/* uart_puts("FILE_FORMAT_GRP :"); */
/* uart_putnbr(CardInfo_p->FILE_FORMAT_GRP);uart_CarRet(); */
/* uart_puts("COPY :"); */
/* uart_putnbr(CardInfo_p->COPY);uart_CarRet(); */
/* uart_puts("PERM_WRITE_PROTECT :"); */
/* uart_putnbr(CardInfo_p->PERM_WRITE_PROTECT);uart_CarRet(); */
/* uart_puts("TMP_WRITE_PROTECT :"); */
/* uart_putnbr(CardInfo_p->TMP_WRITE_PROTECT);uart_CarRet(); */
/* uart_puts("FILE_FORMAT :"); */
/* uart_putnbr(CardInfo_p->FILE_FORMAT);uart_CarRet(); */

/* } */

