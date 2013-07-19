#include "../i2c/i2c.h"
#ifndef LCD__H
#define LCD__H

#define LCD_CLEARDISPLAY 0x1
#define LCD_RETURNHOME 0x2
#define LCD_ENTRYMODESET 0x4
#define LCD_DISPLAYCONTROL 0x8

#define LCD_ENABLE 0x4
#define LCD_RW 0x2
#define LCD_RS 0x1
#define LCD_NULL 0x0

#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x2
#define LCD_ENTRYSHIFTINCREMENT 0x1
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x4
#define LCD_DISPLAYOFF 0x0
#define LCD_CURSORON 0x2
#define LCD_CURSOROFF 0x0
#define LCD_BLINKON 0x1
#define LCD_BLINKOFF 0x00
#define LCD_BACKLIGHT 0x8 // used to pick out the backlight flag since _displaycontrol will never set itself

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x8
#define LCD_CURSORMOVE 0x0
#define LCD_MOVERIGHT 0x4
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x8
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x4
#define LCD_5x8DOTS 0x00

#define LCD_SUCCESS 0x0
#define LCD_ERROR 0x1

#define FLAGS_LCD_BACKLIGHT 1
#define FLAGS_LCD_4BEN 2
#define FLAGS_LCD_CHARMODE 4

 extern volatile char timeoutvar;


char SetupLCD();
#endif
