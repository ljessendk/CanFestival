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


#if defined(WIN32) && !defined(__CYGWIN__)
	#include <windows.h>
	#define CLEARSCREEN "cls"
	#define SLEEP(time) Sleep(time * 1000)
#else
	#include <unistd.h>
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <signal.h>
	#define CLEARSCREEN "clear"
	#define SLEEP(time) sleep(time)
#endif

//****************************************************************************
// INCLUDES
#include "canfestival.h"
#include "CANOpenShell.h"
#include "CANOpenShellMasterOD.h"
#include "CANOpenShellSlaveOD.h"

//****************************************************************************
// DEFINES
#define MAX_NODES 127
#define cst_str4(c1, c2, c3, c4) ((((unsigned int)0 | \
                                    (char)c4 << 8) | \
                                   (char)c3) << 8 | \
                                  (char)c2) << 8 | \
                                 (char)c1

#define INIT_ERR 2
#define QUIT 1

//****************************************************************************
// GLOBALS
char BoardBusName[11];
char BoardBaudRate[5];
s_BOARD Board = {BoardBusName, BoardBaudRate};
CO_Data* CANOpenShellOD_Data;
int init_step = 0;
char LibraryPath[512];

/*****************************************************************************/
#if !defined(WIN32) || defined(__CYGWIN__)
void catch_signal(int sig)
{
	signal(SIGTERM, catch_signal);
	signal(SIGINT, catch_signal);
	printf("Got Signal %d\n",sig);
}
#endif

/* Sleep for n seconds */
void SleepFunction(int second)
{
	SLEEP(second);
}

/* Ask a slave node to go in operational mode */
void StartNode(CO_Data* d, UNS8 nodeid)
{
	EnterMutex();
	masterSendNMTstateChange(d, nodeid, NMT_Start_Node);
	LeaveMutex();
}

/* Ask a slave node to go in pre-operational mode */
void StopNode(CO_Data* d, UNS8 nodeid)
{
	EnterMutex();
	masterSendNMTstateChange(d, nodeid, NMT_Stop_Node);
	LeaveMutex();
}

/* Ask a slave node to reset */
void ResetNode(CO_Data* d, UNS8 nodeid)
{
	EnterMutex();
	masterSendNMTstateChange(d, nodeid, NMT_Reset_Node);
	LeaveMutex();
}

/* Reset all nodes on the network and print message when boot-up*/
void DiscoverNodes(CO_Data* d)
{
	printf("Wait for Slave nodes bootup...\n\n");
	ResetNode(CANOpenShellOD_Data, 0x0);
}

/* Callback function that check the read SDO demand */
void CheckReadInfoSDO(CO_Data* d, UNS8 nodeid)
{
	UNS32 abortCode;
	UNS32 data;
	UNS32 size=64;

	if(getReadResultNetworkDict(d, nodeid, &data, &size, &abortCode) != SDO_FINISHED)
		printf("Master : Failed in getting information for slave %2.2x, AbortCode :%4.4x \n", nodeid, abortCode);

	/* Finalise last SDO transfer with this node */
	closeSDOtransfer(CANOpenShellOD_Data, nodeid, SDO_CLIENT);

	/* Display data received */
	switch(init_step)
	{
		case 1:
				printf("Device type     : %x\n", data);
				break;
		case 2:
				printf("Vendor ID       : %x\n", data);
				break;
		case 3:
				printf("Product Code    : %x\n", data);
				break;
		case 4:
				printf("Revision Number : %x\n", data);
				break;
	}
	GetSlaveNodeInfo(d, nodeid);
}

/* Retrieve node informations located at index 0x1000 (Device Type) and 0x1018 (Identity) */
void GetSlaveNodeInfo(CO_Data* d, UNS8 nodeid)
{
		switch(++init_step)
		{
			case 1: /* Get device type */
				printf("##################################\n");
				printf("#### Informations for node %x ####\n", nodeid);
				printf("##################################\n");
				readNetworkDictCallback(d, nodeid, 0x1000, 0x00, 0, CheckReadInfoSDO);
				break;

			case 2: /* Get Vendor ID */
				readNetworkDictCallback(d, nodeid, 0x1018, 0x01, 0, CheckReadInfoSDO);
				break;

			case 3: /* Get Product Code */
				readNetworkDictCallback(d, nodeid, 0x1018, 0x02, 0, CheckReadInfoSDO);
				break;

			case 4: /* Get Revision Number */
				readNetworkDictCallback(d, nodeid, 0x1018, 0x03, 0, CheckReadInfoSDO);
				break;

			case 5: /* Print node info */
				init_step = 0;
		}
}

/* Callback function that check the read SDO demand */
void CheckReadSDO(CO_Data* d, UNS8 nodeid)
{
	UNS32 abortCode;
	UNS32 data;
	UNS32 size=64;

	if(getReadResultNetworkDict(d, nodeid, &data, &size, &abortCode) != SDO_FINISHED)
		printf("\nResult : Failed in getting information for slave %2.2x, AbortCode :%4.4x \n", nodeid, abortCode);
	else
		printf("\nResult : = %x\n", data);

	/* Finalise last SDO transfer with this node */
	closeSDOtransfer(CANOpenShellOD_Data, nodeid, SDO_CLIENT);
}

/* Read a slave node object dictionary entry */
void ReadDeviceEntry(CO_Data* d, char* sdo)
{
	int nodeid;
	int index;
	int subindex;
	int size;
	int datatype = 0;

	sscanf(sdo, "%2x%4x%2x", &nodeid, &index, &subindex);

	printf("##################################\n");
	printf("#### Read SDO                 ####\n");
	printf("##################################\n");
	printf("NodeId   : %2.2x\n", nodeid);
	printf("Index    : %4.4x\n", index);
	printf("SubIndex : %2.2x\n", subindex);

	readNetworkDictCallback(d, (UNS8)nodeid, (UNS16)index, (UNS8)subindex, (UNS8)datatype, CheckReadSDO);
}

/* Callback function that check the write SDO demand */
void CheckWriteSDO(CO_Data* d, UNS8 nodeid)
{
	UNS32 abortCode;

	if(getWriteResultNetworkDict(d, nodeid, &abortCode) != SDO_FINISHED)
		printf("\nResult : Failed in getting information for slave %2.2x, AbortCode :%4.4x \n", nodeid, abortCode);
	else
		printf("\nSend data OK\n");

	/* Finalise last SDO transfer with this node */
	closeSDOtransfer(CANOpenShellOD_Data, nodeid, SDO_CLIENT);
}

/* Write a slave node object dictionnary entry */
void WriteDeviceEntry(CO_Data* d, char* sdo)
{
	int nodeid;
	int index;
	int subindex;
	int size;
	int data;

	sscanf(sdo, "%2x%4x%2x%2x%4x", &nodeid , &index, &subindex, &size, &data);

	printf("##################################\n");
	printf("#### Write SDO                ####\n");
	printf("##################################\n");
	printf("NodeId   : %2.2x\n", nodeid);
	printf("Index    : %4.4x\n", index);
	printf("SubIndex : %2.2x\n", subindex);
	printf("Size     : %2.2x\n", size);
	printf("Data     : %x\n", data);

	writeNetworkDictCallBackAI(d, nodeid, index, subindex, size, 0, &data, CheckWriteSDO, 1);
}

void CANOpenShellOD_post_SlaveBootup(CO_Data* d, UNS8 nodeid)
{
	printf("Slave %x boot up\n", nodeid);
}

/***************************  CALLBACK FUNCTIONS  *****************************************/
void CANOpenShellOD_initialisation(CO_Data* d)
{
	//printf("Master_initialisation\n");
}

void CANOpenShellOD_preOperational(CO_Data* d)
{
	//printf("Master_preOperational\n");
}

void CANOpenShellOD_operational(CO_Data* d)
{
	//printf("Master_operational\n");
}

void CANOpenShellOD_stopped(CO_Data* d)
{
	//printf("Master_stopped\n");
}

void CANOpenShellOD_post_sync(CO_Data* d)
{
	//printf("Master_post_sync\n");
}

void CANOpenShellOD_post_TPDO(CO_Data* d)
{
	//printf("Master_post_TPDO\n");
}

/***************************  MASTER INITIALISATION **********************************/
void Init(CO_Data* d, UNS32 nodeid)
{
	if(Board.baudrate)
	{
		/* Defining the node Id */
		setNodeId(CANOpenShellOD_Data, nodeid);

		/* Init */
		setState(CANOpenShellOD_Data, Initialisation);
	}
}

/***************************  MASTER CLEANUP  *****************************************/
void Exit(CO_Data* d, UNS32 nodeid)
{
	if(strcmp(Board.baudrate, "none"))
	{
		/* Reset all nodes on the network */
		masterSendNMTstateChange(CANOpenShellOD_Data, (UNS8)nodeid, NMT_Reset_Node);

		/* Stop master */
		setState(CANOpenShellOD_Data, Stopped);
	}
}

int ExtractNodeId(char *command) {
	int nodeid;
	sscanf(command, "%2x", &nodeid);
	return nodeid;
}

int NodeInit(CO_Data* d, int DeviceNodeID, int DeviceIsMaster)
{
	if(DeviceIsMaster)
	{
		CANOpenShellOD_Data = &CANOpenShellMasterOD_Data;
	}
	else
	{
		CANOpenShellOD_Data = &CANOpenShellSlaveOD_Data;
	}

	/* Load can library */
	LoadCanDriver(LibraryPath);

	/* Init stack timer */
	TimerInit();

	/* Define callback functions */
	CANOpenShellOD_Data->initialisation = CANOpenShellOD_initialisation;
	CANOpenShellOD_Data->preOperational = CANOpenShellOD_preOperational;
	CANOpenShellOD_Data->operational = CANOpenShellOD_operational;
	CANOpenShellOD_Data->stopped = CANOpenShellOD_stopped;
	CANOpenShellOD_Data->post_sync = CANOpenShellOD_post_sync;
	CANOpenShellOD_Data->post_TPDO = CANOpenShellOD_post_TPDO;
	CANOpenShellOD_Data->post_SlaveBootup=CANOpenShellOD_post_SlaveBootup;

	/* Open the Peak CANOpen device */
	if(!canOpen(&Board,CANOpenShellOD_Data)) return 1;

	/* Start Timer thread */
	StartTimerLoop(&Init);
	return 0;
}

void help_menu(void)
{
	printf("   MANDATORY COMMAND (must be the first command):\n");
	printf("      load#Can library path,channel,baudrate,device nodeid,device type (m for master, s for slave)\n");
	printf("\n");
	printf("   NETWORK: (if nodeid=0x00 : broadcast)\n");
	printf("     ssta#[nodeid] : Start a node\n");
	printf("     ssto#[nodeid] : Stop a node\n");
	printf("     srst#[nodeid] : Reset a node\n");
	printf("     scan : Reset all nodes and print message when bootup\n");
	printf("     wait#[seconds] : Sleep for n seconds\n");
	printf("\n");
	printf("   SDO: (size parameter : nb BYTES)\n");
	printf("     info#[nodeid]\n");
	printf("     rsdo#[nodeid][index][subindex] : read sdo\n");
	printf("     wsdo#[nodeid][index][subindex][size][data] : write sdo\n");
	printf("\n");
	printf("     help : Display this menu\n");
	printf("     quit : Quit application\n");
	printf("\n");
	printf("\n");
}

int ProcessCommand(char* command)
{
	int ret = 0;
	int sec = 0;
	int DeviceNodeID;
	int DeviceType;

	switch(cst_str4(command[0], command[1], command[2], command[3]))
	{
		case cst_str4('l', 'o', 'a', 'd') : /* Library Interface*/
					ret = sscanf(command, "load#%100[^,],%10[^,],%4[^,],%d,%d",
							LibraryPath,
							BoardBusName,
							BoardBaudRate,
							&DeviceNodeID,
							&DeviceType);

					if(ret == 5)
					{
						ret = NodeInit(CANOpenShellOD_Data, DeviceNodeID, DeviceType);
						return ret;
					}
					else{
						help_menu();
						exit(1);
					}
					break;
		case cst_str4('h', 'e', 'l', 'p') : /* Display Help*/
					help_menu();
					break;
		case cst_str4('s', 's', 't', 'a') : /* Slave Start*/
					StartNode(CANOpenShellOD_Data, ExtractNodeId(command + 5));
					break;
		case cst_str4('s', 's', 't', 'o') : /* Slave Stop */
					StopNode(CANOpenShellOD_Data, ExtractNodeId(command + 5));
					break;
		case cst_str4('s', 'r', 's', 't') : /* Slave Reset */
					ResetNode(CANOpenShellOD_Data, ExtractNodeId(command + 5));
					break;
		case cst_str4('i', 'n', 'f', 'o') : /* Retrieve node informations */
					GetSlaveNodeInfo(CANOpenShellOD_Data, ExtractNodeId(command + 5));
					break;
		case cst_str4('r', 's', 'd', 'o') : /* Read device entry */
					ReadDeviceEntry(CANOpenShellOD_Data, command + 5);
					break;
		case cst_str4('w', 's', 'd', 'o') : /* Write device entry */
					WriteDeviceEntry(CANOpenShellOD_Data, command + 5);
					break;
		case cst_str4('s', 'c', 'a', 'n') : /* Display master node state */
					DiscoverNodes(CANOpenShellOD_Data);
					break;
		case cst_str4('w', 'a', 'i', 't') : /* Display master node state */
					ret = sscanf(command, "wait=%d", &sec);
					if(ret == 1) SleepFunction(sec);
					break;
		case cst_str4('q', 'u', 'i', 't') : /* Quit application */
					return QUIT;
		default :
					help_menu();
	}
	return 0;
}

/****************************************************************************/
/***************************  MAIN  *****************************************/
/****************************************************************************/

int main(int argc, char** argv)
{
	extern char *optarg;
	char command[20];
	char* res;
	int ret=0;
	int sysret=0;
	int i=0;

	/* Print help and exit immediatly*/
	if(argc < 1)
	{
		help_menu();
		exit(1);
	}

	/* Strip command-line*/
	for(i=1 ; i<argc ; i++)
	{
		if(ProcessCommand(argv[i]) == INIT_ERR) goto init_fail;
	}

	#if !defined(WIN32) || defined(__CYGWIN__)
		/* install signal handler for manual break */
		signal(SIGTERM, catch_signal);
		signal(SIGINT, catch_signal);
	#endif

	/* Enter in a loop to read stdin command until "quit" is called */
	while(ret != QUIT)
	{
		// wait on stdin for string command
		res = fgets(command, sizeof(command), stdin);
		sysret = system(CLEARSCREEN);
		ret = ProcessCommand(command);
		fflush(stdout);
	}

	printf("Finishing.\n");

	// Stop timer thread
	StopTimerLoop(&Exit);

init_fail:
	TimerCleanup();
	return 0;
}

