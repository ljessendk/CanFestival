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

#include <stddef.h> /* for NULL */

#include <asm-m68hc12/portsaccess.h>
#include <asm-m68hc12/ports_def.h>
#include <asm-m68hc12/ports.h>
#include  <interrupt.h>

#include <applicfg.h>
#include <candriver.h>
#include <timerhw.h>

#include "def.h"
#include "can.h"
#include "objdictdef.h"
#include "objacces.h"
#include "canOpenDriver.h"
#include "sdo.h"
#include "pdo.h"
#include "init.h"
#include "timer.h"
#include "lifegrd.h"
#include "sync.h"

#include "nmtMaster.h"

// For prototype of exit();
#define exit _exit




// HCS12 configuration
// -----------------------------------------------------

enum E_CanBaudrate 
{
   CAN_BAUDRATE_250K,
   CAN_BAUDRATE_500K,
   CAN_BAUDRATE_1M,
   CAN_BAUDRATE_OLD_VALUE
};

const canBusTime CAN_Baudrates[] =
{
   {
      1,  /* clksrc: Use the bus clock : 16 MHz, the freq. of the quartz's board        */
      3,  /* brp :  chose btw 0 and 63 (6 bits).  freq time quantum = 16MHz / (brp + 1) */
      0,  /* sjw : chose btw 0 and 3 (2 bits). Sync on (sjw + 1 ) time quantum          */
      0,  /* samp : chose btw 0 and 3 (2 bits) (samp + 1 ) samples per bit              */
      1,  /* tseg2 : chose btw 0 and 7 (3 bits) Segment 2 width = (tseg2 + 1)  tq       */
     12,  /* tseg1 : chose btw 0 and 15 (4 bits) Segment 1 width = (tseg1 + 1)  tq      */

      /*
      With these values, 
      - The width of the bit time is 16 time quantum :
          - 1 tq for the SYNC segment (could not be modified)
          - 13 tq for the TIME 1 segment (tseg1 = 12)
          - 2 tq for the TIME 2 segment (tseg2 = 1)
      - Because the bus clock of the MSCAN is 16 MHZ, and the 
        freq of the time quantum is 4 MHZ (brp = 3+1), and  there are 16 tq in the bit time,
        so the freq of the bit time is 250 kHz.
      */
   },

   {
      1,  /* clksrc: Use the bus clock : 16 MHz, the freq. of the quartz's board        */
      1,  /* brp :  chose btw 0 and 63 (6 bits).  freq time quantum = 16MHz / (brp + 1) */
      0,  /* sjw : chose btw 0 and 3 (2 bits). Sync on (sjw + 1 ) time quantum          */
      0,  /* samp : chose btw 0 and 3 (2 bits) (samp + 1 ) samples per bit              */
      1,  /* tseg2 : chose btw 0 and 7 (3 bits) Segment 2 width = (tseg2 + 1)  tq       */
     12,  /* tseg1 : chose btw 0 and 15 (4 bits) Segment 1 width = (tseg1 + 1)  tq      */

      /*
      With these values, 
      - The width of the bit time is 16 time quantum :
          - 1 tq for the SYNC segment (could not be modified)
          - 13 tq for the TIME 1 segment (tseg1 = 12)
          - 2 tq for the TIME 2 segment (tseg2 = 1)
      - Because the bus clock of the MSCAN is 16 MHZ, and the 
        freq of the time quantum is 8 MHZ (brp = 1+1), and  there are 16 tq in the bit time,
        so the freq of the bit time is 500 kHz.
      */
    },

	{
      1,  /* clksrc: Use the bus clock : 16 MHz, the freq. of the quartz's board        */
      1,  /* brp :  chose btw 0 and 63 (6 bits).  freq time quantum = 16MHz / (brp + 1) */
      0,  /* sjw : chose btw 0 and 3 (2 bits). Sync on (sjw + 1 ) time quantum          */
      0,  /* samp : chose btw 0 and 3 (2 bits) (samp + 1 ) samples per bit              */
      1,  /* tseg2 : chose btw 0 and 7 (3 bits) Segment 2 width = (tseg2 + 1)  tq       */
      4,  /* tseg1 : chose btw 0 and 15 (4 bits) Segment 1 width = (tseg1 + 1)  tq      */

      /*
      With these values, 
      - The width of the bit time is 16 time quantum :
          - 1 tq for the SYNC segment (could not be modified)
          - 5 tq for the TIME 1 segment (tseg1 = 4)
          - 2 tq for the TIME 2 segment (tseg2 = 1)
      - Because the bus clock of the MSCAN is 16 MHZ, and the 
        freq of the time quantum is 8 MHZ (brp = 1+1), and  there are 8 tq in the bit time,
        so the freq of the bit time is 1 MHz.
      */
    },

	{
      1,  /* clksrc: Use the bus clock : 16 MHz, the freq. of the quartz's board        */
      0,  /* brp :  chose btw 0 and 63 (6 bits).  freq time quantum = 16MHz / (brp + 1) */
      1,  /* sjw : chose btw 0 and 3 (2 bits). Sync on (sjw + 1 ) time quantum          */
      1,  /* samp : chose btw 0 and 3 (2 bits) (samp + 1 ) samples per bit              */
      4,  /* tseg2 : chose btw 0 and 7 (3 bits) Segment 2 width = (tseg2 + 1)  tq       */
      9,  /* tseg1 : chose btw 0 and 15 (4 bits) Segment 1 width = (tseg1 + 1)  tq      */

      /*
      With these values, 
      - The width of the bit time is 16 time quantum :
          - 1 tq for the SYNC segment (could not be modified)
          - 10 tq for the TIME 1 segment (tseg1 = 9)
          - 5 tq for the TIME 2 segment (tseg2 = 4)
      - Because the bus clock of the MSCAN is 16 MHZ, and the 
        freq of the time quantum is 16 MHZ (brp = 0), and  there are 16 tq in the bit time,
        so the freq of the bit time is 1 MHz.
      */
    }
};



// Required definition variables
// -----------------------------
// The variables that you should define for debugging.
// They are used by the macro MSG_ERR and MSG_WAR in applicfg.h
// if the node is a slave, they can be mapped in the object dictionnary.

UNS8 printMsgErrToConsole = 1;
UNS8 printMsgWarToConsole = 1;

// The variables mapped in the object dictionnary
// ----------------------------------------------
extern UNS32 canopenErrNB_node5;   // Mapped at index 0x6000, subindex 0x0
extern UNS32 canopenErrVAL_node5;  // Mapped at index 0x6001, subindex 0x0
extern UNS8 second;		   // Mapped at index 0x6002, subindex 0x1
extern UNS8 minutes;		   // Mapped at index 0x6002, subindex 0x2
extern UNS8 hour;		   // Mapped at index 0x6002, subindex 0x3
extern UNS8 day;		   // Mapped at index 0x6002, subindex 0x4
extern UNS32 canopenErrNB;	   // Mapped at index 0x6003, subindex 0x1
extern UNS32 canopenErrVAL;	   // Mapped at index 0x6003, subindex 0x2

/*************************User's variables declaration**************************/
UNS8 connectedNode[128];
volatile UNS8 sec = 0; // To count the time every second
UNS8 softCount = 0;

/* The variable to map in a PDO is defined at index and subIndex. Its length is size bytes */
typedef struct mappedVar 
{
  UNS32 index;
  UNS8  subIndex;
  UNS8  size; // in byte
} s_mappedVar;

typedef struct heartbeatConsumer
{
  UNS8 nodeProducer;
  UNS16 time_ms;
} s_heartbeatConsumer;

/**************************prototypes*****************************************/
/* You *must* have these 2 functions in your code*/
void heartbeatError(UNS8 heartbeatID );
void SD0timeoutError(UNS8 bus_id, UNS8 line);

void waitMessage (void );
void heartBeat (void );
void transmitSync (void);
e_nodeState stateNode (UNS8 node);
void configure_master_SDO (UNS32 index, UNS8 serverNode);
UNS8 waitingWriteToSlaveDict ( UNS8 slaveNode, UNS8 error);
UNS8 waitingReadToSlaveDict (UNS8 slaveNode, void * data, UNS8 * size, UNS8 error);
UNS8 configure_client_SDO (UNS8 slaveNode, UNS8 clientNode);
void masterMappingPDO (UNS32 indexPDO, UNS32 cobId, 
		      s_mappedVar *tabMappedVar, UNS8 nbVar);
void slaveMappingPDO (UNS8 slaveNode, UNS32 indexPDO, UNS32 cobId, 
		     s_mappedVar *tabMappedVar, UNS8 nbVar);
void masterHeartbeatConsumer (s_heartbeatConsumer 
			     *tabHeartbeatConsumer, UNS8 nbHeartbeats);
void masterHeartbeatProducer (UNS16 time);
void slaveHeartbeatConsumer (UNS8 slaveNode, s_heartbeatConsumer 
			    *tabHeartbeatConsumer, UNS8 nbHeartbeats);
void slaveHeartbeatProducer (UNS8 slaveNode, UNS16 time);
void masterPDOTransmissionMode (UNS32 indexPDO,  UNS8 transType);
void slavePDOTransmissionMode (UNS8 slaveNode, UNS32 indexPDO,  UNS8 transType);
void masterSYNCPeriod (UNS32 SYNCPeriod);
int main (void);

// Interruption timer 3. (The timer 4 is used by CanOpen)
void __attribute__((interrupt)) timer3Hdl (void);
void incDate (void);
void initLeds (void);
void initCanHCS12 (void);
void initTimerClk (void);


/*****************************************************************************/
void heartbeatError(UNS8 heartbeatID)
{
  // MSG_ERR should send the values canopenErrNB and canopenErrVAL on event in a PDO,
  // But we do not have mapped the variables in a PDO, so it sends nothing.
  // See the note at the end of END CONFIGURING THE NETWORK.
  MSG_WAR(0x2F00, "HeartBeat, no response from node : ", heartbeatID);
}

/*****************************************************************************/
void SD0timeoutError (UNS8 bus_id, UNS8 line)
{
  // Informations on what occurs are in transfers[bus_id][line]....
  // See scanSDOtimeout() in sdo.c
}

//------------------------------------------------------------------------------
/************************** FUNCTIONS TO CONFIGURE THE NETWORK******************/

//------------------------------------------------------------------------------
/* Node mode result after NodeGuard query */
e_nodeState stateNode(UNS8 node) 
{
  e_nodeState state = getNodeState(0, node);
  switch (state) {
  case Unknown_state: 
    MSG_WAR(0x3F05, "Not connected (Does not have sent its status) node :", node);
    break;
  case Operational: 
    MSG_WAR(0x3F06, "Ok, in operational mode, node : ", node);
    break;
  case Pre_operational: 
    MSG_WAR(0x3F07, "OK in pre-operational mode, node : ", node);
    break;
  default:
    MSG_WAR(0x3F08, "OK connected but in curious mode, node : ", node);
  }
  return state;
}

//------------------------------------------------------------------------------
/* The master is writing in its dictionnary to configure the SDO parameters 
to communicate with server_node
*/
void configure_master_SDO(UNS32 index, UNS8 serverNode)
{
  UNS32 data32;
  UNS8  data8;
  UNS8 sizeData = 4 ; // in bytes

  /* At subindex 1, the cobId of the Can message from the client.
  It is always defined inside the server dictionnary as 0x600 + server_node.
  So, we have no choice here ! */
  data32 = 0x600 + serverNode;
  setODentry(index, 1, &data32, sizeData, 0);

  {
    // Test
    UNS32  *pbData;
    UNS8 length;
    UNS32 returnValue;
    UNS8 dataType;
    // Relecture
    MSG_WAR(0x1000, "Reading index : ", index);
    MSG_WAR(0x1000, "     subindex : ", 1);
    returnValue = getODentry(index, 1, (void * *)&pbData, (UNS8 *)&length, &dataType, 0);
    MSG_WAR(0x1000, "          val : ", *pbData);
  }


  /* At subindex 2, the cobId of the Can message from the server to the client.
  It is always defined inside the server dictionnary as 0x580 + client_node.
  So, we have no choice here ! */
  data32 = 0x580 + serverNode;
  setODentry(index, 2, &data32, sizeData, 0);

  /* At subindex 3, the node of the server */
  data8 = serverNode;
  sizeData = 1;
  setODentry(index, 3, &data8, sizeData, 0);

  {
    UNS8  *pbData;
    UNS8 length;
    UNS32 returnValue;
    UNS8 dataType;
    // Relecture
    MSG_WAR(0x1000, "Reading  index : ", index);
    MSG_WAR(0x1000, "      subindex : ", 3);
    returnValue = getODentry(index, 1, (void * *)&pbData, (UNS8 *)&length, &dataType, 0);
    MSG_WAR(0x1000, "           val : ", *pbData);
  }
}

//------------------------------------------------------------------------------
/*
 */
UNS8 waitingWriteToSlaveDict(UNS8 slaveNode, UNS8 error)
{
   UNS8 err;
  UNS32 abortCode;
  MSG_WAR(0x3F21, "Sending SDO to write in dictionnary of node : ", slaveNode);
  if (error) {
    MSG_ERR(0x1F22, "Unable to send the SDO to node ", slaveNode);
    return -1;
  }
  /* Waiting until the slave has responded */
  while (getWriteResultNetworkDict (0, slaveNode, &abortCode) == SDO_DOWNLOAD_IN_PROGRESS) {
    // Check if some SDO response are missing
    scanSDOtimeout();
  }

  err = getWriteResultNetworkDict (0, slaveNode, &abortCode);
  if (err == SDO_FINISHED) {
    MSG_WAR(0x3F22, "SDO download finished to Node : ", slaveNode);
    // Release the line. Don't forget !!!
    closeSDOtransfer(0, slaveNode, SDO_CLIENT);
    return 0;
  }

  if (err == SDO_ABORTED_RCV) {
    MSG_WAR(0x2F20, "Received SDO abort from node : ", slaveNode);
  }

  if (err == SDO_ABORTED_INTERNAL) {
    MSG_WAR(0x2F20, "Internal SDO abort for node : ", slaveNode);
  }
  // Looking for the line transfert number to read the index, subindex and releasing the line.
  {
    UNS8 line;
    err = getSDOlineOnUse(0, slaveNode, SDO_CLIENT, &line);
    if (err) {
      MSG_WAR(0x2F21, "No line found for node : ", slaveNode);
      exit(-1);
    }
    MSG_WAR (0x2F22, "while writing at his index : ", transfers[0][line].index);
    MSG_WAR (0x2F23, "                  subIndex : ", transfers[0][line].subIndex);
    //Releasing the line.
    closeSDOtransfer(0, slaveNode, SDO_CLIENT);
    exit(-1);
  }
 return 0;
}

//------------------------------------------------------------------------------
/*
 */
UNS8 waitingReadToSlaveDict(UNS8 slaveNode, void * data, UNS8 * size, UNS8 error)
{
  UNS8 err;
  UNS32 abortCode;
  MSG_WAR(0x3F2A, "Sending SDO to read in dictionnary of node : ", slaveNode);
  if (error) {
    MSG_ERR(0x1F2B, "Unable to send the SDO to node ", slaveNode);
    return -1;
  }
  /* Waiting until the slave has responded */
  while (getReadResultNetworkDict (0, slaveNode, data, size, &abortCode) == SDO_UPLOAD_IN_PROGRESS) {
    // Check if some SDO response are missing
    scanSDOtimeout();
  }
  err = getReadResultNetworkDict (0, slaveNode, data, size, &abortCode);
  if (err == SDO_FINISHED) {
    MSG_WAR(0x3F2C, "SDO upload finished to Node : ", slaveNode);
    // Release the line. Don't forget !!!
    closeSDOtransfer(0, slaveNode, SDO_CLIENT);
    return 0;
  }
  if (err == SDO_ABORTED_RCV) {
    MSG_WAR(0x2F2D, "Received SDO abort from node : ", slaveNode);
  }

  if (err == SDO_ABORTED_INTERNAL) {
    MSG_WAR(0x2F2E, "Internal SDO abort for node : ", slaveNode);
  }
  // Looking for the line transfert number to read the index, subindex and releasing the line.
  {
    UNS8 line;
    err = getSDOlineOnUse(0, slaveNode, SDO_CLIENT, &line);
    if (err) {
      MSG_WAR(0x2F2F, "No line found for node : ", slaveNode);
      exit(-1);
    }
    MSG_WAR (0x2F30, "while writing at his index : ", transfers[0][line].index);
    MSG_WAR (0x2F31, "                  subIndex : ", transfers[0][line].subIndex);
    //Releasing the line.
    closeSDOtransfer(0, slaveNode, SDO_CLIENT);
    exit(-1);
  }    

 return 0;
}

//------------------------------------------------------------------------------
/* The master is writing in the slave dictionnary to configure the SDO parameters
Remember that the slave is the server, and the master is the client.
 */
UNS8 configure_client_SDO(UNS8 slaveNode, UNS8 clientNode)
{
  UNS8 data;
  UNS8 NbDataToWrite = 1 ; // in bytes
  UNS8 err = 0;
  MSG_WAR(0x3F20, "Configuring SDO by writing in dictionnary Node ", slaveNode);
  /* It is only to put at subindex 3 the serverNode. It is optionnal.
     In the slave dictionary, only one SDO server is defined, at index 
     0x1200 */
  data = clientNode;
  err = writeNetworkDict(0, slaveNode, 0x1200, 3, NbDataToWrite, 0, &data); 
  waitingWriteToSlaveDict(slaveNode, err);
 
  return 0;
}		
  
//------------------------------------------------------------------------------
/*
 */

void masterMappingPDO(UNS32 indexPDO, UNS32 cobId, 
		      s_mappedVar *tabMappedVar, UNS8 nbVar)
{
  UNS32 *pbData;
  UNS32 data32; 
  UNS8 i;
  UNS8 size = 0;
  UNS8 dataType;

  if ((indexPDO >= 0x1400) && (indexPDO <= 0x15FF))
    MSG_WAR(0x3F30, "Configuring MASTER for PDO receive, COBID : ", cobId);

  if ((indexPDO >= 0x1800) && (indexPDO <= 0x19FF))
    MSG_WAR(0x3F31, "Configuring MASTER for PDO transmit, COBID : ", cobId);

  /* At indexPDO, subindex 1, defining the cobId of the PDO */
  setODentry(indexPDO, 1, &cobId, 4, 0);
  /* The mapping ... */
  /* ----------------*/
  /* At subindex 0, the number of variables in the PDO */
  setODentry(indexPDO + 0x200, 0, &nbVar, 1, 0);
  getODentry(indexPDO + 0x200, 0, (void * *)&pbData, &size, &dataType, 0);
  /* At each subindex 1 .. nbVar, The index,subindex and size of the variable to map in 
     the PDO. The first variable after the COBID is defined at subindex 1, ... 
     The data to write is the concatenation on 32 bits of (msb ... lsb) : 
     index(16b),subIndex(8b),sizeVariable(8b)
*/
  for (i = 0 ; i < nbVar ; i++) {
    data32 = ((tabMappedVar + i)->index << 16) |
      (((tabMappedVar + i)->subIndex & 0xFF) << 8) |
      ((tabMappedVar + i)->size & 0xFF);
    // Write dictionary
    setODentry(indexPDO + 0x200, i + 1, &data32, 4, 0);

#   ifdef MORE_COMMENTS
    printf("Mapped variable defined  at index 0x%X, subIndex 0x%X, %d bits\n", 
	   (tabMappedVar + i)->index, (tabMappedVar + i)->subIndex, 8 * (tabMappedVar + i)->size);
    // Only to verify.
    // Read dictionnary
    getODentry(indexPDO + 0x200, i + 1, (void * *)&pbData, &size, &dataType, 0);
    printf("Writen à  index 0x%X, subIndex 0x%X, %d bits : 0x%08X\n", 
	   indexPDO + 0x200, i + 1, 8 * size, *pbData);
#   endif
    
  }
}

//------------------------------------------------------------------------------
/*
 */

void slaveMappingPDO(UNS8 slaveNode, UNS32 indexPDO, UNS32 cobId, 
		     s_mappedVar *tabMappedVar, UNS8 nbVar)
{
  UNS32 data32; 
  UNS8 i;
  UNS8 err;
  UNS8 nbBytes = 1;
  if ((indexPDO >= 0x1400) && (indexPDO <= 0x15FF))
    MSG_WAR(0x3F32, "Configuring slave for PDO receive, COBID : ", cobId);

  if ((indexPDO >= 0x1800) && (indexPDO <= 0x19FF))
    MSG_WAR(0x3F33, "Configuring slave for PDO transmit, COBID : ", cobId);

  /* At indexPDO, subindex 1, defining the cobId of the PDO */
  err = writeNetworkDict(0, slaveNode, indexPDO, 1, 4, 0, &cobId); 
  waitingWriteToSlaveDict(slaveNode, err);

  /* The mapping ... */
  /* ----------------*/
  /* At subindex 0, the number of variables in the PDO */
  err = writeNetworkDict(0, slaveNode, indexPDO + 0x200, 0, nbBytes, 0, &nbVar); 
  waitingWriteToSlaveDict(slaveNode, err);

  /* At each subindex 1 .. nbVar, The index,subindex and size of the variable to map in 
     the PDO. The first variable after the COBID is defined at subindex 1, ... 
     The data to write is the concatenation on 32 bits of (msb ... lsb) : 
     index(16b),subIndex(8b),sizeVariable(8b)
*/
  for (i = 0 ; i < nbVar ; i++) {
    data32 = ((tabMappedVar + i)->index << 16) |
      (((tabMappedVar + i)->subIndex & 0xFF) << 8) |
      ((tabMappedVar + i)->size & 0xFF);

    // Write dictionary
    err = writeNetworkDict(0, slaveNode, indexPDO + 0x200, i + 1, 4, 0, &data32); 
    waitingWriteToSlaveDict(slaveNode, err);

#   ifdef MORE_COMMENTS
    printf("Mapped variable defined  at index 0x%X, subIndex 0x%X, %d bits\n", 
	   (tabMappedVar + i)->index, (tabMappedVar + i)->subIndex, 8 * (tabMappedVar + i)->size);

    printf("At node 0x%X Writen at  index 0x%X, subIndex 0x%X, %d bits : 0x%08X\n", 
	   slaveNode, indexPDO + 0x200, i + 1, 32, data32);
#   endif
    
  }
}

//------------------------------------------------------------------------------
/*
 */
void masterHeartbeatConsumer(s_heartbeatConsumer 
			     *tabHeartbeatConsumer, UNS8 nbHeartbeats)
{
  UNS32 data;
  UNS8 i;
  UNS8 nbHB = nbHeartbeats;

  MSG_WAR(0x3F40, "Configuring heartbeats consumers for master", 0);
  /* At index 1016, subindex 0 : the nb of consumers (ie nb of nodes of which are expecting heartbeats) */
  setODentry(0x1016, 0, & nbHB, 1, 0);
  
  /* At Index 1016, subindex 1, ... : 32 bit values : msb ... lsb :
     00 - node_consumer (8b) - time_ms (16b)
     Put 0 to ignore the entry.
  */
  for (i = 0 ; i < nbHeartbeats ; i++) {
    data = (((tabHeartbeatConsumer + i)->nodeProducer & 0xFF)<< 16) | ((tabHeartbeatConsumer + i)->time_ms & 0xFFFF);
    setODentry(0x1016, i + 1, & data, 4, 0);
  }
}

//------------------------------------------------------------------------------
/*
 */

void masterHeartbeatProducer(UNS16 time)
{
  UNS16 hbTime = time;
  MSG_WAR(0x3F45, "Configuring heartbeat producer for master", 0);
  /* At index 1017, subindex 0, defining the time to send the heartbeat. Put 0 to never send heartbeat */
  setODentry(0x1017, 0, &hbTime, 2, 0);
}

//------------------------------------------------------------------------------
/*
 */
void slaveHeartbeatConsumer(UNS8 slaveNode, s_heartbeatConsumer 
			    *tabHeartbeatConsumer, UNS8 nbHeartbeats)
{
  UNS32 data;
  UNS8 err;
  UNS8 i;
  
  MSG_WAR(0x3F46, "Configuring heartbeats consumers for node  : ", slaveNode);
  
  /* At Index 1016, subindex 1, ... : 32 bit values : msb ... lsb :
     00 - node_consumer (8b) - time_ms (16b)
     Put 0 to ignore the entry.
  */
  for (i = 0 ; i < nbHeartbeats ; i++) {
    data = (((tabHeartbeatConsumer + i)->nodeProducer & 0xFF)<< 16) | 
      ((tabHeartbeatConsumer + i)->time_ms & 0xFFFF);
    err = writeNetworkDict(0, slaveNode, 0x1016, i + 1, 4, 0, &data); 
    waitingWriteToSlaveDict(slaveNode, err);
  }
}

//------------------------------------------------------------------------------
/*
 */

void slaveHeartbeatProducer(UNS8 slaveNode, UNS16 time)
{
  UNS8 err;
  MSG_WAR(0x3F47, "Configuring heartbeat producer for node  : ", slaveNode);
  /* At index 1017, subindex 0, defining the time to send the heartbeat. Put 0 to never send heartbeat */

  err = writeNetworkDict(0, slaveNode, 0x1017, 0, 2, 0, &time); 
  waitingWriteToSlaveDict(slaveNode, err);
}

//------------------------------------------------------------------------------
/*
 */

void masterPDOTransmissionMode(UNS32 indexPDO,  UNS8 transType)
{
  MSG_WAR(0x3F48, "Configuring transmission from master, indexPDO : ", indexPDO);
 
  /* At subindex 2, the transmission type */
  setODentry(indexPDO, 2, &transType, 1, 0);
}


//------------------------------------------------------------------------------
/*
 */

void slavePDOTransmissionMode(UNS8 slaveNode, UNS32 indexPDO,  UNS8 transType)
{
  UNS8 err;
  MSG_WAR(0x3F41, "Configuring transmission mode for node : ", slaveNode);
  MSG_WAR(0x3F42, "                              indexPDO : ", indexPDO);

  err = writeNetworkDict(0, slaveNode, indexPDO, 2, 1, 0, &transType); 
  waitingWriteToSlaveDict(slaveNode, err);
}

//------------------------------------------------------------------------------
/*
 */

void masterSYNCPeriod(UNS32 SYNCPeriod)
{
 UNS32 cobId = 0x40000080;
 MSG_WAR(0x3F49, "Configuring master to send SYNC every ... micro-seconds :", SYNCPeriod);
 /* At index 0x1006, subindex 0 : the period in ms */
 setODentry(0x1006, 0, &SYNCPeriod , 4, 0);
 /* At index 0x1005, subindex 0 : Device generate SYNC signal with CobId 0x80 */
 setODentry(0x1005, 0, &cobId, 4, 0);
}

//------------------------------------------------------------------------------


//Initialisation of the port B for the leds.
void initLeds(void)
{
  // Port B is output
  IO_PORTS_8(DDRB)= 0XFF;
  // RAZ
  IO_PORTS_8(PORTB) = 0xFF;
}

//------------------------------------------------------------------------------



void initCanHCS12 (void)
{  
  //Init the HCS12 microcontroler for CanOpen 
  initHCS12();
   // Init the HCS12  CAN driver
  const canBusInit bi0 = {
    0,    /* no low power                 */ 
    0,    /* no time stamp                */
    1,    /* enable MSCAN                 */
    0,    /* clock source : oscillator (In fact, it is not used)   */
    0,    /* no loop back                 */
    0,    /* no listen only               */
    0,    /* no low pass filter for wk up */
	CAN_Baudrates[CAN_BAUDRATE_250K],
    {
      0x00,    /* Filter on 16 bits. See Motorola Block Guide V02.14 fig 4-3 */
      0x00, 0xFF, /* filter 0 hight accept all msg      */
      0x00, 0xFF, /* filter 0 low accept all msg        */
      0x00, 0xFF, /* filter 1 hight filter all of  msg  */
      0x00, 0xFF, /* filter 1 low filter all of  msg    */
      0x00, 0xFF, /* filter 2 hight filter most of  msg */
      0x00, 0xFF, /* filter 2 low filter most of  msg   */
      0x00, 0xFF, /* filter 3 hight filter most of  msg */
      0x00, 0xFF, /* filter 3 low filter most of  msg   */
    }
  };   

  canInit(CANOPEN_LINE_NUMBER_USED, bi0);  //initialize filters...
  unlock(); // Allow interruptions
}

/*********************************************************************/
// For Second timer
void incDate(void)
{
  if (sec == 59) 
    sec = 0;
  else
    sec++;

  // Toggle the led 4 every seconds
  IO_PORTS_8(PORTB) ^= 0x10;

}

// Init the time for the second counter
void initTimerClk(void)
{

  lock();   // Inhibe les interruptions

  // Configuration du Channel 3
  IO_PORTS_8(TIOS) |= 0x08;     // Canal 3 en sortie
  IO_PORTS_8(TCTL2) &= ~(0xC0); // Canal 3 déconnecté du pin de sortie
  IO_PORTS_8(TIE) |= 0x08;      // Autorise interruption Canal 3
  IO_PORTS_8(TSCR1) |= 0x80;    // Mise en route du timer
  unlock(); // Autorise les interruptions
}


/*********************************************************************/
void __attribute__((interrupt)) timer3Hdl(void)
{
  //IO_PORTS_8(PORTB) ^= 0x10;
  //IO_PORTS_8(PORTB) &= ~0x20;
  IO_PORTS_8(TFLG1) = 0x08; // RAZ du flag interruption timer 3
  // Calcul evt suivant. Clock 8 MHz -> 8000 evt de 1 ms!! Doit tenir sur 16 bits
  // Attention, ça change si on utilise la pll
  // Lorsque le timer atteindra la valeur de TC3 (16 bits), l'interruption timer3Hdl sera déclenchée
  // Si on utilise la PLL à 24 MHZ, alors la vitesse du bus est multipliée par 3.

/*   Assume that our board uses a 16 MHz quartz */
/*   Without pre-division, 8000 counts takes 1 ms. */
/*   We are using a pre-divisor of 32. (register TSCR2) See in CanOpenDriverHC12/timerhw.c  */
/*   So 1000 counts takes 4 ms. */
/*   We must have a soft counter of 250 to count a second. */
  
/*
  We check in an interrupt handler if a message is arrived.
*/
  receiveMsgHandler(0);

  IO_PORTS_16(TC3H) += (1000); // IT every 4000 count.
  softCount++;
  if (softCount == 250) {
    softCount = 0;
    incDate();
  }
}




/*****************************************************************************/



/********************************* MAIN ***************************************/

 
int main()
{

  UNS8 second_last;
  UNS8 minutes_last;
  UNS8 sendingResetError = 0;
  UNS8 ok, i;

  /* initialisation du bus Can */
  initCanHCS12();  
 
  /* arrays initialisation, etc */
  initCANopenMain();     
      
  /* arrays initialisation, etc */
  initCANopenMaster();  

/* Defining the node Id */
  setNodeId(0x01);
  MSG_WAR(0x3F50, "My node ID is : ", getNodeId()); 

  /* Put the master in operational mode */
  setState(Operational);

 /* Init the table of connected nodes */
  for (i = 0 ; i < 128 ; i++)
    connectedNode[i] = 0;

  /* Initialisation */
  initLeds();
  initTimer( );
  initTimerClk();
    


  /******************** CONFIGURING THE NETWORK **************************/
  
  /* Which nodes are connected ? */
  /* Sending a request Node guard to node 5 and 6 */
  MSG_WAR(0x3F04, "Sending a node guard to node : ", 5);
  masterReadNodeState(0, 0x05);

  /* Sending a message to the node 6, only as example */
  MSG_WAR(0x3F04, "Sending a node guard to node : ", 6);
  masterReadNodeState(0, 0x06);
  /* Waiting for a second the response */
  sec = 0;
  while (sec < 2) {};

  /* Whose node have answered ? */
  connectedNode[5] = stateNode(5);
  if (connectedNode[5] != Unknown_state) {
    MSG_WAR(0x3F06, "Node 5 connected. Its state is : ", connectedNode[5]);
  }
  else {
    MSG_WAR(0x3F07, "Node 5 NOT connected ", connectedNode[5]);
  }

  connectedNode[6] = stateNode(6);
  if (connectedNode[6] != Unknown_state) {
    MSG_WAR(0x3F08, "Node 6 connected. Its state is : ", connectedNode[6]);
  }
  else {
    MSG_WAR(0x3F09, "Node 6 NOT connected ", connectedNode[6]);
  }

  /* Configure the SDO master to communicate with node 5 and node 6 */
  configure_master_SDO(0x1280, 0x05);
  /* Configure the SDO of node 5 */
  /* getNodeId() returns my node Id */
  configure_client_SDO(0x05, getNodeId());

  /* Mapping of the PDO 
     Chose some COBID in (hexa) 181-1FF, 201-27F, 281-2FF, 301-37F, 
     381-3FF, 401-47F, 481-4FF, 501-57F,
     without other restriction.
     (Of course, you must not define 2 PDO transmit with the same cobId !!)
  */
 
  /*
     *** PDO node 1 <-- node 5 ***
     *** cobId 0x181 *************
     MASTER (node 1)
     Mapped to variables (node1) [index-subindex-size_bits]: 
       day    [0x6002 - 0x04 - 8]
       hour   [0x6002 - 0x03 - 8]
       second [0x6002 - 0x01 - 8]

     SLAVE (node 5)
     Mapped to variables (node5) [index-subindex-size_bits]: 
       day    [0x2000 - 0x04 - 8]
       hour   [0x2000 - 0x03 - 8]
       second [0x2000 - 0x01 - 8]
*/

  /* Configuring the first PDO receive, defined at index 0x1400 and 0x1600 */
  {
    s_mappedVar tabMappedVar[8] = { {0x6002,4,8}, {0x6002,3,8}, {0x6002,1,8}, };
    masterMappingPDO(0x1400, 0x181, tabMappedVar, 3);
  }

  /* Configuring the first PDO transmit, defined at index 0x1800 and 0x1A00 */
  {
    s_mappedVar tabMappedVar[8] = { {0x2000,4,8}, {0x2000,3,8}, {0x2000,1,8}, };
    slaveMappingPDO(0x05, 0x1800, 0x181, tabMappedVar, 3);
  }
   /*
     *** PDO node 1 <-- node 5 ***
     *** cobId 0x182 *************
     MASTER (node 1)
     Mapped to variables (node1) [index-subindex-size_bits]: 
       minute    [0x6002 - 0x02 - 8]

     SLAVE (node 5)
     Mapped to variables (node5) [index-subindex-size_bits]: 
       minute    [0x2000 - 0x02 - 8]
   */

  /* Configuring PDO receive, defined at index 0x1400 and 0x1600 */
  {
    s_mappedVar tabMappedVar[8] = { {0x6002,2,8} };
    masterMappingPDO(0x1401, 0x182, tabMappedVar, 1);
  }

  /* Configuring PDO transmit, defined at index 0x1800 and 0x1A00 */
  {
    s_mappedVar tabMappedVar[8] = { {0x2000,2,8} };
    slaveMappingPDO(0x05, 0x1801, 0x182, tabMappedVar, 1);
  }


  /*
     *** PDO node 1 <-- node 5 ***
     *** cobId 0x183 *************
     Error management :  By this way, The node can send by PDO an error
     MASTER (node 1)
     Mapped to variables (node1) [index-subindex-size_bits]: 
       canopenErrNb_node5     [0x6000 - 0x00 - 32]
       canopenErrVal_node5    [0x6001 - 0x00 - 32] 

     SLAVE (node 5)
     Mapped to variables (node5) [index-subindex-size_bytes]: 
       canopenErrNb     [0x6000 - 0x00 - 32]
       canopenErrVal    [0x6001 - 0x00 - 32]  
  */

  /* Configuring  PDO receive, defined at index 0x1402 and 0x1602 */
  {
    s_mappedVar tabMappedVar[8] = { {0x6000,0,32}, {0x6001, 0, 32}};
    masterMappingPDO(0x1402, 0x183, tabMappedVar, 2);
  }

  /* Configuring PDO transmit, defined at index 0x1802 and 0x1A02 */
  {
    s_mappedVar tabMappedVar[8] = { {0x6000,0,32}, {0x6001, 0, 32}};
    slaveMappingPDO(0x05, 0x1802, 0x183, tabMappedVar, 2);
  }
 
  /*
     *** PDO node 1 --> node 5 ***
     *** cobId 0x184 *************
     Error management :  To reset the error
     MASTER (node 1)
     Mapped to variables (node1) [index-subindex-size_bits]: 
       canopenErrNb_node5     [0x6000 - 0x00 - 32]
       canopenErrVal_node5    [0x6001 - 0x00 - 32] 

     SLAVE (node 5)
     Mapped to variables (node5) [index-subindex-size_bytes]: 
       canopenErrNb     [0x6000 - 0x00 - 32]
       canopenErrVal    [0x6001 - 0x00 - 32]  
  */

  /* Configuring  PDO transmit, defined at index 0x1803 and 0x1103 */
  {
    s_mappedVar tabMappedVar[8] = { {0x6000,0,32}, {0x6001, 0, 32}};
    masterMappingPDO(0x1801, 0x184, tabMappedVar, 2);
  }

  /* Configuring PDO transmit, defined at index 0x1403 and 0x1603 */
  {
    s_mappedVar tabMappedVar[8] = { {0x6000,0,32}, {0x6001, 0, 32}};
    slaveMappingPDO(0x05, 0x1400, 0x184, tabMappedVar, 2);
  }

  /* Configuring the node 5 heartbeat */
  /* Check every 3000 ms if it have received a heartbeat from node 1 */
  {
    UNS8 nbHeartbeatsToReceive = 1;
    s_heartbeatConsumer tabHeartbeatConsumer[10] = {{1, 0xBB8}};
    slaveHeartbeatConsumer(0x05, tabHeartbeatConsumer, nbHeartbeatsToReceive);
  }
  /* Sending every 1000 ms an heartbeat */
  slaveHeartbeatProducer(0x05, 0x3E8);

  /* Configuring the master heartbeat */
  /* Check every 3000 ms if it have received a heartbeat from node 5 */
    {
    UNS8 nbHeartbeatsToReceive = 1;
    s_heartbeatConsumer tabHeartbeatConsumer[10] = {{5, 0xBB8}};
    masterHeartbeatConsumer(tabHeartbeatConsumer, nbHeartbeatsToReceive);
  }

    /* Sending every 1000 ms an heartbeat */
    masterHeartbeatProducer(0x3E8);


    
    /* Configuring the transmission mode of the PDO */
    slavePDOTransmissionMode(0x05, 0x1800,  TRANS_EVERY_N_SYNC (1));
    slavePDOTransmissionMode(0x05, 0x1801,  TRANS_EVENT);
    slavePDOTransmissionMode(0x05, 0x1802,  TRANS_EVENT);
    masterPDOTransmissionMode(0x1801,  TRANS_EVENT);


    /* Configuring the master to send a SYNC message every 1 s */
    /* Note than any other node can send the SYNC instead of the master */
    masterSYNCPeriod(1000000); 

    {
      // Reading the period of heartbeat which has been written in node 5 dictionary
      UNS8 node = 5;
      UNS16 index = 0x1017;
      UNS8 subindex = 0;
      //UNS8 notused = 0;
      UNS16 hb = 0;
      UNS8  size_data = 0;
      UNS8 error;
      MSG_WAR(0x3F50, "Reading dictionary noeud 5, 1017/0", 0);
      error = readNetworkDict(0, node, index, subindex, 0);
      //error = readNetworkDict(0, node, index, subindex, &notused);
      if (error) {
	MSG_ERR(0x1F50, "!!! ERROR reading dictionary noeud 5, 1017/0", 0);
	exit (-1);
      }
      /* Waiting until the server has responded */
      error = waitingReadToSlaveDict(node,  (UNS16 *)&hb,  &size_data, error);
      MSG_WAR(0x1F51, "Read dictionary of node 5, index/subindex 1017/0 value = ", hb);
      MSG_WAR(0x1F51, "         size of data (bytes) = ", size_data);
    }

    /* Put the node 5 in operational mode 
       The mode is changed according to the slave state machine mode :
         initialisation  ---> pre-operational (Automatic transition)
         pre-operational <--> operational
         pre-operational <--> stopped
         pre-operational, operational, stopped -> initialisation
       NMT_Start_Node           // Put the node in operational mode       
       NMT_Stop_Node		// Put the node in stopped mode    
       NMT_Enter_PreOperational // Put the node in pre_operational mode
       NMT_Reset_Node		// Put the node in initialization mode 
       NMT_Reset_Comunication	// Put the node in initialization mode 
    */
    masterSendNMTstateChange(0, 0x05, NMT_Start_Node);

    // Note
    //-----
    // We do not have mapped the variable canopenErrNB and canopenErrVAL.
    // We should have done that !
    // the macro MSG_ERR try to send the PDO(s) which contains these two variables.
    // While the PDO will not be found, if you are printing the warnings in file pdo.c,
    // it will print "0X393B Unable to send variable on event :  not mapped in a PDO to send on event" for  
    // example when you enter the function heartbeatError. 

   /******************** END CONFIGURING THE NETWORK **********************/
    
    
    
    /* Init the errors values that may send the node 5 */
    canopenErrNB_node5 = 0;
    canopenErrVAL_node5 = 0;

    /***********/
    /* Running */
    /***********/

  /* SDO test with node 5 */
  /* This code may takes too much room in memory if you are also debugging the file sdo.c */
  {
    // Reading string
    UNS8 dataW[20];
    UNS8 dataR[20];
    UNS8 size;
    UNS8 err;
    MSG_WAR(0x3F05, "Test SDO", 0);

    MSG_WAR(0x3F10, "Writing to node 5 at 0x6002-0 ...", 0);
    strcpy(dataW, "Au Revoir");
    MSG_WAR(0x3F10, dataW, 0);
    size = 20;
    err = writeNetworkDict(0, 5, 0x6002, 0, 10, visible_string, dataW);
    err = waitingWriteToSlaveDict(5, err);
  
    err = readNetworkDict(0, 5, 0x6002, 0, visible_string);
    err = waitingReadToSlaveDict(5, dataR, &size, err);
    MSG_WAR(0x3F08, "Read from node 5 at 0x6002-0" , 0);
    MSG_WAR(0x3F08, dataR, 0);
    
    MSG_WAR(0x3F08, "node 5. Hardware version. (default = compil. date) ...", 0);
    err = readNetworkDict(0, 5, 0x1009, 0, visible_string);
 
    err = waitingReadToSlaveDict(5, dataR, &size, err);
    MSG_WAR(0x3F08, dataR, 0);

    MSG_WAR(0x3F08, "node 5. Software version. (default = compil. time) ...", 0);
    err = readNetworkDict(0, 5, 0x100A, 0, visible_string);
    err = waitingReadToSlaveDict(5, dataR, &size, err);
    MSG_WAR(0x3F08, dataR, 0);
  }

  // Node identity ?
  {
    UNS8 *data;
    UNS8 size;
    UNS8 dataType;
    // Manufacturer Device name (default = empty string)
    getODentry(0x1008, 0x0, (void **)&data, &size, &dataType, 0);
    MSG_WAR(0x3F09, data, 0);
    // Manufacturer Hardware version. (default = compilation. date)
    getODentry(0x1009, 0x0, (void **)&data, &size, &dataType, 0);
    MSG_WAR(0x3F09, data, 0);
    // Manufacturer Software version. (default = compilation. time)
    getODentry(0x100A, 0x0, (void **)&data, &size, &dataType, 0);
    MSG_WAR(0x3F09, data, 0);
  }
    
  while(1) { 
    // To transmit the SYNC if it is time to do.
    computeSYNC();

    // Testing if heartsbeat have been received, and send a heartbeat if it is time.
     heartbeatMGR();

     // Messages received ?
     // The function is called in void __attribute__((interrupt)) timer3Hdl (void)
     //receiveMsgHandler(0);

 if (minutes != minutes_last) {
	MSG_WAR(0x3F80, "Minutes changed :", minutes);
	minutes_last = minutes;
      }

      if (second != second_last) {
	MSG_WAR(0x3F81, "Seconds : ", second);
	second_last = second;

	if (canopenErrNB_node5) {
	  MSG_WAR(0x3F82, "Received an error from node 5, NB : ", canopenErrNB_node5);
	  MSG_WAR(0x3F83, "                            VALUE : ", canopenErrVAL_node5);
	  // Reseting the error
	  canopenErrNB_node5 = 0;
	  canopenErrVAL_node5 = 0;
	  sendingResetError = 1;
	}
       
	if ((second == 00) && sendingResetError) {
	  MSG_WAR(0x3F84, 
		 "Sending to node 5 a PDO envent to reset the error NB and VAL : ",0);
	  sendPDOevent(0, &canopenErrNB_node5);
	  sendingResetError = 0; 
	}
      
      
      }	// end if (second != second_last)

      
  } // end while
      
 
  return (0); 
}
 
