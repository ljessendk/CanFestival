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
void TestMaster_heartbeatError(UNS8 heartbeatID)
{
	eprintf("TestMaster_heartbeatError %d\n", heartbeatID);
}

/*****************************************************************************/
void TestMaster_SDOtimeoutError (UNS8 line)
{
	eprintf("TestMaster_SDOtimeoutError %d\n", line);
}

/*****************************************************************************/
void TestMaster_initialisation()
{
	eprintf("TestMaster_initialisation\n");
}

void TestMaster_preOperational()
{
	eprintf("TestMaster_preOperational\n");
}

void TestMaster_operational()
{
	eprintf("TestMaster_operational\n");
}

void TestMaster_stopped()
{
	eprintf("TestMaster_stopped\n");
}

void TestMaster_post_sync()
{
	eprintf("TestMaster_post_sync\n");
	eprintf("Master: %d %d %d %d\n",MasterMap1, MasterMap2, MasterMap3, MasterMap4);
}

char query_result = 0;
char waiting_answer = 0;

void TestMaster_post_TPDO()
{
	eprintf("TestMaster_post_TPDO\n");

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

	if(waiting_answer){
		UNS32 abortCode;			
		UNS8 size;			
		switch(getReadResultNetworkDict (
			&TestMaster_Data, 
			*TestSlave_Data.bDeviceNodeId,
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
					*TestSlave_Data.	bDeviceNodeId,
					SDO_CLIENT);
			break;
			case SDO_DOWNLOAD_IN_PROGRESS:
			case SDO_UPLOAD_IN_PROGRESS:
			break;
		}
	}else if(MasterMap1 % 10 == 0){
		readNetworkDict (
			&TestMaster_Data,
			*TestSlave_Data.bDeviceNodeId,
			0x2002,
			0x00,
			0);
		waiting_answer = 1;
	}
		
}
