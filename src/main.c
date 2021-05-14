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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "chif.h"

int DumpHeader(struct ChifPkt *pkt)
{
	printf("pkt_size = %d\n", pkt->header.pkt_size);
	printf("sequence  = 0x%04x\n", pkt->header.sequence);
	printf("command = 0x%04x\n", pkt->header.command);
	printf("service_id = 0x%02x\n", pkt->header.service_id);
	printf("version = %d\n", pkt->header.version);
}

int ChifHandler(void *recv, int recv_len, void *resp, int resp_len) {
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;

	switch(recvPkt->header.service_id) {
		default:
			break;
	}

	printf("unspported service id(0x%02x) command(0x%04x)\n", recvPkt->header.service_id, recvPkt->header.command);
	return -1;
}

int main()
{
	int fd;
	int size;
	uint8_t recv[CHIF_PKT_MAX_SIZE];
	uint8_t resp[CHIF_PKT_MAX_SIZE];
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;

	fd = open("/dev/chif24", O_RDWR);

	while(1) {
		size = read(fd, recv, CHIF_PKT_MAX_SIZE);
		if(size<=0) {
			break;
		}

		printf("read = %d, srqNum=0x%04x\n", size, recvPkt->header.sequence);
		DumpHeader((struct ChifPkt *)recv);

		size = ChifHandler(recv, size, resp, CHIF_PKT_MAX_SIZE);
		if(size<=0) {
			//error handler
			printf("ChifHandler error %d\n", size);
			continue;
		}

		printf("write = %d, seqNum=0x%04x\n", size, respPkt->header.sequence);
		write(fd, resp, size);
	}

	close(fd);
}
