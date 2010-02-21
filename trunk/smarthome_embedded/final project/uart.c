/******************************************************************************
 *
 * Copyright:
 *    (C) 2005 Embedded Artists AB
 *
 * File:
 *    uart.c
 *
 * Description:
 *    Implementation of interrupt driven UART.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "pre_emptive_os/api/general.h"
#include <lpc2xxx.h>
#include "uart.h"
#include "irq/irqUart.h"


/*****************************************************************************
 * Local variables
 ****************************************************************************/
tU8 uart1TxBuf[TX_BUFFER_SIZE];
volatile tU32 uart1TxHead = 0;
volatile tU32 uart1TxTail = 0;
volatile tU8  uart1TxRunning = FALSE;

tU8 uart1RxBuf[RX_BUFFER_SIZE];
volatile tU32 uart1RxHead = 0;
volatile tU32 uart1RxTail = 0;


/*****************************************************************************
 * Implementation of public functions
 ****************************************************************************/

/*****************************************************************************
 *
 * Description:
 *    Initialize UART #1 in polled mode, i.e., interrupts are not used.
 *
 * Parameters:
 *    [in] div_factor - UART clock division factor to get desired bit rate.
 *                      Use definitions in uart.h to calculate correct value.
 *    [in] mode       - transmission format settings. Use constants in uart.h
 *    [in] fifo_mode  - FIFO control settings. Use constants in uart.h
 *
 ****************************************************************************/
void
initUart1(tU16 div_factor, tU8 mode, tU8 fifo_mode)
{
  volatile tU32 dummy;
  
  //enable uart #1 pins in GPIO (P0.8 = TxD1, P0.9 = RxD1)
  PINSEL0 = (PINSEL0 & 0xfff0ffff) | 0x00050000;

  U1IER = 0x00;                        //disable all uart interrupts
  dummy = U1IIR;                       //clear all pending interrupts
  dummy = U1RBR;                       //clear receive register
  dummy = U1LSR;                       //clear line status register

  //set the bit rate = set uart clock (pclk) divisionfactor
  U1LCR = 0x80;                        //enable divisor latches (DLAB bit set, bit 7)
  U1DLL = (tU8)div_factor;             //write division factor LSB
  U1DLM = (tU8)(div_factor >> 8);      //write division factor MSB

  //set transmissiion and fifo mode
  U1LCR = (mode & ~0x80);              //DLAB bit (bit 7) must be reset
  U1FCR = fifo_mode;

  //initialize the interrupt vector
  VICIntSelect &= ~0x00000080;      // UART1 selected as IRQ
  VICVectCntl7  =  0x00000027;
  VICVectAddr7  =  (tU32)uart1ISR;  // address of the ISR
  VICIntEnable |=  0x00000080;      // UART1 interrupt enabled

  //initialize the transmit data queue
  uart1TxHead    = 0;
  uart1TxTail    = 0;
  uart1TxRunning = FALSE;

  //initialize the receive data queue
  uart1RxHead   = 0;
  uart1RxTail   = 0;

  //enable receiver interrupts
  U1IER = 0x01;
}

/*****************************************************************************
 *
 * Description:
 *    Blocking output routine, i.e., the routine waits until the uart 
 *    buffer is free and then sends the character. 
 *
 * Params:
 *    [in] charToSend - The character to print (to uart #1) 
 *
 ****************************************************************************/
void
uart1SendChar(tU8 charToSend)
{
  volatile tU32 cpsrReg;
  tU32 tmpHead;

  //calculate head index
  tmpHead = (uart1TxHead + 1) & TX_BUFFER_MASK; 

  //wait for free space in buffer
  while(tmpHead == uart1TxTail)
    ;

  //disable IRQ
  cpsrReg = disIrq();

  U1IER &= ~0x02;   //disable TX IRQ

  //enable IRQ
  restoreIrq(cpsrReg);

  if(uart1TxRunning == TRUE)
  {
    uart1TxBuf[tmpHead] = charToSend; 
    uart1TxHead         = tmpHead;
  }
  else
  {
    uart1TxRunning = TRUE;
    U1THR          = charToSend;
  }

  //disable IRQ
  cpsrReg = disIrq();

  U1IER |= 0x02;   //enable TX IRQ

  //enable IRQ
  restoreIrq(cpsrReg);
}


/*****************************************************************************
 *
 * Description:
 *    Output routine that adds extra line feeds at line breaks. 
 *
 * Params:
 *    [in] charToSend - The character to print (to uart #1) 
 *
 ****************************************************************************/
void
uart1SendCh(tU8 charToSend)
{
  if(charToSend == '\n')
    uart1SendChar('\r');

  uart1SendChar(charToSend);
}


/*****************************************************************************
 *
 * Description:
 *    Print NULL-terminated string to uart #1. 
 *
 * Params:
 *    [in] pString - Pointer to NULL-terminated string to be printed 
 *
 ****************************************************************************/
void
uart1SendString(tU8 *pString)
{
  while(*pString)
    uart1SendCh(*pString++);
}

/*****************************************************************************
 *
 * Description:
 *    Print a fixed number of bytes (as opposed to NULL-terminated string).
 *
 * Params:
 *    [in] pBuff - The character to print (to uart #1) 
 *    [in] count - Number of characters to print
 *
 ****************************************************************************/
void
uart1SendChars(char *pBuff, tU16 count)
{
  while (count--)
    uart1SendChar(*pBuff++);
}

/*****************************************************************************
 *
 * Description:
 *    Blocking function that waits for a received character. 
 *
 * Return:
 *    The received character. 
 *
 ****************************************************************************/
tU8
uart1GetCh(void)
{
	tU8 rxChar;

  //wait for a character to be available
  while(uart1GetChar(&rxChar) == FALSE)
    ;
  return rxChar;
}

/*****************************************************************************
 *
 * Description:
 *    Non-blocking receive function.
 *
 * Params:
 *    [in] pRxChar - Pointer to buffer where the received character shall
 *                   be placed.
 *
 * Return:
 *    TRUE if character was received, else FALSE.
 *
 ****************************************************************************/
tU8
uart1GetChar(tU8 *pRxChar)
{
  tU32 tmpTail;

  /* buffer is empty */
  if(uart1RxHead == uart1RxTail)
    return FALSE;

  tmpTail     = (uart1RxTail + 1) & RX_BUFFER_MASK;
  uart1RxTail = tmpTail; 

  *pRxChar = uart1RxBuf[tmpTail];
  return TRUE;
}
