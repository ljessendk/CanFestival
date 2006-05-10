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

#include "data.h"

// prototypes of function to be filled by app
void gene_SYNC_SDOtimeoutError(UNS8 line);
void gene_SYNC_heartbeatError(UNS8);

UNS8 gene_SYNC_canSend(Message *);

void gene_SYNC_initialisation();
void gene_SYNC_preOperational();
void gene_SYNC_operational();
void gene_SYNC_stopped();

void gene_SYNC_post_sync();
void gene_SYNC_post_TPDO();

// Master node data struct
extern CO_Data gene_SYNC_Data;

