/*
// Copyright 2003, 2016-2025 Hewlett Packard Enterprise Development, LP
//
// Hewlett-Packard and the Hewlett-Packard logo are trademarks of Hewlett-Packard
// Development Company, L.P. in the U.S. and/or other countries.
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

#ifndef CFG_SMBIOS_H
#define CFG_SMBIOS_H

#define SMBIOS_MAX_RECSIZE    (256)  // Longest individual record supported
#define SMBIOS_MAX_TYPESTRING (128)  // Longest type description
#define SMBIOS_NUM_REC       (1000) 
#define SMBIOS_CFG_VERSION      (3)
/* empirical evidence shows roughly 35 bytes/record average */
#define SMBIOS_SZ_DATA (35*SMBIOS_NUM_REC)
#define SMBIOS_MAX_COMPRESSED_FILE  16*1024

#define SMBIOS_DATA_FILE  "/tmp/tmp_smbiosdata"     // temporary new smbios data file

/* structures */
/* The smbios_db contains an array of record indexes for quickly locating
 * individual records within the data buffer.
 *
 * If an index length is 0 then the element is not valid.
 *
 * Records are packed into the data section contiguously.  Index offset and
 * length are used to identify record boundaries.
 *
 * type:    SMBIOS record type
 * length:  number of bytes of data in the record
 * handle:  SMBIOS unique record handle
 * offset:  offset from the beginning of data where the record begins (DWORD aligned)
 * data:    SMBIOS records
 * insert:  offset from the beginning of data for new data
 */
typedef struct {
    int type;                   /* SMBIOS record type.  See smbios.h */
    int length;                 /* actual length of record data. 0 indicates end of data */
    int handle;                 /* SMBIOS record unique handle */
    int offset;                 /* offset (4-byte aligned) to record data from data start */
} smbios_rec_header;

typedef struct {
    UINT32 ver;                     /* version number starts at "3" */
    UINT32 size;                    /* size (in bytes) of structure - for sufficient malloc */
    UINT32 num;                     /* number of records supported by index */
    UINT32 capacity;                /* number of bytes in free-form data section */
    UINT32 writes;                  /* number of file writes */
    smbios_rec_header index[SMBIOS_NUM_REC];  /* index into SMBIOS data */
    char data[SMBIOS_SZ_DATA];      /* packed binary data */
    int insert;                     /* insert offset for additional records */
} smbios_cfg_type;


/***********************************************************************
* Function Prototypes
************************************************************************/
extern int   smbios_cfg_get_writecount( void );
extern int   smbios_cfg_read_into_globalvar(void);
extern int   smbios_cfg_read ( smbios_cfg_type * sdr );
extern int   smbios_cfg_write( const smbios_cfg_type * sdr );
extern void  smbios_cfg_default(  smbios_cfg_type *sdb);

#endif

