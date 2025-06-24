/*
// Copyright (c) 2001-2025 Hewlett-Packard Enterprise Development, LP
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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "cfg_smbios.hpp"
#include "misc.hpp"

static int smbios_cfg_writecount = -1; //  <0: no writes and no successful reads
                                // >=0: had successful writes or reads since boot AND
                                // smbios_db contains latest smbios records
                                // can only be changed by routines in this file
smbios_cfg_type smbios_db;      // should only be changed by routines in this file


/*****************************************************************************
*
*****************************************************************************/
int smbios_cfg_get_writecount( void )
{
    return smbios_cfg_writecount;
}

/* smbios_cfg_read_into_globalvar()
 * 
 * read smbios cfg into global variable smbios_db
 */
int smbios_cfg_read_into_globalvar()
{
   int wipe = 0;

   dbPrintf("SMBIOS Loading\n");

   if (!smbios_cfg_read(&smbios_db)) {
      if ((smbios_db.ver      == SMBIOS_CFG_VERSION) && 
          (smbios_db.size     == sizeof(smbios_cfg_type)) &&
          (smbios_db.num      == SMBIOS_NUM_REC) &&
          (smbios_db.capacity == SMBIOS_SZ_DATA)) {
          /* file content matches code base */
          dbPrintf("SMBIOS DB OK\n");
      } else {
         dbPrintf("version difference!  (file %d %d %d %d) != (fw %d %d %d %d)\n",
               smbios_db.ver, smbios_db.size, smbios_db.num, smbios_db.capacity,
               (UINT32)SMBIOS_CFG_VERSION, (UINT32)sizeof(smbios_cfg_type), (UINT32)SMBIOS_NUM_REC, (UINT32)SMBIOS_SZ_DATA);
         wipe = 1;
      }

      if (smbios_cfg_writecount < 0) {
         smbios_cfg_writecount = 0;
      }
   } else {
      dbPrintf("Failed to read smbios data, defaulting.\n");
      wipe = 1;
   }
   
   if (wipe) {
      printf("SMBIOS DB defaulted\n");
      smbios_cfg_default(&smbios_db);
   }

   return 0;
}


/* smbios_cfg_read()
 *
 * Read config data into provided buffer
 */
int smbios_cfg_read( smbios_cfg_type *smbios_cfg_ptr )
{
    FILE *fd = NULL;
    int   wipe = 0;
    size_t bytesToRead = sizeof(smbios_cfg_type);
    size_t bytesRead;

    fd = fopen(SMBIOS_DATA_FILE,"rb");
    if (fd !=NULL) {
        bytesRead = fread(smbios_cfg_ptr, 1, bytesToRead, fd);
        if (bytesRead == sizeof(smbios_cfg_type)) {
            if ((smbios_cfg_ptr->ver       == SMBIOS_CFG_VERSION) &&
                (smbios_cfg_ptr->size      == sizeof(smbios_cfg_type)) &&
                (smbios_cfg_ptr->num       == SMBIOS_NUM_REC) &&
                (smbios_cfg_ptr->capacity  == SMBIOS_SZ_DATA)) {
               /* file content matches code base */
            } else {
                /* handle upgrade here.  TODO when upgrade needed */
                dbPrintf(" version difference!  (file %d %d %d %d) != (fw %d %d %d %d)\n",
                    smbios_cfg_ptr->ver, smbios_cfg_ptr->size ,smbios_cfg_ptr->num, smbios_cfg_ptr->capacity,
                    (UINT32)SMBIOS_CFG_VERSION, (UINT32)sizeof(smbios_cfg_type), (UINT32)SMBIOS_NUM_REC, (UINT32)SMBIOS_SZ_DATA);
                wipe = 1;
            }
        } else {
            dbPrintf("Could not read all of smbios data\n");
        }
        fclose(fd);
    } else {
        dbPrintf("Failed to read smbios data; set to defaults\n");
        wipe = 1;
    }
 
    if (wipe) {
        smbios_cfg_default(smbios_cfg_ptr);
    }
 
    return 0;
}

/* smbios_cfg_write()
 *
 * Write the configuration structures to corresponding compressed files
 */
int smbios_cfg_write( const smbios_cfg_type * smbios_cfg_ptr )
{
    int retval = -1;
    FILE *fd = NULL;
    size_t bytesToXfer;
    size_t bytesXfered;

    retval = 0;
    smbios_cfg_writecount++;
    smbios_db = *smbios_cfg_ptr; //sync with global variable

    fd = fopen(SMBIOS_DATA_FILE,"wb+");
    if (fd !=NULL) {
        bytesToXfer = sizeof(smbios_cfg_type);
        dbPrintf("SCW, Writing %ld bytes\n", bytesToXfer);
        bytesXfered = fwrite(smbios_cfg_ptr, 1, bytesToXfer, fd);
        dbPrintf("bytes xferred: %ld\n", bytesXfered);

        if (bytesXfered != bytesToXfer) {
            dbPrintf("Failed: %ld != %ld\n", bytesXfered, bytesXfered);
            dbPrintf("Error writing to %s and bytes written %ld\n", SMBIOS_DATA_FILE, bytesXfered);
            retval = -1;
        }
    
        fclose(fd);
    
    } else {
       printf("SMBIOS Error: opening %s, fd=%ld\n", SMBIOS_DATA_FILE, (uint64_t)fd);
       retval = -1;
    }

    return retval;
}
   
/* smbios_cfg_default()
 *
 * Implemented in a single place
 */
void smbios_cfg_default( smbios_cfg_type *sdb )
{
    dbPrintf("defaulting smbios table\n");
    memset(sdb, 0, sizeof(smbios_cfg_type));
    sdb->ver      = SMBIOS_CFG_VERSION;
    sdb->size     = sizeof(smbios_cfg_type);
    sdb->num      = SMBIOS_NUM_REC;
    sdb->capacity = SMBIOS_SZ_DATA;
}

/*****************************************************************************
*
*****************************************************************************/

