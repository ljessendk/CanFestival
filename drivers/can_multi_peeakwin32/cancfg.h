/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Jaroslav Fojtik

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

#ifndef __CANCFG_H__
#define __CANCFG_H__


#if defined(__CYGWIN__)
#include <windef.h>
#else
#include <windows.h>
#endif

// Following part of the file is copied by configure script
// from choosen PcanLight header file
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

#ifdef PCAN2_HEADER_
 #include "pcan_2usb.h"
 #define CAN_Init(wBTR0BTR1,Type) CAN2_Init(wBTR0BTR1,Type)
 #define CAN_Close()		  CAN2_Close()
 #define CAN_Status()		  CAN2_Status()
 #define CAN_Write(PCANMsg)	  CAN2_Write(PCANMsg)
 #define CAN_Read(PCANMsg)	  CAN2_Read(PCANMsg)  
 #define CAN_VersionInfo(lpszTextBuff) CAN2_VersionInfo(lpszTextBuff)
 #define CAN_ResetClient()       CAN2_ResetClient() 
 #define CAN_MsgFilter(FromID,ToID,Type) CAN2_MsgFilter(FromID,ToID,Type)
 #define CAN_ReadEx(pMsgBuff,pRcvTime) CAN2_ReadEx(pMsgBuff,pRcvTime)
 #define CAN_SetRcvEvent(hEvent) CAN2_SetRcvEvent(hEvent)
#else
 #include "pcan_usb.h"
#endif


#endif