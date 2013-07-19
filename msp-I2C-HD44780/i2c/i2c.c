#include "i2c.h"

unsigned char Rx;
unsigned int RXByteCtr;
unsigned int TXByteCtr;
unsigned char *pBuffer;



char I2C_TXCallback(unsigned char c){
  if( (IFG2 & UCB0RXIFG) || (IFG2 & UCB0TXIFG) ){
    if(Rx == 1){
      if( IFG2 & UCB0RXIFG ){
	//	RXByteCtr--;
	if (RXByteCtr){
	  if(pBuffer)
	  *pBuffer++ = UCB0RXBUF; 
	  RXByteCtr--; // Move RX data to address PRxData
	  return 0;
	}else{
	  UCB0CTL1 |= UCTXSTP;                // No Repeated Start: stop condition
	  return 1;
	} 
      }
      return 0;
    }else{  
      // Master Transmit
      if (TXByteCtr)                        // Check TX byte counter
	{
	  if(pBuffer)
	  UCB0TXBUF = *pBuffer++;
	  TXByteCtr--;	  
	  return 0;                            // Decrement TX byte counter
	}else{
	UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
	return 1;
      }
    }
  }
  return 0;
  
}


void Setup_i2c_TX(int div,unsigned char addr){
  //  _DINT();
  Rx = 0;
  P1SEL =  BIT6 | BIT7;  // P1.1 = RXD, P1.2=TXD , P1.6 = SDA, P1.7=SCL
  P1SEL2 =  BIT6 | BIT7; 
  IE2 &= ~UCB0RXIE;  
  while (UCB0CTL1 & UCTXSTP);               // Ensure stop condition got sent// Disable RX interrupt
  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
  UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
  //  UCB0BR0 = 13;                             // fSCL = DCO/13 = ~75kHz
  UCB0BR0 = div & 0xff;                             // fSCL = DCO/20 = ~50kHz
  UCB0BR1 = div >> 8;
  UCB0I2CSA = addr;                         // Slave Address is 0x27
  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  // IE2 |= UCB0TXIE;                          // Enable TX interrupt
}
void Setup_i2c_RX(int div,unsigned char addr){
  //_DINT();
  Rx = 1;
  P1SEL =  BIT6 | BIT7;  // P1.1 = RXD, P1.2=TXD , P1.6 = SDA, P1.7=SCL
  P1SEL2 =  BIT6 | BIT7; 
  IE2 &= ~UCB0TXIE;  
  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
  UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
  //  UCB0BR0 = 13;                             // fSCL = DCO/13 = ~75kHz
  UCB0BR0 = div & 0xff;                             // fSCL = DCO/20 = ~50kHz
  UCB0BR1 = div >> 8;
  UCB0I2CSA = addr;                         // Slave Address is 048h
  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  // IE2 |=  UCB0RXIE|UCB0TXIE;                          // Enable RX interrupt
}
void Transmit_i2c(unsigned char *TxBuffer,unsigned char TxBuffersz){
  __bic_SR_register(GIE);
  pBuffer = TxBuffer;                      // TX array start address
  TXByteCtr = TxBuffersz;                  // Load TX byte counter
  while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
  UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
  IE2 = UCB0TXIE;
  __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
}
void Receive_i2c(unsigned char *RxBuffer,unsigned char RxBuffersz){
  _DINT();
  pBuffer = RxBuffer;    // Start of RX buffer
  RXByteCtr = RxBuffersz-1;              // Load RX byte counter
  while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
  UCB0CTL1 |= UCTXSTT;  
         IE2 |=  UCB0RXIE|UCB0TXIE;          // I2C start condition
  __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
}
