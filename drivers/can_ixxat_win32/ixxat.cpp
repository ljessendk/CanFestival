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
// pragma based message
// http://www.codeproject.com/macro/location_pragma.asp
#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC2__ __FILE__ "("__STR1__(__LINE__)") : "


#pragma message("*********************************************************************************")
#pragma message("  NOTE: IXXAT Win32 drivers and API should be installed in order to build this project!")
#pragma message(__LOC2__ "See IXXAT.Cpp header for details.")
#pragma message("*********************************************************************************")


// IXXAT adapter driver for CanFestival-3 Win32 port
//
// Notes
//--------------------------------------------
// For building of this project you will need 
// the following IXXAT API files
// Vci2.h
// Vci11un6.lib
//
// IXXAT Win32 drivers and API can be downloaded from
// http://www.ixxat.com/download_vci_en,7547,5873.html
//
// Copy Vci2.h & Vci11un6.lib files to can_ixxat_win32 folder of add path to them in Project settings.


#include <stdio.h>
extern "C" {
#include "applicfg.h"
#include "can_driver.h"
#include "def.h"
}
#include "VCI2.h"
#include "async_access_que.h"

#define CAN_NUM 0

class IXXAT
   {
   public:
      class error
        {
        };
      IXXAT(s_BOARD *board);
      ~IXXAT();
      bool send(const Message *m);
      bool receive(Message *m);
   private:
      bool open(int board_number, const char* baud_rate);
      bool close();
      void receive_queuedata(UINT16 que_hdl, UINT16 count, VCI_CAN_OBJ* p_obj);
      // VCI2 handler      
      static void VCI_CALLBACKATTR message_handler(char *msg_str);
      static void VCI_CALLBACKATTR receive_queuedata_handler(UINT16 que_hdl, UINT16 count, VCI_CAN_OBJ* p_obj);
      static void VCI_CALLBACKATTR exception_handler(VCI_FUNC_NUM func_num, INT32 err_code, UINT16 ext_err, char* err_str);
      
      static void CALLBACK canBusWatchdog(HWND hwnd, UINT msg, UINT_PTR idEvent, DWORD dwTime);
      void watchdog();
   private:
      UINT16 m_BoardHdl;
      UINT16 m_TxQueHdl;
      UINT16 m_RxQueHdl;
      async_access_que<VCI_CAN_OBJ> m_RX_Que;
      static IXXAT* m_callbackPtr;
      static UINT_PTR m_watchdogTimerId;
      static const unsigned int CAN_BUS_WATCHDOG_INTERVAL_MSEC = 10000;

      /** Bitmask inside sts from VCI_ReadCanStatus() that defines the can bus off state.*/
      static const unsigned char STS_CAN_BUS_OFF = 0x80;

      /** Bitmask inside sts from VCI_ReadCanStatus() that defines the can data overrun state.*/
      static const unsigned char STS_CAN_DATA_OVERRUN = 0x20;

      /** Bitmask inside sts from VCI_ReadCanStatus() that defines the remote queue overrun state.*/
      static const unsigned char STS_REMOTE_QUEUE_OVERRUN = 0x04;
   };

IXXAT *IXXAT::m_callbackPtr = NULL;

UINT_PTR IXXAT::m_watchdogTimerId = 0;

IXXAT::IXXAT(s_BOARD *board) : m_BoardHdl(0xFFFF),
                               m_TxQueHdl(0xFFFF),
                               m_RxQueHdl(0xFFFF)
                               
   {
   if (!board)
      {
      close();
      throw error();
      }

   long board_number = 0;

   if (board->busname)
      {
      board_number = atol(board->busname);
      }

   if (!open(board_number, board->baudrate))
      {
      close();
      throw error();
      }
   m_callbackPtr = this;
   }

IXXAT::~IXXAT()
   {
   close();
   m_callbackPtr = 0;
   }

bool IXXAT::send(const Message *m)
   {
   if (m_BoardHdl == 0xFFFF)
      return true; // true -> NOT OK
   long res = VCI_ERR;
   if (m->rtr == NOT_A_REQUEST)
      res = VCI_TransmitObj(m_BoardHdl, m_TxQueHdl, m->cob_id, m->len, const_cast<unsigned char*>(m->data));
   else
      res = VCI_RequestObj(m_BoardHdl, m_TxQueHdl, m->cob_id, m->len);

   return (res == VCI_OK);
   }


bool IXXAT::receive(Message *m)
   {
   if (m_BoardHdl == 0xFFFF)
      return false;
   VCI_CAN_OBJ obj;
   if (m_RX_Que.extract_top(obj))
      {
      m->cob_id = static_cast<UNS16>(obj.id); //valid for 11Bit ids
      m->len = obj.len;
      m->rtr = (obj.rtr == VCI_RX_BUF) ? NOT_A_REQUEST : REQUEST;
      if (m->rtr == NOT_A_REQUEST)
         ::memcpy(m->data, obj.a_data, m->len);
      return true;
      }
   return false;
   }

bool IXXAT::open(int board_number, const char* baud_rate)
   {
   // check, if baudrate is supported
   struct IXXAT_baud_rate_param 
     { 
     UINT8  bt0; 
     UINT8  bt1;
     };
   struct IXXAT_look_up_table
     {
     char baud_rate[20];
     IXXAT_baud_rate_param bt;
     };
   static const IXXAT_look_up_table br_lut[] = {
               {"10K",{VCI_10KB}},
               {"20K",{VCI_20KB}},
               {"50K",{VCI_50KB}},
               {"100K",{VCI_100KB}},
               {"125K",{VCI_125KB}},
               {"250K",{VCI_250KB}},
               {"500K",{VCI_500KB}},
               {"800K",{VCI_800KB}},
               {"1M",{VCI_1000KB}}
               };
   static const long br_lut_size = sizeof (br_lut)/sizeof(IXXAT_look_up_table);
   int index;
   for (index = 0; index < br_lut_size; ++index)
       {
       if (::strcmp(br_lut[index].baud_rate,baud_rate)==0)
          break;
       }
   if (index == br_lut_size)
   {
      MSG_ERR_DRV("IXXAT::open: The given baudrate %S is invalid.", baud_rate);
      return false;
   }
   // close existing board   
   close();
   // init IXXAT board
   long res = VCI2_PrepareBoard(   0,                         // board type, unused in VCI2
                                   board_number,              // unique board index, see XAT_EnumHwEntry() and XAT_GetConfig()
                                   NULL,                      // pointer to buffer for additional info 
                                   0,                         // length of additional info buffer
                                   message_handler,           // pointer to msg-callbackhandler
                                   receive_queuedata_handler, // pointer to receive-callbackhandler
                                   exception_handler);        // pointer to exception-callbackhandler
   if (res < 0)
   {
      MSG_ERR_DRV("IXXAT::open: VCI2_PrepareBoard failed with code '%d'.", res);
      return false;
   }
   m_BoardHdl = (UINT16)res;

   VCI_ResetBoard(m_BoardHdl);
   
   // init CAN parameters
   
   // initialize CAN-Controller
   res = VCI_InitCan(m_BoardHdl, CAN_NUM, br_lut[index].bt.bt0,br_lut[index].bt.bt1, VCI_11B);
   
   //  definition of Acceptance-Mask (define to receive all IDs)
   res = VCI_SetAccMask(m_BoardHdl, CAN_NUM, 0x0UL, 0x0UL);

   // definition of Transmit Queue
   res = VCI_ConfigQueue(m_BoardHdl, CAN_NUM, VCI_TX_QUE, 100 , 0, 0, 0,  &m_TxQueHdl);
   
   //  definition of Receive Queue (interrupt mode)
   res = VCI_ConfigQueue(m_BoardHdl, CAN_NUM, VCI_RX_QUE, 500, 1, 0, 100,  &m_RxQueHdl);

   //  assign the all IDs to the Receive Queue
   res = VCI_AssignRxQueObj(m_BoardHdl, m_RxQueHdl ,VCI_ACCEPT, 0, 0);

   //  And now start the CAN
   res = VCI_StartCan(m_BoardHdl, CAN_NUM);

   //Start CAN Bus-Off watchdog
   m_watchdogTimerId = SetTimer(NULL, NULL, IXXAT::CAN_BUS_WATCHDOG_INTERVAL_MSEC, IXXAT::canBusWatchdog);
   
   return true;
   }

bool IXXAT::close()
   {
   if (m_BoardHdl == 0xFFFF)
      return true;
   VCI_ResetBoard(m_BoardHdl);   
   VCI_CancelBoard(m_BoardHdl);
   m_BoardHdl = 
   m_TxQueHdl = 
   m_RxQueHdl = 0xFFFF;
   return true;
   }

void IXXAT::receive_queuedata(UINT16 que_hdl, UINT16 count, VCI_CAN_OBJ *p_obj)
   {
   for (int i = 0; i < count; ++i)
       m_RX_Que.append(p_obj[i]); // can packet
   }

void VCI_CALLBACKATTR IXXAT::receive_queuedata_handler(UINT16 que_hdl, UINT16 count, VCI_CAN_OBJ *p_obj)
  {
   if (m_callbackPtr != NULL)
      m_callbackPtr->receive_queuedata(que_hdl, count, p_obj);
  }

void VCI_CALLBACKATTR IXXAT::message_handler(char *msg_str)
  {
  MSG_ERR_DRV("IXXAT Message: [%S]", msg_str);
  }

void VCI_CALLBACKATTR IXXAT::exception_handler(VCI_FUNC_NUM func_num, INT32 err_code, UINT16 ext_err, char* err_str)
  {
  static const char* Num2Function[] =
    {
    "VCI_Init",
    "VCI_Searchboard",
    "VCI_Prepareboard",
    "VCI_Cancel_board",
    "VCI_Testboard",
    "VCI_ReadBoardInfo",
    "VCI_ReadBoardStatus",
    "VCI_Resetboard",
    "VCI_ReadCANInfo",
    "VCI_ReadCANStatus",
    "VCI_InitCAN",
    "VCI_SetAccMask",
    "VCI_ResetCAN",
    "VCI_StartCAN",
    "VCI_ResetTimeStamps",
    "VCI_ConfigQueue",
    "VCI_AssignRxQueObj",
    "VCI_ConfigBuffer",
    "VCI_ReconfigBuffer",
    "VCI_ConfigTimer",
    "VCI_ReadQueStatus",
    "VCI_ReadQueObj",
    "VCI_ReadBufStatus",
    "VCI_ReadBufData",
    "VCI_TransmitObj",
    "VCI_RequestObj",
    "VCI_UpdateBufObj",
    "VCI_CciReqData"
    };

  MSG_ERR_DRV("IXXAT Exception: %S (%i / %u) [%S]", Num2Function[func_num], err_code, ext_err, err_str);
  }

  void IXXAT::watchdog()
  {
    VCI_CAN_STS sts;
    long res = VCI_ReadCanStatus(m_BoardHdl, CAN_NUM, &sts);

    if (res < 0)
    {
      MSG_ERR_DRV("IXXAT canBusWatchdog: ERROR: Reading the can state failed!");
    }
    else
    {
      if (sts.sts & (STS_CAN_BUS_OFF | STS_CAN_DATA_OVERRUN | STS_REMOTE_QUEUE_OVERRUN))
      {
        if (sts.sts & STS_CAN_BUS_OFF)
        {
          MSG_ERR_DRV("IXXAT canBusWatchdog: CAN bus off detected!");
        }
        if (sts.sts & STS_CAN_DATA_OVERRUN)
        {
          MSG_ERR_DRV("IXXAT canBusWatchdog: CAN data overrun detected!");
        }
        if (sts.sts & STS_REMOTE_QUEUE_OVERRUN)
        {
          MSG_ERR_DRV("IXXAT canBusWatchdog: Remote queue overrun detected!");
        }

        res = VCI_ResetCan(m_BoardHdl, CAN_NUM);
        if (res <= 0)
        {
          MSG_ERR_DRV("IXXAT canBusWatchdog: ERROR: Resetting the can module failed with code '%d'!", res);
        }

        res = VCI_StartCan(m_BoardHdl, CAN_NUM);
        if (res <= 0)
        {
          MSG_ERR_DRV("IXXAT canBusWatchdog: ERROR: Restaring the can module failed with code '%d'!", res);
        }
      }
    }

    if (SetTimer(NULL, m_watchdogTimerId, IXXAT::CAN_BUS_WATCHDOG_INTERVAL_MSEC, IXXAT::canBusWatchdog) == 0)
    {
      MSG_ERR_DRV("IXXAT canBusWatchdog: ERROR: Creation of the watchdog timer failed!");
    }
  }

void CALLBACK IXXAT::canBusWatchdog(HWND hwnd, UINT msg, UINT_PTR idEvent, DWORD dwTime)
{
   if (m_callbackPtr != NULL)
      m_callbackPtr->watchdog();
}

//------------------------------------------------------------------------
extern "C"
   UNS8 __stdcall canReceive_driver(CAN_HANDLE inst, Message *m)
   {
     return reinterpret_cast<IXXAT*>(inst)->receive(m) ? 0 : 1;
   }
                            
extern "C"
   UNS8 __stdcall canSend_driver(CAN_HANDLE inst, Message const *m)
   {
     return reinterpret_cast<IXXAT*>(inst)->send(m) ? 0 : 1;
   }

extern "C"
   CAN_HANDLE __stdcall canOpen_driver(s_BOARD *board)
   {
   try
      {
      return new IXXAT(board);
      }
   catch (IXXAT::error&)
      {
      return 0;
      }
   }

extern "C"
   int __stdcall canClose_driver(CAN_HANDLE inst)
   {
   delete reinterpret_cast<IXXAT*>(inst);
   return 1;
   }
   
extern "C"
   UNS8 __stdcall canChangeBaudRate_driver( CAN_HANDLE fd, char* baud)
	{
	//printf("canChangeBaudRate not yet supported by this driver\n");
	return 0;
	}
