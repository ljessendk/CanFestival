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

#include <stdio.h>
#include <stdlib.h>

#include "applicfg.h"
// #include "objdictdef.h"

#include "lpc2138_pinout.h"
#include "lpc2138_defs.h"
#include "lpc2138.h"

#define IAP_LOCATION    0x7ffffff1

// define a page of data of 256 bytes
//
short          data_len; /* 0 to 256 bytes */
unsigned int   *data_page = NULL;
unsigned int   data_addr;

// local definitons
void ee_erase(unsigned int ,unsigned int[]);		//function erases EEPROM
void ee_write_page(unsigned int);	//function adds a record in EEPROM
void ee_read_page(unsigned int);	//function reads the latest valid record in EEPROM

typedef void (*IAP)(unsigned int [],unsigned int[]);
IAP iap_entry;


/***************************************************************************************/

void lpc2138_pinsel_set(int pin, LPC2138_PORT port, int size, int func) 
{
    int i;

    for (i = 0; i < size; pin++, i++) 
    {
        /* 2 bits par broche. */
        int shift = (pin - ((pin < 16) ? 0 : 16)) << 1;

        REG32_ADDR pinsel = (port == 1) ?
            (REG32_ADDR) P1_PINSEL2_ADDR : ((pin < 16) ?
                (REG32_ADDR) P0_PINSEL0_ADDR : (REG32_ADDR) P0_PINSEL1_ADDR);

        *pinsel = (*pinsel & ~(BITMASK_2 << shift)) | (func << shift);
    }
}

void lpc2138_pinsel_clear() 
{
    P0_PINSEL0 = 0x00000000;
    P0_PINSEL1 = 0x00000000;
    P1_PINSEL2 = 0x00000000;
}


int lpc2138_printf(void) 
{
    return lpc2138_fprintf(stdout);
}


int lpc2138_printf_pins(void) 
{
    return lpc2138_fprintf_pins(stdout);
}


int lpc2138_fprintf(FILE *stream) 
{
    return fprintf(stream,
        "[p0=0x%08X,p0_iodir=0x%08X,p0_pinsel0=0x%08X,p0_pinsel1=0x%08X," \
        "p1=0x%08X,p1_iodir=0x%08X,p1_pinsel2=0x%08X]",
        P0_IOPIN, P0_IODIR, P0_PINSEL0, P0_PINSEL1,
        P1_IOPIN, P1_IODIR, P1_PINSEL2);
}

int lpc2138_fprintf_pins(FILE *stream) 
{
    return fprintf(stream,
        "[cs_s1d13706=0x%X," \
        "cs_sja1000=0x%X," \
        "wait=0x%X," \
        "bhe=0x%X," \
        "interrupt_sja1000=0x%X," \
        "redgreenled=0x%X," \
        "ale=0x%X," \
        "rd=0x%X," \
        "wr=0x%X," \
        "data=0x%X," \
        "addresses=0x%X]",
        lpc2138_cs_s1d13706_get(),
        lpc2138_cs_sja1000_get(),
        lpc2138_wait_get(),
        lpc2138_bhe_get(),
        lpc2138_redgreenled_get(),
        lpc2138_interrupt_sja1000_get(),
        lpc2138_ale_get(),
        lpc2138_rd_get(),
        lpc2138_wr_get(),
        lpc2138_data_get(),
        lpc2138_addresses_get());
}


/*
	SJA1000 interface
*/

unsigned char sja1000_read(unsigned char addr8)
{
    unsigned char data;

    lpc2138_data_set_mode(LPC2138_MODE_OUTPUT);
    lpc2138_ale_set(1);
    lpc2138_data_set(addr8);

    lpc2138_ale_set(0);
    lpc2138_data_set_mode(LPC2138_MODE_INPUT);

    lpc2138_cs_sja1000_set(0);
    lpc2138_rd_set(0);
    data = lpc2138_data_get();
    data = lpc2138_data_get();

    lpc2138_rd_set(1);

    lpc2138_cs_sja1000_set(1);
    lpc2138_data_set_mode(LPC2138_MODE_OUTPUT);

    return data;
}


void sja1000_write(unsigned char addr8, unsigned char data)
{
    lpc2138_data_set_mode(LPC2138_MODE_OUTPUT);

    lpc2138_data_set(addr8);

    lpc2138_ale_set(1);

    lpc2138_ale_set(0);
    lpc2138_cs_sja1000_set(0);
    lpc2138_wr_set(0);

    lpc2138_data_set(data);

    lpc2138_wr_set(1);
    lpc2138_cs_sja1000_set(1);

}

/*
	FLASH interface
*/

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
void iat_flash_erase(unsigned int command_ee,unsigned int result_ee[])
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

void iat_flash_write_page(unsigned int addr)
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
void iat_flash_read_page(unsigned int addr)
{
	memcpy(data_page, (unsigned int *)addr, sizeof(unsigned int)*64);
}




