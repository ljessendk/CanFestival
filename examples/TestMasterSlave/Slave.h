#include "TestSlave.h"

void TestSlave_SDOtimeoutError(UNS8 line);
void TestSlave_heartbeatError(UNS8);

UNS8 TestSlave_canSend(Message *);

void TestSlave_initialisation(void);
void TestSlave_preOperational(void);
void TestSlave_operational(void);
void TestSlave_stopped(void);

void TestSlave_post_sync(void);
void TestSlave_post_TPDO(void);
void TestSlave_storeODSubIndex(UNS16 wIndex, UNS8 bSubindex);
