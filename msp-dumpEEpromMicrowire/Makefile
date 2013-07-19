NAME 	:= dumpEEpromMicrowire
TARGCHIP := 2553
TARGET 	:= $(NAME).elf
CC 	:= msp430-gcc
CFLAGS	:= -Os -ggdb3 -Wall -mmcu=msp430g$(TARGCHIP)
OBJS 	:= $(NAME).o uart.o

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(CFLAGS) $(OBJS)

uart.o:
	$(CC) $(CFLAGS) -o uart.o -c uart.c

clean :
	rm -rf $(TARGET) $(OBJS)

prog :	$(TARGET)
	mspdebug -q rf2500 "prog $(TARGET)"

