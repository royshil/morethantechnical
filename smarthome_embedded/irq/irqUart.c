/******************************************************************************
 *
 * Copyright:
 *    (C) 2005 Embedded Artists AB
 *
 * File:
 *    irqUart.c
 *
 * Description:
 *    Sample irq code, that must be compiled in ARM code.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "../pre_emptive_os/api/general.h"
#include <lpc2xxx.h>
#include "irqUart.h"
#include "../uart.h"

/*****************************************************************************
 * Public function prototypes
 ****************************************************************************/
//void uart1ISR(void) __attribute__ ((interrupt));


/*****************************************************************************
 * Implementation of public functions
 ****************************************************************************/

/*****************************************************************************
 *
 * Description:
 *    Actual uart #1 ISR that is called whenever the uart generated an interrupt.
 *
 ****************************************************************************/
void
uart1ISR(void)
{
  volatile tU8  statusReg;
  volatile tU8  dummy;
  volatile tU32 tmpHead;
  volatile tU32 tmpTail;

  //loop until not more interrupt sources
  while (((statusReg = U1IIR) & 0x01) == 0)
  {
    //identify and process the highest priority interrupt
    switch (statusReg & 0x0E)
    {
      case 0x06:  //Receive Line Status
      dummy = U1LSR;  //read LSR to clear bits
      break;

      case 0x0c:  //Character Timeout Indicator
      case 0x04:  //Receive Data Available
      do
      {
        tmpHead     = (uart1RxHead + 1) & RX_BUFFER_MASK;
        uart1RxHead = tmpHead;

        if(tmpHead == uart1RxTail)
          tmpHead = U1RBR;              //dummy read to reset IRQ flag
        else
          uart1RxBuf[tmpHead] = U1RBR;  //will reset IRQ flag
      } while (U1LSR & 0x01);
      break;

      case 0x02:  //Transmit Holding Register Empty
      //check if all data is transmitted
      if (uart1TxHead != uart1TxTail)
      {
      	tU32 bytesToSend;
      	
      	if (statusReg & 0xc0)
      	  bytesToSend = 16;    //FIFO enabled
      	else
      	  bytesToSend = 1;     //no FIFO enabled

      	do
      	{
          //calculate buffer index
          tmpTail = (uart1TxTail + 1) & TX_BUFFER_MASK;

          uart1TxTail = tmpTail;
          U1THR = uart1TxBuf[tmpTail]; 
        } while((uart1TxHead != uart1TxTail) && --bytesToSend);
      }

      //all data has been transmitted
      else
      {
        uart1TxRunning = FALSE;
        U1IER &= ~0x02;        //disable TX IRQ
      }
      break;

      default:  //unknown
      dummy = U1LSR;
      dummy = U1RBR;
      break;
    }
  }

  VICVectAddr = 0x00000000;    //dummy write to VIC to signal end of interrupt
}

/*****************************************************************************
 *
 * Description:
 *    Disable interrupts 
 *
 * Returns:
 *    The current status register, before disabling interrupts. 
 *
 ****************************************************************************/
tU32
disIrq(void)
{
  tU32 returnReg;

  asm volatile ("disIrq1: mrs %0, cpsr  \n\t"
                "orr r1, %0, #0xC0      \n\t"
                "msr cpsr_c, r1         \n\t"
                "mrs r1, cpsr           \n\t"
                "and r1, r1, #0xC0      \n\t"
                "cmp r1, #0xC0          \n\t"
                "bne disIrq1            \n\t"
                : "=r"(returnReg)
                :
                : "r1"
               );
  return returnReg;
}

/*****************************************************************************
 *
 * Description:
 *    Restore interrupt state. 
 *
 * Params:
 *    [in] restoreValue - The value of the new status register. 
 *
 ****************************************************************************/
void
restoreIrq(tU32 restoreValue)
{
  asm volatile ("msr cpsr_c, %0  \n\t"
                :
                : "r" (restoreValue)
                : "r1"
               );
}
