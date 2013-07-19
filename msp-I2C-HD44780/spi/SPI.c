#include "SPI.h"


#ifndef DUMMY_CHAR
#define DUMMY_CHAR 0xFF
#endif

volatile unsigned char* _pBuffer; 
volatile unsigned long _size=0;
volatile unsigned int _i=0;/*can be def long if needed W > 0xffff... well*/
volatile unsigned int _k=0;


#define SPI_STATUS_INIT 0x1
#define SPI_STATUS_RX 0x2

volatile unsigned char status = SPI_STATUS_INIT;

volatile unsigned char CurrDummyChar = DUMMY_CHAR;


unsigned char (*_pipeliningControlRX) (unsigned char)=0;

void setPipeliningControlRX(
			  unsigned char (*pipeliningControlRX) (unsigned char)
			  ){
  _pipeliningControlRX=pipeliningControlRX;
}

inline unsigned char SPI_TXCallback(void){
  if( (_i == _size) ){
    while(IFG2 & UCB0TXIFG)  
    IFG2 &= ~UCB0TXIFG;
    return(1);
  }

  if(status&SPI_STATUS_RX){
    halSPITXBUF = CurrDummyChar;
    _i++;
  }else{
    halSPITXBUF = _pBuffer[_i++];
  }
  return(0);
}

inline unsigned char SPI_RXCallback(unsigned char c){
  if(_pBuffer && (status&SPI_STATUS_RX))
    _pBuffer[_k] = c;
  
  if(_pipeliningControlRX){
    if((*_pipeliningControlRX)(c)){
      _pipeliningControlRX=0;
      _i=_k=_size;
    }
  }
 
  if(++_k>=_size){
    _pipeliningControlRX=0;
    return 1;
  }
     
  return 0;
}

//Read a frame of bytes via SPI
unsigned char spiReadFrameAltDummy(unsigned char AltDummy, unsigned int size)
{
  /* will send size dummy chars without reading if pBuffer = NULL (0L)  */
  /* see interrupt callback*/
  if(! size)
    return(0);
  CurrDummyChar = AltDummy;
  _i=1;_k=0;
  _size=size;
  _pBuffer=0;

  
  status|=SPI_STATUS_RX;
  halSPITXBUF = AltDummy;
 
  IE2 |=  UCB0RXIE|UCB0TXIE;
  if(size>1) // avoid beeing starved of RX
  __bis_SR_register(LPM1_bits + GIE);


  return(0);
}

//Read a frame of bytes via SPI
unsigned char spiReadFrame(unsigned char* pBuffer, unsigned int size)
{
  /* will send size dummy chars without reading if pBuffer = NULL (0L)  */
  /* see interrupt callback*/
  if(! size)
    return(0);
  CurrDummyChar = DUMMY_CHAR;
  _i=1;_k=0;
  _size=size;
  _pBuffer=pBuffer;
  status|=SPI_STATUS_RX;
  
  halSPITXBUF = DUMMY_CHAR;

  IE2 |=  UCB0RXIE|UCB0TXIE;
   if(size>1)// avoid beeing starved of RX
  __bis_SR_register(LPM1_bits + GIE);


  return(0);
}


//Send a frame of bytes via SPI
unsigned char spiSendFrame(unsigned char* pBuffer, unsigned int size)
{
  if(! size)
    return(0);
  /* if(size==1){ */
  /*   IFG2|=  UCB0TXIFG; */
  /*   halSPITXBUF =*pBuffer; */
  /*   return(0); */
  /* } */
    
  CurrDummyChar = DUMMY_CHAR;
  _i=_k=0;
  _size=size;
  _pBuffer=pBuffer;
  
  status&=~SPI_STATUS_RX;
  IFG2|=  UCB0TXIFG;
  if(size==1)
    IE2 |=  UCB0TXIE;
  else
    IE2 |=  UCB0RXIE|UCB0TXIE;
  __bis_SR_register(LPM1_bits + GIE);
  return(0);
}

void INITSPISetup(unsigned char div0,unsigned char div1)
{
  
  _pipeliningControlRX=0;
  UCB0CTL0 = UCMST+UCCKPL/*+UCCKPL*/+UCMSB+UCSYNC;     // 3-pin, 8-bit SPI master
  UCB0CTL1 = UCSSEL_2+UCSWRST;              // SMCLK
  UCB0BR0 = div0;                          // 4 Mhz/10 = 400khz
  UCB0BR1 = div1;
  UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

}

//Send one byte via SPI
void spiSendDummyByte()
{
  IFG2|=  UCB0TXIFG;
  halSPITXBUF = DUMMY_CHAR;
}




