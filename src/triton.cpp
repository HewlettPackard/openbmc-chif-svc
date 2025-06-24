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

#include "triton.hpp"
#include "chif.hpp"
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>


int Triton_Response(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = recvPkt->header.command | 0x8000;
	respPkt->header.service_id = TRITON_SERVICE_ID;
    printf("Triton command: %04x\n", recvPkt->header.command);
    printf("Triton resp command: %04x\n", respPkt->header.command);
	return respPkt->header.pkt_size;
}


int TritonHandler(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
//	struct ChifPkt *respPkt = (struct ChifPkt *)resp;

	printf("TritonHandler: command:0x%08x\n", recvPkt->header.command);

	switch(recvPkt->header.command) {
		case 0x06:
				return Triton_Response(recv, resp);
		default:
			//other rom command doesn't need to response
			return -5;
	}
}
