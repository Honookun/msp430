#include <msp430.h>
 
unsigned char datearray[7];    
unsigned char datearraysz = 7;
unsigned char Txarray[8];
unsigned char RXByteCtr,TXByteCtr, RX = 0;


volatile unsigned char *PRxData;
unsigned char *PTxData;

void Setup_i2c_TX(unsigned char addr);
void Setup_i2c_RX(unsigned char addr);
void Transmit_i2c(unsigned char *TxBuffer,unsigned char TxBuffersz);
void Receive_i2c(unsigned char *RxBuffer,unsigned char RxBuffersz);


int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
  DCOCTL = CALDCO_1MHZ;
  P1DIR = 0xFF;                             // All P1.x outputs
  P1OUT = 0;                                // All P1.x reset

  BCSCTL2 = DIVS0 ;                         // SMCLK = DCO -> 500khz
  BCSCTL3 = XCAP_3;                         // for xtal

  P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
  P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0

  UCB0CTL1 |= UCSWRST;  

int i;
  for(i=0;i<8;i++)
    Txarray[i]=i; 

  // i2c mem adress = 0; 
  // Txarray[0]=0;
  
  
  /*
    Send the initial array 0-...-7
   */
  Setup_i2c_TX(0x68);
  Transmit_i2c(Txarray,8);
  while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
 
  /*
    then send address to read (0x0)
   */


  Setup_i2c_TX(0x68);
  Transmit_i2c(Txarray,1);
  while (UCB0CTL1 & UCTXSTP);  // Ensure stop condition got sent
 
  /*
    Read back
   */
                    
  Setup_i2c_RX(0x68);
  Receive_i2c(datearray,datearraysz);
  while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent


  __bis_SR_register(CPUOFF + GIE); // <- BREAKPOINT IT HERE then x/7xc datearray
  while(1){
       __bis_SR_register(CPUOFF + GIE);

  };

}


void __attribute__((interrupt (USCIAB0TX_VECTOR))) USCIAB0TX_ISR(void){

   if(RX == 1){                              // Master Recieve?
     	 RXByteCtr--;    
       if (RXByteCtr){
	 *PRxData++ = UCB0RXBUF;                 // Move RX data to address PRxData
       } 
       else{

	 /* Move final RX data to PRxData
	    will not work
	    (for a strange reason need +1 or it will do nothing not even crush the last-1 byte)
	    as is rx buffer is like (nbr desgnates the rank of the byte, . designates a null byte)
	    [0,1,2,3,4,5,7,.]
	    with +1 :
	    [0,1,2,3,4,5,7,6] which is strange enough, i suspect it is linked to
	    http://www.ti.com/litv/pdf/slaz440d (errata for the device : usci30)
	    but then i don't understand why it happens just for this value.
	    
	 */	
	   UCB0CTL1 |= UCTXSTP;                // No Repeated Start: stop condition
	  *(PRxData) = UCB0RXBUF;
	 __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
       } 
   }else{  
     // Master Transmit
     if (TXByteCtr)                        // Check TX byte counter
       {
	 UCB0TXBUF = *PTxData++;
	 TXByteCtr--;                            // Decrement TX byte counter
       }
     else
       {
	 UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	 IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
	 __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
       }
   }
}

/* following functions come straight from TI example*/
/* just here in order to work */

void Setup_i2c_TX(unsigned char addr){
  _DINT();
  RX = 0;
  IE2 &= ~UCB0RXIE;  
  while (UCB0CTL1 & UCTXSTP);               // Ensure stop condition got sent// Disable RX interrupt
  UCB0CTL1 |= UCSWRST;                      // Enable SW resetx
  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
  UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
  UCB0BR0 = 20;                             // fSCL = DCO/2/5 = ~100kHz
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
  UCB0BR0 = 20;                             // fSCL = DCO/2/5 = ~100Hz
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
