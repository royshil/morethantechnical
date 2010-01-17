/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2007 Embedded Artists AB
 *
 * Description:
 *    Main program for LPC2148 Education Board test program
 *
 *****************************************************************************/

#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>
#include <lpc2xxx.h>
#include <consol.h>

#include "testXBee.h"
#include "uart.h"
#include "smarthome_protocol.c"

#define PROC1_STACK_SIZE 2048
#define INIT_STACK_SIZE  400

static tU8 proc1Stack[PROC1_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
static tU8 pid1;

static void proc1(void* arg);
static void initProc(void* arg);

//tU8  testXBeeComm(void);

tU8 xbeePresent;
volatile tU32 msClock;

/*****************************************************************************
 *
 * Description:
 *    The first function to execute 
 *
 ****************************************************************************/
int
main(void)
{
  tU8 error;
  tU8 pid;

  //immediately turn off buzzer (if connected)
  IODIR0 |= 0x00000080;
  IOSET0  = 0x00000080;
  
  osInit();
  osCreateProcess(initProc, initStack, INIT_STACK_SIZE, &pid, 1, NULL, &error);
  osStartProcess(pid, &error);
  
  osStart();
  return 0;
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
proc1(void* arg)
{
  tU8 xbeePresent = FALSE;

	for(;;)
	{
    printf("\n\n\n\n\n*******************************************************\n");
    printf("*                                                     *\n");
    printf("* This program demonstrate communication with         *\n");
    printf("* the XBee (PRO) module on Embedded Artists'          *\n");
    printf("* LPC2148 Education Board v2.1...                     *\n");
    printf("*                                                     *\n");
    printf("* (C) Embedded Artists 2005-2007                      *\n");
    printf("*                                                     *\n");
    printf("*******************************************************\n");

    //
    //Test XBee module (if present)
    //
    xbeePresent = testXBeeComm('2','1');

    SM_OP smop;
 //   smop.op = SM_INIT;
 //   smop.from = '1';
 //   smop.to = '2';
 //   

    for(;;)
    {
//	uart1SendString("2\r");
//	printf("sent '2'\n");
	
//	SM_sendOpOnUART(&smop);
//	printf("sent op\n");

	SM_parseOpFromUART(&smop);
        if(SM_isGoodOp(&smop)) {
	  SM_printOp(&smop);
	}

        osSleep(300);

//	readBackIfAvail();
    }
  }
}


/*****************************************************************************
 *
 * Description:
 *    The entry function for the initialization process. 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
initProc(void* arg)
{
  tU8 error;

  eaInit();
  osCreateProcess(proc1, proc1Stack, PROC1_STACK_SIZE, &pid1, 3, NULL, &error);
  osStartProcess(pid1, &error);

  osDeleteProcess();
}

/*****************************************************************************
 *
 * Description:
 *    The timer tick entry function that is called once every timer tick
 *    interrupt in the RTOS. Observe that any processing in this
 *    function must be kept as short as possible since this function
 *    execute in interrupt context.
 *
 * Params:
 *    [in] elapsedTime - The number of elapsed milliseconds since last call.
 *
 ****************************************************************************/
void
appTick(tU32 elapsedTime)
{
  msClock += elapsedTime;
}
