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

#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "update_firmware.h"
#include "canfestival.h"



CO_Data* pObjData = &update_firmware_ObjDictData;

static s_BOARD SlaveBoard0 = {"0", ""};
static char Run;

void display_usage(char *prog)
{
    printf("usage: %s itf nodeid\n", prog); 
    printf("    itf is the can interface\n");
    printf("    nodeid is the canopen node id from 1 to 127\n");
    printf("Ex: %s can0 12\n", prog);
}


/* A callback called when node state changes */
void state_change(CO_Data *d)
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

void Exit(CO_Data *d, UNS32 id)
{
    setState(pObjData, Stopped);
    printf("Program terminating\n");
}

UNS32 userOnWrDomainInd(CO_Data *d, UNS16 wIndex, UNS8 bSubindex, UNS32 offset, UNS32 nbBytes, UNS8* data)
{
    printf("offset: %d\n", offset);
    for (UNS32 i = 0; i < nbBytes; ++i)
    {
        printf("%d ", data[i]);
    }
    printf("\n");

    return 0;
}


/*--- handler on SIGINT (CTL-C) signal ---*/
void stopHandler(int sig)
{
    Run = 0;
}

int main(int argc,char **argv)
{
    // register handler on SIGINT signal 
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    // Check that we have the right command line parameters
    if(argc != 3)
    {
        display_usage(argv[0]);
        exit(1);
    }

    // register domain write callback
    pObjData->onWrDomainInd = userOnWrDomainInd;

    // get command line parameters
    UNS8 nodeid = strtoul(argv[2], NULL, 10);
    SlaveBoard0.busname = argv[1];
    printf("Starting on %s with node id = %u\n", SlaveBoard0.busname, nodeid);

    // register the callbacks
    pObjData->initialisation = state_change;
    pObjData->preOperational = state_change;
    pObjData->operational = state_change;
    pObjData->stopped = state_change;


    if(!canOpen(&SlaveBoard0, pObjData))
    {
        printf("Cannot open can interface %s\n",SlaveBoard0.busname);
        exit(1);
    }

    TimerInit();
    setNodeId(pObjData, nodeid);
    setState(pObjData, Initialisation);

    printf("Canfestival initialisation done\n");
    Run = 1;
    while(Run)
    {
        sleep(1);
    }

    // Stop timer thread
    StopTimerLoop(&Exit);
    // Close CAN devices (and can threads)
    canClose(pObjData);
    return 0;
}




