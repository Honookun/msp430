#include <msp430.h>

volatile unsigned char timeoutvar ;
volatile unsigned char readingchar =0;
volatile unsigned char readingbit  =0;
volatile unsigned int timedetect;

unsigned char read_value[5];

char * sepline =  "**************************\n\r";
char * nret ;
char clrscrn[5] = {0x1b,0x5b,0x32,0x4a,0x0};
#define BITPIN1DW BIT4

unsigned char timeouts_array[] = {80,70};

void __attribute__((interrupt (TIMER0_A0_VECTOR))) TimerA0_a0_isr (void){
  timeoutvar++;
  P1DIR |= BIT6;
  P1OUT ^= BIT6;
}

void __attribute__((interrupt (PORT1_VECTOR))) Port1_isr (void){
 
 
  if(P1IFG & BITPIN1DW){
    if(P1IES & BITPIN1DW){
      //  P1OUT &= ~BIT5;
       readingbit++;
      // INTERRUPT FRONT FALLING EDGE OF 1DWIRE
      TACTL = MC_0;
      // stop timer, if here : no timeout yet
      timedetect = TAR;
      // 
      P1IES &= ~BITPIN1DW; 
      //prepare for next edge
      
      //data
      read_value[readingchar] <<=1;
      if(timedetect>50){
	read_value[readingchar] |= 1;
	P1OUT |= BIT5;
	P1OUT &= ~BIT5;
	  }
     
      if(readingbit==8){
	readingbit=0;
	readingchar++;
	
      }
      
    }else{
      //P1OUT |= BIT5;
      //  INTERRUPT FRONT RAISING EDGE OF 1DWIRE
      if(readingchar<5){
	TACTL = MC_0;
	// TIMER A stop
	TACTL |= TACLR;
	TACTL = TASSEL_2|ID_1|MC_1; // 1mhz
	// smclk | /2 | up
	CCR0 = 77;
	//timeout if > 70 + 10%
	TA0CCTL0 |= CCIE;
	timeoutvar = 0;
	P1IES |= BITPIN1DW; //prepare for next edge
      }else{
	TACTL = MC_0;
      }
    }
  } 
  while(P1IFG & BITPIN1DW)
    P1IFG &= ~BITPIN1DW;
}

void __attribute__((interrupt (USCIAB0RX_VECTOR))) USCIAB0RX_ISR(void){
 
  if(IFG2 & UCA0RXIFG){
    while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
     UCA0TXBUF = UCA0RXBUF; // echo and set command global
  }
}

void Setup_uart();
void uart_putc(unsigned char c);
void uart_puts(char *str);
void uart_putcuc(unsigned char c);

int main(void)
{

  WDTCTL = WDTPW|WDTHOLD;
  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_8MHZ;                    // Set DCO
  DCOCTL = CALDCO_8MHZ;
  BCSCTL2 =  DIVS1 ;//DIVS0;  // smclk = dco/8 ie 2Mhz
  BCSCTL1 |= XT2OFF;
  BCSCTL3 = XCAP_3;

  P1SEL = BIT1 | BIT2 ;
  P1SEL2 = BIT1 | BIT2 ;
  Setup_uart();
  nret=sepline+26;              
  __eint();

  uart_puts(clrscrn);
  uart_puts(nret);
  uart_puts(sepline);
  uart_puts("*    MSP430 : DHT11      *");
  uart_puts(nret);
  uart_puts(sepline);

 
  // enable timer interrupt


 
  P1DIR |= BIT0;
  P1OUT |= BIT0;
  timeoutvar=0;

  P1OUT &= ~BIT5;
  TACTL = TASSEL_2|ID_1|MC_2;

  /* smclk | /2 | conti */
  //CCR0 =  0xffff;
  
  TA0CCTL0 |= CCIE;
  while(timeoutvar<16);
  // 16 * 0xffff = 1048560 -> 1.048560 s
  // 1 sec for DHT to stabilize
  // __delay_cycles(1000000);

  P1OUT &= ~BIT0;
  P1DIR |=BITPIN1DW|BIT5;
  P1OUT &= ~BITPIN1DW;
  // P1OUT |= BIT5;
  
  TACTL = MC_0;
  // TIMER A stop
  TACTL = TASSEL_2|ID_3|MC_1; //1mhz
  // smclk : 2mhz | /8 250khz | up
  CCR0 = 0x1200;
  //CCR0 = 0xffff;
  TA0CCTL0 |= CCIE;
  // (1e6/4)*0c35
  timeoutvar=0;
  while(!timeoutvar);
  // TIMER A stop
  // keep pin low for ~20 ms (18ms for dht to get start signal)
  //while(timeoutvar<16);
  P1OUT &= ~BIT5; 

 

  P1REN |= BITPIN1DW;
  P1DIR &= ~BITPIN1DW; // start input pulling up
  P1OUT |= BITPIN1DW;
 
  while(P1IN &  BITPIN1DW); // i'm pulling up, wait till dev pulls down;
  while(! (P1IN &  BITPIN1DW)); // dev pulls down;
  while((P1IN &  BITPIN1DW)); // dev pulls up;
  //then dev will start by pulling down
  while(P1IFG & BITPIN1DW)
    P1IFG &= ~BITPIN1DW;
  readingbit=0;
  P1IES &= ~BITPIN1DW; // interrupt on raising edge
  P1IE |= BITPIN1DW; // enable interrupt

  timeoutvar=0;
  while( (readingchar<5) && (!timeoutvar));

  uart_puts("RH:");
  uart_putcuc(read_value[0]);
  uart_puts("\n\rT:");
  uart_putcuc(read_value[2]);
  uart_puts("\n\rChck:");
  if(read_value[4] == (read_value[3]+read_value[2]+read_value[1]+read_value[0])){
    uart_puts("OK\n\r");
  }else{
    uart_puts("KO\n\r");
  }
  
  uart_puts("\n\r");

  

  while(1){
    P1OUT &= ~(BIT5); ;
  }
}


void uart_putc(unsigned char c)
{
        while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
        UCA0TXBUF = c;                                  // TX
}
 
void uart_puts(char *str)
{
     while(*str) uart_putc(*str++);
}
 
void uart_putcuc(unsigned char c){
  char chararray[4];
  chararray[3]=0;chararray[2]=0;chararray[1]=0;
  unsigned char i = 3;
 
  if(! c){
    chararray[0]='0';
    i=0;
  }
  while(c){
    i--;  
    chararray[i] = c%10 + '0';
    c /= 10;  
  }
  uart_puts(chararray+i);
}


void Setup_uart(){
 
        UCA0CTL1 |= UCSSEL_2;                     // SMCLK = 250kz
        UCA0BR0 = 208;                            // 2MHz 9600
        UCA0BR1 = 0;                              // 2MHz 9600
        UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
        UCA0CTL1 &= ~UCSWRST;                     // Initialize USCI state machine
        IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
 
}
