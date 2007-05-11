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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h> /* for NULL */

#include "config.h"

#ifdef RTCAN_SOCKET
#include "rtdm/rtcan.h"
#define CAN_IFNAME     "rtcan%s"
#define CAN_SOCKET     rt_dev_socket
#define CAN_CLOSE      rt_dev_close
#define CAN_RECV       rt_dev_recv
#define CAN_SEND       rt_dev_send
#define CAN_BIND       rt_dev_bind
#define CAN_IOCTL      rt_dev_ioctl
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include "linux/can.h"
#include "linux/can/raw.h"
#include "net/if.h"
#define PF_CAN 29
#define AF_CAN PF_CAN
//#include "af_can.h"
#define CAN_IFNAME     "can%s"
#define CAN_SOCKET     socket
#define CAN_CLOSE      close
#define CAN_RECV       recv
#define CAN_SEND       send
#define CAN_BIND       bind
#define CAN_IOCTL      ioctl
#endif

#include "can_driver.h"

/*********functions which permit to communicate with the board****************/
UNS8 canReceive_driver(CAN_HANDLE fd0, Message *m)
{
       int res;
       struct can_frame frame;

       res = CAN_RECV(*(int*)fd0, &frame, sizeof(frame), 0);
       if (res < 0)
               return 1;

       m->cob_id.w = frame.can_id & CAN_EFF_MASK;
       m->len      = frame.can_dlc;
       if (frame.can_id & CAN_RTR_FLAG)
               m->rtr = 1;
       else
               m->rtr = 0;
       memcpy(m->data, frame.data, 8);

       return 0;
}


/***************************************************************************/
UNS8 canSend_driver(CAN_HANDLE fd0, Message *m)
{
       int res;
       struct can_frame frame;

       frame.can_id = m->cob_id.w;
       if (frame.can_id >= 0x800)
               frame.can_id |= CAN_EFF_FLAG;
       frame.can_dlc = m->len;
       if (m->rtr)
               frame.can_id |= CAN_RTR_FLAG;
       else
               memcpy(frame.data, m->data, 8);

       res = CAN_SEND(*(int*)fd0, &frame, sizeof(frame), 0);
       if (res < 0)
               return 1;

       return 0;
}

/***************************************************************************/
CAN_HANDLE canOpen_driver(s_BOARD *board)
{
       struct ifreq ifr;
       struct sockaddr_can addr;
       int err;
       CAN_HANDLE fd0 = malloc(sizeof(int));

       *(int*)fd0 = CAN_SOCKET(PF_CAN, SOCK_RAW, CAN_RAW);
       if(*(int*)fd0 < 0){
               fprintf(stderr,"Socket creation failed.\n");
               goto error_ret;
       }

       snprintf(ifr.ifr_name, IFNAMSIZ, CAN_IFNAME, board->busname);
       err = CAN_IOCTL(*(int*)fd0, SIOCGIFINDEX, &ifr);
       if (err) {
               fprintf(stderr, "Unknown device: %s\n", ifr.ifr_name);
               goto error_close;
       }

       addr.can_family  = AF_CAN;
       addr.can_ifindex = ifr.ifr_ifindex;
       err = CAN_BIND(*(int*)fd0, (struct sockaddr *)&addr,
                             sizeof(addr));
       if (err) {
               fprintf(stderr, "Binding failed.\n");
               goto error_close;
       }

       return fd0;

 error_close:
       CAN_CLOSE(*(int*)fd0);

 error_ret:
       free(fd0);
       return NULL;
}

/***************************************************************************/
int canClose_driver(CAN_HANDLE fd0)
{
       if (fd0) {
               CAN_CLOSE(*(int*)fd0);
               free(fd0);
       }
       return 0;
}
