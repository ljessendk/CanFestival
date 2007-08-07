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


/**
** @file   dcf.c
** @author Edouard TISSERANT and Francis DUPIN
** @date   Mon Jun  4 17:06:12 2007
**
** @brief EXEMPLE OF SOMMARY
**
**
*/


#include "objacces.h"
#include "sdo.h"
#include "dcf.h"
#include "sysdep.h"

extern UNS8 _writeNetworkDict (CO_Data* d, UNS8 nodeId, UNS16 index,
                               UNS8 subIndex, UNS8 count, UNS8 dataType, void *data, SDOCallback_t Callback, UNS8 endianize);

const indextable *ptrTable;

/**
**
**
** @param d
** @param nodeId
*/
static void CheckSDOAndContinue(CO_Data* d, UNS8 nodeId)
{
  UNS32 abortCode;

  if(getWriteResultNetworkDict (d, nodeId, &abortCode) != SDO_FINISHED)
    {
      MSG_ERR(0x1A01, "SDO error in consise DCF", abortCode);
      MSG_WAR(0x2A02, "server node : ", nodeId);
    }

  closeSDOtransfer(d, nodeId, SDO_CLIENT);
  decompo_dcf(d,nodeId);
}

/**
**
**
** @param d
** @param nodeId
**
** @return
*/
UNS32 decompo_dcf(CO_Data* d,UNS8 nodeId)
{
  UNS32 errorCode;
  UNS16 target_Index;
  UNS8 target_Subindex;
  UNS32 target_Size;
  UNS32 res;
  ODCallback_t *Callback;

  ptrTable = (*d->scanIndexOD)(0x1F22, &errorCode, &Callback);
  if (errorCode != OD_SUCCESSFUL)
    {
      return errorCode;
    }

  /* Loop on all Nodes supported in DCF subindexes*/
  while (nodeId < ptrTable->bSubCount){
    UNS32 nb_targets;

    UNS8 szData = ptrTable->pSubindex[nodeId].size;
    UNS8* dcfend;

    {
      UNS8* dcf = *((UNS8**)ptrTable->pSubindex[nodeId].pObject);
      dcfend = dcf + szData;
      if (!d->dcf_cursor){
        d->dcf_cursor = (UNS8*)dcf + 4;
        d->dcf_count_targets = 0;
      }
      nb_targets = UNS32_LE(*((UNS32*)dcf));
    }

    /* condition on consise DCF string for NodeID, if big enough */
    if((UNS8*)d->dcf_cursor + 7 < (UNS8*)dcfend && d->dcf_count_targets < nb_targets)
      {
        /* pointer to the DCF string for NodeID */
        target_Index = UNS16_LE(*((UNS16*)(d->dcf_cursor))); d->dcf_cursor += 2;
        target_Subindex = *((UNS8*)((UNS8*)d->dcf_cursor++));
        target_Size = UNS32_LE(*((UNS32*)(d->dcf_cursor))); d->dcf_cursor += 4;

        /* printf("Master : ConfigureSlaveNode %2.2x (Concise
          DCF)\n",nodeId);*/
        res = _writeNetworkDict(d, /* CO_Data* d*/
                                nodeId, /* UNS8 nodeId*/
                                target_Index, /* UNS16 index*/
                                target_Subindex, /* UNS8 subindex*/
                                target_Size, /* UNS8 count*/
                                0, /* UNS8 dataType*/
                                d->dcf_cursor,/* void *data*/
                                CheckSDOAndContinue,/* SDOCallback_t
                                                      Callback*/
                                0); /* no endianize*/
        /* Push d->dcf_cursor to the end of data*/

        d->dcf_cursor += target_Size;
        d->dcf_count_targets++;

        return ;
      }
    nodeId++;
    d->dcf_cursor = NULL;
  }
  /*  Switch Master to preOperational state */
  (*d->preOperational)();

}
