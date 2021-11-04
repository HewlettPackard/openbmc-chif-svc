/*
// Copyright (c) 2021 Hewlett-Packard Development Company, L.P.
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
#include <stdint.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "chif.hpp"
#include "smif.hpp"
#include "sysrom.hpp"
#include "ev.hpp"

void dumpheader(struct ChifPkt *pkt, bool recv_resp, int message_size)
{
	printf("\nFull Payload Size: %d\n", message_size);
	if(recv_resp)
		printf("\nDumping Chif Recv Packet Info:\n");
	else
		printf("\nDumping Chif Resp Packet Info:\n");
		printf("\nHeader:\n");
		printf("pkt_size = %d, ", pkt->header.pkt_size);
		printf("sequence  = 0x%04x, ", pkt->header.sequence);
		printf("command = 0x%04x, ", pkt->header.command);
		printf("service_id = 0x%02x, ", pkt->header.service_id);
		printf("version = %d\n", pkt->header.version);
		printf("\n\nFull Message Data:\n");
	for (int idx=0; idx < (message_size - 1); idx++) {
		printf("0x%x ", pkt->msg[idx]);
	}
	printf("\n\n");
}

int ChifHandler(void *recv, int recv_len, void *resp, int resp_len) {
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;

	switch(recvPkt->header.service_id) {
		case SMIF_SERVICE_ID:
			return SmifHandler(recv, recv_len, resp, resp_len);

		case ROM_SERVICE_ID:
			return RomHandler(recv, recv_len, resp, resp_len);

		default:
			break;
	}

	printf("SMIF HEADER, SERVICE ID: (0x%02x) COMMAND: (0x%04x)\n", recvPkt->header.service_id, recvPkt->header.command);
	return -1;
}

int main()
{
	int fd;
	int in_size;
	int out_size;
	int write_size;
	uint8_t recv[CHIF_PKT_MAX_SIZE];
	uint8_t resp[CHIF_PKT_MAX_SIZE];
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_8130 *respMsg = (struct pkt_8130 *)&respPkt->msg[0];
	//int index;

	fd = open("/dev/chif24", O_RDWR);
	printf("InitEV\n");
	initEV();

	while(1) {
		in_size = read(fd, recv, CHIF_PKT_MAX_SIZE);
		if(in_size<=0) {
		printf("Size is less than 0...\n");
			break;
		}
				printf("----------------------------");
		dumpheader((struct ChifPkt *)recv, 1, in_size);

		out_size = ChifHandler(recv, in_size, resp, CHIF_PKT_MAX_SIZE);
		if(out_size<=0) {
			//error handler
			printf("ChifHandler error %d\n", out_size);
			continue;
		}

		dumpheader((struct ChifPkt *)resp, 0, out_size);
		printf("----------------------------");
		write_size = write(fd, resp, out_size);
		if(write_size<=0) {
			//printf("Size is less than 0...\n");
			continue;
		}
	}

	close(fd);
}
