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
	example for application with CO-PCICAN card.
*/

#include <linux/module.h>
#include <linux/comedi.h>
#include <linux/comedilib.h>
#include "canfestival.h"

/* only for mdelay() */
#include <linux/delay.h>

MODULE_LICENSE("GPL");

#define CHTX 2 /* channel number of the CO-PCICAN */
#define CHRX 0 /* channel number of the CO-PCICAN */

void mySyncHandler( CO_Data* d )
{
  printk( "test_copcican_comedi:   got a SYNC message...\n" );
}

static int main( void )
{
  s_BOARD  bd;
  CO_Data  myData;
  CAN_PORT port;
  Message myMessage;
  const char busname[]  = "/dev/comedi0";
  const char baudrate[] = "1M";
  comedi_t *dev;

  memset( &myData, 0x00, sizeof(CO_Data) );
  myData.CurrentCommunicationState.csSYNC = 1;
  myData.post_sync = mySyncHandler;
  bd.busname  = (char*)busname;
  bd.baudrate = (char*)baudrate;

  printk( "test_copcican_comedi: This example sends three SYNCs from port#%u to port#%u with a CO-PCICAN card\n", CHTX, CHRX );

  /* direction: 0=RX, 1=TX */
  co_pcican_select_channel( CHRX, 0 );
  co_pcican_select_channel( CHTX, 1 );

  port = canOpen( &bd, &myData );

  if( port == NULL )
  {
    /* something strange happenend */
    return 0;
  }

  /* get device pointer to control the opened device */
  dev = get_dev_of_port( port );

  co_pcican_enter_config_mode( dev );
  co_pcican_configure_selected_channel( dev, &bd, 0 );
  co_pcican_configure_selected_channel( dev, &bd, 1 );
  co_pcican_enter_run_mode( dev );

  memset( &myMessage, 0x00, sizeof(Message) );
  myMessage.cob_id = 0x80; /* SYNC message */
  myMessage.len = 1;
  myMessage.data[0] = 0xA5;

  /* SEND HERE */
  canSend( port, &myMessage );

  myMessage.data[0] = 0x5A;
  canSend( port, &myMessage );

  myMessage.data[0] = 0xA5;
  canSend( port, &myMessage );

  /* mySyncHandler() is called by the receive thread and shows a received SYNC message in the kernel log */
  mdelay( 1*1000 ); /* 1s */

  canClose( &myData );

  return 0;
}

static int init(void)
{
  printk("test_copcican_comedi for CanFestival loaded\n");

  return main();
}

static void exit(void)
{
  printk("test_copcican_comedi unloaded\n");
}

module_init(init);
module_exit(exit);
