/*
  This file is part of CanFestival, a library implementing CanOpen
  Stack.

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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
  USA
*/
#include "pdo.h"
#include "objacces.h"
#include "canfestival.h"

/*!
** @file   pdo.c
** @author Edouard TISSERANT and Francis DUPIN
** @date   Tue Jun  5 09:32:32 2007
**
** @brief
**
**
*/

/*!
**
**
** @param d
** @param TPDO_com TPDO communication parameters OD entry
** @param TPDO_map TPDO mapping parameters OD entry
**
** @return
**/

UNS8 buildPDO(CO_Data* d, UNS8 numPdo, Message *pdo)
{
	const indextable* TPDO_com = d->objdict + d->firstIndex->PDO_TRS + numPdo; 
	const indextable* TPDO_map = d->objdict + d->firstIndex->PDO_TRS_MAP + numPdo;
	
	UNS8 prp_j = 0x00;
	UNS8 offset = 0x00;
	const UNS8* pMappingCount = (UNS8*) TPDO_map->pSubindex[0].pObject;

	pdo->cob_id.w = *(UNS32*)TPDO_com->pSubindex[1].pObject;
	pdo->rtr = NOT_A_REQUEST;

	MSG_WAR(0x3009, "  PDO CobId is : ", *(UNS32*)TPDO_com->pSubindex[1].pObject);
	MSG_WAR(0x300D, "  Number of objects mapped : ",*pMappingCount );
	
	do{
		UNS8 dataType; /* Unused */
		UNS8 tmp[]= {0,0,0,0,0,0,0,0}; /* temporary space to hold bits */

		/* pointer fo the var which holds the mapping parameter of an mapping entry  */
		UNS32* pMappingParameter = (UNS32*) TPDO_map->pSubindex[prp_j + 1].pObject;
		UNS16 index = (UNS16)((*pMappingParameter) >> 16);
		UNS8 Size = (UNS8)(*pMappingParameter); /* Size in bits */
		UNS8 ByteSize = 1 + ((Size - 1) >> 3); /*1->8 => 1 ; 9->16 => 2, ... */
		UNS8 subIndex = (UNS8)(( (*pMappingParameter) >> (UNS8)8 ) & (UNS32)0x000000FF);
		
		MSG_WAR(0x300F, "  got mapping parameter : ", *pMappingParameter);
		MSG_WAR(0x3050, "    at index : ", TPDO_map->index);
		MSG_WAR(0x3051, "    sub-index : ", prp_j + 1);
		
		if( getODentry(d, index, subIndex, tmp, &ByteSize, &dataType, 0 ) != OD_SUCCESSFUL ){
			MSG_ERR(0x1013, " Couldn't find mapped variable at index-subindex-size : ", (UNS16)(*pMappingParameter));
			return 0xFF;
		}
		/* copy bit per bit in little endian*/
		CopyBits(Size, ((UNS8*)tmp), 0 , 0, (UNS8*)&pdo->data[offset>>3], offset%8, 0);

		offset += Size ;
		prp_j++;
	}while( prp_j < *pMappingCount );

	pdo->len = 1 + ((offset - 1) >> 3);

	MSG_WAR(0x3015, "  End scan mapped variable", 0);

	return 0;
}

/*!
**
**
** @param d
** @param cobId
**
** @return
**/
UNS8 sendPDOrequest( CO_Data* d, UNS32 cobId )
{
  UNS32 * pwCobId;
  UNS16          offset;
  UNS16          lastIndex;
  UNS8           err;

  MSG_WAR(0x3930, "sendPDOrequest ",0);
  /* Sending the request only if the cobid have been found on the PDO
     receive */
  /* part dictionary */
  offset = d->firstIndex->PDO_RCV;
  lastIndex = d->lastIndex->PDO_RCV;
  if (offset)
    while (offset <= lastIndex) {
      /* get the CobId*/
      pwCobId = (UNS32*) d->objdict[offset].pSubindex[1].pObject;

      if ( *pwCobId  == cobId ) {
        Message pdo = {*pwCobId, REQUEST, 0};
	return canSend(d->canHandle,&pdo);
      }
      offset++;
    }
  MSG_WAR(0x1931, "sendPDOrequest : COBID not found : ", cobId);
  return 0xFF;
}


/*!
**
**
** @param d
** @param m
**
** @return
**/
UNS8 proceedPDO(CO_Data* d, Message *m)
{
  UNS8   numPdo;
  UNS8   numMap;  /* Number of the mapped varable */
  UNS8 i;
  UNS8 *     pMappingCount = NULL;    /* count of mapped objects... */
  /* pointer to the var which is mapped to a pdo... */
  /*  void *     pMappedAppObject = NULL;   */
  /* pointer fo the var which holds the mapping parameter of an
     mapping entry */
  UNS32 *    pMappingParameter = NULL;
  UNS8  *    pTransmissionType = NULL; /* pointer to the transmission
                                         type */
  UNS32 *    pwCobId = NULL;
  UNS8       Size;
  UNS8       dataType;
  UNS8       offset;
  UNS8       status;
  UNS32      objDict;
  UNS16      offsetObjdict;
  UNS16      lastIndex;
  
  status = state2;

  MSG_WAR(0x3935, "proceedPDO, cobID : ", ((*m).cob_id.w & 0x7ff));
  offset = 0x00;
  numPdo = 0;
  numMap = 0;
  if((*m).rtr == NOT_A_REQUEST ) { /* The PDO received is not a
                                     request. */

    offsetObjdict = d->firstIndex->PDO_RCV;
    lastIndex = d->lastIndex->PDO_RCV;

    /* study of all the PDO stored in the dictionary */
    if(offsetObjdict)
      while (offsetObjdict <= lastIndex) {

        switch( status ) {

        case state2:
          /* get CobId of the dictionary correspondant to the received
             PDO */
          pwCobId = (UNS32*) d->objdict[offsetObjdict].pSubindex[1].pObject;
          /* check the CobId coherance */
          /*pwCobId is the cobId read in the dictionary at the state 3
            */
          if ( *pwCobId == (*m).cob_id.w ){
            /* The cobId is recognized */
            status = state4;
            MSG_WAR(0x3936, "cobId found at index ", 0x1400 + numPdo);
            break;
          }
          else {
            /* cobId received does not match with those write in the
              dictionnary */
            numPdo++;
            offsetObjdict++;
            status = state2;
            break;
          }

            case state4:/* Get Mapped Objects Number */
               /* The cobId of the message received has been found in the
                 dictionnary. */
               offsetObjdict = d->firstIndex->PDO_RCV_MAP;
             lastIndex = d->lastIndex->PDO_RCV_MAP;
             pMappingCount = (UNS8*) (d->objdict + offsetObjdict + numPdo)->pSubindex[0].pObject;
             numMap = 0;
             while (numMap < *pMappingCount) {
               UNS8 tmp[]= {0,0,0,0,0,0,0,0};
               UNS8 ByteSize;
               pMappingParameter = (UNS32*) (d->objdict + offsetObjdict + numPdo)->pSubindex[numMap + 1].pObject;
               if (pMappingParameter == NULL) {
                 MSG_ERR(0x1937, "Couldn't get mapping parameter : ", numMap + 1);
                 return 0xFF;
               }
               /* Get the addresse of the mapped variable. */
               /* detail of *pMappingParameter : */
               /* The 16 hight bits contains the index, the medium 8 bits
                 contains the subindex, */
               /* and the lower 8 bits contains the size of the mapped
                 variable. */

               Size = (UNS8)(*pMappingParameter);

               /* copy bit per bit in little endian */
               CopyBits(Size, (UNS8*)&m->data[offset>>3], offset%8, 0, ((UNS8*)tmp), 0, 0);

               ByteSize = 1 + ((Size - 1) >> 3); /*1->8 => 1 ; 9->16 =>
                                                   2, ... */

               objDict = setODentry(d, (UNS16)((*pMappingParameter) >> 16),
                                    (UNS8)(((*pMappingParameter) >> 8 ) & 0xFF),
                                 tmp, &ByteSize, 0 );

               if(objDict != OD_SUCCESSFUL) {
                 MSG_ERR(0x1938, "error accessing to the mapped var : ", numMap + 1);
                 MSG_WAR(0x2939, "         Mapped at index : ", (*pMappingParameter) >> 16);
                 MSG_WAR(0x2940, "                subindex : ", ((*pMappingParameter) >> 8 ) & 0xFF);
                 return 0xFF;
               }

               MSG_WAR(0x3942, "Variable updated with value received by PDO cobid : ", m->cob_id.w);
               MSG_WAR(0x3943, "         Mapped at index : ", (*pMappingParameter) >> 16);
               MSG_WAR(0x3944, "                subindex : ", ((*pMappingParameter) >> 8 ) & 0xFF);
               /* MSG_WAR(0x3945, "                data : ",*((UNS32*)pMappedAppObject)); */
               offset += Size;
               numMap++;
               /*TODO :  check that offset is not not greater that message size (in bit) */
             } /* end loop while on mapped variables */

             offset=0x00;
             numMap = 0;
             return 0;

        }/* end switch status*/
      }/* end while*/
  }/* end if Donnees */
  else if ((*m).rtr == REQUEST ){
    MSG_WAR(0x3946, "Receive a PDO request cobId : ", m->cob_id.w);
    status = state1;
    offsetObjdict = d->firstIndex->PDO_TRS;
    lastIndex = d->lastIndex->PDO_TRS;
    if(offsetObjdict) while( offsetObjdict  <= lastIndex ){
      /* study of all PDO stored in the objects dictionary */

      switch( status ){

      case state1:/* check the CobId */
        /* get CobId of the dictionary which match to the received PDO
         */
        pwCobId = (UNS32*) (d->objdict + offsetObjdict)->pSubindex[1].pObject;
        if ( *pwCobId == (*m).cob_id.w ) {
          status = state4;
          break;
        }
        else {
          numPdo++;
          offsetObjdict++;
        }
        status = state1;
        break;


      case state4:/* check transmission type (after request?) */
        pTransmissionType = (UNS8*) d->objdict[offsetObjdict].pSubindex[2].pObject;
        if ( (*pTransmissionType == TRANS_RTR) ||
             (*pTransmissionType == TRANS_RTR_SYNC )) {
          status = state5;
          break;
        }else if(
             (*pTransmissionType == TRANS_EVENT_PROFILE) ||
             (*pTransmissionType == TRANS_EVENT_SPECIFIC) ) {
	  /* Zap all timers and inhibit flag */
	  d->PDO_status[numPdo].event_timer = DelAlarm(d->PDO_status[numPdo].event_timer);
	  d->PDO_status[numPdo].inhibit_timer = DelAlarm(d->PDO_status[numPdo].inhibit_timer);
  	  d->PDO_status[numPdo].transmit_type_parameter &= ~PDO_INHIBITED;
  	  /* Call  PDOEventTimerAlarm for this TPDO, this will trigger emission et reset timers */
          PDOEventTimerAlarm(d, numPdo);
          return 0;
        }else {
          /* The requested PDO is not to send on request. So, does
            nothing. */
          MSG_WAR(0x2947, "PDO is not to send on request : ", m->cob_id.w);
          return 0xFF;
        }

      case state5:/* build and send requested PDO */
      {
      	Message pdo;
    	if( buildPDO(d, numPdo, &pdo))
    	{
          MSG_ERR(0x1013, " Couldn't find mapped variable at index-subindex-size : ", (UNS16)(*pMappingParameter));
          //return 0xFF; /*No real reason to stop...*/
    	}
    	canSend(d->canHandle,&pdo);
        return 0;
      }
      }/* end switch status */
    }/* end while */
  }/* end if Requete */

  return 0;
}

/*!
**
**
** @param NbBits
** @param SrcByteIndex
** @param SrcBitIndex
** @param SrcBigEndian
** @param DestByteIndex
** @param DestBitIndex
** @param DestBigEndian
**/
void CopyBits(UNS8 NbBits, UNS8* SrcByteIndex, UNS8 SrcBitIndex, UNS8 SrcBigEndian, UNS8* DestByteIndex, UNS8 DestBitIndex, UNS8 DestBigEndian)
{
  /* This loop copy as many bits that it can each time, crossing*/
  /* successively bytes*/
  // boundaries from LSB to MSB.
  while(NbBits > 0)
    {
      /* Bit missalignement between src and dest*/
      INTEGER8 Vect = DestBitIndex - SrcBitIndex;

      /* We can now get src and align it to dest*/
      UNS8 Aligned = Vect>0 ? *SrcByteIndex << Vect : *SrcByteIndex >> -Vect;

      /* Compute the nb of bit we will be able to copy*/
      UNS8 BoudaryLimit = (Vect>0 ? 8 - DestBitIndex :  8 - SrcBitIndex );
      UNS8 BitsToCopy = BoudaryLimit > NbBits ? NbBits : BoudaryLimit;

      /* Create a mask that will serve in:*/
      UNS8 Mask = ((0xff << (DestBitIndex + BitsToCopy)) | (0xff >> (8 - DestBitIndex)));

      /* - Filtering src*/
      UNS8 Filtered = Aligned & ~Mask;

      /* - and erase bits where we write, preserve where we don't*/
      *DestByteIndex &= Mask;

      /* Then write.*/
      *DestByteIndex |= Filtered ;

      /*Compute next time cursors for src*/
      if((SrcBitIndex += BitsToCopy)>7)/* cross boundary ?*/
        {
          SrcBitIndex = 0;/* First bit*/
          SrcByteIndex += (SrcBigEndian ? -1 : 1);/* Next byte*/
        }


      /*Compute next time cursors for dest*/
      if((DestBitIndex += BitsToCopy)>7)
        {
          DestBitIndex = 0;/* First bit*/
          DestByteIndex += (DestBigEndian ? -1 : 1);/* Next byte*/
        }

      /*And decrement counter.*/
      NbBits -= BitsToCopy;
    }

}
/*!
**
**
** @param d
**
** @return
**/

UNS8 sendPDOevent( CO_Data* d)
{
  /* Calls _sendPDOevent specifying it is not a sync event */
  return _sendPDOevent(d, 0);
}
	

void PDOEventTimerAlarm(CO_Data* d, UNS32 pdoNum)
{
	printf("EV PDOEventTimerAlarm : %d\n", pdoNum);
	
	/* This is needed to avoid deletion of re-attribuated timer */
	d->PDO_status[pdoNum].event_timer = TIMER_NONE;
	/* force emission of PDO by artificially changing last emitted*/
	d->PDO_status[pdoNum].last_message.cob_id.w = 0;
	_sendPDOevent( d, 0 ); /* not a Sync Event*/	
}

void PDOInhibitTimerAlarm(CO_Data* d, UNS32 pdoNum)
{
	printf("EV PDOInhibitTimerAlarm : %d\n", pdoNum);

	/* This is needed to avoid deletion of re-attribuated timer */
	d->PDO_status[pdoNum].inhibit_timer = TIMER_NONE;
	/* Remove inhibit flag */
	d->PDO_status[pdoNum].transmit_type_parameter &= ~PDO_INHIBITED;
	_sendPDOevent( d, 0 ); /* not a Sync Event*/
}

/*!
**
**
** @param d
** @param isSyncEvent
**
** @return
**/

UNS8 _sendPDOevent( CO_Data* d, UNS8 isSyncEvent )
{ 
  UNS8 	pdoNum = 0x00;       /* number of the actual processed pdo-nr. */
  UNS8* pTransmissionType = NULL;  
  UNS8 status = state3;
  UNS16 offsetObjdict = d->firstIndex->PDO_TRS;
  UNS16 offsetObjdictMap = d->firstIndex->PDO_TRS_MAP;
  UNS16 lastIndex = d->lastIndex->PDO_TRS;  

  /* study all PDO stored in the objects dictionary */	
  if(offsetObjdict){
   Message clean = Message_Initializer;
   Message pdo = Message_Initializer;
   while( offsetObjdict <= lastIndex) {  
    switch( status ) {
    case state3:    /* get the PDO transmission type */
      if (d->objdict[offsetObjdict].bSubCount <= 2) {
	  MSG_ERR(0x1004, "Subindex 2  not found at index ", 0x1800 + pdoNum);
	  return 0xFF;
	}
      pTransmissionType = (UNS8*) d->objdict[offsetObjdict].pSubindex[2].pObject;    
      MSG_WAR(0x3005, "Reading PDO at index : ", 0x1800 + pdoNum);

      /* check if transmission type is after (this) SYNC */
      /* The message may not be transmited every SYNC but every n SYNC */      
      if( isSyncEvent && 
      	  (*pTransmissionType >= TRANS_SYNC_MIN) &&
      	  (*pTransmissionType <= TRANS_SYNC_MAX) &&
          (++d->PDO_status[pdoNum].transmit_type_parameter == *pTransmissionType) ) {	
	d->PDO_status[pdoNum].transmit_type_parameter = 0;
	MSG_WAR(0x3007, "  PDO is on SYNCHRO. Trans type : ", *pTransmissionType);
	pdo = clean;
        if(buildPDO(d, pdoNum, &pdo))
        {
            MSG_ERR(0x3006, " Couldn't build TPDO number : ", pdoNum);
	    status = state11;
	    break;
        }
	status = state5;
      }
      /* If transmission on Event and not inhibited, check for changes */
      else if((*pTransmissionType == TRANS_EVENT_PROFILE ||
               *pTransmissionType == TRANS_EVENT_SPECIFIC )&&
              !(d->PDO_status[pdoNum].transmit_type_parameter & PDO_INHIBITED)) {
	MSG_WAR(0x3008, "  PDO is on EVENT. Trans type : ", *pTransmissionType);
	pdo = pdo = clean;
        if(buildPDO(d, pdoNum, &pdo))
        {
            MSG_ERR(0x3007, " Couldn't build TPDO number : ", pdoNum);
	    status = state11;
	    break;
        }
        
	/*Compare new and old PDO*/
	if(d->PDO_status[pdoNum].last_message.cob_id.w == pdo.cob_id.w &&
	   d->PDO_status[pdoNum].last_message.len == pdo.len &&
	   *(UNS64*)(&d->PDO_status[pdoNum].last_message.data[0]) == *(UNS64*)(&pdo.data[0])){
	   	/* No changes -> go to next pdo*/
		status = state11;
	}else{
		MSG_WAR(0x3008, "Changes TPDO number : ", pdoNum);
		printf("EV Changes TPDO number : %d\n", pdoNum);
		/* Changes detected -> transmit message */
		status = state5;
		
		/* Start both event_timer and inhibit_timer*/
		DelAlarm(d->PDO_status[pdoNum].event_timer);
		d->PDO_status[pdoNum].event_timer = SetAlarm(d, pdoNum, &PDOEventTimerAlarm, MS_TO_TIMEVAL(*(UNS16*)d->objdict[offsetObjdict].pSubindex[5].pObject), 0);
		
		DelAlarm(d->PDO_status[pdoNum].inhibit_timer);
		d->PDO_status[pdoNum].inhibit_timer = SetAlarm(d, pdoNum, &PDOInhibitTimerAlarm, US_TO_TIMEVAL(*(UNS16*)d->objdict[offsetObjdict].pSubindex[3].pObject * 100), 0);
		
		/* and inhibit TPDO */
		d->PDO_status[pdoNum].transmit_type_parameter |= PDO_INHIBITED;
	}
      }else{
	MSG_WAR(0x3009, "  PDO is not on EVENT or synchro or not at this SYNC. Trans type : ", *pTransmissionType);
	status = state11;
      }      
        break;
    case state5: /*Send the pdo*/
	/*store_as_last_message*/
	d->PDO_status[pdoNum].last_message = pdo;	
	MSG_WAR(0x3901, "sendPDO cobId :", pdo.cob_id.w);
	MSG_WAR(0x3902,  "     Nb octets  : ",  pdo.len);
	{int i;
	for (i = 0 ; i < pdo.len ; i++) {
		MSG_WAR(0x3903,"           data : ", pdo.data[i]);
	}}
    	
    	canSend(d->canHandle,&pdo);
	status = state11;
	break;     
    case state11: /*Go to next TPDO*/     
	pdoNum++;
	offsetObjdict++;
	offsetObjdictMap++;
	MSG_WAR(0x3017, "next pdo index : ", pdoNum);
	status = state3;
	break;
      
    default:
      MSG_ERR(0x1019,"Unknown state has been reached : %d",status);
      return 0xFF;
    }/* end switch case */
    
  }/* end while */
  }
  return 0;
}

void PDOInit(CO_Data* d)
{
	
	/* TODO: implement callbacks on 140xh
	 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXx
	 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXx
	 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXx
	 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXx
	 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXx
	 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXx
	 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXx
	 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXx
	*/	
	
  _sendPDOevent(d, 0 );
}

void PDOStop(CO_Data* d)
{
  UNS8 	pdoNum = 0x00;       /* number of the actual processed pdo-nr. */
  UNS16 offsetObjdict = d->firstIndex->PDO_TRS;
  UNS16 lastIndex = d->lastIndex->PDO_TRS;
  if(offsetObjdict) while( offsetObjdict <= lastIndex) {
	d->PDO_status[pdoNum].event_timer = DelAlarm(d->PDO_status[pdoNum].event_timer);
	d->PDO_status[pdoNum].inhibit_timer = DelAlarm(d->PDO_status[pdoNum].inhibit_timer);
	d->PDO_status[pdoNum].transmit_type_parameter = 0;
	d->PDO_status[pdoNum].last_message.cob_id.w = 0;
	pdoNum++;
	offsetObjdict++;
  }  
}
