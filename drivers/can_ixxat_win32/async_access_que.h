// thread safe que
// Tochinski Leonid, Chatten Associayes, Inc.
#pragma once
#include <deque>
#include "AutoReleaseCS.h"

template<typename type>
class async_access_que
   {
   public:
      async_access_que()
         {
         ::InitializeCriticalSection(&m_cs);
         }
      ~async_access_que()
         {
         ::DeleteCriticalSection(&m_cs);
         }

      void append(const type& data)
         {
         AutoReleaseCS acs(m_cs);
         m_data.push_back(data);
         }

      bool extract_top(type& data)
         {
         AutoReleaseCS acs(m_cs);
         if (m_data.empty())
            return false;
          data = m_data.front();
          m_data.pop_front();
         return true;
         }
         
      void clear()
         {
         AutoReleaseCS acs(m_cs);
         m_data.clear();
         }
         
   protected:
      std::deque<type> m_data;
      CRITICAL_SECTION m_cs;
   };
