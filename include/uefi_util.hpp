/*
// Copyright (c) 2001-2002, 2021-2025 Hewlett Packard Enterprise Development, LP
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

// This mutex is used to synchronize the API to hold off VASCOM calls during
// time critical periods
#define DATE_TIME_STR_MAX_SIZE       30
#define UEFI_ROM_IMAGE "/tmp/rom.bin"


extern UEFI_RC uefi_util_file_find_with_retries( const UEFI_REDROM_SIDE, const EFI_GUID* pFwVolGUID, const EFI_GUID* pFwFileGUID, 
                        UINT32* pOffset, UINT32* pSize, UINT8* pChecksum, 
                        unsigned int retry_count, unsigned int ms_retry_interval );
extern UEFI_RC uefi_util_platdef_store(void);
