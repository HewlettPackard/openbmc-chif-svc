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

#ifndef __CHIF_H__
#define __CHIF_H__

#include <sys/types.h>
#include <stdint.h>

#define CHIF_PKT_MAX_SIZE		4096
#define SMIF_SERVICE_ID			0
#define ROM_SERVICE_ID			2
#define TRITON_SERVICE_ID		12
#define AHS_SERVICE_ID			0x11

#define CHIF_SERVICE_ID_SMIF                0x0000
#define CHIF_SERVICE_ID_IPMI                0x0001
#define CHIF_SERVICE_ID_ROM                 0x0002
#define CHIF_SERVICE_ID_SMIF_FAF            0x0003
#define CHIF_SERVICE_ID_BLACKBOX_FAF        0x0004
#define CHIF_SERVICE_ID_HEALTH              0x0010
#define CHIF_SERVICE_ID_BLACKBOX            0x0011
#define CHIF_SERVICE_ID_EMBMEDIA            0x0012

struct ChifPktHeader {
	uint16_t pkt_size;
	uint16_t sequence;
	uint16_t command;
	uint8_t service_id;
	uint8_t version;
} __attribute__ ((packed));

struct ChifPkt {
	struct ChifPktHeader header;
	uint8_t msg[CHIF_PKT_MAX_SIZE - sizeof(struct ChifPktHeader)];
} __attribute__ ((packed));


typedef uint64_t   UINT64;
typedef uint32_t   UINT32;
typedef uint16_t   UINT16;
typedef uint8_t    UINT8; 

typedef int32_t    INT32;
typedef int16_t    INT16;
typedef int8_t     INT8;


#if !defined(_TYPE_Address)
#define _TYPE_Address	__PTR_BIT
typedef unsigned long	Address;
typedef unsigned long	Value;
#endif

#define FALSE 0
#define TRUE  1

#endif
