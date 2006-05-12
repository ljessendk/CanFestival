/*
This file is part of CanFestival, a library implementing CanOpen Stack.
  ____    _    _   _
 / ___|  / \  | \ | | ___  _ __   ___ _ __
| |     / _ \ |  \| |/ _ \| '_ \ / _ \ '_ \
| |___ / ___ \| |\  | (_) | |_) |  __/ | | |
 \____/_/   \_\_| \_|\___/| .__/ \___|_| |_|
                          |_|
          ____                      _
         / ___|__ _ _ __   __ _  __| | __ _
        | |   / _` | '_ \ / _` |/ _` |/ _` |
        | |__| (_| | | | | (_| | (_| | (_| |
         \____\__,_|_| |_|\__,_|\__,_|\__,_|

                   canfestival@canopencanada.ca
/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

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

#if !defined(_NVRAM_IO_H_)
#define _NVRAM_IO_H_


int    nvram_open(void);
void   nvram_close(void);

char nvram_write_data(int type, int access_attr, void *data);
char nvram_read_data(int type, int access_attr, void *data);

void nvram_write_regs(void);
void nvram_read_regs(void);

#endif

