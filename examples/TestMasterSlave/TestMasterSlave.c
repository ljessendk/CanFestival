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
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include <applicfg.h>
#include <can_driver.h>
#include <timers_driver.h>

#include "Master.h"
#include "Slave.h"
#include "TestMasterSlave.h"

#define MyCase(fc) case fc: eprintf(#fc);break;
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
	eprintf(" rtr:%d", m->rtr);
	eprintf(" len:%d", m->len);
	for (i = 0 ; i < m->len ; i++)
		eprintf(" %02x", m->data[i]);
	eprintf("\n");
}

UNS32 OnMasterMap1Update(CO_Data* d, const indextable * unsused_indextable, UNS8 unsused_bSubindex)
{
	eprintf("OnSlaveMap1Update:%d\n", SlaveMap1);
	return 0;
}



void InitNodes(CO_Data* d, UNS32 id)
{
//  TestSlave_Index2000_callbacks[0] = &OnMasterMap1Update;
  RegisterSetODentryCallBack(&TestMaster_Data, 0x2000, 0, &OnMasterMap1Update);

  /****************************** INITIALISATION MASTER *******************************/
  /* Defining the node Id */
  setNodeId(&TestMaster_Data, 0x01);

  /* init */
  setState(&TestMaster_Data, Initialisation);

  /****************************** INITIALISATION SLAVE *******************************/
  /* Defining the node Id */
  setNodeId(&TestSlave_Data, 0x02);

  /* init */
  setState(&TestSlave_Data, Initialisation);

  /****************************** START *******************************/
  /* Put the master in operational mode */
  setState(&TestMaster_Data, Operational);
  
  masterSendNMTstateChange (&TestMaster_Data, 0x02, NMT_Start_Node);
	
}

CAN_HANDLE SlaveCanHandle;
CAN_HANDLE MasterCanHandle;

// Baudrate values for Peak board :
// CAN_BAUD_1M CAN_BAUD_500K CAN_BAUD_250K CAN_BAUD_125K CAN_BAUD_100K CAN_BAUD_50K
// CAN_BAUD_20K CAN_BAUD_10K CAN_BAUD_5K

#ifdef CAN_BAUD_500K
// Appli have been compiled for Peak. Baudrate is defined
# define BAUDRATE CAN_BAUD_500K
#else
// Appli have been compiled for Generic. Baudrate not used
# define BAUDRATE 0
#endif

void catch_signal(int sig)
{
  signal(SIGTERM, catch_signal);
  signal(SIGINT, catch_signal);
  eprintf("Got Signal %d\n",sig);
}

void help()
{
  printf("**************************************************************\n");
  printf("*  TestMasterSlave                                           *\n");
  printf("*                                                            *\n");
  printf("*  A simple example for PC. It does implement 2 CanOpen      *\n");
  printf("*  nodes in the same process. A master and a slave. Both     *\n");
  printf("*   communicate together, exchanging periodically NMT, SYNC, *\n");
  printf("*   SDO and PDO.                                             *\n");
  printf("*                                                            *\n");
  printf("*   If you have chosen virtual CAN driver, just type         *\n");
  printf("*   ./TestMasterSlave                                        *\n");
  printf("*                                                            *\n");
  printf("*   Else you need to specify bus:                            *\n");
  printf("*                                                            *\n");
  printf("*     -s : slave CAN bus [default 0, peak first PCI]         *\n");
  printf("*     -m : master CAN bus [default 1, peak second PCI]       *\n");
  printf("*                                                            *\n");
  printf("**************************************************************\n");
}

/****************************************************************************/
/***************************  MAIN  *****************************************/
/****************************************************************************/
int main(int argc,char **argv)
{
	s_BOARD SlaveBoard = {"0", BAUDRATE, &TestSlave_Data};
	s_BOARD MasterBoard = {"1", BAUDRATE, &TestMaster_Data};


  char c;
  extern char *optarg;

  while ((c = getopt(argc, argv, "-m:s:")) != EOF)
  {
    switch(c)
    {
      case 's' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        SlaveBoard.busname = optarg;
        break;
      case 'm' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        MasterBoard.busname = optarg;
        break;
      default:
        help();
        exit(1);
    }
  }

	/* install signal handler for manual break */
	signal(SIGTERM, catch_signal);
	signal(SIGINT, catch_signal);
	
	// Open CAN devices
	SlaveCanHandle = canOpen(&SlaveBoard);
	MasterCanHandle = canOpen(&MasterBoard);	
	
	// Start timer thread
	StartTimerLoop(&InitNodes);

	// wait Ctrl-C
	pause();
	eprintf("Finishing.\n");
	
	// Stop timer thread
	StopTimerLoop();
	
	// Close CAN devices (and can threads)
	canClose(SlaveCanHandle);
	canClose(MasterCanHandle);	
	

  return 0;
}
