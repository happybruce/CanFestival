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
#include <string.h>
#include <signal.h>
#include <stdbool.h>


#include "masterdic.h"
#include "canfestival.h"
#include "gendcf.h"
#include "dcf.h"


CO_Data * pDictData = &masterdic_Data;

extern subindex masterdic_Index1F22[]; // TODO: need fix, masterdic_Index1F22 is const

UNS8 dcfdatas[DCF_MAX_NODE_ID][DCF_MAX_SIZE];

s_BOARD MasterBoard0 = {"0", ""};

static char Run;

e_nodeState gNewNodeState = Initialisation;

void setup_dcf(void)
{
    // uint8_t subidx;
    // uint8_t nbr_subidx = *(uint8_t *)(masterdic_Index1F22[0].pObject);
    // printf("setup_dcf : %u sub indexes to set\n", nbr_subidx);
    // dcf_read_in_file(DEVICE_DICT_NAME, dcfdatas);
    // dcf_data_display(dcfdatas);
    // for(subidx = 0 ; subidx < nbr_subidx ; subidx++)
    // {
    //     masterdic_Index1F22[subidx + 1].pObject = dcfdatas[subidx];
    //     masterdic_Index1F22[subidx + 1].size = DCF_MAX_SIZE;
    // }
}

void display_usage(char *prog)
{
    printf("usage: %s itf\n", prog); 
    printf("    itf is the can interface\n");
    printf("Ex: %s can0\n", prog);
}

void slave_bootup_callback(CO_Data* d, UNS8 nodeid)
{
    printf("Master : node %u bootup received\n",nodeid);
    check_and_start_node(d, nodeid);
}

void Exit(CO_Data* d, UNS32 id)
{
    masterSendNMTstateChange(pDictData, 0x0, NMT_Reset_Node);    
    setState(pDictData, Stopped);
}

void slave_state_change_callback(CO_Data* d, UNS8 nodeId, e_nodeState newNodeState)
{
    if(newNodeState == Initialisation)
        printf("Node %u state is now  : Initialisation\n", nodeId);
    else if(newNodeState == Disconnected)
        printf("Node %u state is now  : Disconnected\n", nodeId);
    else if(newNodeState == Connecting)
        printf("Node %u state is now  : Connecting\n", nodeId);
    else if(newNodeState == Preparing)
        printf("Node %u state is now  : Preparing\n", nodeId);
    else if(newNodeState == Stopped)
        printf("Node %u state is now  : Stopped\n", nodeId);
    else if(newNodeState == Operational)
        printf("Node %u state is now  : Operational\n", nodeId);
    else if(newNodeState == Pre_operational)
        printf("Node %u state is now  : Pre_operational\n", nodeId);
    else if(newNodeState == Unknown_state)
        printf("Node %u state is now  : Unknown_state\n", nodeId);
    else
        printf("Error : node %u unexpected state\n", nodeId);
}

void heartbeatTimeOut(CO_Data* d, UNS8 nodeid)
{
    slave_state_change_callback(d, nodeid, Disconnected);
}

const uint8_t SlaveNodeId = 1;

bool fInformed = false;
void SDOCallbackFunc(CO_Data* d, UNS8 nodeId)
{
    if (nodeId == SlaveNodeId)
    {
        fInformed = true;
        printf("==> informed!\n");
    }
}


/*--- handler on SIGINT (CTL-C) signal ---*/
void sortie(int sig)
{
    Run = 0;
}

int main(int argc,char **argv)
{
    struct sigaction act;

    // register handler on SIGINT signal 
    act.sa_handler=sortie;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGINT,&act,0);

    // Check that we have the right command line parameters
    if(argc != 2){
        display_usage(argv[0]);
        exit(1);
    }
    
    // Fill DCF entries in the OD with data
    setup_dcf();
    
    

    MasterBoard0.busname = argv[1];
    printf("Starting on %s with node id = 100\n", MasterBoard0.busname);

    // Set the master node id, we do not call setNodeId() because we do not need all the predefined connexion set
    // *(pDictData->bDeviceNodeId) = 100;
    setNodeId(pDictData, 100);

    // register the callbacks we use
    pDictData->post_SlaveBootup = slave_bootup_callback;
    pDictData->post_SlaveStateChange = slave_state_change_callback;
    pDictData->heartbeatError = heartbeatTimeOut;

    // if (LoadCanDriver("./libcanfestival_can_socket.so") == NULL){
    //     printf("Unable to load driver library\n");
    //     printf("please put file libcanfestival_can_socket.so in the current directory\n");
    //     exit(1);
    // }

    if(!canOpen(&MasterBoard0, pDictData))
    {
        printf("Cannot open can interface %s\n", MasterBoard0.busname);
        exit(1);
    }

    TimerInit();
    
    /* Put the master in Pre_operational mode, this will broadcast a comunication reset */
    setState(pDictData, Pre_operational);
    /* Now put it in Operational */
    setState(pDictData, Operational);
    
    printf("Master starting on %s\n", MasterBoard0.busname);
    Run = 1;
    int16_t expectedData = 1000;
    while(Run)
    {
        bool fReadDone = FALSE;
        bool fWRDone = FALSE;
        
        EnterMutex();
        printf("Slaves : counter 1 = %u, counter 2 = %u, counter 3 = %u\n",counter_1, counter_2, counter_3);
        position_1 += 1;
        position_2 += 2;
        position_3 += 3;
        sendOnePDOevent(pDictData, 0);
        sendOnePDOevent(pDictData, 1);
        sendOnePDOevent(pDictData, 2);
        LeaveMutex();

        sleep(3);
        
        if ( (pDictData->NMTable[SlaveNodeId] == Operational) || 
             (pDictData->NMTable[SlaveNodeId] == Pre_operational) )
        {
            uint8_t ret = writeNetworkDictCallBack(pDictData, SlaveNodeId, 
                                                   0x4003, 0, 2, int16, &expectedData, 
                                                   SDOCallbackFunc, 0);
            if (ret == 0)
            {
                while (fInformed == false)
                {
                    sleep(1);
                }

                fInformed = false; // reset

                UNS32 abort_code;
                uint8_t ret = getWriteResultNetworkDict(pDictData, 1, &abort_code);
                if (ret == SDO_FINISHED)
                {
                    printf("==> Write done\n");;
                }
                else
                {
                    printf("==> Write fail\n");
                }
            }
            else
            {
                printf("==> Call writeNetworkDictCallBack() fail\n");
            }
        }

    #if 0

        if (gNewNodeState == Operational || gNewNodeState == Pre_operational)
        {
            uint8_t ret = writeNetworkDict(pDictData, 1, 0x4003, 0, 2, int16, &expectedData, 0);

            if (ret == 0)
            {
                expectedData++;
                fWRDone = TRUE;
            }
            else
            {
                printf("==>Write Error: 0x%x\n", ret);
            }
        }

        if (fWRDone)
        {
            while (TRUE && Run)
            {
                UNS32 abort_code;
                uint8_t ret = getWriteResultNetworkDict(pDictData, 1, &abort_code);
                if (ret == SDO_FINISHED)
                {
                    break;
                }
                else
                {
                    printf("==> Write not finished, wait 1s\n");
                    sleep(1);
                }
            }

        
            uint8_t ret = readNetworkDict(pDictData, 1, 0x4003, 0, int16, 0);

            if (ret == 0)
            {
                fReadDone = TRUE;
            }
            else
            {
                printf("==> Read Error: 0x%x\n", ret);
            }
            
        }

        if (fReadDone)
        {
            while (TRUE && Run)
            {
                int16_t readData = 0;
                UNS32 size = 2;
                UNS32 abortCode = 0;
                uint8_t ret = getReadResultNetworkDict(pDictData, 1, &readData, &size, &abortCode);
                if (ret == SDO_FINISHED)
                {
                    printf("==> recv data: %d, size: %d\n", readData, size);
                    break;
                }
                else
                {
                    printf("==> Read not finished, wait 1s\n");
                    sleep(1);
                }
            }

        }
    #endif
    }

    // Stop timer thread
    StopTimerLoop(&Exit);
    // Close CAN devices (and can threads)
    canClose(pDictData);
    return 0;
}




