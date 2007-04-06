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
#include "pdo.h"
#include "objacces.h"
#include "canfestival.h"

/****************************************************************************/
UNS8 sendPDO(CO_Data* d, s_PDO pdo, UNS8 req)
{
  UNS8 i;
  if( d->nodeState == Operational ) {
    Message m;

    /* Message copy for sending */
    m.cob_id.w = pdo.cobId & 0x7FF; /* Because the cobId is 11 bytes length */
    if ( req == NOT_A_REQUEST ) {
      UNS8 i;
      m.rtr = NOT_A_REQUEST;
      m.len = pdo.len;
      /* memcpy(&m.data, &pdo.data, m.len); */
      /* This Memcpy depends on packing structure. Avoid */
      for (i = 0 ; i < pdo.len ; i++)
	m.data[i] = pdo.data[i];
    }
    else {
      m.rtr = REQUEST;
      m.len = 0;
    }

    MSG_WAR(0x3901, "sendPDO cobId :", m.cob_id.w);
    MSG_WAR(0x3902,  "     Nb octets  : ",  m.len);
    for (i = 0 ; i < m.len ; i++) {
      MSG_WAR(0x3903,"           data : ", m.data[i]);
    }
		  
    return canSend(d->canHandle,&m);
  } /* end if */
  return 0xFF;
}

/***************************************************************************/
UNS8 PDOmGR(CO_Data* d, UNS32 cobId) /* PDO Manager */
{
  UNS8 res;
  UNS8 i;
  s_PDO pdo;

  MSG_WAR(0x3905, "PDOmGR",0);
	
  /* if PDO is waiting for transmission,
     preparation of the message to send */
    pdo.cobId = cobId;
    pdo.len =  d->process_var.count;
    /* memcpy(&(pdo.data), &(process_var.data), pdo.len); */
    /* Ce memcpy devrait être portable */
    for ( i = 0 ; i < pdo.len ; i++) 
      pdo.data[i] = d->process_var.data[i];

    res = sendPDO(d, pdo, NOT_A_REQUEST);

    return res;
}

/**************************************************************************/
UNS8 buildPDO(CO_Data* d, UNS16 index)
{ /* DO NOT USE MSG_ERR because the macro may send a PDO -> infinite loop if it fails. */	
  UNS16 ind;
  UNS8      subInd;

  UNS8 *     pMappingCount = NULL;      /* count of mapped objects... */
  /* pointer to the var which is mapped to a pdo */
/*  void *     pMappedAppObject = NULL;  */
  /* pointer fo the var which holds the mapping parameter of an mapping entry  */ 
  UNS32 *    pMappingParameter = NULL;  

  UNS8      Size;
  UNS8      dataType;
  UNS8      offset;
  UNS16     offsetObjdict;
  UNS16     offsetObjdictPrm;
  UNS32     objDict;

  subInd=(UNS8)0x00;
  offset = 0x00;
  ind = index - 0x1800;
  
  MSG_WAR(0x3910,"Prepare PDO to send index :", index);

  /* only operational state allows PDO transmission */
  if( d->nodeState != Operational ) {
    MSG_WAR(0x2911, "Unable to send the PDO (node not in OPERATIONAL mode). Node : ", index);
    return 0xFF;
  }
  offsetObjdictPrm = d->firstIndex->PDO_TRS;
  offsetObjdict = d->firstIndex->PDO_TRS_MAP;
  
  if (offsetObjdictPrm && offsetObjdict)
  {
	  /* get mapped objects number to transmit with this PDO */
	  pMappingCount = (d->objdict + offsetObjdict + ind)->pSubindex[0].pObject;
	  MSG_WAR(0x3912, "Nb maped objects : ",* pMappingCount);
	  MSG_WAR(0x3913, "        at index : ", 0x1A00 + ind);
	  while (subInd < *pMappingCount) { /* Loop on mapped variables */
	    /* get mapping parameters */
	    pMappingParameter = (d->objdict + offsetObjdict + ind)->pSubindex[subInd + 1].pObject;
	    MSG_WAR(0x3914, "Get the mapping      at index : ", (UNS16)0x1A00 + ind);
	    MSG_WAR(0x3915, "                     subIndex : ", subInd + 1);
	    MSG_WAR(0x3916, "                     value    : ", *(UNS32 *)pMappingParameter);
	    /* Get the mapped variable */
	     Size = ((UNS8)(((*pMappingParameter) & 0xFF) >> 3));
	     objDict = getODentry(d, (UNS16)((*pMappingParameter) >> 16),
				    (UNS8)(((*pMappingParameter) >> 8 ) & 0x000000FF),
				    (void *)&d->process_var.data[offset], &Size, &dataType, 0 ); 

	     if (objDict != OD_SUCCESSFUL) {
	        MSG_WAR(0x2919, "error accessing to the mapped var : ", subInd + 1);  
		MSG_WAR(0x2920, "         Mapped at index : ", (*pMappingParameter) >> 16);
		MSG_WAR(0x2921, "                subindex : ", ((*pMappingParameter) >> 8 ) & 0xFF);
		return 0xFF;
	     } 

	      offset += Size;
	      d->process_var.count = offset;
	      subInd++;					
	}/* end Loop on mapped variables  */
  }
  return 0;
}

/**************************************************************************/
UNS8 sendPDOrequest( CO_Data* d, UNS32 cobId )
{		
  UNS32 *	 pwCobId;	
  UNS16          offset;
  UNS16          lastIndex;
  UNS8           err;

  MSG_WAR(0x3930, "sendPDOrequest ",0);  
  /* Sending the request only if the cobid have been found on the PDO receive */
  /* part dictionary */
  offset = d->firstIndex->PDO_RCV;
  lastIndex = d->lastIndex->PDO_RCV;
  if (offset)
	  while (offset <= lastIndex) {
	    /*get the CobId*/
	    pwCobId = d->objdict[offset].pSubindex[1].pObject;
	      
	    if ( *pwCobId  == cobId ) {
	      s_PDO pdo;
	      pdo.cobId = *pwCobId;
	      pdo.len = 0;
	      err  = sendPDO(d, pdo, REQUEST);	
	      return err;
	    }
	    offset++;
	  }
  MSG_WAR(0x1931, "sendPDOrequest : COBID not found : ", cobId); 
  return 0xFF;
}



/***********************************************************************/
UNS8 proceedPDO(CO_Data* d, Message *m)
{		
  UNS8   numPdo;
  UNS8   numMap;  /* Number of the mapped varable */                      
  UNS8 i;
  UNS8 *     pMappingCount = NULL;    /* count of mapped objects... */
  /* pointer to the var which is mapped to a pdo... */
/*  void *     pMappedAppObject = NULL;   */
  /* pointer fo the var which holds the mapping parameter of an mapping entry */
  UNS32 *    pMappingParameter = NULL;  
  UNS8  *    pTransmissionType = NULL; /* pointer to the transmission type */
  UNS32 *    pwCobId = NULL;
  UNS8       Size;
  UNS8       dataType;
  UNS8       offset;
  UNS8       status;
  UNS32      objDict;
  UNS16      offsetObjdict;
  UNS16      lastIndex;
  status = state1;

  MSG_WAR(0x3935, "proceedPDO, cobID : ", ((*m).cob_id.w & 0x7ff)); 
  offset = 0x00;
  numPdo = 0;
  numMap = 0;
  if((*m).rtr == NOT_A_REQUEST ) { /* The PDO received is not a request. */
    offsetObjdict = d->firstIndex->PDO_RCV;
    lastIndex = d->lastIndex->PDO_RCV;

    /* study of all the PDO stored in the dictionary */   
    if(offsetObjdict)
	    while (offsetObjdict <= lastIndex) {
					
	      switch( status ) {
						
	        case state1:	/* data are stored in process_var array */
		  /* memcpy(&(process_var.data), &m->data, (*m).len); */
		  /* Ce memcpy devrait être portable. */
		  for ( i = 0 ; i < m->len ; i++) 
		    d->process_var.data[i] = m->data[i];
		  d->process_var.count = (*m).len;
	
		  status = state2; 
		  break;
	
		case state2:
		  /* get CobId of the dictionary correspondant to the received PDO */
	          pwCobId = d->objdict[offsetObjdict].pSubindex[1].pObject;
		  /* check the CobId coherance */
		  /*pwCobId is the cobId read in the dictionary at the state 3 */
		  if ( *pwCobId == (*m).cob_id.w ){
		    /* The cobId is recognized */
		    status = state4;
		    MSG_WAR(0x3936, "cobId found at index ", 0x1400 + numPdo);
		    break;
		  }
		  else {
		    /* cobId received does not match with those write in the dictionnary */
		    numPdo++;
		    offsetObjdict++;
		    status = state2;
		    break;
		  }
	
		case state4:	/* get mapped objects number */
		  /* The cobId of the message received has been found in the dictionnary. */
		  offsetObjdict = d->firstIndex->PDO_RCV_MAP;
		  lastIndex = d->lastIndex->PDO_RCV_MAP;
		  pMappingCount = (d->objdict + offsetObjdict + numPdo)->pSubindex[0].pObject;	  
		  numMap = 0;
		  while (numMap < *pMappingCount) {
		    pMappingParameter = (d->objdict + offsetObjdict + numPdo)->pSubindex[numMap + 1].pObject;
		    if (pMappingParameter == NULL) {
		      MSG_ERR(0x1937, "Couldn't get mapping parameter : ", numMap + 1); 
		      return 0xFF;
		    }
		    /* Get the addresse of the mapped variable. */
		    /* detail of *pMappingParameter : */
	            /* The 16 hight bits contains the index, the medium 8 bits contains the subindex, */
		    /* and the lower 8 bits contains the size of the mapped variable. */

		    Size = ((UNS8)(((*pMappingParameter) & 0xFF) >> 3));

		    objDict = setODentry(d, (UNS16)((*pMappingParameter) >> 16),
				            (UNS8)(((*pMappingParameter) >> 8 ) & 0xFF),
					    (void *)&d->process_var.data[offset], &Size, 0 );

		    if(objDict != OD_SUCCESSFUL) {
		      MSG_ERR(0x1938, "error accessing to the mapped var : ", numMap + 1);  
		      MSG_WAR(0x2939, "         Mapped at index : ", (*pMappingParameter) >> 16);
		      MSG_WAR(0x2940, "                subindex : ", ((*pMappingParameter) >> 8 ) & 0xFF);
		      return 0xFF;
		    }

		    MSG_WAR(0x3942, "Variable updated with value received by PDO cobid : ", m->cob_id.w);  
		    MSG_WAR(0x3943, "         Mapped at index : ", (*pMappingParameter) >> 16);
		    MSG_WAR(0x3944, "                subindex : ", ((*pMappingParameter) >> 8 ) & 0xFF);
		    /* MSG_WAR(0x3945, "                data : ",*((UNS32 *)pMappedAppObject)); */
		    offset += Size;
		    numMap++;
		  } /* end loop while on mapped variables */
		  
		  offset=0x00;		
		  numMap = 0;
		  return 0;
		  
	      }/* end switch status	*/	 
	    }/* end while	*/
  }/* end if Donnees */


  else if ((*m).rtr == REQUEST ){  
      MSG_WAR(0x3946, "Receive a PDO request cobId : ", m->cob_id.w);
      status = state1;
      offsetObjdict = d->firstIndex->PDO_TRS;
      lastIndex = d->lastIndex->PDO_TRS;
      if(offsetObjdict) while( offsetObjdict  <= lastIndex ){ 
	/* study of all PDO stored in the objects dictionary */

	switch( status ){

	case state1:	/* check the CobId */
			/* get CobId of the dictionary which match to the received PDO */
	  pwCobId = (d->objdict + offsetObjdict)->pSubindex[1].pObject;	  
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


	case state4:	/* check transmission type (after request?) */
	  pTransmissionType = d->objdict[offsetObjdict].pSubindex[2].pObject;
	  if ( (*pTransmissionType == TRANS_RTR) || (*pTransmissionType == TRANS_RTR_SYNC ) || (*pTransmissionType == TRANS_EVENT) ) {
	    status = state5;
	    break;
	  }
	  else {
	    /* The requested PDO is not to send on request. So, does nothing. */
	    MSG_WAR(0x2947, "PDO is not to send on request : ", m->cob_id.w);
	    return 0xFF;
	  }

	case state5:	/* get mapped objects number */
	  offsetObjdict = d->firstIndex->PDO_TRS_MAP;
	  lastIndex = d->lastIndex->PDO_TRS_MAP;
	  pMappingCount = (d->objdict + offsetObjdict + numPdo)->pSubindex[0].pObject;
	  numMap = 0;
	  while (numMap < *pMappingCount) {
	    pMappingParameter = (d->objdict + offsetObjdict + numPdo)->pSubindex[numMap + 1].pObject;
	    /* Get the mapped variable */
	    Size = ((UNS8)(((*pMappingParameter) & 0xFF) >> 3));
	    objDict = getODentry( d, (UNS16)((*pMappingParameter) >> (UNS8)16), 
				     (UNS8)(( (*pMappingParameter) >> (UNS8)8 ) & 0xFF),
				     (void *)&d->process_var.data[offset], &Size, &dataType, 0 );
	    if (objDict != OD_SUCCESSFUL) {
	      MSG_ERR(0x1948, "error accessing to the mapped var : ", numMap + 1);  
	      MSG_WAR(0x2949, "         Mapped at index : ", (*pMappingParameter) >> 16);
	      MSG_WAR(0x2950, "                subindex : ", ((*pMappingParameter) >> 8 ) & 0xFF);
	      return 0xFF;
	    }
	    offset += (UNS8) (((*pMappingParameter) & 0xFF) >> 3);
	    d->process_var.count = offset;
	    numMap++;

	  } /* end while */
	  PDOmGR( d, *pwCobId ); /* Transmit the PDO */
	  return 0;

	}/* end switch status */
      }/* end while	 */			
    }/* end if Requete */
		
  return 0;
}




#if 0

/*********************************************************************/
/* TODO : reimplement this using CallBacks                           */
/*********************************************************************/

UNS8 sendPDOevent( CO_Data* d, void * variable )
{ /* DO NOT USE MSG_ERR because the macro may send a PDO -> infinite loop if it fails.	*/
  UNS32           objDict = 0;
  UNS8            ind, sub_ind;
  UNS8            status; 
  UNS8            offset;
  UNS8 *     pMappingCount = NULL;
  UNS32 *    pMappingParameter = NULL;
  void *     pMappedAppObject = NULL;
  UNS8 *     pTransmissionType = NULL; /* pointer to the transmission type */
  UNS32 *    pwCobId = NULL;
  UNS8 *     pSize;
  UNS8       size;
  UNS8       dataType;
  UNS16      offsetObjdict;
  UNS16      offsetObjdictPrm;
  UNS16      lastIndex;
  UNS8       numMap;
  ind     = 0x00;
  sub_ind = 1; 
  offset  = 0x00;
  pSize   = &size;
  status  = state1;

  /* look for the index and subindex where the variable is mapped */
  /* Then, send the pdo which contains the variable. */

  MSG_WAR (0x3960, "sendPDOevent", 0);
  offsetObjdictPrm = d->firstIndex->PDO_TRS;
  
  offsetObjdict = d->firstIndex->PDO_TRS_MAP;
  lastIndex = d->lastIndex->PDO_TRS_MAP;

  if (offsetObjdictPrm && offsetObjdict) 
	  /* Loop on PDO Transmit */
	  while(offsetObjdict <= lastIndex){
	    /* Check the transmission mode */
	    pTransmissionType = d->objdict[offsetObjdictPrm].pSubindex[2].pObject;
	    if (*pTransmissionType != TRANS_EVENT) {
	      ind++;
	      offsetObjdict++;  
	      offsetObjdictPrm++;
	      continue;
	    }
	    pMappingCount = d->objdict[offsetObjdict].pSubindex[0].pObject;
	    numMap = 1; /* mapped variable */
	    while (numMap <= *pMappingCount) {
	      pMappingParameter = d->objdict[offsetObjdict].pSubindex[numMap].pObject;
	      /* Get the variable */
	      objDict = getODentry( d,
	                            (UNS16)((*pMappingParameter) >> 16), 
				    (UNS8)(( (*pMappingParameter) >> (UNS8)8 ) & (UNS32)0x000000FF),
				    (void * *)&pMappedAppObject, pSize, &dataType, 0 );
	      if( objDict != OD_SUCCESSFUL ) {  
		MSG_WAR(0x2961, "Error in dict. at index : ", 
			(*pMappingParameter) >> (UNS8)16);
	      
		MSG_WAR(0x2962, "               subindex : ", 
			((*pMappingParameter) >> (UNS8)8 ) & (UNS32)0x000000FF);
		return 0xFF;
	      }
	      if (pMappedAppObject == variable) { // Variable found !
		MSG_WAR(0x3963, "Variable to send found at index : ", 
			(*pMappingParameter) >> 16);
		MSG_WAR(0x3964, "                       subIndex : ", 
			((*pMappingParameter) >> 8 ) & 0x000000FF);
		buildPDO(d, 0x1800 + ind);
		/* Get the cobId */
		pwCobId = d->objdict[offsetObjdictPrm].pSubindex[1].pObject;
		PDOmGR( d, *pwCobId ); /* Send the PDO */
		return 0;	    
	      }
	      numMap++;
	    } /* End loop on mapped variable */
	    ind++;	
	    offsetObjdict++;  
	    offsetObjdictPrm++;
	  } /* End loop while on PDO */

  MSG_WAR(0x2965, "Variable not found in a PDO to send on event", 0);
  return 0xFF;

}
#endif
