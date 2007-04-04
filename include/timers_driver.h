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

#ifndef __timer_driver_h__
#define __timer_driver_h__

#include "timerscfg.h"
#include "timer.h"
/*void initTimer();*/

// For use from CAN driver
void EnterMutex(void);
void LeaveMutex(void);
void WaitReceiveTaskEnd(TASK_HANDLE);

// For use from application
void StartTimerLoop(TimerCallback_t init_callback);
void StopTimerLoop(void);
void CreateReceiveTask(CAN_PORT , TASK_HANDLE* , void* );

#endif
