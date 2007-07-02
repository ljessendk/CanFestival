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
      
      SlaveMap1=1;
      SlaveMap2=1;
      SlaveMap3=0;
      SlaveMap4=0;
      SlaveMap5=1;
      SlaveMap6=0;
      SlaveMap7=1;
      SlaveMap8=0;
      SlaveMap9=16;
      SlaveMap10 = 0xff00ff00;
      SlaveMap11 = 0xabcd;
      SlaveMap12 += SlaveMap12 > 0x80 ? 0 : 1;
      eprintf("Slave: %d %d %d %d %d %d %d %d %d %x %x %d %d \n",
      	SlaveMap1, 
      	SlaveMap2, 
      	SlaveMap3, 
      	SlaveMap4, 
      	SlaveMap5, 
      	SlaveMap6, 
      	SlaveMap7, 
      	SlaveMap8, 
      	SlaveMap9, 
      	SlaveMap10,
      	SlaveMap11, 
      	SlaveMap12,
      	SlaveMap13);
}

void TestSlave_post_TPDO()
{
        SlaveMap13 += 1;
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
