/*
                   canfestival@canopencanada.ca

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

flash.c
	save to / retrieve from  the non-volatile memory
	to be tested
	- can we write/read into an address without working with the whole page (256bytes)
*/


#include <stdio.h>
#include <stdlib.h>

#include "applicfg.h"
#include "data.h"
#include "objdictdef.h"

#include "lpc2138_defs.h"                    /* LPC21xx definitions */

#define IAP_LOCATION 			0x7ffffff1


// define a page of data of NVRAM_BLOCK_SIZE bytes
//
short data_len; /* 0 to NVRAM_BLOCK_SIZE bytes */
short data_num_pages;
unsigned int *data_page = NULL;
unsigned int data_addr;

unsigned int *regs_page = NULL;

// local definitons
void ee_erase(unsigned int ,unsigned int[]);		//function erases EEPROM
void ee_write_page(unsigned int);	//function adds a record in EEPROM
void ee_read_page(unsigned int);	//function reads the latest valid record in EEPROM

typedef void (*IAP)(unsigned int [],unsigned int[]);
IAP iap_entry;


int iat_init()
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


void iat_end()
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



/************************************************************************/
/*                                                                    	*/
/* function:								*/
/*  void ee_erase(unsigned int command_ee,unsigned int result_ee[])	*/
/*                                                                     	*/
/* type: void                                                          	*/
/*                                                                     	*/
/* parameters: 								*/
/* 	command_ee   - Not used.  	               			*/
/*  result_ee[0] - Returns a response to the last IAP command used.	*/
/*                 0 - EEPROM successfully erased.			*/
/*                 For all other response values, see microcontroller 	*/
/*		   User Manual, IAP Commands and Status Codes Summary.	*/
/*  result_ee[1] - Not used.                         			*/
/*                                                                     	*/
/* version: 1.1 (01/27/2006)                                           	*/
/*                                                                     	*/
/* constants defined in LPC2k_ee.h used in this function:              	*/
/*  EE_SEC_L 	 - microcontroller's Flash sector where EEPROM begins	*/
/*  EE_SEC_H 	 - microcontroller's Flash sector where EEPROM ends	*/
/*  EE_CCLK		 - microcontroller's system clock (cclk)        */
/*                                                                     	*/
/* description:								*/
/*  This function erases LPC2000 on-chip Flash sectors selected to act 	*/
/*  as an EEPROM. All Flash sectors between EE_SEC_L abd EE_SEC_H	*/
/*  (including these sectors) will be erased using the In Application	*/
/*  Programming (IAP) routines (see User Manual for more details). 	*/
/*  Also, this function disables all interrupts while erasing the       */
/*  EEPROM. If this is not needed, three lines of the ee_erase          */
/*  subroutine can simply be commented-out without affecting the        */
/*  routine performance at all.                                         */
/*                                                                     	*/
/* revision history:                                                   	*/
/* - Rev. 1.1 adds interrupt disable feature.				*/
/*                                                                     	*/
/************************************************************************/
void ee_erase(unsigned int command_ee,unsigned int result_ee[])
{
	unsigned int command_iap[5];
	unsigned int result_iap[3];
	unsigned long int enabled_interrupts;

	enabled_interrupts = VICIntEnable;  //disable all interrupts
	VICIntEnClr        = enabled_interrupts;

	command_iap[0]=50;	// prepare sectors from EE_SEC_L to EE_SEC_H for erase
	command_iap[1]=EE_SEC_L;
	command_iap[2]=EE_SEC_H;
	iap_entry=(IAP) IAP_LOCATION;
	iap_entry(command_iap,result_iap);

	command_iap[0]=52;	// erase sectors from EE_SEC_L to EE_SEC_H
	command_iap[1]=EE_SEC_L;
	command_iap[2]=EE_SEC_H;
	command_iap[3]=EE_CCLK;
	iap_entry=(IAP) IAP_LOCATION;
	iap_entry(command_iap,result_iap);

	command_iap[0]=53;	// blankcheck sectors from EE_SEC_L to EE_SEC_H
	command_iap[1]=EE_SEC_L;
	command_iap[2]=EE_SEC_H;
	iap_entry=(IAP) IAP_LOCATION;
	iap_entry(command_iap,result_iap);

	VICIntEnable = enabled_interrupts;  //restore interrupt enable register

	result_ee[0]=result_iap[0];
	return;
}

/************************************************************************/
/*                                                                    	*/
/* function: 								*/
/*  void ee_write(unsigned int command_ee,unsigned int result_ee[])	*/
/*                                                                     	*/
/* type: void                                                          	*/
/*                                                                     	*/
/* parameters: 								*/
/* 	command_ee   - An address of a content of ee_data type that has	*/
/*                 to be programmed into EEPROM.                       	*/
/*  result_ee[0] - Returns a response to the last IAP command used.	*/
/*                 0 - data successfully programmed in EEPROM.		*/
/*               501 - no space in EEPROM to program data.             	*/
/*                 For all other response values, see microcontroller 	*/
/*		   User Manual, IAP Commands and Status Codes Summary.	*/
/*  result_ee[1] - Not used.                           			*/
/*                                                                     	*/
/* version: 1.1 (01/27/2006)                                           	*/
/*                                                                     	*/
/* constants defined in LPC2k_ee.h used in this function:              	*/
/*  EE_BUFFER_SIZE 	   - IAP buffer size; must be 256 or 512 	*/
/*  NO_SPACE_IN_EEPROM - EEPROM is full and no data can be programmed	*/
/*  EE_BUFFER_MASK	   - parameter used for interfacing with IAP	*/
/*  EE_REC_SIZE   	   - ee_data structure size in bytes        	*/
/*  EE_SEC_L 	 	   - micro's Flash sector where EEPROM begins	*/
/*  EE_SEC_H 	 	   - micro's Flash sector where EEPROM ends	*/
/*  EE_CCLK		 	   - micro's system clock (cclk)       	*/
/*                                                                     	*/
/* description:								*/
/*  This function writes a single structure of ee_data type into the	*/
/*  EEPROM using an In Application	Programming (IAP) routines (see */
/*  User Manual for more details). command_ee contains an address of	*/
/*  this structure. EEPROM is scanned for the last (if any) record 	*/
/*  identifier (EE_REC_ID), and a new record is added next to it.      	*/
/*  Also, this function disables all interrupts while erasing the       */
/*  EEPROM. If this is not needed, three lines of the ee_write          */
/*  subroutine can simply be commented-out without affecting the        */
/*  routine performance at all.                                         */
/*                                                                     	*/
/* revision history:                                                   	*/
/* - Rev. 1.1 fixes a bug related to verifying a content written into	*/
/*   the EEPROM. 1.0 was reporting missmatch even when there were no	*/
/*   problems at all.							*/
/*   Rev. 1.1 adds interrupt disable feature.				*/
/*                                                                     	*/
/************************************************************************/

void ee_write_page(unsigned int addr)
{
	unsigned long int enabled_interrupts;
	// unsigned char ee_buffer[16];
	unsigned int command_iap[5], result_iap[3];

	enabled_interrupts = VICIntEnable;  //disable all interrupts
	VICIntEnClr        = enabled_interrupts;

	iap_entry = (IAP) IAP_LOCATION;

	// prepare sectors from EE_SEC_L to EE_SEC_H for erase
	command_iap[0] = 50;			
	command_iap[1] = EE_SEC_L;
	command_iap[2] = EE_SEC_H;
	iap_entry(command_iap, result_iap);

	// copy RAM to flash/eeprom
	command_iap[0] = 51;
	command_iap[1] = (unsigned int) (addr & EE_START_MASK); // 256 kb boundary
	command_iap[2] = (unsigned int) (data_page);            // should be on a word boundary
	command_iap[3] = 256;
	command_iap[4] = EE_CCLK;
	iap_entry(command_iap, result_iap);

#if 0 
	// compare RAM and flash/eeprom
	command_iap[0] = 56;
	command_iap[1] = (unsigned int) data;
	command_iap[2] = addr;
	command_iap[3] = dlen;
	iap_entry(command_iap, result_iap);
#endif

	VICIntEnable = enabled_interrupts;  //restore interrupt enable register
}


/************************************************************************/
/*                                                                    	*/
/* function: 								*/
/*  void ee_read(unsigned int command_ee,unsigned int result_ee[])	*/
/*                                                                     	*/
/* type: void                                                          	*/
/*                                                                     	*/
/* parameters: 								*/
/* 	command_ee   - Not used.					*/
/*  result_ee[0] - Returns a response.					*/
/*                 0 - data successfully found in EEPROM.		*/
/*               500 - no data/records available in EEPROM.		*/
/*  result_ee[1] - an address of the last record of ee_data type	*/
/*				   in EEPROM.  	              		*/
/*                                                                     	*/
/* version: 1.1 (01/27/2006)                                           	*/
/*                                                                     	*/
/* constants defined in LPC2k_ee.h used in this function:              	*/
/*  NO_RECORDS_AVAILABLE - EEPROM is empty/no records identifiable	*/
/*			   with a record identifier (EE_REC_ID) found	*/
/*  EE_ADR_L 	    - micro's Flash address from where EEPROM begins	*/
/*  EE_REC_SIZE    - size (in bytes) of a ee_data structure        	*/
/*                                                                 	*/
/* description:								*/
/*  This function scans an EEPROM content looking for the last record 	*/
/*  that can be identified with a record identifier (EE_REC_ID). When 	*/
/*  such data is found, its address is passed as result_ee[1].		*/
/*                                                                     	*/
/* revision history:                                                   	*/
/* - Rev. 1.0 had problems with accessing the last record in a fully	*/
/*   occupied EEPROM. Rev. 1.1 fixes this.				*/
/*                                                                     	*/
/************************************************************************/
void ee_read_page(unsigned int addr)
{
	memcpy(data_page, (unsigned int *)addr, sizeof(unsigned int)*64);
}


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////


/*
	CAN FESTIVAL interface functions
*/


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



