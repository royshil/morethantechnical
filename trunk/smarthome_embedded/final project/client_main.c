/*

Client program

1. listen for new op on UART
2. if op is good, send ACK to master (sender of op), else send ERROR
3. act upon op.

*/

#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>
#include <lpc2xxx.h>
#include <consol.h>

#include "testXBee.h"
#include "uart.h"

#include "i2c.h"

#include "smarthome_protocol.c"

// #define LCD_USE_4BIT
#include "char-lcd.c"

#define PROC1_STACK_SIZE 2048
#define INIT_STACK_SIZE  400

#define LOCAL_EEPROM_ADDR 0x0         //if EEPROM has local address pins
#define EEPROM_ADDR       0xA0
#define I2C_EEPROM_ADDR   (EEPROM_ADDR + (LOCAL_EEPROM_ADDR << 1))
#define TMP_SENSOR_ADDRESS 0x91  


static tU8 proc1Stack[PROC1_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
static tU8 pid1;

static void proc1(void* arg);
static void initProc(void* arg);

//tU8  testXBeeComm(void);

tU8 xbeePresent;
volatile tU32 msClock;

static char __str[32];

static void temp_sensor(tU8*);

/*****************************************************************************
 * 				main
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
 *			process main function
 ****************************************************************************/
static void
proc1(void* arg)
{
    printf("\n\n\n\n\n*******************************************************\n");
    printf("*                                                     *\n");
    printf("* This is the client of smarthome					  *\n");
    printf("*                                                     *\n");
    printf("*******************************************************\n");

	tU8 xbeePresent = FALSE;

    //Test XBee module (if present)
    //xbeePresent = testXBeeComm('2','1');

	lcdInit();
	
    SM_OP smop;

    for(;;)
    {
		SM_parseOpFromUART(&smop);
		
		if(SM_isGoodOp(&smop)) {
			SM_printOp(&smop); 
			
			if(smop.op == SM_POLL) {
				printf("client in POLL op\n");
				strcpy(__str, "client in POLL op");
				lcdPrintString(__str);
	
				//return all data on  ACK
				SM_crateACK_OP(&smop,'2');
				
				temp_sensor(smop.data);	//write 2 bytes of temperature
				
				
			} else if(smop.op == SM_OPERATE) {
				//act upon op, and sent ACK
			} else if(smop.op == SM_INIT) {
				//do init, and send ACK
			} else if(smop.op == SM_FIND) {
				//do find, send ACK
			} else {
				//error on op type
				printf("error in op type %c\n",smop.op);
			}
			
			SM_sendOpOnUART(&smop);
		}
		
		printf(".");
		
		// lcdPrintChar(_c++);
		strcpy(__str, "client recieve");
		lcdPrintString(__str);
		
		osSleep(100);
    }
}

 static void
temp_sensor(tU8* out)
{
	i2cInit(100000);
	tU16 length;
	tU8  buffer[20];
	tS8  retCode;
	
	retCode = i2cStart();
	if (retCode == I2C_CODE_OK)
		retCode = i2cRead(TMP_SENSOR_ADDRESS, buffer, 2);
	printf("\n  Temperatue : %d", buffer[0]);
	if(buffer[1]>128)
		printf(".5");
		
	out[0] = buffer[0];//copy the temperature to the out buffer
	out[1] = buffer[1];
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
