/*
// Copyright (c) 2003, 2016, 2021-2025 Hewlett Packard Enterprise Development, LP
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

#include "chif.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <thread>
#include "smbios.hpp"
#include "cfg_smbios.hpp"
#include "misc.hpp"

// Globals
int smbios_n_received;
int smbios_n_filtered;
int smbios_n_dropped;
int smbios_n_bytes;
int smbios_rec_longest;

///////////////////////////////////////////////

// Array of hashes to identify when we transfer to BlackBox or not
unsigned int smbios_bb_last_smbios_type, smbios_bb_instance;

///////////////////////////////////////////////

// Statistic Information
static struct {
   UINT8 r; // received, stored, filtered, dropped
   UINT8 s; // received, stored, filtered, dropped
   UINT8 f; // received, stored, filtered, dropped
   UINT8 d; // received, stored, filtered, dropped
} smbios_rec_stats[256];

extern smbios_cfg_type smbios_db; 
static smbios_cfg_type smbios_load_db; 

/* private (internal use) API */
//void smbios_dump_stats(void);

/* smbios_data_begin()
 *
 * This routine is called by romchf when the SMBIOS begin message is received.
 * This message is received just before the records are transmitted.
 */
void  smbios_data_begin(void)
{
//    smbios_post_output = svcsilo_smbios_auto_stats_get();
//    if (smbios_post_output) {
//       ddbPrintf("smbios: incoming data.\n");
//    }

    smbios_cfg_default(&smbios_load_db);
    smbios_n_received = smbios_n_filtered = smbios_n_dropped = smbios_n_bytes = 0;

    // Reference in romchf_bb.c
    smbios_bb_last_smbios_type = 0xFFFFFFFF;
    smbios_bb_instance = 0;

    memset(smbios_rec_stats, 0, sizeof(smbios_rec_stats));
    smbios_rec_longest = 0;
}

/* smbios_dump_stats()
 *
 * Output statistics about stored records
 */
void smbios_dump_stats(void)
{
   unsigned int i;

   /* SMBIOS upload statistics */
   dbPrintf("\r\n"
           "SMBIOS received stats since last POST.  Database @0x%lx   Writecount:%u\n"
           "        Received %d  Filtered %d  Dropped %d  Gross Bytes %d Longest %d\n"
           "Type    Rec  Saved   Drop  Filtr\n",
           (ulong)&smbios_db, smbios_cfg_get_writecount(),
           smbios_n_received, smbios_n_filtered, smbios_n_dropped, smbios_n_bytes, smbios_rec_longest);
   for (i=0;i<sizeof(smbios_rec_stats)/(sizeof(smbios_rec_stats[0])); i++) {
      if (smbios_rec_stats[i].r) {
         dbPrintf(" %3d    %3d    %3d    %3d    %3d  \"%s\"\n",
           i, smbios_rec_stats[i].r, smbios_rec_stats[i].s,
           smbios_rec_stats[i].d, smbios_rec_stats[i].f,
           smbios_type_string ((int)i));
      }
   }
   /* seek the end of the index to determine how full it is */
   for (i=0;i<SMBIOS_NUM_REC; i++) {
      if (smbios_db.index[i].length == 0) {
         break;
      }
   }
   dbPrintf("smbios resource use- index/database:  %d%%/%d%%\n",
         (100 * i) / SMBIOS_NUM_REC, (100 * smbios_db.insert) / SMBIOS_SZ_DATA);
   std::this_thread::sleep_for(std::chrono::microseconds(100));
   //usleep(100);
}

/* smbios_data_end()
 *
 * This routine is called by romchf when the SMBIOS end message is received.
 * This message is received after all the records have been transmitted.
 */
void  smbios_data_end(void)
{
//    if (smbios_post_output) {
//       smbios_dump_stats();
//    }

    if (smbios_cfg_write(&smbios_load_db)) {
        printf("file write FAILED!\n");
    }

    if (smbios_cfg_read(&smbios_load_db)) {
        printf("file read FAILED!\n");
    }

    if (memcmp(&smbios_db, &smbios_load_db, sizeof(smbios_db))) {
        printf("Data miscompare!\n");
    }

//    /* perform  ROM test now that we have current ROM info */
//    diag_run(DIAG_HOST_ROM);
//    diag_run(DIAG_CPLD);
//    diag_run(DIAG_PMC);
}


/* smbios_data_record()
 *
 * Called by ROMCHF when an SMBIOS record is received from RBSU.
 * This is called one time for each SMBIOS record located in host ROM.
 *
 * p is a pointer to the beginning of the record data.
 * len is the number of bytes of data in the record - not the value stored
 * in the record header, because this does not include the unformatted string
 * region.
 */
void  smbios_data_record(void *p, int len)
{
    int             i, ii;
    SMBIOS_HDR      *h;

    if ((!p) || (!len)) {
        printf("bad parm: 0x%p; (%d bytes)\n", p, len);
        return;
    }

    smbios_n_received ++;
    smbios_n_bytes += len;

    if (len > smbios_rec_longest) smbios_rec_longest = len;

    h = (SMBIOS_HDR *)p;
     
    smbios_rec_stats[h->type].r ++;
        
    /* implement record filtering .
     * These records are filtered because iLO does not use them and because on
     * some platforms like the DL740, the database fills with them. */
    switch (h->type) {
//        case   8: /* Port Connector Information */
//        case  19: /* Memory Array Mapped Address */
//        case  20: /* Memory Device Mapped Address */
        case 218: /* Invalidated record (used to delete other unusued records) */
            smbios_rec_stats[h->type].f ++;
            smbios_n_filtered ++;
            dbPrintf("FILTERED Type %d\n", h->type);
            return;
        default: /* do not filter */
            break;
    }
    
    /* Prepare to store.
     * Test for room in the index */
    for (i=0;i<SMBIOS_NUM_REC; i++) {
        if (smbios_load_db.index[i].length == 0) {
            break;
        }
    }
    if (i == SMBIOS_NUM_REC) {
       smbios_rec_stats[h->type].d ++; /* SMBIOS.C
 *
 *  Copyright 2003, 2016 Hewlett Packard Enterprise Development, LP
 *
 *  Hewlett-Packard and the Hewlett-Packard logo are trademarks of
 *  Hewlett-Packard Development Company, L.P. in the U.S. and/or other countries.
 *
 *  Confidential computer software. Valid license from Hewlett Packard Enterprise is required for
 *  possession, use or copying. Consistent with FAR 12.211 and 12.212,
 *  Commercial Computer Software, Computer Software Documentation, and Technical
 *  Data for Commercial Items are licensed to the U.S. Government under
 *  vendor's standard commercial license.
 */
       dbPrintf("DROPPED  Type %d (index full)\n", h->type);
       smbios_n_dropped ++;
       return;
    }
    /* test for room in the data */
    if ((smbios_load_db.insert + len) > SMBIOS_SZ_DATA) {
       smbios_rec_stats[h->type].d ++;
       dbPrintf("DROPPED  Type %d (storage full)\n", h->type);
       smbios_n_dropped ++;
       return;
    }

//    smbios_rec_stats[h->type].s ++;

    /* move insert point in record buffer */
    ii = smbios_load_db.insert;
    dbPrintf("type: %02x, ii = %d", (UINT8)((smbios_rec_header *)p)->type, ii);
    smbios_load_db.insert += (len%4)?len+(4-(len%4)):len; //always start at 4-byte boundary
    /* record the the record data */
    memcpy(&smbios_load_db.data[ii], p, (unsigned)len);
    /* update the index; length field last and use data length, not header value */
    smbios_load_db.index[i].type   = h->type;
    smbios_load_db.index[i].handle = h->handle;
    smbios_load_db.index[i].offset = ii;
    smbios_load_db.index[i].length = len;
}

