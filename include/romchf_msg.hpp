/*
// Copyright (c) 2003, 2018-2025 Hewlett Packard Enterprise Development, LP
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

#ifndef ROMCHF_MSG_H
#define ROMCHF_MSG_H

#include "romchfservice.hpp"
#include "cfg_smbios.hpp"
#include "smbios.hpp"

#define ROMCHFSERVICE_ERRCODE_BASE 1000        // added to VCOM failure code

typedef enum {
  ROMCHF_MT_REQ_INVALID = 0,
  ROMCHF_MT_REQ_SMBIOS_GET_DIMM,              // Retrive DIMM [instance] data
  ROMCHF_MT_REQ_SMBIOS_GET_MB,                // Retrive millbrook [instance] data
  ROMCHF_MT_REQ_SMBIOS_GET_REC_BY_TYPE_AND_POSITION,
  ROMCHF_MT_REQ_SMBIOS_GET_REC_BY_HANDLE,
  ROMCHF_MT_REQ_SMBIOS_GET_REC_BY_TYPE_AND_HANDLE,
  ROMCHF_MT_REQ_SMBIOS_GET_DIMM_LOC,          // Retrive DIMM Location data ONLY

  ROMCHF_MT_REQ_SMBIOS_TYPE_STRING = 20,    //char *smbios_type_string(int type)
  ROMCHF_MT_REQ_SMBIOS_WRITECOUNT,
  ROMCHF_MT_REQ_ROMCHF_STATISTICS,
  ROMCHF_MT_REQ_SMBIOS_STATISTICS
} romchf_req_msg_type;

typedef enum {
  ROMCHF_MT_REP_INVALID = 0,
  ROMCHF_MT_REP_SMBIOS_GET_DIMM,             // Retrive DIMM [instance] data
  ROMCHF_MT_REP_SMBIOS_GET_MB,               // Retrive millbrook [instance] data
  ROMCHF_MT_REP_SMBIOS_GET_REC_BY_TYPE_AND_POSITION,
  ROMCHF_MT_REP_SMBIOS_GET_REC_BY_HANDLE,
  ROMCHF_MT_REP_SMBIOS_GET_REC_BY_TYPE_AND_HANDLE,
  ROMCHF_MT_REP_SMBIOS_GET_DIMM_LOC,          // Retrive DIMM Location data ONLY

  ROMCHF_MT_REP_SMBIOS_TYPE_STRING = 20,     //char *smbios_type_string(int type)
  ROMCHF_MT_REP_SMBIOS_WRITECOUNT,
} romchf_rep_msg_type;

typedef struct {
   int num;
   UINT8 type;
} ROMCHF_REQ_SMBIOS_GET_DIMM;

typedef struct {
   UINT8 device_type;
   int   position;
   UINT32 szbuf;          //size of receiving buffer
} ROMCHF_REQ_SMBIOS_GET_REC_BY_TYPE_AND_POSITION;

typedef struct {
   UINT16 handle;
   UINT32 szbuf;          //size of receiving buffer
} ROMCHF_REQ_SMBIOS_GET_REC_BY_HANDLE;

typedef struct {
   UINT8  device_type;
   UINT16 handle;
   UINT32 szbuf;          //size of receiving buffer
} ROMCHF_REQ_SMBIOS_GET_REC_BY_TYPE_AND_HANDLE;

typedef struct {
   int  type;
} ROMCHF_REQ_SMBIOS_TYPE_STRING;

// ROMCHF service request message envelope
typedef struct {
   romchf_req_msg_type msgtype;
   UINT8 msgid;
   union {
     ROMCHF_REQ_SMBIOS_GET_DIMM                     get_dimm;
     ROMCHF_REQ_SMBIOS_GET_REC_BY_TYPE_AND_POSITION rec_by_type_and_position;
     ROMCHF_REQ_SMBIOS_GET_REC_BY_HANDLE            rec_by_handle;
     ROMCHF_REQ_SMBIOS_GET_REC_BY_TYPE_AND_HANDLE   rec_by_type_and_handle;
     ROMCHF_REQ_SMBIOS_TYPE_STRING                  type_string;
   } request;
} ROMCHF_REQ;

typedef struct {
   int status;
   int smbios_writecount;
   int num;
   type_dimm dimm;
} ROMCHF_REP_SMBIOS_GET_DIMM;

typedef struct {
   int   status;
   int   smbios_writecount;
   int   position;
   UINT8 device_type;
   UINT8 length;  // the TRUE size of data as stored (unless > buffer size provided)
   UINT8 retbuf[SMBIOS_MAX_RECSIZE];
} ROMCHF_REP_SMBIOS_GET_REC_BY_TYPE_AND_POSITION;

typedef struct {
   int status;
   UINT8 retbuf[SMBIOS_MAX_RECSIZE];
} ROMCHF_REP_SMBIOS_GET_REC_BY_HANDLE;

typedef struct {
   int status;
   int smbios_writecount;
   UINT16 handle;
   UINT8  device_type;
   UINT8  retbuf[SMBIOS_MAX_RECSIZE];
} ROMCHF_REP_SMBIOS_GET_REC_BY_TYPE_AND_HANDLE;

typedef struct {
   int  type;
   char name[100];
} ROMCHF_REP_SMBIOS_TYPE_STRING;

typedef struct {
   int num;
} ROMCHF_REP_WRITECOUNT;

// ROMCHF service reply message envelope
typedef struct {
  romchf_rep_msg_type msgtype;
  UINT8 msgid;
  union {
    ROMCHF_REP_SMBIOS_GET_DIMM      get_dimm;
    ROMCHF_REP_SMBIOS_GET_REC_BY_TYPE_AND_POSITION rec_by_type_and_position;
    ROMCHF_REP_SMBIOS_GET_REC_BY_HANDLE rec_by_handle;
    ROMCHF_REP_SMBIOS_GET_REC_BY_TYPE_AND_HANDLE rec_by_type_and_handle;
    ROMCHF_REP_SMBIOS_TYPE_STRING    type_string;
    ROMCHF_REP_WRITECOUNT            writecount;
  } reply;
} ROMCHF_REP;


#endif

