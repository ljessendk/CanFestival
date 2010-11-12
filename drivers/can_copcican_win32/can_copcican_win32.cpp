// can_copcican_win32.cpp : Defines the exported functions for the DLL application.
//


#include "stdafx.h"

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
//#include <unistd.h>
#include <fcntl.h>

//#define NEED_PRINT_MESSAGE
static int initdone = 0;

inline void * maskDevID(int devId){	return ((void*)( (devId & 0x0000000F) | 0xC05A0000)); }
inline int unmaskDevID(void * Ptr){	return ((int)((int)Ptr & 0x0000000F)); }

/* at the moment not threadsafe :-( */
static unsigned char selectedChannelRx = 1, selectedChannelTx = 1;

static cosateq::devices::can::co_baudrate_t TranslateBaudRate( char* optarg )
{
  /* values see documentation of CO-PCICAN */
	if( !strcmp( optarg, "1M"    ) ) return cosateq::devices::can::CO_BAUD_1M;
	if( !strcmp( optarg, "800K"  ) ) return cosateq::devices::can::CO_BAUD_800k;
	if( !strcmp( optarg, "500K"  ) ) return cosateq::devices::can::CO_BAUD_500k;
  if( !strcmp( optarg, "250K"  ) ) return cosateq::devices::can::CO_BAUD_250k;
  if( !strcmp( optarg, "125K"  ) ) return cosateq::devices::can::CO_BAUD_125k;
  if( !strcmp( optarg, "100K"  ) ) return cosateq::devices::can::CO_BAUD_100k;
  if( !strcmp( optarg, "83.3K" ) ) return cosateq::devices::can::CO_BAUD_83k;
  if( !strcmp( optarg, "10K"   ) ) return cosateq::devices::can::CO_BAUD_10k;

  return (cosateq::devices::can::co_baudrate_t)-1;
}

int co_pcican_select_channel( const unsigned char channel, const unsigned int direction )
{
  if( channel >= CHANNELS )
  {
    MSG("can_copcican_linux: co_pcican_select_channel(): invalid channel\n");
    return -1;
  }

  /* at the moment not threadsafe :-( */
  switch( direction )
  {
    case cosateq::devices::can::CO_CAN_RX: selectedChannelRx = channel;
             break;
    case cosateq::devices::can::CO_CAN_TX: selectedChannelTx = channel;
             break;
    default: return -1;
  }

  return 0;
}

int co_pcican_configure_selected_channel( const int fd, s_BOARD *board, const unsigned int direction )
{
  unsigned int selectedChannel;
  int ret = 0;

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
    case cosateq::devices::can::CO_CAN_RX: selectedChannel = selectedChannelRx;
             break;
    case cosateq::devices::can::CO_CAN_TX: selectedChannel = selectedChannelTx;
             break;
    default: selectedChannel = 0xff;
  }

  if( selectedChannel >= CHANNELS )
  {
    MSG("can_copcican_linux: co_pcican_configure_selected_channel(): invalid channel selected\n");
    return -1;
  }

  ret = can_setChannelConfig( TranslateBaudRate( board->baudrate ),0x0,unmaskDevID((void*)fd),selectedChannel);
  if ( ret != 0 ) {
	  can_enterRunMode(unmaskDevID((void*)fd),selectedChannel/2);
	  return -ret;
  }else{
	  ret = can_setIrqMode(unmaskDevID((void*)fd),selectedChannel);
	  can_enterRunMode(unmaskDevID((void*)fd),selectedChannel/2);
  }
  return ret;
}

/*********functions which permit to communicate with the board****************/

UNS8 __stdcall canReceive_driver( CAN_HANDLE fd0, Message *m )
{
  cosateq::devices::can::co_can_packet_t p;
  int ret = 0;

  if ( !m )
	  return 0;

  memset(&p,0,sizeof(cosateq::devices::can::co_can_packet_t));
  ret = can_receive(&p,unmaskDevID(fd0),selectedChannelRx);
  //printf("rec: %d\n",ret);
  if ( ret != 0 )
	  return 1;

  //printf("rec2: %d\n",ret);
  if ( p.size > 8 )
	  p.size = 8;

  if ( !(p.type & MSG_RTR ) )
    memcpy(m->data,p.data,p.size);

  m->cob_id = p.id;
  m->len = p.size;
  m->rtr = p.type & MSG_RTR;
  
  return 0;
}

/***************************************************************************/

UNS8 __stdcall canSend_driver( CAN_HANDLE fd0, Message *m )
{
  cosateq::devices::can::co_can_packet_t p;
  
  if ( !m )
	  return 0;
  memset(&p,0,sizeof(cosateq::devices::can::co_can_packet_t));
  p.id = m->cob_id;
  p.size = (m->len > 8) ? 8 : m->len;
  p.type = ( m->rtr ) ? 0x2:0x0;
  if ( p.id > 0x800 )
	  p.type |= MSG_EXT;
  memcpy(p.data,m->data,p.size);

  return can_send(&p,unmaskDevID(fd0),selectedChannelTx);
}



/***************************************************************************/

UNS8 __stdcall canChangeBaudRate_driver( CAN_HANDLE fd0, char* baud )
{
  UNS8 ret = 0;

  ret = can_enterCfgMode(unmaskDevID(fd0),0);
  if ( ret != 0)
    MSG("Enter config mode for Channelpair 0 failed in Function %s! Error: %d\n",__FUNCTION__,ret);
  ret = can_enterCfgMode(unmaskDevID(fd0),1);
  if ( ret != 0)
    MSG("Enter config mode for Channelpair 1 failed in Function %s! Error: %d\n",__FUNCTION__,ret);

  for ( int i = 0; i < 4; i++ ) {
      ret = can_setChannelConfig(TranslateBaudRate(baud),0x0,unmaskDevID(fd0),i);
      if ( ret != 0)
        MSG("Set config for channel %d failed in Function %s! Error: %d\n",i,__FUNCTION__,ret);
	  can_setIrqMode(unmaskDevID(fd0),i);
  }
  ret = can_enterRunMode(unmaskDevID(fd0),0);
  if ( ret != 0)
    MSG("Enter run mode for Channelpair 1 failed in Function %s! Error: %d\n",__FUNCTION__,ret);

  ret = can_enterRunMode(unmaskDevID(fd0),1);
  if ( ret != 0)
    MSG("Enter run mode for Channelpair 1 failed in Function %s! Error: %d\n",__FUNCTION__,ret);

  return ret;
}

CAN_HANDLE __stdcall canOpen_driver( s_BOARD *board )
{
	int ret = 0;
	int devId = 0;
	if ( !board )
		return NULL;
	if ( !board->busname )
		return NULL;
	//TODO find out how boardname resolves

	//printf("BOARD: 0x%x %s\n",board,board->busname);
	//sscanf_s(board->busname,"%d",&devId);

	if ( devId < 0 || devId > 15 )
		return NULL;

	//return (can_HANDLE)NULL;
	if ( !initdone ) {
		can_init();
		initdone = 1;
	}

	ret = can_open(devId);
	if ( ret < 0 )
		return (CAN_HANDLE)NULL;
	
	canChangeBaudRate_driver( maskDevID(devId),board->baudrate );
	
	return maskDevID(devId);
}

int __stdcall canClose_driver(CAN_HANDLE fd0 )
{
	return can_close(unmaskDevID(fd0));
}

