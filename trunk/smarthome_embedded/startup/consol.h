/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 * Description:
 *    Framework specific implementation of consol
 *
 *****************************************************************************/


#ifndef _consol_h_
#define _consol_h_

/******************************************************************************
 * Includes
 *****************************************************************************/

/******************************************************************************
 * Defines, macros, and typedefs
 *****************************************************************************/
#if CONSOL_UART == 0
#define UART_RBR UART0_RBR     //RO - Receiver Buffer
#define UART_THR UART0_THR     //WO - Transmit Holding
#define UART_IER UART0_IER     //RW - Interrupt Enable
#define UART_IIR UART0_IIR     //RO - Interrupt ID
#define UART_FCR UART0_FCR     //WO - FIFO Control
#define UART_LCR UART0_LCR     //RW - Line Control
#define UART_LSR UART0_LSR     //RO - Line Status
#define UART_SCR UART0_SCR     //RW - Scratch Pad
#define UART_DLL UART0_DLL     //RW - Divisor Latch LSB (DLAB = 1)
#define UART_DLM UART0_DLM     //RW - Divisor Latch MSB (DLAB = 1)
#else
#define UART_RBR UART1_RBR     //RO - Receiver Buffer
#define UART_THR UART1_THR     //WO - Transmit Holding
#define UART_IER UART1_IER     //RW - Interrupt Enable
#define UART_IIR UART1_IIR     //RO - Interrupt ID
#define UART_FCR UART1_FCR     //WO - FIFO Control
#define UART_LCR UART1_LCR     //RW - Line Control
#define UART_LSR UART1_LSR     //RO - Line Status
#define UART_SCR UART1_SCR     //RW - Scratch Pad
#define UART_DLL UART1_DLL     //RW - Divisor Latch LSB (DLAB = 1)
#define UART_DLM UART1_DLM     //RW - Divisor Latch MSB (DLAB = 1)
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/


/*****************************************************************************
 *
 * Description:
 *    Initializes the consol channel (init UART, including setting bitrate and 
 *    GPIO). 
 *
 ****************************************************************************/
void consolInit(void);


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
void consolSendChar(char charToSend);


/*****************************************************************************
 *
 * Description:
 *    Consol output routine that adds extra line feeds at line breaks. 
 *
 * Params:
 *    [in] charToSend - The character to print (to the consol) 
 *
 ****************************************************************************/
void consolSendCh(char charToSend);


/*****************************************************************************
 *
 * Description:
 *    Print string to consol. 
 *
 * Params:
 *    [in] pString - Pointer to string to be printed 
 *
 ****************************************************************************/
void consolSendString(char* pString);


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
void consolSendNumber(unsigned char base,
                      unsigned char noDigits,
                      unsigned char sign,
                      char          pad,
                      int           number);


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
void simplePrintf(const char * fmt,
                  ...          );
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
char consolGetCh(void);


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
char consolGetChar(char *pChar);


/*****************************************************************************
 *
 * Description:
 *    Receive (blocking call) a string (end with '\r'). 
 *
 * Params:
 *    [inout] pString - pointer to return the received string
 *
 ****************************************************************************/
void consolGetString(char *pString);


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
int consolGetIntNum(void);


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
void consolScanf(char *pFmt, ...);


#endif

#endif
