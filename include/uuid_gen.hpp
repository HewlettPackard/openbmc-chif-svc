/*
// Copyright (c) 2010-2013, 2021-2025 Hewlett Packard Enterprise Development, LP
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

#ifndef UUID_GEN_H
#define UUID_GEN_H

#define MAX_UUID_LEN 16

typedef struct _UUID_T {
    UINT8  octet[MAX_UUID_LEN];
} UUID_T, *PUUID_T;

void uuid_get_canonical(char* uuid_str, size_t us_len, PUUID_T uuid);

#endif

