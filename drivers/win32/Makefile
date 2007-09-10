#! gmake

#
# Copyright (C) 2006 Laurent Bessard
# 
# This file is part of canfestival, a library implementing the canopen
# stack
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# 

CC = mingw32-g++
OPT_CFLAGS = -O2
CFLAGS = $(OPT_CFLAGS)
PROG_CFLAGS = 
OS_NAME = MINGW32
ARCH_NAME = x86
PREFIX = /usr/local
TARGET = win32
CAN_DRIVER = can_peak_win32
TIMERS_DRIVER = timers_win32
ENABLE_DLL_DRIVERS = 1

INCLUDES = -I../../include -I../../include/$(TARGET) -I../../include/$(TIMERS_DRIVER)

OBJS = $(TARGET).o

# add timers driver if any
ifneq ($(TIMERS_DRIVER),timers_)
OBJS += ../$(TIMERS_DRIVER)/$(TIMERS_DRIVER).o
endif

SRC_HFILES = ../../include/$(TARGET)/applicfg.h ../../include/$(TARGET)/canfestival.h

TARGET_HFILES = $(PREFIX)/include/$(TARGET)/applicfg.h $(PREFIX)/include/$(TARGET)/canfestival.h

all: driver

driver: $(OBJS)


%o: %cpp
	$(CC) $(CFLAGS) $(PROG_CFLAGS) ${PROGDEFINES} $(INCLUDES) -o $@ -c $<

#../$(TIMERS_DRIVER)/$(TIMERS_DRIVER).o:
#	$(MAKE) -C ../$(TIMERS_DRIVER) driver

#../$(CAN_DRIVER)/$(CAN_DRIVER).o:
#	$(MAKE) -C ../$(CAN_DRIVER) driver

libcanfestival_$(TARGET).a: $(OBJS)
	@echo Building [libcanfestival_$(TARGET).a]
	ar rc $@ $(OBJS)
	ranlib $@

install: libcanfestival_$(TARGET).a
	mkdir -p $(PREFIX)/lib/
	mkdir -p $(PREFIX)/include/canfestival
	cp libcanfestival_$(TARGET).a $(PREFIX)/lib/
	cp $(SRC_HFILES) $(PREFIX)/include/canfestival

uninstall:
	rm -f $(PREFIX)/lib/libcanfestival_$(TARGET).a
	rm -f $(TARGET_HFILES)

clean:
	rm -f $(OBJS)
	rm -f libcanfestival_$(TARGET).a

mrproper: clean


