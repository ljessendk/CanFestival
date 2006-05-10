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

#include <stddef.h> /* for NULL */

#include <asm-m68hc12/portsaccess.h>
#include <asm-m68hc12/ports_def.h>
#include <asm-m68hc12/ports.h>
#include  <interrupt.h>

#include <applicfg.h>
#include <candriver.h>
#include <timerhw.h>

#include "../include/def.h"
#include "../include/can.h"
#include "../include/objdictdef.h"
#include "../include/objacces.h"
#include "../include/canOpenDriver.h"
#include "../include/sdo.h"
#include "../include/pdo.h"
#include "../include/init.h"
#include "../include/timer.h"
#include "../include/lifegrd.h"
#include "../include/sync.h"

#include "../include/nmtSlave.h"



// HCS12 configuration
// -----------------------------------------------------

enum E_CanBaudrate 
{
   CAN_BAUDRATE_250K,
   CAN_BAUDRATE_500K,
   CAN_BAUDRATE_1M,
   CAN_BAUDRATE_OLD_VALUE
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
      0,  /* samp : chose btw 0 and 3 (2 bits) (samp + 1 ) samples per bit              */
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
    },

	{
      1,  /* clksrc: Use the bus clock : 16 MHz, the freq. of the quartz's board        */
      0,  /* brp :  chose btw 0 and 63 (6 bits).  freq time quantum = 16MHz / (brp + 1) */
      1,  /* sjw : chose btw 0 and 3 (2 bits). Sync on (sjw + 1 ) time quantum          */
      1,  /* samp : chose btw 0 and 3 (2 bits) (samp + 1 ) samples per bit              */
      4,  /* tseg2 : chose btw 0 and 7 (3 bits) Segment 2 width = (tseg2 + 1)  tq       */
      9,  /* tseg1 : chose btw 0 and 15 (4 bits) Segment 1 width = (tseg1 + 1)  tq      */

      /*
      With these values, 
      - The width of the bit time is 16 time quantum :
          - 1 tq for the SYNC segment (could not be modified)
          - 10 tq for the TIME 1 segment (tseg1 = 9)
          - 5 tq for the TIME 2 segment (tseg2 = 4)
      - Because the bus clock of the MSCAN is 16 MHZ, and the 
        freq of the time quantum is 16 MHZ (brp = 0), and  there are 16 tq in the bit time,
        so the freq of the bit time is 1 MHz.
      */
    }
};


// The variables sent or updated by PDO
// -----------------------------------------------------
extern UNS8 seconds;		// Mapped at index 0x2000, subindex 0x1
extern UNS8 minutes;		// Mapped at index 0x2000, subindex 0x2
extern UNS8 hours;		// Mapped at index 0x2000, subindex 0x3
extern UNS8 day;		// Mapped at index 0x2000, subindex 0x4
extern UNS32 canopenErrNB;	// Mapped at index 0x6000, subindex 0x0
extern UNS32 canopenErrVAL;	// Mapped at index 0x6001, subindex 0x0

// Required definition variables
// -----------------------------
// The variables that you should define for debugging.
// They are used by the macro MSG_ERR and MSG_WAR in applicfg.h
// if the node is a slave, they can be mapped in the object dictionnary.
// if not null, allow the printing of message to the console
// Could be managed by PDO
UNS8 printMsgErrToConsole = 1;
UNS8 printMsgWarToConsole = 1;



/*************************User's variables declaration**************************/
UNS8 softCount = 0;
UNS8 lastMinute = 0;
UNS8 lastSecond = 0;
UNS8 sendingError = 0;
//--------------------------------FONCTIONS-------------------------------------
/* You *must* have these 2 functions in your code*/
void heartbeatError(UNS8 heartbeatID);
void SD0timeoutError(UNS8 bus_id, UNS8 line);

// Interruption timer 3. (The timer 4 is used by CanOpen)
void __attribute__((interrupt)) timer3Hdl (void);

void incDate(void);
void initLeds(void);
void initTimerClk(void);
void initCanHCS12 (void);
void initialisation(void);
void preOperational(void);
void operational(void);
void stopped(void);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Interruption timer 3
void __attribute__((interrupt)) timer3Hdl (void)
{
  //IO_PORTS_8(PORTB) ^= 0x10;
  //IO_PORTS_8(PORTB) &= ~0x20;
  IO_PORTS_8(TFLG1) = 0x08; // RAZ du flag interruption timer 3
  // Calcul evt suivant. Clock 8 MHz -> 8000 evt de 1 ms!! Doit tenir sur 16 bits
  // Attention, ça change si on utilise la pll
  // Lorsque le timer atteindra la valeur de TC3 (16 bits), l'interruption timer3Hdl sera déclenchée
  // Si on utilise la PLL à 24 MHZ, alors la vitesse du bus est multipliée par 3.

/*   Assume that our board uses a 16 MHz quartz */
/*   Without pre-division, 8000 counts takes 1 ms. */
/*   We are using a pre-divisor of 32. (register TSCR2) See in CanOpenDriverHC12/timerhw.c  */
/*   So 10000 counts takes 40 ms. */
/*   We must have a soft counter of 25 to count a second. */
  
  IO_PORTS_16(TC3H) += (10000); // IT every 40000 count.
  softCount++;
  if (softCount == 25) {
    softCount = 0;
    incDate();
  }
}

//------------------------------------------------------------------------------
void heartbeatError(UNS8 heartbeatID)
{
  MSG_ERR(0x1F00, "!!! No heart beat received from node : ", heartbeatID);
}

//------------------------------------------------------------------------------
void SD0timeoutError (UNS8 bus_id, UNS8 line)
{
  // Informations on what occurs are in transfers[bus_id][line]....
  // See scanSDOtimeout() in sdo.c
}

//------------------------------------------------------------------------------
// Incrementation of the date, every second
void incDate(void)
{
  if (seconds == 59) {
    seconds = 0;
    if (minutes == 59) {
      minutes = 0;
      if (hours == 23) {
	hours = 0;
	day++;
      } 
      else
	hours++;
    }
    else 
      minutes++;
  }
  else
    seconds++;

  // Toggle the led 4 every seconds
  IO_PORTS_8(PORTB) ^= 0x10;

}

//Initialisation of the port B for the leds.
void initLeds(void)
{
  // Port B is output
  IO_PORTS_8(DDRB)= 0XFF;
  // RAZ
  IO_PORTS_8(PORTB) = 0xFF;
}



//------------------------------------------------------------------------------
// Init the timer for the clock demo
void initTimerClk(void)
{

  lock();   // Inhibe les interruptions

  // Configuration du Channel 3
  IO_PORTS_8(TIOS) |= 0x08;     // Canal 3 en sortie
  IO_PORTS_8(TCTL2) &= ~(0xC0); // Canal 3 déconnecté du pin de sortie
  IO_PORTS_8(TIE) |= 0x08;      // Autorise interruption Canal 3
  IO_PORTS_8(TSCR1) |= 0x80;    // Mise en route du timer
  unlock(); // Autorise les interruptions
}


//------------------------------------------------------------------------------

// A placer avant initTimer de la bibliothèque CanOpen
/* void initTimerbis(void) */
/* {   */

/*   lock();                      // Inhibe les interruptions */
/*   // Configuration des IT Channels (0..3) */
/*   IO_PORTS_8(TIOS)  &= 0xF0;  // Canals 0->3 en entrées.    */
/*   IO_PORTS_8(TCTL4) &= 0XFD;  // Canal 0 détection sur front montant. */
/*   IO_PORTS_8(TCTL4) |= 0X01;    */
/*   IO_PORTS_8(TCTL4) &= 0XF7;  // Canal 1 détection sur front montant. */
/*   IO_PORTS_8(TCTL4) |= 0X04; */
/*   IO_PORTS_8(TCTL4) &= 0XDF;  // Canal 2 détection sur front montant. */
/*   IO_PORTS_8(TCTL4) |= 0X10;   */
/*   IO_PORTS_8(TCTL4) &= 0X7F;  // Canal 3 détection sur front montant. */
/*   IO_PORTS_8(TCTL4) |= 0X40;        */
/*   IO_PORTS_8(TSCR2) |= 0X05;  // Pre-scaler = 32.  */
   
/*   IO_PORTS_8(ICOVW) |= 0x0F;  // La sauvgrade des valeures de TC0 et TC0H  */
/*                               // correspondant aux canals (0..3) jusqu'a la */
/*                               // prochaine lecture dans ces registres.  */
/*   MASK  = IO_PORTS_8(ICSYS); */
/*   MASK &= 0xFE;               // Canals (0..3) en IC QUEUE MODE. */
/*   MASK |= 0x08;               // Canals (0..3) : génére une interruption aprés  */
/*                               // la capture de deux valeures du timer sur detection */
/*                               // d'un front montant à l'entrée des canals (0..3). */
/*   MASK |= 0x02;               */
/*   IO_PORTS_8(ICSYS) = MASK; */
/*   IO_PORTS_16(TC0HH);         // Vider le registre holding correspondant au canal0. */
/*   IO_PORTS_8(TSCR1) |= 0x10;  // RAZ automatique des flags d'interruption aprés lecture */
/*                               // dans les registres correspondant.   */
                              
/*   IO_PORTS_8(TIE)   |= 0x0F;  // Autorise interruption Canals (0..3). */
/*   IO_PORTS_8(TSCR2) |= 0X80;  // Autorise interruption sur l'Overflow.  */
/*   unlock();                   // Autorise les interruptions  */
  
/* } */

//------------------------------------------------------------------------------



void initCanHCS12 (void)
{  
  //Init the HCS12 microcontroler for CanOpen 
  initHCS12();
   // Init the HCS12  CAN driver
  const canBusInit bi0 = {
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

  canInit(CANOPEN_LINE_NUMBER_USED, bi0);  //initialize filters...
  unlock(); // Allow interruptions
}


/*********************************************************************/
void initialisation( void )
{ 
  //initcapteur();      //initialisation du capteur, timer, compteurs logiciels
  initCanHCS12();   //initialisation du bus Can
  MSG_WAR(0X3F05, "I am in INITIALISATION mode ", 0);
  /* Defining the node Id */
  setNodeId(0x05);
  MSG_WAR(0x3F06, "My node ID is : ", getNodeId()); 
  {
    UNS8 *data;
    UNS8 size;
    UNS8 dataType;
    // Manufacturer Device name (default = empty string)
    getODentry(0x1008, 0x0, (void **)&data, &size, &dataType, 0);
    MSG_WAR(0x3F09, data, 0);
    // Manufacturer Hardware version. (default = compilation. date)
    getODentry(0x1009, 0x0, (void **)&data, &size, &dataType, 0);
    MSG_WAR(0x3F09, data, 0);
    // Manufacturer Software version. (default = compilation. time)
    getODentry(0x100A, 0x0, (void **)&data, &size, &dataType, 0);
    MSG_WAR(0x3F09, data, 0);
  }
  initCANopenMain();    //initialisation du canopen 
  heartbeatInit();      //initialisation du lifeguarding
  initResetMode();
  initTimer();          //initialisation of the timer used by Canopen
  initTimerClk();
}


/*********************************************************************/
void preOperational(void)
{
  // Test if the heartBeat have been received. Send headbeat
  heartbeatMGR();
  // Read message
  receiveMsgHandler(0);
}


/********************************************************************/
void operational( void )
{ 

  // Init the errors
  canopenErrNB = 0;
  canopenErrVAL = 0;

  // Test if the heartBeat have been received. Send headbeat
  heartbeatMGR();
  // Read message
  receiveMsgHandler(0); 
  
  if (lastMinute != minutes) {
    MSG_WAR(0x3F00, "event : minutes change -> node decides to send it. Value : ", minutes);
    sendPDOevent( 0, &minutes );
    lastMinute = minutes;
  }

  if (canopenErrNB == 0)
    sendingError = 0;

  
  if (lastSecond != seconds) {
    MSG_WAR (0x3F50, "Seconds = ", seconds);
    if ((seconds == 50) && (sendingError == 0))
      {
	MSG_ERR(0x1F55, "DEMO of ERROR. Sent by PDO. Value : ", 0xABCD);
	sendingError = 1;
      }
    
    if (canopenErrNB) {
      MSG_WAR(0x3F56, "ERROR nb : ",  canopenErrNB);
    }
    lastSecond = seconds;
    
  }

}


/*****************************************************************************/
void stopped( void )
{
  heartbeatMGR();
  // Read message
  receiveMsgHandler(0);
}


/*****************************************************************************/



/********************************* MAIN ***************************************/

 
int main ()
{
  e_nodeState lastState = Unknown_state;

  /* CanOpen slave state machine         */
  /* ------------------------------------*/
    
  while(1) { /* slave's state machine */
      
    switch( getState() ) {				
    case Initialisation:
      if (lastState != getState()) {
	initLeds();
	IO_PORTS_8(PORTB) &= ~ 0x01; // led  0         : ON
	IO_PORTS_8(PORTB) |=   0x0E; // leds 1, 2, 3   : OFF
	MSG_WAR(0X3F10, "Entering in INITIALISATION mode ", 0);
      }
      initialisation();
      /* change automatically into pre_operational state */ 
      lastState = Initialisation;
      setState(Pre_operational);
      break;
					
    case Pre_operational:
      if (lastState != getState()) {
	IO_PORTS_8(PORTB) &= ~ 0x03; // leds 0, 1      : ON
	IO_PORTS_8(PORTB) |=   0x0C; // leds 2, 3      : OFF
	MSG_WAR(0X3F11, "Entering in PRE_OPERATIONAL mode ", 0);
	initPreOperationalMode();
      }
      preOperational();
      if (lastState == Initialisation)
	slaveSendBootUp(0);
      lastState = Pre_operational;
      break;
					
    case Operational:
      if (lastState != getState()) {
	IO_PORTS_8(PORTB) &= ~ 0x07; // leds 0, 1, 2   : ON
	IO_PORTS_8(PORTB) |=   0x08; // leds 3         : OFF
	MSG_WAR(0X3F12, "Entering in OPERATIONAL mode ", 0);
      }
      operational();    
      lastState = Operational;	
      break;
	  		
    case Stopped:
      if (lastState != getState()) {
	IO_PORTS_8(PORTB) |=   0x0F; // leds 0, 1, 2, 3 : OFF
	MSG_WAR(0X3F13, "Entering in  STOPPED mode", 0);
      }
      stopped();
      lastState = Stopped;
      break;
    }//end switch case	

  }
  return (0); 
}
 
