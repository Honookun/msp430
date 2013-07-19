#ifndef _SPILIB_H
#define _SPILIB_H


#include "msp430g2553.h"
#define SPI_PxSEL         P1SEL      // interfaces, according to the pin
#define SPI_PxSEL2        P1SEL2      // interfaces, according to the pin
#define SPI_PxDIR         P1DIR      // assignments indicated in the
#define SPI_PxIN          P1IN       // chosen MSP430 device datasheet.
#define SPI_PxOUT         P1OUT

#define SPI_CS_PxDIR      P1DIR
#define SPI_CS_PxOUT      P1OUT
#define SPI_CS            BIT4

#define SPI_CD_PxIN       P2IN
#define SPI_CD_PxDIR      P2DIR
#define SPI_CD            BIT0

#define SPI_SIMO          BIT7
#define SPI_SOMI          BIT6
#define SPI_UCLK          BIT5

/*
 * ABSTRACTION LAYER
 */

#define MMC_PxSEL         SPI_PxSEL      // interfaces, according to the pin
#define MMC_PxSEL2        SPI_PxSEL2
#define MMC_PxDIR         SPI_PxDIR      // assignments indicated in the
#define MMC_PxIN          SPI_PxIN       // chosen MSP430 device datasheet.
#define MMC_PxOUT         SPI_PxOUT      
#define MMC_SIMO          SPI_SIMO
#define MMC_SOMI          SPI_SOMI
#define MMC_UCLK          SPI_UCLK

// Chip Select
#define MMC_CS_PxOUT      SPI_CS_PxOUT     
#define MMC_CS_PxDIR      SPI_CS_PxDIR
#define MMC_CS            SPI_CS

// Card Detect
#define MMC_CD_PxIN       SPI_CD_PxIN
#define MMC_CD_PxDIR      SPI_CD_PxDIR
#define MMC_CD            SPI_CD

#define halSPIRXBUF  UCB0RXBUF
#define halSPITXBUF  UCB0TXBUF
#define halSPI_SEND(x) UCB0TXBUF=x
#define halSPITXREADY  (UC0IFG&UCB0TXIFG)     /* Wait for TX to be ready */
#define halSPITXDONE  (UCB0STAT&UCBUSY)       /* Wait for TX to finish */
#define halSPIRXREADY (UC0IFG&UCB0RXIFG)      /* Wait for TX to be ready */
#define halSPIRXFG_CLR UC0IFG &= ~UCB0RXIFG
#define halSPI_PxIN  SPI_USART0_PxIN
#define halSPI_SOMI  SPI_USART0_SOMI

#define CS_LOW()    MMC_CS_PxOUT &= ~MMC_CS               // Card Select
#define CS_HIGH()   while(halSPITXDONE); MMC_CS_PxOUT |= MMC_CS  // Card Deselect

#define DUMMY_CHAR 0xFF


// Function Prototypes
unsigned char SPI_TXCallback(void);
unsigned char SPI_RXCallback(unsigned char c);

void INITSPISetup (unsigned char,unsigned char);

void spiSendDummyByte();
unsigned char spiReadFrame(unsigned char* pBuffer, unsigned int size);
unsigned char spiReadFrameAltDummy(unsigned char AltDummy, unsigned int size);
unsigned char spiSendFrame(unsigned char* pBuffer, unsigned int size);
void setPipeliningControlRX(unsigned char (*pipeliningControlRX) (unsigned char));

#endif /* _SPILIB_H */
