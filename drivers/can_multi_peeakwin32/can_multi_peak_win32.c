/*
  This file is part of CanFestival, a library implementing CanOpen Stack. 

  Copyright (C): Jaroslav Fojtik
*/

#if defined(WIN32) && !defined(__CYGWIN__)
#define usleep(micro) Sleep(micro%1000 ? (micro/1000) + 1 : (micro/1000))
#else
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#endif


#include "cancfg.h"
#include "can_driver.h"
#include "def.h"

UNS8 LIBAPI canSend_driver(CAN_HANDLE fd0, Message const *m);


#define VERSION_2

/* dummy implementation for older version. */
#ifndef VERSION_2
void CAN_SetRcvEvent(HANDLE hEventx)
{
  SetEvent(hEventx);
}
#endif


#define SLAVE_COUNT 10
#define QueueSize 100


#ifndef extra_PCAN_init_params
    #define extra_PCAN_init_params  /**/
#else
    long int print_getenv(const char* pcanparam)
    {
        char* param=NULL;
        long int res=0;

        param = getenv(pcanparam);
        if(param != NULL){
            res = strtol(param,NULL,0);
        }
        else
            printf("Environment variable %s not defined !\n", pcanparam);
        printf("Found environment variable %s : %ld\n", pcanparam ,res);
        return res;
    }
    #define extra_PCAN_init_params\
        ,print_getenv("PCANHwType")\
        ,print_getenv("PCANIO_Port")\
        ,print_getenv("PCANInterupt")
#endif



typedef struct 
{
  s_BOARD *board;
  Message MQueue[QueueSize];
  unsigned QStart, QEnd;
  HANDLE hEventx;
} QueueRecord;

int initialisedQ = 0;
QueueRecord Q_DATA[10];


/** Store message into a queue. */
static void PushMsgToQueue(QueueRecord *QR, Message *m)
{
  if(QR==NULL || m==NULL) return;
  if(QR->board==NULL) return;							// No Board assigned yet
  memcpy(&QR->MQueue[QR->QStart], m, sizeof(Message));
  QR->QStart = (QR->QStart + 1) % QueueSize;
  if(QR->QEnd==QR->QStart) QR->QEnd = (QR->QEnd+1) % QueueSize;
  if(QR->hEventx) SetEvent(QR->hEventx);					// Signalise internal flag
}


/** Get message from a queue. */
static int PopMsgFromQueue(QueueRecord *QR, Message *m)
{
  if(QR==NULL || m==NULL) return 0;
  if(QR->QEnd == QR->QStart) return 0;

  memcpy(m, &QR->MQueue[QR->QEnd], sizeof(Message));
  QR->QEnd = (QR->QEnd+1) % QueueSize;
  return 1;
}


/** Create the Event for the first board */
HANDLE hEvent1 = NULL;
CRITICAL_SECTION InitLock;


// Define for rtr CAN message
#define CAN_INIT_TYPE_ST_RTR MSGTYPE_STANDARD | MSGTYPE_RTR


/***************************************************************************/
static int TranslateBaudeRate(char* optarg)
{
  if(!strcmp( optarg, "1M")) return CAN_BAUD_1M;
  if(!strcmp( optarg, "500K")) return CAN_BAUD_500K;
  if(!strcmp( optarg, "250K")) return CAN_BAUD_250K;
  if(!strcmp( optarg, "125K")) return CAN_BAUD_125K;
  if(!strcmp( optarg, "100K")) return CAN_BAUD_100K;
  if(!strcmp( optarg, "50K")) return CAN_BAUD_50K;
  if(!strcmp( optarg, "20K")) return CAN_BAUD_20K;
  if(!strcmp( optarg, "10K")) return CAN_BAUD_10K;
  if(!strcmp( optarg, "5K")) return CAN_BAUD_5K;
  if(!strcmp( optarg, "none")) return 0;
  return 0x0000;
}


static UNS8 canInit(s_BOARD *board)
{
int baudrate;
int ret = 0;

  if(hEvent1==NULL)
  {	//Create the Event for the first board
    hEvent1 = CreateEvent(NULL, // lpEventAttributes
                        FALSE,  // bManualReset
                        FALSE,  // bInitialState
                        "");    // lpName
    InitializeCriticalSection(&InitLock);
  }

  EnterCriticalSection(&InitLock);  

  if(baudrate = TranslateBaudeRate(board->baudrate))
  {
    ret = CAN_Init(baudrate, CAN_INIT_TYPE_ST extra_PCAN_init_params);
    if(ret != CAN_ERR_OK) 
    {
      LeaveCriticalSection(&InitLock);      
      return 0;
    }
  }
        
  CAN_SetRcvEvent(hEvent1);  //Set Event Handler for CANReadExt
  LeaveCriticalSection(&InitLock);    
  return 1;
}


/********* functions which permit to communicate with the board ****************/
UNS8 LIBAPI canReceive_driver(CAN_HANDLE fd0, Message *m)
{
static int HeavyCounter = 0;
int ret=0;
UNS8 data;
TPCANMsg peakMsg;
DWORD Res;
DWORD result;
HANDLE hh[2];
int i;

#ifdef CAN_READ_EX
    TPCANTimestamp peakRcvTime;
#endif      

  i = strtol(((s_BOARD *)fd0)->busname,NULL,0);
  if(i>=SLAVE_COUNT || i<0) return 1;           // error
  if(Q_DATA[i].board!=(s_BOARD *)fd0) return 1;      // error
  
  hh[0]=hEvent1; hh[1]=Q_DATA[i].hEventx;
  
	// loop until valid message or fatal error
  do
  {
CONTINUE:
     if(PopMsgFromQueue(&Q_DATA[i],m)) return 0;	//message is waiting in the internal queue

        // We read the queue looking for messages.
#ifdef VERSION_2
     result = WaitForMultipleObjects(2,hh,FALSE,15);  
     if(Q_DATA[i].board==NULL) return 1;		//exit hook, exit immediatelly when device is closed

     if(result == WAIT_OBJECT_0+1)
	 goto CONTINUE;	//look to a PopMsgFromQueue() (continue will check while(), goto skips it)

     if(result==WAIT_OBJECT_0 || result==WAIT_TIMEOUT)
     {
#endif     
#ifdef CAN_READ_EX
       Res = CAN_ReadEx(&peakMsg, &peakRcvTime);
#else
       Res = CAN_Read(&peakMsg);
#endif
                // Exit receive thread when handle is no more valid
#ifdef CAN_ERRMASK_ILLHANDLE
       if(Res & CAN_ERRMASK_ILLHANDLE) return 1;
#else
       if(Res & CAN_ERR_ILLHANDLE) return 1;
#endif

#ifndef VERSION_2
       if(Res != CAN_ERR_OK) 
             result = WaitForSingleObject(hEvent1, 1);   //pooling for pcan release<2
#endif
	if(Res==CAN_ERR_QRCVEMPTY) goto CONTINUE;
#ifdef VERSION_2       
     }
     else
     {
       //if(result==WAIT_TIMEOUT || result==(WAIT_OBJECT_0+1)) 
       //     Res = CAN_ERR_BUSLIGHT;
       //else 
              Res = CAN_ERR_UNKNOWN;
     }
#endif

    if(Res==CAN_ERR_BUSHEAVY)
    {
      if(HeavyCounter++>10) 
      {
	HeavyCounter = 0;
	Res=CAN_ERR_BUSOFF;
      }
    }

    if(Res & CAN_ERR_BUSOFF)
    {      
      peakMsg.MSGTYPE = MSGTYPE_STATUS;
      peakMsg.DATA[3] = CAN_ERR_BUSOFF;
      Res = CAN_ERR_OK;
    }   

        // A message was received : we process the message(s)
    if(Res == CAN_ERR_OK)
    {
            // if something different that 11bit or rtr... problem      
	switch(peakMsg.MSGTYPE)
	{
	  case MSGTYPE_STATUS:
		  switch(peakMsg.DATA[3])
	          {
		    case CAN_ERR_BUSHEAVY: 
				      break;
	            case CAN_ERR_BUSOFF: 
			              printf ("Peak board read BUSOFF: re-init!!!\n");
				      canInit((s_BOARD*)fd0);
				      usleep(2000);
				      break;
		  }
		  return peakMsg.DATA[3];	/* if something different that 11bit or rtr... problem */

	  case MSGTYPE_STANDARD:		/* bits of MSGTYPE_ */
	  case MSGTYPE_EXTENDED:
			  m->rtr = 0;
			  break;

	  case MSGTYPE_RTR:			/* bits of MSGTYPE_ */
			  m->rtr = 1;
			  break;

	  default: return CAN_ERR_OVERRUN;	/* If status, return status if 29bit, return overrun. */
		    
	}
   
      m->cob_id = peakMsg.ID;

      if (peakMsg.MSGTYPE == CAN_INIT_TYPE_ST)  /* bits of MSGTYPE_ */
                m->rtr = 0;
      else
                m->rtr = 1;
      m->len = peakMsg.LEN; /* count of data bytes (0..8) */
      for(data=0; data<peakMsg.LEN; data++)
                m->Data[data] = peakMsg.DATA[data]; /* data bytes, up to 8 */
#if defined DEBUG_MSG_CONSOLE_ON
      MSG("in : ");
      print_message(m);
#endif
    }
    else
    {       // not benign error => fatal error
      if(!(Res & CAN_ERR_QRCVEMPTY
                    || Res & CAN_ERR_BUSLIGHT
                    || Res & CAN_ERR_BUSHEAVY))
      {
        printf ("canReceive returned error (%d)\n", Res);
        return 1;
      }
    }
  } while(Res != CAN_ERR_OK);


     // populate message received to other drivers
  for(i=0; i<SLAVE_COUNT; i++)
  {
    if(Q_DATA[i].board != (s_BOARD *)fd0)	// do not populate to own queue
    {
      PushMsgToQueue(&Q_DATA[i],m);
    }   
  }

  return 0;
}


/***************************************************************************/
UNS8 LIBAPI canSend_driver(CAN_HANDLE fd0, Message const *m)
{
UNS8 data;
TPCANMsg peakMsg;
int i, j;
int loc_errno;
int MaxLoops = 100;

  i = -1;
  for(j=0; j<SLAVE_COUNT; j++)
  {
    if(Q_DATA[j].board != (s_BOARD *)fd0)   // store this message forr all other drivers
    {
      PushMsgToQueue(&Q_DATA[j],m);
      i = j;
    }
  }

  if(i<0) return 1;                         // no board found

  peakMsg.ID = m->cob_id;                   /* 11/29 bit code */
  if(m->rtr == 0)
  {
    if(peakMsg.ID > 0x7FF)
	peakMsg.MSGTYPE = MSGTYPE_EXTENDED; /* bits of MSGTYPE_ */
    else
	peakMsg.MSGTYPE = MSGTYPE_STANDARD; /* bits of MSGTYPE_ */
  }
  else	
    peakMsg.MSGTYPE = MSGTYPE_RTR;	    /* bits of MSGTYPE_ */
  
  peakMsg.LEN = m->len;
    /* count of data bytes (0..8) */
  for(data = 0; data < m->len; data++)
        peakMsg.DATA[data] = m->Data[data]; /* data bytes, up to 8 */

  do 
  {
    errno = loc_errno = CAN_Write(&peakMsg);

    if(loc_errno)
    {
      if(loc_errno==CAN_ERR_BUSOFF && (MaxLoops%20)==1)
      {
#if defined DEBUG_MSG_CONSOLE_ON
        printf ("Peak board write: re-init!!!\n");
#endif
        canInit((s_BOARD*)fd0);
        usleep(1000);
      }
      usleep(80);
    }
    if(MaxLoops-- == 0) break;			// limit max looping
  } while(loc_errno != CAN_ERR_OK);

#if defined DEBUG_MSG_CONSOLE_ON
    MSG("out : ");
    print_message(m);
#endif
    return 0;
}


/***************************************************************************/
UNS8 LIBAPI canChangeBaudRate_driver(CAN_HANDLE fd, char* baud)
{
    printf("canChangeBaudRate not yet supported by this driver\n");
    return 0;
}


/***************************************************************************/
LIBPUBLIC CAN_HANDLE LIBAPI canOpen_driver(s_BOARD * board)
{
int ret;
int i;

  if(!initialisedQ)
  {
    memset(Q_DATA,0,sizeof(Q_DATA));
    initialisedQ = 1;
  }

  i = strtol(board->busname,NULL,0);			// Get slot name
  //printf ("Board Busname=%d.\n",strtol(board->busname, &pEnd,0));
  if(i<SLAVE_COUNT && i>=0)
  {
    Q_DATA[i].board = board;    
      //printf ("First Board selected\n");
    if(Q_DATA[i].hEventx==NULL)				// Create local event
    {
      Q_DATA[i].hEventx = CreateEvent(NULL, FALSE, FALSE, "");
    }

    if(hEvent1!=NULL) return (CAN_HANDLE)board;		// Create global event, if needed

    ret = canInit(board);
    if(ret)
      return (CAN_HANDLE)board;
  }

  return NULL;
}


/***************************************************************************/
int LIBAPI canClose_driver(CAN_HANDLE fd0)
{
s_BOARD *x_board = NULL;
int ActiveBoards = 0;
int i;

  if((s_BOARD *)fd0==NULL) return 0;
  for(i=0; i<SLAVE_COUNT; i++)
  {
    if(Q_DATA[i].board == (s_BOARD *)fd0)
    {
      x_board = Q_DATA[i].board;
      Q_DATA[i].board = NULL;
      CloseHandle(Q_DATA[i].hEventx);
      Q_DATA[i].hEventx = NULL;
    }
    else
      ActiveBoards++;
  }

  if(ActiveBoards<=0)
  {				// No can device is used.
    CAN_SetRcvEvent(NULL);
    CAN_Close();    
    if(hEvent1) 
    {
      SetEvent(hEvent1);
      CloseHandle(hEvent1);
      hEvent1 = NULL;
    }
  }
  else    
    SetEvent(hEvent1);

  return 0;
}






