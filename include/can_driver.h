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

#include "applicfg.h"
#include "can.h"

struct struct_s_BOARD {
  char * busname;
  char * baudrate;
};

#ifndef DLL_CALL
#define DLL_CALL(funcname) funcname##_driver
#endif

#ifndef FCT_PTR_INIT
#define FCT_PTR_INIT
#endif


UNS8 DLL_CALL(canReceive)(CAN_HANDLE, Message *)FCT_PTR_INIT;
UNS8 DLL_CALL(canSend)(CAN_HANDLE, Message *)FCT_PTR_INIT;
CAN_HANDLE DLL_CALL(canOpen)(s_BOARD *)FCT_PTR_INIT;
int DLL_CALL(canClose)(CAN_HANDLE)FCT_PTR_INIT;

#endif
