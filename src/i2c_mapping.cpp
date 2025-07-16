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

#include <stdio.h>
#include <stdlib.h>
#include "chif.hpp"
#include "i2c_mapping.hpp"
#include "misc.hpp"

struct i2cMapEntry i2cSystemEntries[MAX_I2C_TABLE_REMAP];
int i2cAllocatedEntries=0;

void load_i2c_mapping()
{
    FILE *mapping;
    char input[1024];
    dbPrintf("loading i2c mapping file\n");
    mapping=fopen(I2C_MAPPING,"r");
    if ( mapping != NULL )
    {
        while(fgets(input, 1024, mapping) != NULL )
        {
            if ( i2cAllocatedEntries < MAX_I2C_TABLE_REMAP )
            {
                sscanf(input,"%hx %hu %hu\n", &i2cSystemEntries[i2cAllocatedEntries].cpldReg,
                               &i2cSystemEntries[i2cAllocatedEntries].RegVal,
                               &i2cSystemEntries[i2cAllocatedEntries].i2cKernelSegment);
                i2cAllocatedEntries++;
            }
        }
        fclose(mapping);
    }
    else
        printf("I2CMAP: i2c mapping file do not exist\n");
}
