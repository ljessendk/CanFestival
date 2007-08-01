#ifndef CALLBACK_H_
#define CALLBACK_H_

extern "C" 
  {
	#include "ObjDict.h"
  }

void Call_heartbeatError(UNS8);

UNS8 Call_canSend(Message *);

void Call_initialisation(void);
void Call_preOperational(void);
void Call_operational(void);
void Call_stopped(void);

void Call_post_sync(void);
void Call_post_TPDO(void);
void Call_storeODSubIndex(UNS16 wIndex, UNS8 bSubindex);

#endif /*CALLBACK_H_*/
