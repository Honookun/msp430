#include "uart.h"

char * sepline =  "**************************\n\r";
char * nret;
char clrscrn[5] = {0x1b,0x5b,0x32,0x4a,0x0};
char hexref[16] = "0123456789ABCDEF";

void uart_putc(unsigned char c)
{
        while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
        UCA0TXBUF = c;                                  // TX
}
void uart_cls(){
   uart_putssep(clrscrn,0);
}
void uart_sepline(){
  uart_putssep(sepline,0);
}

void uart_CarRet(){
  uart_putssep(nret,0);
}
void uart_puts(char *str)
{
  uart_putssep(str,0);  
}

void uart_putsNsep(char *str,char sep,int k)
{
  int j=0;
  while(j < k)
    if( *(str++) == sep) j++;  
     while(*str != sep) uart_putc(*str++);
}

void uart_putssep(char *str,char sep)
{
     while(*str != sep) uart_putc(*str++);
}

void uart_putnbrhex(unsigned long c){
  int i;
  char chararray[9];
  for(i=0;i<9;i++)
    chararray[i]=0;
  if(! c){
    chararray[0]='0';
    i=0;
  }
  while(c){
    i--;  
    chararray[i] = hexref[c&0x0f] ;
    c >>= 4;  
  }
  uart_puts(chararray+i);
}
 
void uart_putnbr(unsigned long c){
  int i;
  char chararray[10];
  for(i=0;i<10;i++)
    chararray[i]=0;
  if(! c){
    chararray[0]='0';
    i=0;
  }
  while(c){
    i--;  
    chararray[i] = (c%10) + '0';
    c /= 10;  
  }
  uart_puts(chararray+i);
}

void Stop_uart(){
  IE2 &= ~UCA0RXIE;  
  UCA0CTL1 |= UCSWRST;
}
void Setup_uart(unsigned int div,unsigned char mod,char* welcomeline){
  unsigned char divs[2];
  divs[0] = div & 0xff;
  divs[1] = div >> 8;
  nret = sepline + 26;
  P1SEL = BIT1 | BIT2 ;
  P1SEL2 = BIT1 | BIT2 ;
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK = 250kz
  UCA0BR0 = divs[0];                            // xMHz 
  UCA0BR1 = divs[1];                            // xMHz 
  UCA0MCTL =mod;                  // Modulation UCBRS
  UCA0CTL1 &= ~UCSWRST;                     // Initialize USCI state machine
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
  uart_cls();
  if(welcomeline){
    uart_sepline();
    uart_puts(welcomeline);uart_CarRet();
    uart_sepline();
  }
}
