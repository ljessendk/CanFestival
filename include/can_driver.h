/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Edouard TISSERANT and Francis DUPIN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __can_driver_h__
#define __can_driver_h__

struct struct_s_BOARD;

typedef struct struct_s_BOARD s_BOARD;

#include "can.h"

UNS8 canReceive(CAN_HANDLE fd0, Message *m);
UNS8 canSend(CAN_HANDLE fd0, Message *m);
CAN_HANDLE canOpen(s_BOARD *board);
int canClose(CAN_HANDLE fd0);
void canReceiveLoop(CAN_HANDLE fd0);

#include "data.h"

struct struct_s_BOARD {
  char * busname;
  int baudrate;
  CO_Data * d;
};

#endif
