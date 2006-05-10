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

#include "objdict.h"

/**************************************************************************/
/* Declaration of the mapped variables                                    */
/**************************************************************************/

/**************************************************************************/
/* Declaration of the value range types                                   */
/**************************************************************************/



UNS32 gene_SYNC_valueRangeTest (UNS8 typeValue, UNS32 unsValue, REAL32 realValue)
{
  switch (typeValue) {
  }
  return 0;
}


/**************************************************************************/
/* The node id                                                            */
/**************************************************************************/
/* node_id default value. 
   This default value is deprecated.
   You should always overwrite this by using the function setNodeId(UNS8 nodeId) in your C code.
*/
#define NODE_ID 0x03
UNS8 gene_SYNC_bDeviceNodeId = NODE_ID;


//*****************************************************************************/
/* Array of message processing information */
/* Should not be modified */

const UNS8 gene_SYNC_iam_a_slave = 1;

  // Macros definition

/* Beware : 
index                 *must* be writen 4 numbers in hexa
sub_index             *must* be writen 2 numbers in hexa
size_variable_in_UNS8 *must* be writen 2 numbers in hexa
*/
#define PDO_MAP(index, sub_index, size_variable_in_bits)\
0x ## index ## sub_index ## size_variable_in_bits

/** This macro helps creating the object dictionary entries.
 *  by calling this macro
 *  it creates an entry in form of: 7 of entries, pointer to the entry. 
 */
#define DeclareIndexTableEntry(entryname, index)    { (subindex*)entryname,sizeof(entryname)/sizeof(entryname[0]), index}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//
//                       OBJECT DICTIONARY
//                   
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// Make your change, depending of your application
 

/* index 0x1000 :   Device type. 
                    You have to change the value below, so
                    it fits your canopen-slave-module */
 
                    /* Not used, so, should not be modified */
                    
                    UNS32 gene_SYNC_obj1000 = 0;
                    subindex gene_SYNC_Index1000[] =
                    {
                      { RO, uint32, sizeof(UNS32), (void*)&gene_SYNC_obj1000 }
                    };

/* index 0x1001 :   Error register. 
                    Change the entries to fit your application 
                    Not used, so, should not be modified */
                    UNS8 gene_SYNC_obj1001 = 0x00000000;
                    subindex gene_SYNC_Index1001[] =
                    {
                      { RO, uint8, sizeof(UNS8), (void*)&gene_SYNC_obj1001 }
                    };

/* index 0x1005 :   COB_ID SYNC */
                    /* Should not be modified */
                    UNS32 gene_SYNC_obj1005 = 0x40000080; // bit 30 = 1 : device can generate a SYNC message
                                                // Beware, it is over written when the node 
                                                // enters in reset mode
                                                // See initResetMode() in init.c
                    subindex gene_SYNC_Index1005[] =
                    {
                      { RW, uint32, sizeof(UNS32), (void*)&gene_SYNC_obj1005 }
                    };

/* index 0x1006 :   SYNC period */
                    // For producing the SYNC signal every n micro-seconds.
                    // Put 0 to not producing SYNC
                    UNS32 gene_SYNC_obj1006 = 0x000186A0; 
                                         // Default 0 to not produce SYNC //
                                         // Beware, it is over written when the 
                                         // node enters in reset mode.
                                         // See initResetMode() in init.c
                    subindex gene_SYNC_Index1006[] =
                    {
                      { RW, uint32, sizeof(UNS32), (void*)&gene_SYNC_obj1006 }
                    };


                    TIMER_HANDLE gene_SYNC_heartBeatTimers[0];
                    UNS32 gene_SYNC_obj1016[0]; 
                    UNS8 gene_SYNC_obj1016_cnt = 0;
                    subindex gene_SYNC_Index1016[0];

                    UNS16 gene_SYNC_obj1017 = 0x0000;
/* index 0x1018 :   Identity object */
                    /** index 1018: identify object. Adjust the entries for your node/company
                    */
                    /* Values can be modified */

                    s_identity gene_SYNC_obj1018 =
                    {
                      4,       // number of supported entries
                      0,  // Vendor-ID (given by the can-cia)
                      0,  // Product Code
                      0,  // Revision number
                      0  // serial number
                    };

                    subindex gene_SYNC_Index1018[] =
                    {
                      { RO, uint8,  sizeof(UNS8),  (void*)&gene_SYNC_obj1018.count },
                      { RO, uint32, sizeof(UNS32), (void*)&gene_SYNC_obj1018.vendor_id},
                      { RO, uint32, sizeof(UNS32), (void*)&gene_SYNC_obj1018.product_code},
                      { RO, uint32, sizeof(UNS32), (void*)&gene_SYNC_obj1018.revision_number},
                      { RO, uint32, sizeof(UNS32), (void*)&gene_SYNC_obj1018.serial_number}
                    };

/* index 0x1200 :   The SDO Server parameters */
                    /* BEWARE You cannot define more than one SDO server */
                    /* The values should not be modified here, 
                    but can be changed at runtime */
                    // Beware that the default values that you could put here
                    // will be over written at the initialisation of the node. 
                    // See setNodeId() in init.c
                    s_sdo_parameter gene_SYNC_obj1200  = 
                      { 3,                   // Number of entries. Always 3 for the SDO           
                        0x000,     // The cob_id transmited in CAN msg to the server     
                        0x000,     // The cob_id received in CAN msg from the server  
                        0x03      // The node id of the client. Should not be modified
                      };
                    subindex gene_SYNC_Index1200[] =
                    {
                      { RO, uint8,  sizeof( UNS8 ), (void*)&gene_SYNC_obj1200.count },
                      { RO, uint32, sizeof( UNS32), (void*)&gene_SYNC_obj1200.cob_id_client },
                      { RO, uint32, sizeof( UNS32), (void*)&gene_SYNC_obj1200.cob_id_server },
                      { RW, uint8,  sizeof( UNS8),  (void*)&gene_SYNC_obj1200.node_id }
                    };

/* index 0x1280 :   SDO client parameter */
                    s_sdo_parameter gene_SYNC_obj1280 = 
                      { 3,     // Nb of entries 
                        0x000, // cobid transmited to the server. The good value should be 0x600 + server nodeId
                        0x000, // cobid received from the server. The good value should be 0x580 + server nodeId
                        0x00  // server NodeId
                      };
                    subindex gene_SYNC_Index1280[] = 
                      { 
                        { RO, uint8, sizeof( UNS8  ), (void*)&gene_SYNC_obj1280.count },
                        { RW, uint8, sizeof( UNS32 ), (void*)&gene_SYNC_obj1280.cob_id_client },
                        { RW, uint8, sizeof( UNS32 ), (void*)&gene_SYNC_obj1280.cob_id_server },
                        { RW, uint8, sizeof( UNS8  ), (void*)&gene_SYNC_obj1280.node_id }
                      }; 


const indextable gene_SYNC_objdict[] = 
{
  DeclareIndexTableEntry(gene_SYNC_Index1000, 0x1000),
  DeclareIndexTableEntry(gene_SYNC_Index1001, 0x1001),
  DeclareIndexTableEntry(gene_SYNC_Index1005, 0x1005),
  DeclareIndexTableEntry(gene_SYNC_Index1006, 0x1006),
  DeclareIndexTableEntry(gene_SYNC_Index1018, 0x1018),
  DeclareIndexTableEntry(gene_SYNC_Index1200, 0x1200),
  DeclareIndexTableEntry(gene_SYNC_Index1280, 0x1280),
};

// To count at which received SYNC a PDO must be sent.
// Even if no pdoTransmit are defined, at least one entry is computed
// for compilations issues.
UNS8 gene_SYNC_count_sync[1] = {0, };

quick_index gene_SYNC_firstIndex = {
    SDO_SVR : 5,
    SDO_CLT : 6,
    PDO_RCV : 0,
    PDO_RCV_MAP : 0,
    PDO_TRS : 0,
    PDO_TRS_MAP : 0
};

quick_index gene_SYNC_lastIndex = {
    SDO_SVR : 5,
    SDO_CLT : 6,
    PDO_RCV : 0,
    PDO_RCV_MAP : 0,
    PDO_TRS : 0,
    PDO_TRS_MAP : 0
};

UNS16 gene_SYNC_ObjdictSize = sizeof(gene_SYNC_objdict)/sizeof(gene_SYNC_objdict[0]); 

CO_Data gene_SYNC_Data = CANOPEN_NODE_DATA_INITIALIZER(gene_SYNC);

