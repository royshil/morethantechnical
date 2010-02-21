/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 * Description:
 *    Framework specific implementation of consol
 *
 *****************************************************************************/


/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc2xxx.h"                            /* LPC2xxx definitions */
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "consol.h"
#include "framework.h"

#if (CONSOLE_API_PRINTF == 1) || (CONSOLE_API_SCANF == 1) //own simple printf() or scanf()
#include <stdarg.h>
#endif

#define UART_DLL_VALUE (unsigned short)((PCLK / (CONSOL_BITRATE * 16.0)) + 0.5)

#if (CONSOLE_API_SCANF == 1)  //SIMPLE
#define __isalpha(c) (c >'9')
#define __isupper(c) !(c & 0x20)
#define __ishex(c) (((c >= '0')&&(c <= '9'))||((c >= 'A')&&(c <= 'F')))
#define __ascii2hex(c) ((c <= '9')? c-'0': c-'A'+10)
#endif

/******************************************************************************
 * Implementation of local functions
 *****************************************************************************/

#if (CONSOLE_API_PRINTF == 1)  //OWN_PRINTF
/*****************************************************************************
 *
 * Description:
 *    Print a number. 
 *
 * Params:
 *    [in] outputFnk - Output function where characters to be printed are 
 *                      sent 
 * 
 *                Params (callback):
 *                   ch - Character to be printed 
 *    [in] number    - Number to be printed 
 *    [in] base      - Base when printing number (2-16) 
 *
 ****************************************************************************/
static void
printNum(void (*outputFnk) (char ch),
         unsigned int number,
         unsigned int base)
{
	char *p;
	char  buf[33];
	char  hexChars[] = "0123456789abcdef";
	
	p = buf;
	
	do {
		*p++ = hexChars[number % base];
	} while (number /= base);
	
	do {
		outputFnk(*--p);
	} while (p > buf);
}

/*****************************************************************************
 *
 * Description:
 *    A simple implementation of printf that uses a minimum of stack space.
 *
 * Params:
 *    [in] outputFnk - Output function where characters to be printed are 
 *                      sent 
 * 
 *                Params (callback):
 *                   ch - Character to be printed 
 *    [in] fmt       - Format string that specifies what to be printed 
 *    [in] ap        - Structure that hold information about the variable 
 *                     number of parameters 
 *
 ****************************************************************************/
static void
simplePrint(void         (*outputFnk) (char ch),
            const char * fmt,
            va_list      ap)
{
  char          *p;
  char           ch;
  unsigned long  ul;
  unsigned char  lflag;

  for (;;)
  {
    while ((ch = *fmt++) != '%')
    {
      if (ch == '\0')
        	return;
      outputFnk(ch);
    }
    lflag = 0;

reswitch:

    switch (ch = *fmt++)
    {
    case '\0':
      return;

    case 'l':
      lflag = 1;
      goto reswitch;

    case 'c':
	    ch = va_arg(ap, unsigned int);
	    outputFnk(ch & 0x7f);
	    break;

    case 's':
      p = va_arg(ap, char *);
      while ((ch = *p++) != 0)
        outputFnk(ch);
      break;

    case 'd':
      ul = lflag ? va_arg(ap, long) : va_arg(ap, unsigned int);
      if ((long)ul < 0)
      {
        outputFnk('-');
        ul = -(long)ul;
      }
      printNum(outputFnk, ul, 10);
      break;

    case 'o':
      ul = va_arg(ap, unsigned int);
      printNum(outputFnk, ul, 8);
      break;

    case 'u':
      ul = va_arg(ap, unsigned int);
      printNum(outputFnk, ul, 10);
      break;

    case 'p':
      outputFnk('0');
      outputFnk('x');
      lflag = 1;
      // fall through

    case 'x':
      ul = va_arg(ap, unsigned int);
      printNum(outputFnk, ul, 16);
      break;

    default:
//      outputFnk('%');
      if (lflag)
        outputFnk('l');
      outputFnk(ch);
    }
  }
  va_end(ap);
}
#endif

/******************************************************************************
 * Implementation of public functions
 *****************************************************************************/


/*****************************************************************************
 *
 * Description:
 *    Initializes the consol channel (init UART, including setting bitrate and 
 *    GPIO). 
 *
 ****************************************************************************/
void
consolInit(void)
{
#if (CONSOL_UART == 0)
  //enable uart #0 pins in GPIO (P0.0 = TxD0, P0.1 = RxD0)
  PINSEL0 = (PINSEL0 & 0xfffffff0) | 0x00000005;
#else
  //enable uart #1 pins in GPIO (P0.8 = TxD1, P9.1 = RxD1)
  PINSEL0 = (PINSEL0 & 0xfff0ffff) | 0x00050000;
#endif

  //initialize bitrate (by first enable DL registers, DLAB-bit = 1)
  UART_LCR = 0x80;
  UART_DLL = (unsigned char)(UART_DLL_VALUE & 0x00ff);
  UART_DLM = (unsigned char)(UART_DLL_VALUE>>8);
  UART_LCR = 0x00;

  //initialize LCR: 8N1
  UART_LCR = 0x03;

  //reset FIFO
  UART_FCR = 0x00;

  //clear interrupt bits
  UART_IER = 0x00;
}

/*****************************************************************************
 *
 * Description:
 *    Blocking consol output routine, i.e., the routine waits until the uart 
 *    buffer is free and then sends the character. 
 *
 * Params:
 *    [in] charToSend - The character to print (to the consol) 
 *
 ****************************************************************************/
void
consolSendChar(char charToSend)
{
  //Wait until THR is empty
  while(!(UART_LSR & 0x20))
    ;
  UART_THR = charToSend;
}

/*****************************************************************************
 *
 * Description:
 *    Consol output routine that adds extra line feeds at line breaks. 
 *
 * Params:
 *    [in] charToSend - The character to print (to the consol) 
 *
 ****************************************************************************/
void
consolSendCh(char charToSend)
{
  if(charToSend == '\n')
    consolSendChar('\r');

  consolSendChar(charToSend);
}

/*****************************************************************************
 *
 * Description:
 *    Print string to consol. 
 *
 * Params:
 *    [in] pString - Pointer to string to be printed 
 *
 ****************************************************************************/
void
consolSendString(char *pString)
{
  while(*pString)
    consolSendCh(*pString++);
}

/*****************************************************************************
 *
 * Description:
 *    Routine for printing integer numbers in various formats. The number is 
 *    printed in the specified 'base' using exactly 'noDigits', using +/- if 
 *    signed flag 'sign' is TRUE, and using the character specified in 'pad' 
 *    to pad extra characters. 
 *
 * Params:
 *    [in] base     - Base to print number in (2-16) 
 *    [in] noDigits - Number of digits to print (max 32) 
 *    [in] sign     - Flag if sign is to be used (TRUE), or not (FALSE) 
 *    [in] pad      - Character to pad any unused positions 
 *    [in] number   - Signed number to print 
 *
 ****************************************************************************/
void
consolSendNumber(unsigned char base,
                 unsigned char noDigits,
                 unsigned char sign,
                 char          pad,
                 int           number)
{
  static char    hexChars[16] = "0123456789ABCDEF";
  char          *pBuf;
  char           buf[32];
  unsigned int   numberAbs;
  unsigned char  count;

  // prepare negative number
  if(sign && (number < 0))
    numberAbs = -number;
  else
    numberAbs = number;

  // setup little string buffer
  count = (noDigits - 1) - (sign ? 1 : 0);
  pBuf = buf + sizeof(buf);
  *--pBuf = '\0';

  // force calculation of first digit
  // (to prevent zero from not printing at all!!!)
  *--pBuf = hexChars[(numberAbs % base)];
  numberAbs /= base;

  // calculate remaining digits
  while(count--)
  {
    if(numberAbs != 0)
    {
      //calculate next digit
      *--pBuf = hexChars[(numberAbs % base)];
      numberAbs /= base;
    }
    else
      // no more digits left, pad out to desired length
      *--pBuf = pad;
  }

  // apply signed notation if requested
  if(sign)
  {
    if(number < 0)
      *--pBuf = '-';
    else if(number > 0)
       *--pBuf = '+';
    else
       *--pBuf = ' ';
  }

  // print the string right-justified
  consolSendString(pBuf);
}

#if (CONSOLE_API_PRINTF == 1)  //OWN_PRINTF
/*****************************************************************************
 *
 * Description:
 *    Simple implementation of printf 
 *
 * Params:
 *    [in] fmt - Format string that specifies what to be printed 
 *    [in] ... - Variable number of parameters to match format string 
 *
 ****************************************************************************/
void
simplePrintf(const char * fmt,
             ...          )
{
	va_list ap;

	va_start(ap, fmt);
	simplePrint(consolSendCh, fmt, ap);
	va_end(ap);
}
#endif


#if (CONSOLE_API_SCANF == 1)  //SIMPLE

/*****************************************************************************
 *
 * Description:
 *    Wait (blocking call) for a character to be received. 
 *
 * Returns:
 *    char - Received character
 *
 ****************************************************************************/
char
consolGetCh(void)
{
  while(!(UART_LSR & (0x01<<0)))
    ;
  return UART_RBR;
}

/*****************************************************************************
 *
 * Description:
 *    Check (non-blocking call) if a character has been received. 
 *
 * Params:
 *    [inout] pChar - pointer to where to place received character
 *
 * Returns:
 *    char - TRUE if character has been received (placed at pointed location)
 *           FALSE if no character has been received.
 *
 ****************************************************************************/
char
consolGetChar(char *pChar)
{
  if((UART_LSR & 0x01) != 0x00)
  {
    *pChar = UART_RBR;
    return 1;
  }
  return 0;
}

/*****************************************************************************
 *
 * Description:
 *    Receive (blocking call) a string (end with '\r'). 
 *
 * Params:
 *    [inout] pString - pointer to return the received string
 *
 ****************************************************************************/
void
consolGetString(char *pString)
{
  char *pString2;
  char rxChar;

  pString2 = pString;
  while((rxChar = consolGetCh()) != '\r')
  {
    if (rxChar == '\b')
    {
      if ((int)pString2 < (int)pString)
      {
        consolSendString("\b \b");
        pString--;
      }
    }
    else
    {
      *pString++ = rxChar;
      consolSendCh(rxChar);
    }
  }
  *pString = '\0';
  consolSendCh('\n');
}

/*****************************************************************************
 *
 * Description:
 *    Receive (blocking call) a number (end with '\r').
 *    Accepts hexadecimal (radix 16) and integer (radix 10) numbers.
 *
 * Returns:
 *    int - the received number
 *
 ****************************************************************************/
int
consolGetIntNum(void)
{
  char  abStr[30];
  char *pString = abStr;
  int   wBase=10;
  int   wMinus=0;
  int   wLastIndex;
  int   wResult=0;
  int   wI;
        
  consolGetString(pString);
        
  if(pString[0] == '-')
  {
    wMinus = 1;
    pString++;
  }

  if(pString[0] == '0' && (pString[1] == 'x' || pString[1] == 'X'))
  {
    wBase = 16;
    pString += 2;
  }

  wLastIndex = strlen(pString) - 1;
  if(pString[wLastIndex] == 'h' || pString[wLastIndex] == 'H')
  {
    wBase = 16;
    pString[wLastIndex] = 0;
    wLastIndex--;
  }

  if(wBase == 10)
  {
    wResult = atoi(pString);
    wResult = wMinus ? (-1*wResult):wResult;
  }
  else
  {
    for(wI=0; wI<=wLastIndex; wI++)
    {
      if(__isalpha(pString[wI]))
      {
        if(__isupper(pString[wI]))
          wResult = (wResult<<4) + pString[wI] - 'A' + 10;
        else
          wResult = (wResult<<4) + pString[wI] - 'a' + 10;
      }
      else
      {
        wResult = (wResult<<4) + pString[wI] - '0';
      }
    }
    wResult = wMinus ? (-1*wResult):wResult;
  }
  return wResult;
}

/*****************************************************************************
 *
 * Description:
 *    A simple implementation of the scanf()-function. 
 *
 * Params:
 *    [inout] pFmt - format string, accepts:
 *                   %s - string
 *                   %c - character
 *                   %i - integer
 *
 ****************************************************************************/
void
consolScanf(char *pFmt, ...)
{
  va_list pArg;
  char    character;
  int    *pInt;
  char   *pChar;
        
  va_start(pArg, pFmt);
  while((character = *pFmt++) != '\0')
  {
    if(character != '%')continue;
    switch(*pFmt)
    {
    case 's':
    case 'S':
      pChar = va_arg (pArg, char *);
      consolGetString(pChar);
      break;
    case 'i':
    case 'I':
      pInt  = va_arg (pArg, int *);
      *pInt = consolGetIntNum();
      break;
    case 'c':
    case 'C':
      pChar  = va_arg (pArg, char *);
      *pChar = consolGetCh();
      break;
    }
  }
  va_end(pArg);
}

#endif

