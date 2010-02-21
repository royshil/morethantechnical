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
	tU8 i;
	tU8 online[3] = {1,1,1};
	
    printf("\n\n\n\n\n*******************************************************\n");
    printf("*                                                     *\n");
    printf("* This is the master of smarthome					  *\n");
    printf("*                                                     *\n");
    printf("*******************************************************\n");

	if(xbee_initXBee('1') == 0) return;   

	printf("pc=master_online\n");

	printf(" *** FIND STATIONS *** \n");
	//TODO: send INITs to all stations to check if alive
	for(i=0;i<3;i++)
	{
		xbee_enterCommandMode();
		xbee_setXBeeRemoteAddr('1'+i);
		xbee_exitCommandMode();

		SM_OP smop;
		SM_createINIT_OP(&smop,'1'+i);

		SM_sendOpOnUART(&smop);
		printf("sent INIT op\n");

		SM_parseOpFromUART(&smop);
		if(SM_isGoodOp(&smop)) {
		   SM_printOp(&smop);
		}

		//debug
		printf("pc=up=%d\n",i);

		osSleep(300);
	}
	
	printf(" *** POLL ONLINE STATIONS *** \n");
	
	//scan all stations
	for(;;) {
		for(i=0;i<3;i++)
		{
			if(online[i] == 0) continue;
		
			xbee_enterCommandMode();
			xbee_setXBeeRemoteAddr('1'+i);
			xbee_exitCommandMode();

			SM_OP smop;
			SM_newOP(&smop);
			smop.op = SM_POLL;
			smop.from = '1';
			smop.to = '2';
			
			SM_sendOpOnUART(&smop);
			printf("sent op\n");

			SM_parseOpFromUART(&smop);
			if(SM_isGoodOp(&smop)) {
			   SM_printOp(&smop);
			}

			//debug
			printf("pc=temp=%d 25\n",i);

			osSleep(300);

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
