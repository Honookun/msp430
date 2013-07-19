#include "lcd.h"

char flags=0;

char SendCommand(unsigned char cmd);

void clear(){
unsigned char oldflags = flags;
 flags&=~FLAGS_LCD_CHARMODE;
  SendCommand(LCD_CLEARDISPLAY);
 flags=oldflags;
 TACCR0=0x100; // setup 2.ms 
 timeoutvar=0;TAR=0;TACTL |=MC_1; //TA0 start up mode
 while(!timeoutvar);
}
void home(){
  unsigned char oldflags = flags;
  flags&=~FLAGS_LCD_CHARMODE;
  SendCommand(LCD_RETURNHOME);
  flags=oldflags;
  TACCR0=0x100; // setup 2.ms 
  timeoutvar=0;TAR=0;TACTL |=MC_1; //TA0 start up mode
  while(!timeoutvar);
}

void SetCursor(unsigned char col,unsigned char row){
  unsigned char oldflags = flags;
  unsigned char row_offsets[] = { 0x0, 0x40,0x14,0x54 };
  flags&=~FLAGS_LCD_CHARMODE;
  SendCommand(LCD_SETDDRAMADDR | (col + row_offsets[row]));
  flags=oldflags;
  /* TACCR0=0x7; // setup 100us 
  timeoutvar=0;TAR=0;TACTL |=MC_1; //TA0 start up mode
  while(!timeoutvar);*/
}

void lcdprint(unsigned char* str){
  unsigned char i=0;
  clear();
  home();
  flags|=FLAGS_LCD_CHARMODE;
  while(*str){
    if(i==16){
      SetCursor(0,1); 
    }
    SendCommand(*str++);
    i++;
  }

}

void addChar(char addr,unsigned char* bits){
  unsigned char oldflags = flags;
  unsigned char i;
  flags&=~FLAGS_LCD_CHARMODE;
  SendCommand(LCD_SETCGRAMADDR | (addr<<3));
  flags|=FLAGS_LCD_CHARMODE;
  for(i=0;i<8;i++)
    SendCommand(*bits++);
  flags = oldflags;
}

char SendCommand(unsigned char cmd){
  unsigned char buff;
  

  if(flags &  FLAGS_LCD_4BEN){

    buff = (cmd&0xf0)|LCD_ENABLE ;
    if(flags&FLAGS_LCD_BACKLIGHT)
      buff |= LCD_BACKLIGHT;
    if(flags&FLAGS_LCD_CHARMODE)
      buff |= LCD_RS;
    Transmit_i2c(&buff,1);
    buff &=~LCD_ENABLE ;
    Transmit_i2c(&buff,1);

    buff = ((cmd&0xf)<<4)|LCD_ENABLE ;
    if(flags&FLAGS_LCD_BACKLIGHT)
      buff |= LCD_BACKLIGHT;
    if(flags&FLAGS_LCD_CHARMODE)
      buff |= LCD_RS;
    Transmit_i2c(&buff,1);
    buff &=~LCD_ENABLE ;
    Transmit_i2c(&buff,1);
  } else 
    Transmit_i2c(&cmd,1);
  /* TACTL = TASSEL_2|ID_3|MC_1;
  TACCR0=0x65; // setup 100us 
  timeoutvar=0;TAR=0;TACTL |=MC_1; //TA0 start up mode
  while(!timeoutvar);*/
  return LCD_SUCCESS;
}

char SetupLCD(){
 unsigned int c =0xff;
 unsigned char BatteryChar[]={0xe,0x1b,0x11,0x11,0x11,0x11,0x11,0x1f};
 unsigned char HxR[]={0x1f,0x15,0x1b,0xe,0,0x11,0xe,0x11};
 
 flags|=FLAGS_LCD_BACKLIGHT;

  Setup_i2c_TX(10,0x27);
  Transmit_i2c(((unsigned char*)&c),1);
  
 
 
  TACTL = TASSEL_2|ID_3|MC_1;//smclk /8
  TA0CCTL0 |= CCIE;
  TACCR0=0x2710; // 40ms
  timeoutvar=0;
  while(!timeoutvar);
  TACTL&= ~MC_1;
  SendCommand(LCD_FUNCTIONSET|LCD_8BITMODE);//30


  TACTL = TASSEL_2|ID_3|MC_1;TAR=0;
  TA0CCTL0 |= CCIE;
  TACCR0=0x20d; // setup 4.2ms
  timeoutvar=0;TACTL |=MC_1; //TA0 start up mode
  while(!timeoutvar);
  SendCommand(LCD_FUNCTIONSET|LCD_8BITMODE);//30
  
  TACCR0=0xd; // setup 100us 
  timeoutvar=0;TAR=0;TACTL |=MC_1; //TA0 start up mode
  while(!timeoutvar);
  SendCommand(LCD_FUNCTIONSET|LCD_8BITMODE);//30



  timeoutvar=0;TAR=0;TACTL |=MC_1; //TA0 start up mode
  while(!timeoutvar); 
  SendCommand(LCD_FUNCTIONSET);//30
  flags|=FLAGS_LCD_4BEN; // IN 4 BIT MODE

 
  //timeoutvar=0;TAR=0;TACTL |=MC_1; //TA0 start up mode
  //while(!timeoutvar); 
 SendCommand(LCD_FUNCTIONSET| LCD_2LINE| LCD_5x8DOTS);//30

 //timeoutvar=0;TAR=0;TACTL |=MC_1; //TA0 start up mode
 //while(!timeoutvar); 

  SendCommand(LCD_DISPLAYCONTROL| LCD_DISPLAYON  |  LCD_BLINKON);
  //SendCommand(LCD_DISPLAYCONTROL| LCD_DISPLAYON | LCD_CURSORON |  LCD_BLINKON);

  //SendCommand(LCD_CLEARDISPLAY);
  SendCommand(LCD_ENTRYMODESET|LCD_ENTRYRIGHT|LCD_ENTRYSHIFTDECREMENT);
  //home();
  clear();
  
  unsigned int j;
  addChar(0,BatteryChar);
  for(j=1;j<7;j++){
    *(BatteryChar+7-j)=0x1f;
    addChar(j,BatteryChar);
  }
  addChar(7,HxR);


 
 
  /* for(j=6;j>1;j--){
    *(BatteryChar+j)=0x1f;
    addChar(j,BatteryChar);
    }*/
		   
 
  lcdprint((unsigned char*)":::DemoString:::::::::::::::::::");
  TACTL = TASSEL_2|ID_3|MC_2;//smclk /8 */
  TA0CCTL0 |= CCIE; 
  timeoutvar=0; 
  while(timeoutvar<4);  


  

  for(j=0;j<1;j++){
  
    // home();
  clear();
  
  //SendCommand(LCD_SETDDRAMADDR);//dram adr =0
  SetCursor(0,1);
    flags|=FLAGS_LCD_CHARMODE;
    for(c=0+(32*j);c<(32*j)+8;c++) {
    SendCommand((unsigned char)c&0xff);
    }
    /* SetCursor(0,1);
    flags|=FLAGS_LCD_CHARMODE;
    for(c=16+(32*j);c<32+(32*j);c++) {
      SendCommand((unsigned char)c&0xff);
    }
    TACTL = TASSEL_2|ID_3|MC_2;//smclk /8
    TA0CCTL0 |= CCIE;
    timeoutvar=0;
    while(timeoutvar<8); */
  }
  TACTL = TASSEL_2|ID_3;
  // SetCursor(13,0); 
  // for(c=16;c<32;c++) 
  //SendCommand(10);
  /* flags&=~FLAGS_LCD_CHARMODE; */
  /* SendCommand(LCD_RETURNHOME); */

  return LCD_SUCCESS;

}
