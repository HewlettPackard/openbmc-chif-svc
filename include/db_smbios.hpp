/*
// Copyright (c) 2009-2025 Hewlett Packard Enterprise Development, LP
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

#ifndef db_smbios_h
#define db_smbios_h

/**********************************************************************************************************
*
**********************************************************************************************************/
extern void db_smbios_help(void);
extern void db_smbios_handler( int argc, const char argv[][24] );

extern int  smbios_rec_dump(void *p);
extern int  smbios_rec_decode(void *p);

#endif
