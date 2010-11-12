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

#include <stdio.h>
#include "canfestival.h"

/* only for usleep() */
#include <linux/delay.h>

#define CHTX 2 /* channel number of the CO-PCICAN */
#define CHRX 0 /* channel number of the CO-PCICAN */

void mySyncHandler( CO_Data* d )
{
  printf( "  got a SYNC message...\n" );
}

int main()
{
  LIB_HANDLE driver;

  /* handles for additional CO-PCICAN functions */
  int (*get_fd_of_port)( CAN_PORT port );
  int (*co_pcican_enter_run_mode)( const int fd );
  int (*co_pcican_enter_config_mode)( const int fd );
  int (*co_pcican_select_channel)( const unsigned char channel, const unsigned int direction );
  int (*co_pcican_configure_selected_channel)( const int fd, s_BOARD *board, const unsigned int direction );

  s_BOARD  bd;
  CO_Data  myData;
  CAN_PORT port;
  Message myMessage;
  const char busname[]  = "/dev/co_pcican-0";
  const char baudrate[] = "1M";
  int fd;

  memset( &myData, 0x00, sizeof(CO_Data) );
  myData.CurrentCommunicationState.csSYNC = 1;
  myData.post_sync = mySyncHandler;
  bd.busname  = (char*)busname;
  bd.baudrate = (char*)baudrate;

  printf( "This example sends three SYNCs from port#%u to port#%u with a CO-PCICAN card\n", CHTX, CHRX );

  driver = LoadCanDriver( "/usr/local/lib/libcanfestival_can_copcican_linux.so" );

  if( driver == NULL )
  {
    /* something strange happenend */
    return 0;
  }

  /* dynamic load of additional library functions */
  get_fd_of_port = dlsym( driver, "get_fd_of_port" );
  co_pcican_enter_run_mode = dlsym( driver, "co_pcican_enter_run_mode" );
  co_pcican_enter_config_mode = dlsym( driver, "co_pcican_enter_config_mode" );
  co_pcican_select_channel = dlsym( driver, "co_pcican_select_channel" );
  co_pcican_configure_selected_channel = dlsym( driver, "co_pcican_configure_selected_channel" );

  /* direction: 0=RX, 1=TX */
  co_pcican_select_channel( CHRX, 0 );
  co_pcican_select_channel( CHTX, 1 );

  port = canOpen( &bd, &myData );

  if( port == NULL )
  {
    UnLoadCanDriver( driver );

    /* something strange happenend */
    return 0;
  }

  /* get file descriptor to control the opened device */
  fd = get_fd_of_port( port );

  co_pcican_enter_config_mode( fd );
  co_pcican_configure_selected_channel( fd, &bd, 0 );
  co_pcican_configure_selected_channel( fd, &bd, 1 );
  co_pcican_enter_run_mode( fd );

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

  /* mySyncHandler() is called by the receive thread and shows a received SYNC message on console */
  usleep( 1*1000*1000 ); /* 1s */

  canClose( &myData );

  UnLoadCanDriver( driver );

  return 0;
}

