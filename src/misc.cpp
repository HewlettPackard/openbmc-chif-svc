/*
// Copyright (c) 2021-2025 Hewlett-Packard Enterprise Development, LP
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

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <iostream>
#include <cstdarg>
#include "misc.hpp"
#include "chif.hpp"

bool gdbPrint=false;

int GenResponse(void *recv, void *resp, uint8_t service_id)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
    struct pkt_gen *respMsg = (struct pkt_gen *)&respPkt->msg[0];

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader);
	respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = recvPkt->msg[0];
	respPkt->header.service_id = service_id;
    respMsg->ErrorCode = 0;
	return respPkt->header.pkt_size;
}


int EmbMediaHandler(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;

	dbPrintf("EmbMediaHandler: command:0x%08x\n", recvPkt->header.command);

	switch(recvPkt->header.command) {
        case 0x01:
        case 0x04:
		case 0x06:
				return GenResponse(recv, resp, CHIF_SERVICE_ID_EMBMEDIA);
		default:
			//other rom command doesn't need to response
			return -1;
	}
}

#define EH_INTF_TYPE_RESERVED             0x00
#define EH_INTF_TYPE_NVRAM_SECURE         0x11
#define EH_INTF_TYPE_RESET_STATUS         0x12
#define EH_INTF_TYPE_APML_VERSION         0x1C
#define EH_INTF_TYPE_POST_FLAGS           0x1D


int HealthHandler(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;

	dbPrintf("HealthHandler: command:0x%08x\n", recvPkt->header.command);

    // see eh_intf_dispatcher() function in iLO health/sr/eh_intf.c for why we return data as 0 for all these
	switch(recvPkt->msg[0]) {
        case EH_INTF_TYPE_RESERVED:
        case EH_INTF_TYPE_RESET_STATUS:
        case EH_INTF_TYPE_NVRAM_SECURE:
        case EH_INTF_TYPE_APML_VERSION:
        case EH_INTF_TYPE_POST_FLAGS:
				return GenResponse(recv, resp, CHIF_SERVICE_ID_HEALTH);
		default:
			//other, respond with error
			return -1;
	}
}

int BlackBoxHandler(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;

	dbPrintf("BlackBoxHandler: command:0x%08x\n", recvPkt->header.command);

	switch(recvPkt->header.command) {
        case 0x00:
        case 0x8000:
				return GenResponse(recv, resp, CHIF_SERVICE_ID_BLACKBOX);
		default:
            //other, respond with error
			return -1;
	}
}

/* Prints debug output if enabled at the command line */
void dbPrintf(const char *format, ...)
{
    if (!gdbPrint) {
        return;
    }
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

