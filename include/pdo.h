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

#ifndef __pdo_h__
#define __pdo_h__

#include <applicfg.h>
#include <def.h>

/* The process_var structure
 Used to store the PDO before the transmission or the reception.
*/
typedef struct struct_s_process_var {
  UNS8 count; /* Size of data. Ex : for a PDO of 6 bytes of data, count = 6 */
  /* WARNING s_process_var.data is subject to ENDIANISATION 
   * (with respect to CANOPEN_BIG_ENDIAN)
   */
  UNS8 data[PDO_MAX_LEN];
}s_process_var;

#include "data.h"

/** The PDO structure */
typedef struct struct_s_PDO {
  UNS32 cobId;	  /* COB-ID */
  UNS8           len;	  /* Number of data transmitted (in data[]) */
  UNS8           data[8]; /* Contain the data */
}s_PDO;

/** Transmit a PDO data frame on the bus bus_id
 * pdo is a structure which contains the pdo to transmit
 * bus_id is hardware dependant
 * return canSend(bus_id,&m) or 0xFF if error
 * request can take the value  REQUEST or NOT_A_REQUEST
 */
UNS8 sendPDO (CO_Data* d, s_PDO pdo, UNS8 request);

/** Prepare a PDO frame transmission, 
 * whose different parameters are stored in process_var table,
 * to the slave.
 * bus_id is hardware dependant
 * call the function sendPDO
 * return the result of the function sendPDO or 0xFF if error
 */
UNS8 PDOmGR (CO_Data* d, UNS32 cobId);

/** Prepare the PDO defined at index to be sent by  PDOmGR
 * Copy all the data to transmit in process_var
 * *pwCobId : returns the value of the cobid. (subindex 1)
 * Return 0 or 0xFF if error.
 */
UNS8 buildPDO (CO_Data* d, UNS16 index);

/** Transmit a PDO request frame on the bus bus_id
 * to the slave.
 * bus_id is hardware dependant
 * Returns 0xFF if error, other in success.
 */
UNS8 sendPDOrequest (CO_Data* d, UNS32 cobId);

/** Compute a PDO frame reception
 * bus_id is hardware dependant
 * return 0xFF if error, else return 0
 */
UNS8 proceedPDO (CO_Data* d, Message *m);

/* used by the application to send a variable by PDO.
 * Check in which PDO the variable is mapped, and send the PDO. 
 * of course, the others variables mapped in the PDO are also sent !
 * ( ie when a specific event occured)
 * bus_id is hardware dependant
 * variable is a pointer to the variable which has to be sent. Must be
 * defined in the object dictionary
 * return 0xFF if error, else return 0
 */
UNS8 sendPDOevent (CO_Data* d, void * variable);

#endif
