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


#include <stddef.h>

#include <canfestival/applicfg.h>
#include "canfestival/def.h"
#include "canfestival/can.h"
#include "canfestival/objdictdef.h"
#include "canfestival/pdo.h"
#include "canfestival/sdo.h"
#include "canfestival/sync.h"
#include "canfestival/lifegrd.h"
#include "canfestival/nmtSlave.h"

/**************************************************************************/
/* Declaration of the mapped variables                                    */
/**************************************************************************/
UNS8 seconds = 0;		// Mapped at index 0x2000, subindex 0x01
UNS8 minutes = 0;		// Mapped at index 0x2000, subindex 0x02
UNS8 hours = 0;		// Mapped at index 0x2000, subindex 0x03
UNS8 day = 0;		// Mapped at index 0x2000, subindex 0x04
UNS32 canopenErrNB = 0;		// Mapped at index 0x6000, subindex 0x00
UNS32 canopenErrVAL = 0;		// Mapped at index 0x6001, subindex 0x00
UNS8 strTest[10] = 0;		// Mapped at index 0x6002, subindex 0x00

/**************************************************************************/
/* Declaration of the value range types                                   */
/**************************************************************************/



UNS32 Linux_slave_valueRangeTest (UNS8 typeValue, UNS32 unsValue, REAL32 realValue)
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
#define NODE_ID 0x01
UNS8 Linux_slave_bDeviceNodeId = NODE_ID;


//*****************************************************************************/
/* Array of message processing information */
/* Should not be modified */

const UNS8 Linux_slave_iam_a_slave = 1

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
                    
                    UNS32 Linux_slave_obj1000 = 0;
                    subindex Linux_slave_Index1000[] =
                    {
                      { RO, uint32, sizeof(UNS32), (void*)&Linux_slave_obj1000 }
                    };

/* index 0x1001 :   Error register. 
                    Change the entries to fit your application 
                    Not used, so, should not be modified */
                    /*const*/ UNS8 Linux_slave_obj1001 = 0x0;
                    /*const*/ subindex Linux_slave_Index1001[] =
                    {
                      { RO, uint8, sizeof(UNS8), (void*)&Linux_slave_obj1001 }
                    };

/* index 0x1005 :   COB_ID SYNC */
                    /* Should not be modified */
                    UNS32 Linux_slave_obj1005 = 0x00000080; // bit 30 = 1 : device can generate a SYNC message
                                                // Beware, it is over written when the node 
                                                // enters in reset mode
                                                // See initResetMode() in init.c
                    /*const*/ subindex Linux_slave_Index1005[] =
                    {
                      { RW, uint32, sizeof(UNS32), (void*)&Linux_slave_obj1005 }
                    };

/* index 0x1006 :   SYNC period */
                    // For producing the SYNC signal every n micro-seconds.
                    // Put 0 to not producing SYNC
                    /*const*/ UNS32 Linux_slave_obj1006 = 0x0; 
                                         // Default 0 to not produce SYNC //
                                         // Beware, it is over written when the 
                                         // node enters in reset mode.
                                         // See initResetMode() in init.c
                    /*const*/ subindex Linux_slave_Index1006[] =
                    {
                      { RW, uint32, sizeof(UNS32), (void*)&Linux_slave_obj1006 }
                    };

/* index 0x1007 :   Synchronous Window Length
                    Seems to be needed by DS401 to generate the SYNC signal ! */
                    /*const*/ UNS32 Linux_slave_obj1007 = 0x0; /* Default 0 */
                    /*const*/ subindex Linux_slave_Index1007[] =
                    {
                      { RW, uint32, sizeof(UNS32), (void*)&Linux_slave_obj1007 }
                    };

/* index 0x1008 :   Manufacturer device name */
                    UNS8 Linux_slave_obj1008[] = "Appli_Slave_HC12"; /* Default 0 */
                    subindex Linux_slave_Index1008[] =
                    {
                      { RO, uint32, sizeof(Linux_slave_obj1008), (void*)&Linux_slave_obj1008 }
                    };

/* index 0x1009 :   Manufacturer hardware version */
                    UNS8 Linux_slave_obj1009[] = "__DATE__"; /* Default 0 */
                    subindex Linux_slave_Index1009[] =
                    {
                      { RO, uint32, sizeof(Linux_slave_obj1009), (void*)&Linux_slave_obj1009 }
                    };

/* index 0x100A :   Manufacturer software version */
                    UNS8 Linux_slave_obj100A[] = __TIME__; /* Default 0 */
                    subindex Linux_slave_Index100A[] =
                    {
                      { RO, uint32, Linux_slave_sizeof(obj100A), (void*)&Linux_slave_obj100A}
                    };


                    TIMER_HANDLE Linux_slave_heartBeatTimers[1] = {TIMER_NONE,};
/* index 0x1016 :   HeartBeat consumers 
                    The nodes which can send a heartbeat */ 
                    UNS32 Linux_slave_obj1016[] = {// Consumer time for each node 
                    0x00000000}; // Format 0x00NNTTTT (N=Node T=time in ms)

                    UNS8 Linux_slave_obj1016_cnt = 1; // 1 nodes could send me
                                                  // their heartbeat.
                    subindex Linux_slave_Index1016[] = 
                    {
                      { RO, uint8, sizeof(UNS8), (void*)&Linux_slave_obj1016_cnt },
                      { RW, uint32, sizeof(UNS32), (void*)&Linux_slave_obj1016[0] }
                    }; 

/* index 0x1017 :   Heartbeat producer                    
                    Every HBProducerTime, the node sends its heartbeat */
                    UNS16 Linux_slave_obj1017 = 0; //HBProducerTime in ms. If 0 : not activated 
                                                     // Beware, it is over written when the 
                                                     // node enters in reset mode.
                                                     // See initResetMode() in init.c
                    subindex Linux_slave_Index1017[] =
                    {
	              { RW, uint16, sizeof(UNS16), &Linux_slave_obj1017 }
                    };

/* index 0x1018 :   Identity object */
                    /** index 1018: identify object. Adjust the entries for your node/company
                    */
                    /* Values can be modified */

                    s_identity Linux_slave_obj1018 =
                    {
                      4,       // number of supported entries
                      0,  // Vendor-ID (given by the can-cia)
                      0,  // Product Code
                      0,  // Revision number
                      0  // serial number
                    };

                    subindex Linux_slave_Index1018[] =
                    {
                      { RO, uint8,  sizeof(UNS8),  (void*)&Linux_slave_obj1018.count },
                      { RO, uint32, sizeof(UNS32), (void*)&Linux_slave_obj1018.vendor_id},
                      { RO, uint32, sizeof(UNS32), (void*)&Linux_slave_obj1018.product_code},
                      { RO, uint32, sizeof(UNS32), (void*)&Linux_slave_obj1018.revision_number},
                      { RO, uint32, sizeof(UNS32), (void*)&Linux_slave_obj1018.serial_number}
                    };

/* index 0x1200 :   The SDO Server parameters */
                    /* BEWARE You cannot define more than one SDO server */
                    /* The values should not be modified here, 
                    but can be changed at runtime */
                    // Beware that the default values that you could put here
                    // will be over written at the initialisation of the node. 
                    // See setNodeId() in init.c
                    s_sdo_parameter Linux_slave_obj1200  = 
                      { 3,                   // Number of entries. Always 3 for the SDO	       
                        0x601,     // The cob_id transmited in CAN msg to the server     
                        0x581,     // The cob_id received in CAN msg from the server  
                        0x01      // The node id of the client. Should not be modified
                      };
                    subindex Linux_slave_Index1200[] =
                    {
                      { RO, uint8,  sizeof( UNS8 ), (void*)&Linux_slave_obj1200.count },
                      { RO, uint32, sizeof( UNS32), (void*)&Linux_slave_obj1200.cob_id_client },
                      { RO, uint32, sizeof( UNS32), (void*)&Linux_slave_obj1200.cob_id_server },
                      { RW, uint8,  sizeof( UNS8),  (void*)&Linux_slave_obj1200.node_id }
                    };

/* index 0x1280 :   SDO client parameter */
                    s_sdo_parameter Linux_slave_obj1280 = 
                      { 3,     // Nb of entries 
                        0x600, // cobid transmited to the server. The good value should be 0x600 + server nodeId
                        0x580, // cobid received from the server. The good value should be 0x580 + server nodeId
                        0x01  // server NodeId
                      };
                    subindex Linux_slave_Index1280[] = 
                      { 
                        { RO, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1280.count },
                        { RW, uint8, sizeof( UNS32 ), (void*)&Linux_slave_obj1280.cob_id_client },
                        { RW, uint8, sizeof( UNS32 ), (void*)&Linux_slave_obj1280.cob_id_server },
                        { RW, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1280.node_id }
                      }; 

/* index 0x1400 :   PDO receive communication parameter */
                    s_pdo_communication_parameter Linux_slave_obj1400 = 
                      { 2, // Largest subindex supported 
                        0x0, // Default COBID (overwritten at init for index 0x1400 to 0x1403)
                        253 // Transmission type. See objdictdef.h 
                      };
                    subindex Linux_slave_Index1400[] = 
                      { 
                        { RO, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1400.count },
                        { RW, uint8, sizeof( UNS32 ), (void*)&Linux_slave_obj1400.cob_id },
                        { RW, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1400.type },
                      }; 

/* index 0x1401 :   PDO receive communication parameter */
                    s_pdo_communication_parameter Linux_slave_obj1401 = 
                      { 2, // Largest subindex supported 
                        0x0, // Default COBID (overwritten at init for index 0x1400 to 0x1403)
                        253 // Transmission type. See objdictdef.h 
                      };
                    subindex Linux_slave_Index1401[] = 
                      { 
                        { RO, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1401.count },
                        { RW, uint8, sizeof( UNS32 ), (void*)&Linux_slave_obj1401.cob_id },
                        { RW, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1401.type },
                      }; 

/* index 0x1402 :   PDO receive communication parameter */
                    s_pdo_communication_parameter Linux_slave_obj1402 = 
                      { 2, // Largest subindex supported 
                        0x0, // Default COBID (overwritten at init for index 0x1400 to 0x1403)
                        253 // Transmission type. See objdictdef.h 
                      };
                    subindex Linux_slave_Index1402[] = 
                      { 
                        { RO, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1402.count },
                        { RW, uint8, sizeof( UNS32 ), (void*)&Linux_slave_obj1402.cob_id },
                        { RW, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1402.type },
                      }; 

/* index 0x1600 :   PDO receive mapping parameter of PDO communication index 0x1400 */
                    UNS8 Linux_slave_obj1600_cnt = 0; // Number of mapped variables
                    UNS32 Linux_slave_obj1600_mappedVar[] = { 
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000
                      };
                    subindex Linux_slave_Index1600[] = 
                      { 
                        { RW, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1600_cnt },
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1600_mappedVar[0]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1600_mappedVar[1]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1600_mappedVar[2]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1600_mappedVar[3]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1600_mappedVar[4]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1600_mappedVar[5]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1600_mappedVar[6]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1600_mappedVar[7]}
                      }; 

/* index 0x1601 :   PDO receive mapping parameter of PDO communication index 0x1401 */
                    UNS8 Linux_slave_obj1601_cnt = 0; // Number of mapped variables
                    UNS32 Linux_slave_obj1601_mappedVar[] = { 
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000
                      };
                    subindex Linux_slave_Index1601[] = 
                      { 
                        { RW, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1601_cnt },
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1601_mappedVar[0]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1601_mappedVar[1]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1601_mappedVar[2]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1601_mappedVar[3]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1601_mappedVar[4]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1601_mappedVar[5]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1601_mappedVar[6]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1601_mappedVar[7]}
                      }; 

/* index 0x1602 :   PDO receive mapping parameter of PDO communication index 0x1402 */
                    UNS8 Linux_slave_obj1602_cnt = 0; // Number of mapped variables
                    UNS32 Linux_slave_obj1602_mappedVar[] = { 
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000
                      };
                    subindex Linux_slave_Index1602[] = 
                      { 
                        { RW, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1602_cnt },
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1602_mappedVar[0]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1602_mappedVar[1]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1602_mappedVar[2]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1602_mappedVar[3]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1602_mappedVar[4]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1602_mappedVar[5]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1602_mappedVar[6]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1602_mappedVar[7]}
                      }; 

/* index 0x1800 :   PDO transmit communication parameter */
                    s_pdo_communication_parameter Linux_slave_obj1800 = 
                      { 2, // Largest subindex supported 
                        0x0, // Default COBID (overwritten at init for index 0x1400 to 0x1403)
                        253 // Transmission type. See objdictdef.h 
                      };
                    subindex Linux_slave_Index1800[] = 
                      { 
                        { RO, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1800.count },
                        { RW, uint8, sizeof( UNS32 ), (void*)&Linux_slave_obj1800.cob_id },
                        { RW, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1800.type },
                      }; 

/* index 0x1801 :   PDO transmit communication parameter */
                    s_pdo_communication_parameter Linux_slave_obj1801 = 
                      { 2, // Largest subindex supported 
                        0x0, // Default COBID (overwritten at init for index 0x1400 to 0x1403)
                        253 // Transmission type. See objdictdef.h 
                      };
                    subindex Linux_slave_Index1801[] = 
                      { 
                        { RO, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1801.count },
                        { RW, uint8, sizeof( UNS32 ), (void*)&Linux_slave_obj1801.cob_id },
                        { RW, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1801.type },
                      }; 

/* index 0x1802 :   PDO transmit communication parameter */
                    s_pdo_communication_parameter Linux_slave_obj1802 = 
                      { 2, // Largest subindex supported 
                        0x0, // Default COBID (overwritten at init for index 0x1400 to 0x1403)
                        253 // Transmission type. See objdictdef.h 
                      };
                    subindex Linux_slave_Index1802[] = 
                      { 
                        { RO, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1802.count },
                        { RW, uint8, sizeof( UNS32 ), (void*)&Linux_slave_obj1802.cob_id },
                        { RW, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1802.type },
                      }; 

/* index 0x1A00 :   PDO transmit mapping parameter of PDO communication index 0x1800 */
                    UNS8 Linux_slave_obj1A00_cnt = 0; // Number of mapped variables
                    UNS32 Linux_slave_obj1A00_mappedVar[] = { 
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000
                      };
                    subindex Linux_slave_Index1A00[] = 
                      { 
                        { RW, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1A00_cnt },
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A00_mappedVar[0]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A00_mappedVar[1]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A00_mappedVar[2]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A00_mappedVar[3]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A00_mappedVar[4]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A00_mappedVar[5]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A00_mappedVar[6]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A00_mappedVar[7]}
                      }; 

/* index 0x1A01 :   PDO transmit mapping parameter of PDO communication index 0x1801 */
                    UNS8 Linux_slave_obj1A01_cnt = 0; // Number of mapped variables
                    UNS32 Linux_slave_obj1A01_mappedVar[] = { 
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000
                      };
                    subindex Linux_slave_Index1A01[] = 
                      { 
                        { RW, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1A01_cnt },
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A01_mappedVar[0]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A01_mappedVar[1]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A01_mappedVar[2]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A01_mappedVar[3]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A01_mappedVar[4]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A01_mappedVar[5]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A01_mappedVar[6]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A01_mappedVar[7]}
                      }; 

/* index 0x1A02 :   PDO transmit mapping parameter of PDO communication index 0x1802 */
                    UNS8 Linux_slave_obj1A02_cnt = 0; // Number of mapped variables
                    UNS32 Linux_slave_obj1A02_mappedVar[] = { 
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00000000
                      };
                    subindex Linux_slave_Index1A02[] = 
                      { 
                        { RW, uint8, sizeof( UNS8  ), (void*)&Linux_slave_obj1A02_cnt },
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A02_mappedVar[0]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A02_mappedVar[1]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A02_mappedVar[2]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A02_mappedVar[3]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A02_mappedVar[4]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A02_mappedVar[5]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A02_mappedVar[6]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&Linux_slave_obj1A02_mappedVar[7]}
                      }; 


/* index 0x2000 :   Mapped variable */
                    UNS8 Linux_slave_highestSubIndex_2000 = 4; // number of subindex - 1
                    subindex Linux_slave_Index2000[] = 
                     {
                       { RO, uint8, sizeof (UNS8), (void*)&Linux_slave_highestSubIndex_2000 },
                       { RW, uint8, sizeof (UNS8), (void*)&seconds },
                       { RW, uint8, sizeof (UNS8), (void*)&minutes },
                       { RW, uint8, sizeof (UNS8), (void*)&hours },
                       { RW, uint8, sizeof (UNS8), (void*)&day }
                     };

/* index 0x6000 :   Mapped variable */
                    UNS8 Linux_slave_highestSubIndex_6000 = 0; // number of subindex - 1
                    subindex Linux_slave_Index6000[] = 
                     {
                       { RW, uint32, sizeof (UNS32), (void*)&canopenErrNB }
                     };

/* index 0x6001 :   Mapped variable */
                    UNS8 Linux_slave_highestSubIndex_6001 = 0; // number of subindex - 1
                    subindex Linux_slave_Index6001[] = 
                     {
                       { RW, uint32, sizeof (UNS32), (void*)&canopenErrVAL }
                     };

/* index 0x6002 :   Mapped variable */
                    UNS8 Linux_slave_highestSubIndex_6002 = 0; // number of subindex - 1
                    subindex Linux_slave_Index6002[] = 
                     {
                       { RW, visible_string, sizeof (strTest), (void*)&strTest }
                     };

const indextable Linux_slave_objdict[] = 
{
  DeclareIndexTableEntry(Linux_slave_Index1000, 0x1000),
  DeclareIndexTableEntry(Linux_slave_Index1001, 0x1001),
  DeclareIndexTableEntry(Linux_slave_Index1005, 0x1005),
  DeclareIndexTableEntry(Linux_slave_Index1006, 0x1006),
  DeclareIndexTableEntry(Linux_slave_Index1007, 0x1007),
  DeclareIndexTableEntry(Linux_slave_Index1008, 0x1008),
  DeclareIndexTableEntry(Linux_slave_Index1009, 0x1009),
  DeclareIndexTableEntry(Linux_slave_Index100A, 0x100A),
  DeclareIndexTableEntry(Linux_slave_Index1016, 0x1016),
  DeclareIndexTableEntry(Linux_slave_Index1017, 0x1017),
  DeclareIndexTableEntry(Linux_slave_Index1018, 0x1018),
  DeclareIndexTableEntry(Linux_slave_Index1200, 0x1200),
  DeclareIndexTableEntry(Linux_slave_Index1280, 0x1280),
  DeclareIndexTableEntry(Linux_slave_Index1400, 0x1400),
  DeclareIndexTableEntry(Linux_slave_Index1401, 0x1401),
  DeclareIndexTableEntry(Linux_slave_Index1402, 0x1402),
  DeclareIndexTableEntry(Linux_slave_Index1600, 0x1600),
  DeclareIndexTableEntry(Linux_slave_Index1601, 0x1601),
  DeclareIndexTableEntry(Linux_slave_Index1602, 0x1602),
  DeclareIndexTableEntry(Linux_slave_Index1800, 0x1800),
  DeclareIndexTableEntry(Linux_slave_Index1801, 0x1801),
  DeclareIndexTableEntry(Linux_slave_Index1802, 0x1802),
  DeclareIndexTableEntry(Linux_slave_Index1A00, 0x1A00),
  DeclareIndexTableEntry(Linux_slave_Index1A01, 0x1A01),
  DeclareIndexTableEntry(Linux_slave_Index1A02, 0x1A02),
  DeclareIndexTableEntry(Linux_slave_Index2000, 0x2000),
  DeclareIndexTableEntry(Linux_slave_Index6000, 0x6000),
  DeclareIndexTableEntry(Linux_slave_Index6001, 0x6001),
  DeclareIndexTableEntry(Linux_slave_Index6002, 0x6002),
};

// To count at which received SYNC a PDO must be sent.
// Even if no pdoTransmit are defined, at least one entry is computed
// for compilations issues.
UNS8 Linux_slave_count_sync[1] = {0, };

quick_index Linux_slave_firstIndex = {
    SDO_SVR : 11,
    SDO_CLT : 12,
    PDO_RCV : 13,
    PDO_RCV_MAP : 16,
    PDO_TRS : 19,
    PDO_TRS_MAP : 22
}

quick_index Linux_slave_lastIndex{
    SDO_SVR : 11,
    SDO_CLT : 12,
    PDO_RCV : 15,
    PDO_RCV_MAP : 18,
    PDO_TRS : 21,
    PDO_TRS_MAP : 24
}

UNS16 Linux_slave_ObjdictSize = sizeof(Linux_slave_objdict)/sizeof(Linux_slave_objdict[0]); 

