/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 * Description:
 *    Framework for ARM7 processor
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "lpc2xxx.h"                            /* LPC2xxx definitions */
#include "config.h"
#include "framework.h"
#include "consol.h"

/******************************************************************************
 * Defines, macros, and typedefs
 *****************************************************************************/


/******************************************************************************
 * External variables
 *****************************************************************************/
extern char end asm ("end");  //the symbol "end" is defined by the linker script


/******************************************************************************
 * Public variables
 *****************************************************************************/
unsigned char *pHeapStart;
unsigned char *pHeapEnd;


/******************************************************************************
 * External functions
 *****************************************************************************/


/******************************************************************************
 * Public functions
 *****************************************************************************/
void lowLevelInit(void);
void eaInit(void);
void exceptionHandlerInit(void);

/******************************************************************************
 * Implementation of local functions
 *****************************************************************************/

/*****************************************************************************
 *
 * Description:
 *    Default exception handler for undefined instruction.
 *    Prints information and halt. Observe that code execture in exception
 *    mode (as opposed to supervisor mode, which is the normal execution mode).
 *
 ****************************************************************************/
static void
exceptionHandlerUndef(void)
{
  register unsigned int programCounter asm ("lr");
  unsigned int          value = programCounter - (unsigned int)0x04;

  consolSendString("Undefined instruction exception !!!\nAddress: 0x");
  consolSendNumber(16, 8, 0, '0', value); 
  while(1)
    ;
}

/*****************************************************************************
 *
 * Description:
 *    Default exception handler for software interrupt.
 *    Prints information and halt. Observe that code execture in exception
 *    mode (as opposed to supervisor mode, which is the normal execution mode).
 *
 ****************************************************************************/
static void
exceptionHandlerSwi(void)
{
  register unsigned int programCounter asm ("lr");
  unsigned int          value = programCounter - (unsigned int)0x04;

  consolSendString("SWI exception !!!\nAddress: 0x");
  consolSendNumber(16, 8, 0, '0', value); 
  while(1)
    ;
}

/*****************************************************************************
 *
 * Description:
 *    Default exception handler for invalid program fetch.
 *    Prints information and halt. Observe that code execture in exception
 *    mode (as opposed to supervisor mode, which is the normal execution mode).
 *
 ****************************************************************************/
static void
exceptionHandlerPabort()
{
  register unsigned int programCounter asm ("lr");
  unsigned int          value = programCounter - (unsigned int)0x04;

  consolSendString("Pabort exception !!!\nAddress: 0x");
  consolSendNumber(16, 8, 0, '0', value); 
  while(1)
    ;
}

/*****************************************************************************
 *
 * Description:
 *    Default exception handler for invalid data access.
 *    Prints information and halt. Observe that code execture in exception
 *    mode (as opposed to supervisor mode, which is the normal execution mode).
 *
 ****************************************************************************/
static void
exceptionHandlerDabort(void)
{
  register unsigned int programCounter asm ("lr");
  unsigned int          value = programCounter - (unsigned int)0x08;

  consolSendString("Dabort exception !!!\nAddress: 0x");
  consolSendNumber(16, 8, 0, '0', value); 
  while(1)
    ;
}

/*****************************************************************************
 *
 * Description:
 *    Default exception handler for fast interrupts.
 *    Prints information and halt. Observe that code execture in exception
 *    mode (as opposed to supervisor mode, which is the normal execution mode).
 *
 ****************************************************************************/
static void
exceptionHandlerFiq(void)
{
  register unsigned int programCounter asm ("lr");
  unsigned int          value = programCounter - (unsigned int)0x04;

  consolSendString("FIQ exception !!!\nAddress: 0x");
  consolSendNumber(16, 8, 0, '0', value); 
  while(1)
    ;
}

#if (IRQ_HANDLER == 0)
/*****************************************************************************
 *
 * Description:
 *    Default exception handler for normal interrupts. This handler should
 *    normally never be used.
 *    Prints information and halt. Observe that code execture in exception
 *    mode (as opposed to supervisor mode, which is the normal execution mode).
 *
 ****************************************************************************/
static void
exceptionHandlerIrq(void)
{
  register unsigned int programCounter asm ("lr");
  unsigned int          value = programCounter - (unsigned int)0x04;

  consolSendString("IRQ exception !!!\nAddress: 0x");
  consolSendNumber(16, 8, 0, '0', value); 
  while(1)
    ;
}
#endif

/******************************************************************************
 * Implementation of public functions
 *****************************************************************************/

/*****************************************************************************
 *
 * Description:
 *    Connect all default exception handlers.
 *    The application can overwrite these handlers with new ones.
 *
 ****************************************************************************/
void
exceptionHandlerInit(void)
{
  pISR_UNDEF  = (unsigned int)exceptionHandlerUndef;
  pISR_SWI    = (unsigned int)exceptionHandlerSwi;
  pISR_PABORT = (unsigned int)exceptionHandlerPabort;
  pISR_DABORT = (unsigned int)exceptionHandlerDabort;
  pISR_FIQ    = (unsigned int)exceptionHandlerFiq;

#if (IRQ_HANDLER == 0)
  pISR_IRQ    = (unsigned int)exceptionHandlerIrq;
#endif

  //set heap limits
  pHeapStart = &end;
  pHeapEnd   = (unsigned char*)((STK_SADDR - 1) & 0xfffffffc);
}

/*****************************************************************************
 *
 * Description:
 *    Initialize system functions and GPIO
 *
 ****************************************************************************/
void
lowLevelInit(void)
{
  PINSEL0 = 0x00000000;  
  PINSEL1 = 0x00000000;  

  IOSET = 0x00000000;       //Initialize pins to high level
  IOCLR = 0xffffffff;       //Initialize pins to low level
  IODIR = 0x00000000;       //Set pin direction

  //initialize the MAM (Memory Accelerator Module)
  MAMTIM = MAM_TIMING;       //number of CCLK to read from the FLASH
  MAMCR  = MAM_SETTING;      //0=disabled, 1=partly enabled (enabled for code prefetch, but not for data), 2=fully enabled

  //initialize the exception vector mapping
  MAMMAP = MAM_MAP;

  //set the peripheral bus speed, PCLK = CCLK / PBSD
	VPBDIV = PBSD;
	
	//initialize VIC
  VICIntEnClr    = 0xFFFFFFFF;           /* Disable ALL interrupts                             */
  VICProtection  = 0;                    /* Setup interrupt controller                         */
  VICDefVectAddr = (unsigned int)0;      /* Direct unvectored IRQs to reset, i.e., address 0x0 */

  VICVectAddr0   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr1   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr2   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr3   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr4   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr5   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr6   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr7   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr8   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr9   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr10  = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr11  = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr12  = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr13  = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr14  = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr15  = (unsigned int)0;      /* Set the vector address                             */
	
  //enable interrupts (both IRQ and FIQ) 
  asm volatile ("mrs r3, cpsr       \n\t"                          
                "bic r3, r3, #0xC0  \n\t"                      
                "msr cpsr, r3       \n\t"                          
                :                                       
                :                                       
                : "r3" );
}

/*****************************************************************************
 *
 * Description:
 *    Initialize the consol
 *
 ****************************************************************************/
void
eaInit(void)
{
  //initialize the consol
  consolInit();
  
#ifdef CONSOL_STARTUP_DELAY
  /*
   * setup timer #1 for a short delay
   */
  TIMER1_TCR = 0x02;          //stop and reset timer
  TIMER1_PR  = 0x00;          //set prescaler to zero
  TIMER1_MR0 = CONSOL_STARTUP_DELAY_LENGTH * ((FOSC * PLL_MUL)/ (1000000 * PBSD));
  TIMER1_IR  = 0xff;          //reset all interrrupt flags
  TIMER1_MCR = 0x04;          //stop timer on match
  TIMER1_TCR = 0x01;          //start timer
  
  //wait until delay time has elapsed
  while (TIMER1_TCR & 0x01)
    ;
#endif
}

/*****************************************************************************
 *
 * Description:
 *    Below, a number of hook-functions that are needed in order to use
 *    uclibc-functions.
 *
 ****************************************************************************/
#if (USE_NEWLIB == 1)

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <errno.h>

///////////////////
int putchar (int ch)  {                  /* Write character to Serial Port    */

  if (ch == '\n')
    consolSendChar(0x0d);                /* output CR */
  consolSendChar(ch);
  return 1;
}

int getchar (void)  {                      /* Read character from Serial Port */
  while(!(UART_LSR & 0x01));
  return UART_RBR;
}
///////////////////

int _close_r(int fd)
{
    return -1;
}


void _exit_r(int stat)
{
   int oldStat, i;
putchar('\n');
putchar('E');
putchar('X');
putchar('I');
putchar('T');
putchar(':');
consolSendNumber(10,10,0,'0', stat);
putchar('\n');
   oldStat= stat;
   i = 0;
    while (1) {
     i++;         // trap it for debug
	}
}


int _stat_r (struct _reent *ptr, const char *filename, struct stat *st) {
    return -1;
}

int _fstat_r (struct _reent *ptr, int file, struct stat *st)
{
    return -1;
}

int _getpid_r(struct _reent *ptr)
{
    return 1;
}


int _gettimeofday_r (struct _reent *ptr, void * tp, void * tzp) {
    return -1;
}

#if 0
int isatty(int fd)
{
    return 1;
}
#endif

int _kill_r(struct _reent *ptr, int pid, int sig)
{
  if(pid == 1)
    _exit_r(sig);
  return 0;
}


off_t _lseek_r(struct _reent *ptr, int fd, off_t offset, int whence) {
    return (off_t)-1;
}


int _open_r(struct _reent *ptr, const char *buf, int flags, int mode) {
	  return -1;
}


int _write_r(struct _reent *ptr, int fd, const char *buf, int nbytes) {
  int i;
  for (i = 0; i < nbytes; i++) putchar (*buf++);
  return nbytes;
}

void print(char *ptr)
{
  char *p = ptr;

	while (*p != '\0')
    p++;

  _write_r (0, 1, ptr, p-ptr);
}

void _raise_r (struct _reent *ptr)
{
    return;
}


int _read_r(struct _reent *ptr, int fd, char *buf, int nbytes) {
  char c;
  int  i;

  for (i = 0; i < nbytes; i++) {
    c = getchar();
    if (c == 0x0D) break;
    *buf++ = c;
    putchar(c);
  }
  return nbytes - i;
}


char *heap_ptr;

char * _sbrk_r (struct _reent *ptr, int nbytes)
{
    char        *base;

    if (!heap_ptr)  // if it is the very first time for memory allocation.
//	   heap_ptr = (char *)&_heap_begin;      // the begining of the heap memory.
	   heap_ptr = pHeapStart;

    base = heap_ptr;
    heap_ptr += nbytes;
//    if ( heap_ptr >= ( char *) &_heap_end )
    if ( heap_ptr >= pHeapEnd )
        _exit_r( 666);  // out of heap memory!!!
    return base;
}

clock_t _times_r(struct _reent *ptr, struct tms * tp)
{
    clock_t utime;

	utime = 0;
    if (tp) {
	tp->tms_utime = utime;
	tp->tms_stime = 0;
	tp->tms_cutime = 0;
	tp->tms_cstime = 0;
    }

    return utime;
}

int _rename_r (struct _reent *ptr, const char *oldpath, const char *newpath) {
    return -1;
}

int _unlink_r (struct _reent *ptr, const char *pathname)
{
    return -1;
}

int _system_r (struct _reent *ptr, const char *command)
{
    return -1;
}

#define SYS_meminfo     1001

void * __get_memtop(void)
{
  unsigned long topmem = 0;

  return (void*)topmem;
}

#endif
