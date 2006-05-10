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


#ifndef __data_h__
#define __data_h__

// declaration of CO_Data type let us include all necessary headers
// struct struct_CO_Data can then be defined later
typedef struct struct_CO_Data CO_Data;

#include <applicfg.h>
#include "def.h"
#include "can.h"
#include "objdictdef.h"
#include "objacces.h"
#include "sdo.h"
#include "pdo.h"
#include "states.h"
#include "lifegrd.h"
#include "sync.h"
#include "nmtMaster.h"

// This structurs contains all necessary information for a CanOpen node
struct struct_CO_Data {
	// Object dictionary
	UNS8 *bDeviceNodeId;
	const indextable *objdict;
	UNS8 *count_sync;
	quick_index *firstIndex;
	quick_index *lastIndex;
	UNS16 *ObjdictSize;
	const UNS8 *iam_a_slave;
	valueRangeTest_t valueRangeTest;
	
	// SDO
	s_transfer transfers[SDO_MAX_SIMULTANEOUS_TRANSFERTS];
	SDOtimeoutError_t SDOtimeoutError;
	//s_sdo_parameter *sdo_parameters;

	// State machine
	e_nodeState nodeState;
	s_state_communication CurrentCommunicationState;
	initialisation_t initialisation;
	preOperational_t preOperational;
	operational_t operational;
	stopped_t stopped;

	// NMT-heartbeat
	UNS8 *ConsumerHeartbeatCount;
	UNS32 *ConsumerHeartbeatEntries;
	TIMER_HANDLE *ConsumerHeartBeatTimers;
	UNS16 *ProducerHeartBeatTime;
	TIMER_HANDLE ProducerHeartBeatTimer;
	heartbeatError_t heartbeatError;
	e_nodeState NMTable[NMT_MAX_NODE_ID]; 

	// SYNC
	TIMER_HANDLE syncTimer;
	UNS32 *COB_ID_Sync;
	UNS32 *Sync_Cycle_Period;
	/*UNS32 *Sync_window_length*/;
	post_sync_t post_sync;
	post_TPDO_t post_TPDO;
	
	// PDO
	s_process_var process_var;
	
	// General
	UNS8 toggle;
	canSend_t canSend;	
	scanIndexOD_t scanIndexOD;
};

// A macro to initialize the data in client app.
#define CANOPEN_NODE_DATA_INITIALIZER(NODE_PREFIX) {\
	/* Object dictionary*/\
	bDeviceNodeId:& NODE_PREFIX ## _bDeviceNodeId,\
	objdict: NODE_PREFIX ## _objdict,\
	count_sync: NODE_PREFIX ## _count_sync,\
	firstIndex: & NODE_PREFIX ## _firstIndex,\
	lastIndex: & NODE_PREFIX ## _lastIndex,\
	ObjdictSize: & NODE_PREFIX ## _ObjdictSize,\
	iam_a_slave: & NODE_PREFIX ## _iam_a_slave,\
	valueRangeTest: NODE_PREFIX ## _valueRangeTest,\
	\
	/* SDO */\
	transfers:{{\
		nodeId: 0,\
		index: 0,\
		subIndex: 0,\
		state: SDO_RESET,\
		toggle: 0,\
		count: 0,\
		offset: 0,\
		data: {0,},\
		dataType: 0,\
		timer: -1},},\
	SDOtimeoutError: &NODE_PREFIX ## _SDOtimeoutError,\
	\
	/* State machine */\
	nodeState:Unknown_state,\
	CurrentCommunicationState:{\
		csBoot_Up: 0,\
		csSDO: 0,\
		csEmergency: 0,\
		csSYNC: 0,\
		csHeartbeat: 0,\
		csPDO: 0},\
	initialisation: &NODE_PREFIX ## _initialisation,\
	preOperational: &NODE_PREFIX ## _preOperational,\
	operational: &NODE_PREFIX ## _operational,\
	stopped: &NODE_PREFIX ## _stopped,\
	\
	/* NMT-heartbeat */\
	ConsumerHeartbeatCount: & NODE_PREFIX ## _highestSubIndex_obj1016,\
	ConsumerHeartbeatEntries: NODE_PREFIX ## _obj1016,\
	ConsumerHeartBeatTimers: NODE_PREFIX ## _heartBeatTimers,\
	ProducerHeartBeatTime: & NODE_PREFIX ## _obj1017,\
	ProducerHeartBeatTimer: TIMER_NONE,\
	heartbeatError: NODE_PREFIX ## _heartbeatError,\
	NMTable:{Unknown_state,},\
	\
	/* SYNC */\
	syncTimer: TIMER_NONE,\
	COB_ID_Sync: & NODE_PREFIX ## _obj1005,\
	Sync_Cycle_Period: & NODE_PREFIX ## _obj1006,\
	/*Sync_window_length: & NODE_PREFIX ## _obj1007,*/\
	post_sync: NODE_PREFIX ## _post_sync,\
	post_TPDO: NODE_PREFIX ## _post_TPDO,\
	\
	/* PDO */\
	process_var: {\
		count: 0,\
		data: {0,}},\
	\
	/* General */\
	toggle: 0,\
	canSend: NODE_PREFIX ## _canSend,\
	scanIndexOD: NODE_PREFIX ## _scanIndexOD\
}

#endif // __data_h__


