#
# *** Startup Code (executed after Reset) ***
#

#include "config.h"

# Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs

        .equ    Mode_USR,       0x10
        .equ    Mode_FIQ,       0x11
        .equ    Mode_IRQ,       0x12
        .equ    Mode_SVC,       0x13
        .equ    Mode_ABT,       0x17
        .equ    Mode_UND,       0x1B
        .equ    Mode_SYS,       0x1F

        .equ    I_Bit,          0x80        /* when I bit is set, IRQ is disabled */
        .equ    F_Bit,          0x40        /* when F bit is set, FIQ is disabled */

        .equ    sram_bottom, SRAM_SADDR
        .equ    sram_top,    SRAM_TOP
        .equ    stackTop,    SRAM_TOP

#define VAL_PLLCFG_MSEL  ((PLL_MUL - 1) << 0)
#if (PLL_DIV == 1)
#define PLL_DIV_VALUE 0x00
#elif (PLL_DIV == 2)
#define PLL_DIV_VALUE 0x01
#elif (PLL_DIV == 4)
#define PLL_DIV_VALUE 0x10
#elif (PLL_DIV == 8)
#define PLL_DIV_VALUE 0x11
#endif
#define VAL_PLLCFG_PSEL  (PLL_DIV_VALUE << 5)
#define VAL_PLLCFG       (VAL_PLLCFG_MSEL | VAL_PLLCFG_PSEL)

# Phase Locked Loop (PLL) definitions
        .equ    PLL_BASE,       0xE01FC080  /* PLL Base Address */
        .equ    PLLCON_OFS,     0x00        /* PLL Control Offset*/
        .equ    PLLCFG_OFS,     0x04        /* PLL Configuration Offset */
        .equ    PLLSTAT_OFS,    0x08        /* PLL Status Offset */
        .equ    PLLFEED_OFS,    0x0C        /* PLL Feed Offset */
        .equ    PLLCON_PLLE,    (1<<0)      /* PLL Enable */
        .equ    PLLCON_PLLC,    (1<<1)      /* PLL Connect */
        .equ    PLLSTAT_PLOCK,  (1<<10)     /* PLL Lock Status */

#define HANDLER(HandlerLabel,HandleLabel) \
HandlerLabel:                   ;\
        sub   sp, sp, #4        ;\
        stmfd sp!, {r0}         ;\
        ldr   r0, =HandleLabel  ;\
        ldr   r0, [r0]          ;\
        str   r0, [sp,#4]       ;\
        ldmfd sp!, {r0,pc}

# Starupt Code must be linked first at Address at which it expects to run.

        .text
        .arm
# ******************************************************************************
#   Declare external function
# ******************************************************************************
        .extern lowLevelInit
        .extern exceptionHandlerInit

        .global _startup
        .func   _startup
_startup:

# Exception Vectors
# Mapped to Address 0.

Vectors:
_vectors:
# If vectors are in FLASH, starting at 0x00000000
#if (MAM_MAP == 1)
                B handleReset               /* jump to reset code              */
                B HandlerUndef              /* handlerUndef                    */
                B HandlerSWI                /* SWI interrupt handler           */
                B HandlerPabort             /* handlerPAbort                   */
                B HandlerDabort             /* handlerDAbort                   */
                NOP                         /* Reserved Vector                 */
#if (IRQ_HANDLER == 0)
                B HandlerIRQ                /* handlerIRQ                      */
#else
                LDR PC,[PC,#-0xFF0]         /* jump to address supplied by VIC */
#endif
                B HandlerFIQ                /* handlerFIQ                      */

# Create handlers
HANDLER(HandlerUndef,  HandleUndef)
HANDLER(HandlerSWI,    HandleSWI)
HANDLER(HandlerPabort, HandlePabort)
HANDLER(HandlerDabort, HandleDabort)
#if (IRQ_HANDLER == 0)
HANDLER(HandlerIRQ,    HandleIRQ)
#endif
HANDLER(HandlerFIQ,    HandleFIQ)

# If vectors are in RAM, starting at 0x40000000
#else
                LDR PC,[PC,#resetHandlerAddress - . - 8]    /* handle reset                      */
                LDR PC,[PC,#undefHandlerAddress - . - 8]    /* handlerUndef                      */
                LDR PC,[PC,#swiHandlerAddress - . - 8]      /* SWI interrupt handler             */
                LDR PC,[PC,#pabortHandlerAddress - . - 8]   /* handlerPAbort                     */
                LDR PC,[PC,#dabortHandlerAddress - . - 8]   /* handlerDAbort                     */
                NOP                                         /* Reserved Vector                   */
#if (IRQ_HANDLER == 0)
                LDR PC,[PC,#irqHandlerAddress - . - 8]      /* jump to common irq handler        */
#else
                LDR PC,[PC,#-0xFF0]                         /* jump to address supplied from VIC */
#endif
                LDR PC,[PC,#fiqHandlerAddress - . - 8]      /* handlerFIQ                        */

resetHandlerAddress:
  .word handleReset

undefHandlerAddress:
  .word 0

swiHandlerAddress:
  .word 0

pabortHandlerAddress:
  .word 0

dabortHandlerAddress:
  .word 0

irqHandlerAddress:
  .word 0

fiqHandlerAddress:
  .word 0

#endif

# Reset Handler
handleReset:

#if (USE_PLL == 1)
                LDR     R0, =PLL_BASE
                MOV     R1, #0xAA
                MOV     R2, #0x55

# Disable PLL before programming (in case it was enabled before)
                MOV     R3, #0
                STR     R3, [R0, #PLLCON_OFS]
                STR     R1, [R0, #PLLFEED_OFS]
                STR     R2, [R0, #PLLFEED_OFS]

# Wait for 		
# Configure and Enable PLL
                MOV     R3, #VAL_PLLCFG
                STR     R3, [R0, #PLLCFG_OFS] 
                MOV     R3, #PLLCON_PLLE
                STR     R3, [R0, #PLLCON_OFS]
                STR     R1, [R0, #PLLFEED_OFS]
                STR     R2, [R0, #PLLFEED_OFS]

# Wait until PLL Locked
PLL_Loop:       LDR     R3, [R0, #PLLSTAT_OFS]
                ANDS    R3, R3, #PLLSTAT_PLOCK
                BEQ     PLL_Loop

# Switch to PLL Clock
                MOV     R3, #(PLLCON_PLLE | PLLCON_PLLC)
                STR     R3, [R0, #PLLCON_OFS]
                STR     R1, [R0, #PLLFEED_OFS]
                STR     R2, [R0, #PLLFEED_OFS]
#endif


# Setup Stack for each mode
                LDR     R0, =stackTop

#  Enter Undefined Instruction Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_UND|I_Bit|F_Bit
                MOV     SP, R0
                SUB     R0, R0, #stackSize_UND

#  Enter Abort Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_ABT|I_Bit|F_Bit
                MOV     SP, R0
                SUB     R0, R0, #stackSize_ABT

#  Enter FIQ Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_FIQ|I_Bit|F_Bit
                MOV     SP, R0
                SUB     R0, R0, #stackSize_FIQ

#  Enter IRQ Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_IRQ|I_Bit|F_Bit
                MOV     SP, R0
                SUB     R0, R0, #stackSize_IRQ

#  Enter Supervisor Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_SVC|I_Bit|F_Bit
                MOV     SP, R0
                SUB     R0, R0, #stackSize_SVC

#  Enter System Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_SYS|I_Bit|F_Bit
                MOV     SP, R0

#  Setup a default Stack Limit (when compiled with "-mapcs-stack-check")
                SUB     SL, SP, #stackSize_SYS

#if (MAM_MAP == 2)
# Copy exception vectors into SRAM
                MOV     R8, #SRAM_SADDR
                LDR     R9, =Vectors
#               Move exception vectors (7 vectors + 1 reserved)
                LDMIA   R9!, {R0-R7}
                STMIA   R8!, {R0-R7}
#               Move addresses belonging to exception vectors (7 addresses)
                LDMIA   R9!, {R0-R6}
                STMIA   R8!, {R0-R6}
#endif

#  Call low-level initialization
                BL      lowLevelInit

# Relocate .data section (Copy from ROM to RAM)
                LDR     R1, =_etext
                LDR     R2, =_data
                LDR     R3, =_edata
LoopRel:        CMP     R2, R3
                LDRLO   R0, [R1], #4
                STRLO   R0, [R2], #4
                BLO     LoopRel

# Clear .bss section (Zero init)
                MOV     R0, #0
                LDR     R1, =__bss_start__
                LDR     R2, =__bss_end__
LoopZI:         CMP     R1, R2
                STRLO   R0, [R1], #4
                BLO     LoopZI

# Initialize exception vectors
                BL      exceptionHandlerInit

# Enter the C code
Jump_To_Main:
                ADR     LR, __Return_from_Main           
                MOV     R0, #0
                MOV     R1, #0
                LDR     R2, =main
                BX      R2

__Return_from_Main:
                B       __Return_from_Main	 

        .size   _startup, . - _startup
        .endfunc


#
# Reserve memory for exception handlers
#
        .struct SRAM_SADDR + 0x20
HandleReset:

        .struct SRAM_SADDR + 0x24
HandleUndef:

        .struct SRAM_SADDR + 0x28
HandleSWI:

        .struct SRAM_SADDR + 0x2c
HandlePabort:

        .struct SRAM_SADDR + 0x30
HandleDabort:

        .struct SRAM_SADDR + 0x34
HandleIRQ:

        .struct SRAM_SADDR + 0x38
HandleFIQ:

#
# Reserve memory for stack areas
#
        .struct STK_SADDR
UserStack:                   /* User stack not used (size = 0) */

        .struct UserStack  + stackSize_SYS
SYSStack:

        .struct SYSStack  + stackSize_SVC
SVCStack:

        .struct SVCStack   + stackSize_UND
UndefStack:

        .struct UndefStack + stackSize_ABT
AbortStack:

        .struct AbortStack + stackSize_IRQ
IRQStack:

        .struct IRQStack   + stackSize_FIQ
FIQStack:                   /* this position equals top of SRAM */

        .end

