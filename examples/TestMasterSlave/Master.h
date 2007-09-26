#include "TestMaster.h"

void TestMaster_heartbeatError(UNS8);

UNS8 TestMaster_canSend(Message *);

void TestMaster_initialisation(void);
void TestMaster_preOperational(void);
void TestMaster_operational(void);
void TestMaster_stopped(void);

void TestMaster_post_sync(void);
void TestMaster_post_TPDO(void);
void TestMaster_post_emcy(UNS8 nodeID, UNS16 errCode, UNS8 errReg);
