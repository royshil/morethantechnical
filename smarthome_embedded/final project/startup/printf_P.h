#ifndef _PRINTF_P_H_
#define _PRINTF_P_H_

#include "config.h"

#if (USE_NEWLIB == 1)   //use newlib
#include <stdio.h>

#elif (CONSOLE_API_PRINTF == 1) //OWN_PRINTF
#include "consol.h"
#define printf(format, args...)   simplePrintf(format, ## args)

#elif (CONSOLE_API_PRINTF == 0)  //SIMPLE
#include "consol.h"
#define printf(format, args...)   consolSendString(format)
#endif

#endif
