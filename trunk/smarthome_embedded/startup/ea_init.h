/******************************************************************************
*
* Copyright:
*    (C) 2000-2005 Embedded Artists AB
*
* Description:
*     Initialization API for the LPC2106 Evaluation Board.
*
* Version:
*     1.0.0
*
* NOTE:
*
*
******************************************************************************/

#ifndef _EA_INIT_H_
#define _EA_INIT_H_

#define ENABLE_INTERRUPTS   asm volatile (                      \
                                          "mrs r3,cpsr;"        \
                                          "bic r3,r3,#0xC0;"    \
                                          "msr cpsr,r3"         \
                                          :                     \
                                          :                     \
                                          : "r3"                \
                                         )

#define DISABLE_INTERRUPTS  asm volatile (                      \
                                          "mrs r0,cpsr;"        \
                                          "orr r1,r0,#0xC0;"    \
                                          "msr cpsr_c,r1;"      \
                                          :                     \
                                          :                     \
                                          : "r0", "r1"          \
                                         )

/******************************************************************************
*
* Description:
*     Initializes the evaluation board environment
* 
*******************************************************************************/
extern void eaInit (void);

#endif
