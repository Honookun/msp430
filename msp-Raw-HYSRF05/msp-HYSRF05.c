#include <msp430.h>
#include "uart/uart.h"

#define HYSRF05POUT P2OUT
#define HYSRF05PDIR P2DIR
#define HYSRF05PIES P2IES
#define HYSRF05PIFG P2IFG
#define HYSRF05PIE P2IE
#define HYSRF05_TRIGGERPIN BIT1
#define HYSRF05_ECHOPIN BIT2

unsigned int time = 0;

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

volatile unsigned char timeoutvar;
void __attribute__((interrupt (TIMER0_A0_VECTOR))) TimerA0_a0_isr (void){
  timeoutvar--;
  if(!timeoutvar){
    __bic_SR_register_on_exit(CPUOFF);
  }
}

void __attribute__((interrupt (PORT1_VECTOR))) Port1_isr (void){
  if(P1IFG & BIT3){
    P1IE &= ~BIT3;
    __bic_SR_register_on_exit(CPUOFF);
    while(P1IFG & BIT3)
      P1IFG &= ~BIT3;
  }
}

void __attribute__((interrupt (PORT2_VECTOR))) Port2_isr (void){
  if(HYSRF05PIFG & HYSRF05_ECHOPIN){
    if(!(HYSRF05PIES & HYSRF05_ECHOPIN)){
      // First Rising
      TAR=0;TA0CCTL0 |= CCIE;
      TACTL = TASSEL_2|MC_2;  // 1 MHZ TA0 CONTI
      HYSRF05PIES |= HYSRF05_ECHOPIN; //prepare for next edge
    }else{
      time=TAR;
      TACTL = MC_0;//Stop timer
      __bic_SR_register_on_exit(CPUOFF);
    }
    while(HYSRF05PIFG & HYSRF05_ECHOPIN)
      HYSRF05PIFG &= ~HYSRF05_ECHOPIN;
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
void initClock8Mhz_SMCLK_D8(){
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

unsigned int Sample(unsigned char _SampleSz){
  unsigned int loctime = 0;
  unsigned char SampleSz = _SampleSz;
  while(SampleSz--){
    time=0;
    HYSRF05POUT|= HYSRF05_TRIGGERPIN;
    _DINT();
    TACTL = TASSEL_2|MC_1;//smclk /1 : 1mhz
    TACCR0=0xa; // 10us
    timeoutvar=1; TA0CCTL0 |= CCIE;
    __bis_SR_register(CPUOFF + GIE);
    HYSRF05POUT&=~ HYSRF05_TRIGGERPIN; //ensure low
    TACTL = MC_0;//Stop Timer
    //  TA0CCTL0 &= ~CCIE; // TA0 interrupt off
    
    _DINT();
    HYSRF05PIES &=~ HYSRF05_ECHOPIN;
    HYSRF05PIE |= HYSRF05_ECHOPIN;
    __bis_SR_register(CPUOFF + GIE);
    loctime+=time;
  }
  return(loctime/ _SampleSz);
}

int main(){
  
WDTCTL = WDTPW|WDTHOLD;
  initClock8Mhz_SMCLK_D8();
  P1DIR |= BIT0;
  P1OUT ^= BIT0;
  

  
  
  Setup_uart(104,0,"* ultraSnd Range Sensor");
  HYSRF05POUT&=~ HYSRF05_TRIGGERPIN; //ensure low
  HYSRF05PDIR|=HYSRF05_TRIGGERPIN; //default input
  
  P1OUT |= BIT3;
  P1REN |=BIT3;

  while(1){
  uart_putnbr(Sample(1)/58);
  uart_CarRet();
  P1IE |= BIT3;
   __bis_SR_register(CPUOFF + GIE);
  }


}
