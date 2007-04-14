#include "TestMaster.h"

void TestMaster_SDOtimeoutError(UNS8 line);
void TestMaster_heartbeatError(UNS8);

UNS8 TestMaster_canSend(Message *);

void TestMaster_initialisation(void);
void TestMaster_preOperational(void);
void TestMaster_operational(void);
void TestMaster_stopped(void);

void TestMaster_post_sync(void);
void TestMaster_post_TPDO(void);
