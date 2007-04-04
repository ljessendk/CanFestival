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

#include "nvram_driver.h"
#include "lss_driver.h"
#include "timers_driver.h"
   };

typedef UNS8 (*CANRECEIVE_DRIVER_PROC)(void* inst, Message *m);
typedef UNS8 (*CANSEND_DRIVER_PROC)(void* inst, const Message *m);
typedef void* (*CANOPEN_DRIVER_PROC)(s_BOARD *board);
typedef int (*CANCLOSE_DRIVER_PROC)(void* inst);

typedef int (*NVRAM_OPEN_PROC)(void);
typedef void (*NVRAM_CLOSE_PROC)(void);
typedef char (*NVRAM_WRITE_PROC)(int type, int access_attr, void *data);
typedef char (*NVRAM_READ_PROC)(int type, int access_attr, void *data);

typedef void (*LED_SET_REDGREEN_PROC)(CO_Data *d, unsigned char bits);

typedef UNS8 (*BAUDRATE_VALID_PROC)(UNS32 table_index);
typedef void (*BAUDRATE_SET_PROC)(UNS8 table_index);

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

      // nvram driver
      NVRAM_OPEN_PROC m_nvram_open;
      NVRAM_CLOSE_PROC m_nvram_close;
      NVRAM_WRITE_PROC m_nvram_write;
      NVRAM_READ_PROC m_nvram_read;
      // led driver
      LED_SET_REDGREEN_PROC m_led_set_redgreen;
      // lss driver
      BAUDRATE_VALID_PROC m_baudrate_valid;
      BAUDRATE_SET_PROC m_baudrate_set;

      // driver module habndle
      HMODULE m_driver_handle;
   };

driver_procs::driver_procs() : m_canReceive(0),
      m_canSend(0),
      m_canOpen(0),
      m_canClose(0),
      m_nvram_open(0),
      m_nvram_close(0),
      m_nvram_write(0),
      m_nvram_read(0),
      m_led_set_redgreen(),
      m_baudrate_valid(0),
      m_baudrate_set(0),
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
   if (can_driver_valid())
      return m_driver_handle;
   m_driver_handle = ::LoadLibrary(driver_name);
   if (m_driver_handle == NULL)
      return NULL;

   m_canReceive = (CANRECEIVE_DRIVER_PROC)::GetProcAddress(m_driver_handle, myTEXT("canReceive_driver"));
   m_canSend = (CANSEND_DRIVER_PROC)::GetProcAddress(m_driver_handle, myTEXT("canSend_driver"));
   m_canOpen = (CANOPEN_DRIVER_PROC)::GetProcAddress(m_driver_handle, myTEXT("canOpen_driver"));
   m_canClose = (CANCLOSE_DRIVER_PROC)::GetProcAddress(m_driver_handle, myTEXT("canClose_driver"));

   m_nvram_open = (NVRAM_OPEN_PROC)::GetProcAddress(m_driver_handle, myTEXT("nvram_open_driver"));
   m_nvram_close = (NVRAM_CLOSE_PROC)::GetProcAddress(m_driver_handle, myTEXT("nvram_close_driver"));
   m_nvram_write = (NVRAM_WRITE_PROC)::GetProcAddress(m_driver_handle, myTEXT("nvram_write_driver"));
   m_nvram_read = (NVRAM_READ_PROC)::GetProcAddress(m_driver_handle, myTEXT("nvram_read_driver"));

   m_led_set_redgreen = (LED_SET_REDGREEN_PROC)::GetProcAddress(m_driver_handle, myTEXT("led_set_redgreen_driver"));

   m_baudrate_valid = (BAUDRATE_VALID_PROC)::GetProcAddress(m_driver_handle, myTEXT("baudrate_valid_driver"));
   m_baudrate_set = (BAUDRATE_SET_PROC)::GetProcAddress(m_driver_handle, myTEXT("baudrate_set_driver"));

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

LIB_HANDLE LoadCanDriver(char* driver_name)
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
      driver_data* data = (driver_data*)fd0;
      if ((*s_driver_procs.m_canSend)(data->inst, m))
         return 0;
      }
   return 1;
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
         CreateReceiveTask(data, &data->receive_thread, &canReceiveLoop);
         return data;
         }
      }
   return NULL;
   }

/***************************************************************************/
int canClose(CAN_PORT fd0)
   {
   if (fd0 != NULL && s_driver_procs.m_canClose != NULL)
      {
      driver_data* data = (driver_data*)fd0;
      data->continue_receive_thread = false;
      WaitReceiveTaskEnd(&data->receive_thread);
      (*s_driver_procs.m_canClose)(data->inst);
      delete data;
      return 0;
      }
   return 0;
   }

/***************************************************************************/
int nvram_open(void)
   {
   if (s_driver_procs.m_nvram_read != NULL)
      return (*s_driver_procs.m_nvram_open)();
   return -1;
   }

void nvram_close(void)
   {
   if (s_driver_procs.m_nvram_close != NULL)
      (*s_driver_procs.m_nvram_close)();
   }

char nvram_write(int type, int access_attr, void *data)
   {
   if (s_driver_procs.m_nvram_write != NULL)
      return (*s_driver_procs.m_nvram_write)(type, access_attr, data);
   return 0;
   }

char nvram_read(int type, int access_attr, void *data)
   {
   if (s_driver_procs.m_nvram_read != NULL)
      return (*s_driver_procs.m_nvram_read)(type, access_attr, data);
   return 0;
   }

/***************************************************************************/

void led_set_redgreen(CO_Data *d, unsigned char bits)
   {
   if (s_driver_procs.m_led_set_redgreen != NULL)
      (*s_driver_procs.m_led_set_redgreen)(d, bits);
   }

/***************************************************************************/

UNS8 baudrate_valid(UNS32 table_index)
   {
   if (s_driver_procs.m_baudrate_valid != NULL)
      return (*s_driver_procs.m_baudrate_valid)(table_index);
   return 0;
   }

void baudrate_set(UNS8 table_index)
   {
   if (s_driver_procs.m_baudrate_set != NULL)
      (*s_driver_procs.m_baudrate_set)(table_index);
   }
