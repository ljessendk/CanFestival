/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Cosateq GmbH & Co.KG
               http://www.cosateq.com/
               http://www.scale-rt.com/

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
	COMEDI interface for CO-PCICAN card.
*/

#ifndef _CO_PCICANOPS_
#define _CO_PCICANOPS_

#define NUM_CAN_CHANNELS 4

#define CMDQ_OPC_ENTER_RUN_MODE     0x01
#define CMDQ_OPC_ENTER_CONFIG_MODE  0x02
#define CMDQ_OPC_SET_CONFIG_CHANNEL 0xA2

typedef struct message_t
{
  unsigned long type;
  unsigned long size;
  unsigned long id;
  unsigned char data[8];
  unsigned long timestamp_lo;
  unsigned long timestamp_hi;
} MESSAGE_T;

/*=============================================================================
  directions
  ===========================================================================*/
#define RX         0
#define TX         1

/*=============================================================================
  supported types
  ===========================================================================*/
#define MSG_EXT    0x00000001
#define MSG_RTR    0x00000002



#endif

