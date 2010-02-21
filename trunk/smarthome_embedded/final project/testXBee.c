/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2007 Embedded Artists AB
 *
 *****************************************************************************/


#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <lpc2xxx.h>
#include <consol.h>
#include "uart.h"

extern volatile tU32 msClock;

#include "testXBee.h"

/*****************************************************************************
 *
 * Description:
 *    
 *
 ****************************************************************************/
tU8 xbee_initXBee(tU8 MY)
{
 
 

  printf("\n\n*************************************************************\n");
  printf("*  Communication with XBee module below:                    *\n\n");

  //initialize uart #1: 9600 bps, 8N1, no FIFO
  initUart1(B9600((FOSC * PLL_MUL) / PBSD), UART_8N1, UART_FIFO_4);

  IODIR0 |= 0x00008000;
  IOSET0  = 0x00008000;
  IOCLR0  = 0x00008000;
  osSleep(1);
  IOSET0  = 0x00008000;

  IODIR0 |= 0x00000400;  //P0.10-RTS output
  IOCLR0  = 0x00000400;
  IODIR0 |= 0x00002000;  //P0.13-DTR output
  IOSET0  = 0x00002000;
  
  osSleep(20);

  xbee_enterCommandMode();
  
  printf("ATVL\n");
  uart1SendString("ATVL\n");
  tU32 rxChars = readBackTimed;
  printf("pc=ATVL retuned %d chars\n",rxChars);
  
  if(rxChars < 75) {
	  printf("pc=e=Error initializing XBee: NO XBEE module!!\n"); 
	return 0;
  }

  printf("ATID 1A\n");
  uart1SendString("ATID 1A\n");
  osSleep(110);
  rxChars = readBackTimed();

  if(rxChars != 3) {
	  //did not recieve "OK/r"
	  printf("pc=e=Error intializing xbee\n");
	  return 0;
  }

//set chan
  printf("ATCH 1A\n");
  uart1SendString("ATCH 1A\n");
  osSleep(110);
  rxChars = readBackTimed();

  if(rxChars != 3) {
	  //did not recieve "OK/r"
	  printf("pc=e=Error intializing xbee: cannot set channel\n");
	  return 0;
  }

//set self addr
  printf("ATMY %c",MY);
  uart1SendChars("ATMY ",5);
  uart1SendCh(MY);
  uart1SendCh('\n');
  osSleep(110);
  rxChars = readBackTimed();

  if(rxChars != 3) {
	  //did not recieve "OK/r"
	  printf("pc=e=Error intializing xbee: cannot set own ID\n");
	  return 0;
  }
  
  xbee_exitCommandMode();

  printf("\n\n*************************************************************\n");
  
  return 1;
}

tU8 xbee_enterCommandMode() {
printf("+++\n");
  uart1SendCh('+');
  uart1SendCh('+');
  uart1SendCh('+');
  osSleep(110);
}

tU8 xbee_setXBeeRemoteAddr(tU8 DL) {
  printf("\n\n*************************************************************\n");
  printf("*		set remote addr 										*\n");
  
//set remote addr
printf("ATDL %c",DL);
  uart1SendChars("ATDL ",5);
  uart1SendCh(DL);
  uart1SendCh('\n');
  osSleep(110);
  tU32 rxChars = readBackTimed();
  
  if(rxChars != 3) {
	  //did not recieve "OK/r"
	  printf("pc=e=Error: xbee cannot set remote address\n");
	  return 0;
  }


  printf("\n\n*************************************************************\n");

  return 1;
}

tU8 xbee_exitCommandMode() {

  //out of command mode
  printf("ATCN\n");
  uart1SendString("ATCN\n");
  osSleep(110);

  tU32 rxChars = readBackTimed();
  
  if(rxChars != 3) {
	  //did not recieve "OK/r"
	  printf("pc=e=Error: xbee cannot exit command mode\n");
	  return 0;
  }

}

tU32 readBackTimed() {
  //get current time
   tU32 timeStamp = msClock;
 tU32 rxChars = 0;

  while((msClock - timeStamp) < 5000)
  {
    tU8 rxChar = 0;
    
    //check if any character has been received
    if (TRUE == uart1GetChar(&rxChar))
    {
      rxChars++;
      if (rxChar == '\r')
        printf("\n%c", rxChar);
      else
        printf("%c", rxChar);
    }
  }
  return rxChars;
}

tU32 readBackIfAvail() {
 tU32 rxChars = 0;

    tU8 rxChar = 0;
    
    //check if any character has been received
    while (TRUE == uart1GetChar(&rxChar))
    {
      rxChars++;
      if (rxChar == '\r')
        printf("\n%c", rxChar);
      else
        printf("%c", rxChar);
    }

	return rxChars;
}

/*  
  if (rxChars > 75)
  {
    printf("\n*******************************************************");
    printf("\n*  XBee module detected!!!                            *");
    printf("\n*******************************************************\n\n\n");
    return 1; //= XBee module present
  }
  else
  {
    printf("\n*******************************************************");
    printf("\n*  No XBee module detected...                         *");
    printf("\n*******************************************************\n\n\n");
    return 0; //= no XBee module present
  }
}
*/
