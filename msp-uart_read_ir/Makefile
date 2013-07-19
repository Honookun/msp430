NAME 	:= uart_read_ir
TARGCHIP := 2553
TARGET 	:= $(NAME).elf
CC 	:= msp430-gcc
CFLAGS	:= -Os -Wall -mmcu=msp430g$(TARGCHIP)
OBJS 	:= $(NAME).o uart.o

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(CFLAGS) $(OBJS)

uart.o : uart.c uart.h
	$(CC) -o uart.o -c $(CFLAGS) uart.c

clean :
	rm -rf $(TARGET) $(OBJS)

prog :
	mspdebug -q rf2500 "prog $(TARGET)"