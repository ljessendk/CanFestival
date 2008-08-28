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

/*
 CAN driver interface.
*/

#include <windows.h>

extern "C"
   {
#define DLL_CALL(funcname) (*_##funcname)
#define FCT_PTR_INIT =NULL
#include "canfestival.h"
#include "timer.h"

#include "timers_driver.h"
   };

typedef UNS8 (*CANRECEIVE_DRIVER_PROC)(void* inst, Message *m);
typedef UNS8 (*CANSEND_DRIVER_PROC)(void* inst, const Message *m);
typedef void* (*CANOPEN_DRIVER_PROC)(s_BOARD *board);
typedef int (*CANCLOSE_DRIVER_PROC)(void* inst);


class driver_procs
   {
   public:
      driver_procs();
      ~driver_procs();

      HMODULE load_canfestival_driver(LPCTSTR driver_name);
      bool can_driver_valid() const;

   public:
      // can driver
      CANRECEIVE_DRIVER_PROC m_canReceive;
      CANSEND_DRIVER_PROC m_canSend;
      CANOPEN_DRIVER_PROC m_canOpen;
      CANCLOSE_DRIVER_PROC m_canClose;

      // driver module habndle
      HMODULE m_driver_handle;
   };

driver_procs::driver_procs() : m_canReceive(0),
      m_canSend(0),
      m_canOpen(0),
      m_canClose(0),
      m_driver_handle(0)
   {}

driver_procs::~driver_procs()
   {
   if (m_driver_handle)
      ::FreeLibrary(m_driver_handle);
   }

bool driver_procs::can_driver_valid() const
   {
   return ((m_canReceive != NULL) &&
           (m_canSend != NULL) &&
           (m_canOpen != NULL) &&
           (m_canClose != NULL));
   }

// GetProcAddress doesn't have an UNICODE version for NT
#ifdef UNDER_CE
  #define myTEXT(str) TEXT(str)
#else
  #define myTEXT(str) str
#endif

HMODULE driver_procs::load_canfestival_driver(LPCTSTR driver_name)
   {
  //LPCTSTR driver1 = "C:\\msys\\1.0\\home\\Ontaide\\can\\CanFestival-3\\drivers\\can_peak_win32\\cygcan_peak_win32.dll";
  //LPCTSTR driver2 = "C:\\msys\\1.0\\home\\Ontaide\\can\\CanFestival-3\\drivers\\can_peak_win32\\cygcan_peak_win32.dll";
  //printf("can_driver_valid=%d\n",can_driver_valid());
   if (can_driver_valid())
      return m_driver_handle;
   printf("driver_name=%s\n",driver_name);
   m_driver_handle = ::LoadLibrary(driver_name);
   //printf("m_driver_handle=%d\n",m_driver_handle);
   //printf("testerror =%s\n",GetLastError());
   if (m_driver_handle == NULL)
      return NULL;

   m_canReceive = (CANRECEIVE_DRIVER_PROC)::GetProcAddress(m_driver_handle, myTEXT("canReceive_driver"));
   m_canSend = (CANSEND_DRIVER_PROC)::GetProcAddress(m_driver_handle, myTEXT("canSend_driver"));
   m_canOpen = (CANOPEN_DRIVER_PROC)::GetProcAddress(m_driver_handle, myTEXT("canOpen_driver"));
   m_canClose = (CANCLOSE_DRIVER_PROC)::GetProcAddress(m_driver_handle, myTEXT("canClose_driver"));
   return can_driver_valid()?m_driver_handle:NULL;
   }

struct driver_data
   {
   CO_Data * d;
   HANDLE receive_thread;
   void* inst;
   volatile bool continue_receive_thread;
   };

driver_procs s_driver_procs;

LIB_HANDLE LoadCanDriver(const char* driver_name)
   {
		return s_driver_procs.load_canfestival_driver((LPCTSTR)driver_name);
   }

UNS8 canReceive(CAN_PORT fd0, Message *m)
   {
  
   if (fd0 != NULL && s_driver_procs.m_canReceive != NULL)
	   {
		  driver_data* data = (driver_data*)fd0;
		  return (*s_driver_procs.m_canReceive)(data->inst, m);
	   }
   return 1;
   }

void* canReceiveLoop(CAN_PORT fd0)
   {
   driver_data* data = (driver_data*)fd0;
   Message m;
   while (data->continue_receive_thread)
      {
      if (!canReceive(fd0, &m))
         {
         EnterMutex();
         canDispatch(data->d, &m);
         LeaveMutex();
         }
      else
         {
		 break;
         ::Sleep(1);
         }
      }
   return 0;
   }

/***************************************************************************/
UNS8 canSend(CAN_PORT fd0, Message *m)
   {   
   if (fd0 != NULL && s_driver_procs.m_canSend != NULL)
      {
      UNS8 res;
      driver_data* data = (driver_data*)fd0;
      res = (*s_driver_procs.m_canSend)(data->inst, m);      
      if (res)
         return 1; // OK
      }
   return 0; // NOT OK
   }

/***************************************************************************/
CAN_HANDLE canOpen(s_BOARD *board, CO_Data * d)
   {
   if (board != NULL && s_driver_procs.m_canOpen != NULL)
      {
      void* inst = (*s_driver_procs.m_canOpen)(board);
      if (inst != NULL)
         {
         driver_data* data = new driver_data;
         data->d = d;
         data->inst = inst;
         data->continue_receive_thread = true;
         CreateReceiveTask(data, &data->receive_thread, (void*)&canReceiveLoop);
	 EnterMutex();
         d->canHandle = data;
         LeaveMutex();
         return data;
         }
      }
   return NULL;
   }

/***************************************************************************/
int canClose(CO_Data * d)
   {
   if (s_driver_procs.m_canClose != NULL)
      {
		  driver_data* data;
		  EnterMutex();
		  if(d->canHandle != NULL){
			data = (driver_data*)d->canHandle;
			d->canHandle = NULL;
			data->continue_receive_thread = false;}
		  LeaveMutex();
		  (*s_driver_procs.m_canClose)(data->inst);
		  WaitReceiveTaskEnd(&data->receive_thread);
		  delete data;
		  return 0;
      }
   return 0;
   }


