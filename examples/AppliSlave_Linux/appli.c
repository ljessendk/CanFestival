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
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include <applicfg.h>
#include <timerhw.h>
#include <linuxCan.h>

#include "def.h"
#include "can.h"
#include "canOpenDriver.h"
#include "sdo.h"
#include "pdo.h"
#include "init.h"
#include "timer.h"
#include "lifegrd.h"

#include "nmtSlave.h"

// Adlink 7841 or Peak PCI/CAN board
// ---------------------------------

// Baudrate values for Peak board :
// CAN_BAUD_1M CAN_BAUD_500K CAN_BAUD_250K CAN_BAUD_125K CAN_BAUD_100K CAN_BAUD_50K
// CAN_BAUD_20K CAN_BAUD_10K CAN_BAUD_5K

#ifdef CAN_BAUD_250K
# define BAUDRATE CAN_BAUD_250K
#else
// Appli have been compiled for Adlink-arbraca. Baudrate not used
# define BAUDRATE 0
#endif

s_BOARD board = {"1", BAUDRATE};


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
struct tm date;
struct tm *ptrdate;
struct tm date_prec;
time_t  tme;
UNS8 lastSecond;
e_nodeState lastState;
pthread_t threadRcvMsg;
pthread_t threadHeartbeatAndSDO;
UNS8 sendingError = 0;


/******************************prototypes*****************************/
/* You *must* have these 2 functions in your code*/
void heartbeatError(UNS8 heartbeatID);
void SD0timeoutError(UNS8 bus_id, UNS8 line);

UNS8 scanSDOtimeout(void);
void preOperational(void);
void operational(void); 
void stopped(void);
void waitMessage(void);
void waitMessage_heartbeat(void);

/*********************************************************************/
void heartbeatError(UNS8 heartbeatID)
{
  MSG_ERR(0x1F00, "!!! No heart beat received from node : ", heartbeatID);
}

/*****************************************************************************/
void SD0timeoutError (UNS8 bus_id, UNS8 line)
{
  // Informations on what occurs are in transfers[bus_id][line]....
  // See scanSDOtimeout() in sdo.c
}

/*********************************** THREADS **********************************/
//------------------------------------------------------------------------------
// Wait for a received message
void waitMessage( void )
{       
  while (1) {
    receiveMsgHandler(0); // blocked until new message
  }
}

//------------------------------------------------------------------------------
// Heartbeat Manager (sending and receiving) and test SDO timeout
void heartbeatAndSDO(void)
{
  while (1) {
    heartbeatMGR();
    // Check if some SDO response are missing
    scanSDOtimeout();
    // Sleep 10 ms
    usleep(10000);	
  }
}

/*********************************************************************/
void preOperational( void )
{	
  /* Init some variables */
  canopenErrNB = 0;
  canopenErrVAL = 0;
}


/********************************************************************/
void operational( void )
{
  /* read systeme date */
  tme = time (&tme);
  ptrdate = gmtime(&tme);
  date = *ptrdate;
  
  /* Update the dictionary */
  day = date.tm_mday;
  hours = date.tm_hour;
  minutes = date.tm_min;
  seconds = date.tm_sec;
  

  if ( date_prec.tm_min != date.tm_min ) {
    MSG_WAR(0x3F00, "event : minutes change -> node decides to send it. Value : ", date.tm_min);
    sendPDOevent( 0, &minutes );
    date_prec = date;
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
}

void help()
{
  printf("**************************************************************\n");
  printf("*  AppliSlave                                                *\n");
  printf("*                [-b b]                                      *\n");
  printf("*                                                            *\n");
  printf("*     b : bus [default 1]                                    *\n");
  printf("*                                                            *\n");
  printf("*  This exemple run AppliSlave on bus 0                      *\n");
  printf("*   AppliSlave -b 0                                          *\n");
  printf("*                                                            *\n");
  printf("**************************************************************\n");
}

/****************************************************************************/
/***************************  MAIN  *****************************************/
/****************************************************************************/
int main(int argc,char **argv)
{

  HANDLE ok;
  UNS32 *    pSize;
  UNS32      size;
  pSize = &size;
  char c;
  extern char *optarg;

  while ((c = getopt(argc, argv, "-b:")) != EOF)
  {
    switch(c)
    {
      case 'b' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        board.busname = optarg;
        break;
      default:
        help();
        exit(1);
    }
  }

  // Global initialization before launching the threads. (also done in init mode.
  /* Defining the node Id */
  setNodeId(0x05);
  MSG_WAR(0x3F06, "My node ID is : ", getNodeId()); 
  initCANopenMain();
  initTimer( );
  heartbeatInit();
  initResetMode();

  /* Launch the thread to receive the messages */
  pthread_create( &threadRcvMsg, NULL, (void *)&waitMessage, NULL); 
  /* Launch the thread to manage the heartbeat */
  pthread_create( &threadHeartbeatAndSDO, NULL, (void *)&heartbeatAndSDO, NULL); 

  /* open the communication with the board */
  ok = f_can_open(& board);
  if (ok == NULL) {
    MSG_ERR(0x1F02,"Unable to open the board", 0);
    MSG_ERR(0x1F03,"Edit includeMakefileLinux to verify that the application is configured for the good board", 0);
    exit (-1);
  }
  else {
    MSG_WAR(0x3F03, "Board 1 opened ", 0);
    /* slave's state initialization */
    setState(Initialisation);
    lastState = Unknown_state;


    while(1) { /* slave's state machine */
      switch( getState() ) {	
      case Initialisation:
	if (lastState != getState()) 
	  MSG_WAR(0X3F05, "I am in INITIALISATION mode ", 0);
        /* Defining the node Id */
	setNodeId(0x05);
	MSG_WAR(0x3F06, "My node ID is : ", getNodeId()); 
	  // Node identity ?
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

	initCANopenMain();
	initTimer( );
	heartbeatInit();
	initResetMode();
	/* the slave send an NMT trame to say to the master 
	   that it is going to enter into operational state 
	   In fact, you must send the boot-up when you are in 
	   operational mode !
	*/
  
	/* change automatically into pre_operational state */ 
	lastState = getState();
	setState(Pre_operational);
	break;
					
      case Pre_operational:
	if (lastState != getState())
	  {
	  MSG_WAR(0X3F11, "I am in PRE_OPERATIONAL mode ", 0);
	  // Some stuff to do when the node enter in pre-operational mode
	  initPreOperationalMode();
	  }
	if (lastState == Initialisation)
	  slaveSendBootUp(0);
	lastState = getState();
	preOperational( );	
	break;
					
      case Operational:
	if (lastState != getState())
	  MSG_WAR(0X3F12, "I am in OPERATIONAL mode ", 0);
	lastState = getState();
	operational( );
	break;
			
      case Stopped:
	if (lastState !=  getState())
	  MSG_WAR(0X3F13, "I am in STOPPED mode", 0);
	lastState = getState();
	stopped( );
	break;
      }//end switch case
      // Sleep 10 ms
      usleep(10000);	
    }//end while
  }//end else

 
  return 0;	
}

