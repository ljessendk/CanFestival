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

#include "Master.h"
#include "Slave.h"
#include "TestMasterSlave.h"

/*****************************************************************************/
void TestMaster_heartbeatError(CO_Data* d, UNS8 heartbeatID)
{
	eprintf("TestMaster_heartbeatError %d\n", heartbeatID);
}

/********************************************************
 * ConfigureSlaveNode is responsible to
 *  - setup master RPDO 1 to receive TPDO 1 from id 2
 *  - setup master RPDO 2 to receive TPDO 2 from id 2
 ********************************************************/
void TestMaster_initialisation(CO_Data* d)
{
	UNS32 PDO1_COBID = 0x0182; 
	UNS32 PDO2_COBID = 0x0282;
	UNS8 size = sizeof(UNS32); 
	UNS32 SINC_cicle=0;
	UNS8 data_type = 0;
	
	eprintf("TestMaster_initialisation\n");

	/*****************************************
	 * Define RPDOs to match slave ID=2 TPDOs*
	 *****************************************/
	writeLocalDict( &TestMaster_Data, /*CO_Data* d*/
			0x1400, /*UNS16 index*/
			0x01, /*UNS8 subind*/ 
			&PDO1_COBID, /*void * pSourceData,*/ 
			&size, /* UNS8 * pExpectedSize*/
			RW);  /* UNS8 checkAccess */
			
	writeLocalDict( &TestMaster_Data, /*CO_Data* d*/
			0x1401, /*UNS16 index*/
			0x01, /*UNS8 subind*/ 
			&PDO2_COBID, /*void * pSourceData,*/ 
			&size, /* UNS8 * pExpectedSize*/
			RW);  /* UNS8 checkAccess */
			
}

// Step counts number of times ConfigureSlaveNode is called
static init_step = 0;

/*Froward declaration*/
static void ConfigureSlaveNode(CO_Data* d, UNS8 nodeId);

/**/
static void CheckSDOAndContinue(CO_Data* d, UNS8 nodeId)
{
	UNS32 abortCode;	
	if(getWriteResultNetworkDict (d, nodeId, &abortCode) != SDO_FINISHED)
		eprintf("Master : Failed in initializing slave %2.2x, step %d, AbortCode :%4.4x \n", nodeId, init_step, abortCode);

	/* Finalise last SDO transfer with this node */
	closeSDOtransfer(&TestMaster_Data, nodeId, SDO_CLIENT);

	ConfigureSlaveNode(d, nodeId);
}

/********************************************************
 * ConfigureSlaveNode is responsible to
 *  - setup slave TPDO 1 transmit time
 *  - setup slave TPDO 2 transmit time
 *  - switch to operational mode
 *  - send NMT to slave
 ********************************************************
 * This an example of :
 * Network Dictionary Access (SDO) with Callback 
 * Slave node state change request (NMT) 
 ********************************************************
 * This is called first by TestMaster_preOperational
 * then it called again each time a SDO exchange is
 * finished.
 ********************************************************/
 
static void ConfigureSlaveNode(CO_Data* d, UNS8 nodeId)
{
	/* Master configure heartbeat producer time at 1000 ms 
	 * for slave node-id 0x02 by DCF concise */
	 
	UNS8 Transmission_Type = 0x01;
	UNS32 abortCode;
	UNS8 res;
	eprintf("Master : ConfigureSlaveNode %2.2x\n", nodeId);

	switch(++init_step){
		case 1: /*First step : setup Slave's TPDO 1 to be transmitted on SYNC*/
			eprintf("Master : set slave %2.2x TPDO 1 transmit type\n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					nodeId, /*UNS8 nodeId*/
					0x1800, /*UNS16 index*/
					0x02, /*UNS8 subindex*/
					1, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&Transmission_Type,/*void *data*/
					CheckSDOAndContinue); /*SDOCallback_t Callback*/
					break;
		
		case 2:	/*Second step*/
			eprintf("Master : set slave %2.2x TPDO 2 transmit type\n", nodeId);
			writeNetworkDictCallBack (d, /*CO_Data* d*/
					nodeId, /*UNS8 nodeId*/
					0x1801, /*UNS16 index*/
					0x02, /*UNS16 index*/
					1, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&Transmission_Type,/*void *data*/
					CheckSDOAndContinue); /*SDOCallback_t Callback*/
					break;
		case 3: 
		
		/****************************** START *******************************/
		
			/* Put the master in operational mode */
			setState(d, Operational);
		 
			/* Ask slave node to go in operational mode */
			masterSendNMTstateChange (d, nodeId, NMT_Start_Node);
			
	}
}

#ifdef CO_ENABLE_LSS
static void ConfigureLSSNode(CO_Data* d);
// Step counts number of times ConfigureLSSNode is called
UNS8 init_step_LSS=1;

static void CheckLSSAndContinue(CO_Data* d, UNS8 command)
{
	UNS32 dat1;
	UNS8 dat2;
	printf("CheckLSS->");
	if(getConfigResultNetworkNode (d, command, &dat1, &dat2) != LSS_FINISHED){
		if(command==LSS_IDENT_REMOTE_NON_CONF){
			eprintf("Master : There are not no-configured slaves in the net\n", command);
			return;
		}
		else{
			eprintf("Master : Failed in LSS comand %d.  Trying again\n", command);
		}
	}
	else
	{
		init_step_LSS++;
	
		switch(command){
		case LSS_CONF_NODE_ID:
   			switch(dat1){
   				case 0: printf("Node ID change succesful\n");break;
   				case 1: printf("Node ID change error:out of range\n");break;
   				case 0xFF:printf("Node ID change error:specific error\n");break;
   				default:break;
   			}
   			break;
   		case LSS_CONF_BIT_TIMING:
   			switch(dat1){
   				case 0: printf("Baud rate change succesful\n");break;
   				case 1: printf("Baud rate change error: change baud rate not supported\n");break;
   				case 0xFF:printf("Baud rate change error:specific error\n");break;
   				default:break;
   			}
   			break;
   		case LSS_CONF_STORE:
   			switch(dat1){
   				case 0: printf("Store configuration succesful\n");break;
   				case 1: printf("Store configuration error:not supported\n");break;
   				case 0xFF:printf("Store configuration error:specific error\n");break;
   				default:break;
   			}
   			break;
		case LSS_SM_SELECTIVE_SERIAL:
   			printf("Slave in CONFIGURATION mode\n");
   			break;
   		case LSS_IDENT_REMOTE_SERIAL_HIGH:
   			printf("node identified\n");
   			break;
   		case LSS_IDENT_REMOTE_NON_CONF:
   			printf("non-configured remote slave in the net\n");
   			break;
   		case LSS_INQ_VENDOR_ID:
   			printf("Slave VendorID %x\n", dat1);
   			break;
   		case LSS_INQ_PRODUCT_CODE:
   			printf("Slave Product Code %x\n", dat1);
   			break;
   		case LSS_INQ_REV_NUMBER:
   			printf("Slave Revision Number %x\n", dat1);
   			break;
   		case LSS_INQ_SERIAL_NUMBER:
   			printf("Slave Serial Number %x\n", dat1);
   			break;
   		case LSS_INQ_NODE_ID:
   			printf("Slave nodeid %x\n", dat1);
   			break;
#ifdef CO_ENABLE_LSS_FS
   		case LSS_IDENT_FASTSCAN:
   			if(dat1==0)
   				printf("Slave node identified with FastScan\n");
   			else
   			{
   				printf("There is not unconfigured node in the net\n");
   				return;
   			}	
   			init_step_LSS++;
   			break;
#endif		
		}
	}

	printf("\n");
	ConfigureLSSNode(d);
}


/* First ask if there is a node with an invalid nodeID.
 * If FastScan is activated it is used to put the node in the state â€œconfigurationâ€?.
 * If FastScan is not activated, identification services are used to identify the node.
 * Then  switch mode service is used to put it in configuration state.
 * Next all the inquire and configuration services are used.
 * Finally, the node LSS state is restored to â€œwaitingâ€? and all the process is repeated 
 * again until there isn't any node with a invalid nodeID.
 * */
static void ConfigureLSSNode(CO_Data* d)
{
	UNS32 Vendor_ID=0x12345678;
	UNS32 Product_Code=0x90123456;
	UNS32 Revision_Number=0x78901234;
	UNS32 Serial_Number=0x56789012;
	UNS32 Revision_Number_high=0x78901240;
	UNS32 Revision_Number_low=0x78901230;
	UNS32 Serial_Number_high=0x56789020;
	UNS32 Serial_Number_low=0x56789010;
	UNS8 NodeID=0x02;
	UNS8 Baud_Table=0;
	UNS8 Baud_BitTiming=3;
	UNS16 Switch_delay=1;
	UNS8 LSS_mode=LSS_WAITING_MODE;
	UNS8 res;
	eprintf("ConfigureLSSNode -> ",0);

	switch(init_step_LSS){
		case 1:	/* LSS=>identify non-configured remote slave */
			eprintf("LSS=>identify non-configured remote slave\n");
			res=configNetworkNodeCallBack(&TestMaster_Data,LSS_IDENT_REMOTE_NON_CONF,0,0,CheckLSSAndContinue);
			break;
#ifdef CO_ENABLE_LSS_FS
		case 2:	/* LSS=>FastScan */
			eprintf("LSS=>FastScan\n");
			res=configNetworkNodeCallBack(&TestMaster_Data,LSS_IDENT_FASTSCAN,0,0,CheckLSSAndContinue);
			break;
#else
		case 2:	/* LSS=>identify node */
			eprintf("LSS=>identify node\n");
			res=configNetworkNode(&TestMaster_Data,LSS_IDENT_REMOTE_VENDOR,&Vendor_ID,0);
			res=configNetworkNode(&TestMaster_Data,LSS_IDENT_REMOTE_PRODUCT,&Product_Code,0);
			res=configNetworkNode(&TestMaster_Data,LSS_IDENT_REMOTE_REV_LOW,&Revision_Number_low,0);
			res=configNetworkNode(&TestMaster_Data,LSS_IDENT_REMOTE_REV_HIGH,&Revision_Number_high,0);
			res=configNetworkNode(&TestMaster_Data,LSS_IDENT_REMOTE_SERIAL_LOW,&Serial_Number_low,0);
			res=configNetworkNodeCallBack(&TestMaster_Data,LSS_IDENT_REMOTE_SERIAL_HIGH,&Serial_Number_high,0,CheckLSSAndContinue);
			break;
		case 3: /*LSS=>put in configuration mode*/
			eprintf("LSS=>put in configuration mode\n");
			res=configNetworkNode(&TestMaster_Data,LSS_SM_SELECTIVE_VENDOR,&Vendor_ID,0);
			res=configNetworkNode(&TestMaster_Data,LSS_SM_SELECTIVE_PRODUCT,&Product_Code,0);
			res=configNetworkNode(&TestMaster_Data,LSS_SM_SELECTIVE_REVISION,&Revision_Number,0);
			res=configNetworkNodeCallBack(&TestMaster_Data,LSS_SM_SELECTIVE_SERIAL,&Serial_Number,0,CheckLSSAndContinue);
			break;
#endif
		case 4:	/* LSS=>inquire nodeID */
			eprintf("LSS=>inquire nodeID\n");
			res=configNetworkNodeCallBack(&TestMaster_Data,LSS_INQ_NODE_ID,0,0,CheckLSSAndContinue);
			break;
		case 5:	/* LSS=>inquire VendorID */
			eprintf("LSS=>inquire VendorID\n");
			res=configNetworkNodeCallBack(&TestMaster_Data,LSS_INQ_VENDOR_ID,0,0,CheckLSSAndContinue);
			break;
		case 6:	/* LSS=>inquire Product code */
			eprintf("LSS=>inquire Product code\n");
			res=configNetworkNodeCallBack(&TestMaster_Data,LSS_INQ_PRODUCT_CODE,0,0,CheckLSSAndContinue);
			break;
		case 7:	/* LSS=>inquire Revision Number */
			eprintf("LSS=>inquire Revision Number\n");
			res=configNetworkNodeCallBack(&TestMaster_Data,LSS_INQ_REV_NUMBER,0,0,CheckLSSAndContinue);
			break;
		case 8:	/* LSS=>inquire Serial Number */
			eprintf("LSS=>inquire Serial Number\n");
			res=configNetworkNodeCallBack(&TestMaster_Data,LSS_INQ_SERIAL_NUMBER,0,0,CheckLSSAndContinue);
			break;
		case 9:	/* LSS=>change the nodeID */
			eprintf("LSS=>change the nodeId\n");
			res=configNetworkNodeCallBack(&TestMaster_Data,LSS_CONF_NODE_ID,&NodeID,0,CheckLSSAndContinue);
			break;
		case 10:	/* LSS=>change the Baud rate */
			eprintf("LSS=>change the Baud rate\n");
			res=configNetworkNodeCallBack(&TestMaster_Data,LSS_CONF_BIT_TIMING,&Baud_Table,&Baud_BitTiming,CheckLSSAndContinue);
			break;
		case 11:
			eprintf("LSS=>Activate Bit Timing\n");
			res=configNetworkNode(&TestMaster_Data,LSS_CONF_ACT_BIT_TIMING,&Switch_delay,0);
			/*no break;*/
			init_step_LSS++;
		case 12:
			/*LSS=>store configuration*/
			/* It will fail the first time (time out) due to the switch delay */
			/* It will fail the second time because it is not implemented in the slave */
			eprintf("LSS=>store configuration\n");
			res=configNetworkNodeCallBack(&TestMaster_Data,LSS_CONF_STORE,0,0,CheckLSSAndContinue);
			break;
		case 13: /* LSS=>put in operation mod */
			eprintf("LSS=>put in operation mode\n");
			res=configNetworkNode(&TestMaster_Data,LSS_SM_GLOBAL,&LSS_mode,0);
			/* Search again for not-configured slaves*/
			eprintf("LSS=>identify not-configured remote slave\n");
			res=configNetworkNodeCallBack(&TestMaster_Data,LSS_IDENT_REMOTE_NON_CONF,0,0,CheckLSSAndContinue);
			init_step_LSS=1;
			break;
	}
}
#endif

void TestMaster_preOperational(CO_Data* d)
{
	eprintf("TestMaster_preOperational\n");
#ifdef CO_ENABLE_LSS
	/* Ask slave node to go in stop mode */
	masterSendNMTstateChange (&TestMaster_Data, 0, NMT_Stop_Node);
	ConfigureLSSNode(&TestMaster_Data);
#endif
}

void TestMaster_operational(CO_Data* d)
{
	eprintf("TestMaster_operational\n");
}

void TestMaster_stopped(CO_Data* d)
{
	eprintf("TestMaster_stopped\n");
}

void TestMaster_post_sync(CO_Data* d)
{
	eprintf("TestMaster_post_sync\n");
	eprintf("Master: %d %d %d %d %d %d %d %d %d %x %x %d %d\n",
		MasterMap1,
		MasterMap2,
		MasterMap3, 
		MasterMap4,
		MasterMap5,
		MasterMap6,
		MasterMap7,
		MasterMap8,
		MasterMap9,
		MasterMap10,
		MasterMap11,
		MasterMap12,
		MasterMap13);
}

void TestMaster_post_emcy(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg)
{
	eprintf("Master received EMCY message. Node: %2.2x  ErrorCode: %4.4x  ErrorRegister: %2.2x\n", nodeID, errCode, errReg);
}

char query_result = 0;
char waiting_answer = 0;

static void CheckSDO(CO_Data* d, UNS8 nodeId)
{
	UNS32 abortCode;	
	if(getWriteResultNetworkDict (d, nodeId, &abortCode) != SDO_FINISHED)
		eprintf("Master : Failed in changing Slave's transmit type AbortCode :%4.4x \n", abortCode);

	/* Finalise last SDO transfer with this node */
	closeSDOtransfer(&TestMaster_Data, nodeId, SDO_CLIENT);
}


static int MasterSyncCount = 0;
void TestMaster_post_TPDO(CO_Data* d)
{
	eprintf("TestMaster_post_TPDO MasterSyncCount = %d \n", MasterSyncCount);
//
//	{
//		char zero = 0;
//		if(MasterMap4 > 0x80){
//			writeNetworkDict (
//				&TestMaster_Data,
//				TestSlave_Data->bDeviceNodeId,
//				0x2002,
//				0x00,
//				1,
//				0,
//				&zero); 
//		}
//	}

#if 0
	if(waiting_answer){
		UNS32 abortCode;			
		UNS8 size;			
		switch(getReadResultNetworkDict (
			&TestMaster_Data, 
			0x02,
			&query_result,
			&size,
			&abortCode))
		{
			case SDO_FINISHED:
				/* Do something with result here !!*/
				eprintf("Got SDO answer (0x2002, 0x00), %d %d\n",query_result,size);
			case SDO_ABORTED_RCV:
			case SDO_ABORTED_INTERNAL:
			case SDO_RESET:
				waiting_answer = 0;
				closeSDOtransfer(
					&TestMaster_Data,
					0x02,
					SDO_CLIENT);
			break;
			case SDO_DOWNLOAD_IN_PROGRESS:
			case SDO_UPLOAD_IN_PROGRESS:
			break;
		}
	}else if(MasterSyncCount % 10 == 0){
		readNetworkDict (
			&TestMaster_Data,
			0x02,
			0x2002,
			0x00,
			0);
		waiting_answer = 1;
	}
#endif	
	if(MasterSyncCount % 17 == 0){
		eprintf("Master : Ask RTR PDO (0x1402)\n");
		sendPDOrequest(&TestMaster_Data, 0x1402 );
		sendPDOrequest(&TestMaster_Data, 0x1403 );
	}
	if(MasterSyncCount % 50 == 0){
		eprintf("Master : Change slave's transmit type to 0xFF\n");
		UNS8 transmitiontype = 0xFF;
		writeNetworkDictCallBack (&TestMaster_Data, /*CO_Data* d*/
					2, /*UNS8 nodeId*/
					0x1802, /*UNS16 index*/
					0x02, /*UNS16 index*/
					1, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&transmitiontype,/*void *data*/
					CheckSDO); /*SDOCallback_t Callback*/
	}
	if(MasterSyncCount % 50 == 25){
		eprintf("Master : Change slave's transmit type to 0x00\n");
		UNS8 transmitiontype = 0x00;
		writeNetworkDictCallBack (&TestMaster_Data, /*CO_Data* d*/
					2, /*UNS8 nodeId*/
					0x1802, /*UNS16 index*/
					0x02, /*UNS16 index*/
					1, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&transmitiontype,/*void *data*/
					CheckSDO); /*SDOCallback_t Callback*/
	}
	MasterSyncCount++;
}

void TestMaster_post_SlaveBootup(CO_Data* d, UNS8 nodeid)
{
	eprintf("TestMaster_post_SlaveBootup %x\n", nodeid);
	
	ConfigureSlaveNode(&TestMaster_Data, nodeid);
}

