/*
This file is part of CanFestival, a library implementing CanOpen Stack.

CanFestival Copyright (C): Edouard TISSERANT and Francis DUPIN
CanFestival Win32 port Copyright (C) 2007 Leonid Tochinski, ChattenAssociates, Inc.

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

// thread safe que
#ifndef __async_access_que_h__
#define __async_access_que_h__

#include <deque>
#include "AutoReleaseCS.h"

template<typename type>
class async_access_que
   {
   public:
      async_access_que()
         {
         ::InitializeCriticalSection(&m_cs);

		 m_newObject = CreateEvent(NULL, FALSE, FALSE, NULL);
		 m_stop = CreateEvent(NULL, FALSE, FALSE, NULL);
		 m_stopped = CreateEvent(NULL, FALSE, FALSE, NULL);
		 m_commands[0] = m_newObject;
		 m_commands[1] = m_stop;

         }
      ~async_access_que()
         {
		 SignalObjectAndWait(m_stop, m_stopped, 500, FALSE);
         ::DeleteCriticalSection(&m_cs);
		 CloseHandle(m_stop);
		 CloseHandle(m_stopped);
		 CloseHandle(m_newObject);
         }

      void append(const type& data)
         {
         AutoReleaseCS acs(m_cs);
         m_data.push_back(data);
	 SetEvent(m_newObject);
         }

      bool extract_top(type& data)
         {
			 bool empty = true;
			 {
				 AutoReleaseCS acs(m_cs);
				 empty = m_data.empty();
			 }

			 if (empty)
			 {
				 DWORD objectIndex;
				 do
				 {
					objectIndex = WaitForMultipleObjects(sizeof(m_commands) / sizeof(&(m_commands[0])), m_commands, FALSE, INFINITE);
					if (objectIndex - WAIT_OBJECT_0 == 1)  //m_stop
					{
						SetEvent(m_stopped);
						return false; //This will exit the canReceive-loop
					}
				} while (objectIndex - WAIT_OBJECT_0 != 0);
			 }

			 {
				AutoReleaseCS acs(m_cs);
				if (m_data.empty())
				{
					return false; //This will exit the canReceive-loop
				}
				data = m_data.front();
				m_data.pop_front();
				ResetEvent(m_newObject);
				return true;
			}
         }
         
      void clear()
         {
         AutoReleaseCS acs(m_cs);
         m_data.clear();
         }
         
   protected:
      std::deque<type> m_data;
      CRITICAL_SECTION m_cs;

	  HANDLE m_newObject;
	  HANDLE m_stop;
	  HANDLE m_stopped;
	  HANDLE m_commands[2];
   };
#endif //__async_access_que_h__