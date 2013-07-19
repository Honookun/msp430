#include <legacymsp430.h>
#include "uart.h"

#define SPIPORTOUT P1OUT
#define SPIPORTDIR P1DIR
#define SPIPORTIN P1IN

#define PIN_CLOCK BIT3
#define PIN_DATAIN BIT4
#define PIN_DATAOUT BIT5
#define PIN_CS BIT6
#define PIN_ORG BIT7

volatile unsigned char nmicnt;

interrupt(NMI_VECTOR) nmi_isr(void){ /* HERE = PANIC !*/
  if(IFG1 & OFIFG){
    do {
      IFG1 &= ~OFIFG;
      for(nmicnt= 0xff;nmicnt>0;nmicnt--);
      P1OUT ^= 0x01;
      
    } while (IFG1 & OFIFG);
    IE1 |= OFIE;
    P1OUT &= 0x01;
  }
}

void oscillatorFault(){ /*blinks in case of osc fault*/
  BCSCTL3 = LFXT1S_2; // DEFAULT : XTS = 0 -> uses VLO for LFXT1
  IFG1 &= ~OFIFG; // clear interrupt flg
  BCSCTL2 |= SELM_3|SELS; // MCLK = SMCLK = LFXT1 (see -2lines = VLO)
  
  __bis_status_register(SCG1+SCG0); 
  // stop MCLCK & SMCLK (settingthem vlo => cut power consumption)
  
  while(1){
    P1OUT ^= 0x01;
    __delay_cycles(500); // blink green for problem
  }
}

void initClock(){
  int rep;
  P1OUT |= 0x01;

  DCOCTL  = CALDCO_1MHZ;
  BCSCTL1 = CALBC1_1MHZ;
  /* DEVICE DEPENDENT !!! 
     DCO running @16MHz => internal calibration
     BOTH LINE COUNTS : RSEL are in BCSCTL1 while subfreq in range for
     DCO is in DCOCTL
  */

  //  BCSCTL1 |= XT2OFF;  
/* ACLK = LFXTCLK =  32768hz xtal, /1 prescaler
			XT2 input is OFF
		     */


  // BCSCTL2 = 0; // MCLCK = DCO / 1, SMCLK = DCO / 1 
  //BCSCTL3 = XCAP_3; /* internal XTAL caps (TI provided : 12.5pf)*/
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

void disableWDT(){
  WDTCTL= WDTPW|WDTHOLD; // disable watchdog
}

void clock(){
  //  P1OUT ^= BIT6;
SPIPORTOUT &= (~PIN_CLOCK);
SPIPORTOUT |= PIN_CLOCK;
}


void uart_rx_isr(unsigned char c) {
  //	uart_putc(c);
	P1OUT ^= BIT0;		// toggle P1.0 (red led)
}


unsigned int read16bits(unsigned int addr){
  unsigned int read=0;
  unsigned int i;
  SPIPORTOUT |=PIN_ORG; // ORG = 1 : 16bit
  SPIPORTOUT |=PIN_CS;
  SPIPORTOUT |= PIN_DATAOUT; // 1 start bit
   clock();
    // readopcode 0b10;
    SPIPORTOUT |= PIN_DATAOUT; // 1
    clock();
    SPIPORTOUT &= ~PIN_DATAOUT; // 0
    clock();
    for(i=0;i<8;i++){
      if(addr & (1<<i)){
	SPIPORTOUT |= PIN_DATAOUT;
      }else{
	SPIPORTOUT &= ~PIN_DATAOUT;
      }
       clock();
    }
    read=0;
    clock(); // a dummy 0 is outputed
    for(i=0;i<16;i++){
      clock(); // raising edge to input data
      if(SPIPORTIN & PIN_DATAIN)
	read |= (1<<i);
    }
    return read;
}

int main(){
  unsigned char i ;
  unsigned int addr;
  unsigned int read;
  unsigned int org =1;
  unsigned char str[5];
  char * hexalpha = "0123456789abcdef";

  str[4]=0;

  disableWDT(); // disable watchdog
  initClock(); // see func
  uart_init();
  IE1 |= OFIE;  // enable clock problem interrupt
  uart_set_rx_isr_ptr(uart_rx_isr);
  SPIPORTDIR |= PIN_DATAOUT|PIN_CS|PIN_CLOCK ;
   __bis_SR_register(GIE);
  uart_puts((char *)"\n\r***************\n\r");
  uart_puts((char *)" MSP430 Microwiredumper\n\r");
  uart_puts((char *)"***************\n\r\n\r");
  uart_puts((char *)"PRESS any key to start dumping...\n\r");
  unsigned char c = uart_getc();

  
  addr = 0;
  while(addr < 256){
    if(!(addr%16))
      uart_puts((char *)"\n\r");
    i=0;
    read=read16bits(addr);
    *str=*(hexalpha+(read>>12));
    *(str+1)=*(hexalpha+((read>>8)&15));
    *(str+2)=*(hexalpha+((read>>4)&15));
    *(str+3)=*(hexalpha+(read&15));
      uart_puts((char*)str);
      uart_puts((char *)" ");
      /*for(i=0;i<4;i++){
	*(str+i)=0;
    }
     *str=((read>>8) & 0xff);
    *(str+1)=(read & 0xff);
    
    uart_puts((char*)str);
    uart_puts((char *)"\"");
    uart_puts((char *)"\n\r");
    */
    

    /*
    SPIPORTOUT |=PIN_CS;
    SPIPORTOUT |= PIN_DATAOUT; // 1 start bit
    clock();
    // readopcode 0b10;
    SPIPORTOUT |= PIN_DATAOUT; // 1
    clock();
    SPIPORTOUT &= ~PIN_DATAOUT; // 0
    clock();
    for(i=0;i<8;i++){
      if(addr & (1<<i)){
	SPIPORTOUT |= PIN_DATAOUT;
      }else{
	SPIPORTOUT &= ~PIN_DATAOUT;
      }
       clock();
    }
    read=0;
    clock(); // a dummy 0 is outputed
    for(i=0;i<8;i++){
      clock(); // raising edge to input data
      if(SPIPORTIN & PIN_DATAIN)
	read |= (1<<i);
      *str=*(hexalpha+(read>>4));
      *(str+1)=*(hexalpha+(read&15));
    }
     uart_puts((char*)str);
   
    read=0;
    for(i=0;i<8;i++){
      clock(); // raising edge to input data
      if(SPIPORTIN & PIN_DATAIN)
	read |= (1<<i);
     
      *str=*(hexalpha+(read>>4);
      *(str+1)=*(hexalpha+(read&15));
    }
    uart_puts((char*)str);
   
    SPIPORTOUT &= ~PIN_CS;*/
     addr++;
  }


  return 1; 
}

