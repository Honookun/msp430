#include <legacymsp430.h>
#include <msp430.h>
#include <stdio.h>
#include "uart.h"

volatile unsigned int sig_array[100];
volatile char str[16];
volatile unsigned int i;

int uinttostr(char* dst,unsigned int u){
  int j =0;
  int k =0;
  if(u==0){
   *(dst)='0';
   return 1;
  }
  for(j=0;j<7;j++)
    *(dst+j)=0;
  k=u;
  j=0;
  while(k>0){
    j++;
    k/=10;
  }
  k=j;
  j--;
  while(u>0){
    *(dst+j)='0'+(u%10);
    u/=10;
    j--;
  }
  //  *(dst)='0'+u;
  return k;
}


void uart_rx_isr(unsigned char c) {
  unsigned int j;
 	uart_putc(c);
      	uart_puts("\n\r");
	switch(c){
	case '@':
	  uart_puts("\n\r");
	  uart_puts("Starting Dump :\n\r");
	  uart_puts("\n\r");
	  uinttostr(str,0);
	 
	  //sprintf(str,"%u\n\r",0);
	    uart_puts(str);
 uart_puts("\n\r");
	    j=1;
	  while(sig_array[j] != 0){
	    //sprintf(str,"%u",sig_array[j]);
	    uinttostr(str,sig_array[j]);
	    uart_puts("\n\r");
	    uart_puts(str);
	    j++;
	  }
	  uart_puts("\n\r#");
	  //sprintf(str,"#%d",j);
	  uinttostr(str,j);
	  uart_puts(str);
	   for(j=0;j<16;j++)
	     str[j]=0;
	   for(j=0;j<100;j++)
	     sig_array[j]=0;
	   j=i=0;
	   uart_puts("\n\r");
	   uart_puts("--END--");
	   uart_puts("\n\r");
	   break;
	 default :
	  break;
	}

	P1OUT ^= BIT0;		// toggle P1.0 (red led)
}
 

interrupt(NMI_VECTOR) nmi_isr(void){
  if(IFG1 & OFIFG){
    do {
      IFG1 &= ~OFIFG;
      for(i= 0xff;i>0;i--);
      P1OUT ^= 0x01;
      
    } while (IFG1 & OFIFG);
    IE1 |= OFIE;
    P1OUT &= 0x01;
  }
}

void oscillatorFault(){
  BCSCTL3 = LFXT1S_2;
  IFG1 &= ~OFIFG;
  BCSCTL2 |= SELM_3|SELS;
  __bis_status_register(SCG1+SCG0);
  while(1){
    P1OUT ^= 0x01;
    __delay_cycles(500);
  }
}

void initClock(){
  int rep;
  P1OUT |= 0x01;
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL  = CALDCO_1MHZ;
  BCSCTL2 = 0;
  BCSCTL3 = XCAP_3;
  rep = 0;
  IFG1 &= ~OFIFG;
  while (IFG1 & OFIFG){
    IFG1 &= ~OFIFG;
    __delay_cycles(10000);
    if(!(rep<10))
      oscillatorFault();
    rep++;
  }
  P1OUT = 0x00;
}

void disableWDT(){
  WDTCTL= WDTPW|WDTHOLD;
}


interrupt (PORT2_VECTOR) interrupt_port2(void) {
  if(P2IFG & BIT3){ //int was on pin 2.3
    P1OUT ^= BIT6; // blink red led
    do{
      P2IFG &= ~BIT3; // clear interrupt flag on pin 2.3 
    } while(P2IFG & BIT3);
    if(i == 0){
      TAR = 0;
      i++;
    }else{
      if(i<100 && TAR != 0){
      sig_array[i++]= TAR;
      }else{
	uart_putc('!');
      	uart_puts("\n\r");	
      }
    }

    P2IES ^= BIT3; // change front direction detection

  }
}



/**
 * Main routine
 */
int main(void)
{
  disableWDT();
  initClock();
  uart_init();
  for(i=0;i<50;i++)
    sig_array[i]=0;

  i=0;
  // register ISR called when data was received
  uart_set_rx_isr_ptr(uart_rx_isr);

  // configure time (just as a time reference)
  TACTL = MC1     | TASSEL1            | ID_2;
    //   Up mode   SMCLCK (38khz Xtal)  (/4)  

  P1DIR |= (BIT0|BIT6) ; // pin 1.0 & 1.6 output 
 
  P2DIR &= ~BIT3; // pin 2.3 input
  P2REN |= BIT3; // enable pull* res on pin 2.3 
  P2OUT |= BIT3; // it's a pullup     
  

  P2IFG &= ~BIT3; // clear interrupt flag on pin 2.3 
  P2IE |=  BIT3 ; // interrupt on pin 2.3 enabled

  IE1|=OFIE;

  //Enable all interrupts
  __bis_SR_register(GIE);
	

    
  while(1){
    ;
  }
  return 0;
}

