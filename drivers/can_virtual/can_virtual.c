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

/*
	Virtual CAN driver.
*/

#include <stdlib.h>

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include <applicfg.h>
#include "timer.h"
#include "can_driver.h"
#include "timers_driver.h"

#define MAX_NB_CAN_PIPES 10

typedef struct {
  char used;
  int pipe[2];
  TASK_HANDLE receiveTask;
  CO_Data* d;
} CANPipe;

CANPipe canpipes[MAX_NB_CAN_PIPES] = {{0,{0,0},},};

/*********functions which permit to communicate with the board****************/
UNS8 canReceive(CAN_HANDLE fd0, Message *m)
{
	if(read(((CANPipe*)fd0)->pipe[0], m, sizeof(Message)) < sizeof(Message))
	{
		return 1;
	}
	return 0;
}

void canReceiveLoop(CAN_HANDLE fd0)
{
	CO_Data* d = ((CANPipe*)fd0)->d;
	Message m;
	while (1) {
		if(!canReceive(fd0, &m))
		{  
			EnterMutex();
			canDispatch(d, &m);
			LeaveMutex();
		}else{
			break;
		}
	}
}

/***************************************************************************/
UNS8 canSend(CAN_HANDLE fd0, Message *m)
{
  int i;
  // Send to all readers, except myself
  for(i=0; i < MAX_NB_CAN_PIPES; i++)
  {
  	if(canpipes[i].used && &canpipes[i] != (CANPipe*)fd0)
  	{
		write(canpipes[i].pipe[1], m, sizeof(Message));
  	}
  }
  return 0;
}

/***************************************************************************/
CAN_HANDLE canOpen(s_BOARD *board)
{
  int i;  
  for(i=0; i < MAX_NB_CAN_PIPES; i++)
  {
  	if(!canpipes[i].used)
	  	break;
  }

  /* Create the pipe.  */
  if (i==MAX_NB_CAN_PIPES || pipe(canpipes[i].pipe))
    {
      fprintf (stderr, "Open failed.\n");
      return (CAN_HANDLE)NULL;
    }

   canpipes[i].used = 1;

   canpipes[i].d = board->d;
   CreateReceiveTask((CAN_HANDLE) &canpipes[i], &canpipes[i].receiveTask);

   return (CAN_HANDLE) &canpipes[i];
}

/***************************************************************************/
int canClose(CAN_HANDLE fd0)
{
  close(((CANPipe*)fd0)->pipe[0]);
  close(((CANPipe*)fd0)->pipe[1]);
  ((CANPipe*)fd0)->used = 0;
  WaitReceiveTaskEnd(&((CANPipe*)fd0)->receiveTask);
}


