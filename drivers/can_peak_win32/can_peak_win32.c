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

#if defined(WIN32) && !defined(__CYGWIN__)
#define usleep(micro) Sleep(micro%1000 ? (micro/1000) + 1 : (micro/1000))
#else
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#endif

#include "cancfg.h"
#include "can_driver.h"

#ifndef extra_PCAN_init_params
	#define extra_PCAN_init_params /**/
#else
	#define extra_PCAN_init_params\
		,getenv("PCANHwType") ? strtol(getenv("PCANHwType"),NULL,0):0\
		,getenv("PCANIO_Port") ? strtol(getenv("PCANIO_Port"),NULL,0):0\
		,getenv("PCANInterupt") ? strtol(getenv("PCANInterupt"),NULL,0):0
#endif

static s_BOARD *first_board = NULL;
#ifdef PCAN2_HEADER_
static s_BOARD *second_board = NULL;
#endif

//pthread_mutex_t PeakCan_mutex = PTHREAD_MUTEX_INITIALIZER;

// Define for rtr CAN message
#define CAN_INIT_TYPE_ST_RTR MSGTYPE_STANDARD | MSGTYPE_RTR

/***************************************************************************/
int TranslateBaudeRate(char* optarg){
	if(!strcmp( optarg, "1M")) return CAN_BAUD_1M;
	if(!strcmp( optarg, "500K")) return CAN_BAUD_500K;
	if(!strcmp( optarg, "250K")) return CAN_BAUD_250K;
	if(!strcmp( optarg, "125K")) return CAN_BAUD_125K;
	if(!strcmp( optarg, "100K")) return CAN_BAUD_100K;
	if(!strcmp( optarg, "50K")) return CAN_BAUD_50K;
	if(!strcmp( optarg, "20K")) return CAN_BAUD_20K;
	if(!strcmp( optarg, "10K")) return CAN_BAUD_10K;
	if(!strcmp( optarg, "5K")) return CAN_BAUD_5K;
	if(!strcmp( optarg, "none")) return 0;
	return 0x0000;
}

void
canInit (s_BOARD *board)
{
	int baudrate;
	
#ifdef PCAN2_HEADER_
	// if not the first handler
	if(second_board == (s_BOARD *)board)
		if(baudrate = TranslateBaudeRate(board->baudrate))
			CAN2_Init (baudrate,
			  CAN_INIT_TYPE_ST extra_PCAN_init_params);
#endif
	if(first_board == (s_BOARD *)board)
		if(baudrate = TranslateBaudeRate(board->baudrate))
			CAN_Init (baudrate,
			  CAN_INIT_TYPE_ST extra_PCAN_init_params);
}

/*********functions which permit to communicate with the board****************/
UNS8
canReceive_driver (CAN_HANDLE fd0, Message * m)
{
	UNS8 data;
	TPCANMsg peakMsg;

	DWORD Res;

	do{
		// We read the queue looking for messages.
		// 
		//pthread_mutex_lock (&PeakCan_mutex);
#ifdef PCAN2_HEADER_
		// if not the first handler
		if(second_board == (s_BOARD *)fd0)
			Res = CAN2_Read (&peakMsg);
		else
#endif
		if(first_board == (s_BOARD *)fd0)
			Res = CAN_Read (&peakMsg);
		else
			Res = CAN_ERR_BUSOFF;
	
		// A message was received
		// We process the message(s)
		// 
		if (Res == CAN_ERR_OK)
		{
			// if something different that 11bit or rtr... problem
			if (peakMsg.MSGTYPE & ~(MSGTYPE_STANDARD | MSGTYPE_RTR))
			{
				if (peakMsg.MSGTYPE == CAN_ERR_BUSOFF)
				{
					printf ("!!! Peak board read : re-init\n");
					canInit((s_BOARD*) fd0);
					usleep (10000);
				}
	
				// If status, return status if 29bit, return overrun
				//pthread_mutex_unlock (&PeakCan_mutex);
				return peakMsg.MSGTYPE ==
					MSGTYPE_STATUS ? peakMsg.DATA[2] : CAN_ERR_OVERRUN;
			}
			m->cob_id.w = peakMsg.ID;
			if (peakMsg.MSGTYPE == CAN_INIT_TYPE_ST)	/* bits of MSGTYPE_ */
				m->rtr = 0;
			else
				m->rtr = 1;
			m->len = peakMsg.LEN;	/* count of data bytes (0..8) */
			for (data = 0; data < peakMsg.LEN; data++)
				m->data[data] = peakMsg.DATA[data];	/* data bytes, up to 8 */
	
		}else{
		//pthread_mutex_unlock (&PeakCan_mutex);
		//if (Res != CAN_ERR_OK)
		//{
			if (!
				(Res & CAN_ERR_QRCVEMPTY || Res & CAN_ERR_BUSLIGHT
				 || Res & CAN_ERR_BUSHEAVY))
			{
				printf ("canReceive returned error (%d)\n", Res);
				return 1;
			}
			usleep (1000);		
		}
	}while(Res != CAN_ERR_OK);
	return 0;
}

/***************************************************************************/
UNS8
canSend_driver (CAN_HANDLE fd0, Message * m)
{
	UNS8 data;
	TPCANMsg peakMsg;
	peakMsg.ID = m->cob_id.w;	/* 11/29 bit code */
	if (m->rtr == 0)
		peakMsg.MSGTYPE = CAN_INIT_TYPE_ST;	/* bits of MSGTYPE_ */
	else
	{
		peakMsg.MSGTYPE = CAN_INIT_TYPE_ST_RTR;	/* bits of MSGTYPE_ */
	}
	peakMsg.LEN = m->len;
	/* count of data bytes (0..8) */
	for (data = 0; data < m->len; data++)
		peakMsg.DATA[data] = m->data[data];	/* data bytes, up to 8 */
	do
	{
#ifdef PCAN2_HEADER_
		// if not the first handler
		if(second_board == (s_BOARD *)fd0)
			errno = CAN2_Write (&peakMsg);
		else 
#endif
		if(first_board == (s_BOARD *)fd0)
			errno = CAN_Write (&peakMsg);
		else 
			goto fail;
		if (errno)
		{
			if (errno == CAN_ERR_BUSOFF)
			{
				printf ("!!! Peak board write : re-init\n");
				canInit((s_BOARD*)fd0);
				usleep (10000);
			}
			usleep (1000);
		}
	}
	while (errno != CAN_ERR_OK);
	return 0;
fail:
	return 1;
}

/***************************************************************************/
CAN_HANDLE
canOpen_driver (s_BOARD * board)
{
#ifdef PCAN2_HEADER_
	if(first_board != NULL && second_board != NULL)
#else
	if(first_board != NULL)
#endif
	{
		fprintf (stderr, "Open failed.\n");
		fprintf (stderr,
				 "can_peak_win32.c: no more can port available with this pcan library\n");
		fprintf (stderr,
				 "can_peak_win32.c: please link another executable with another pcan lib\n");
		return NULL;
	}

#ifdef PCAN2_HEADER_
	if(first_board == NULL)
		first_board = board;
	else
		second_board = board; 
#else
	first_board = board;
#endif

	canInit(board);
	
	return (CAN_HANDLE)board;
}

/***************************************************************************/
int
canClose_driver (CAN_HANDLE fd0)
{
#ifdef PCAN2_HEADER_
	// if not the first handler
	if(second_board == (s_BOARD *)fd0)
	{
		CAN2_Close ();
		second_board = (s_BOARD *)NULL;
	}else	
#endif
	if(first_board == (s_BOARD *)fd0)
	{
		CAN_Close ();
		first_board = (s_BOARD *)NULL;
	}
	return 0;
}
