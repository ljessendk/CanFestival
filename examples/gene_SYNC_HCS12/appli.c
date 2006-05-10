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


#define DEBUG_WAR_CONSOLE_ON
#define DEBUG_ERR_CONSOLE_ON

#include <stddef.h> /* for NULL */

#include <asm-m68hc12/portsaccess.h>
#include <asm-m68hc12/ports_def.h>
#include <asm-m68hc12/ports.h>
#include  <interrupt.h>

#include <applicfg.h>



#include <../include/hcs12/candriver.h>

#include "../include/def.h"
#include "../include/can.h"
#include "../include/objdictdef.h"
#include "../include/objacces.h"
#include "../include/sdo.h"
#include "../include/pdo.h"
#include "../include/timer.h"
#include "../include/lifegrd.h"
#include "../include/sync.h"


#include "../include/nmtSlave.h"
#include "objdict.h"


// HCS12 configuration
// -----------------------------------------------------

enum E_CanBaudrate 
{
   CAN_BAUDRATE_250K,
   CAN_BAUDRATE_500K,
   CAN_BAUDRATE_1M,
};


const canBusTime CAN_Baudrates[] =
{
   {
      1,  /* clksrc: Use the bus clock : 16 MHz, the freq. of the quartz's board        */
      3,  /* brp :  chose btw 0 and 63 (6 bits).  freq time quantum = 16MHz / (brp + 1) */
      0,  /* sjw : chose btw 0 and 3 (2 bits). Sync on (sjw + 1 ) time quantum          */
      0,  /* samp : chose btw 0 and 3 (2 bits) (samp + 1 ) samples per bit              */
      1,  /* tseg2 : chose btw 0 and 7 (3 bits) Segment 2 width = (tseg2 + 1)  tq       */
     12,  /* tseg1 : chose btw 0 and 15 (4 bits) Segment 1 width = (tseg1 + 1)  tq      */

      /*
      With these values, 
      - The width of the bit time is 16 time quantum :
          - 1 tq for the SYNC segment (could not be modified)
          - 13 tq for the TIME 1 segment (tseg1 = 12)
          - 2 tq for the TIME 2 segment (tseg2 = 1)
      - Because the bus clock of the MSCAN is 16 MHZ, and the 
        freq of the time quantum is 4 MHZ (brp = 3+1), and  there are 16 tq in the bit time,
        so the freq of the bit time is 250 kHz.
      */
   },

   {
      1,  /* clksrc: Use the bus clock : 16 MHz, the freq. of the quartz's board        */
      1,  /* brp :  chose btw 0 and 63 (6 bits).  freq time quantum = 16MHz / (brp + 1) */
      0,  /* sjw : chose btw 0 and 3 (2 bits). Sync on (sjw + 1 ) time quantum          */
      0,  /* samp : chose btw 0 and 3 (2 bits) (samp + 1 ) samples per bit              */
      1,  /* tseg2 : chose btw 0 and 7 (3 bits) Segment 2 width = (tseg2 + 1)  tq       */
     12,  /* tseg1 : chose btw 0 and 15 (4 bits) Segment 1 width = (tseg1 + 1)  tq      */

      /*
      With these values, 
      - The width of the bit time is 16 time quantum :
          - 1 tq for the SYNC segment (could not be modified)
          - 13 tq for the TIME 1 segment (tseg1 = 12)
          - 2 tq for the TIME 2 segment (tseg2 = 1)
      - Because the bus clock of the MSCAN is 16 MHZ, and the 
        freq of the time quantum is 8 MHZ (brp = 1+1), and  there are 16 tq in the bit time,
        so the freq of the bit time is 500 kHz.
      */
    },

	{
      1,  /* clksrc: Use the bus clock : 16 MHz, the freq. of the quartz's board        */
      1,  /* brp :  chose btw 0 and 63 (6 bits).  freq time quantum = 16MHz / (brp + 1) */
      0,  /* sjw : chose btw 0 and 3 (2 bits). Sync on (sjw + 1 ) time quantum          */
      0,  /* samp : chose btw 0 and 3 (2 bits) (samp +MSG_WAR(0x3F33, "Je suis le noeud ", getNodeId());    1 ) samples per bit              */
      1,  /* tseg2 : chose btw 0 and 7 (3 bits) Segment 2 width = (tseg2 + 1)  tq       */
      4,  /* tseg1 : chose btw 0 and 15 (4 bits) Segment 1 width = (tseg1 + 1)  tq      */

      /*
      With these values, 
      - The width of the bit time is 16 time quantum :
          - 1 tq for the SYNC segment (could not be modified)
          - 5 tq for the TIME 1 segment (tseg1 = 4)
          - 2 tq for the TIME 2 segment (tseg2 = 1)
      - Because the bus clock of the MSCAN is 16 MHZ, and the 
        freq of the time quantum is 8 MHZ (brp = 1+1), and  there are 8 tq in the bit time,
        so the freq of the bit time is 1 MHz.
      */
    }
};




/**************************prototypes*****************************************/

//fonction d'initialisation du bus can et la couche CANOPEN pour le capteur
void initCanopencapteur (void);
// les fonctions d'initialisation du capteur: timer, compteurs logiciel
void initSensor(void);
void initPortB(void);

//------------------------------------------------------------------------------
//Initialisation of the port B for the leds.
void initPortB(void)
{
  // Port B is output
  IO_PORTS_8(DDRB)= 0XFF;
  // RAZ
  IO_PORTS_8(PORTB) = 0xFF;
}



//------------------------------------------------------------------------------
void initSensor(void)
{ 
  UNS8 baudrate = 0;
  MSG_WAR(0x3F33, "I am the node :  ", getNodeId(&gene_SYNC_Data));  
  // Init led control
  initPortB(); 
  IO_PORTS_8(PORTB) &= ~ 0x01; //One led ON
  // Init port to choose se CAN baudrate by switch
  IO_PORTS_8(ATD0DIEN) = 0x03;
  
  canBusInit bi0 = {
    0,    /* no low power                 */ 
    0,    /* no time stamp                */
    1,    /* enable MSCAN                 */
    0,    /* clock source : oscillator (In fact, it is not used)   */
    0,    /* no loop back                 */
    0,    /* no listen only               */
    0,    /* no low pass filter for wk up */
	CAN_Baudrates[CAN_BAUDRATE_250K],
    {
      0x00,    /* Filter on 16 bits. See Motorola Block Guide V02.14 fig 4-3 */
      0x00, 0xFF, /* filter 0 hight accept all msg      */
      0x00, 0xFF, /* filter 0 low accept all msg        */
      0x00, 0xFF, /* filter 1 hight filter all of  msg  */
      0x00, 0xFF, /* filter 1 low filter all of  msg    */
      0x00, 0xFF, /* filter 2 hight filter most of  msg */
      0x00, 0xFF, /* filter 2 low filter most of  msg   */
      0x00, 0xFF, /* filter 3 hight filter most of  msg */
      0x00, 0xFF, /* filter 3 low filter most of  msg   */
    }
  };
  
  //Init the HCS12 microcontroler for CanOpen 
  initHCS12();
   
  // Chose the CAN rate
  baudrate = IO_PORTS_8(PORTAD0) & 0x03;
  switch (baudrate) {
  case 1:
    bi0.clk = CAN_Baudrates[CAN_BAUDRATE_250K];
    MSG_WAR(0x3F30, "CAN 250 kbps ", 0);
    break;
  case 2:
    bi0.clk = CAN_Baudrates[CAN_BAUDRATE_500K];
    MSG_WAR(0x3F31, "CAN 500 kbps ", 0);
    break;
  case 3:
    bi0.clk = CAN_Baudrates[CAN_BAUDRATE_1M];
    MSG_WAR(0x3F31, "CAN 1000 kbps ", 0);
    break;   
  default:
    MSG_WAR(0x2F32, "CAN BAUD RATE NOT DEFINED ", 0);
  }

  MSG_WAR(0x3F33, "SYNC signal generator", 0);

  canInit(CANOPEN_LINE_NUMBER_USED, bi0);  //initialize filters...
  initTimer(); // Init hcs12 timer used by CanFestival. (see timerhw.c)
  unlock(); // Allow interruptions  
}






//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// FUNCTIONS WHICH ARE PART OF CANFESTIVAL and *must* be implemented here.

//------------------------------------------------------------------------------
void gene_SYNC_heartbeatError( UNS8 heartbeatID )
{
  
  MSG_ERR(0x1F00, "HeartBeat not received from node : ", heartbeatID);
}

//------------------------------------------------------------------------------
void gene_SYNC_SDOtimeoutError (UNS8 line)
{
  MSG_ERR(0x1F01, "SDO timeout for line : ", line);
}

//------------------------------------------------------------------------------
UNS8 gene_SYNC_canSend(Message *m)
{
  // HCS12 driver function to send the CAN msg
  canMsgTransmit(CAN0, *m);
  return 0;
}

//------------------------------------------------------------------------------
void gene_SYNC_initialisation()
{  
  MSG_WAR (0x3F00, "Entering in INIT ", 0); 
  initSensor();
  IO_PORTS_8(PORTB) &= ~ 0x01; // led  0         : ON
  IO_PORTS_8(PORTB) |=   0x0E; // leds 1, 2, 3   : OFF
}


//------------------------------------------------------------------------------
void gene_SYNC_preOperational()
{
  MSG_WAR (0x3F01, "Entering in PRE-OPERATIONAL ", 0); 
  IO_PORTS_8(PORTB) &= ~ 0x03; // leds 0, 1      : ON
  IO_PORTS_8(PORTB) |=   0x0C; // leds 2, 3      : OFF
}


//------------------------------------------------------------------------------
void gene_SYNC_operational()
{ 
   MSG_WAR (0x3F02, "Entering in OPERATIONAL ", 0); 
   IO_PORTS_8(PORTB) &= ~ 0x07; // leds 0, 1, 2   : ON
   IO_PORTS_8(PORTB) |=   0x08; // leds 3         : OFF
}

//------------------------------------------------------------------------------
void gene_SYNC_stopped()
{
  MSG_WAR (0x3F02, "Entering in STOPPED ", 0); 
}

//------------------------------------------------------------------------------
void gene_SYNC_post_sync()
{
}

//------------------------------------------------------------------------------
void gene_SYNC_post_TPDO()
{
}

// End functions which are part of Canfestival
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


/******************************************************************************/
/********************************* MAIN ***************************************/
/******************************************************************************/

 
UNS8 main (void)
{

  MSG_WAR(0x3F34, "Entering in the main", 0);
  //----------------------------- INITIALISATION --------------------------------
  /* Defining the node Id */
  setNodeId(&gene_SYNC_Data, 0x03);

  /* Put the node in Initialisation mode */
  MSG_WAR(0x3F35, "va passer en init", 0);
  setState(&gene_SYNC_Data, Initialisation);

  //----------------------------- START -----------------------------------------
  /* Put the node in pre-operational mode */
  //MSG_WAR(0x3F36, "va passer en pre-op", 0);
  //setState(&gene_SYNC_Data, Pre_operational);

    while (1) {
      {
	Message m;
	if (f_can_receive(0, &m)) {
	  MSG_WAR(0x3F36, "Msg received", m.cob_id.w);
	  canDispatch(&gene_SYNC_Data, &m);
	}
	  
	
      }
      
    }

  return (0); 
}



