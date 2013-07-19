#ifndef UART_H
#define UART_H
#include <msp430.h>
void Setup_uart(unsigned int ,unsigned char,char* );
void Stop_uart();
void uart_putnbr(unsigned long );
void uart_putssep(char *,char );
void uart_puts(char *);
void uart_sepline();
void uart_CarRet();
void uart_putc(unsigned char);
void uart_putsNsep(char *,char,int);
void uart_putnbrhex(unsigned long c);
#endif
