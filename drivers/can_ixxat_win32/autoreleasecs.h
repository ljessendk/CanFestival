// Critical Section Autorelease
// Tochinski Leonid, Chatten Associates, Inc. 2007
#pragma once

class AutoReleaseCS
   {
   public:
      AutoReleaseCS(CRITICAL_SECTION& cs) : m_cs(cs)
         {
         ::EnterCriticalSection(&m_cs);
         }
      ~AutoReleaseCS()
         {
         ::LeaveCriticalSection(&m_cs);
         }
      CRITICAL_SECTION& m_cs;
   };
