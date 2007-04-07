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

#ifndef __SYNC_h__
#define __SYNC_h__

void startSYNC(CO_Data* d);

void stopSYNC(CO_Data* d);

typedef void (*post_sync_t)(void);
void _post_sync(void);

typedef void (*post_TPDO_t)(void);
void _post_TPDO(void);

/** transmit a SYNC message on the bus number bus_id
 * bus_id is hardware dependant
 * return canSend(bus_id,&m)
 */
UNS8 sendSYNC (CO_Data* d, UNS32 cob_id);

/** This function is called when the node is receiving a SYNC 
 * message (cob-id = 0x80).
 * What does the function :
 * check if the node is in OERATIONAL mode. (other mode : return 0 but does nothing).
 * Get the SYNC cobId by reading the dictionary index 1005. (Return -1 if it does not correspond 
 * to the cobId received).
 * Scan the dictionary from index 0x1800 to the last PDO defined (dict_cstes.max_count_of_PDO_transmit)
 *   for each PDO whose transmission type is on synchro (transmission type < 241) and if the msg must
 *   be send at this SYNC. read the COBID. Verify that the nodeId inside the 
 *   nodeId correspond to bDeviceNodeId. (Assume that the cobId of a PDO Transmit is made 
 *   with the node id of the node who transmit), get the mapping, launch PDOmGR to send the PDO
 * *m is a pointer to the message received
 * bus_id is hardware dependant
 * return 0 if OK, 0xFF if error
 */
UNS8 proceedSYNC (CO_Data* d, Message * m);

#endif
