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

CAN_HANDLE SlaveCanHandle;
CAN_HANDLE MasterCanHandle;

// Baudrate values for Peak board :
// CAN_BAUD_1M CAN_BAUD_500K CAN_BAUD_250K CAN_BAUD_125K CAN_BAUD_100K CAN_BAUD_50K
// CAN_BAUD_20K CAN_BAUD_10K CAN_BAUD_5K

#ifdef CAN_BAUD_500K
int TranslateBaudeRate(char* optarg){
	if(!strcmp( optarg, "1M")) return CAN_BAUD_1M;
	if(!strcmp( optarg, "500K")) return CAN_BAUD_500K;
	if(!strcmp( optarg, "250K")) return CAN_BAUD_250K;
	if(!strcmp( optarg, "125K")) return CAN_BAUD_125K;
	if(!strcmp( optarg, "100K")) return CAN_BAUD_100K;
	if(!strcmp( optarg, "50K")) return CAN_BAUD_50K;
	if(!strcmp( optarg, "20K")) return CAN_BAUD_20K;
	if(!strcmp( optarg, "10K")) return CAN_BAUD_10K;
	if(!strcmp( optarg, "5K")) return CAN_BAUD_5K;
	if(!strcmp( optarg, "none")) return 0;
	return 0x0000;
}
s_BOARD SlaveBoard = {"0", CAN_BAUD_500K, &TestSlave_Data};
s_BOARD MasterBoard = {"1", CAN_BAUD_500K, &TestMaster_Data};
#else
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
}
s_BOARD SlaveBoard = {"0", 500, &TestSlave_Data};
s_BOARD MasterBoard = {"1", 500, &TestMaster_Data};
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
  printf("*  communicate together, exchanging periodically NMT, SYNC,  *\n");
  printf("*  SDO and PDO.                                              *\n");
  printf("*                                                            *\n");
  printf("*   Usage:                                                   *\n");
  printf("*   ./TestMasterSlave  [OPTIONS]                             *\n");
  printf("*                                                            *\n");
  printf("*   OPTIONS:                                                 *\n");
  printf("*    Slave:                                                  *\n");
  printf("*     -s : bus name [\"0\"]                                    *\n");
  printf("*     -S : 1M,500K,250K,125K,100K,50K,20K,10K,none(disable)  *\n");
  printf("*                                                            *\n");
  printf("*    Master:                                                 *\n");
  printf("*     -m : bus name [\"1\"]                                    *\n");
  printf("*     -M : 1M,500K,250K,125K,100K,50K,20K,10K,none(disable)  *\n");
  printf("*                                                            *\n");
  printf("**************************************************************\n");
}

/***************************  INIT  *****************************************/
void InitNodes(CO_Data* d, UNS32 id)
{
	/****************************** INITIALISATION SLAVE *******************************/
	if(SlaveBoard.baudrate) {
		/* Defining the node Id */
		setNodeId(&TestSlave_Data, 0x02);
		/* init */
		setState(&TestSlave_Data, Initialisation);
	}

	/****************************** INITIALISATION MASTER *******************************/
	if(MasterBoard.baudrate){
 		RegisterSetODentryCallBack(&TestMaster_Data, 0x2000, 0, &OnMasterMap1Update);

		/* Defining the node Id */
		setNodeId(&TestMaster_Data, 0x01);

		/* init */
		setState(&TestMaster_Data, Initialisation);

		/****************************** START *******************************/
		/* Put the master in operational mode */
		setState(&TestMaster_Data, Operational);
  
		/* Ask slave node to go in operational mode */
		masterSendNMTstateChange (&TestMaster_Data, 0x02, NMT_Start_Node);
	}
}

/****************************************************************************/
/***************************  MAIN  *****************************************/
/****************************************************************************/
int main(int argc,char **argv)
{

  char c;
  extern char *optarg;

  while ((c = getopt(argc, argv, "-m:s:M:S:")) != EOF)
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
      case 'S' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        SlaveBoard.baudrate = TranslateBaudeRate(optarg);
        break;
      case 'M' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        MasterBoard.baudrate = TranslateBaudeRate(optarg);
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
	if(SlaveBoard.baudrate)
		if((SlaveCanHandle = canOpen(&SlaveBoard))==NULL) goto fail_slave;

	if(MasterBoard.baudrate)
		if((MasterCanHandle = canOpen(&MasterBoard))==NULL) goto fail_master;
	
	// Start timer thread
	StartTimerLoop(&InitNodes);

	// wait Ctrl-C
	pause();
	eprintf("Finishing.\n");
	
	// Stop timer thread
	StopTimerLoop();
	
	// Close CAN devices (and can threads)
	if(SlaveBoard.baudrate) canClose(SlaveCanHandle);
fail_master:
	if(MasterBoard.baudrate) canClose(MasterCanHandle);	
fail_slave:
	

  return 0;
}
