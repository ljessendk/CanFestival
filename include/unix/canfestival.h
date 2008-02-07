#ifndef UNIX_H_
#define UNIX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "timerscfg.h"
#include "can_driver.h"
#include "data.h"
#include "timers_driver.h"

#include <dlfcn.h>
typedef void* LIB_HANDLE;

UNS8 UnLoadCanDriver(LIB_HANDLE handle);
LIB_HANDLE LoadCanDriver(char* driver_name);
UNS8 canSend(CAN_PORT port, Message *m);
CAN_PORT canOpen(s_BOARD *board, CO_Data * d);
int canClose(CO_Data * d);
UNS8 canChangeBaudRate(CAN_PORT port, char* baud);

#ifdef __cplusplus
};
#endif

#endif /*UNIX_H_*/
