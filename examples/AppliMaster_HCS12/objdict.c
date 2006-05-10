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

#include <applicfg.h>
#include "def.h"
#include "can.h"
#include "objdictdef.h"
#include "pdo.h"
#include "sdo.h"
#include "sync.h"
#include "lifegrd.h"
#include "nmtSlave.h"

/**************************************************************************/
/* Declaration of the mapped variables                                    */
/**************************************************************************/



/*// Copy and uncomment this code to your main file
extern UNS32 canopenErrNB_node5;		// Mapped at index 0x6000, subindex 0x00
extern UNS32 canopenErrVAL_node5;		// Mapped at index 0x6001, subindex 0x00
extern UNS8 second;		// Mapped at index 0x6002, subindex 0x01
extern UNS8 minutes;		// Mapped at index 0x6002, subindex 0x02
extern UNS8 hour;		// Mapped at index 0x6002, subindex 0x03
extern UNS8 day;		// Mapped at index 0x6002, subindex 0x04
extern UNS32 canopenErrNB;		// Mapped at index 0x6003, subindex 0x01
extern UNS32 canopenErrVAL;		// Mapped at index 0x6003, subindex 0x02

// END copy to main file
*/

UNS32 canopenErrNB_node5;		// Mapped at index 0x6000, subindex 0x00
UNS32 canopenErrVAL_node5;		// Mapped at index 0x6001, subindex 0x00
UNS8 second;		// Mapped at index 0x6002, subindex 0x01
UNS8 minutes;		// Mapped at index 0x6002, subindex 0x02
UNS8 hour;		// Mapped at index 0x6002, subindex 0x03
UNS8 day;		// Mapped at index 0x6002, subindex 0x04
UNS32 canopenErrNB;		// Mapped at index 0x6003, subindex 0x01
UNS32 canopenErrVAL;		// Mapped at index 0x6003, subindex 0x02

/**************************************************************************/
/* Declaration of the value range types                                   */
/**************************************************************************/



UNS32 valueRangeTest (UNS8 typeValue, UNS32 unsValue, REAL32 realValue)
{
  return 0;
}


/**************************************************************************/
/* The node id                                                            */
/**************************************************************************/
/* Computed by strNode */
/* node_id default value. 
   This default value is deprecated.
   You should always overwrite this by using the function setNodeId(UNS8 nodeId) in your C code.
*/
#define NODE_ID 0x01
UNS8 bDeviceNodeId = NODE_ID;


//*****************************************************************************/
/* Computed by strStartDico */

/* Array of message processing information */
/* Should not be modified */
volatile const proceed_info proceed_infos[] = {
  {NMT,		"NMT",	        NULL},
  {SYNC,        "SYNC",         proceedSYNC},
  {TIME_STAMP,	"TStmp",	NULL},
  {PDO1tx,	"PDO1t",        proceedPDO},
  {PDO1rx,	"PDO1r",	proceedPDO},
  {PDO2tx,	"PDO2t",	proceedPDO},
  {PDO2rx,	"PDO2r",	proceedPDO},
  {PDO3tx,	"PDO3t",	proceedPDO},
  {PDO3rx,	"PDO3r",	proceedPDO},
  {PDO4tx,	"PDO4t",	proceedPDO},
  {PDO4rx,	"PDO4r",	proceedPDO},
  {SDOtx,	"SDOt",	        proceedSDO},
  {SDOrx,	"SDOr",         proceedSDO},
  {0xD,		"Unkw",	        NULL},
  {NODE_GUARD,	"NGrd",         proceedNMTerror},
  {0xF,		"Unkw",	        NULL}
};

  // Macros definition

/* Beware : 
index                 *must* be writen 4 numbers in hexa
sub_index             *must* be writen 2 numbers in hexa
size_variable_in_UNS8 *must* be writen 2 numbers in hexa
*/
#define PDO_MAP(index, sub_index, size_variable_in_bits)\
0x ## index ## sub_index ## size_variable_in_bits

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
                    
                    UNS32 obj1000 = 0;
                    subindex Index1000[] =
                    {
                      { RO, uint32, sizeof(UNS32), (void*)&obj1000 }
                    };

/* index 0x1001 :   Error register. 
                    Change the entries to fit your application 
                    Not used, so, should not be modified */
                    /*const*/ UNS8 obj1001 = 0x0;
                    /*const*/ subindex Index1001[] =
                    {
                      { RO, uint8, sizeof(UNS8), (void*)&obj1001 }
                    };

/* index 0x1005 :   COB_ID SYNC */
                    /* Should not be modified */
                    UNS32 obj1005 = 0x00000080; // bit 30 = 1 : device can generate a SYNC message
                                                // Beware, it is over written when the node 
                                                // enters in reset mode
                                                // See initResetMode() in init.c
                    /*const*/ subindex Index1005[] =
                    {
                      { RW, uint32, sizeof(UNS32), (void*)&obj1005 }
                    };

/* index 0x1006 :   SYNC period */
                    // For producing the SYNC signal every n micro-seconds.
                    // Put O to not producing SYNC
                    /*const*/ UNS32 obj1006 = 0x0; 
                                         // Default 0 to not produce SYNC //
                                         // Beware, it is over written when the 
                                         // node enters in reset mode.
                                         // See initResetMode() in init.c
                    /*const*/ subindex Index1006[] =
                    {
                      { RW, uint32, sizeof(UNS32), (void*)&obj1006 }
                    };
 
/* index 0x1007 :   Synchronous Window Length
                    Seems to be needed by DS401 to generate the SYNC signal ! */
                    /*const*/ UNS32 obj1007 = 0x0; /* Default 0 */
                    /*const*/ subindex Index1007[] =
                    {
                      { RW, uint32, sizeof(UNS32), (void*)&obj1007 }
                    };

/* index 0x1008 :   Manufacturer device name */
                    UNS8 obj1008[] = "Appli_Master_HC12"; /* Default 0 */
                    subindex Index1008[] =
                    {
                      { RO, uint32, sizeof(obj1008), (void*)&obj1008 }
                    };

/* index 0x1009 :   Manufacturer hardware version */
                    UNS8 obj1009[] = __DATE__; /* Default 0 */
                    subindex Index1009[] =
                    {
                      { RO, uint32, sizeof(obj1009), (void*)&obj1009 }
                    };

/* index 0x100A :   Manufacturer software version */
                    UNS8 obj100A[] = __TIME__; /* Default 0 */
                    subindex Index100A[] =
                    {
                      { RO, uint32, sizeof(obj100A), (void*)&obj100A}
                    };

/* index 0x1016 :   HeartBeat consumers 
                    The nodes which can send a heartbeat */ 
                    static  UNS32 obj1016[] = {// Consumer time for each node 
                    0x00000000}; // Format 0x00NNTTTT (N=Node T=time in ms)

                    static  UNS8 obj1016_cnt = 1; // 1 nodes could send me
                                                  // their heartbeat.
                    subindex Index1016[] = 
                    {
                      { RO, uint8, sizeof(UNS8), (void*)&obj1016_cnt },
                      { RW, uint32, sizeof(UNS32), (void*)&obj1016[0] }
                    }; 

/* index 0x1017 :   Heartbeat producer                    
                    Every HBProducerTime, the node sends its heartbeat */
                    static UNS16 obj1017 = 0; //HBProducerTime in ms. If 0 : not activated 
                                                     // Beware, it is over written when the 
                                                     // node enters in reset mode.
                                                     // See initResetMode() in init.c
                    subindex Index1017[] =
                    {
	              { RW, uint16, sizeof(UNS16), &obj1017 }
                    };

/* index 0x1018 :   Identity object */
                    /** index 1018: identify object. Adjust the entries for your node/company
                    */
                    /* Values can be modified */

                    s_identity obj1018 =
                    {
                      4,       // number of supported entries
                      0,  // Vendor-ID (given by the can-cia)
                      0,  // Product Code
                      0,  // Revision number
                      0  // serial number
                    };

                    subindex Index1018[] =
                    {
                      { RO, uint8,  sizeof(UNS8),  (void*)&obj1018.count },
                      { RO, uint32, sizeof(UNS32), (void*)&obj1018.vendor_id},
                      { RO, uint32, sizeof(UNS32), (void*)&obj1018.product_code},
                      { RO, uint32, sizeof(UNS32), (void*)&obj1018.revision_number},
                      { RO, uint32, sizeof(UNS32), (void*)&obj1018.serial_number}
                    };

/* index 0x1200 :   The SDO Server parameters */
                    /* BEWARE You cannot define more than one SDO server */
                    /* The values should not be modified here, 
                    but can be changed at runtime */
                    // Beware that the default values that you could put here
                    // will be over written at the initialisation of the node. 
                    // See setNodeId() in init.c
                    static s_sdo_parameter obj1200  = 
                      { 3,                   // Number of entries. Always 3 for the SDO	       
                        0x600 + NODE_ID,     // The cob_id transmited in CAN msg to the server     
                        0x580 + NODE_ID,     // The cob_id received in CAN msg from the server  
                        NODE_ID              // The node id of the client. Should not be modified 
                      };
                    static subindex Index1200[] =
                    {
                      { RO, uint8,  sizeof( UNS8 ), (void*)&obj1200.count },
                      { RO, uint32, sizeof( UNS32), (void*)&obj1200.cob_id_client },
                      { RO, uint32, sizeof( UNS32), (void*)&obj1200.cob_id_server },
                      { RW, uint8,  sizeof( UNS8),  (void*)&obj1200.node_id }
                    };

/* index 0x1280 :   SDO client parameter */
                    static s_sdo_parameter obj1280 = 
                      { 3,     // Nb of entries 
                        0x600, // cobid transmited to the server. The good value should be 0x600 + server nodeId
                        0x580, // cobid received from the server. The good value should be 0x580 + server nodeId
                        0x0    // server NodeId
                      };
                    static subindex Index1280[] = 
                      { 
                        { RO, uint8, sizeof( UNS8  ), (void*)&obj1280.count },
                        { RW, uint8, sizeof( UNS32 ), (void*)&obj1280.cob_id_client },
                        { RW, uint8, sizeof( UNS32 ), (void*)&obj1280.cob_id_server },
                        { RW, uint8, sizeof( UNS8  ), (void*)&obj1280.node_id }
                      }; 

/* index 0x1281 :   SDO client parameter */
                    static s_sdo_parameter obj1281 = 
                      { 3,     // Nb of entries 
                        0x600, // cobid transmited to the server. The good value should be 0x600 + server nodeId
                        0x580, // cobid received from the server. The good value should be 0x580 + server nodeId
                        0x0    // server NodeId
                      };
                    static subindex Index1281[] = 
                      { 
                        { RO, uint8, sizeof( UNS8  ), (void*)&obj1281.count },
                        { RW, uint8, sizeof( UNS32 ), (void*)&obj1281.cob_id_client },
                        { RW, uint8, sizeof( UNS32 ), (void*)&obj1281.cob_id_server },
                        { RW, uint8, sizeof( UNS8  ), (void*)&obj1281.node_id }
                      }; 

/* index 0x1400 :   PDO receive communication parameter */
                    static s_pdo_communication_parameter obj1400 = 
                      { 2, // Largest subindex supported 
                        0x0, // Default COBID (overwritten at init for index 0x1400 to 0x1403)
                        253 // Transmission type. See objdictdef.h 
                      };
                    static subindex Index1400[] = 
                      { 
                        { RO, uint8, sizeof( UNS8  ), (void*)&obj1400.count },
                        { RW, uint8, sizeof( UNS32 ), (void*)&obj1400.cob_id },
                        { RW, uint8, sizeof( UNS8  ), (void*)&obj1400.type },
                      }; 

/* index 0x1401 :   PDO receive communication parameter */
                    static s_pdo_communication_parameter obj1401 = 
                      { 2, // Largest subindex supported 
                        0x0, // Default COBID (overwritten at init for index 0x1400 to 0x1403)
                        253 // Transmission type. See objdictdef.h 
                      };
                    static subindex Index1401[] = 
                      { 
                        { RO, uint8, sizeof( UNS8  ), (void*)&obj1401.count },
                        { RW, uint8, sizeof( UNS32 ), (void*)&obj1401.cob_id },
                        { RW, uint8, sizeof( UNS8  ), (void*)&obj1401.type },
                      }; 

/* index 0x1402 :   PDO receive communication parameter */
                    static s_pdo_communication_parameter obj1402 = 
                      { 2, // Largest subindex supported 
                        0x0, // Default COBID (overwritten at init for index 0x1400 to 0x1403)
                        253 // Transmission type. See objdictdef.h 
                      };
                    static subindex Index1402[] = 
                      { 
                        { RO, uint8, sizeof( UNS8  ), (void*)&obj1402.count },
                        { RW, uint8, sizeof( UNS32 ), (void*)&obj1402.cob_id },
                        { RW, uint8, sizeof( UNS8  ), (void*)&obj1402.type },
                      }; 

/* index 0x1600 :   PDO receive mapping parameter of PDO communication index 0x1400 */
                    static UNS8 obj1600_cnt = 0; // Number of mapped variables
                    static UNS32 obj1600_mappedVar[] = { 
                        // Example to map a variable of 16 bits defined at index 0x6035, subindex 0x12 :
                        // PDO_MAP(6035,12,16)
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00)
                      };
                    subindex Index1600[] = 
                      { 
                        { RW, uint8, sizeof( UNS8  ), (void*)&obj1600_cnt },
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1600_mappedVar[0]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1600_mappedVar[1]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1600_mappedVar[2]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1600_mappedVar[3]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1600_mappedVar[4]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1600_mappedVar[5]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1600_mappedVar[6]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1600_mappedVar[7]}
                      }; 

/* index 0x1601 :   PDO receive mapping parameter of PDO communication index 0x1401 */
                    static UNS8 obj1601_cnt = 0; // Number of mapped variables
                    static UNS32 obj1601_mappedVar[] = { 
                        // Example to map a variable of 16 bits defined at index 0x6035, subindex 0x12 :
                        // PDO_MAP(6035,12,16)
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00)
                      };
                    subindex Index1601[] = 
                      { 
                        { RW, uint8, sizeof( UNS8  ), (void*)&obj1601_cnt },
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1601_mappedVar[0]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1601_mappedVar[1]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1601_mappedVar[2]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1601_mappedVar[3]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1601_mappedVar[4]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1601_mappedVar[5]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1601_mappedVar[6]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1601_mappedVar[7]}
                      }; 

/* index 0x1602 :   PDO receive mapping parameter of PDO communication index 0x1402 */
                    static UNS8 obj1602_cnt = 0; // Number of mapped variables
                    static UNS32 obj1602_mappedVar[] = { 
                        // Example to map a variable of 16 bits defined at index 0x6035, subindex 0x12 :
                        // PDO_MAP(6035,12,16)
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00)
                      };
                    subindex Index1602[] = 
                      { 
                        { RW, uint8, sizeof( UNS8  ), (void*)&obj1602_cnt },
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1602_mappedVar[0]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1602_mappedVar[1]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1602_mappedVar[2]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1602_mappedVar[3]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1602_mappedVar[4]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1602_mappedVar[5]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1602_mappedVar[6]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1602_mappedVar[7]}
                      }; 

/* index 0x1800 :   PDO transmit communication parameter */
                    static s_pdo_communication_parameter obj1800 = 
                      { 2, // Largest subindex supported 
                        0x0, // Default COBID (overwritten at init for index 0x1400 to 0x1403)
                        253 // Transmission type. See objdictdef.h 
                      };
                    static subindex Index1800[] = 
                      { 
                        { RO, uint8, sizeof( UNS8  ), (void*)&obj1800.count },
                        { RW, uint8, sizeof( UNS32 ), (void*)&obj1800.cob_id },
                        { RW, uint8, sizeof( UNS8  ), (void*)&obj1800.type },
                      }; 

/* index 0x1801 :   PDO transmit communication parameter */
                    static s_pdo_communication_parameter obj1801 = 
                      { 2, // Largest subindex supported 
                        0x0, // Default COBID (overwritten at init for index 0x1400 to 0x1403)
                        253 // Transmission type. See objdictdef.h 
                      };
                    static subindex Index1801[] = 
                      { 
                        { RO, uint8, sizeof( UNS8  ), (void*)&obj1801.count },
                        { RW, uint8, sizeof( UNS32 ), (void*)&obj1801.cob_id },
                        { RW, uint8, sizeof( UNS8  ), (void*)&obj1801.type },
                      }; 

/* index 0x1802 :   PDO transmit communication parameter */
                    static s_pdo_communication_parameter obj1802 = 
                      { 2, // Largest subindex supported 
                        0x0, // Default COBID (overwritten at init for index 0x1400 to 0x1403)
                        253 // Transmission type. See objdictdef.h 
                      };
                    static subindex Index1802[] = 
                      { 
                        { RO, uint8, sizeof( UNS8  ), (void*)&obj1802.count },
                        { RW, uint8, sizeof( UNS32 ), (void*)&obj1802.cob_id },
                        { RW, uint8, sizeof( UNS8  ), (void*)&obj1802.type },
                      }; 

/* index 0x1A00 :   PDO transmit mapping parameter of PDO communication index 0x1800 */
                    static UNS8 obj1A00_cnt = 0; // Number of mapped variables
                    static UNS32 obj1A00_mappedVar[] = { 
                        // Example to map a variable of 16 bits defined at index 0x6035, subindex 0x12 :
                        // PDO_MAP(6035,12,16)
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00)
                      };
                    subindex Index1A00[] = 
                      { 
                        { RW, uint8, sizeof( UNS8  ), (void*)&obj1A00_cnt },
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A00_mappedVar[0]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A00_mappedVar[1]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A00_mappedVar[2]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A00_mappedVar[3]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A00_mappedVar[4]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A00_mappedVar[5]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A00_mappedVar[6]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A00_mappedVar[7]}
                      }; 

/* index 0x1A01 :   PDO transmit mapping parameter of PDO communication index 0x1801 */
                    static UNS8 obj1A01_cnt = 0; // Number of mapped variables
                    static UNS32 obj1A01_mappedVar[] = { 
                        // Example to map a variable of 16 bits defined at index 0x6035, subindex 0x12 :
                        // PDO_MAP(6035,12,16)
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00)
                      };
                    subindex Index1A01[] = 
                      { 
                        { RW, uint8, sizeof( UNS8  ), (void*)&obj1A01_cnt },
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A01_mappedVar[0]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A01_mappedVar[1]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A01_mappedVar[2]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A01_mappedVar[3]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A01_mappedVar[4]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A01_mappedVar[5]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A01_mappedVar[6]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A01_mappedVar[7]}
                      }; 

/* index 0x1A02 :   PDO transmit mapping parameter of PDO communication index 0x1802 */
                    static UNS8 obj1A02_cnt = 0; // Number of mapped variables
                    static UNS32 obj1A02_mappedVar[] = { 
                        // Example to map a variable of 16 bits defined at index 0x6035, subindex 0x12 :
                        // PDO_MAP(6035,12,16)
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00),
                        PDO_MAP(0000, 00, 00)
                      };
                    subindex Index1A02[] = 
                      { 
                        { RW, uint8, sizeof( UNS8  ), (void*)&obj1A02_cnt },
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A02_mappedVar[0]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A02_mappedVar[1]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A02_mappedVar[2]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A02_mappedVar[3]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A02_mappedVar[4]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A02_mappedVar[5]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A02_mappedVar[6]},
                        { RW, uint32, sizeof( UNS32 ), (void*)&obj1A02_mappedVar[7]}
                      }; 


/* index 0x6000 :   Mapped variable */
                    subindex Index6000[] = 
                     {
                       { RW, uint32, sizeof (UNS32), (void*)&canopenErrNB_node5 }
                     };

/* index 0x6001 :   Mapped variable */
                    subindex Index6001[] = 
                     {
                       { RW, uint32, sizeof (UNS32), (void*)&canopenErrVAL_node5 }
                     };

/* index 0x6002 :   Mapped variable */
                    static UNS8 highestSubIndex_6002 = 4; // number of subindex - 1
                    subindex Index6002[] = 
                     {
                       { RO, uint8, sizeof (UNS8), (void*)&highestSubIndex_6002 },
                       { RW, uint8, sizeof (UNS8), (void*)&second },
                       { RW, uint8, sizeof (UNS8), (void*)&minutes },
                       { RW, uint8, sizeof (UNS8), (void*)&hour },
                       { RW, uint8, sizeof (UNS8), (void*)&day }
                     };

/* index 0x6003 :   Mapped variable */
                    static UNS8 highestSubIndex_6003 = 2; // number of subindex - 1
                    subindex Index6003[] = 
                     {
                       { RO, uint8, sizeof (UNS8), (void*)&highestSubIndex_6003 },
                       { RW, uint32, sizeof (UNS32), (void*)&canopenErrNB },
                       { RW, uint32, sizeof (UNS32), (void*)&canopenErrVAL }
                     };

const indextable objdict[] = 
{
  DeclareIndexTableEntry(Index1000, 0x1000),
  DeclareIndexTableEntry(Index1001, 0x1001),
  DeclareIndexTableEntry(Index1005, 0x1005),
  DeclareIndexTableEntry(Index1006, 0x1006),
  DeclareIndexTableEntry(Index1007, 0x1007),
  DeclareIndexTableEntry(Index1008, 0x1008),
  DeclareIndexTableEntry(Index1009, 0x1009),
  DeclareIndexTableEntry(Index100A, 0x100A),
  DeclareIndexTableEntry(Index1016, 0x1016),
  DeclareIndexTableEntry(Index1017, 0x1017),
  DeclareIndexTableEntry(Index1018, 0x1018),
  DeclareIndexTableEntry(Index1200, 0x1200),
  DeclareIndexTableEntry(Index1280, 0x1280),
  DeclareIndexTableEntry(Index1281, 0x1281),
  DeclareIndexTableEntry(Index1400, 0x1400),
  DeclareIndexTableEntry(Index1401, 0x1401),
  DeclareIndexTableEntry(Index1402, 0x1402),
  DeclareIndexTableEntry(Index1600, 0x1600),
  DeclareIndexTableEntry(Index1601, 0x1601),
  DeclareIndexTableEntry(Index1602, 0x1602),
  DeclareIndexTableEntry(Index1800, 0x1800),
  DeclareIndexTableEntry(Index1801, 0x1801),
  DeclareIndexTableEntry(Index1802, 0x1802),
  DeclareIndexTableEntry(Index1A00, 0x1A00),
  DeclareIndexTableEntry(Index1A01, 0x1A01),
  DeclareIndexTableEntry(Index1A02, 0x1A02),
  DeclareIndexTableEntry(Index6000, 0x6000),
  DeclareIndexTableEntry(Index6001, 0x6001),
  DeclareIndexTableEntry(Index6002, 0x6002),
  DeclareIndexTableEntry(Index6003, 0x6003),
};

// To count at which received SYNC a PDO must be sent.
// Even if no pdoTransmit are defined, at least one entry is computed
// for compilations issues.
UNS8 count_sync[1] = {0, };
  
UNS16 firstIndex (enum e_first_object object, UNS16 *lastIndex)
{
  switch (object) {
    case FIRST_SDO_SERVER :
      *lastIndex = 11;
      return 11;
    case FIRST_SDO_CLIENT :
      *lastIndex = 13;
      return 12;
    case FIRST_PDO_RCV :
      *lastIndex = 16;
      return 14;
    case FIRST_PDO_RCV_MAP :
      *lastIndex = 19;
      return 17;
    case FIRST_PDO_TRS :
      *lastIndex = 22;
      return 20;
    case FIRST_PDO_TRS_MAP :
      *lastIndex = 25;
      return 23;
  }
  *lastIndex = 0;
  return 0;
}

UNS16 getObjdictSize (void) 
{
  return sizeof(objdict)/sizeof(objdict[0]);
} 
