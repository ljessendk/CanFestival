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

#include <applicfg.h>
#include <def.h>

#include <can.h>
#include <data.h>
#include <objdictdef.h>
#include <objacces.h>
#include "can_driver.h"

#ifdef LED_ENABLE
#include "led.h"
#else
#define led_set_state(a,b)
#endif

#include "lss.h"



/*
	NOTES

	1. since in the LSS protocol all LSS Slave use the same COB, only 1 Slave
	must be allowed to communicate with the Master

	2. the Master always take the iniative. the Slave is only allowed to transmit
	within a confirmed service

	3. requesting message (from the Master) using COB-ID 2021 and response messages
	(from the Slave) using COB-ID 2020
*/

/*
	0 = this slave is not talking to the master
	1 = this slave is talking to the master (this slave has been selected via )
*/
int	slave_selector;

int	current_mode;

int	lss_table_selector, lss_table_index;


/* slave storing the information sent by the master */
UNS32 lss_buffer[10];
/*
	how this buffer is used

	for a SLAVE 
	[0..3]  used to store the LSS Address
	[4..9]  use by LSS Identify Remort Slave

	for the MASTER
	[0..3] hold the answer from the slave regarding its ID
*/


void lss_copy(UNS8 *data, UNS32 value)
/* transfert 32 bits value into uns8 data vector */
{
	data[0] = value & 0xff;
	data[1] = (value>>8) & 0xff;
	data[2] = (value>>16) & 0xff;
	data[3] = (value>>24) & 0xff;
}


UNS32 lss_get_value(UNS8 *data)
/* build a 'UNS32' value from a 'unsigned char' vector */
{
	return data[0] + (data[1]<<8) + (data[2]<<16) + (data[3]<<24);
}


void lss_init_msg(Message *msg)
{
	msg->cob_id.w = 0;
	msg->rtr = 0;
	msg->len = 0;
	msg->data[0] = 0;
	msg->data[1] = 0;
	msg->data[2] = 0;
	msg->data[3] = 0;
	msg->data[4] = 0;
	msg->data[5] = 0;
	msg->data[6] = 0;
	msg->data[7] = 0;
}


void lss_SwitchModeGlobal(CO_Data *d, UNS32 mode)
/*
	this service is used to switch all LSS slaves in the network between operation
	mode and configuration mode.
*/
{	
	Message msg;
	lss_init_msg(&msg);

	/* 
		sending a COB-ID 2021
		[0] = 4 (for switch mode global)
		[1] = 0 for operation mode, = 1 for configuration mode
		[2..7] = 0 reserved
	*/
	
	if (! *(d->iam_a_slave))
	{
		msg.cob_id.w = 0x07E5 /* 2021 */;

		msg.len = 2;
		msg.data[0] = 4;
		msg.data[1] = (UNS8)mode;
	
		/* transmit */
		(*d->canSend)(&msg);
	}
	else
	{
		/* set mode global */
		current_mode = mode;
	}
}


void lss_SwitchModeSelective_master(CO_Data *d, UNS32 *LSSaddr)
/*
	LSS address : <vendor-id> <product-code> <revision-number> <serial-number>
	vendor-id : provided by CiA
	identical to the CANopen identity object

	select the slave corresponding to this ADDRESS
*/
{
	Message msg;
	lss_init_msg(&msg);

	if (*(d->iam_a_slave)) /* not the master */
		return; 

	msg.cob_id.w = 0x07E5 /* 2021 */;
	msg.len=5;

	msg.data[0] = 64;
	lss_copy(msg.data+1, LSSaddr[0]); 
	/* transmit */
	(*d->canSend)(&msg);

	msg.data[0] = 65;
	lss_copy(msg.data+1, LSSaddr[1]); 
	/* transmit */
	(*d->canSend)(&msg);

	msg.data[0] = 66;
	lss_copy(msg.data+1, LSSaddr[2]); 
	/* transmit */
	(*d->canSend)(&msg);

	msg.data[0] = 67;
	lss_copy(msg.data+1, LSSaddr[3]); 
	/* transmit */
	(*d->canSend)(&msg);
}


UNS32 lss_validate_address(CO_Data* d)
{
#if 0 
	extern s_identity obj1018;

	/* maybe we should go throught getODentry but those
	data are 1) RO and 2) the proper ID of this device */
#else
	UNS32  r, vendor_id, product_code, revision_number, serial_number;
	UNS8   sz = sizeof(UNS32), dt = int32;

	r = getODentry(d, 0x1018, 1, (void *)&vendor_id, &sz, &dt, 0);
	r = getODentry(d, 0x1018, 2, (void *)&product_code, &sz, &dt, 0);
	r = getODentry(d, 0x1018, 3, (void *)&revision_number, &sz, &dt, 0);
	r = getODentry(d, 0x1018, 4, (void *)&serial_number, &sz, &dt, 0);

#endif
	/*
		if
		lss_buffer[0] == vendor-id
		lss_buffer[1] == product code
		lss_buffer[2] == revision
		lss_buffer[3] == serial

		then return 1
		else return 0;
	*/

	if (lss_buffer[0] == vendor_id  &&
	    lss_buffer[1] == product_code &&
	    lss_buffer[2] == revision_number &&
	    lss_buffer[3] == serial_number)
	{
		return 1;
	}

	return 0;
}


void lss_SwitchModeSelective_slave(CO_Data *d)
/* 
	SwitchModeSelective for the SLAVE
	received the frames from the master and start building 
	the lss address 
*/
{
	Message msg;
	lss_init_msg(&msg);

	/*
		the master broadcast the address of a particular slave
		for 64,65 and 66 store the partial address
		when 67 arrive process the call and asknowledge if necessary
	*/

	if (lss_validate_address(d))
	{
		/* slave should transmit cob_id 2020  */
		msg.cob_id.w = 0x07E4 /* 2020 */;

		msg.len = 2;
		msg.data[0] = 68;
		msg.data[1] = (UNS8)current_mode;

		/* transmit */
		(*d->canSend)(&msg);
	}

	/* reset the address */
	lss_buffer[0] = 0;
	lss_buffer[1] = 0;
	lss_buffer[2] = 0;
	lss_buffer[3] = 0;
}


void lss_ConfigureNode_ID(CO_Data *d, UNS32 node_id)
/*
	through this service the LSS Master configures the NMT-address
	parameter of a LSS slave.
*/
{
	Message msg;
	lss_init_msg(&msg);

	if (!*(d->iam_a_slave))
	{
		msg.cob_id.w = 0x07E5 /* 2021 */;

		msg.len = 2;
		msg.data[0] = 17;
		msg.data[1] = (UNS8)node_id;

		/* transmit */
		(*d->canSend)(&msg);
	}
	else
	{
		/*
			receiving NODE ID from the master
		*/

		/*
			error code 
			0 = success
			1 = node id out of range
			2..254 = reserved
			255 = implementation specific error occured
				spec error = mode detailed error
		*/
		msg.cob_id.w = 0x07E4 /* 2020 */;

		msg.len = 3;
		msg.data[0] = 17;
		/* msg.data[1] = error code */
		/* msg.data[2] = spec error */

		/* transmit */
		(*d->canSend)(&msg);
	}
}


void lss_ConfigureBitTimingParameters(CO_Data *d, 
                                      UNS32 table_selector,
                                      UNS32 table_index)
/*
	this service allows all LSS slaves in configuration mode.
	must be followed by an Activate Bit Timing Parameters
*/
{
	Message msg;
	lss_init_msg(&msg);

	if (!*(d->iam_a_slave))
	{
		msg.cob_id.w = 0x07E5 /* 2021 */;

		msg.len = 3;
		msg.data[0] = 19;
		msg.data[1] = (UNS8)table_selector;
		msg.data[2] = (UNS8)table_index;

		/* transmit */
		(*d->canSend)(&msg);
	}
	else
	{
		UNS8 error_code;

		/* validate if this baudrate is possible */
		if (table_selector == 0  &&  baudrate_valid(table_index) == 1)
		{
			lss_table_selector = table_selector;
			lss_table_index = table_index;
		}
		else
			error_code = 1; /* bit timing not supported */

		msg.cob_id.w = 0x07E4 /* 2020 */;

		msg.len = 3;
		msg.data[0] = 19;
		msg.data[1] = error_code;
		msg.data[2] = 0;

		/* transmit */
		(*d->canSend)(&msg);
	}

	led_set_state(d, LED_AUTOBITRATE);
}


void lss_ActivateBitTimingParameters_master(CO_Data *d, unsigned short switch_delay)
/*
	switch_delay in ms

	switch_delay has to be longer than the longest timeof any node
	in the network to avoid that a node already switches while another
	stills transmist the old bit timing parameters
*/
{
	Message msg;
	lss_init_msg(&msg);

	if (*(d->iam_a_slave))
		return;
	
	msg.cob_id.w = 0x07E5 /* 2021 */;

	msg.len = 3;
	msg.data[0] = 21;
	msg.data[1] = (UNS8)(switch_delay &  0xff);
	msg.data[2] = (UNS8)((switch_delay >> 8) & 0xff);

#ifdef LED_ENABLE
	led_set_state(LED_AUTOBITRATE);		
#endif
	/* transmit */
	(*d->canSend)(&msg);
}


void lss_ActivateBitTimingParameters_slave(UNS8 byte_low, UNS8 byte_high)
{
	/* rebuild the delay value (short) from the 2 (UNS8) data */
	unsigned short switch_delay = byte_low + (((UNS16)byte_high)<<8);

	/* set the baudrate to the value proposed by the master */
	if (lss_table_selector == 0)
		baudrate_set(lss_table_index);

	/* wait for switch_delay ms before continuing */
}


void lss_StoreConfiguredParameters(CO_Data *d)
/*
	store configured parameters into non-volatile storage
*/
{
	Message msg;
	lss_init_msg(&msg);

	if (!*(d->iam_a_slave))
	{
		msg.cob_id.w = 0x07E5 /* 2021 */;

		msg.len = 1;
		msg.data[0] = 23;

		/* transmit */
		(*d->canSend)(&msg);
	}
	else
	{
		msg.cob_id.w = 0x07E4 /* 2020 */;

		msg.data[0] = 23;
		/* msg.data[1] = error code; */
		/* msg.data[2] = spec err */
		
		/* transmit */
		(*d->canSend)(&msg);
	}
}


void lss_InquireLSSAddress_master(CO_Data *d, int flag)
/*
	this service allows to determine the LSS-address parameters of a LSS-slave in
	configuration mode

	request 1 single item of the LSS address
	0 = request vendor-id
	1 = request product-id
	2 = request revision
	3 = request serial
*/
{
	Message msg;
	lss_init_msg(&msg);

	if (!*(d->iam_a_slave))
	{
		msg.cob_id.w = 0x07E5 /* 2021 */;

		msg.len = 1;
		msg.data[0] = 90 + flag;
	
		/* transmit */
		(*d->canSend)(&msg);
	}
}


int lss_InquireLSSAddress_slave(CO_Data *d, int cs)
{
	Message msg;
	lss_init_msg(&msg);

	if (!*(d->iam_a_slave)) /* not a slave */
		return -1;

	UNS32 value = 0;

	switch(cs)
	{
		case 90:
			value = 0; /* = vendor id */
			break;
		case 91:
			value = 0; /* = product code */
			break;
		case 92:
			value = 0; /* = revision */
			break;
		case 93:
			value = 0; /* = serial */
			break;
	}

	if (value > 0)
	{
		msg.cob_id.w = 0x07E4 /* 2020 */;
	
		msg.len=5;
		msg.data[0] = cs;
		lss_copy(msg.data+1, value);
		
		/* transmit */
		(*d->canSend)(&msg);

		return 0;
	}

	return -1;
}


void lss_IdentifyRemoteSlaves(CO_Data *d, 
                              UNS32 vendor_id,
                              UNS32 product_code,
                              UNS32 rev_low,
                              UNS32 rev_high,
                              UNS32 serial_low,
                              UNS32 serial_high)
/*
	throught this service, the LSS Master requests all LSS slave whose LSS address
	meets the LSSaddr_sel to idenfity themselves through LSS Identify Slave
*/
{
	Message msg;
	lss_init_msg(&msg);

	if (!*(d->iam_a_slave))
	{
		/*
			request answers from all slaves corresponding
			to the revision and serial range of values
		*/

		msg.cob_id.w = 0x07E5 /* 2021 */;
		msg.len=5;

		msg.data[0] = 70;
		lss_copy(msg.data+1, vendor_id);
		/* transmit */
		(*d->canSend)(&msg);

		msg.data[0] = 71;
		lss_copy(msg.data+1, product_code); 
		/* transmit */
		(*d->canSend)(&msg);
	
		msg.data[0] = 72; /* revision number low */
		lss_copy(msg.data+1, rev_low);
		/* transmit */
		(*d->canSend)(&msg);

		msg.data[0] = 73; /* revision number high */
		lss_copy(msg.data+1, rev_high);
		/* transmit */
		(*d->canSend)(&msg);
	
		msg.data[0] = 74; /* serial number low */
		lss_copy(msg.data+1, serial_low);
		/* transmit */
		(*d->canSend)(&msg);

		msg.data[0] = 75; /* serial number high */
		lss_copy(msg.data+1, serial_high);
		/* transmit */
		(*d->canSend)(&msg);
	}
}


int lss_validate_range_addr(CO_Data *d)
{
	/*
		if 

		lss_buffer[4] == vendor_id
		lss_buffer[5] == product code
		lss_buffer[6] <= revision <= lss_buffer[7]
		lss_buffer[8] <= serial <= lss_buffer[9]

		then return 1
		else return 0
	*/

	UNS32  r, vendor_id, product_code, revision_number, serial_number;
	UNS8   sz = sizeof(UNS32), dt = int32;

	r = getODentry(d, 0x1018, 1, (void *)&vendor_id, &sz, &dt, 0);
	r = getODentry(d, 0x1018, 2, (void *)&product_code, &sz, &dt, 0);
	r = getODentry(d, 0x1018, 3, (void *)&revision_number, &sz, &dt, 0);
	r = getODentry(d, 0x1018, 4, (void *)&serial_number, &sz, &dt, 0);

	if (lss_buffer[4] == vendor_id  &&
	    lss_buffer[5] == product_code &&
	    lss_buffer[6] <= revision_number &&
	    revision_number <= lss_buffer[7] &&
	    lss_buffer[8] <= serial_number &&
	    serial_number <= lss_buffer[9])
	{
		return 1;
	}

	return 0;
}


void lss_IdentifySlave(CO_Data *d)
/*
	through this service, an LSS slave indicates that it is a slave
	with LSS address within the LSSaddr_sel
*/
{
	Message msg;
	lss_init_msg(&msg);

	if (lss_validate_range_addr(d))
	{
		msg.cob_id.w = 0x07E4 /* 2020 */;

		msg.len = 1;
		msg.data[0] = 79;

		/* transmit */
		(*d->canSend)(&msg);
	}

	/* reset */
	lss_buffer[4] = 0;
	lss_buffer[5] = 0;
	lss_buffer[6] = 0;
	lss_buffer[7] = 0;
	lss_buffer[8] = 0;
	lss_buffer[9] = 0;
}


int lss_process_msg(CO_Data *d, Message *msg)
{
	/* process the incoming message */
	if (msg->cob_id.w == 0x07E4 /* 2020 */)
	{
		// should be the master
		// a slave just answered
		switch(msg->data[0])
		{
			/* slave acknowledging the SwitchModeSelective call */
			case 68: 
				/* msg->data[1] contains the 'mode global' value from the slave*/
				break;

			/* a slave had acknowledged the call from LSS Identify Remote Slave */
			case 79:
				break;

			/* the slave acknowledged and sent the requested data */
			case 90:
				lss_buffer[0] = lss_get_value(msg->data+1);
				/* action ? */
				break;
			case 91:
				lss_buffer[1] = lss_get_value(msg->data+1);
				/* action ? */
				break;
			case 92:
				lss_buffer[2] = lss_get_value(msg->data+1);
				/* action ? */
				break;
			case 93:
				lss_buffer[3] = lss_get_value(msg->data+1);
				/* action ? */
				break;
		}
	}

	else if (msg->cob_id.w == 0x07E5 /* 2021 */)
	{
		// should be a slave
		// the master sent a request
		switch(msg->data[0])
		{
			case 4:
				lss_SwitchModeGlobal(d, msg->data[1]);
				break;

			case 17:
				lss_ConfigureNode_ID(d, msg->data[1]);
				break;

			case 19:
				lss_ConfigureBitTimingParameters(d, msg->data[1], msg->data[2]);
				break;
			case 21:
				lss_ActivateBitTimingParameters_slave(msg->data[1], msg->data[2]);
				break;

			/* Switch Mode Selective */
			case 64:
				lss_buffer[0] = lss_get_value(msg->data+1);
				break;
			case 65:
				lss_buffer[1] = lss_get_value(msg->data+1);
				break;
			case 66:
				lss_buffer[2] = lss_get_value(msg->data+1);
				break;
			case 67:
				lss_buffer[3] = lss_get_value(msg->data+1);
				lss_SwitchModeSelective_slave(d);
				break;

			/* Identify Remote Slave */
			case 70:
				lss_buffer[4] = lss_get_value(msg->data+1);
				break;
			case 71:
				lss_buffer[5] = lss_get_value(msg->data+1);
				break;
			case 72:
				lss_buffer[6] = lss_get_value(msg->data+1);
				break;
			case 73:
				lss_buffer[7] = lss_get_value(msg->data+1);
				break;
			case 74:
				lss_buffer[8] = lss_get_value(msg->data+1);
				break;
			case 75:
				lss_buffer[9] = lss_get_value(msg->data+1);
				lss_IdentifySlave(d);
				break;

			/* Inquire Identify of Slave */
			case 90:
			case 91:
			case 92:
			case 93:
				lss_InquireLSSAddress_slave(d, msg->data[0]);
				break;
		}
	}

	return 0;
}
