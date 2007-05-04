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

#include "objacces.h"
#include "sdo.h"
#include "dcf.h"

const indextable *ptrTable;

static void CheckSDOAndContinue(CO_Data* d, UNS8 nodeId)
{
	UNS32 res;
	UNS8 line;
	UNS8 err;
	UNS16 Index;
	UNS8 SubIndex;
	UNS32 abortCode;
	
	if(getWriteResultNetworkDict (d, nodeId, &abortCode) != SDO_FINISHED)
		printf("Master : Failed in initializing slave %2.2x, AbortCode :%4.4x \n", nodeId, abortCode);
	
	closeSDOtransfer(d, nodeId, SDO_CLIENT);
	res = decompo_dcf(d,nodeId);
}

UNS32 decompo_dcf(CO_Data* d,UNS8 nodeId)
{
		UNS32 errorCode;
		UNS16 target_Index;
		UNS8 target_Subindex;
		UNS32 target_Size;
		void* target_data = NULL;
		UNS32 res;
  		ODCallback_t *Callback;

		ptrTable = (*d->scanIndexOD)(0x1F22, &errorCode, &Callback);
		if (errorCode != OD_SUCCESSFUL)
		{
    		return errorCode;
		}

		/*Loop on all Nodes supported in DCF subindexes*/
		while (nodeId < ptrTable->bSubCount){
			UNS32 nb_targets;
	  		
	  		UNS8 szData = ptrTable->pSubindex[nodeId].size;
	  		void* dcfend;
	  		
		  	{
			  	void* dcf = *((void**)ptrTable->pSubindex[nodeId].pObject);
		  		dcfend = dcf + szData;
				if (!d->dcf_cursor)	{
					d->dcf_cursor = dcf + 4;
					d->dcf_count_targets = 0;
				}
	#ifdef CANOPEN_BIG_ENDIAN
				nb_targets = ((UNS8*)d->dcf++) | ((UNS8*)d->dcf++) << 8 | ((UNS8*)d->dcf++) << 16 | ((UNS8*)d->dcf++) << 24;
	#else
				nb_targets = *((UNS32*)dcf);
	#endif
		  	}
			
			// condition on consise DCF string for NodeID, if big enough
			if(d->dcf_cursor + 7 < dcfend && d->dcf_count_targets < nb_targets)
			{
				// pointer to the DCF string for NodeID
	#ifdef CANOPEN_BIG_ENDIAN
				target_Index = ((UNS8*)d->dcf_cursor++) | ((UNS8*)d->dcf_cursor++) << 8;
				target_Subindex = ((UNS8*)d->dcf_cursor++);
				target_Size = ((UNS8*)d->dcf_cursor++) | ((UNS8*)d->dcf_cursor++) << 8 | ((UNS8*)d->dcf_cursor++) << 16 | ((UNS8*)d->dcf_cursor++) << 24;
	#else
				target_Index = *((UNS16*)(d->dcf_cursor)); d->dcf_cursor += 2;
				target_Subindex = *((UNS8*)(d->dcf_cursor++));
				target_Size = *((UNS32*)(d->dcf_cursor)); d->dcf_cursor += 4;
	#endif
				
					printf("Master : ConfigureSlaveNode %2.2x (Concise DCF)\n",nodeId);
					res = writeNetworkDictCallBack(d, /*CO_Data* d*/
							nodeId, /*UNS8 nodeId*/
							target_Index, /*UNS16 index*/
							target_Subindex, /*UNS8 subindex*/
							target_Size, /*UNS8 count*/
							0, /*UNS8 dataType*/
							d->dcf_cursor,/*void *data*/
							CheckSDOAndContinue); /*SDOCallback_t Callback*/					
					/*Push d->dcf_cursor to the end of data*/
					
					d->dcf_cursor += target_Size;
					d->dcf_count_targets++;
					
					return ;
			}			
				nodeId++;
				d->dcf_cursor = NULL;
		}
		/* Switch Master to preOperational state */
		(*d->preOperational)();
		
}
