#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#include "canfestival.h"
#include "masterdic.h"
#include "objdictdef.h"
#include "objacces.h"

static volatile int g_run = 1;
static void on_sigint(int sig){ (void)sig; g_run = 0; }

static void usage(const char* prog){
    fprintf(stderr, "Usage: %s <can-if> <nodeId> <index-hex> <subidx-hex> <type> <value>\n", prog);
    fprintf(stderr, "  type: u8|u16|u32|i8|i16|i32\n");
    fprintf(stderr, "  value: decimal or 0x-prefixed hex\n");
}

static UNS8 parse_type(const char* s){
    if(!s) return 0;
    if(!strcmp(s, "u8")) return uint8;
    if(!strcmp(s, "u16")) return uint16;
    if(!strcmp(s, "u32")) return uint32;
    if(!strcmp(s, "i8")) return int8;
    if(!strcmp(s, "i16")) return int16;
    if(!strcmp(s, "i32")) return int32;
    return 0;
}

static unsigned size_of_type(UNS8 t){
    switch(t){
        case uint8: case int8: return 1;
        case uint16: case int16: return 2;
        case uint32: case int32: return 4;
        default: return 0;
    }
}

static void post_SlaveStateChange(CO_Data* d, UNS8 nodeId, e_nodeState newState){
    (void)d;
    const char* name = "Unknown";
    switch(newState){
        case Initialisation:  name = "Initialisation"; break;
        case Disconnected:    name = "Disconnected";  break;
        case Connecting:      name = "Connecting/Preparing";    break;
        case Stopped:         name = "Stopped";       break;
        case Operational:     name = "Operational";   break;
        case Pre_operational: name = "Pre_operational"; break;
        case Unknown_state:   name = "Unknown_state"; break;
        default: break;
    }
    printf("Node %u -> %s\n", nodeId, name);
}

int main(int argc, char** argv){
    if(argc < 7){ usage(argv[0]); return 1; }
    const char* ifname = argv[1];
    int nodeId = (int)strtol(argv[2], NULL, 0);
    UNS16 index = (UNS16)strtol(argv[3], NULL, 16);
    UNS8 sub = (UNS8)strtol(argv[4], NULL, 16);
    UNS8 dtype = parse_type(argv[5]);
    const char* valStr = argv[6];
    if(dtype == 0){ fprintf(stderr, "Unknown type: %s\n", argv[5]); usage(argv[0]); return 1; }

    // Parse value (hex or dec)
    long long sval = 0;
    if(strncmp(valStr, "0x", 2) == 0 || strncmp(valStr, "0X", 2) == 0)
        sval = strtoll(valStr, NULL, 16);
    else
        sval = strtoll(valStr, NULL, 0);

    s_BOARD board = {0};
    board.busname = (char*)ifname;
    board.baudrate = "";

    extern CO_Data masterdic_Data;
    CO_Data* d = &masterdic_Data;
    d->post_SlaveStateChange = post_SlaveStateChange;

    if(!canOpen(&board, d)){
        fprintf(stderr, "Cannot open CAN interface %s\n", ifname);
        return 2;
    }

    signal(SIGINT, on_sigint);

    TimerInit();
    setNodeId(d, 100);
    setState(d, Pre_operational);
    setState(d, Operational);

    // Configure SDO client to this node
    {
        UNS8 nodeU8 = (UNS8)nodeId;
        UNS32 cob_tx = 0x600 + (UNS32)nodeU8;
        UNS32 cob_rx = 0x580 + (UNS32)nodeU8;
        UNS32 sz32 = sizeof(UNS32);
        UNS32 sz8  = sizeof(UNS8);
        if (writeLocalDict(d, 0x1280, 1, &cob_tx, &sz32, RW) != OD_SUCCESSFUL)
            fprintf(stderr, "Warn: failed to set 0x1280:1 (TX COB-ID)\n");
        if (writeLocalDict(d, 0x1280, 2, &cob_rx, &sz32, RW) != OD_SUCCESSFUL)
            fprintf(stderr, "Warn: failed to set 0x1280:2 (RX COB-ID)\n");
        if (writeLocalDict(d, 0x1280, 3, &nodeU8, &sz8, RW) != OD_SUCCESSFUL)
            fprintf(stderr, "Warn: failed to set 0x1280:3 (Server NodeID)\n");
    }

    masterSendNMTstateChange(d, (UNS8)nodeId, NMT_Start_Node);

    unsigned sz = size_of_type(dtype);
    if(sz == 0){ fprintf(stderr, "Unsupported type for write.\n"); canClose(d); return 3; }

    // Prepare value container matching type
    UNS8  v_u8;  UNS16 v_u16;  UNS32 v_u32;
    INTEGER8 v_i8; INTEGER16 v_i16; INTEGER32 v_i32;
    void* pData = NULL;
    switch(dtype){
        case uint8:  v_u8  = (UNS8)(sval & 0xFF); pData = &v_u8;  break;
        case uint16: v_u16 = (UNS16)(sval & 0xFFFF); pData = &v_u16; break;
        case uint32: v_u32 = (UNS32)(sval & 0xFFFFFFFFu); pData = &v_u32; break;
        case int8:   v_i8  = (INTEGER8)(sval & 0xFF); pData = &v_i8;  break;
        case int16:  v_i16 = (INTEGER16)(sval & 0xFFFF); pData = &v_i16; break;
        case int32:  v_i32 = (INTEGER32)(sval & 0xFFFFFFFFu); pData = &v_i32; break;
        default: break;
    }

    printf("Writing SDO 0x%04X:%02X to node %d on %s (type=%s)\n", index, sub, nodeId, ifname, argv[5]);

    // Initiate SDO write
    UNS8 ret = writeNetworkDict(d, (UNS8)nodeId, index, sub, sz, dtype, pData, 0);
    if(ret){
        fprintf(stderr, "writeNetworkDict failed: 0x%02X\n", ret);
        canClose(d);
        return 4;
    }

    // Wait for completion
    int loops = 0;
    while(g_run){
        UNS32 abortCode = 0;
        UNS8 st = getWriteResultNetworkDict(d, (UNS8)nodeId, &abortCode);
        if(st == SDO_FINISHED){
            switch(dtype){
                case uint8:  printf("Wrote: 0x%02X (u8=%u)\n",  v_u8,  (unsigned)v_u8); break;
                case uint16: printf("Wrote: 0x%04X (u16=%u)\n", v_u16, (unsigned)v_u16); break;
                case uint32: printf("Wrote: 0x%08X (u32=%u)\n", v_u32, (unsigned)v_u32); break;
                case int8:   printf("Wrote: 0x%02X (i8=%d)\n",  (unsigned)(UNS8)v_i8,  (int)v_i8); break;
                case int16:  printf("Wrote: 0x%04X (i16=%d)\n", (unsigned)(UNS16)v_i16, (int)v_i16); break;
                case int32:  printf("Wrote: 0x%08X (i32=%d)\n", (unsigned)v_i32, (int)v_i32); break;
                default: break;
            }
            break;
        } else if(st == SDO_ABORTED_RCV || st == SDO_ABORTED_INTERNAL){
            fprintf(stderr, "SDO write aborted (st=0x%02X): 0x%08X\n", st, (unsigned)abortCode);
            canClose(d);
            return 5;
        }
        usleep(1000 * 10);
        if(++loops > 1000){
            fprintf(stderr, "Timeout waiting SDO write response\n");
            canClose(d);
            return 6;
        }
    }

    canClose(d);
    return 0;
}
