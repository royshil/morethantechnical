/******************************************************************************
 *
 * Copyright:
 *    (C) 2005 Embedded Artists AB
 *
 * File:
 *    irqUart.h
 *
 * Description:
 *    Contains interface definitions for interrupt routines
 *
 *****************************************************************************/
#ifndef _IRQUART_H_
#define _IRQUART_H_

/*****************************************************************************
 * External variables
 ****************************************************************************/
extern tU8 uart1TxBuf[];
extern volatile tU32 uart1TxHead;
extern volatile tU32 uart1TxTail;
extern volatile tU8  uart1TxRunning;

extern tU8 uart1RxBuf[];
extern volatile tU32 uart1RxHead;
extern volatile tU32 uart1RxTail;

/*****************************************************************************
 * Public function prototypes
 ****************************************************************************/
void uart1ISR(void);

tU32 disIrq(void);
void restoreIrq(tU32 restoreValue);


#endif
