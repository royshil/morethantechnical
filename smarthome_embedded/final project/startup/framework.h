/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 * Description:
 *    Framework for ARM7 processor
 *
 *****************************************************************************/


#ifndef _framework_h_
#define _framework_h_

#include "config.h"

//
//check that PLL values are within valid ranges
//
#if (USE_PLL == 1)

#if PLL_MUL > 32
#error PLL_MUL must be in the range 1-32
#elif PLL_MUL < 1
#error PLL_MUL must be in the range 1-32
#endif

#if PLL_DIV == 1
#elif PLL_DIV == 2
#elif PLL_DIV == 4
#elif PLL_DIV == 8
#else
#error PLL_DIV must be 1, 2, 4, or 8
#endif

#define CCLK (FOSC * PLL_MUL)          //CPU core clock frequency

//check if CCLK is within valid range
#if ((CCLK < 10000000) || (CCLK > 60000000))
#error CCLK is out of range (valid range is: 10MHz-60MHz)
#endif

#define FCCO (FOSC * PLL_MUL * 2 * PLL_DIV)      //CC Osc. Freq.

//check if FCCO is within valid range
#if ((FCCO < 156000000) || (FCCO > 320000000))
#error FCCO is out of range (valid range is: 156MHz-320MHz)
#endif

#else
#define CCLK (FOSC)
#endif

#if PBSD == 1
#elif PBSD == 2
#elif PBSD == 4
#else
#error PBSD must be 1, 2, or 4
#endif
#define PCLK (CCLK / PBSD)           //Peripheral bus clock frequency

/******************************************************************************
 * Defines
 *****************************************************************************/

/* Addresses of the instruction that is executed after an exception
   (when vectors are in RAM) */
#define pInst_RESET    (*(unsigned int *)(SRAM_SADDR + 0x00))
#define pInst_UNDEF    (*(unsigned int *)(SRAM_SADDR + 0x04))
#define pInst_SWI      (*(unsigned int *)(SRAM_SADDR + 0x08))
#define pInst_PABORT   (*(unsigned int *)(SRAM_SADDR + 0x0c))
#define pInst_DABORT   (*(unsigned int *)(SRAM_SADDR + 0x10))
#define pInst_IRQ      (*(unsigned int *)(SRAM_SADDR + 0x18))
#define pInst_FIQ      (*(unsigned int *)(SRAM_SADDR + 0x1c))

/* Pointers to the jump addresses that are used after an exception
   (valid both for vectors in FLASH and RAM) */
#define pISR_RESET     (*(unsigned int *)(SRAM_SADDR + 0x20))
#define pISR_UNDEF     (*(unsigned int *)(SRAM_SADDR + 0x24))
#define pISR_SWI       (*(unsigned int *)(SRAM_SADDR + 0x28))
#define pISR_PABORT    (*(unsigned int *)(SRAM_SADDR + 0x2c))
#define pISR_DABORT    (*(unsigned int *)(SRAM_SADDR + 0x30))
#define pISR_IRQ       (*(unsigned int *)(SRAM_SADDR + 0x34))
#define pISR_FIQ       (*(unsigned int *)(SRAM_SADDR + 0x38))

/******************************************************************************
 * External variables
 *****************************************************************************/
//Pointers that define the available heap
extern unsigned char *pHeapStart;
extern unsigned char *pHeapEnd;

/******************************************************************************
 * Public functions
 *****************************************************************************/
void lowLevelInit(void);
void eaInit(void);

#endif
