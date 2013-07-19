#include <msp430.h>

volatile unsigned char timeoutvar ;
volatile unsigned char readingchar =0;
volatile unsigned char readingbit  =0;
volatile unsigned int timedetect;
volatile unsigned char init_D1W=0;
unsigned char read_value[5];

volatile unsigned int myflags = 0; 
#define FLG_1ST_READ_DHT11 1


char * sepline =  "**************************\n\r";
char * nret ;
char clrscrn[5] = {0x1b,0x5b,0x32,0x4a,0x0};
#define BITPIN1DW BIT4

unsigned char timeouts_array[] = {80,70};

void __attribute__((interrupt (TIMER0_A0_VECTOR))) TimerA0_a0_isr (void){
  timeoutvar++;
  /*  P1DIR |= BIT6;
      P1OUT ^= BIT6;*/
  __bic_SR_register_on_exit(LPM1_bits);
}

void __attribute__((interrupt (PORT1_VECTOR))) Port1_isr (void){
 
 
 
  if(P1IFG & BITPIN1DW){
    if(init_D1W){
      if(init_D1W < 3 ){
	init_D1W++;
	  }else{
	if(P1IES & BITPIN1DW){
#ifdef DEBUG_PIN_1_5
	  P1OUT &= ~BIT5;
#endif
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
#ifdef DEBUG_PIN_1_5
	    P1OUT |= BIT5;
	    P1OUT &= ~BIT5;
#endif
	  }
	  
	  if(readingbit==8){
	    readingbit=0;
	    readingchar++;
	    if(readingchar >4)
	      __bic_SR_register_on_exit(CPUOFF);
	  }
	  
	}else{
#ifdef DEBUG_PIN_1_5
	  P1OUT |= BIT5;
#endif
	  //  INTERRUPT FRONT RAISING EDGE OF 1DWIRE
	  if(readingchar<5){
	    TACTL = MC_0;
	    // TIMER A stop
	    TACTL |= TACLR;
	    TACTL = TASSEL_2|ID_1|MC_1; // 1mhz
	    // smclk | /2  | up
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
void Stop_uart();

void readD1W();

int main(void)
{

  WDTCTL = WDTPW|WDTHOLD;
  DCOCTL = 0;                               // Select lowest DCOx and MODx settings

  
  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
  DCOCTL = CALDCO_1MHZ;
  // BCSCTL2 =  DIVS1|DIVS0;  // 
  //smclk = dco/8 ie 2Mhz
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
 
  P1DIR |= BIT0;
  P1OUT |= BIT0;
 
  myflags |= FLG_1ST_READ_DHT11;
  while(1){

  
  readD1W();
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
  
  TACTL = TASSEL_2|ID_3|MC_1;
  // smclk : 1Mhz | /8 | conti 
  //CCR0 =  0xffff;
  CCR0 = 0xf480;
  TA0CCTL0 |= CCIE;
  timeoutvar=0;
  while(timeoutvar<10 /* /2 = sec */){
    __bis_SR_register(LPM1_bits + GIE);
  };
    

  }

  
}

void readD1W(){
  timeoutvar=0;
  read_value[4] = read_value[3] = read_value[2] = read_value[1] = read_value[0] =0;
  P1OUT &= ~BIT5;
  if(myflags & FLG_1ST_READ_DHT11){
    TACTL = TASSEL_2|ID_3|MC_1;
    /* smclk : 1Mhz | /8 | conti */
    //CCR0 =  0xffff;
    CCR0 = 0xf480;
    TA0CCTL0 |= CCIE;
    timeoutvar=0;
    while(timeoutvar<2){
       __bis_SR_register(LPM1_bits + GIE);
    };
    // 16 * 0xffff = 1048560 -> 1.048560 s
    // 1 sec for DHT to stabilize
    // __delay_cycles(1000000);
    myflags &= ~ FLG_1ST_READ_DHT11;
  } 

  P1REN &= ~BITPIN1DW;
  P1OUT &= ~BIT0;
  P1DIR |=BITPIN1DW;

#ifdef DEBUG_PIN_1_5
  P1DIR |=BIT5;
#endif

  P1OUT &= ~BITPIN1DW;
  // P1OUT |= BIT5;
  
  TACTL = MC_0;
  // TIMER A stop
  TACTL = TASSEL_2|ID_3|MC_1; //1mhz
  // smclk : 1mhz | /8 250khz | up
  CCR0 = 0x8cb;
   TA0CCTL0 |= CCIE;
  // 0x8cb/(1e6/8) 18ms
  timeoutvar=0;
  while(!timeoutvar){
    __bis_SR_register(LPM1_bits + GIE);
  };
  // TIMER A stop
  // keep pin low for 18ms for dht to get start signal)
  //while(timeoutvar<16);
  P1OUT &= ~BIT5; 
  
  Stop_uart();

  BCSCTL1 = CALBC1_8MHZ;  // Set quick DCO for interrupts
  DCOCTL = CALDCO_8MHZ;   // 
  BCSCTL2 =  DIVS1; //|DIVS0; // 2mhz smclk

  P1REN |= BITPIN1DW;
  P1DIR &= ~BITPIN1DW; // start input pulling up
  P1OUT |= BITPIN1DW;
  init_D1W = 1;  
  readingbit  = 0;
  readingchar = 0;
  P1IES &= ~BITPIN1DW; // interrupt on raising edge
  P1IE |= BITPIN1DW; // enable interrupt

  timeoutvar=0;
  __bis_SR_register(LPM1_bits + GIE);
  
  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
  DCOCTL = CALDCO_1MHZ;
  BCSCTL2 =0;
  P1IE &= ~BITPIN1DW; // disable interrupts
  P1IFG &= ~BITPIN1DW; // clear interrupt
  Setup_uart();


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

inline void Stop_uart(){
UCA0CTL1 |= UCSWRST;
}

void Setup_uart(){
 
        UCA0CTL1 |= UCSSEL_2;                     // SMCLK = 2mhz
        UCA0BR0 = 104;                            // 1MHz 9600
        UCA0BR1 = 0;                              // 1MHz 9600
        UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
        UCA0CTL1 &= ~UCSWRST;                     // Initialize USCI state machine
        IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
 
}
