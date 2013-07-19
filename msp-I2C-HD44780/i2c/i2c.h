#include <msp430.h>
#ifndef _I2c__H
#define _I2c__H
char I2C_TXCallback(unsigned char c);
void Setup_i2c_TX(int div,unsigned char addr);
void Setup_i2c_RX(int div,unsigned char addr);
void Transmit_i2c(unsigned char *TxBuffer,unsigned char TxBuffersz);
void Receive_i2c(unsigned char *RxBuffer,unsigned char RxBuffersz);



#endif
