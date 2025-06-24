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

/****************************************************************************
*
*  strutil.c
*     Contains miscellaneous string functions.
*
*----------------------------------------------------------------------------
*  Revision History:
*
*  01/17/97  Wesley Ellinger
*     Creation date.
*
****************************************************************************/

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "strutil.hpp"
#include "misc.hpp"


/****************************************************************************
*  strutil_icmp()
*     Compares the two strings (not case sensitive).
*
*  Input parameters:
*     str1: First string to compare.
*     str2: Second string to compare.
*
*  Output:
*     Returns  : <0 = str1 <  str2
*                >0 = str1 >  str2
*                 0 = str1 == str2
****************************************************************************/
int strutil_icmp(const char* str1, const char* str2)
{
    int char1,char2;

    if (!str1 || !str2) return 1; // do not match on a NULL and filter bad input

    for (;;) {
        char1 = (int)((unsigned char) toupper(*str1));
        char2 = (int)((unsigned char) toupper(*str2));

        if (!char1 || !char2 || (char1 != char2)) {
            break;
        }

        str1++;
        str2++;
    }
    return(char1 - char2);
}

/* hexdump_b()
 *
 * byte hex dumper with ASCII decode.  This can be called from other routines
 * instead of coding one up special.
 */
int hexdump_b(void *p, int len)
{
    unsigned char data, *d;
    unsigned char pbuf[17];
    int i;

    if ((p==NULL) || (len==0))
        return(-1);


    d = (unsigned char*)p;
    for ( i=0; i<len; i++, d++ ) {
        if ( i % 16 == 0 )  {
            if ( i ) {
                dbPrintf("  %s\r\n%p  ", pbuf, d);
            } else {
                dbPrintf("%p  ", d);
            }
            memset (pbuf, 0, sizeof(pbuf));
        }
        data = *d;

        dbPrintf("%02x ", data);
        pbuf[i % 16] = (data>31 && data<127) ? data : '.';
    }

    if ( i % 16 ) {
        dbPrintf ("  %*c%s\r\n", 3 * (16 - (i % 16)), ' ', pbuf);
    } else {
        dbPrintf("  %s\r\n", pbuf);
    }
    return(0);
}

/* hexdump_b_hind()
 *
 * byte hex dumper with ASCII decode, no address, hanging indent.
 * This can be called from other routines instead of coding one up special.
 *
 * Rather than generate output incrementally (which is slow with Multi attached),
 * build a one-line buffer and then output it in one step.
 */
int hexdump_b_hind(void *p, int len, char *indent_lbl)
{
    unsigned char data, *d;
    char pbuf[17];
    char chunk[8];
    char line[160];
    size_t szlbl;
    int i;

    if ((p==NULL) || (len==0))
        return(-1);

    if (!indent_lbl) indent_lbl=(char*)"";
    //FIXME: if there is desire to remove this str len, hexdump_b_hind called by at least 8 other source files. 
    szlbl = strlen(indent_lbl);

    d = (unsigned char*)p;
    for ( i=0; i<len; i++, d++ ) {
        if ( i % 16 == 0 )  {
            if ( i ) {
                dbPrintf("%s  %s\r\n", line, pbuf);
            }
            if (i == 0) {
               snprintf(line, sizeof(line)-1, (char*)"%s", indent_lbl);
            } else {
               memset(line, ' ', sizeof(line));
               line[szlbl]=0;
            }
            memset (pbuf, 0, sizeof(pbuf));
        }

        data = *d;
        snprintf( chunk, sizeof(chunk), (char*)"%02x ", data);
        strncat(line, chunk, sizeof(line)-strlen(line)-1);
        pbuf[i % 16] = (data>31 && data<127) ? data : '.';
    }

    if ( i % 16 ) {
        // remainder - pad line with spaces then concatenate pbuf
        // printf ("  %*c%s\r\n", 3 * (16 - (i % 16)), ' ', pbuf);
        for (len=0; len< (16 - (i % 16)); len++) {
           strncat(line, "   ", sizeof(line));
        }
    }

    dbPrintf("%s  %s\r\n", line, pbuf);
    return(0);
}

/* hexdump()
 *
 * byte hex dumper with ASCII decode, no address.  This can be called from other routines
 * instead of coding one up special.
 *
 * Rather than generate output incrementally (which is slow with Multi attached),
 * build a one-line buffer and then output it in one step.
 */
int hexdump(void *p, int len)
{
   int rc;

   rc = hexdump_b_hind(p, len, NULL);
   return rc;
}

