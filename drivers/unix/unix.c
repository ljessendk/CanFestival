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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef NOT_USE_DYNAMIC_LOADING
#define DLL_CALL(funcname) (* funcname##_driver)
#define FCT_PTR_INIT =NULL

#define DLSYM(name)\
	*(void **) (&name##_driver) = dlsym(handle, #name"_driver");\
	if ((error = dlerror()) != NULL)  {\
		fprintf (stderr, "%s\n", error);\
		UnLoadCanDriver(handle);\
		return NULL;\
	}

#else /*NOT_USE_DYNAMIC_LOADING*/

/*Function call is direct*/
#define DLL_CALL(funcname) funcname##_driver

#endif /*NOT_USE_DYNAMIC_LOADING*/

#include "data.h"
#include "canfestival.h"
#include "timers_driver.h"

#define MAX_NB_CAN_PORTS 16

typedef struct {
  char used;
  CAN_HANDLE fd;
  TASK_HANDLE receiveTask;
  CO_Data* d;
} CANPort;

#include "can_driver.h"

/*Declares the funtion pointers for dll binding or simple protos*/
/*UNS8 DLL_CALL(canReceive)(CAN_HANDLE, Message *);
UNS8 DLL_CALL(canSend)(CAN_HANDLE, Message *);
CAN_HANDLE DLL_CALL(canOpen)(s_BOARD *);
int DLL_CALL(canClose)(CAN_HANDLE);
*/
CANPort canports[MAX_NB_CAN_PORTS] = {{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,}};

#ifndef NOT_USE_DYNAMIC_LOADING

/*UnLoads the dll*/
UNS8 UnLoadCanDriver(LIB_HANDLE handle)
{
	if(handle!=NULL)
	{
		dlclose(handle);

		handle=NULL;
		return 0;
	}
	return -1;
}

/*Loads the dll and get funcs ptr*/
LIB_HANDLE LoadCanDriver(char* driver_name)
{
	LIB_HANDLE handle = NULL;
	char *error;
	

	if(handle==NULL)
	{
		handle = dlopen(driver_name, RTLD_LAZY);
	}

	if (!handle) {
		fprintf (stderr, "%s\n", dlerror());
        	return NULL;
	}
 
	/*Get function ptr*/
	DLSYM(canReceive)
	DLSYM(canSend)
	DLSYM(canOpen)
	DLSYM(canClose)

	return 0;
}

#endif



/*Not needed -- canReceiveLoop calls _canReceive directly *//*
UNS8 canReceive(CAN_PORT port, Message *m)
{
	return DLL_CALL(canReceive)(port->fd, Message *m);
}
*/

UNS8 canSend(CAN_PORT port, Message *m)
{
	if(port){
		UNS8 res;
	        //LeaveMutex();
		res = DLL_CALL(canSend)(((CANPort*)port)->fd, m);
		//EnterMutex();
		return res;
	}               
	return -1;
}

void canReceiveLoop(CAN_PORT port)
{
       Message m;

       while (1) {
               if (DLL_CALL(canReceive)(((CANPort*)port)->fd, &m) != 0)
                       break;

               EnterMutex();
               canDispatch(((CANPort*)port)->d, &m);
               LeaveMutex();
       }
}
CAN_PORT canOpen(s_BOARD *board, CO_Data * d)
{
	int i;
	for(i=0; i < MAX_NB_CAN_PORTS; i++)
	{
		if(!canports[i].used)
		break;
	}
	
#ifndef NOT_USE_DYNAMIC_LOADING
	if (&DLL_CALL(canOpen)==NULL) {
        	fprintf(stderr,"CanOpen : Can Driver dll not loaded\n");
        	return NULL;
	}
#endif	
	CAN_HANDLE fd0 = DLL_CALL(canOpen)(board);

	canports[i].used = 1;
	canports[i].fd = fd0;
	canports[i].d = d;

	CreateReceiveTask(&(canports[i]), &canports[i].receiveTask, &canReceiveLoop);
	
	EnterMutex();
	d->canHandle = (CAN_PORT)&canports[i];
	LeaveMutex();
	return (CAN_PORT)&canports[i];
}

int canClose(CO_Data * d)
{
	EnterMutex();
	((CANPort*)d->canHandle)->used = 0;
	CANPort* tmp = (CANPort*)d->canHandle;
	d->canHandle = NULL;
	LeaveMutex();
	
	int res = DLL_CALL(canClose)(tmp->fd);
	
	WaitReceiveTaskEnd(tmp->receiveTask);
	return res;
}
