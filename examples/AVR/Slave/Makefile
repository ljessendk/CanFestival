###############################################################################
# Makefile for the project SlaveAVR
###############################################################################

## General Flags
PROJECT = SlaveAVR
MCU = at90can128
TARGET = AVR
CC = avr-gcc
SRC = ../../../src
DRV = ../../../drivers/AVR
OBJDICTGEN = ../../../objdictgen/objdictgen.py
OBJDICTGEN_DEPS = ../../../objdictgen/gen_cfile.py
VPATH+=$(SRC)
VPATH+=$(DRV)

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -Os -fsigned-char -fpack-struct
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=$(PROJECT).map

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Include Directories
INCLUDES = -I../../../include/AVR -I../../../include

## Objects that must be built in order to link
OBJECTS = 	can_AVR.o\
		timer_AVR.o\
		dcf.o\
		timer.o\
		emcy.o\
		lifegrd.o\
		lss.o\
		nmtMaster.o\
		nmtSlave.o\
		objacces.o\
		pdo.o\
		sdo.o\
		states.o\
		sync.o\
		ObjDict.o\
		ds401.o\
		main.o

## Build
all: $(PROJECT).elf $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss size

## Compile
%.o: %.c
#	@echo " "
	@echo "---------------------------------------------------------------------------"
	@echo "**Compiling $< -> $@"
#	@echo "*********************************************"
	$(CC) $(INCLUDES) $(CFLAGS) -c $<
#	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $< 


##Link
$(PROJECT).elf: $(OBJECTS)
#	@echo " "
	@echo "---------------------------------------------------------------------------"
	@echo "**Linking :  $@"
#	@echo "*********************************************"
	$(CC) $(LDFLAGS) $(LIBDIRS) $(LIBS) $(^F) -o $@

%.hex: $(PROJECT).elf
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(PROJECT).elf
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(PROJECT).elf
	avr-objdump -h -S $< > $@

size: $(PROJECT).elf
	@echo
	@avr-size $(PROJECT).elf

## Clean target
.PHONY: clean
clean:
	-rm -rf *.o $(PROJECT).elf dep/* $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss $(PROJECT).map

ObjDict.c ObjDict.h : ObjDict.od $(OBJDICTGEN) $(OBJDICTGEN_DEPS)
	$(OBJDICTGEN) $< ObjDict.c
	

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

