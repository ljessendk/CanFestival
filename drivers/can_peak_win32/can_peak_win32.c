/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Edouard TISSERANT and Francis DUPIN

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h> /* for NULL */
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

/* driver pcan pci for Peak board */
//#include "libpcan.h"
//#include "pcan.h"

#include <applicfg.h>
#include "timer.h"
#include "can_driver.h"
#include "timers_driver.h"

#define MAX_NB_CAN_PORTS 1

typedef struct {
  char used;
  TASK_HANDLE receiveTask;
  CO_Data* d;
} CANPort;

CANPort canports[MAX_NB_CAN_PORTS] = {{0,},};

// Define for rtr CAN message
#define CAN_INIT_TYPE_ST_RTR MSGTYPE_STANDARD | MSGTYPE_RTR 

/*********functions which permit to communicate with the board****************/
UNS8 canReceive(CAN_HANDLE fd0, Message *m)
{
  UNS8 data; 
  TPCANMsg peakMsg;
  if ((errno = CAN_Read(& peakMsg))) {		// Blocks until no new message or error.
    perror("!!! Peak board : error of reading. (from f_can_receive function) \n");
    return 1;
  }
  m->cob_id.w = peakMsg.ID;   
  if (peakMsg.MSGTYPE == CAN_INIT_TYPE_ST)         	/* bits of MSGTYPE_*/
    m->rtr = 0;
  else 
    m->rtr = 1;
  m->len = peakMsg.LEN;					/* count of data bytes (0..8) */
  for(data = 0  ; data < peakMsg.LEN ; data++)             			
    m->data[data] = peakMsg.DATA[data];         	/* data bytes, up to 8 */
  
  return 0;
}

void canReceiveLoop(CAN_HANDLE fd0)
{
	CO_Data* d = ((CANPort*)fd0)->d;
	Message m;
	while (1) {
		if(!canReceive(fd0, &m))
		{
			EnterMutex();
			canDispatch(d, &m);
			LeaveMutex();
		}else{
//			printf("canReceive returned error\n");
			break;
		}
	}
}

/***************************************************************************/
UNS8 canSend(CAN_HANDLE fd0, Message *m)
{
  UNS8 data;
  TPCANMsg peakMsg;
  peakMsg.ID=m -> cob_id.w;              			/* 11/29 bit code */
  if(m->rtr == 0)	
    peakMsg.MSGTYPE = CAN_INIT_TYPE_ST;       /* bits of MSGTYPE_*/
  else {
    peakMsg.MSGTYPE = CAN_INIT_TYPE_ST_RTR;       /* bits of MSGTYPE_*/
  }
  peakMsg.LEN = m->len;   
          			/* count of data bytes (0..8) */
  for(data = 0 ; data <  m->len; data ++)
  	peakMsg.DATA[data] = m->data[data];         	/* data bytes, up to 8 */
  
  if((errno = CAN_Write(& peakMsg))) {
    perror("!!! Peak board : error of writing. (from canSend function) \n");
    return 1;
  }
  return 0;

}

/***************************************************************************/
CAN_HANDLE canOpen(s_BOARD *board)
{
  HANDLE fd0 = NULL;
  char busname[64];
  char* pEnd;
  int i;  
  
  for(i=0; i < MAX_NB_CAN_PORTS; i++)
  {
  	if(!canports[i].used)
	  	break;
  }
  if(canports[i].used)
  {
  	perror("can_peak_win32.c: no more can port available with this pcan library\n");
  	perror("can_peak_win32.c: please link another executable with another pcan lib\n");
  	return NULL;
  }
//  if(strtol(board->busname, &pEnd,0) >= 0)
//  {
//    sprintf(busname,"/dev/pcan%s",board->busname);
//    fd0 = LINUX_CAN_Open(busname, O_RDWR);
//  }

  if (i==MAX_NB_CAN_PORTS || fd0 == NULL)
    {
      fprintf (stderr, "Open failed.\n");
      return (CAN_HANDLE)NULL;
    }

   CAN_Init(board->baudrate, CAN_INIT_TYPE_ST);

   canports[i].used = 1;

   canports[i].d = board->d;
   CreateReceiveTask((CANPort*) &canports[i], &canports[i].receiveTask);

   return (CANPort*) &canports[i];
}

/***************************************************************************/
int canClose(CAN_HANDLE fd0)
{
  CAN_Close();
  ((CANPort*)fd0)->used = 0;
  WaitReceiveTaskEnd(&((CANPort*)fd0)->receiveTask);
  return 0;
}
