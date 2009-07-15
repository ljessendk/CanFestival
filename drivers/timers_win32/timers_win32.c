/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Edouard TISSERANT and Francis DUPIN
Copyright (C) Win32 Port Leonid Tochinski

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

#include <windows.h>
#include <stdlib.h>
#include <sys/timeb.h>

#include "applicfg.h"
#include "can_driver.h"
#include "timer.h"
#include "timers_driver.h"


struct _timeb timebuffer;

/* Synchronization Object Implementation */
CRITICAL_SECTION CanFestival_mutex;
HANDLE timer_thread = NULL;
HANDLE timer = NULL;

stop_timer=0;

static TimerCallback_t init_callback;


void EnterMutex(void)
{
	EnterCriticalSection(&CanFestival_mutex);
}

void LeaveMutex(void)
{
	LeaveCriticalSection(&CanFestival_mutex);
}

// --------------- CAN Receive Thread Implementation ---------------

void CreateReceiveTask(CAN_HANDLE fd0, TASK_HANDLE* Thread, void* ReceiveLoopPtr)
{
	unsigned long thread_id = 0;
	*Thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReceiveLoopPtr, fd0, 0, &thread_id);
}

void WaitReceiveTaskEnd(TASK_HANDLE *Thread)
{
	WaitForSingleObject(*Thread, INFINITE);
	CloseHandle(*Thread);
}

int TimerThreadLoop(void)
{
	int ret = 0;
	EnterMutex();
	// At first, TimeDispatch will call init_callback.
	SetAlarm(NULL, 0, init_callback, 0, 0);
	LeaveMutex();

	while(!stop_timer && ret == WAIT_OBJECT_0)
	{
		ret = WaitForSingleObject(timer, INFINITE);
		_ftime(&timebuffer);
		EnterMutex();
		TimeDispatch();
		LeaveMutex();
	}
	return 0;
}

void TimerInit(void)
{
	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart = 0;

	InitializeCriticalSection(&CanFestival_mutex);

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	if(NULL == timer)
    {
        printf("CreateWaitableTimer failed (%d)\n", GetLastError());
    }

	// Take first absolute time ref in milliseconds.
	_ftime(&timebuffer);
}

void TimerCleanup(void)
{
	DeleteCriticalSection(&CanFestival_mutex);
}

void StopTimerLoop(TimerCallback_t exitfunction)
{
	EnterMutex();
	exitfunction(NULL,0);
	LeaveMutex();

	stop_timer = 1;
	CloseHandle(timer);
	WaitForSingleObject(timer_thread, INFINITE);
	CloseHandle(timer_thread);
}

void StartTimerLoop(TimerCallback_t _init_callback)
{
	unsigned long timer_thread_id;
	stop_timer = 0;
	init_callback = _init_callback;
	timer_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TimerThreadLoop, NULL, 0, &timer_thread_id);
}

/* Set the next alarm */
void setTimer(TIMEVAL value)
{
	if(value == TIMEVAL_MAX)
	{
		/* cancel timer */
		CancelWaitableTimer(timer);
	}
	else
	{
		LARGE_INTEGER liDueTime;

		/* arg 2 of SetWaitableTimer take 100 ns interval */
		liDueTime.QuadPart = -value;
		//printf("SetTimer(%llu)\n", value);

		if (!SetWaitableTimer(timer, &liDueTime, 0, NULL, NULL, FALSE))
		{
			printf("SetWaitableTimer failed (%d)\n", GetLastError());
		}
	}
}

/* Get the elapsed time since the last occured alarm */
TIMEVAL getElapsedTime(void)
{
	struct _timeb timetmp;
	_ftime(&timetmp);
	return (timetmp.time - timebuffer.time) * 1000000 + (timetmp.millitm - timebuffer.millitm) * 1000;
}

