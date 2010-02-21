#----------------------------------------------------------------------
# DEFINE TARGET TO BE CREATED
#----------------------------------------------------------------------
ifeq (true, $(PROGRAM_MK))
TARGET     = $(NAME).hex
endif

ifeq (true, $(LIBRARY_MK))
TARGET     = $(NAME).a
endif

ifeq ($(origin TARGET), undefined)
.PHONY: error_message
error_message:
	$(error ERROR: You must specify PROGRAM_MK or LIBRARY_MK)
endif

#----------------------------------------------------------------------
# TOOL DEFINITIONS
#----------------------------------------------------------------------
TOOLTARGET      = arm-elf
AR              = $(TOOLTARGET)-ar
AS              = $(TOOLTARGET)-gcc
AWK             = gawk
CC              = $(TOOLTARGET)-gcc
LD              = $(TOOLTARGET)-gcc
OBJCOPY         = $(TOOLTARGET)-objcopy
RANLIB          = $(TOOLTARGET)-ranlib
RM              = rm -f
SED             = sed
SHELL           = bash --posix
SIZE            = $(TOOLTARGET)-size
STRIP           = $(TOOLTARGET)-strip

#----------------------------------------------------------------------
# DETERMINE ARM/THUMB MODE
#----------------------------------------------------------------------
#check if CODE is undefined
ifeq ($(origin CODE), undefined)
CODE        = ARM
TARGET_MODE = armthumb
else
ifeq ($(CODE),THUMB)
TARGET_MODE = armthumb
else
TARGET_MODE = arm
endif
endif

ifeq ($(TARGET_MODE),armthumb)
THUMB_IW = -mthumb-interwork
else
THUMB_IW =
endif

ifeq ($(CODE),THUMB)
T_FLAGS = -mthumb -DTHUMB_CSTART -DTHUMB_INTERWORK
else
T_FLAGS =
endif

ifeq ($(DEBUG),1)
DBFLAGS += -gdwarf-2
else
DBFLAGS +=
endif

ifeq (RAM, $(LD_RAMROM))
RAM_EXEC = -DRAM_EXEC
else
RAM_EXEC =
endif
#----------------------------------------------------------------------
# COMPILER AND ASSEMBLER OPTIONS
#----------------------------------------------------------------------
W_OPTS    = -Wall -Wcast-align -Wcast-qual -Wimplicit \
            -Wnested-externs -Wpointer-arith -Wswitch \
            -Wreturn-type 
# -Wshadow -Wunused
# -Wmissing-declarations -Wmissing-prototypes -Wredundant-decls -Wstrict-prototypes

CPU       = arm7tdmi
OPTS      = -mcpu=$(CPU) $(THUMB_IW)
CA_OPTS   = $(OPTS) $(INC) -DEL -DGCC $(THUMB_IW) $(T_FLAGS) $(EFLAGS) -D$(CPU_VARIANT) $(RAM_EXEC)
CC_OPTS   = $(CA_OPTS) $(OFLAGS) $(DBFLAGS) $(W_OPTS) -Wa,-ahlms=$(<:.c=.lst)
CC_OPTS_A = $(CA_OPTS) -x assembler-with-cpp -gstabs -Wa,-alhms=$(<:.S=.lst)

#----------------------------------------------------------------------
# LINKER OPTIONS
#----------------------------------------------------------------------

ifeq ($(CPU_VARIANT),LPC2101)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_2k_8k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_2k_8k_rom.ld
      else
LD_SCRIPT = build_files/link_2k_8k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2102)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_4k_16k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_4k_16k_rom.ld
      else
LD_SCRIPT = build_files/link_4k_16k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2103)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_8k_32k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_8k_32k_rom.ld
      else
LD_SCRIPT = build_files/link_8k_32k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2104)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_16k_128k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_16k_128k_rom.ld
      else
LD_SCRIPT = build_files/link_16k_128k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2105)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_32k_128k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_32k_128k_rom.ld
      else
LD_SCRIPT = build_files/link_32k_128k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2106)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_64k_128k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_64k_128k_rom.ld
      else
LD_SCRIPT = build_files/link_64k_128k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2114)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_16k_128k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_16k_128k_rom.ld
      else
LD_SCRIPT = build_files/link_16k_128k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2124)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_16k_256k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_16k_256k_rom.ld
      else
LD_SCRIPT = build_files/link_16k_256k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2119)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_16k_128k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_16k_128k_rom.ld
      else
LD_SCRIPT = build_files/link_16k_128k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2129)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_16k_256k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_16k_256k_rom.ld
      else
LD_SCRIPT = build_files/link_16k_256k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2131)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_8k_32k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_8k_32k_rom.ld
      else
LD_SCRIPT = build_files/link_8k_32k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2132)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_16k_64k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_16k_64k_rom.ld
      else
LD_SCRIPT = build_files/link_16k_64k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2134)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_16k_128k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_16k_128k_rom.ld
      else
LD_SCRIPT = build_files/link_16k_128k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2136)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_32k_256k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_32k_256k_rom.ld
      else
LD_SCRIPT = build_files/link_32k_256k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2138)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_32k_512k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_32k_512k_rom.ld
      else
LD_SCRIPT = build_files/link_32k_512k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2141)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_8k_32k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_8k_32k_rom.ld
      else
LD_SCRIPT = build_files/link_8k_32k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2142)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_16k_64k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_16k_64k_rom.ld
      else
LD_SCRIPT = build_files/link_16k_64k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2144)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_16k_128k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_16k_128k_rom.ld
      else
LD_SCRIPT = build_files/link_16k_128k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2146)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_32k_256k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_32k_256k_rom.ld
      else
LD_SCRIPT = build_files/link_32k_256k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2148)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_32k_512k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_32k_512k_rom.ld
      else
LD_SCRIPT = build_files/link_32k_512k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2194)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_16k_256k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_16k_256k_rom.ld
      else
LD_SCRIPT = build_files/link_16k_256k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2210)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_16k_0k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_16k_0k_rom.ld
      else
LD_SCRIPT = build_files/link_16k_0k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2220)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_64k_0k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_64k_0k_rom.ld
      else
LD_SCRIPT = build_files/link_64k_0k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2212)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_16k_128k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_16k_128k_rom.ld
      else
LD_SCRIPT = build_files/link_16k_128k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2214)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_16k_256k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_16k_256k_rom.ld
      else
LD_SCRIPT = build_files/link_16k_256k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2290)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_16k_0k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_16k_0k_rom.ld
      else
LD_SCRIPT = build_files/link_16k_0k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2292)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_16k_256k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_16k_256k_rom.ld
      else
LD_SCRIPT = build_files/link_16k_256k_ram.ld
      endif
    endif
  endif
else

ifeq ($(CPU_VARIANT),LPC2294)
  ifndef LD_SCRIPT
    ifndef LD_RAMROM
LD_SCRIPT = build_files/link_16k_256k_rom.ld
    else
      ifeq ($(LD_RAMROM),ROM)
LD_SCRIPT = build_files/link_16k_256k_rom.ld
      else
LD_SCRIPT = build_files/link_16k_256k_ram.ld
      endif
    endif
  endif
else
.PHONY: error_message2
error_message2:
	$(error ERROR: You must specify CPU_VARIANT in the makefile)
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif

ifndef LD_SCRIPT_PATH
LD_OPTS   = $(OPTS) $(EFLAGS) -nostartfiles -T $(LD_SCRIPT) -o $(NAME).elf -Wl,-Map=$(NAME).map,--cref
else
LD_OPTS   = $(OPTS) $(EFLAGS) -nostartfiles -T $(LD_SCRIPT_PATH)/$(LD_SCRIPT) -o $(NAME).elf -Wl,-Map=$(NAME).map,--cref
endif

#----------------------------------------------------------------------
# LIST ALL OBJECT FILES
#----------------------------------------------------------------------
OBJS ?= $(CSRCS:.c=.o) $(ASRCS:.S=.o)

#----------------------------------------------------------------------
# BUILD RULES
#----------------------------------------------------------------------
export LD_RAMROM

all: pre_all depend $(TARGET)

pre_all:
ifdef SUBDIRS
	@for dir in $(SUBDIRS) ; do \
	  (cd $$dir && $(MAKE) $(MAKEFLAGS) all) || exit 1; \
	done
endif

depend:
	$(RM) .depend 
	@for f in $(CSRCS) ; do \
		$(CC) -MM $(CC_OPTS) $$f >> .depend ; \
	done
	@for f in $(ASRCS) ; do \
		$(AS) -MM $(CC_OPTS) $$f >> .depend ; \
	done

ifneq (clean, $(MAKECMDGOALS))
-include .depend
endif

%.o: %.c
	$(CC) -c $(CC_OPTS) -o $@ $<
%.o: %.S
	$(AS) -c $(CC_OPTS_A) -o $@ $<

# Always recompile all if makefile changes
$(OBJS): ./makefile

clean:
ifdef SUBDIRS
	@for dir in $(SUBDIRS) ; do \
		(cd $$dir && $(MAKE) $(MAKEFLAGS) clean) || exit 1; \
	done
endif
	$(RM) .depend $(TARGET) $(OBJS) $(OBJS:.o=.lst) 
ifeq (0, $(MAKELEVEL))
ifeq (true, $(PROGRAM_MK))
	$(RM) $(NAME).hex $(NAME).elf $(NAME).map $(NAME).obj
endif
endif


#----------------------------------------------------------------------
# CODE SIZE
#----------------------------------------------------------------------
ifeq (0, $(MAKELEVEL))
all: codesize
endif

codesize: $(TARGET)
	@echo ""
	@echo "=== Result ==============================================="
	@echo ""
	@echo " TEXT: Code size"
	@echo " DATA: Initialized data"
	@echo " BSS:  Uninitialized data"
	@echo " ROM:  Size in non-volatile program memory (TEXT + DATA)"
	@echo " RAM:  Size in volatile data memory (DATA + BSS)"
ifdef LIBS
	@echo ""
	@echo "=== Component sizes ======================================"
	@echo ""
	@echo "   TEXT    DATA     BSS     ROM     RAM FILENAME"
	@echo "   ====    ====     ===     ===     === ========"
# ifdef LIBS
	@for f in $(LIBS) ; do \
	  $(SIZE) "$$f" \
	  | $(AWK) "NR > 1 { t+=\$$1; d+=\$$2; b+=\$$3; } \
		    END { ro=t+d; ra=d+b; printf \"%7d %7d %7d %7d %7d %s\n\", \
						 t,d,b,ro,ra,\"$$f\" }" ; \
	done
endif
	@echo ""
	@echo ""
ifeq (true, $(PROGRAM_MK))
	@echo "=== Total size (including external libraries) ============"
	@echo ""
	@echo "   Code compiled with optimization switch: " $(OFLAGS)
	@echo "   (other settings may produce different sizes)"
	@echo "   (external libraries may be compiled with different settings)"
	@echo ""
	@echo "   TEXT    DATA     BSS     ROM     RAM FILENAME"
	@echo "   ====    ====     ===     ===     === ========"
	@$(SIZE) $(NAME).elf \
	| $(AWK) 'NR > 1 { printf "%7d %7d %7d %7d %7d %s\n", \
			   $$1,$$2,$$3,$$1+$$2,$$2+$$3,"$(NAME).elf" }'
	@echo ""
ifeq (RAM, $(LD_RAMROM))
	@echo "                          |===========|"
	@echo "                               ^^^"
	@echo "   Everything placed in RAM since LD_RAMROM = RAM"
	@echo ""
endif

else
	@echo "=== Total size ==========================================="
	@echo ""
	@echo "   Code compiled with optimization switch: " $(OFLAGS)
	@echo "   (other settings may produce different sizes)"
	@echo ""
	@echo "   TEXT    DATA     BSS     ROM     RAM FILENAME"
	@echo "   ====    ====     ===     ===     === ========"
	@$(SIZE) $(TARGET) \
	| $(AWK) 'NR > 1 { t+=$$1; d+=$$2; b+=$$3; } \
		  END { ro=t+d; ra=d+b; printf "%7d %7d %7d %7d %7d %s\n", \
					t,d,b,ro,ra,"$(TARGET)" }'
endif

#----------------------------------------------------------------------
# BUILD RULES FOR EXECUTABLE PROGRAM
#----------------------------------------------------------------------
ifeq (true, $(PROGRAM_MK))
$(TARGET): $(OBJS) $(LIBS)
	$(LD) $(OBJS) $(LIBS) $(INC) $(LD_OPTS)
	$(OBJCOPY) -O $(HEX_FORMAT) $(NAME).elf $(TARGET)
endif

#----------------------------------------------------------------------
# BUILD RULES FOR LIBRARY
#----------------------------------------------------------------------
ifeq (true, $(LIBRARY_MK))
$(TARGET): $(OBJS) $(LIBS)
	$(AR) cr $@ $(OBJS)
ifdef LIBS
	set -x ; \
	for lib in $(LIBS) ; do \
	  contents=`$(AR) t $$lib` ; \
	  $(AR) x $$lib ; \
	  $(AR) r $@ $$contents ; \
	  $(RM) $$contents ; \
	done
endif
	$(RANLIB) $@
endif

#----------------------------------------------------------------------
# RULES FOR STARTING PROGRAM DOWNLOAD
#----------------------------------------------------------------------
deploy: all
	@echo ""
	@echo "=== Start program download ==============================="
	@echo ""
	$(DOWNLOAD) -hex -term -control $(TARGET) $(DL_COMPORT) $(DL_BAUDRATE) $(DL_CRYSTAL)

#----------------------------------------------------------------------
# RULES FOR STARTING TERMINAL PROGRAM
#----------------------------------------------------------------------
terminal:
	@echo ""
	@echo "=== Start terminal ======================================="
	@echo ""
	$(DOWNLOAD) -hex -termonly $(TARGET) $(DL_COMPORT) $(DL_BAUDRATE) $(DL_CRYSTAL)
