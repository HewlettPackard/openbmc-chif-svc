/*
// Copyright (c) 2021-2025 Hewlett-Packard Enterprise Development, LP
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
#include <string.h>
#include <strings.h>

#include "platdef.h"
#include "platdef_api.hpp"

#define I2C_ENGINE_COUNT                    10
#define I2C_STANDARD_ENGINES                9       //This does not include DDC and SNA engine
#define I2C_SEGMENT_COUNT                   255


/*
This is used to select the root engine's
connection that corresponds to the desired segment
*/
unsigned char                   segment_to_engine[I2C_SEGMENT_COUNT];

PlatDefI2CEngine                apml_root_engines[I2C_ENGINE_COUNT];
PlatDefI2CSegment               apml_segments[I2C_SEGMENT_COUNT];

void update_i2c_topology(void)
{
    int         i, j;
    PlatDefI2CEngine * apml_i2c_engine_record;
    PlatDefI2CSegment * apml_i2c_segment_record;

    memset(segment_to_engine, 0xFF, sizeof(segment_to_engine));

    for (i = 0; (apml_i2c_engine_record = i2c_engine_by_index(i)); i++)
    {
        if (i >= I2C_ENGINE_COUNT)
        {
            //
            printf("UIT: APML error QUIX this! Too many apml engine records!\n");
            break;
        }

        if (i != apml_i2c_engine_record->ID)
        {
            //
            printf("UIT: APML error QUIX this! Skipping record! Engine index does not match ID. Index: %d ID: %d\n", i, apml_i2c_engine_record->ID);
            continue;
        }

        segment_to_engine[i] = i;

        /* Copy the engine into the root engines array */
        memcpy(&(apml_root_engines[i]), apml_i2c_engine_record, sizeof(apml_root_engines[i]));

        /* Add the segments attached to this root engine to our segment map */
        for (j = 0; j < apml_i2c_engine_record->Count; j++)
        {
            apml_i2c_segment_record = &apml_i2c_engine_record->Segments[j];

            /*
            If the I2CFlag_IgnoreSegment bit is set ignore this segment and do not add it to our map
            */
            if (apml_i2c_segment_record->Flags & I2CFlag_IgnoreSegment)
            {
                continue;
            }

            /* Set this segments root engine in the segment to engine map */
            segment_to_engine[apml_i2c_segment_record->ID] = apml_i2c_engine_record->ID;

            /* Copy the segment into our segments array */
            memcpy(&(apml_segments[apml_i2c_segment_record->ID]), apml_i2c_segment_record, sizeof(apml_segments[apml_i2c_segment_record->ID]));      
        }
    }

    /*
    Clear empty entries
    */
    for (j = 0; j < I2C_SEGMENT_COUNT; j++)
    {
        if (segment_to_engine[j] == 0xFF)
        {
            memset(&(apml_segments[j]), 0, sizeof(PlatDefI2CSegment));
        }
    }
}
