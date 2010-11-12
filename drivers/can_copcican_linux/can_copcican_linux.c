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
	CAN driver interface for CO-PCICAN card.
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define NEED_PRINT_MESSAGE
#include "can_driver.h"
#include "def.h"
#include "co_pcicanops.h"

/* at the moment not threadsafe :-( */
static unsigned char selectedChannelRx = 0, selectedChannelTx = 0;

static int TranslateBaudRate( char* optarg )
{
  /* values see documentation of CO-PCICAN */
  if( !strcmp( optarg, "1M"    ) ) return 0;
  if( !strcmp( optarg, "800K"  ) ) return 1;
  if( !strcmp( optarg, "500K"  ) ) return 2;
  if( !strcmp( optarg, "250K"  ) ) return 3;
  if( !strcmp( optarg, "125K"  ) ) return 4;
  if( !strcmp( optarg, "100K"  ) ) return 5;
  if( !strcmp( optarg, "83.3K" ) ) return 6;
  if( !strcmp( optarg, "10K"   ) ) return 7;

  return -1;
}

/*********CO-PCICAN specific functions to communicate with the board**********/
typedef struct
{
  char used;
  CAN_HANDLE fd;
  void* receiveTask;
  void* d;
} CANPort; /* taken from drivers/unix.c */

int get_fd_of_port( CAN_PORT port )
{
  CANPort *thisPort = (CANPort*)port;
  CAN_HANDLE thisHandle;
  int *pfd;

  if( thisPort == NULL )
  {
    MSG("can_copcican_linux: get_fd_of_port(): thisPort is NULL\n");
    return -1;
  }

  thisHandle = thisPort->fd;

  if( thisHandle == NULL )
  {
    MSG("can_copcican_linux: get_fd_of_port(): thisHandle is NULL\n");
    return -1;
  }

  pfd = (int*)thisHandle;

  /*MSG("can_copcican_linux: get_fd_of_port(): handle is %d\n", *pfd);*/

  return *pfd;
}

int co_pcican_enter_run_mode( const int fd )
{
  co_pcican_config_t board_config;

  if( fd < 0 )
  {
    MSG("can_copcican_linux: co_pcican_enter_run_mode(): invalid file descriptor\n");
    return -1;
  }

  memset( &board_config, 0x00, sizeof(co_pcican_config_t) );
  board_config.opcode = CMDQ_OPC_ENTER_RUN_MODE;

  return ioctl( fd, CAN_CONFIG, &board_config );
}

int co_pcican_enter_config_mode( const int fd )
{
  co_pcican_config_t board_config;

  if( fd < 0 )
  {
    MSG("can_copcican_linux: co_pcican_enter_config_mode(): invalid file descriptor\n");
    return -1;
  }

  memset( &board_config, 0x00, sizeof(co_pcican_config_t) );
  board_config.opcode = CMDQ_OPC_ENTER_CONFIG_MODE;

  return ioctl( fd, CAN_CONFIG, &board_config );
}

int co_pcican_select_channel( const unsigned char channel, const unsigned int direction )
{
  if( channel >= NUM_CAN_CHANNELS )
  {
    MSG("can_copcican_linux: co_pcican_select_channel(): invalid channel\n");
    return -1;
  }

  /* at the moment not threadsafe :-( */
  switch( direction )
  {
    case RX: selectedChannelRx = channel;
             break;
    case TX: selectedChannelTx = channel;
             break;
    default: return -1;
  }

  return 0;
}

int co_pcican_configure_selected_channel( const int fd, s_BOARD *board, const unsigned int direction )
{
  co_pcican_config_t board_config;
  unsigned int selectedChannel;

  if( fd < 0 )
  {
    MSG("can_copcican_linux: co_pcican_configure_selected_channel(): invalid file descriptor\n");
    return -1;
  }

  if( board == NULL )
  {
    MSG("can_copcican_linux: co_pcican_configure_selected_channel(): board is NULL\n");
    return -1;
  }

  if( board->baudrate == NULL )
  {
    MSG("can_copcican_linux: co_pcican_configure_selected_channel(): baudrate is NULL\n");
    return -1;
  }

  switch( direction )
  {
    case RX: selectedChannel = selectedChannelRx;
             break;
    case TX: selectedChannel = selectedChannelTx;
             break;
    default: selectedChannel = 0xff;
  }

  if( selectedChannel >= NUM_CAN_CHANNELS )
  {
    MSG("can_copcican_linux: co_pcican_configure_selected_channel(): invalid channel selected\n");
    return -1;
  }

  memset( &board_config, 0x00, sizeof(co_pcican_config_t) );
  board_config.opcode   = CMDQ_OPC_SET_CONFIG_CHANNEL;
  board_config.param[0] = selectedChannel;
  board_config.param[1] = TranslateBaudRate( board->baudrate );

  return ioctl( fd, CAN_CONFIG, &board_config );
}

/*********functions which permit to communicate with the board****************/
UNS8 canReceive_driver( CAN_HANDLE fd0, Message *m )
{
  co_pcican_message_t canmsg;
  UNS8 ret = 0;
  int *pfd = (int*)fd0;

  if( pfd == NULL )
  {
    MSG("can_copcican_linux: canReceive_driver(): file descriptor is NULL\n");
    return 1;
  }

  if( *pfd < 0 )
  {
    MSG("can_copcican_linux: canReceive_driver(): invalid file descriptor\n");
    return 1;
  }

  if( selectedChannelRx >= NUM_CAN_CHANNELS )
  {
    MSG("can_copcican_linux: canReceive_driver(): invalid channel selected\n");
    return 1;
  }

  if( m == NULL )
  {
    MSG("can_copcican_linux: canReceive_driver(): message is NULL\n");
    return 1;
  }

  memset( &canmsg, 0x00, sizeof(co_pcican_message_t) );
  canmsg.channelnum = selectedChannelRx;

  ioctl( *pfd, CAN_READ, &canmsg );

  if( canmsg.timestamp_lo == 0 )
  {
    memset( m, 0x00, sizeof(Message) );

    m->cob_id = 0xffff; /* set to invalid so nothing happens */
  }
  else
  {
    m->len = canmsg.size;
    m->cob_id = canmsg.id;
    m->rtr = canmsg.type & MSG_RTR;

    if( !m->rtr )
    {
      /* this is for safety */
      if( m->len > 8 )
        m->len = 8;

      memcpy( m->data, canmsg.data, m->len);
    }
  }

  return ret;
}

/***************************************************************************/
UNS8 canSend_driver( CAN_HANDLE fd0, Message *m )
{
  co_pcican_message_t canmsg;
  UNS8 ret = 0;
  int *pfd = (int*)fd0;

  if( pfd == NULL )
  {
    MSG("can_copcican_linux: canSend_driver(): file descriptor is NULL\n");
    return 1;
  }

  if( *pfd < 0 )
  {
    MSG("can_copcican_linux: canSend_driver(): invalid file descriptor\n");
    return 1;
  }

  if( selectedChannelTx >= NUM_CAN_CHANNELS )
  {
    MSG("can_copcican_linux: canSend_driver(): invalid channel selected\n");
    return 1;
  }

  if( m == NULL )
  {
    MSG("can_copcican_linux: canSend_driver(): message is NULL\n");
    return 1;
  }

  memset( &canmsg, 0x00, sizeof(co_pcican_message_t) );
  canmsg.channelnum = selectedChannelTx;
  canmsg.size = m->len;
  canmsg.id = m->cob_id;

  if( canmsg.id >= 0x800 )
    canmsg.type |= MSG_EXT;

  if( m->rtr )
  {
    canmsg.type |= MSG_RTR;
  }
  else
  {
    /* this is for safety */
    if( canmsg.size > 8 )
      canmsg.size = 8;

    memcpy( canmsg.data, m->data, canmsg.size);
  }

  if( ioctl( *pfd, CAN_WRITE, &canmsg ) < 0 )
    ret = 1;

  return ret;
}

/***************************************************************************/
CAN_HANDLE canOpen_driver( s_BOARD *board )
{
  int *pfd;

  if( board == NULL )
  {
    MSG("can_copcican_linux: canOpen_driver(): board is NULL\n");
    return NULL;
  }

  if( board->busname == NULL )
  {
    MSG("can_copcican_linux: canOpen_driver(): busname is NULL\n");
    return NULL;
  }

  /* create dynamically to avoid global variable */
  pfd = (int*)malloc( sizeof(int) );

  if( pfd == NULL )
  {
    MSG("can_copcican_linux: canOpen_driver(): file descriptor is NULL\n");
    return NULL;
  }

  *pfd = open( board->busname, O_RDWR, 0 );

  if( *pfd < 0 )
  {
    MSG("can_copcican_linux: canOpen_driver(): invalid file descriptor\n");

    /* clear resources if open failed */
    free( pfd );
    pfd = NULL;
  }

  /*MSG("can_copcican_linux: canOpen_driver(): handle is %d\n", *pfd);*/

  return (CAN_HANDLE)pfd;
}

/***************************************************************************/
int canClose_driver( CAN_HANDLE fd0 )
{
  int *pfd = (int*)fd0;

  if( pfd == NULL )
  {
    MSG("can_copcican_linux: canClose_driver(): file descriptor is NULL\n");
    return -1;
  }

  if( *pfd < 0 )
  {
    MSG("can_copcican_linux: canClose_driver(): invalid file descriptor\n");
    return -1;
  }

  close( *pfd );
  free( pfd );
  pfd = NULL;

  selectedChannelRx = 0;
  selectedChannelTx = 0;

  return 0;
}

/***************************************************************************/
UNS8 canChangeBaudRate_driver( CAN_HANDLE fd0, char* baud )
{
  s_BOARD board;
  UNS8 ret = 0;
  int *pfd = (int*)fd0;

  if( pfd == NULL )
  {
    MSG("can_copcican_linux: canChangeBaudRate_driver(): file descriptor is NULL\n");
    return 1;
  }

  if( *pfd < 0 )
  {
    MSG("can_copcican_linux: canChangeBaudRate_driver(): invalid file descriptor\n");
    return 1;
  }

  if( baud == NULL )
  {
    MSG("can_copcican_linux: canChangeBaudRate_driver(): baud is NULL\n");
    return 1;
  }

  memset( &board, 0x00, sizeof(s_BOARD) );
  board.baudrate = baud;

  if( co_pcican_configure_selected_channel( *pfd, &board, RX ) < 0 )
    ret = 1;

  if( co_pcican_configure_selected_channel( *pfd, &board, TX ) < 0 )
    ret = 1;

  return ret;
}

