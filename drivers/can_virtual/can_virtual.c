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

#include <stdio.h>
#include <unistd.h>

#include "can_driver.h"
#include "def.h"

#define MAX_NB_CAN_PIPES 16

typedef struct {
  char used;
  int pipe[2];
} CANPipe;

CANPipe canpipes[MAX_NB_CAN_PIPES] = {{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},};

/*********functions which permit to communicate with the board****************/
UNS8 canReceive_driver(CAN_HANDLE fd0, Message *m)
{
	if(read(((CANPipe*)fd0)->pipe[0], m, sizeof(Message)) < sizeof(Message))
	{
		return 1;
	}
	return 0;
}

#define MyCase(fc) case fc: printf(#fc);break;
void print_message(Message *m)
{
	int i;
	switch(m->cob_id.w >> 7)
	{
		MyCase(SYNC)
		MyCase(TIME_STAMP)
		MyCase(PDO1tx)
		MyCase(PDO1rx)
		MyCase(PDO2tx)
		MyCase(PDO2rx)
		MyCase(PDO3tx)
		MyCase(PDO3rx)
		MyCase(PDO4tx)
		MyCase(PDO4rx)
		MyCase(SDOtx)
		MyCase(SDOrx)
		MyCase(NODE_GUARD)
		MyCase(NMT)
	}
	printf(" rtr:%d", m->rtr);
	printf(" len:%d", m->len);
	for (i = 0 ; i < m->len ; i++)
		printf(" %02x", m->data[i]);
	printf("\n");
}

/***************************************************************************/
UNS8 canSend_driver(CAN_HANDLE fd0, Message *m)
{
  int i;
  
  printf("%x->[ ", (CANPipe*)fd0 - &canpipes[0]); 
  for(i=0; i < MAX_NB_CAN_PIPES; i++)
  {
  	if(canpipes[i].used && &canpipes[i] != (CANPipe*)fd0)
  	{
		printf("%x ",i);	
  	}
  }
  printf(" ]");	
  print_message(m);
  
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
/*
int TranslateBaudeRate(char* optarg){
	if(!strcmp( optarg, "1M")) return 1000;
	if(!strcmp( optarg, "500K")) return 500;
	if(!strcmp( optarg, "250K")) return 250;
	if(!strcmp( optarg, "125K")) return 125;
	if(!strcmp( optarg, "100K")) return 100;
	if(!strcmp( optarg, "50K")) return 50;
	if(!strcmp( optarg, "20K")) return 20;
	if(!strcmp( optarg, "10K")) return 10;
	if(!strcmp( optarg, "5K")) return 5;
	if(!strcmp( optarg, "none")) return 0;
	return 0;
}*/
/***************************************************************************/
CAN_HANDLE canOpen_driver(s_BOARD *board)
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
   return (CAN_HANDLE) &canpipes[i];
}

/***************************************************************************/
int canClose_driver(CAN_HANDLE fd0)
{
  close(((CANPipe*)fd0)->pipe[0]);
  close(((CANPipe*)fd0)->pipe[1]);
  ((CANPipe*)fd0)->used = 0;
  return 0;
}


