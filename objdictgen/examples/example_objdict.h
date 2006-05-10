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
extern UNS8 seconds;		// Mapped at index 0x2000, subindex 0x01
extern UNS8 minutes;		// Mapped at index 0x2000, subindex 0x02
extern UNS8 hours;		// Mapped at index 0x2000, subindex 0x03
extern UNS8 day;		// Mapped at index 0x2000, subindex 0x04
extern UNS32 canopenErrNB;		// Mapped at index 0x6000, subindex 0x00
extern UNS32 canopenErrVAL;		// Mapped at index 0x6001, subindex 0x00
extern UNS8 strTest[10];		// Mapped at index 0x6002, subindex 0x00
