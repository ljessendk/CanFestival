// IXXAT adapter driver for CanFestival-3 Win32 port
// Copyright (C) 2007 Leonid Tochinski, ChattenAssociates, Inc.
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

#pragma warning(disable:4996)

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
      bool open(const char* board_name, int board_number, const char* baud_rate);
      bool close();                             
      void receive_queuedata(UINT16 que_hdl, UINT16 count, VCI_CAN_OBJ* p_obj);
      // VCI2 handler      
      static void VCI_CALLBACKATTR message_handler(char *msg_str);
      static void VCI_CALLBACKATTR receive_queuedata_handler(UINT16 que_hdl, UINT16 count, VCI_CAN_OBJ* p_obj);
      static void VCI_CALLBACKATTR exception_handler(VCI_FUNC_NUM func_num, INT32 err_code, UINT16 ext_err, char* err_str);
      
   private:
      UINT16 m_BoardHdl;
      UINT16 m_TxQueHdl;
      UINT16 m_RxQueHdl;
      async_access_que<VCI_CAN_OBJ> m_RX_Que;
      static IXXAT* m_callbackPtr;
   };

IXXAT *IXXAT::m_callbackPtr = NULL;

IXXAT::IXXAT(s_BOARD *board) : m_BoardHdl(0xFFFF),
                               m_TxQueHdl(0xFFFF),
                               m_RxQueHdl(0xFFFF)
                               
   {
   char busname[100];
   ::strcpy(busname,board->busname);
   char board_name[100];      
   long board_number = 0;   
   char *ptr = ::strrchr(busname,':');
   if (ptr != 0)
      {
      *ptr = 0;
      ::strcpy(board_name,busname);
      if (++ptr - busname < (int)::strlen(board->busname))
         board_number = ::atoi(ptr);
      }
   if (!open(board_name,board_number,board->baudrate))
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
      return false;
   long res = VCI_ERR;
   if (m->rtr == NOT_A_REQUEST)
      res = VCI_TransmitObj(m_BoardHdl, m_TxQueHdl, m->cob_id.w, m->len, const_cast<unsigned char*>(m->data));
   else
      res = VCI_RequestObj(m_BoardHdl, m_TxQueHdl, m->cob_id.w, m->len);
   return (res == VCI_OK);   
   }


bool IXXAT::receive(Message *m)
   {
   if (m_BoardHdl == 0xFFFF)
      return false;
   VCI_CAN_OBJ obj;
   if (m_RX_Que.extract_top(obj))
      {
      m->cob_id.w = obj.id;
      m->len = obj.len;
      m->rtr = (obj.rtr == VCI_RX_BUF) ? NOT_A_REQUEST : REQUEST;
      if (m->rtr == NOT_A_REQUEST)
         ::memcpy(m->data, obj.a_data, m->len);
      return true;
      }
   return false;
   }

bool IXXAT::open(const char* board_name, int board_number, const char* baud_rate)
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
      return false;
   // close existing board   
   close();
   // init IXXAT board
   unsigned long board_type = VCI_GetBrdTypeByName(const_cast<char*>(board_name));
   long res = VCI2_PrepareBoard( board_type,                  // board type
                                   board_number,              // unique board index
                                   NULL,                      // pointer to buffer for additional info 
                                   0,                         // length of additional info buffer
                                   message_handler,           // pointer to msg-callbackhandler
                                   receive_queuedata_handler, // pointer to receive-callbackhandler
                                   exception_handler);        // pointer to exception-callbackhandler
   if (res < 0)
      return false;
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
   res = VCI_ConfigQueue(m_BoardHdl, CAN_NUM, VCI_RX_QUE, 50, 1, 0, 100,  &m_RxQueHdl);

   //  assign the all IDs to the Receive Queue
   res = VCI_AssignRxQueObj(m_BoardHdl, m_RxQueHdl ,VCI_ACCEPT, 0, 0) ;

   //  And now start the CAN
   res = VCI_StartCan(m_BoardHdl, CAN_NUM);
   
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
  char buf[200];
  ::sprintf(buf,"IXXAT Message: [%s]\n", msg_str);
  ::OutputDebugString(buf);
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
  char buf[200];
  ::sprintf(buf, "IXXAT Exception: %s (%i / %u) [%s]\n", Num2Function[func_num], err_code, ext_err, err_str);
  ::OutputDebugString(buf);
  }

//------------------------------------------------------------------------
extern "C"
   UNS8 canReceive_driver(CAN_HANDLE inst, Message *m)
   {
   return (UNS8)reinterpret_cast<IXXAT*>(inst)->receive(m);
   }
                            
extern "C"
   UNS8 canSend_driver(CAN_HANDLE inst, Message *m)
   {
   return (UNS8)reinterpret_cast<IXXAT*>(inst)->send(m);
   }

extern "C"
   CAN_HANDLE canOpen_driver(s_BOARD *board)
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
   int canClose_driver(CAN_HANDLE inst)
   {
   delete reinterpret_cast<IXXAT*>(inst);
   return 1;
   }
