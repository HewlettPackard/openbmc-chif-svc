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

#define I2C_MAPPING "/tmp/ubm/ubm_map.txt"
#define MAX_I2C_TABLE_REMAP 256


struct i2cMapEntry {
	unsigned short int cpldReg;
	unsigned short int RegVal;
	unsigned short int i2cKernelSegment;
};

extern struct i2cMapEntry i2cSystemEntries[MAX_I2C_TABLE_REMAP];
extern int i2cAllocatedEntries;

void load_i2c_mapping();
