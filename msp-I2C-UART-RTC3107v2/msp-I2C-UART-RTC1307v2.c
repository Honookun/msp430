#include <msp430.h>
 

// main data 
unsigned char tmpdatearray[7];    
unsigned char datearray[7];    
unsigned char datearraysz = 7;

// main utility functions
void datefromstrings(unsigned char* datearray, char* date,char* time);
void fromtobcdarray(unsigned char* arrayin,unsigned char* arrayout, unsigned char arraysz,char dir);
unsigned char* parsedatetimestring(unsigned char* tststr,unsigned char* dest);
unsigned char Ask4confirm();

// I2C management data
unsigned char Txarray[8];
unsigned char RXByteCtr,TXByteCtr, RX = 0;

unsigned char *PRxData;
unsigned char *PTxData;

// UART management data
unsigned char command;
unsigned char commandr;
unsigned char commandw;
 
char * sepline =  "**************************\n\r";
char * nret ;
char clrscrn[5] = {0x1b,0x5b,0x32,0x4a,0x0};

char command_buffer[128];
unsigned char command_buffer_rnk;
unsigned char command_writing;
unsigned char command_buffersz=128;

char * multicharcommands = "sh";
char * monocharcommands = "r";
char * multicharcommandsptr;

// I2C management functions declaration
void Setup_i2c_TX(unsigned char addr);
void Setup_i2c_RX(unsigned char addr);
void Transmit_i2c(unsigned char *TxBuffer,unsigned char TxBuffersz);
void Receive_i2c(unsigned char *RxBuffer,unsigned char RxBuffersz);

// UART management functions declaration
void Setup_uart();
void uart_putc(unsigned char c);
void uart_puts(char *str);
void uart_putcuc(unsigned char c);
void uart_print_datetime(unsigned char* datearray);



int main(void)
{
 
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
  DCOCTL = CALDCO_1MHZ;
  P1DIR = 0xFF;                             // All P1.x outputs
  P1OUT = 0;                                // All P1.x reset

  // BCSCTL2 = DIVS0 ;                   // SMCLK = DCO -> 500khz
  BCSCTL3 = XCAP_3;                         // for xtal


  P1SEL = BIT1 + BIT2 + BIT6 + BIT7;  // P1.1 = RXD, P1.2=TXD , P1.6 = SDA, P1.7=SCL
  P1SEL2 = BIT1 + BIT2 + BIT6 + BIT7;     
  UCB0CTL1 |= UCSWRST;  

  P1OUT |= BIT0; // light led during rtc setup;

  datefromstrings(Txarray+1,__DATE__,__TIME__);
  fromtobcdarray(Txarray+1,Txarray+1,7,1);
  nret=sepline+26;
  //i2c mem adress = 0; 
  Txarray[0]=0;

  //cpy txarray in datearray
  
  unsigned char i,j;
  for(i=0;i<7;i++){
    datearray[i]=Txarray[i+1];
  }
  
  /*
    send init data
   */

  Setup_i2c_TX(0x68);
  Transmit_i2c(Txarray,8);
  while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
 

  /*
    then send address to read
   */

  Setup_i2c_TX(0x68);
  Transmit_i2c(Txarray,1);
  while (UCB0CTL1 & UCTXSTP);  // Ensure stop condition got sent
  
                                                                                         //Receive process
  Setup_i2c_RX(0x68);
  Receive_i2c(Txarray+1,datearraysz);
  while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
  
  // DATA ON THE LINE :
  // [0xD1+0x01+0x02+0x03+0x04+0x05+0x06+0x07-]
  

  // resend it to check on log analzr
   Setup_i2c_TX(0x68);
  Transmit_i2c(Txarray,8);
  while (UCB0CTL1 & UCTXSTP);

  // DATA ON THE LINE :
  // [0xD0+0x01+0x02+0x03+0x04+0x05+0x07+0x00+]
  
  
  i=0;j=1;
  while((i<7) && j){
    if(datearray[i]==Txarray[i+1]){
      i++;
    }else{
      j=0;
    }
  }
  if(j)
    P1OUT ^= BIT0;
  // rtc init success : switch off led

  fromtobcdarray(datearray,datearray,7,0);
 
  //init uart
  Setup_uart();
  uart_puts(clrscrn);
  uart_puts(nret);
  uart_puts(sepline);
  uart_puts("*  MSP430 :  UART + I2C  *");
  uart_puts(nret);
  uart_puts(sepline);
  uart_puts("Compiled :");
  uart_print_datetime(datearray);
  uart_puts(nret);
  uart_puts(sepline);

  command=0;
  command_writing=0; 
  multicharcommandsptr= multicharcommands;

  while(1){
   
    uart_putc('>');
       __bis_SR_register(CPUOFF + GIE);
       P1OUT ^= BIT0;
       switch(command){
       case 'r':
	 Setup_i2c_TX(0x68);
	 Transmit_i2c(Txarray,1);
	 while (UCB0CTL1 & UCTXSTP);
	 Setup_i2c_RX(0x68);
	 Receive_i2c(datearray,datearraysz);
	 while (UCB0CTL1 & UCTXSTP);
	 fromtobcdarray(datearray,datearray,7,0);
	 uart_print_datetime(datearray);
	 uart_puts(nret);
	 break;
       case 's':
	 // uart_puts(command_buffer);
	 // uart_puts(nret);
	 if(! parsedatetimestring((unsigned char*)command_buffer+2,tmpdatearray)){
	   uart_puts("error in parsing :");
	   uart_puts(command_buffer+2);
	   uart_puts(nret);
	   command_writing=0;
	 }else{
	    uart_print_datetime(tmpdatearray);
	    if(Ask4confirm()){
	      fromtobcdarray(tmpdatearray,Txarray+1,7,1);
	      *(Txarray) = 0;
	      Setup_i2c_TX(0x68);
	      Transmit_i2c(Txarray,8);
	      while (UCB0CTL1 & UCTXSTP);
	      for(i=0;i<6;i++){
		datearray[i]=tmpdatearray[i];
	      }
	    }
	 }
	 break;
       case 0:
	 break;
       default:
	 uart_putc(command);
	 uart_puts(" : Unknown Command"); uart_puts(nret);
	 break;
   }
  };

}

void __attribute__((interrupt (USCIAB0RX_VECTOR))) USCIAB0RX_ISR(void){
 
  if(IFG2 & UCA0RXIFG){
    while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
    commandr = UCA0TXBUF = UCA0RXBUF; // echo and set command global

    if(commandr == '\r'){
      uart_puts(nret);
      command=commandw;
      commandw=0;
      if(command_writing){
	command= *command_buffer;
	command_buffer[command_buffer_rnk]=0;
	command_writing=0;
      }
      command_buffer_rnk=0;
      multicharcommandsptr= multicharcommands;
      __bic_SR_register_on_exit(CPUOFF);
    }else{

      command=0;
      if(! command_writing)
      while(*multicharcommandsptr){
	if(*multicharcommandsptr++ == commandr)
	  command_writing=1;
      }
      if(command_writing){
	if(command_buffer_rnk<command_buffersz)
	command_buffer[command_buffer_rnk++] = commandr;
      }else{
	commandw=commandr;
	multicharcommandsptr= multicharcommands;
      }
    }
   
  }
}

void __attribute__((interrupt (USCIAB0TX_VECTOR))) USCIAB0TX_ISR(void){
  if( (IFG2 & UCB0RXIFG) || (IFG2 & UCB0TXIFG) ){
    if(RX == 1){
      if( IFG2 & UCB0RXIFG ){
	RXByteCtr--;
	if (RXByteCtr){
	  *PRxData++ = UCB0RXBUF;                 // Move RX data to address PRxData
	}else{
	  UCB0CTL1 |= UCTXSTP;                // No Repeated Start: stop condition
	  __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
	} 
      }
    }else{  
      // Master Transmit
      if (TXByteCtr)                        // Check TX byte counter
	{
	  UCB0TXBUF = *PTxData++;
	  TXByteCtr--;                            // Decrement TX byte counter
	}else{
	  UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	  IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
	  __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
	}
    }
  }
}

// Utility function 

unsigned char Ask4confirm(){
  
  while( (command != 'y') && (command != 'n') ){
    uart_puts(" : please confirm [y/n]? ");
    __bis_SR_register(CPUOFF + GIE);
  }
   if(command == 'y'){
     return(1);
   }   else{
     return(0);
   }
}

unsigned char* parsedatetimestring(unsigned char* tststr,unsigned char* dest){
  int ret = 0;
  unsigned char curr =0;
  unsigned char *ptr=tststr;
  char c[6];
  for(curr=0;curr<6;curr++)
    c[curr]=0;

  curr = 0;
  while(!ret){
    if(*ptr >= '0' && *ptr <='9' ){
      c[curr] *=10;
      c[curr] += (*ptr)-'0';
    }else{
      curr++;
    }
    if((*ptr == '\n')||(*ptr == 0))
      ret = 1;
    ptr++;
  }
  ret = 0;
  for(curr=0;curr<6;curr++)
    ret +=c[curr];

  if(ret<6)
    return 0;

   
  if(c[0]>23)
    return 0;
  if(c[1] > 59 || c[2] > 59)
    return 0;
  if(c[3] > 31)
    return 0;
  if(c[3] > 30)
    if(c[4] == 2 || c[4] == 4 || c[5] == 6 || c[4]==9 || c[4]==11 )
      return 0;
  if(c[4] > 12)	
    return 0;
  if(c[5] > 99)
    return 0;
  
  *(dest+2) = c[0];
  *(dest+1) = c[1];
  *(dest+0) = c[2];
  *(dest+4) = c[3];
  *(dest+5) = c[4];
  *(dest+6) = c[5];
  return dest;
}

void datefromstrings(unsigned char* datearray, char* date,char* time){
  /*from ladyada library*/
  datearray[6] = (*(date+9)-'0')*10+(*(date+10)-'0');
  datearray[4] = (*(date+4)-'0')*10+(*(date+5)-'0');
  switch (date[0]) {
  case 'J': datearray[5] = (date[1] == 'a' ? 1 :(date[2] == 'n' ? 6 : 7)); break;
  case 'F': datearray[5] = 2; break;
  case 'A': datearray[5] = (date[2] == 'r' ? 4 : 8); break;
  case 'M': datearray[5] = (date[2] == 'r' ? 3 : 5); break;
  case 'S': datearray[5] = 9; break;
  case 'O': datearray[5] = 10; break;
  case 'N': datearray[5] = 11; break;
  case 'D': datearray[5] = 12; break;
  }
  datearray[3] = 0;
  datearray[2] = (*(time)-'0')*10+(*(time+1)-'0');
  datearray[1] = (*(time+3)-'0')*10+(*(time+4)-'0');
  datearray[0] = (*(time+6)-'0')*10+(*(time+7)-'0');
}

void fromtobcdarray(unsigned char* arrayin,unsigned char* arrayout, unsigned char arraysz,char dir){
  unsigned char i = 0;
  unsigned char c;
  if(dir){
    for(i = 0 ; i < arraysz;i++){
      c = arrayin[i];
      arrayout[i] = (c%10 | ((c/10)<<4));
    }
  }else{
    for(i = 0 ; i < arraysz;i++){
      c = arrayin[i];
      arrayout[i] = (((c & 0b11110000)>>4)*10) + (c & 0b1111);
    }
  }
}

// Uart Management functions

void uart_print_datetime(unsigned char* datearray){
  uart_putcuc(datearray[2]);uart_putc(':');
  uart_putcuc(datearray[1]);uart_putc(':');
  uart_putcuc(datearray[0]);
  uart_putc(' ');
  uart_putcuc(datearray[4]);uart_putc('-');
  uart_putcuc(datearray[5]);uart_putc('-');
  uart_putcuc(datearray[6]);

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
        UCA0BR0 = 104;                             // 1MHz 9600
        UCA0BR1 = 0;                              // 1MHz 9600
        UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
        UCA0CTL1 &= ~UCSWRST;                     // Initialize USCI state machine
        IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
 
}



/* following functions come straight from TI example*/
/* just here in order to work */

void Setup_i2c_TX(unsigned char addr){
  _DINT();
  RX = 0;
  IE2 &= ~UCB0RXIE;  
  while (UCB0CTL1 & UCTXSTP);               // Ensure stop condition got sent// Disable RX interrupt
  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
  UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
  //  UCB0BR0 = 13;                             // fSCL = DCO/13 = ~75kHz
  UCB0BR0 = 20;                             // fSCL = DCO/20 = ~50kHz
  UCB0BR1 = 0;
  UCB0I2CSA = addr;                         // Slave Address is 048h
  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  IE2 |= UCB0TXIE;                          // Enable TX interrupt
}
void Setup_i2c_RX(unsigned char addr){
  _DINT();
  RX = 1;
  IE2 &= ~UCB0TXIE;  
  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
  UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
  //  UCB0BR0 = 13;                             // fSCL = DCO/13 = ~75kHz
  UCB0BR0 = 20;                             // fSCL = DCO/20 = ~50kHz
  UCB0BR1 = 0;
  UCB0I2CSA = addr;                         // Slave Address is 048h
  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  IE2 |=  UCB0RXIE|UCB0TXIE;                          // Enable RX interrupt
}
void Transmit_i2c(unsigned char *TxBuffer,unsigned char TxBuffersz){
  PTxData = TxBuffer;                      // TX array start address
  TXByteCtr = TxBuffersz;                  // Load TX byte counter
  while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
  UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
  __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
}
void Receive_i2c(unsigned char *RxBuffer,unsigned char RxBuffersz){
  PRxData = RxBuffer;    // Start of RX buffer
  RXByteCtr = RxBuffersz-1;              // Load RX byte counter
  while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
  UCB0CTL1 |= UCTXSTT;                    // I2C start condition
  __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
}
