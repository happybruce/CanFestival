#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#include "canfestival.h"
#include "masterdic.h" // reuse generated OD for types and CO_Data
#include "objdictdef.h" // data type codes (uint8/uint16/...)
#include "objacces.h"   // writeLocalDict

static volatile int g_run = 1;
static void on_sigint(int sig){ (void)sig; g_run = 0; }

static void usage(const char* prog){
    fprintf(stderr, "Usage: %s <can-if> <nodeId> <index-hex> <subidx-hex> [type]\n", prog);
    fprintf(stderr, "  type: u8|u16|u32|i8|i16|i32|domain (default u32)\n");
}

static UNS8 parse_type(const char* s){
    if(!s) return uint32;
    if(!strcmp(s, "u8")) return uint8;
    if(!strcmp(s, "u16")) return uint16;
    if(!strcmp(s, "u32")) return uint32;
    if(!strcmp(s, "i8")) return int8;
    if(!strcmp(s, "i16")) return int16;
    if(!strcmp(s, "i32")) return int32;
    if(!strcmp(s, "domain")) return domain;
    return uint32;
}

static void post_SlaveStateChange(CO_Data* d, UNS8 nodeId, e_nodeState newState){
    (void)d;
    const char* name = "Unknown";
    switch(newState){
        case Initialisation:  name = "Initialisation"; break;
        case Disconnected:    name = "Disconnected";  break;
        case Connecting:      name = "Connecting/Preparing";    break; /* Preparing has same value */
        case Stopped:         name = "Stopped";       break;
        case Operational:     name = "Operational";   break;
        case Pre_operational: name = "Pre_operational"; break;
        case Unknown_state:   name = "Unknown_state"; break;
        default: break;
    }
    printf("Node %u -> %s\n", nodeId, name);
}

int main(int argc, char** argv){
    if(argc < 5){ usage(argv[0]); return 1; }
    const char* ifname = argv[1];
    int nodeId = (int)strtol(argv[2], NULL, 0);
    UNS16 index = (UNS16)strtol(argv[3], NULL, 16);
    UNS8 sub = (UNS8)strtol(argv[4], NULL, 16);
    UNS8 dtype = parse_type(argc > 5 ? argv[5] : NULL);

    s_BOARD board = {0};
    board.busname = (char*)ifname;
    board.baudrate = ""; // use pre-configured bitrate on interface

    // Prepare minimal CO_Data using masterdic_Data from example
    extern CO_Data masterdic_Data;
    CO_Data* d = &masterdic_Data;

    // Install minimal callbacks
    d->post_SlaveStateChange = post_SlaveStateChange;

    if(!canOpen(&board, d)){
        fprintf(stderr, "Cannot open CAN interface %s\n", ifname);
        return 2;
    }

    signal(SIGINT, on_sigint);

    // Start timers (for timeouts) and set node state
    TimerInit();
    // Put master into Pre-operational and then Operational
    setNodeId(d, 100);
    setState(d, Pre_operational);
    setState(d, Operational);

    printf("Reading SDO 0x%04X:%02X from node %d on %s (type=%d)\n", index, sub, nodeId, ifname, (int)dtype);

    // Configure SDO client (0x1280) to address the requested nodeId
    {
        UNS8 nodeU8 = (UNS8)nodeId;
        UNS32 cob_tx = 0x600 + (UNS32)nodeU8; // client->server
        UNS32 cob_rx = 0x580 + (UNS32)nodeU8; // server->client
        UNS32 sz32 = sizeof(UNS32);
        UNS32 sz8  = sizeof(UNS8);
        if (writeLocalDict(d, 0x1280, 1, &cob_tx, &sz32, RW) != OD_SUCCESSFUL)
            fprintf(stderr, "Warn: failed to set 0x1280:1 (TX COB-ID)\n");
        if (writeLocalDict(d, 0x1280, 2, &cob_rx, &sz32, RW) != OD_SUCCESSFUL)
            fprintf(stderr, "Warn: failed to set 0x1280:2 (RX COB-ID)\n");
        if (writeLocalDict(d, 0x1280, 3, &nodeU8, &sz8, RW) != OD_SUCCESSFUL)
            fprintf(stderr, "Warn: failed to set 0x1280:3 (Server NodeID)\n");
    }

    // Ensure target node is started
    masterSendNMTstateChange(d, (UNS8)nodeId, NMT_Start_Node);

    // Initiate SDO read
    UNS8 ret = readNetworkDict(d, (UNS8)nodeId, index, sub, dtype, 0);
    if(ret){
        fprintf(stderr, "readNetworkDict failed: 0x%02X\n", ret);
        canClose(d);
        return 3;
    }

    // Poll for result
    int loops = 0;
    union { UNS8 u8; UNS16 u16; UNS32 u32; INTEGER8 i8; INTEGER16 i16; INTEGER32 i32; } val;
    memset(&val, 0, sizeof(val));
    unsigned char domainBuf[512];
    while(g_run){
        UNS32 size;
        switch(dtype){
            case uint8:
            case int8:   size = 1; break;
            case uint16:
            case int16:  size = 2; break;
            case uint32:
            case int32:  size = 4; break;
            case domain: size = sizeof(domainBuf); break;
            default:     size = sizeof(val); break;
        }
        void* dataPtr = (dtype == domain) ? (void*)domainBuf : (void*)&val;
        UNS32 abortCode = 0;

        UNS8 st = getReadResultNetworkDict(d, (UNS8)nodeId, dataPtr, &size, &abortCode);
        if(st == SDO_FINISHED){
            switch(dtype){
                case uint8:  printf("Value: 0x%02X (u8=%u)\n",  val.u8, (unsigned)val.u8); break;
                case uint16: printf("Value: 0x%04X (u16=%u)\n", (unsigned)val.u16, (unsigned)val.u16); break;
                case uint32: printf("Value: 0x%08X (u32=%u)\n", (unsigned)val.u32, (unsigned)val.u32); break;
                case int8:   printf("Value: 0x%02X (i8=%d)\n",  (unsigned)(UNS8)val.i8,  (int)val.i8); break;
                case int16:  printf("Value: 0x%04X (i16=%d)\n", (unsigned)(UNS16)val.i16, (int)val.i16); break;
                case int32:  printf("Value: 0x%08X (i32=%d)\n", (unsigned)val.i32, (int)val.i32); break;
                case domain: {
                    printf("Value: <domain> size=%u\n", (unsigned)size);
                    unsigned toShow = size < 32 ? size : 32;
                    printf("  data:");
                    for(unsigned i=0;i<toShow;i++) printf(" %02X", domainBuf[i]);
                    if(size>toShow) printf(" ...");
                    printf("\n");
                    break;
                }
                default:     printf("Value: <unknown type>\n"); break;
            }
            break;
        } else if(st == SDO_ABORTED_RCV || st == SDO_ABORTED_INTERNAL){
            fprintf(stderr, "SDO aborted (st=0x%02X): 0x%08X\n", st, (unsigned)abortCode);
            canClose(d);
            return 4;
        } else if (st == SDO_PROVIDED_BUFFER_TOO_SMALL) {
            fprintf(stderr, "Provided buffer too small (need %u+)\n", (unsigned)size);
            canClose(d);
            return 6;
        }
        usleep(1000 * 10);
    if(++loops > 1000){
            fprintf(stderr, "Timeout waiting SDO response\n");
            canClose(d);
            return 5;
        }
    }

    canClose(d);
    return 0;
}
