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

/*!
** @file   sync.c
** @author Edouard TISSERANT and Francis DUPIN
** @date   Tue Jun  5 09:32:32 2007
**
** @brief
**
**
*/

#include "data.h"
#include "sync.h"
#include "canfestival.h"

/* Prototypes for internals functions */

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param id                                                                                       
**/  
void SyncAlarm(CO_Data* d, UNS32 id);
UNS32 OnCOB_ID_SyncUpdate(CO_Data* d, const indextable * unsused_indextable, 
	UNS8 unsused_bSubindex);

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param id                                                                                       
**/   
void SyncAlarm(CO_Data* d, UNS32 id)
{
	sendSYNC(d, *d->COB_ID_Sync & 0x1FFFFFFF) ;
}

/*!                                                                                                
** This is called when Index 0x1005 is updated.                                                                                                
**                                                                                                 
** @param d                                                                                        
** @param unsused_indextable                                                                       
** @param unsused_bSubindex                                                                        
**                                                                                                 
** @return                                                                                         
**/  
UNS32 OnCOB_ID_SyncUpdate(CO_Data* d, const indextable * unsused_indextable, UNS8 unsused_bSubindex)
{
	startSYNC(d);
	return 0;
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
**/ 
void startSYNC(CO_Data* d)
{
	RegisterSetODentryCallBack(d, 0x1005, 0, &OnCOB_ID_SyncUpdate);
	RegisterSetODentryCallBack(d, 0x1006, 0, &OnCOB_ID_SyncUpdate);

	if(d->syncTimer != TIMER_NONE){
		stopSYNC(d);
	}
	
	if(*d->COB_ID_Sync & 0x40000000 && *d->Sync_Cycle_Period)
	{
		d->syncTimer = SetAlarm(
				d,
				0 /*No id needed*/,
				&SyncAlarm,
				US_TO_TIMEVAL(*d->Sync_Cycle_Period), 
				US_TO_TIMEVAL(*d->Sync_Cycle_Period));
	}
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
**/   
void stopSYNC(CO_Data* d)
{
	d->syncTimer = DelAlarm(d->syncTimer);
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param cob_id                                                                                   
**                                                                                                 
** @return                                                                                         
**/  
UNS8 sendSYNC(CO_Data* d, UNS32 cob_id)
{
  Message m;
  UNS8 resultat ;
  
  MSG_WAR(0x3001, "sendSYNC ", 0);
  
  m.cob_id.w = cob_id ;
  m.rtr = NOT_A_REQUEST;
  m.len = 0;
  resultat = canSend(d->canHandle,&m) ;
  proceedSYNC(d, &m) ; 
  return resultat ;
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param m                                                                                        
**                                                                                                 
** @return                                                                                         
**/ 
UNS8 proceedSYNC(CO_Data* d, Message *m)
{

  UNS8 	pdoNum,       /* number of the actual processed pdo-nr. */
        prp_j;

  const UNS8 *     pMappingCount = NULL;      /* count of mapped objects...*/
  /* pointer to the var which is mapped to a pdo */
  /* void *     pMappedAppObject = NULL; */
  /* pointer fo the var which holds the mapping parameter of an mapping entry  */
  UNS32 *    pMappingParameter = NULL;  
  /* pointer to the transmissiontype...*/
  UNS8 *     pTransmissionType = NULL;  
  UNS32 *    pwCobId = NULL;	

  UNS8 dataType;
  UNS16 index;
  UNS8 subIndex;
  UNS8 offset;
  UNS8 status;
  UNS8 Size;
  UNS32 objDict;	
  UNS16 offsetObjdict;
  UNS16 offsetObjdictMap;
  UNS16 lastIndex;
  
  status = state3;
  pdoNum = 0x00;
  prp_j = 0x00;
  offset = 0x00;
  
  MSG_WAR(0x3002, "SYNC received. Proceed. ", 0);
  
  (*d->post_sync)();

  /* only operational state allows PDO transmission */
  if( d->nodeState != Operational ) 
    return 0;
  
  /* So, the node is in operational state */
  /* study all PDO stored in the objects dictionary */	
 
  offsetObjdict = d->firstIndex->PDO_TRS;
  lastIndex = d->lastIndex->PDO_TRS;
  offsetObjdictMap = d->firstIndex->PDO_TRS_MAP;
  
  if(offsetObjdict) while( offsetObjdict <= lastIndex) {  
    switch( status ) {
                    
    case state3:    /* get the PDO transmission type */
      if (d->objdict[offsetObjdict].bSubCount <= 2) {
	  MSG_ERR(0x1004, "Subindex 2  not found at index ", 0x1800 + pdoNum);
	  return 0xFF;
	}
      pTransmissionType = (UNS8*) d->objdict[offsetObjdict].pSubindex[2].pObject;    
      MSG_WAR(0x3005, "Reading PDO at index : ", 0x1800 + pdoNum);
      status = state4; 
      break;     
    case state4:	/* check if transmission type is after (this) SYNC */
                        /* The message may not be transmited every SYNC but every n SYNC */      
      if( (*pTransmissionType >= TRANS_SYNC_MIN) && (*pTransmissionType <= TRANS_SYNC_MAX) &&
          (++d->count_sync[pdoNum] == *pTransmissionType) ) {	
	d->count_sync[pdoNum] = 0;
	MSG_WAR(0x3007, "  PDO is on SYNCHRO. Trans type : ", *pTransmissionType);
	status = state5;
	break;
      }
      else {
	MSG_WAR(0x3008, "  Not on synchro or not at this SYNC. Trans type : ", 
		*pTransmissionType);
	pdoNum++;
	offsetObjdict++;
	offsetObjdictMap++;
	status = state11;
	break;
      }      
    case state5:	/* get PDO CobId */
        pwCobId = (UNS32*) d->objdict[offsetObjdict].pSubindex[1].pObject;     
	MSG_WAR(0x3009, "  PDO CobId is : ", *pwCobId);
	status = state7;
	break;     
    case state7:  /* get mapped objects number to transmit with this PDO */
      pMappingCount = (UNS8*) d->objdict[offsetObjdictMap].pSubindex[0].pObject;
	MSG_WAR(0x300D, "  Number of objects mapped : ",*pMappingCount );
	status = state8;
    case state8:	/* get mapping parameters */
      pMappingParameter = (UNS32*) d->objdict[offsetObjdictMap].pSubindex[prp_j + 1].pObject;
	MSG_WAR(0x300F, "  got mapping parameter : ", *pMappingParameter);
	MSG_WAR(0x3050, "    at index : ", 0x1A00 + pdoNum);
	MSG_WAR(0x3051, "    sub-index : ", prp_j + 1);
	status = state9;
    
    case state9:	/* get data to transmit */ 
	{
	  UNS8 ByteSize;
	  UNS8 tmp[]= {0,0,0,0,0,0,0,0};
	  index = (UNS16)((*pMappingParameter) >> 16);
          subIndex = (UNS8)(( (*pMappingParameter) >> (UNS8)8 ) & (UNS32)0x000000FF);
	  Size = (UNS8)(*pMappingParameter); /* Size in bits */
	  ByteSize = 1 + ((Size - 1) >> 3); /*1->8 => 1 ; 9->16 => 2, ... */
	  objDict = getODentry(d, index, subIndex, tmp, &ByteSize, &dataType, 0 );
	  /* copy bit per bit in little endian*/
	  CopyBits(Size, ((UNS8*)tmp), 0 , 0, (UNS8*)&d->process_var.data[offset>>3], offset%8, 0);
	}   
        if( objDict != OD_SUCCESSFUL ){
          MSG_ERR(0x1013, " Couldn't find mapped variable at index-subindex-size : ", (UNS16)(*pMappingParameter));
          return 0xFF;
        }
	
	offset += Size ;
	d->process_var.count = 1 + ((offset - 1) >> 3);
	prp_j++;
	status = state10;	 
	break;					
      
    case state10:	/* loop to get all the data to transmit */
      if( prp_j < *pMappingCount ){
	MSG_WAR(0x3014, "  next variable mapped : ", prp_j);
	status = state8;
	break;
      }
      else {
	MSG_WAR(0x3015, "  End scan mapped variable", 0);
	PDOmGR( d, *pwCobId );	
	MSG_WAR(0x3016, "  End of this pdo. Should have been sent", 0);
	pdoNum++;
	offsetObjdict++;
	offsetObjdictMap++;
	offset = 0x00;
	prp_j = 0x00;
	status = state11;
	break;
      }
      
    case state11:     
      MSG_WAR(0x3017, "next pdo index : ", pdoNum);
      status = state3;
      break;
      
    default:
      MSG_ERR(0x1019,"Unknown state has been reached : %d",status);
      return 0xFF;
    }/* end switch case */
    
  }/* end while( prp_i<dict_cstes.max_count_of_PDO_transmit ) */
   
  (*d->post_TPDO)();

  return 0;
}


void _post_sync(){}
void _post_TPDO(){}
