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

#include "chif.hpp"
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include "ev.hpp"

struct pkt_8002 {
	uint32_t ErrorCode;
	uint16_t status_word;
	uint16_t max_users;
	uint16_t firmware_version;
	uint32_t firmware_date;
	uint32_t post_errcode;
	uint32_t datetime;
	uint32_t hw_revision;
	uint8_t board_serial_number[20];
	uint16_t cable_status;
	uint32_t latest_event_id;
	uint32_t cfg_writecount;
	uint32_t post_errcode_mask;
	uint8_t   class_;
	uint8_t   subclass;
	uint8_t   rc_in_use;
	uint8_t   application;
	uint8_t   security_state;
	uint64_t  chip_id;          // ASIC ID
	uint8_t   reserved[19];
} __attribute__ ((packed));

struct stat_route {
	uint32_t  dest;
	uint32_t  gate;
} __attribute__ ((packed));

struct pkt_8063{
	uint32_t    ErrorCode;
	uint32_t    nic_settings;      /* bitfield defined in CFG_NIC_SET_*   */
	uint32_t    nic_status;         /* bitfield defined in CFG_NIC_STAT_* */
	uint32_t    nic_ipaddr;
	uint32_t    nic_ip_mask;
	uint8_t     nic_mac_addr[6];
	uint32_t    nic_gateway_ip;
	uint32_t    nic_wins_ip[2];
	uint32_t    nic_dns_ip[3];
	uint32_t    dhcp_server_ip_address;
	uint32_t    dhcp_options;      /* bitfield defined in CFG_NIC_*   */
	uint32_t    dhcp_status;      /* bitfield defined in CFG_NIC_DHCPSTAT_* */
	uint32_t    front_nic_ipaddr;
	uint32_t    front_nic_ip_mask;
	uint32_t    front_settings;      /* bitfield defined in CFG_NIC_SET_* applies to front port (if implemented) */
	struct      stat_route  route[3];
	uint8_t        nic_name[50];      /* "gromit" in gromit.eng.acme.com */
	uint8_t        nic_domain_name[128];   /* "eng.acme.com" in gromit.eng.acme.com */
} __attribute__ ((packed));

struct pkt_0072 {
	uint32_t  reserved;
	uint8_t   magic[8];
	uint16_t  address;
	uint8_t   engine;
	uint8_t   write_len;
	uint8_t   read_len;
	uint8_t   data[32];
} __attribute__ ((packed));

struct pkt_8072 {
	uint32_t  ErrorCode;
	uint8_t   reserved1[8];
	uint16_t  address;
	uint8_t   engine;
	uint8_t   reserved2;
	uint8_t   read_len;
	uint8_t   data[32];
} __attribute__ ((packed));

struct pkt_012b {
	uint16_t  idx;       // Index of EV
	uint16_t  pad;       // used for byte alignment
} __attribute__ ((packed));

struct pkt_812b {
	uint32_t ErrorCode; // EV Error/Return Code
	uint8_t name[32];   // EV Name
	uint16_t sz_ev;     // Size of EV
	uint8_t buf[1];     // EV Data
} __attribute__ ((packed));

struct pkt_012c {
	uint8_t   flags;     // Bit Field for command
	uint8_t   rsvd[3];   // Used for growth and byte alignment
	uint8_t   name[32];  // EV Name
	uint16_t  sz_ev;     // Size of EV data
	uint8_t   buf[1];    // EV Data
} __attribute__ ((packed));

struct pkt_812c {
	uint32_t  ErrorCode; // EV Error/Return Code
	uint8_t name[32];    // EV Name
	uint16_t sz_ev;      // Size of EV
	uint8_t buf[1];      // EV Data
} __attribute__ ((packed));

struct pkt_812d {
	uint32_t ErrorCode;
	uint32_t ImageAuthStatusVersion;
	uint8_t ImageAuthBitField;
	uint8_t CurrentStatusSideAState;
	uint8_t CurrentStatusSideBState;
	uint8_t CurrentStatusRemediationActionTaken;
	uint32_t SignedRegionStartingOffset;
	uint32_t SignedRegionEndingOffset;
	uint32_t SignatureSize;
	uint8_t SignatureAlgorithemGuid[16];
	uint8_t FvGuid[16];
	uint16_t ExecutionOrdinal;
	uint16_t ValidatingAgent;

	uint8_t Signature[64];
} __attribute__ ((packed));

struct pkt_0130 {
	uint8_t name[32];
} __attribute__ ((packed));

struct pkt_8130 {
	uint32_t ErrorCode;
	uint8_t name[32];
	uint16_t sz_ev;  // Size of EV
	uint8_t buf[1];  // EV Data
} __attribute__ ((packed));

struct pkt_8132 {
	uint32_t  ErrorCode;      /* 0: success; !0: error */
	uint32_t  rem_sz;         /* remaining size of EV file */
	uint32_t  present_evs;    /* number of EVs present in file */
	uint32_t  max_sz;         /* Max size of EV file in KB */
};

struct pkt_8139 {
	uint32_t ErrorCode;          /* 0: success; !0: error */
	uint8_t  security_state; /* ERROR=0,FACTORY=1,WIPE=2,PRODUCTION=3,HIGH_SECURITY=4,FIPS=5,SUITEB=6 */
	uint8_t  resvd[3];
} __attribute__ ((packed));

struct pkt_0143 {
	uint8_t post_state;
	uint8_t  reserved1;
	uint16_t reserved2;
} __attribute__ ((packed));

struct pkt_8143 {
	uint32_t ErrorCode;
} __attribute__ ((packed));

struct pkt_8150 {
	uint32_t ErrorCode;
} __attribute__ ((packed));

int SmifPkt_0002(void *recv, int recv_len, void *resp, int resp_len)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_8002 *respMsg = (struct pkt_8002 *)&respPkt->msg[0];
	char sn[] = "LON-123456";

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8002);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8002;
	respPkt->header.service_id = 0;

	respMsg->ErrorCode = 0;

	respMsg->firmware_version = 0x0101;
	respMsg->hw_revision = 0x06;
	respMsg->class_ = 7;
	respMsg->subclass = 0;
	respMsg->application = 5;
	respMsg->security_state = 1;
	respMsg->chip_id = 0x123456789;
	strncpy((char *)respMsg->board_serial_number, sn, strlen((char *)respMsg->board_serial_number));

	return respPkt->header.pkt_size;
}

int SmifPkt_0063(void *recv, int recv_len, void *resp, int resp_len)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_8063 *respMsg = (struct pkt_8063 *)&respPkt->msg[0];
	char sn[] = "LON-123456";

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8063);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8063;
	respPkt->header.service_id = 0;

	respMsg->ErrorCode = 0;
	respMsg->nic_settings = 0x03;
	respMsg->nic_status = 0x00;
	respMsg->nic_ipaddr = 0x01020304;
	respMsg->nic_ip_mask = 0xffffffff;

	return respPkt->header.pkt_size;
}

int SmifPkt_0072(void *recv, int recv_len, void *resp, int resp_len)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_0072 *recvMsg = (struct pkt_0072 *)&recvPkt->msg[0];
	struct pkt_8072 *respMsg = (struct pkt_8072 *)&respPkt->msg[0];

	printf("pkt_0072->address=0x%04x\n", recvMsg->address);
	printf("pkt_0072->engine=0x%02x\n", recvMsg->engine);
	printf("pkt_0072->write_len=0x%02x\n", recvMsg->write_len);
	printf("pkt_0072->read_len=0x%02x\n", recvMsg->read_len);

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8072);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8072;
	respPkt->header.service_id = 0;

	if(recvMsg->address == 0xffff && recvMsg->engine == 0xff) {
		printf("pkt_0072->data[0] = %c\n", recvMsg->data[0]);
		respMsg->ErrorCode = 2;
	}
	else {
		respMsg->ErrorCode = 105;
	}

	return respPkt->header.pkt_size;
}

int SmifPkt_012b(void *recv, int recv_len, void *resp, int resp_len)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_012b *recvMsg = (struct pkt_012b *)&recvPkt->msg[0];
	struct pkt_812b *respMsg = (struct pkt_812b *)&respPkt->msg[0];

	int ev_size;

	//printf("smif_012b: ev index: %d\n", recvMsg->idx);
	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_812b) -1;
	printf("SizeOf Packet: %d\n", respPkt->header.pkt_size);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x812b;
	respPkt->header.service_id = 0;

	ev_size = getEVbyIndex(recvMsg->idx, (char *)respMsg->buf, EV_MAX_LEN);
	if(ev_size<0) {
		switch(ev_size) {
			case -1:
				printf("No such EV");
				respMsg->ErrorCode = 0x02; //NO such EV
				break;
			default:
				printf("Other error\n");
				respMsg->ErrorCode = 0x01;
				break;
		}
	}
	else {
		printf("Success!\n");
		respMsg->ErrorCode = 0x00;
		respMsg->sz_ev += ev_size;
	}
	return respPkt->header.pkt_size + respMsg->sz_ev;
}

int SmifPkt_012c(void *recv, int recv_len, void *resp, int resp_len)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_012c *recvMsg = (struct pkt_012c *)&recvPkt->msg[0];
	struct pkt_812c *respMsg = (struct pkt_812c *)&respPkt->msg[0];
	int rc;
	int ev_size;

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_812c);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x812c;
	respPkt->header.service_id = 0;

	printf("\nsmif_012c: ev name: %s flags:%d sz_ev:%d\n", recvMsg->name, recvMsg->flags, recvMsg->sz_ev);
	switch(recvMsg->flags) {
		case 0x01: //Set EV
			printf("smif_012c: set EV\n");
			if(recvMsg->sz_ev>EV_DATA_MAX_LEN) {
				respMsg->ErrorCode = 0x04; //EV data too large
				printf("smif_012c: error_code: 0x04\n");
				break;
			}

			rc = setEV((char *)recvMsg->name, (char *)recvMsg->buf, recvMsg->sz_ev);
			if(rc<0) {
				respMsg->ErrorCode = 0x01; // EV Error
				printf("smif_012c: error_code: 0x01\n");
				break;
			}

			respMsg->ErrorCode = 0x00;
			printf("smif_012c: ok\n");
			ev_size = getEVbyName((char *)recvMsg->name, (char *)respMsg->buf, EV_MAX_LEN);
			break;

		case 0x02: //EV delete
			printf("smif_012c: delete EV\n");
			if(recvMsg->name[32] != 0x00) {
				respMsg->ErrorCode = 0x03; //EV name too large
				printf("smif_012c: error_code: 0x03\n");
				break;
			}

			rc = delEV((char *)recvMsg->name);
			if(rc<0) {
				respMsg->ErrorCode = 0x01; // EV Error
				printf("smif_012c: error_code: 0x01\n");
				break;
			}

			respMsg->ErrorCode = 0x00;
			printf("smif_012c: delete OK\n");
			ev_size = getEVbyName((char *)recvMsg->name, (char *)respMsg->buf, EV_MAX_LEN);
			break;

		case 0x04: //EV delete all
			printf("smif_012: delete all\n");
			clearEV();
			respMsg->ErrorCode = 0x00; //Success
			printf("smif_012: delete all success\n");
			break;
		default:
			respMsg->ErrorCode = 0x05; // EV not supported
			printf("smif_012: ev not supported\n");
			break;
	}
	respMsg->sz_ev += ev_size;           //respMsg->sz_ev is 0 so now this is setting ev_size;
	memcpy(respMsg->name, recvMsg->name, 32);

	printEVs();
	
	printf("\nResp Pkt (after EV set):\n");
	printf("Message Error Code: 0x%x\n", respMsg->ErrorCode);
	printf("EV Name:\n");
	for(int idx=0; idx<32; idx++)
	{
		printf("Char [%d]: 0x%x, ", idx, respMsg->name[idx]);
	}
	printf("\nEnd EV Name\n");
	printf("EV Size: %d\n", respMsg->sz_ev);
	printf("EV Data:\n");
	for(int idx=0; idx<(respMsg->sz_ev) - 1; idx++)
	{
		printf("Byte [%d]: 0x%x\n", idx, respMsg->buf[idx]);
	}
	printf("\nEnd Message Data\n");
	printf("\nResp Message Full:\n");
	for(int idx=0; idx<(respMsg->sz_ev) - 1; idx++)
	{
		printf("Byte [%d]: 0x%x\n", idx, respPkt->msg[idx]);
	}
	printf("\n");
	printf("Header Packet Size: %d\n", respPkt->header.pkt_size);
	printf("Returning Size: %d\n", (respPkt->header.pkt_size + respMsg->sz_ev));

	return respPkt->header.pkt_size + respMsg->sz_ev;
}

int SmifPkt_012d(void *recv, int recv_len, void *resp, int resp_len)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_812d *respMsg = (struct pkt_812d *)&respPkt->msg[0];

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_812d);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x812d;
	respPkt->header.service_id = 0;

	respMsg->ErrorCode = 0x0;
	respMsg->ImageAuthStatusVersion = 1;
	respMsg->ImageAuthBitField = 0x01;
	respMsg->CurrentStatusSideAState = 0;
	respMsg->CurrentStatusSideBState = 0;
	respMsg->CurrentStatusRemediationActionTaken = 0;
	respMsg->ValidatingAgent = 2;

	return respPkt->header.pkt_size;
}

int SmifPkt_0130(void *recv, int recv_len, void *resp, int resp_len)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_0130 *recvMsg = (struct pkt_0130 *)&recvPkt->msg[0];
	struct pkt_8130 *respMsg = (struct pkt_8130 *)&respPkt->msg[0];

	int ev_size;

	printf("\nsmif_0130: ev name: %s\n", recvMsg->name);
	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8130) -1;
	//printf("SizeOf Packet: %d\n", respPkt->header.pkt_size);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8130;
	respPkt->header.service_id = 0;
	//printf("\nRecv Pkt (before EV read):\n");
	//printf("EV Name:\n");
	//for(int idx=0; idx<32; idx++)
	//{
	//  printf("Byte [%d]: 0x%x\n", idx, recvMsg->name[idx]);
	//}

	ev_size = getEVbyName((char *)recvMsg->name, (char *)respMsg->buf, EV_MAX_LEN);
	if(ev_size<0) {
		switch(ev_size) {
			case -1:
				printf("smif_0130: No such EV\n");
				respMsg->ErrorCode = 0x02; //NO such EV
				break;
			default:
        printf("smif_0130: EV Error\n");
				respMsg->ErrorCode = 0x01; //EV Error
				break;
		}
	}
	else {
		respMsg->ErrorCode = 0x00; //return 0x00 as ok
		respMsg->sz_ev += ev_size; //respMsg->sz_ev is 0 so now this is setting ev_size;
		memcpy(respMsg->name, recvMsg->name, 32);
		//respMsg->buff contains the data already so no memcpy
	}
	printEVs();
	printf("\nResp Pkt (after EV read):\n");
	printf("Message Error Code: 0x%x\n", respMsg->ErrorCode);
	printf("EV Name:\n");
	for(int idx=0; idx<32; idx++)
	{
		printf("Char [%d]: 0x%x, ", idx, respMsg->name[idx]);
	}
	printf("\nEnd EV Name\n");
	printf("EV Size: %d\n", respMsg->sz_ev);
	printf("EV Data:\n");
	for(int idx=0; idx< (respMsg->sz_ev - 1); idx++)
	{
		printf("Byte [%d]: 0x%x\n", idx, respMsg->buf[idx]);
	}
	printf("\nEnd Message Data\n");
	printf("\nResp Message Full:\n");
	for(int idx=0; idx<(respMsg->sz_ev - 1); idx++)
	{
		printf("Byte [%d]: 0x%x\n", idx, respPkt->msg[idx]);
	}
	printf("\n");
	printf("Header Packet Size: %d\n", respPkt->header.pkt_size);
	printf("Returning Size: %d\n", (respPkt->header.pkt_size + respMsg->sz_ev));

	return respPkt->header.pkt_size + respMsg->sz_ev;
}

int SmifPkt_0132(void *recv, int recv_len, void *resp, int resp_len)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_8132 *respMsg = (struct pkt_8132 *)&respPkt->msg[0];

	int ev_size;

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8132);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8132;
	respPkt->header.service_id = 0;

	respMsg->ErrorCode = 0x00;
	respMsg->max_sz = EV_FILE_MAX_SIZE / 1024;
	respMsg->rem_sz = (EV_FILE_MAX_SIZE - EV_FILE_HEADER) - getSizeOfEVfile();
	respMsg->present_evs = getNumOfAllEV();

	return respPkt->header.pkt_size;
}

int SmifPkt_0139(void *recv, int recv_len, void *resp, int resp_len)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_8139 *respMsg = (struct pkt_8139 *)&respPkt->msg[0];

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8139);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8139;
	respPkt->header.service_id = 0;

	respMsg->security_state = 0x01;
	respMsg->ErrorCode = 0x00;

	printf("smif_0139: error_code:0x%02x\n", respMsg->ErrorCode);

	return respPkt->header.pkt_size;
}

int SmifPkt_0143(void *recv, int recv_len, void *resp, int resp_len)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_0143 *recvMsg = (struct pkt_0143 *)&recvPkt->msg[0];
	struct pkt_8143 *respMsg = (struct pkt_8143 *)&respPkt->msg[0];

	printf("smif_0143: post_state:0x%02x\n", recvMsg->post_state);

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8143);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8143;
	respPkt->header.service_id = 0;

	respMsg->ErrorCode = 0x00;

	return respPkt->header.pkt_size;
}

int SmifPkt_0150(void *recv, int recv_len, void *resp, int resp_len)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_8150 *respMsg = (struct pkt_8150 *)&respPkt->msg[0];

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8150);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8150;
	respPkt->header.service_id = 0;

	respMsg->ErrorCode = 0x00;

	return respPkt->header.pkt_size;
}

int SmifPkt_badcmd(void *recv, int recv_len, void *resp, int resp_len)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	uint32_t *pErrorCode = (uint32_t*)&recvPkt->msg[0];
	uint8_t *pErrorStr = (uint8_t*)&recvPkt->msg[4];
	uint8_t str_badcmd[] = "Bad command";

	printf("Smif bad packet command 0x%04x\n", recvPkt->header.command);

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(uint32_t) + sizeof(str_badcmd);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0xffff;
	respPkt->header.service_id = 0;

	*pErrorCode = 0xffff0000;
	printf("pErrorStr %s\n", str_badcmd);

	return respPkt->header.pkt_size;
}

int SmifHandler(void *recv, int recv_len, void *resp, int resp_len)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;

	memset(resp, 0, resp_len);
  printf("Recv, ChifPkt Len: %d, ChifPktData: %s", recv_len, (char *)recv);
  printf("Resp, ChifPkt Len: %d, ChifPktData: %s", resp_len, (char *)resp);

	switch(recvPkt->header.command) {
		case 0x0002:
			printf("smif_0x0002: Get Status\n");
			return SmifPkt_0002(recv, recv_len, resp, resp_len);

		case 0x063:
			printf("smif_0x0063: Get NIC Config\n");
			return SmifPkt_0063(recv, recv_len, resp, resp_len);

		case 0x0072:
			printf("smif_0x0072: I2C Transaction Request\n");
			return SmifPkt_0072(recv, recv_len, resp, resp_len);

		case 0x012b:
			printf("smif_0x012B: Get Status BY EV index\n");
			return SmifPkt_012b(recv, recv_len, resp, resp_len);

		case 0x012c:
			printf("smif_0x012C: EV Set/Delete/Delete All\n");
			return SmifPkt_012c(recv, recv_len, resp, resp_len);

		case 0x012d:
			printf("smif_0x012D: Get BIOS Authorization Status\n");
			return SmifPkt_012d(recv, recv_len, resp, resp_len);

		case 0x0130:
			printf("smif_0x0130: Get EV by name\n");
			return SmifPkt_0130(recv, recv_len, resp, resp_len);

		case 0x0132:
			printf("smif_0x0132: Get EV file sys status\n");
			return SmifPkt_0132(recv, recv_len, resp, resp_len);

		case 0x0139:
			printf("smif_0x0139: Get security state response\n");
			return SmifPkt_0139(recv, recv_len, resp, resp_len);

		case 0x0143:
			printf("smif_0x0143: BIOS POST State response\n");
			return SmifPkt_0143(recv, recv_len, resp, resp_len);

		case 0x0150:
			printf("smif_0x0150: Reponse Read CPLD Shutdown & Power Fault\n");
			return SmifPkt_0150(recv, recv_len, resp, resp_len);
		default:
			break;
	}
	printf("Bad Smif Packet\n");
	return SmifPkt_badcmd(recv, recv_len, resp, resp_len);
}
