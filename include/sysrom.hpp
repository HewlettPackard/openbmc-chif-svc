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

#ifndef __SYSROM_H__
#define __SYSROM_H__

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "chif.hpp"

// The following structures are taken from the smbios-mdr submodule in openbmc

struct MDRSMBIOSHeader
{
    uint8_t dirVer;
    uint8_t mdrType;
    uint32_t timestamp;
    uint32_t dataSize;
}__attribute__((packed));

typedef struct {
    uint8_t majorVersion;
    uint8_t minorVersion;
} SMBIOSVersion;

struct EntryPointStructure30 {
    uint8_t anchorString[5];
    uint8_t epChecksum;
    uint8_t epLength;
    SMBIOSVersion smbiosVersion;
    uint8_t smbiosDocRev;
    uint8_t epRevision;
    uint8_t reserved;
    uint32_t structTableMaxSize;
    uint64_t structTableAddr;
}__attribute__((packed));

int RomHandler(void *recv, void *resp, int resp_len);
int WriteSmbiosRecords(char *path, char *buffer, int size);
bool syncSmbiosData();

#endif
