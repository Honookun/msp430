NAME 	 := msp-WaveShare-MicroSDv3
TARGCHIP := 2553
TARGET 	 := $(NAME).elf
CC 	 := msp430-gcc
CFLAGS	 := -Wall -mmcu=msp430g$(TARGCHIP)

ifndef MSP_PROD
CFLAGS	 += -ggdb3 -O0
else
$(info MSP_PROD defined : No Debug Symbol + Opt Size!)
CFLAGS	 += -Os
endif
ifndef MAXIMIZE
CFLAGS += -DMINIMYZE
endif
ifdef HARNESS

CFLAGS += -DHARNESS=$(HARNESS)


endif

B := main
OBJS 	 := $(NAME).o uart.o SPIv2.o  mmcv2.o 

#SPIv2.o mmcv2.o 
#pffdiskio.o pff.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -Wl,-Map=$(TARGET).map
	msp430-size $(TARGET)

uart.o: uart/uart.c uart/uart.h
	$(CC) $(CFLAGS) -o uart.o -c uart/uart.c

SPIv2.o : g2553_MMCv2/SPI.c g2553_MMCv2/SPI.h
	$(CC) $(CFLAGS) -o SPIv2.o -c g2553_MMCv2/SPI.c

mmcv2.o : g2553_MMCv2/mmcv2.c g2553_MMCv2/mmcv2.h
	$(CC) $(CFLAGS) -o mmcv2.o -c g2553_MMCv2/mmcv2.c

pffdiskio.o : pff2asrc/diskio.c
	$(CC) $(CFLAGS) -o pffdiskio.o -c pff2asrc/diskio.c

pff.o : pff2asrc/pff.c
	$(CC) $(CFLAGS) -o pff.o -c pff2asrc/pff.c
clean :
	rm -rf $(TARGET) $(OBJS) $(OBJSV2)

gdb :
	mspdebug --force-reset -q rf2500 "gdb" > /dev/null &
	sleep 1
	echo "target remote :2000\nb "$(B)"\nc" > .gdbinit
	msp430-gdb -q -se $(TARGET)

prog :
	mspdebug --force-reset -q rf2500 "prog $(TARGET)"
