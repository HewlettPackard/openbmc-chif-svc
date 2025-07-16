/*
// Copyright (c) 2001-2025 Hewlett Packard Enterprise Development, LP
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

#ifndef _GENERIC_DECODER_H
#define _GENERIC_DECODER_H

#include "DataExtract.h"
#include "logs.h"

typedef enum {
    EVT_GEN_DATA_ID = 0,
    EVT_ACT_DATA_ID = 1,
    EVT_DESC_DATA_ID = 2,
    EVT_VAR_DATA_ID = 3,
    EVT_TRAP_DATA_ID = 4,
    EVT_RIS_ALERT_DATA_ID = 5,
    EVT_SER_EVENT_DATA_ID = 6,
    EVT_DATA_MAX
} DATA_EXTRACT_ID;

#endif // _GENERIC_DECODER_H
