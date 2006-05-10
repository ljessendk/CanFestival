/*
This file is part of CanFestival, a library implementing CanOpen Stack.

 Author: CANopen Canada (canfestival@canopencanada.ca)

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
/*
	nvram.c

	save the content of the dictionnary into non-volatile memory
	the order of storage must be the same as the order of retrieving

	note (1)
	may need to store/retrieve specific data from non-volatile
	in that case, we need to address where it is stored
	without storing the address in a vector
	- solution 1 : walk throught the list without action until the
	sought object is found.
	 
*/

#include <stdio.h>
#include <data.h>
#include <applicfg.h>
#include <objdictdef.h>
#include "can_driver.h"


int canSaveData(indextable *dict, int max_index)
{
	int i, j;

	if (nvram_open() < 0)
		return -1;

	subindex  *pSubindex;

	for(i=0; i<max_index; i++)
	{
		pSubindex = dict[i].pSubindex;

		for(j=0; j<dict[i].bSubCount; j++)
		{
			/* check pSubindex[j].bAccessType */

			nvram_write(pSubindex[j].bDataType,
				pSubindex[j].bAccessType,
				pSubindex[j].pObject);
		}
	}

	nvram_close();

	return 0;
}


int canReadData(indextable *dict, int max_index)
{
	int i, j;

	subindex *pSubindex;

	union
	{
		UNS8   u8;
		UNS16  u16;
		UNS32  u32;

		float  r4;
		double r8;
	} object;

	if (nvram_open() < 0)
		return -1;

	for(i=0; i<max_index; i++)
	{
		pSubindex = dict[i].pSubindex;

		for(j=0; j<dict[i].bSubCount; j++)
		{

			nvram_read(pSubindex[j].bDataType,
				pSubindex[j].bAccessType,
				(void *)&object);
		}
	}

	nvram_close();

	return 0;
}

