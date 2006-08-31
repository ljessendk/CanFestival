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
#include "af_can.h"
#define CAN_IFNAME     "can%s"
#define CAN_SOCKET     socket
#define CAN_CLOSE      close
#define CAN_RECV       recv
#define CAN_SEND       send
#define CAN_BIND       bind
#define CAN_IOCTL      ioctl
#endif

struct CANPort;
#define CAN_HANDLE struct CANPort *

#include <applicfg.h>

#include "timer.h"
#include "can_driver.h"
#include "timers_driver.h"

typedef struct CANPort {
       int fd;
       TASK_HANDLE receiveTask;
       CO_Data* d;
} CANPort;

/*********functions which permit to communicate with the board****************/
UNS8 canReceive(CAN_HANDLE fd0, Message *m)
{
       int res;
       struct can_frame frame;

       res = CAN_RECV(fd0->fd, &frame, sizeof(frame), 0);
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

void canReceiveLoop(CAN_HANDLE fd0)
{
       CO_Data* d = fd0->d;
       Message m;

       while (1) {
               if (canReceive(fd0, &m) != 0)
                       break;

               EnterMutex();
               canDispatch(d, &m);
               LeaveMutex();
       }
}

/***************************************************************************/
UNS8 canSend(CAN_HANDLE fd0, Message *m)
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

       res = CAN_SEND(fd0->fd, &frame, sizeof(frame), 0);
       if (res < 0)
               return 1;

       return 0;
}

/***************************************************************************/
CAN_HANDLE canOpen(s_BOARD *board)
{
       CAN_HANDLE fd0;
       struct ifreq ifr;
       struct sockaddr_can addr;
       int err;

       fd0 = malloc(sizeof(*fd0));
       if (!fd0)
               return NULL;

       fd0->fd = CAN_SOCKET(PF_CAN, SOCK_RAW, 0);
       if(fd0->fd < 0){
               fprintf(stderr,"Socket creation failed.\n");
               goto error_ret;
       }

       snprintf(ifr.ifr_name, IFNAMSIZ, CAN_IFNAME, board->busname);
       err = CAN_IOCTL(fd0->fd, SIOCGIFINDEX, &ifr);
       if (err) {
               fprintf(stderr, "Unknown device: %s\n", ifr.ifr_name);
               goto error_close;
       }

       addr.can_family  = AF_CAN;
       addr.can_ifindex = ifr.ifr_ifindex;
       err = CAN_BIND(fd0->fd, (struct sockaddr *)&addr,
                             sizeof(addr));
       if (err) {
               fprintf(stderr, "Binding failed.\n");
               goto error_close;
       }

       fd0->d = board->d;
       CreateReceiveTask(fd0, &fd0->receiveTask);
       return fd0;

 error_close:
       CAN_CLOSE(fd0->fd);

 error_ret:
       free(fd0);
       return NULL;
}

/***************************************************************************/
int canClose(CAN_HANDLE fd0)
{
       if (fd0) {
               WaitReceiveTaskEnd(&fd0->receiveTask);
               CAN_CLOSE(fd0->fd);
               free(fd0);
       }
       return 0;
}
