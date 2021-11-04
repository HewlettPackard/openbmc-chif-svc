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

#ifndef __CHIF_H__
#define __CHIF_H__

#include <sys/types.h>
#include <stdint.h>

#define CHIF_PKT_MAX_SIZE		4096
#define SMIF_SERVICE_ID			0
#define ROM_SERVICE_ID			2

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

#endif