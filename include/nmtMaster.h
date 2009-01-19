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

/** @defgroup networkmanagement Network Management
 *  @ingroup userapi
 */
/** @defgroup nmtmaster NMT Master
 *  @brief NMT master provides mechanisms that control and monitor the state of nodes and their behavior in the network.
 *  @ingroup networkmanagement
 */
 
#ifndef __nmtMaster_h__
#define __nmtMaster_h__

#include "data.h"

/** 
 * @ingroup nmtmaster
 * @brief Transmit a NMT message on the bus number bus_id
 * to the slave whose node_id is ID
 * 
 * bus_id is hardware dependant
 * cs represents the order of state changement:
 *  - cs =  NMT_Start_Node            // Put the node in operational mode             
 *  - cs =	 NMT_Stop_Node		   // Put the node in stopped mode
 *  - cs =	 NMT_Enter_PreOperational  // Put the node in pre_operational mode  
 *  - cs =  NMT_Reset_Node		   // Put the node in initialization mode 
 *  - cs =  NMT_Reset_Comunication	   // Put the node in initialization mode 
 * The mode is changed according to the slave state machine mode :
 *  - initialisation  ---> pre-operational (Automatic transition)
 *  - pre-operational <--> operational
 *  - pre-operational <--> stopped
 *  - pre-operational, operational, stopped -> initialisation\n
 * @param *d Pointer on a CAN object data structure
 * @param Node_ID Id of the slave node
 * @param cs State changement
 * @return canSend(bus_id,&m)               
 */
UNS8 masterSendNMTstateChange (CO_Data* d, UNS8 Node_ID, UNS8 cs);

/**
 * @ingroup nmtmaster 
 * @brief Transmit a Node_Guard message on the bus number bus_id
 * to the slave whose node_id is nodeId
 * 
 * bus_id is hardware dependant
 * @param *d Pointer on a CAN object data structure
 * @param nodeId Id of the slave node
 * @return canSend(bus_id,&m)
 */
UNS8 masterSendNMTnodeguard (CO_Data* d, UNS8 nodeId);

/** 
 * @ingroup nmtmaster
 * @brief Prepare a Node_Guard message transmission on the bus number bus_id
 * to the slave whose node_id is nodeId
 * 
 * Put nodeId = 0 to send an NMT broadcast.
 * This message will ask for the slave, whose node_id is nodeId, its state
 * bus_id is hardware dependant
 * @param *d Pointer on a CAN object data structure
 * @param nodeId Id of the slave node
 */
void masterRequestNodeState (CO_Data* d, UNS8 nodeId);


#endif /* __nmtMaster_h__ */
