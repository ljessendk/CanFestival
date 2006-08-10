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
   Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Wed Aug  9 08:28:43 2006
*/
#define pcan_o getenv
#define pcan_6 strtol
#define pcan_v NULL
#define pcan_37 typedef
#define pcan_53 struct
#define pcan_11 char
#define pcan_41 TASK_HANDLE
#define pcan_5 receiveTask
#define pcan_26 CO_Data
#define pcan_s d
#define pcan_27 s_BOARD
#define pcan_l board
#define pcan_48 pthread_mutex_t
#define pcan_40 PTHREAD_MUTEX_INITIALIZER
#define pcan_14 void
#define pcan_m CAN_HANDLE
#define pcan_d if
#define pcan_55 CAN2_Init
#define pcan_20 baudrate
#define pcan_y CAN_INIT_TYPE_ST
#define pcan_j else
#define pcan_57 CAN_Init
#define pcan_u UNS8
#define pcan_17 canReceive
#define pcan_8 Message
#define pcan_f data
#define pcan_24 TPCANMsg
#define pcan_13 DWORD
#define pcan_19 pthread_mutex_lock
#define pcan_31 CAN2_Read
#define pcan_51 CAN_Read
#define pcan_w CAN_ERR_OK
#define pcan_n MSGTYPE
#define pcan_28 MSGTYPE_STANDARD
#define pcan_15 MSGTYPE_RTR
#define pcan_23 CAN_ERR_BUSOFF
#define pcan_2 printf
#define pcan_x usleep
#define pcan_z pthread_mutex_unlock
#define pcan_q return
#define pcan_32 MSGTYPE_STATUS
#define pcan_12 DATA
#define pcan_50 CAN_ERR_OVERRUN
#define pcan_16 cob_id
#define pcan_25 w
#define pcan_18 ID
#define pcan_9 rtr
#define pcan_1 len
#define pcan_7 LEN
#define pcan_3 for
#define pcan_47 canReceiveLoop
#define pcan_21 while
#define pcan_30 EnterMutex
#define pcan_54 canDispatch
#define pcan_35 LeaveMutex
#define pcan_33 CAN_ERR_QRCVEMPTY
#define pcan_42 CAN_ERR_BUSLIGHT
#define pcan_49 CAN_ERR_BUSHEAVY
#define pcan_43 canSend
#define pcan_p errno
#define pcan_36 do
#define pcan_56 CAN2_Write
#define pcan_46 CAN_Write
#define pcan_44 canOpen
#define pcan_22 int
#define pcan_45 break
#define pcan_4 fprintf
#define pcan_10 stderr
#define pcan_34 CreateReceiveTask
#define pcan_59 canClose
#define pcan_52 CAN2_Close
#define pcan_38 CAN_Close
#define pcan_29 WaitReceiveTaskEnd
