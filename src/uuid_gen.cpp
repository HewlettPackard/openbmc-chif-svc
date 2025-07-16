/*
// Copyright (c) 20010-2013, 2025 Hewlett Packard Enterprise Development, LP
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

#include <string.h>
#include <stdio.h>

#include "smbios.hpp"

#include "uuid_gen.hpp"


/*
 * Obtain the UUID in canonical form. 
 *  
 * Parameters 
 *   [out] uuid_str - String buffer to store the UUID in canonical form.
 *                    Make sure the string buffer is atleast 40 bytes in size.
 *   [in]  us_len - The maximum number of bytes to be used in the buffer.
 *   [in]  uuid - The UUID which needs to be stored in canonical form.
 *  
 */
void uuid_get_canonical(char* uuid_str, size_t us_len, PUUID_T uuid)
{
    if ((uuid == NULL) || (uuid_str == NULL))
    {
        return;
    }

    // save canonical string form: cdef0d64-beef-c0de-ebad-3d95297c31f8
    snprintf(uuid_str, us_len, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            uuid->octet[0], uuid->octet[1], uuid->octet[2], uuid->octet[3],
            uuid->octet[4], uuid->octet[5],
            uuid->octet[6], uuid->octet[7],
            uuid->octet[8], uuid->octet[9],
            uuid->octet[10],uuid->octet[11],uuid->octet[12],uuid->octet[13], uuid->octet[14], uuid->octet[15]);
}


