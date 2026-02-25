/*
// Copyright (c) 2021-2025 Hewlett Packard Enterprise Development, LP
// 
// Hewlett-Packard and the Hewlett-Packard logo are trademarks of
// Hewlett-Packard Development Company, L.P. in the U.S. and/or other countries.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "chif.hpp"
#include "smif.hpp"
#include "sysrom.hpp"
#include "ev.hpp"
#include "triton.hpp"
#include "misc.hpp"
#include "zlib.h"
#include "uefi.hpp"
#include "uefi_util.hpp"
#include "platdef_api.hpp"
#include "i2c_mapping.hpp"

// externs, mainly for debugging
extern UINT8 platdef[PLATDEF_UPDATE_BUF_SZ + PLATDEF_BLOB_START];
extern bool gdbPrint;
extern void db_smbios_handler(int argc, char *argv[]);
extern int smbios_cfg_read_into_globalvar();
extern void dump_apml_segments();

char * version = (char*)"CHIF daemon v2.13";

int EVError;

/* Used only for debugging */
void dumpheader(struct ChifPkt *pkt, bool recv_resp, int message_size)
{
    int len = 16;

    // dont walk the buffers in hexdump if we wont print them
    if (!gdbPrint)
	return;

    dbPrintf("\nFull Payload Size: %d\n", message_size);
    if(recv_resp)
        dbPrintf("\nDumping Chif Recv Packet Info:\n");
    else {
        dbPrintf("\nDumping Chif Resp Packet Info:\n");
	len = message_size;
        if (len > 32)
            len = 32;
    }

    dbPrintf("pkt_size = 0x%04x, ", pkt->header.pkt_size);
    dbPrintf("sequence  = 0x%04x, ", pkt->header.sequence);
    dbPrintf("command = 0x%04x, ", pkt->header.command);
    dbPrintf("service_id = 0x%02x, ", pkt->header.service_id);
    dbPrintf("version = %d\n", pkt->header.version);
    dbPrintf("Full Message Data:\n");

    if (pkt->header.command == 0x0072) {
        len = 48;
    }

    hexdump(pkt->msg, len);

    if(recv_resp)
        dbPrintf("\n");
    else
        dbPrintf("\nEND\n");
}

int Unknown_Response(void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
    long *ptr;

    respPkt->header.pkt_size = recvPkt->header.pkt_size + 4 ;
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = recvPkt->header.command |0x8000;
    respPkt->header.service_id = 0;

    ptr=(long *)&respPkt->msg[0];

    *ptr = (long)0xFFFF0000;
    return respPkt->header.pkt_size;
}

int UnknownHandler(void *recv, void *resp, int resp_len)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;

    memset(resp, 0, resp_len);
    printf("UnknownHandler: command:0x%08x\n", recvPkt->header.command);
    return Unknown_Response(recv, resp);
}

int ChifHandler(void *recv, void *resp, int resp_len) {
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;

    memset(resp, 0, resp_len);

    switch(recvPkt->header.service_id) {
        case SMIF_SERVICE_ID:
            return SmifHandler(recv, resp);
        case ROM_SERVICE_ID:
            return RomHandler(recv, resp, resp_len);
        case TRITON_SERVICE_ID:
            return TritonHandler(recv, resp);
        case CHIF_SERVICE_ID_HEALTH:
            return HealthHandler(recv, resp);
        case CHIF_SERVICE_ID_BLACKBOX: // or AHS
            return BlackBoxHandler(recv, resp);
        case CHIF_SERVICE_ID_EMBMEDIA:
            return EmbMediaHandler(recv, resp);
        default:
            return UnknownHandler(recv, resp, resp_len);
            break;
    }

    return -1;
}

int main(int argc, char* argv[])
{
    int fd;
    int in_size;
    int out_size;
    int write_size;
    uint8_t recv[CHIF_PKT_MAX_SIZE];
    uint8_t resp[CHIF_PKT_MAX_SIZE];

    if (argc > 1)
    {
        if (strcmp(argv[1], "-v") == 0) {
            printf("%s\n", version);
            exit(0);
        }

        if (strcmp(argv[1], "-pd") == 0) {
            gdbPrint = true;
            init_platdef();
            printf("\nplatdef buffer 0 - %x:\n", PLATDEF_BLOB_START);
            hexdump(platdef, PLATDEF_BLOB_START);
            
            printf("\nplatdef buffer %x - 128k:\n", PLATDEF_BLOB_START);
            hexdump(&platdef[PLATDEF_BLOB_START], PLATDEF_UPDATE_BUF_SZ / 2);
            exit(0);
        }

        if (strcmp(argv[1], "smbios") == 0) {
            gdbPrint = true;
            smbios_cfg_read_into_globalvar();
            db_smbios_handler(argc-1, &argv[1]);
            exit(0);
        }

        if (strcmp(argv[1], "-da") == 0) {
            gdbPrint = true;
            init_platdef();
            dump_apml_segments();
            exit(0);
        }

        if (strcmp(argv[1], "-dbp") == 0) {
            gdbPrint = true;
        }
        else {
            printf("Bad argument\n");
            exit(1);
        }
    }

    printf("%s\n", version);

    init_platdef();
    load_i2c_mapping();
    init_smif();

    fd = open("/dev/chif24", O_RDWR);
    initEV();
 
    while(1) {
        in_size = read(fd, recv, CHIF_PKT_MAX_SIZE);
        if(in_size<=0) {
            dbPrintf("Size is less than 0...\n");
            break;
        }
        dumpheader((struct ChifPkt *)recv, 1, in_size);

        out_size = ChifHandler(recv, resp, CHIF_PKT_MAX_SIZE);

        if(out_size<=0) {
            //error handler
            dbPrintf("ChifHandler error %d\n", out_size);
            continue;
        }

        dumpheader((struct ChifPkt *)resp, 0, out_size);

        fflush(stdout);
        fflush(stderr);
        write_size = write(fd, resp, out_size);
        dbPrintf("Size written: %04x\n", (uint16_t)out_size);
        dbPrintf("\n\n");
        if(write_size<=0) {
            //printf("Size is less than 0...\n");
            continue;
        }
    }

    close(fd);
}
