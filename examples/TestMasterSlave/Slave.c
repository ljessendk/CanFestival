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

#include "Slave.h"
#include "Master.h"
#include "TestMasterSlave.h"

/*****************************************************************************/
void TestSlave_heartbeatError(UNS8 heartbeatID)
{
	eprintf("TestSlave_heartbeatError %d\n", heartbeatID);
}

void TestSlave_initialisation()
{
	eprintf("TestSlave_initialisation\n");
}

void TestSlave_preOperational()
{
	eprintf("TestSlave_preOperational\n");
}

void TestSlave_operational()
{
	eprintf("TestSlave_operational\n");
}

void TestSlave_stopped()
{
	eprintf("TestSlave_stopped\n");
}

void TestSlave_post_sync()
{
      eprintf("TestSlave_post_sync\n");
      SlaveMap1+=1;
      SlaveMap2+=2;
      SlaveMap3+=3;
      SlaveMap4+=4;
      eprintf("Slave: %d %d %d %d\n",SlaveMap1, SlaveMap2, SlaveMap3, SlaveMap4);
}

void TestSlave_post_TPDO()
{
	eprintf("TestSlave_post_TPDO\n");
}

void TestSlave_storeODSubIndex(UNS16 wIndex, UNS8 bSubindex)
{
	/*TODO : 
	 * - call getODEntry for index and subindex, 
	 * - save content to file, database, flash, nvram, ...
	 * 
	 * To ease flash organisation, index of variable to store
	 * can be established by scanning d->objdict[d->ObjdictSize]
	 * for variables to store.
	 * 
	 * */
	eprintf("TestSlave_storeODSubIndex : %4.4x %2.2x\n", wIndex,  bSubindex);
}
