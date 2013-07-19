#include <msp430.h>
#include "uart/uart.h"
#include "notes.125khz.h"
#include "song.h"



unsigned int c=0;
unsigned char up=0;
unsigned char timeoutvar=2;

void __attribute__((interrupt (TIMER0_A0_VECTOR))) TA0CC0_isr (void){
  timeoutvar--;
  if(!timeoutvar){
    timeoutvar=2;
    if( c == SONG_SIZE)
      c=0;
    if(notes[c]){
    TA1CCR0 = freqs[notes[c]];
    TA1CCR1 = TA1CCR0>>1; //50% PWM
    TA1CTL|=MC_1;
    TA0CCR0 = durations[c++];
    }else{
      TA1CTL&=~MC_1;
       TA0CCR0 = durations[c]+1;
      c++;
    }
  }

}

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
  BCSCTL1 |= DIVA_2|XT2OFF;  /* ACLK = LFXTCLK =  32768hz xtal, /4 prescaler
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
  initClock8Mhz_SMCLK_D8();
  P1DIR |= BIT0;
  P1OUT ^= BIT0;

  P2DIR |= BIT1;
  P2SEL |= BIT1;
  
  c=0;

  _DINT();
  
  TA0CTL = TASSEL_1|ID_0; // 32.767khz
  TA0CCR0 = 0x3fd2;
  

  TA1CCTL1 = OUTMOD_7;
  TA1CTL = TASSEL_2|ID_1; // 1Mhz/2 = 500khz

  TA0CTL|=MC_1;
  TA0CCTL0 |= CCIE;
 
  while(1){
   __bis_SR_register(CPUOFF + GIE);
  }


}
