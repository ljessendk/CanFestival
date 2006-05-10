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

#ifndef __sdo_h__
#define __sdo_h__

typedef void (*SDOtimeoutError_t)(UNS8 line);
struct struct_s_transfer;

#include "timer.h"

/* The Transfer structure
Used to store the different segments of 
 - a SDO received before writing in the dictionary  
 - the reading of the dictionary to put on a SDO to transmit 
*/

struct struct_s_transfer {
  UNS8           nodeId;     //own ID if server, or node ID of the server if client
  
  UNS8           whoami;     // Takes the values SDO_CLIENT or SDO_SERVER
  UNS8           state;      // state of the transmission : Takes the values SDO_...
  UNS8           toggle;
  UNS32          abortCode;  // Sent or received
  // index and subindex of the dictionary where to store
  // (for a received SDO) or to read (for a transmit SDO)
  UNS16          index; 
  UNS8           subIndex; 
  UNS32          count;      // Number of data received or to be sent.
  UNS32          offset;     // stack pointer of data[]
                             // Used only to tranfer part of a line to or from a SDO.
                             // offset is always pointing on the next free cell of data[].
                             // WARNING s_transfer.data is subject to ENDIANISATION 
                             // (with respect to CANOPEN_BIG_ENDIAN)
  UNS8           data [SDO_MAX_LENGTH_TRANSFERT];
  UNS8           dataType;   // Defined in objdictdef.h Value is visible_string 
                             // if it is a string, any other value if it is not a string, 
                             // like 0. In fact, it is used only if client.
  TIMER_HANDLE   timer;    // Time counter to implement a timeout in milliseconds.
                             // It is automatically incremented whenever 
                             // the line state is in SDO_DOWNLOAD_IN_PROGRESS or 
                             // SDO_UPLOAD_IN_PROGRESS, and reseted to 0 
                             // when the response SDO have been received.
};
typedef struct struct_s_transfer s_transfer;
  

#include "data.h"

/// The 8 bytes data of the SDO
struct BODY{
    UNS8 data[8];
};

/// The SDO structure ...
struct struct_s_SDO {
  UNS8 nodeId;		//in any case, Node ID of the server (case sender or receiver).
  struct BODY body;
};


typedef struct struct_s_SDO s_SDO;

/** Reset all sdo buffers
 */
void resetSDO (CO_Data* d);


/** Copy the data received from the SDO line transfert to the object dictionary
 * Returns SDO error code if error. Else, returns 0; 
 */
UNS32 SDOlineToObjdict (CO_Data* d, UNS8 line);

/** Copy the data from the object dictionary to the SDO line for a network transfert.
 * Returns SDO error code if error. Else, returns 0; 
 */
UNS32 objdictToSDOline (CO_Data* d, UNS8 line);

/** copy data from an existant line in the argument "* data"
 * Returns 0xFF if error. Else, returns 0; 
 */
UNS8 lineToSDO (CO_Data* d, UNS8 line, UNS8 nbBytes, UNS8 * data);

/** Add data to an existant line
 * Returns 0xFF if error. Else, returns 0; 
 */
UNS8 SDOtoLine (CO_Data* d, UNS8 line, UNS8 nbBytes, UNS8 * data);

/** Called when an internal SDO abort occurs.
 * Release the line * Only if server * 
 * If client, the line must be released manually in the core application.
 * The reason of that is to permit the program to read the transfers[][] structure before its reset,
 * because many informations are stored on it : index, subindex, data received or trasmited, ...
 * In all cases, sends a SDO abort.
 * Returns 0
 */
UNS8 failedSDO (CO_Data* d, UNS8 nodeId, UNS8 whoami, UNS16 index, 
		UNS8 subIndex, UNS32 abortCode);

/** Reset an unused line.
 * 
 */
void resetSDOline (CO_Data* d, UNS8 line);

/** Initialize some fields of the structure.
 * Returns 0
 */
UNS8 initSDOline (CO_Data* d, UNS8 line, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 state);

/** Search for an unused line in the transfers array
 * to store a new SDO.
 * ie a line which value of the field "state" is "SDO_RESET"
 * An unused line have the field "state" at the value SDO_RESET
 * bus_id is hardware dependant
 * whoami : create the line for a SDO_SERVER or SDO_CLIENT.
 * return 0xFF if all the lines are on use. Else, return 0
 */
UNS8 getSDOfreeLine (CO_Data* d, UNS8 whoami, UNS8 *line);

/** Search for the line, in the transfers array, which contains the
 * beginning of the reception of a fragmented SDO
 * whoami takes 2 values : look for a line opened as SDO_CLIENT or SDO_SERVER
 * bus_id is hardware dependant
 * nodeId correspond to the message node-id 
 * return 0xFF if error.  Else, return 0
 */
UNS8 getSDOlineOnUse (CO_Data* d, UNS8 nodeId, UNS8 whoami, UNS8 *line);

/** Close a transmission.
 * nodeId : Node id of the server if both server or client
 * whoami : Line opened as SDO_CLIENT or SDO_SERVER
 */
UNS8 closeSDOtransfer (CO_Data* d, UNS8 nodeId, UNS8 whoami);

/** Bytes in the line structure which must be transmited (or received)
 * bus_id is hardware dependant.
 * return 0.
 */
UNS8 getSDOlineRestBytes (CO_Data* d, UNS8 line, UNS8 * nbBytes);

/** Store in the line structure the nb of bytes which must be transmited (or received)
 * bus_id is hardware dependant.
 * return 0 if success, 0xFF if error.
 */
UNS8 setSDOlineRestBytes (CO_Data* d, UNS8 line, UNS8 nbBytes);

/** Transmit a SDO frame on the bus bus_id
 * sdo is a structure which contains the sdo to transmit
 * bus_id is hardware dependant
 * whoami takes 2 values : SDO_CLIENT or SDO_SERVER
 * return canSend(bus_id,&m) or 0xFF if error
 */
UNS8 sendSDO (CO_Data* d, UNS8 whoami, s_SDO sdo);

/** Transmit a SDO error to the client. The reasons may be :
 * Read/Write to a undefined object
 * Read/Write to a undefined subindex
 * Read/write a not valid length object
 * Write a read only object
 * whoami takes 2 values : SDO_CLIENT or SDO_SERVER
 */
UNS8 sendSDOabort (CO_Data* d, UNS8 whoami, UNS16 index, UNS8 subIndex, UNS32 abortCode);

/** Treat a SDO frame reception
 * bus_id is hardware dependant
 * call the function sendSDO
 * return 0xFF if error
 *        0x80 if transfert aborted by the server
 *        0x0  ok
 */
UNS8 proceedSDO (CO_Data* d, Message *m);

/** Used by the application to send a SDO request frame to write the data *data
 * at the index and subIndex indicated
 * in the dictionary of the slave whose node_id is nodeId
 * Count : nb of bytes to write in the dictionnary.
 * datatype (defined in objdictdef.h) : put "visible_string" for strings, 0 for integers or reals or other value.
 * bus_id is hardware dependant
 * return 0xFF if error, else return 0
 */
UNS8 writeNetworkDict (CO_Data* d, UNS8 nodeId, UNS16 index, 
		       UNS8 subIndex, UNS8 count, UNS8 dataType, void *data); 

/** Used by the application to send a SDO request frame to read
 * in the dictionary of a server node whose node_id is ID
 * at the index and subIndex indicated
 * bus_id is hardware dependant
 * datatype (defined in objdictdef.h) : put "visible_string" for strings, 0 for integers or reals or other value.
 * return 0xFF if error, else return 0
 */
UNS8 readNetworkDict (CO_Data* d, UNS8 nodeId, UNS16 index, 
		      UNS8 subIndex, UNS8 dataType);

/** Use this function after a readNetworkDict to get the result.
  Returns : SDO_FINISHED             // data is available
            SDO_ABORTED_RCV          // Transfert failed. (abort SDO received)
            SDO_ABORTED_INTERNAL     // Transfert failed. Internal abort.
            SDO_UPLOAD_IN_PROGRESS   // Data not yet available
	    SDO_DOWNLOAD_IN_PROGRESS // Should not arrive ! 

  dataType (defined in objdictdef.h) : type expected. put "visible_string" for strings, 0 for integers or reals.
  abortCode : 0 = not available. Else : SDO abort code. (received if return SDO_ABORTED_RCV)
  example :
  UNS32 data;
  UNS8 size;
  readNetworkDict(0, 0x05, 0x1016, 1, 0) // get the data index 1016 subindex 1 of node 5
  while (getReadResultNetworkDict (0, 0x05, &data, &size) != SDO_UPLOAD_IN_PROGRESS);
*/
UNS8 getReadResultNetworkDict (CO_Data* d, UNS8 nodeId, void* data, 
			       UNS8 *size, UNS32 * abortCode);

/**
  Use this function after a writeNetworkDict to get the result of the write
  It is mandatory to call this function because it is releasing the line used for the transfer.
  Returns : SDO_FINISHED             // data is available
            SDO_ABORTED_RCV          // Transfert failed. (abort SDO received)
            SDO_ABORTED_INTERNAL     // Transfert failed. Internal abort.
            SDO_DOWNLOAD_IN_PROGRESS // Data not yet available
	    SDO_UPLOAD_IN_PROGRESS   // Should not arrive ! 
  abortCode : 0 = not available. Else : SDO abort code. (received if return SDO_ABORTED_RCV)
  example :
  UNS32 data = 0x50;
  UNS8 size;
  UNS32 abortCode;
  writeNetworkDict(0, 0x05, 0x1016, 1, size, &data) // write the data index 1016 subindex 1 of node 5
  while ( getWriteResultNetworkDict (0, 0x05, &abortCode) != SDO_DOWNLOAD_IN_PROGRESS);  
*/
UNS8 getWriteResultNetworkDict (CO_Data* d, UNS8 nodeId, UNS32 * abortCode);

#endif
