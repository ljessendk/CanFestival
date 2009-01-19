#ifndef UNIX_H_
#define UNIX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "timerscfg.h"
#include "can_driver.h"
#include "data.h"
#include "timers_driver.h"

#ifndef __KERNEL__
#include <dlfcn.h>
#endif

typedef void* LIB_HANDLE;

/** @defgroup userapi User API */

/** @defgroup can CAN management
 *  @ingroup userapi
 */

/**
 * @ingroup can
 * @{
 */

/**
 * @ingroup can
 * @brief Unload CAN driver interface
 * @param handle The library handle
 * @return 0 if succes
 */
UNS8 UnLoadCanDriver(LIB_HANDLE handle);

/**
 * @ingroup can
 * @brief Load CAN driver interface
 * @param *driver_name The location of the library to load
 * @return handle The library handle
 */
LIB_HANDLE LoadCanDriver(const char* driver_name);

/**
 * @brief Send a CAN message
 * @param port CanFestival file descriptor
 * @param *m The message to send
 * @return 0 if succes
 */
UNS8 canSend(CAN_PORT port, Message *m);

/**
 * @ingroup can
 * @brief Open a CANopen device
 * @param *board Pointer on the board structure that contains busname and baudrate 
 * @param *d Pointer on the CAN object data structure
 * @return port CanFestival file descriptor
 */
CAN_PORT canOpen(s_BOARD *board, CO_Data * d);

/**
 * @ingroup can
 * @brief Close a CANopen device
 * @param *d Pointer on the CAN object data structure
 * @return 0 if succes
 */
int canClose(CO_Data * d);

/**
 * @ingroup can
 * @brief Change the CANopen device baudrate 
 * @param port CanFestival file descriptor 
 * @param *baud The new baudrate to assign
 * @return 0 if succes
 */
UNS8 canChangeBaudRate(CAN_PORT port, char* baud);
/** @} */


#ifdef __cplusplus
};
#endif

#endif /*UNIX_H_*/
