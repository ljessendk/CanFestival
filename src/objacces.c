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

//#define DEBUG_WAR_CONSOLE_ON
//#define DEBUG_ERR_CONSOLE_ON

#include "objacces.h"

#ifdef DEBUG_WAR_CONSOLE_ON
UNS8 accessDictionaryError(UNS16 index, UNS8 subIndex, 
			     UNS8 sizeDataDict, UNS8 sizeDataGiven, UNS32 code)
{
  MSG_WAR(0x2B09,"Dictionary index : ", index);
  MSG_WAR(0X2B10,"           subindex : ", subIndex);
  switch (code) {
    case  OD_NO_SUCH_OBJECT: 
      MSG_WAR(0x2B11,"Index not found ", index);
      break;
    case OD_NO_SUCH_SUBINDEX :
      MSG_WAR(0x2B12,"SubIndex not found ", subIndex);
      break;   
    case OD_WRITE_NOT_ALLOWED :
      MSG_WAR(0x2B13,"Write not allowed, data is read only ", index);
      break;         
    case OD_LENGTH_DATA_INVALID :    
      MSG_WAR(0x2B14,"Conflict size data. Should be (bytes)  : ", sizeDataDict);
      MSG_WAR(0x2B15,"But you have given the size  : ", sizeDataGiven);
      break;
    case OD_NOT_MAPPABLE :
      MSG_WAR(0x2B16,"Not mappable data in a PDO at index    : ", index);
      break;
    case OD_VALUE_TOO_LOW :
      MSG_WAR(0x2B17,"Value range error : value too low. SDOabort : ", code);
      break;
    case OD_VALUE_TOO_HIGH :
      MSG_WAR(0x2B18,"Value range error : value too high. SDOabort : ", code);
      break;
  default :
    MSG_WAR(0x2B20, "Unknown error code : ", code);
  }
  return 0; 
}	
#endif

UNS32 getODentry( CO_Data* d, 
                  UNS16 wIndex,
		  UNS8 bSubindex,
		  void * pDestData,
		  UNS8 * pExpectedSize,
		  UNS8 * pDataType,
		  UNS8 checkAccess)
{ // DO NOT USE MSG_ERR because the macro may send a PDO -> infinite loop if it fails.
  UNS32 errorCode;
  UNS8 szData;
  const indextable *ptrTable;
  ODCallback_t *Callback;

  ptrTable = (*d->scanIndexOD)(wIndex, &errorCode, &Callback);

  if (errorCode != OD_SUCCESSFUL)
    return errorCode;
  if( ptrTable->bSubCount <= bSubindex ) {
    // Subindex not found
    accessDictionaryError(wIndex, bSubindex, 0, 0, OD_NO_SUCH_SUBINDEX);
    return OD_NO_SUCH_SUBINDEX;
  }
  
  if (checkAccess && !(ptrTable->pSubindex[bSubindex].bAccessType & WO)) {
  	MSG_WAR(0x2B30, "Access Type : ", ptrTable->pSubindex[bSubindex].bAccessType);
    accessDictionaryError(wIndex, bSubindex, 0, 0, OD_WRITE_NOT_ALLOWED);
    return OD_READ_NOT_ALLOWED;
  }

  *pDataType = ptrTable->pSubindex[bSubindex].bDataType;
   szData = ptrTable->pSubindex[bSubindex].size;

  if(	*pExpectedSize == 0 ||
  	*pExpectedSize == szData ||
  	(*pDataType == visible_string && *pExpectedSize > szData)) // We allow to fetch a shorter string than expected
  {
	#ifdef CANOPEN_BIG_ENDIAN
	      if(*pDataType > boolean && dataType < visible_string){
	      {
		// data must be transmited with low byte first
		UNS8 i, j = 0;
		for ( i = ptrTable->pSubindex[bSubindex].size ; i > 0 ; i--) {
			((char*)pDestData)[j++] = ((char*)ptrTable->pSubindex[bSubindex].pObject)[i-1];
		}
	      }
	#else  	
  	      memcpy(pDestData, ptrTable->pSubindex[bSubindex].pObject,*pExpectedSize);
	#endif
      *pExpectedSize = szData;
      return OD_SUCCESSFUL;
  }else{
      *pExpectedSize = szData;
      accessDictionaryError(wIndex, bSubindex, szData, *pExpectedSize, OD_LENGTH_DATA_INVALID);
      return OD_LENGTH_DATA_INVALID;
  }
}

UNS32 setODentry( CO_Data* d, 
                  UNS16 wIndex,
		  UNS8 bSubindex, 
		  void * pSourceData, 
		  UNS8 * pExpectedSize, 
		  UNS8 checkAccess)
{
  UNS8 szData;
  UNS8 dataType;
  UNS32 errorCode;
  const indextable *ptrTable;
  ODCallback_t *Callback;

  ptrTable =(*d->scanIndexOD)(wIndex, &errorCode, &Callback);
  if (errorCode != OD_SUCCESSFUL)
    return errorCode;

  if( ptrTable->bSubCount <= bSubindex ) {
    // Subindex not found
    accessDictionaryError(wIndex, bSubindex, 0, *pExpectedSize, OD_NO_SUCH_SUBINDEX);
    return OD_NO_SUCH_SUBINDEX;
  }
  if (checkAccess && (ptrTable->pSubindex[bSubindex].bAccessType == RO)) {
  	MSG_WAR(0x2B25, "Access Type : ", ptrTable->pSubindex[bSubindex].bAccessType);
    accessDictionaryError(wIndex, bSubindex, 0, *pExpectedSize, OD_WRITE_NOT_ALLOWED);
    return OD_WRITE_NOT_ALLOWED;
  }


   dataType = ptrTable->pSubindex[bSubindex].bDataType;
   szData = ptrTable->pSubindex[bSubindex].size;

  if( *pExpectedSize == 0 ||
  	*pExpectedSize == szData ||
  	(dataType == visible_string && *pExpectedSize < szData)) // We allow to store a shorter string than entry size
  {
      #ifdef CANOPEN_BIG_ENDIAN
	      if(dataType > boolean && dataType < visible_string){
		// we invert the data source directly. This let us do range testing without
		// additional temp variable
		UNS8 i, j = 0;
		for ( i = ptrTable->pSubindex[bSubindex].size >> 1 ; i > 0 ; i--) {
			char tmp = ((char*)pSourceData)[i - 1];
			((char*)pSourceData)[i - 1] = ((char*)pSourceData)[j];
			((char*)pSourceData)[j++] = tmp;
		}
	      }
      #endif
      errorCode = (*d->valueRangeTest)(dataType, pSourceData);
      if (errorCode) {
	accessDictionaryError(wIndex, bSubindex, szData, *pExpectedSize, errorCode);
	return errorCode;
      }
      memcpy(ptrTable->pSubindex[bSubindex].pObject,pSourceData, *pExpectedSize);
      *pExpectedSize = szData;
      
      // Callbacks
      if(Callback && Callback[bSubindex]){
      	 (*Callback[bSubindex])(d, ptrTable, bSubindex);
      }
      
      // TODO : Store dans NVRAM      
      // if (ptrTable->pSubindex[bSubindex].bAccessType & TO_BE_SAVED)
      return OD_SUCCESSFUL;
  }else{
      *pExpectedSize = szData;
      accessDictionaryError(wIndex, bSubindex, szData, *pExpectedSize, OD_LENGTH_DATA_INVALID);
      return OD_LENGTH_DATA_INVALID;
  }
}


const indextable * scanIndexOD (CO_Data* d, UNS16 wIndex, UNS32 *errorCode, ODCallback_t **Callback)
{
  return (*d->scanIndexOD)(wIndex, errorCode, Callback);
}

UNS32 RegisterSetODentryCallBack(CO_Data* d, UNS16 wIndex, UNS8 bSubindex, ODCallback_t Callback)
{
	UNS32 errorCode;
	ODCallback_t *CallbackList;

	scanIndexOD (d, wIndex, &errorCode, &CallbackList);
	if(errorCode == OD_SUCCESSFUL && CallbackList) 
		CallbackList[bSubindex] = Callback;
	return errorCode;
}


