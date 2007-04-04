#ifndef UNIX_H_
#define UNIX_H_

#include "timerscfg.h"
#include "can_driver.h"
#include "data.h"
#ifdef WIN32
#include <windows.h>
typedef HINSTANCE LIB_HANDLE;
#else
#include <dlfcn.h>
typedef void* LIB_HANDLE;
#endif

UNS8 UnLoadCanDriver(LIB_HANDLE handle);
LIB_HANDLE LoadCanDriver(char* driver_name);
UNS8 canSend(CAN_PORT port, Message *m);
CAN_PORT canOpen(s_BOARD *board, CO_Data * d);
int canClose(CAN_PORT port);

#endif /*UNIX_H_*/
