/*
// Copyright (c) 2001, 2016-2025 Hewlett Packard Enterprise Development, LP
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
#include <string>
#include <cstring>
#include <strings.h>
#include <unistd.h>

#include "chif.hpp"
#include "platdef.h"
#include "platdef_api.hpp"
#include "uefi.hpp"
#include "uefi_util.hpp"
#include "zlib.h"
#include "strutil.hpp"
#include "misc.hpp"


// Global Variables
extern UINT8 platdef[PLATDEF_UPDATE_BUF_SZ + PLATDEF_BLOB_START];

extern int uncompress(Bytef * dest, uLongf * destLen, const Bytef * source, uLong sourceLen);

/***********************************************************
 * uefi_util_platdef_store:
 *
 * This API finds the APML FW volume in BIOS Image.
 * Once it finds the APML FW volume it copies the
 * APML content into UEFI Var Store.
 *
 * Also the offset and sizes are updated in NVRAM
 * config file for use while loading Platdef
 ***********************************************************/

UEFI_RC uefi_util_platdef_store(void)
{
    FILE *fptr;
    UEFI_RC rc=UEFI_RC_OK;
    int read_count = 0, i;
    PlatDefTableData* td;
    int zip_rc;
    unsigned long exp_size, comp_size;
    UINT8 *records_ptr;
    void *platdef_tmp;

    dbPrintf("Check for platdef file.\n");

    // the compressed platdef data is in a file
    // so all we need to do is read it in and uncompress it into the memory buffer.

    // clear all of the platdef buffer
    memset(platdef, 0, sizeof(platdef));
    // assign the pointer to where the uncompressed platdef data will start
    records_ptr = (UINT8 *)(platdef + PLATDEF_BLOB_START);

    dbPrintf("platdef address: %px, sizeof platdef: %lx\n", static_cast<void*>(platdef), sizeof(platdef));
    dbPrintf("records_ptr: %p\n", static_cast<void*>(records_ptr));

    // allocate temporary buffer to hold the compressed data
    platdef_tmp = malloc(PLATDEF_UPDATE_BUF_SZ);
    if (platdef_tmp == NULL) {
        printf("PLATDEF: Could not allocate platdef temp buffer\n");
        return UEFI_RC_ERROR;
    }

    dbPrintf("clearing platdef_tmp\n");
    memset(platdef_tmp, 0, PLATDEF_UPDATE_BUF_SZ);

    dbPrintf("opening platdef file\n");
    fptr = fopen(PLATDEF_DATA_FILE, "rb");
    if (!fptr) {
        printf("PLATDEF: Failed to open rom.bin file\n");
        free(platdef_tmp);
        return UEFI_RC_ERROR;
    }

    // seek past the initial header.
    if (fseek(fptr, 32, SEEK_SET)) {
        printf("PLATDEF: Failed to seek to data in platdef file\n");
        free(platdef_tmp);
        fclose(fptr);
        return UEFI_RC_ERROR;
    }

    dbPrintf("Attempting to read %d bytes from platdef file\n", PLATDEF_UPDATE_BUF_SZ);
    // read in the apml data.  size was calculated in the find file routine.
    read_count = fread((UINT8 *)platdef_tmp, 1, PLATDEF_UPDATE_BUF_SZ, fptr);
    dbPrintf("Read count from platdef file: %d\n", read_count);

    fclose(fptr);

    dbPrintf("retrieved compressed data\n");
    hexdump(platdef_tmp, 0x100);

    // now uncompress the data
    td = (PlatDefTableData*) platdef_tmp;

    dbPrintf("again with td\n");
    hexdump(td, 0x100);

    dbPrintf("PlatDefTableData:\n");
    dbPrintf("header: 32 bytes \n");
    dbPrintf("Description: %s\n", td->Description);

    dbPrintf("Description (32):\n"); for(i=0;i<16;i++) { dbPrintf("%c", td->Description[i]);} dbPrintf("\n");
                                   for(;i<32;i++) { dbPrintf("%c", td->Description[i]);} dbPrintf("\n");
    dbPrintf("Flags: %04x", td->Flags);
    dbPrintf("MajorVersion: %02x\n", td->MajorVersion);
    dbPrintf("MinorVersion: %02x\n", td->MinorVersion);
    dbPrintf("BuildTimestamp: %x", td->BuildTimestamp);
    dbPrintf("RecordCount: %x\n", td->RecordCount);
    dbPrintf("TotalSize: %x\n", td->TotalSize);
    dbPrintf("MD5Hash:\n"); for (i = 0; i < 16; i++) { dbPrintf("%c", td->MD5Hash[i]);} dbPrintf("\n");
    dbPrintf("CompressedSize: %x\n", td->CompressedSize);
    dbPrintf("SpecialVersion: %02x\n", td->SpecialVersion);
    dbPrintf("BuildVersion: %02x", td->BuildVersion);
    dbPrintf("reserved 10: \n"); for (i = 0; i < 10; i++) { dbPrintf("%c", td->_Reserved[i]);}
    dbPrintf("\n");
    
    exp_size = PLATDEF_UPDATE_BUF_SZ-sizeof(PlatDefTableData);
    dbPrintf("exp_size: %lx\n", exp_size);
    if(td->CompressedSize <  sizeof(PlatDefTableData)) //Invalid case so return with error
    {
        printf("PLATDEF: Invalid case : compressed size %d is less than the size of table data %ld\n", td->CompressedSize, sizeof(PlatDefTableData));
        free(platdef_tmp);
        return UEFI_RC_ERROR;
    }
    comp_size = td->CompressedSize - sizeof(PlatDefTableData);

    dbPrintf("comp_size: %lx, \n", comp_size);

    // Uncompress after TableData record
    zip_rc = uncompress( (Bytef*)(records_ptr+sizeof(PlatDefTableData)), &exp_size,
                         (const Bytef*)((UINT8 *)platdef_tmp+sizeof(PlatDefTableData)), comp_size );

    if( !zip_rc ) {
        dbPrintf("PlatDef uncompress successful\n");
        dbPrintf("PlatDef uncompressed size: %lx\n", exp_size);
        // Copy TableData record
        memcpy(records_ptr, platdef_tmp, sizeof(PlatDefTableData));

        dbPrintf("blob\n");
        hexdump(platdef + PLATDEF_BLOB_START, 0x100);

        if (platdef_meta_load()) {
            printf("PLATDEF: platdef data failed to parse - possible bad format\n" );
            rc = UEFI_RC_ERROR;

        } else {
            dbPrintf("platdef initialized!\n");
        }
        dbPrintf("meta\n");
        hexdump(platdef, 0x100);
        dbPrintf("blob\n");
        hexdump(platdef + PLATDEF_BLOB_START, 0x100);

    } else {
        printf("PLATDEF: PlatDef uncompress failed, error: %d\n", zip_rc);

        dbPrintf("meta\n");
        hexdump(platdef, 0x100);
        dbPrintf("blob\n");
        hexdump(platdef + PLATDEF_BLOB_START, 0x100);

        rc = UEFI_RC_ERROR;
    }

    free(platdef_tmp);
    return rc;
}
