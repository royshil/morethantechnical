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
tU8
testXBeeComm(tU8 MY, tU8 DL)
{
 
 

  printf("\n\n*************************************************************\n");
  printf("*                                                           *\n");
  printf("* Communication with the XBee ZigBee module is demonstrated *\n");
  printf("*                                                           *\n");
  printf("*************************************************************\n");
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
  uart1SendCh('+');
  uart1SendCh('+');
  uart1SendCh('+');
  osSleep(110);
/*	uart1SendString("ATVL\n");
  uart1SendCh('A');
  uart1SendCh('T');
  uart1SendCh('V');
  uart1SendCh('L');
  uart1S tU32 rxChars = 0;endCh('\n');*/

  uart1SendString("ATID 1A\n");
  osSleep(110);
  readBackTimed();

//set chan
  uart1SendString("ATCH 1A\n");
  osSleep(110);
  readBackTimed();
//set self addr
  uart1SendChars("ATMY ",5);
  uart1SendCh(MY);
  uart1SendCh('\n');
  osSleep(110);
  readBackTimed();
//set remote addr
  uart1SendChars("ATDL ",5);
  uart1SendCh(DL);
  uart1SendCh('\n');
  osSleep(110);
  readBackTimed();
//out of command mode
  uart1SendString("ATCN\n");
  osSleep(110);
  readBackTimed();

  return 1;
}

void readBackTimed() {
  //get current time
   tU32 timeStamp = msClock;
 tU32 rxChars = 0;

  while((msClock - timeStamp) < 1000)
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
}

void readBackIfAvail() {
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
