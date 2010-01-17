##########################################################
#
# General makefile for building executable programs and
# libraries for Embedded Artists' QuickStart Boards.
# (C) 2001-2005 Embedded Artists AB
#
##########################################################

# Name of target (executable program or library) 
NAME      = libea_startup_thumb

# Link program to RAM or ROM (possible values for LD_RAMROM is RAM or ROM,
# if not specified = ROM)
# Get value from parent makefile instead
#LD_RAMROM = ROM 

# Name if specific CPU used (used by linker scripts to define correct memory map)
# Valid CPUs are: LPC2101, LPC2102, LPC2103, LPC2104, LPC2105, LPC2106
#                 LPC2114, LPC2119
#                 LPC2124, LPC2129
#                 LPC2131, LPC2132, LPC2134, LPC2136, LPC2138
#                 LPC2141, LPC2142, LPC2144, LPC2146, LPC2148
#                 LPC2194
#                 LPC2210, LPC2220, LPC2212, LPC2214,
#                 LPC2290, LPC2292, LPC2294
# If you have a new version not specified above, just select one of the old
# versions with the same memory map.
CPU_VARIANT = LPC2148

# It is possible to override the automatic linker file selection with the variable below.
# No not use this opion unless you have very specific needs.
#LD_SCRIPT = build_files/myOwnLinkScript_rom.ld

# ELF-file contains debug information, or not
# (possible values for DEBUG are 0 or 1)
# Extra debug flags can be specified in DBFLAGS
DEBUG   = 1
#DBFLAGS =

# Optimization setting
# (-Os for small code size, -O2 for speed)
OFLAGS  = -Os

# Extra general flags
# For example, compile for ARM / THUMB interworking (EFLAGS = -mthumb-interwork)
EFLAGS  = -mthumb-interwork

# Program code run in ARM or THUMB mode
# Can be [ARM | THUMB]
CODE    = ARM

# List C source files here.
CSRCS   = consol.c \
          framework.c

# List assembler source files here
ASRCS   = startup.S

# List subdirectories to recursively invoke make in 
SUBDIRS =

# List additional libraries to link with
LIBS    =

# Add include search paths
INC     = -I .

# Select if an executable program or a library shall be created
#PROGRAM_MK  = true
LIBRARY_MK  = true

# Output format on hex file (if making a program); can be [srec | ihex]
# Only needed for executable program files
HEX_FORMAT  = ihex

# Program to download executable program file into microcontroller's FLASH
# Only needed for executable program files
DOWNLOAD    = lpc21isp.exe

# Configurations for download program
DL_COMPORT  = com1
DL_BAUDRATE = 38400
DL_CRYSTAL  = 12000

#######################################################################
include ../build_files/general.mk
#######################################################################
