/*

This file is not part of CanFestival.
This is third party contributed file.

It is provided as-this and without any warranty

*/

//****************************************************************************
// Copyright (C) 2006  PEAK System-Technik GmbH
//
// linux@peak-system.com
// www.peak-system.com
//
// This part of software is proprietary. It is allowed to
// distribute it with CanFestival. 
//
// No warranty at all is given.
//
// Maintainer(s): Edouard TISSERANT (edouard.tisserant@lolitech.fr)
//****************************************************************************

/*
   Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Tue Aug  8 23:36:30 2006
*/
#define pcan_34 typedef
#define pcan_51 struct
#define pcan_9 char
#define pcan_39 TASK_HANDLE
#define pcan_1 receiveTask
#define pcan_24 CO_Data
#define pcan_o d
#define pcan_16 s_BOARD
#define pcan_k board
#define pcan_47 pthread_mutex_t
#define pcan_38 PTHREAD_MUTEX_INITIALIZER
#define pcan_12 void
#define pcan_n CAN_HANDLE
#define pcan_d if
#define pcan_52 CAN2_Init
#define pcan_20 baudrate
#define pcan_v CAN_INIT_TYPE_ST
#define pcan_j else
#define pcan_54 CAN_Init
#define pcan_t UNS8
#define pcan_17 canReceive
#define pcan_4 Message
#define pcan_f data
#define pcan_23 TPCANMsg
#define pcan_11 DWORD
#define pcan_22 pthread_mutex_lock
#define pcan_27 CAN2_Read
#define pcan_48 CAN_Read
#define pcan_w CAN_ERR_OK
#define pcan_q MSGTYPE
#define pcan_14 MSGTYPE_STANDARD
#define pcan_10 MSGTYPE_RTR
#define pcan_25 CAN_ERR_BUSOFF
#define pcan_7 printf
#define pcan_y usleep
#define pcan_u pthread_mutex_unlock
#define pcan_p return
#define pcan_28 MSGTYPE_STATUS
#define pcan_8 DATA
#define pcan_46 CAN_ERR_OVERRUN
#define pcan_13 cob_id
#define pcan_15 w
#define pcan_18 ID
#define pcan_2 rtr
#define pcan_z len
#define pcan_6 LEN
#define pcan_3 for
#define pcan_44 canReceiveLoop
#define pcan_19 while
#define pcan_26 EnterMutex
#define pcan_50 canDispatch
#define pcan_30 LeaveMutex
#define pcan_31 CAN_ERR_QRCVEMPTY
#define pcan_40 CAN_ERR_BUSLIGHT
#define pcan_45 CAN_ERR_BUSHEAVY
#define pcan_41 canSend
#define pcan_m errno
#define pcan_32 do
#define pcan_53 CAN2_Write
#define pcan_43 CAN_Write
#define pcan_42 canOpen
#define pcan_21 int
#define pcan_56 break
#define pcan_0 fprintf
#define pcan_5 stderr
#define pcan_37 NULL
#define pcan_29 CreateReceiveTask
#define pcan_57 canClose
#define pcan_49 CAN2_Close
#define pcan_35 CAN_Close
#define pcan_33 WaitReceiveTaskEnd
