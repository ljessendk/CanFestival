/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Francois Beaulier

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
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "slavedic.h"
#include "canfestival.h"


static s_BOARD SlaveBoard0 = {"0", ""};
static char Run;

void display_usage(char *prog)
{
	printf("usage: %s itf nodeid\n", prog); 
    printf("    itf is the can interface\n");
    printf("    nodeid is the canopen node id from 1 to 127\n");
    printf("Ex: %s can0 12\n", prog);
}

/* A callback called when position is written */
UNS32 callback_on_position(CO_Data* d, const indextable *idxtab , UNS8 bSubindex)
{
    printf("position have been set to %d\n", position);
	return 0;
}

/* A callback called when node state changes */
void state_change(CO_Data* d)
{
    if(d->nodeState == Initialisation)
        printf("Node state is now  : Initialisation\n");
    else if(d->nodeState == Disconnected)
        printf("Node state is now  : Disconnected\n");
    else if(d->nodeState == Connecting)
        printf("Node state is now  : Connecting\n");
    else if(d->nodeState == Preparing)
        printf("Node state is now  : Preparing\n");
    else if(d->nodeState == Stopped)
        printf("Node state is now  : Stopped\n");
    else if(d->nodeState == Operational)
        printf("Node state is now  : Operational\n");
    else if(d->nodeState == Pre_operational)
        printf("Node state is now  : Pre_operational\n");
    else if(d->nodeState == Unknown_state)
        printf("Node state is now  : Unknown_state\n");
    else
        printf("Error : unexpected node state\n");
}

void Exit(CO_Data* d, UNS32 id)
{
	setState(&slavedic_Data, Stopped);
    printf("Program terminating\n");
}

/*--- handler on SIGINT (CTL-C) signal ---*/
void sortie(int sig)
{
	Run = 0;
}

int main(int argc,char **argv)
{
	struct sigaction act;
	uint8_t nodeid = 2;

    // register handler on SIGINT signal 
    act.sa_handler=sortie;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGINT,&act,0);

    // Check that we have the right command line parameters
    if(argc != 3){
        display_usage(argv[0]);
        exit(1);
    }

    // get command line parameters
    nodeid = strtoul(argv[2], NULL, 10);
    SlaveBoard0.busname = argv[1];
    printf("Starting on %s with node id = %u\n", SlaveBoard0.busname, nodeid);

    // register the callbacks we use
    RegisterSetODentryCallBack(&slavedic_Data, 0x2001, 0, callback_on_position);
    slavedic_Data.initialisation=state_change;
    slavedic_Data.preOperational=state_change;
    slavedic_Data.operational=state_change;
    slavedic_Data.stopped=state_change;

	// Init Canfestival
	if (LoadCanDriver("./libcanfestival_can_socket.so") == NULL){
	    printf("Unable to load driver library\n");
        printf("please put file libcanfestival_can_socket.so in the current directory\n");
        exit(1);
    }
	if(!canOpen(&SlaveBoard0,&slavedic_Data)){
		printf("Cannot open can interface %s\n",SlaveBoard0.busname);
		exit(1);
	}

    TimerInit();
	setNodeId(&slavedic_Data, nodeid);
	setState(&slavedic_Data, Initialisation);

    printf("Canfestival initialisation done\n");
	Run = 1;
	while(Run)
	{
        sleep(1);
        EnterMutex();
        counter += nodeid;
        LeaveMutex();
	}

	// Stop timer thread
	StopTimerLoop(&Exit);
	// Close CAN devices (and can threads)
	canClose(&slavedic_Data);
	return 0;

}




