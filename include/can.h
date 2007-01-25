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

#ifndef __can_h__
#define __can_h__
/** Used for the Can message structure */
/*
union SHORT_CAN {
  struct { UNS8 b0,b1; } b;
  UNS32 w;
};
*/

typedef struct {
  UNS32 w; /* 32 bits */
} SHORT_CAN;


/** Can message structure */
typedef struct {
  SHORT_CAN cob_id;	/* l'ID du mesg */
  UNS8 rtr;			/* remote transmission request. 0 if not rtr, 
                   1 for a rtr message */
  UNS8 len;			/* message length (0 to 8) */
  UNS8 data[8]; /* data */
} Message;


typedef UNS8 (*canSend_t)(Message *);

#endif /* __can_h__ */
