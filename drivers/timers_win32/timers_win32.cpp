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

extern "C"
{
#include "applicfg.h"
#include "can_driver.h"
#include "timer.h"
#include "timers_driver.h"
};

// --------------- Synchronization Object Implementation ---------------
class ccritical_section
   {
   public:
      ccritical_section()
         {
         ::InitializeCriticalSection(&m_cs);
         }
      ~ccritical_section()
         {
         ::DeleteCriticalSection(&m_cs);
         }
      void enter()
         {
         ::EnterCriticalSection(&m_cs);
         }
      void leave()
         {
         ::LeaveCriticalSection(&m_cs);
         }
   private:
      CRITICAL_SECTION m_cs;
   };

static ccritical_section g_cs;


void EnterMutex(void)
   {
   g_cs.enter();
   }

void LeaveMutex(void)
   {
   g_cs.leave();
   }
// --------------- Synchronization Object Implementation ---------------


// --------------- CAN Receive Thread Implementation ---------------

void CreateReceiveTask(CAN_HANDLE fd0, TASK_HANDLE* Thread, void* ReceiveLoopPtr)
   {
   unsigned long thread_id = 0;
   *Thread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReceiveLoopPtr, fd0, 0, &thread_id);
   }

void WaitReceiveTaskEnd(TASK_HANDLE *Thread)
   {
   ::WaitForSingleObject(*Thread, INFINITE);
   ::CloseHandle(*Thread);
   //*Thread = NULL;
   }
// --------------- CAN Receive Thread Implementation ---------------


// --------------- Timer Thread Implementation ---------------
class class_timers
   {
   public:
      class_timers();
      ~class_timers();
      void start_timer_thread();
      void resume_timer_thread();
      void stop_timer_thread();
      void set_timer(TIMEVAL value);
      TIMEVAL get_elapsed_time();
   private:
      TIMEVAL get_timer() const;   
      static DWORD WINAPI timer_loop_thread_proc(void* arg);
   private:
      TIMEVAL m_last_occured_alarm_time;
      volatile TIMEVAL m_last_alarm_set_time;
      HANDLE m_timer_thread;
      volatile bool m_continue_timer_loop;
      bool m_use_hi_res_timer;
      double m_counts_per_usec;
   };

class_timers::class_timers() : m_last_occured_alarm_time(TIMEVAL_MAX),
      m_last_alarm_set_time(TIMEVAL_MAX),
      m_timer_thread(0),
      m_continue_timer_loop(false),
      m_use_hi_res_timer(false),
      m_counts_per_usec(0.)
   {
   // initialize hi resolution timer
   LARGE_INTEGER counts_per_sec = {0, 0};
   if (::QueryPerformanceFrequency(&counts_per_sec) && counts_per_sec.QuadPart > 0)
      {
      m_use_hi_res_timer = true;
      m_counts_per_usec = counts_per_sec.QuadPart / 1000000.;
      }
   m_use_hi_res_timer = true;
   }

class_timers::~class_timers()
   {
   stop_timer_thread();
   }

// time is in micro seconds
TIMEVAL class_timers::get_timer() const
   {
   if (m_use_hi_res_timer)
      {
      LARGE_INTEGER performance_count = {0, 0};
      ::QueryPerformanceCounter(&performance_count);
      return (TIMEVAL)(performance_count.QuadPart / m_counts_per_usec);
      }
   // hi-res timer is unavailable
   return 1000 * ::GetTickCount();
   }

DWORD WINAPI class_timers::timer_loop_thread_proc(void* arg)
   {
   class_timers* This = reinterpret_cast<class_timers*>(arg);
   while (This->m_continue_timer_loop)
      {
      TIMEVAL cur_time = This->get_timer();
      if (cur_time >= This->m_last_alarm_set_time)
         {
         This->m_last_occured_alarm_time = cur_time;
         This->m_last_alarm_set_time = TIMEVAL_MAX;         
         EnterMutex();
         TimeDispatch();
         LeaveMutex();
         }
      else
         {
         ::Sleep(1);
         }
      }
   return 0;
   }

void class_timers::start_timer_thread()
   {
   if (m_timer_thread == 0)
      {
      unsigned long thread_id = 0;
      m_timer_thread = ::CreateThread(NULL, 0, &timer_loop_thread_proc, this, CREATE_SUSPENDED, &thread_id);
      m_last_alarm_set_time = TIMEVAL_MAX;
      m_last_occured_alarm_time = get_timer();
      }
   }

void class_timers::resume_timer_thread()
   {
   if (m_timer_thread)
      {
      m_continue_timer_loop = true;
      ::ResumeThread(m_timer_thread);
      }
   }

void class_timers::stop_timer_thread()
   {
   if (m_timer_thread)
      {
      m_continue_timer_loop = false;
      ::WaitForSingleObject(m_timer_thread, INFINITE);
      ::CloseHandle(m_timer_thread);
      m_timer_thread = 0;
      }
   }

void class_timers::set_timer(TIMEVAL value)
   {
   m_last_alarm_set_time = (value == TIMEVAL_MAX) ? TIMEVAL_MAX : get_timer() + value;
   }

// elapsed time since last occured alarm
TIMEVAL class_timers::get_elapsed_time()
   {
   return get_timer() - m_last_occured_alarm_time;
   }

// ----------------------------------------------------------

static class_timers s_timers;

void StartTimerLoop(TimerCallback_t init_callback)
   {
   s_timers.start_timer_thread();
   // At first, TimeDispatch will call init_callback.
   if (init_callback != NULL)
      SetAlarm(NULL, 0, init_callback, (TIMEVAL)0, (TIMEVAL)0);
   s_timers.resume_timer_thread();
   }

void StopTimerLoop(void)
   {
   s_timers.stop_timer_thread();
   }

void setTimer(TIMEVAL value)
   {
   s_timers.set_timer(value);
   }

TIMEVAL getElapsedTime(void)
   {
   return s_timers.get_elapsed_time();
   }
