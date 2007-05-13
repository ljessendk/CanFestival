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

#include "states.h"
#include "def.h"

#include "nmtSlave.h"

/* Prototypes for internals functions */
void switchCommunicationState(CO_Data* d, 
	s_state_communication *newCommunicationState);
	
/*****************************************************************************/
e_nodeState getState(CO_Data* d)
{
  return d->nodeState;
}

/*****************************************************************************/
void canDispatch(CO_Data* d, Message *m)
{
	 switch(m->cob_id.w >> 7)
	{
		case SYNC:
			if(d->CurrentCommunicationState.csSYNC)
				proceedSYNC(d,m);
			break;
		/* case TIME_STAMP: */
		case PDO1tx:
		case PDO1rx:
		case PDO2tx:
		case PDO2rx:
		case PDO3tx:
		case PDO3rx:
		case PDO4tx:
		case PDO4rx:
			if (d->CurrentCommunicationState.csPDO)
				proceedPDO(d,m);
			break;
		case SDOtx:
		case SDOrx:
			if (d->CurrentCommunicationState.csSDO)
				proceedSDO(d,m);
			break;
		case NODE_GUARD:
			if (d->CurrentCommunicationState.csHeartbeat)
				proceedNODE_GUARD(d,m);
			break;
		case NMT:
			if (*(d->iam_a_slave))
			{
				proceedNMTstateChange(d,m);
			}
	}
}

#define StartOrStop(CommType, FuncStart, FuncStop) \
	if(newCommunicationState->CommType && !d->CurrentCommunicationState.CommType){\
		MSG_WAR(0x9999,#FuncStart, 9999);\
		d->CurrentCommunicationState.CommType = 1;\
		FuncStart;\
	}else if(!newCommunicationState->CommType && d->CurrentCommunicationState.CommType){\
		MSG_WAR(0x9999,#FuncStop, 9999);\
		d->CurrentCommunicationState.CommType = 0;\
		FuncStop;\
	}
#define None
	
/*****************************************************************************/
void switchCommunicationState(CO_Data* d, s_state_communication *newCommunicationState)
{
	StartOrStop(csSDO,	None,		resetSDO(d))
	StartOrStop(csSYNC,	startSYNC(d),		stopSYNC(d))
	StartOrStop(csHeartbeat,	heartbeatInit(d),	heartbeatStop(d))
/*	StartOrStop(Emergency,,) */
	StartOrStop(csPDO,	None,	None)
	StartOrStop(csBoot_Up,	None,	slaveSendBootUp(d))
}

/*****************************************************************************/
UNS8 setState(CO_Data* d, e_nodeState newState)
{
	UNS16 wIndex = 0x1F22;
	const indextable *ptrTable;
  	ODCallback_t *Callback;
	UNS32 errorCode;
	while(newState != d->nodeState){
		switch( newState ){
			case Initialisation:
			{
				s_state_communication newCommunicationState = {1, 0, 0, 0, 0, 0};
				/* This will force a second loop for the state switch */
				d->nodeState = Initialisation;
				newState = Pre_operational;
				switchCommunicationState(d, &newCommunicationState);
				/* call user app related state func. */
				(*d->initialisation)();
				
			}
			break;
								
			case Pre_operational:
			{
				
				s_state_communication newCommunicationState = {0, 1, 1, 1, 1, 0};
				d->nodeState = Pre_operational;
				newState = Pre_operational;
				switchCommunicationState(d, &newCommunicationState);
				if (!(*(d->iam_a_slave)))
				{
					ptrTable =(*d->scanIndexOD)(wIndex, &errorCode, &Callback);
  					
  					if (errorCode != OD_SUCCESSFUL)
  						{
  							(*d->preOperational)();
  						}
					else
						{
							UNS32 res;
							res = decompo_dcf(d,0x01);
						}				
				}
				else 
				{
					(*d->preOperational)();
				}
			}
			break;
								
			case Operational:
			if(d->nodeState == Initialisation) return 0xFF;
			{
				s_state_communication newCommunicationState = {0, 1, 1, 1, 1, 1};
				d->nodeState = Operational;
				newState = Operational;
				switchCommunicationState(d, &newCommunicationState);
				(*d->operational)();
			}
			break;
						
			case Stopped:
			if(d->nodeState == Initialisation) return 0xFF;
			{
				s_state_communication newCommunicationState = {0, 0, 0, 0, 1, 0};
				d->nodeState = Stopped;
				newState = Stopped;
				switchCommunicationState(d, &newCommunicationState);
				(*d->stopped)();
			}
			break;
			
			default:
				return 0xFF;
		}/* end switch case */
	
	}
	return 0;
}

/*****************************************************************************/
UNS8 getNodeId(CO_Data* d)
{
  return *d->bDeviceNodeId;
}

/*****************************************************************************/
void setNodeId(CO_Data* d, UNS8 nodeId)
{
  UNS16 offset = d->firstIndex->SDO_SVR;
  if(offset){
      /* cob_id_client = 0x600 + nodeId; */
      *(UNS32*)d->objdict[offset].pSubindex[1].pObject = 0x600 + nodeId;
      /* cob_id_server = 0x580 + nodeId; */
      *(UNS32*)d->objdict[offset].pSubindex[2].pObject = 0x580 + nodeId;
      /* node Id client. As we do not know the value, we put the node Id Server */
      /* *(UNS8*)d->objdict[offset].pSubindex[3].pObject = nodeId; */
  }

  /* ** Initialize the server(s) SDO parameters */
  /* Remember that only one SDO server is allowed, defined at index 0x1200 */
 
  /* ** Initialize the client(s) SDO parameters  */
  /* Nothing to initialize (no default values required by the DS 401) */
  /* ** Initialize the receive PDO communication parameters. Only for 0x1400 to 0x1403 */
  {
    UNS8 i = 0;
    UNS16 offset = d->firstIndex->PDO_RCV;
    UNS16 lastIndex = d->lastIndex->PDO_RCV;
    UNS32 cobID[] = {0x200, 0x300, 0x400, 0x500};
    if( offset ) while( (offset <= lastIndex) && (i < 4)) {
      //if(*(UNS32*)d->objdict[offset].pSubindex[1].pObject == cobID[i] + *d->bDeviceNodeId)
	      *(UNS32*)d->objdict[offset].pSubindex[1].pObject = cobID[i] + nodeId;
      i ++;
      offset ++;
    }
  }
  /* ** Initialize the transmit PDO communication parameters. Only for 0x1800 to 0x1803 */
  {
    UNS8 i = 0;
    UNS16 offset = d->firstIndex->PDO_TRS;
    UNS16 lastIndex = d->lastIndex->PDO_TRS;
    UNS32 cobID[] = {0x180, 0x280, 0x380, 0x480};
    i = 0;
    if( offset ) while ((offset <= lastIndex) && (i < 4)) {
      //if(*(UNS32*)d->objdict[offset].pSubindex[1].pObject == cobID[i] + *d->bDeviceNodeId)
	      *(UNS32*)d->objdict[offset].pSubindex[1].pObject = cobID[i] + nodeId;
      i ++;
      offset ++;
    }
  }
  /* bDeviceNodeId is defined in the object dictionary. */
  *d->bDeviceNodeId = nodeId;
}

void _initialisation(){}
void _preOperational(){}
void _operational(){}
void _stopped(){}
