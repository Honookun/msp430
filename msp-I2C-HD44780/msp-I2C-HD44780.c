#include <msp430.h>
#include "i2c/i2c.h"
#include "uart/uart.h"
#include "lcd/lcd.h"

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

volatile char timeoutvar;
void __attribute__((interrupt (TIMER0_A0_VECTOR))) TimerA0_a0_isr (void){
  timeoutvar++;P1OUT ^= BIT0;
}

void __attribute__((interrupt (USCIAB0TX_VECTOR))) USCIAB0TX_isr (void){
  if(IFG2 & UCB0TXIFG){
    if(I2C_TXCallback(0)){
      IE2 &=  ~(UCB0RXIE|UCB0TXIE);
      __bic_SR_register_on_exit(CPUOFF);
    }
  }
  if(IFG2 & UCB0RXIFG){
     if(I2C_TXCallback(UCB0RXBUF)){
      IE2 &=  ~(UCB0RXIE|UCB0TXIE);
      __bic_SR_register_on_exit(CPUOFF);
    }
  }
}

void __attribute__((interrupt (USCIAB0RX_VECTOR))) USCIAB0RX_isr (void){

 
  if(IFG2 & UCA0RXIFG){
    while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
     UCA0TXBUF = UCA0RXBUF; // echo and set command global
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
  char rep;
  P1OUT |= 0x01;

  DCOCTL  = CALDCO_8MHZ;
  BCSCTL1 = CALBC1_8MHZ;
  /* DEVICE DEPENDENT !!! 
     DCO running @16MHz => internal calibration
     BOTH LINE COUNTS : RSEL are in BCSCTL1 while subfreq in range for
     DCO is in DCOCTL
  */


  BCSCTL2 =   DIVS1|DIVS0;//8 -> smlk 1mhz

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
int main(){
  
  WDTCTL = WDTPW|WDTHOLD;
  initClock8Mhz_SMCLK_D2();
P1DIR |= BIT0;
P1OUT ^= BIT0;
  WRITE_SR(GIE);
  Setup_uart(104,0,"* i2cLCD");
  SetupLCD();
  

  while(1);
}
