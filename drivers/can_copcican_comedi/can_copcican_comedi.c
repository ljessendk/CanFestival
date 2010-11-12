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

#include <linux/module.h>
#include <linux/comedi.h>
#include <linux/comedilib.h>

#define NEED_PRINT_MESSAGE
#include "can_driver.h"
#include "def.h"
#include "co_pcicanops.h"

MODULE_LICENSE("GPL");

/* at the moment not threadsafe :-( */
static unsigned char selectedChannelRx = 0, selectedChannelTx = 0;

static int rt_strcmp( const char *src, const char *dst )
{
  int ret = 0;

  while( ! (ret = *(unsigned char *)src - *(unsigned char *)dst) && *dst)
  {
    src++;
    dst++;
  }

  if( ret < 0 )
  {
    ret = -1 ;
  }
  else
  {
    if( ret > 0 )
      ret = 1 ;
  }

  return ret;
}

static int TranslateBaudRate( char* optarg )
{
  /* values see documentation of CO-PCICAN */
  if( !rt_strcmp( optarg, "1M"    ) ) return 0;
  if( !rt_strcmp( optarg, "800K"  ) ) return 1;
  if( !rt_strcmp( optarg, "500K"  ) ) return 2;
  if( !rt_strcmp( optarg, "250K"  ) ) return 3;
  if( !rt_strcmp( optarg, "125K"  ) ) return 4;
  if( !rt_strcmp( optarg, "100K"  ) ) return 5;
  if( !rt_strcmp( optarg, "83.3K" ) ) return 6;
  if( !rt_strcmp( optarg, "10K"   ) ) return 7;

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

comedi_t* get_dev_of_port( CAN_PORT port )
{
  CANPort *thisPort = (CANPort*)port;
  CAN_HANDLE thisHandle;
  comedi_t *dev;

  if( thisPort == NULL )
  {
    MSG("can_copcican_comedi: get_dev_of_port(): thisPort is NULL\n");
    return NULL;
  }

  thisHandle = thisPort->fd;

  if( thisHandle == NULL )
  {
    MSG("can_copcican_comedi: get_dev_of_port(): thisHandle is NULL\n");
    return NULL;
  }

  dev = (comedi_t*)thisHandle;

  /*MSG("can_copcican_comedi: get_dev_of_port(): handle is 0x%08x\n", (unsigned int)dev);*/

  return dev;
}

int co_pcican_enter_run_mode( comedi_t *dev )
{
  unsigned int opcmd[15];
  unsigned int opcode = CMDQ_OPC_ENTER_RUN_MODE;
  comedi_insn insn;

  if( dev == NULL )
  {
    MSG("can_copcican_comedi: co_pcican_enter_run_mode(): dev is NULL\n");
    return 1;
  }

  memset( opcmd, 0x00, sizeof(opcmd) );
  opcmd[0] = 0;

  memset( &insn, 0x00, sizeof(insn) );
  insn.insn      = INSN_CONFIG;
  insn.n         = 1;
  insn.data      = (void*)opcmd;
  insn.subdev    = 0;
  insn.unused[0] = opcode; /* in use for CO-PCICAN */

  comedi_do_insn( dev, &insn );

  opcmd[0] = 2;
  insn.subdev    = 2;
  insn.unused[0] = opcode; /* in use for CO-PCICAN */

  return comedi_do_insn( dev, &insn );
}

int co_pcican_enter_config_mode( comedi_t *dev )
{
  unsigned int opcmd[15];
  unsigned int opcode = CMDQ_OPC_ENTER_CONFIG_MODE;
  comedi_insn insn;

  if( dev == NULL )
  {
    MSG("can_copcican_comedi: co_pcican_enter_config_mode(): dev is NULL\n");
    return 1;
  }

  memset( opcmd, 0x00, sizeof(opcmd) );
  opcmd[0] = 0;

  memset( &insn, 0x00, sizeof(insn) );
  insn.insn      = INSN_CONFIG;
  insn.n         = 1;
  insn.data      = (void*)opcmd;
  insn.subdev    = 0;
  insn.unused[0] = opcode; /* in use for CO-PCICAN */

  comedi_do_insn( dev, &insn );

  opcmd[0] = 2;
  insn.subdev    = 2;
  insn.unused[0] = opcode; /* in use for CO-PCICAN */

  return comedi_do_insn( dev, &insn );
}

int co_pcican_select_channel( const unsigned char channel, const unsigned int direction )
{
  if( channel >= NUM_CAN_CHANNELS )
  {
    MSG("can_copcican_comedi: co_pcican_select_channel(): invalid channel\n");
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

int co_pcican_configure_selected_channel( comedi_t *dev, s_BOARD *board, const unsigned int direction )
{
  unsigned int opcmd[15];
  unsigned int opcode = CMDQ_OPC_SET_CONFIG_CHANNEL;
  unsigned int selectedChannel;
  comedi_insn insn;

  if( dev == NULL )
  {
    MSG("can_copcican_comedi: co_pcican_configure_selected_channel(): dev is NULL\n");
    return -1;
  }

  if( board == NULL )
  {
    MSG("can_copcican_comedi: co_pcican_configure_selected_channel(): board is NULL\n");
    return -1;
  }

  if( board->baudrate == NULL )
  {
    MSG("can_copcican_comedi: co_pcican_configure_selected_channel(): baudrate is NULL\n");
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
    MSG("can_copcican_comedi: co_pcican_configure_selected_channel(): invalid channel selected\n");
    return -1;
  }

  memset( opcmd, 0x00, sizeof(opcmd) );
  opcmd[0] = selectedChannel;
  opcmd[1] = TranslateBaudRate( board->baudrate );

  memset( &insn, 0x00, sizeof(insn) );
  insn.insn      = INSN_CONFIG;
  insn.n         = 1;
  insn.data      = (void*)opcmd;

  if( selectedChannel < 2 )
    insn.subdev    = 0;
  else
    insn.subdev    = 2;

  insn.unused[0] = opcode; /* in use for CO-PCICAN */

  return comedi_do_insn( dev, &insn );
}

/*********functions which permit to communicate with the board****************/
UNS8 canReceive_driver( CAN_HANDLE fd0, Message *m )
{
  MESSAGE_T canmsg;
  comedi_insn insn;
  UNS8 ret = 0;
  comedi_t *dev = (comedi_t*)fd0;

  if( dev == NULL )
  {
    MSG("can_copcican_comedi: canReceive_driver(): dev is NULL\n");
    return 1;
  }

  if( selectedChannelRx >= NUM_CAN_CHANNELS )
  {
    MSG("can_copcican_comedi: canReceive_driver(): invalid channel selected\n");
    return 1;
  }

  if( m == NULL )
  {
    MSG("can_copcican_comedi: canReceive_driver(): message is NULL\n");
    return 1;
  }

  memset( &canmsg, 0x00, sizeof(MESSAGE_T) );

  memset( &insn, 0x00, sizeof(insn) );
  insn.insn      = INSN_READ;
  insn.n         = 1;
  insn.data      = (void*)&canmsg;

  if( selectedChannelRx < 2 )
    insn.subdev    = comedi_find_subdevice_by_type(dev, COMEDI_SUBD_DI, 0);
  else
    insn.subdev    = comedi_find_subdevice_by_type(dev, COMEDI_SUBD_DI, 2);

  insn.unused[1] = selectedChannelRx; /* in use for CO-PCICAN */
  insn.unused[2] = sizeof(MESSAGE_T); /* in use for CO-PCICAN */

  comedi_do_insn( dev, &insn );

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
  MESSAGE_T canmsg;
  comedi_insn insn;
  UNS8 ret = 0;
  comedi_t *dev = (comedi_t*)fd0;

  if( dev == NULL )
  {
    MSG("can_copcican_comedi: canSend_driver(): dev is NULL\n");
    return 1;
  }

  if( selectedChannelTx >= NUM_CAN_CHANNELS )
  {
    MSG("can_copcican_comedi: canSend_driver(): invalid channel selected\n");
    return 1;
  }

  if( m == NULL )
  {
    MSG("can_copcican_comedi: canSend_driver(): message is NULL\n");
    return 1;
  }

  memset( &canmsg, 0x00, sizeof(MESSAGE_T) );
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

  memset( &insn, 0x00, sizeof(insn) );
  insn.insn      = INSN_WRITE;
  insn.n         = 1;
  insn.data      = (void*)&canmsg;

  if( selectedChannelTx < 2 )
    insn.subdev    = comedi_find_subdevice_by_type(dev, COMEDI_SUBD_DO, 0);
  else
    insn.subdev    = comedi_find_subdevice_by_type(dev, COMEDI_SUBD_DO, 2);

  insn.unused[1] = selectedChannelTx; /* in use for CO-PCICAN */
  insn.unused[2] = sizeof(MESSAGE_T) - sizeof(unsigned long); /* in use for CO-PCICAN, no timestamp for tx */

  if( comedi_do_insn( dev, &insn ) < 0 )
    ret = 1;

  return ret;
}

/***************************************************************************/
CAN_HANDLE canOpen_driver( s_BOARD *board )
{
  comedi_t *dev;

  if( board == NULL )
  {
    MSG("can_copcican_comedi: canOpen_driver(): board is NULL\n");
    return NULL;
  }

  if( board->busname == NULL )
  {
    MSG("can_copcican_comedi: canOpen_driver(): busname is NULL\n");
    return NULL;
  }

  dev = comedi_open( board->busname );

  /*MSG("can_copcican_comedi: canOpen_driver(): handle is 0x%08x\n", (unsigned int)dev);*/

  return dev;
}

/***************************************************************************/
int canClose_driver( CAN_HANDLE fd0 )
{
  comedi_t *dev = (comedi_t*)fd0;

  if( dev == NULL )
  {
    MSG("can_copcican_comedi: canClose_driver(): dev is NULL\n");
    return 1;
  }

  comedi_close( dev );

  selectedChannelRx = 0;
  selectedChannelTx = 0;

  return 0;
}

/***************************************************************************/
UNS8 canChangeBaudRate_driver( CAN_HANDLE fd0, char* baud )
{
  s_BOARD board;
  UNS8 ret = 0;
  comedi_t *dev = (comedi_t*)fd0;

  if( dev == NULL )
  {
    MSG("can_copcican_comedi: canChangeBaudRate_driver(): dev is NULL\n");
    return 1;
  }

  if( baud == NULL )
  {
    MSG("can_copcican_comedi: canChangeBaudRate_driver(): baud is NULL\n");
    return 1;
  }

  memset( &board, 0x00, sizeof(s_BOARD) );
  board.baudrate = baud;

  if( co_pcican_configure_selected_channel( dev, &board, RX ) < 0 )
    ret = 1;

  if( co_pcican_configure_selected_channel( dev, &board, TX ) < 0 )
    ret = 1;

  return ret;
}

static int init(void)
{
  printk("can_copcican_comedi for CanFestival loaded\n");

  return 0;
}

static void exit(void)
{
  printk("can_copcican_comedi unloaded\n");
}

module_init(init);
module_exit(exit);

EXPORT_SYMBOL(get_dev_of_port);
EXPORT_SYMBOL(co_pcican_enter_run_mode);
EXPORT_SYMBOL(co_pcican_enter_config_mode);
EXPORT_SYMBOL(co_pcican_select_channel);
EXPORT_SYMBOL(co_pcican_configure_selected_channel);
EXPORT_SYMBOL(canOpen_driver);
EXPORT_SYMBOL(canClose_driver);
EXPORT_SYMBOL(canSend_driver);
EXPORT_SYMBOL(canReceive_driver);
EXPORT_SYMBOL(canChangeBaudRate_driver);
