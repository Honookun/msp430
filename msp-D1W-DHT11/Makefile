NAME 	:= msp-D1W-DHT11
TARGCHIP := 2553
TARGET 	:= $(NAME).elf
CC 	:= msp430-gcc
CFLAGS	:= -Os -Wall -mmcu=msp430g$(TARGCHIP) -ggdb3
OBJS 	:= $(NAME).o

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(CFLAGS) $(OBJS)

clean :
	rm -rf $(TARGET) $(OBJS)

gdb :
	mspdebug --force-reset -q rf2500 "gdb" > /dev/null &
	sleep 3
	echo "target remote :2000\nb main\nc" > .gdbinit
	msp430-gdb -q -se $(TARGET)

prog :
	mspdebug --force-reset -q rf2500 "prog $(TARGET)"