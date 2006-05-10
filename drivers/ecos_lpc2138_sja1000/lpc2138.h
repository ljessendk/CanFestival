/*
This file is part of CanFestival, a library implementing CanOpen Stack.

 Author: Christian Fortin (canfestival@canopencanada.ca)

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


#if !defined(_LPC2138_H_)
#define _LPC2138_H_


extern short data_len;
extern unsigned int *data_page;
extern unsigned int data_addr;


void lpc2138_pinsel_set(int pin, LPC2138_PORT port, int size, int func); 
void lpc2138_pinsel_clear(void); 

int lpc2138_printf(void); 
int lpc2138_printf_pins(void); 

int lpc2138_fprintf(FILE *stream);
int lpc2138_fprintf_pins(FILE *stream);

unsigned char sja1000_read(unsigned char addr8);
void sja1000_write(unsigned char addr8, unsigned char data);

void iat_flash_erase(unsigned int command_ee,unsigned int result_ee[]);
void iat_flash_write_page(unsigned int addr);
void iat_flash_read_page(unsigned int addr);


#endif

