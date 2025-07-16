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

/****************************************************************************
*
*  strutil.h
*     Header file for strutil.c.
*
****************************************************************************/

#ifndef STRUTIL_H
#define STRUTIL_H

#include "chif.hpp"

/* strutil_icmp() Performs case-insensitive string comparison */
extern int strutil_icmp(const char* str1, const char* str2);

/******************************************************************************/
/* Exported Functions:                                                        */
/******************************************************************************/

// hexdump bytes - with address
extern int  hexdump_b(void *p, int len);

// hexdump bytes - without address
extern int  hexdump(void *p, int len);

// hexdump bytes - hanging labeled indent without address
extern int  hexdump_b_hind(void *p, int len, char *label);

#endif

