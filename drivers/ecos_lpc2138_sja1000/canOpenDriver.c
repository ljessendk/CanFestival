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

#include <stdlib.h>

#include <sys/time.h>
#include <signal.h>

#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_arch.h>

#include "applicfg.h"
#include <data.h>
#include <def.h>
#include <can.h>
#include <can_driver.h>
#include <objdictdef.h>
#include <objacces.h>

#include "lpc2138_pinout.h"
#include "lpc2138_defs.h"
#include "lpc2138.h"

#include "sja1000.h"

#include "time_slicer.h"


/*
	SEND/RECEIVE
*/
CAN_HANDLE canOpen(s_BOARD *board)
{
	return NULL;
}

/***************************************************************************/
int canClose(CAN_HANDLE fd0)
{
	return 0;
}

UNS8 canReceive(CAN_HANDLE fd0, Message *m)
/*
Message *m :
	typedef struct {
	  SHORT_CAN cob_id;     // l'ID du mesg
	  UNS8 rtr;             // remote transmission request. 0 if not rtr,
        	                // 1 for a rtr message
	  UNS8 len;             // message length (0 to 8)
	  UNS8 data[8];         // data
	} Message;

Fill the structure "Message" with data from the CAN receive buffer

return : 0
*/
{
/*
	the sja1000 must be set to the PeliCAN mode
*/
    m->cob_id.w = sja1000_read(16) + (sja1000_read(17)<<8); // IO_PORTS_16(CAN0 + CANRCVID) >> 5

    m->rtr = (sja1000_read(17) >> 4) & 0x01; // (IO_PORTS_8(CAN0 + CANRCVID + 1) >> 4) & 0x01; 

    m->len = sja1000_read(18);

    m->data[0] = sja1000_read(19);
    m->data[1] = sja1000_read(20);
    m->data[2] = sja1000_read(21);
    m->data[3] = sja1000_read(22);
    m->data[4] = sja1000_read(23);
    m->data[5] = sja1000_read(24);
    m->data[6] = sja1000_read(25);
    m->data[7] = sja1000_read(26);

    sja1000_write(CMR, 1<<RRB );        // release fifo

    return 0;
}


UNS8 canSend(CAN_HANDLE fd0, Message *m)
/*
Message *m :
	typedef struct {
	  SHORT_CAN cob_id;     // l'ID du mesg
	  UNS8 rtr;                     // remote transmission request. 0 if not rtr,
        	                        // 1 for a rtr message
	  UNS8 len;                     // message length (0 to 8)
	  UNS8 data[8];         // data
	} Message;

Send the content of the structure "Message" to the CAN transmit buffer

return : 0 if OK, 1 if error
*/
{
    unsigned char rec_buf;

    do
    {
        rec_buf = sja1000_read(SR);
    }
    while ( (rec_buf & (1<<TBS))==0);           // loop until TBS high

    sja1000_write(16, m->cob_id.w & 0xff); 
    sja1000_write(17, (m->cob_id.w >> 8) & 0xff);
    sja1000_write(18, m->len);

    sja1000_write(19, m->data[0]); // tx data 1
    sja1000_write(20, m->data[1]); // tx data 2
    sja1000_write(21, m->data[2]); // tx data 3
    sja1000_write(22, m->data[3]); // tx data 4
    sja1000_write(23, m->data[4]); // tx data 5
    sja1000_write(24, m->data[5]); // tx data 6
    sja1000_write(25, m->data[6]); // tx data 7
    sja1000_write(26, m->data[7]); // tx data 8

    sja1000_write(CMR,( (0<<SRR) | (0<<CDO) | (0<<RRB) | (0<<AT) | (1<<TR)));
    do
    {
        rec_buf = sja1000_read(SR);
    }
    while ( (rec_buf & (1<<TBS))==0);           // loop until TBS high

    return 0;
}


/*
	SEQUENTIAL I/O TO FLASH
	those functions are for continous writing and read
*/


int nvram_open(void)
{
	int n = NVRAM_BLOCK_SIZE / sizeof(unsigned int);

	/* some actions to initialise the flash */
	data_len = 0;
	data_num_pages = 0;

	data_page = (unsigned int *)malloc(sizeof(unsigned int) * n);
	memset(data_page, 0, sizeof(unsigned int)*n);

	if (data_page == NULL)
		return -1;

	regs_page = (unsigned int *)malloc(sizeof(unsigned int) * n);
	memset(regs_page, 0, sizeof(unsigned int)*n);
	if (regs_page == NULL)
		return -2;

	iat_flash_read_regs();

	/* start the data at the location specified in the registers */ 
	if (0) /* for now it is 0, but put here a test to know whether
                  or not the NVRAM has been written before */
		data_addr = regs_page[1];
	else
		data_addr = NVRAM_BLOCK_SIZE; /* let start at block 1 */

	return 0;
}


void nvram_close(void)
{
	/* write the last page before closing */
	iat_flash_write_page(data_addr);

	/* some actions to end accessing the flash */
	free(data_page);

	regs_page[4] = data_num_pages;
	/* write the registers to the NVRAM before closing */
	iat_flash_write_regs();
	free(regs_page);
}


void nvram_set_pos(UNS32 pos)
/* set the current position in the NVRAM to pos */
{
}


void nvram_new_firmwave()
{
/*
	this function is called whenever a new firmware is about
	to be written in the NVRAM
*/
	data_addr = regs_page[1] + regs_page[4]*NVRAM_BLOCK_SIZE;
	if (data_addr > NVRAM_MAX_SIZE)
		data_addr = NVRAM_BLOCK_SIZE;
}

int _get_data_len(int type)
{
	int len = 0; /* number of bytes */
	switch(type)
	{
		case  boolean:
			len = 1;
			break;

		case  int8:
		case  uint8:
			len = 1;
			break;
		case  int16:
		case  uint16:
			len = 2;
			break;
		case  int24:
		case  uint24:
			len = 3;
			break;
		case  int32:
		case  uint32:
		case  real32:
			len = 4;
			break;
		case  int40:
		case  uint40:
			len = 5;
			break;
		case  int48:
		case  uint48:
			len = 6;
			break;
		case  int56:
		case  uint56:
			len = 7;
			break;
		case  int64:
		case  uint64:
		case  real64:
			len = 8;
			break;
#if 0
/* TO DO */
		case  visible_string:
		case  octet_string:
		case  unicode_string:
		case  time_of_day:
		case  time_difference:
#endif
	}

	return len;
}


char nvram_write_data(int type, int access_attr, void *data)
/* return 0 if successfull */
{
	int len = _get_data_len(type);

	if (data_len+len > NVRAM_BLOCK_SIZE)
	{
		iat_flash_write_page(data_addr);
		data_len = 0;
		data_addr += NVRAM_BLOCK_SIZE; 

		/* wrap-around address pointer */
		if (data_addr > NVRAM_MAX_SIZE)
			data_addr = NVRAM_BLOCK_SIZE;

		data_num_pages++;
	}
		
	memcpy(((char *)data_page)+data_len, data, len);

	data_len += len;

	return 0;
}


char nvram_read_data(int type, int access_attr, void *data)
/* return 0 if successful */
{
	int len = _get_data_len(type);

	if (data_len+len > NVRAM_BLOCK_SIZE)
	{
		data_addr += NVRAM_BLOCK_SIZE;

		/* wrap-around address pointer */
		if (data_addr > NVRAM_MAX_SIZE)
			data_addr = NVRAM_BLOCK_SIZE;

		iat_flash_read_page(data_addr);
		data_len = 0;		
	}

	memcpy(data, ((char *)data_page)+data_len, len);

	data_len += len;

	return 0;
}

/*
	NVRAM registers at block 0
	pos        description
	0          version of the current dictionnary
	1          starting address for data block
	2          date of last writing
	3          address of the previous dictionnary          
	4          size in pages of the current dict
*/
void nvram_write_reg(UNS32 reg, UNS16 pos)
/* write reg at the position in the data block 0 */
{
	regs_page[pos] = reg;
}

UNS32 nvram_read_reg(UNS16 pos)
/* read reg at the position in the data block 0 */
{
	return regs_page[pos];
}


/*
	LED
*/

void led_set_redgreen(UNS8 bits)
/* bits : each bit of this uns8 is assigned a led 
          0=off, 1=on
*/
{
	lpc2138_redgreenled_set(bits);
}

