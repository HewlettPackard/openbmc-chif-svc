/*
// Copyright (c) 2013, 2021-2025 Hewlett Packard Enterprise Development, LP
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

#ifndef __PLATDEF_API_H
#define __PLATDEF_API_H

#ifndef APML_BASIC_TYPES_DEFINED  //to avoid platdef.h UINT8 etc. redefine error
#define APML_BASIC_TYPES_DEFINED
#endif 

/* COMMON STUFF */
#include "platdef.h"

/* DEFINES */
#define PLATDEF_MEM_REGION_OBJECT_NUMBER  11
#define PLATDEF_MEM_REGION_SIZE           0x42000
#define PLATDEF_SIZE                      0x40000
#if 1 // GEN12_BOARD
#define PLATDEF_BLOB_START                0x04000
#else
#define PLATDEF_BLOB_START                0x02000
#endif
#define PLATDEF_CHUNK_SIZE                4000
#define PLATDEF_SERVICE_NAME              "platDefService"

// FP100 operations
#define FP100_BUILD( a, b )   ( ((a)*100) + ((b)%100) )
#define FP100_INTEGER( a )    ( (a)/100 )
#define FP100_FRACTION( a )   ( abs(a)%100 )
#define FP100_CEILING( a )    ( ((a)%100) ? (((a)/100) + 1) : ((a)/100) )
#define FP100_MULT( a, b )    ( (((a)*(b))/100 < -32768) ? -32768 : (((a)*(b))/100 > 32767) ? 32767 : ((a)*(b))/100 )
#define FP100_OK( a )         ( ((a) < -32768) ? -32768 : ((a) > 32767) ? 32767 : (a) )

#define PWM_RANGE( a )        ( (a > 255) ? 255 : a )

// blob restrictions and requirements
#define PLATDEF_MAX_RECORDS             2000
#define PLATDEF_MAX_HEALTH_DEVICES      608 
#define PLATDEF_MAX_FAN_PWM             24
#define PLATDEF_MAX_TEMP_SENSOR         256
#define PLATDEF_MAX_POWER_SUPPLY        16
#define PLATDEF_MAX_REDUNDANCY_RULE     8
#define PLATDEF_MAX_DOMAIN_TYPE         8      // Assume max domain type COUNT is 8 so the value range is 0 ~ 7.

// Platdef Source Defines
#define PLATDEF_LOAD_SRC_NONE    (0)
#define PLATDEF_LOAD_SRC_ROM     (1)
#define PLATDEF_LOAD_SRC_NVRAM   (2)
#define PLATDEF_LOAD_SRC_OEM_ROM (3)

// Platdef Ready Defines
#define PLATDEF_NOT_READY        (0)
#define PLATDEF_READY            (1)
#define PLATDEF_FINAL_READY      (2)
  //This indicates that the platdef is done building for the final time for the current boot
  //This build happens after SMBIOS records are received

// Flag values
#define PLATDEF_INDEX_UNINIT           (0xFFFF)
#define PLATDEF_RECORD_NOT_FOUND       (0xFFFE)

/* API Return Codes */
typedef enum    // could be #defines but typed for safety and lint completeness checking
{
    PLATDEF_RC_NO_SUCH_RECORD = 0xFFFF, // for calls to access functions with bad values
    PLATDEF_RC_OK = 0,
    PLATDEF_RC_ERROR,                  // general error
    PLATDEF_RC_BADCOMMAND,             // bad command given
    PLATDEF_RC_BADPARAM,               // bad parameter given
    PLATDEF_RC_BADCONFIG,              // bad configuration for attempted operation
    PLATDEF_RC_VASCOMFAIL,             // Vascom call failure
    PLATDEF_RC_MEM_ACCESS_FAIL,        // PLATDEF shared mem access issue
    PLATDEF_RC_TIMEOUT,                // Internal timeout
    PLATDEF_RC_RETRY,                  // Retry again later
    PLATDEF_RC_UNAVAILABLE,            // Unavailable currently
    PLATDEF_RC_UNINITIALIZED,          // not correctly initialized for this call
    PLATDEF_RC_UNIMPLEMENTED,          // We'll get to it
    PLATDEF_RC_UNSUPPORTED,            // not on this platform
    PLATDEF_RC_VALIDATION_FAIL         // failure to validate primitive
} platdef_rc;

/* the strings must match the return codes order above */
#define PLATDEF_RC_STRINGS(x)        ( \
     (x==PLATDEF_RC_NO_SUCH_RECORD)      ?"PLATDEF_RC_NO_SUCH_RECORD":      \
     (x==PLATDEF_RC_OK)                  ?"PLATDEF_RC_OK":                  \
     (x==PLATDEF_RC_ERROR)               ?"PLATDEF_RC_ERROR":               \
     (x==PLATDEF_RC_BADCOMMAND)          ?"PLATDEF_RC_BADCOMMAND":          \
     (x==PLATDEF_RC_BADPARAM)            ?"PLATDEF_RC_BADPARAM":            \
     (x==PLATDEF_RC_BADCONFIG)           ?"PLATDEF_RC_BADCONFIG":           \
     (x==PLATDEF_RC_VASCOMFAIL)          ?"PLATDEF_RC_VASCOMFAIL":          \
     (x==PLATDEF_RC_MEM_ACCESS_FAIL)     ?"PLATDEF_RC_MEM_ACCESS_FAIL":     \
     (x==PLATDEF_RC_TIMEOUT)             ?"PLATDEF_RC_TIMEOUT":             \
     (x==PLATDEF_RC_RETRY)               ?"PLATDEF_RC_RETRY":               \
     (x==PLATDEF_RC_UNAVAILABLE)         ?"PLATDEF_RC_UNAVAILABLE":         \
     (x==PLATDEF_RC_UNINITIALIZED)       ?"PLATDEF_RC_UNINITIALIZED":       \
     (x==PLATDEF_RC_UNIMPLEMENTED)       ?"PLATDEF_RC_UNIMPLEMENTED":       \
     (x==PLATDEF_RC_UNSUPPORTED)         ?"PLATDEF_RC_UNSUPPORTED":         \
     (x==PLATDEF_RC_VALIDATION_FAIL)     ?"PLATDEF_RC_VALIDATION_FAIL": "Unknown" )

//prototype for func pointer param - so points to function type 'void handler_func(void)'
typedef void (*PLATDEF_CHANGED_HANDLER)(void);

/****************************************************************************
 * PLATDEF API SPECIFIC DEFINES AND DATA STRUCTS - see platdef.h for actual APML data
 ***************************************************************************/
//----------------------------------------------------------------------------
//  PLATDEF_METADATA
//     Contains record struct types as an iLO frontend to APML types
//----------------------------------------------------------------------------
typedef struct
{
    UINT16 first_index;
    UINT16 count;
} RECORD_TYPE_DATA;

typedef struct 
{
    int ready;
    int build_count;

    // Storing a few addresses to important records in here
    //  to avoid having to find them everytime they're needed
    UINT16 battery_index;
    UINT16 hood_index;

    Address records[PLATDEF_MAX_RECORDS];
    UINT16 record_count;

    RECORD_TYPE_DATA table_data;
    RECORD_TYPE_DATA visible_temp_sensor;
    RECORD_TYPE_DATA hidden_temp_sensor;
    RECORD_TYPE_DATA fan_pwm;
    RECORD_TYPE_DATA fan_device;
    RECORD_TYPE_DATA power_supply;
    RECORD_TYPE_DATA redundancy_rule;
    RECORD_TYPE_DATA indicator;
    RECORD_TYPE_DATA power_meter;
    RECORD_TYPE_DATA processor;
    RECORD_TYPE_DATA status;
    RECORD_TYPE_DATA satellite_bmc;
    RECORD_TYPE_DATA fru;
    RECORD_TYPE_DATA association;
    RECORD_TYPE_DATA i2c_engine;
    RECORD_TYPE_DATA dimm_mapping;
    RECORD_TYPE_DATA alt_config;
    RECORD_TYPE_DATA validation;
    RECORD_TYPE_DATA sensor_group;
    RECORD_TYPE_DATA lookup_table;
    RECORD_TYPE_DATA throttle;
    RECORD_TYPE_DATA system_device;
    RECORD_TYPE_DATA peci_segment;
    RECORD_TYPE_DATA patch_and_replacement;

    UINT8* dynamic_record_start;
    UINT8* end_of_table;
} PLATDEF_METADATA;

/****************************************************************************
    PLATDEF INIT  - setup APML shared memory
****************************************************************************/
extern void init_platdef(void);

/****************************************************************************
    PLATDEF DATA ACCESS - retrieve particular APML data sub-sections
****************************************************************************/


/****************************************************************************
    PLATDEF SMIF API - for APML SMIF and RESTful/RIS calls
****************************************************************************/
#define MAX_PLATDEF_SPECIFIC_DATA_REQ (500)  //prob could be as much as 600 (4000/6bytes) ?
#define MAX_PLATDEF_PATCH_REQ (200)  //patch can be 2+2+15 - 4000/19 ~ 210

/* Platdef SMIF Return Codes
   per "SMIF and RIS interface for APML" : 
        0 = Success
        1 = Busy
        2 = Can't save
        3 = Invalid request
        4 = Wrong timestamp
        5 = Not found
        6 = No more records
        Other = General failure
*/
typedef enum    // could be #defines but typed for safety and lint completeness checking
{
    PLATDEF_SMIF_RC_OK = 0,
    PLATDEF_SMIF_RC_BUSY,                  // PLATDEF or other not loaded or ready
    PLATDEF_SMIF_RC_CANTSAVE,              // PLATDEF can't update
    PLATDEF_SMIF_RC_BADREQUEST,            // bad command or param given
    PLATDEF_SMIF_RC_BADTIMESTAMP,          // Bad or invalid timestamp
    PLATDEF_SMIF_RC_NOTFOUND,              // record unavailable currently
    PLATDEF_SMIF_RC_NO_MORE_RECORDS,       // ran out of data
    PLATDEF_NOT_VALIDATED,                 // validation records not yet validated
    PLATDEF_SMIF_RC_ERROR                  // general error
} platdef_smif_rc;

/* the SMIF RC strings must match the return codes order above */
#define PLATDEF_SMIF_RC_STRINGS(x)        \
     (x==PLATDEF_SMIF_RC_OK)                 ?"PLATDEF_SMIF_RC_OK":              \
     (x==PLATDEF_SMIF_RC_BUSY)               ?"PLATDEF_SMIF_RC_BUSY":            \
     (x==PLATDEF_SMIF_RC_CANTSAVE)           ?"PLATDEF_SMIF_RC_CANTSAVE":        \
     (x==PLATDEF_SMIF_RC_BADREQUEST)         ?"PLATDEF_SMIF_RC_BADREQUEST":      \
     (x==PLATDEF_SMIF_RC_BADTIMESTAMP)       ?"PLATDEF_SMIF_RC_BADTIMESTAMP":    \
     (x==PLATDEF_SMIF_RC_NOTFOUND)           ?"PLATDEF_SMIF_RC_NOTFOUND":        \
     (x==PLATDEF_SMIF_RC_NO_MORE_RECORDS)    ?"PLATDEF_SMIF_RC_NO_MORE_RECORDS": \
     (x==PLATDEF_NOT_VALIDATED)              ?"PLATDEF_NOT_VALIDATED":           \
     (x==PLATDEF_SMIF_RC_ERROR)              ?"PLATDEF_SMIF_RC_ERROR":"Unknown"

typedef struct {
    UINT32              errcode;
    UINT16              op;
    UINT16              flags;
    UINT32              data_size;
    UINT32              data_offset;
    UINT32              timestamp;
    UINT16              recordID;
    UINT16              count;
    UINT8               Data[PLATDEF_CHUNK_SIZE];
} platdef_smif_resp;

typedef struct {
    UINT16              RecordID;
    UINT16              Offset;
    UINT16              length;
} PlatDefDataRequest;

#define RECORD_ID_OFFSET 2
#define RECORD_ID_LENGTH 2

//For struct TYPE, find size (in bytes) before union member UNION.
#define REQRESP_PREUNION_SIZE(TYPE, UNION) ((size_t)&((TYPE*)0)->UNION)
//For struct TYPE, find actual total size (in bytes) for MEMBER in the UNION.
#define REQRESP_SIZE(TYPE, UNION, MEMBER)  ((size_t)&((TYPE*)0)->UNION.MEMBER + (size_t)sizeof(((TYPE*)0)->UNION.MEMBER))

//Seperate the CHIF VCom from all other - as CHIF requires large buffers and stack usage
typedef struct {
    enum {
        RELOAD_PLATDEF = 1,
        BEGIN_UPLOAD_PLATDEF,
        UPLOAD_PLATDEF_CHUNK,
        FINISH_UPLOAD_PLATDEF,
        FINISH_UPLOAD_PLATDEF_W_RELOAD,
        PATCH_PLATDEF_DATA,
        DOWNLD_1_PLATDEF_DATA,
        DOWNLD_PLATDEF_HDRS,
        DOWNLD_PLATDEF_CHUNK,
        DOWNLD_SPEC_PLATDEF_DATA,
        GET_PLATDEF_VAL_STATUS,
        DOWNLD_VALIDN_RESULTS,   
        DOWNLD_SPEC_PLATDEF_DATA_BY_TYPE
    }command;
    union {
        struct {
            UINT32 operation;
        }reLoad;
        struct {
            UINT16 flags;
        }Begin_Upload;
        struct {
            UINT32 data_size;
            UINT32 data_offset;
            UINT16 flags;
            UINT32 token;
            UINT8 platDefData[PLATDEF_CHUNK_SIZE];
        }upLoad_Chunk;
        struct {
            UINT16 flags;
            UINT32 token;
        }Finished_Load;
        struct {
            UINT32 data_size;
            UINT32 timestamp;
            UINT16 patch_count;
            PlatDefDeltaPatchEntry patch_rec[MAX_PLATDEF_PATCH_REQ]; //?? Max. count of patch records
        }patch_Data;
        struct {
            UINT32 timestamp;
            UINT16 recordID;
            UINT8 platDef_rec[2000];
        }download_1_rec;
        struct {
            UINT32 timestamp;
            UINT16 recID_last; 
        }download_hdrs;
        struct {
            UINT32 data_size;
            UINT32 data_offset;
            UINT32 timestamp;
        }download_Chunk;
        struct {
            UINT32 data_size;
            UINT32 timestamp;
            UINT32 rec_count;
            PlatDefDataRequest req_data[MAX_PLATDEF_SPECIFIC_DATA_REQ];
        }download_specificData;
        struct {
            UINT32 data_size;
            UINT32 timestamp;
            UINT16 recID_last; 
        }download_validation_results;
        struct {
            UINT16 recType;
            UINT32 timestamp;
            UINT16 rec_count;
            UINT16 data_size;
            PlatDefDataRequest req_data[MAX_PLATDEF_SPECIFIC_DATA_REQ];   //??? Max. data requests
        }download_specificData_by_type;
        struct {
            char  str[120];
            PlatDefPrimitive prim;
            UINT8 data[16];
            int val1;
            int val2;
        }misc_cmds;
    }args;
}PLATDEF_CHIF_MESSAGE_REQ;

typedef struct {
    platdef_smif_rc   result;
    union {
        struct {
            UINT32 error_code;
            UINT32 timestamp;
        }reLoad;
        struct {
            UINT32 error_code;
            UINT16 flags;
            UINT32 token;
        }Begin_Upload;
        struct {
            UINT32 error_code;
        }upLoad_Chunk;
        struct {
            UINT32 error_code;
            UINT32 timestamp;
        }Finishup_Load;
        struct {
            UINT32 error_code;
        }patch_Data;
        struct {
            UINT32 error_code;
            UINT32 data_size;
            UINT32 timestamp;
            UINT8  platDefData[PLATDEF_CHUNK_SIZE];
        }download_1_rec;
        struct {
            UINT32 error_code;
            UINT32 data_size;
            UINT32 timestamp;
            UINT16 recID_last; 
            UINT16 record_count;
            UINT8  platDef_record_hdrs[PLATDEF_CHUNK_SIZE];
        }download_hdrs;
        struct {
            UINT32 error_code;
            UINT32 rsp_data_size;
            UINT32 timestamp;
            UINT32 size_remaining;
            UINT8  platDefData[PLATDEF_CHUNK_SIZE];
        }download_Chunk;
        struct {
            UINT32 error_code;
            UINT32 rsp_data_size;
            UINT32 timestamp;
            UINT8  platDefData[PLATDEF_CHUNK_SIZE];
        }download_specificData;
        struct {
            UINT32 error_code;
            UINT32 data_size;
            UINT32 timestamp;
            UINT16 recID_last; 
            UINT16 record_count;
            PlatDefDeltaPatchEntry patchrec[MAX_PLATDEF_PATCH_REQ]; //Max. count of patch records
        }download_validation_results;
        struct {
            UINT32 error_code;
            UINT32 rsp_data_size;
            UINT32 timestamp;
            UINT8  platDefData[PLATDEF_CHUNK_SIZE];
            UINT16 resp_count;
        }download_specificData_by_type;
        struct {
            char str[80];
            UINT8 val;
        }misc_cmds;
    } retv;
}PLATDEF_CHIF_MESSAGE_RESP;

//General platdef VCom cmds
typedef struct {
    enum {
        GET_PLATDEF_VER = 1,
        GET_PLATDEF_LOAD_SOURCE,
        GET_PLATDEF_SELF_TEST,
        LOOKUP_TABLE_GET_BY_TABLEID,
        LOOKUP_TABLE_GET_BY_TABLENAME,
        EXECUTE_RAW_PRIMITIVE,
        GET_PRIMITIVE_STRING,
        GET_PLATDEF_SIZE
    }command;
    union {
        struct {
            UINT16 record_id;
            char   record_name[20];
            UINT32 key;
            char   key_str[16];
            UINT8  key_str_length;
            UINT32 value;
        }lookup;
        struct {
            char str[80];
            PlatDefPrimitive prim;
        }misc_cmds;
    }args;
}PLATDEF_MESSAGE_REQ;

typedef struct {
    platdef_smif_rc   result;
    union {
        struct {
            UINT32 error_code;
            UINT32 key;
            char   key_str[16];
            UINT8  key_str_length;
            UINT32 value;
        }lookup;
        struct {
            char str[120];
            UINT8 val;
            UINT8 data[16];
            int   result;
            int   error;
            int   val1;
            int   val2;
        }misc_cmds;
    } retv;
}PLATDEF_MESSAGE_RESP;

#pragma pack(1)
typedef enum {
    POWER_SUPPLY = 0,
    PIROM,
    MEGACELL,
    SLOT
}EntityType;

typedef struct {
    UINT8 bus;
    UINT8 address;
    UINT8 domain;
    UINT8 reserved[5];
}PowerSupply;

typedef struct {
    UINT8 segment;
    UINT8 address;
    UINT8 fru_address;
    UINT8 reserved[5];
}MegaCell;

typedef struct {
    UINT8 segment;
    UINT8 address;
    UINT8 reserved[6];
}PiRom;

typedef struct {
    UINT8  segment;
    UINT16 slot_num;
    UINT8  reserved[5];
}Slot;

typedef union {
    UINT8       raw_bytes[8];
    PowerSupply power_supply;
    MegaCell    mega_cell;
    PiRom       pirom;
    Slot        slot;
}EntityData;

typedef struct {
    UINT8       entity_type; //EntityType
    EntityData  entity_data;
}Entity;
#pragma pack()

extern platdef_smif_rc platdef_Download_specific_data (UINT32* data_size, UINT32 timestamp, UINT16* req_count, PlatDefDataRequest* req_data, 
                                           UINT8 *response_buf, UINT16 resp_buf_size, UINT32 *token);

extern platdef_smif_rc platdef_Download_specific_data_per_type (UINT32 timestamp, UINT32 recType, PlatDefDataRequest *req_data, UINT32 req_count,
                                                    void* resp, UINT16 * resp_count, UINT16 * data_size, UINT32 *token);

// PLATDEF_METADATA parsing functions
int platdef_meta_record_parse( UINT8* blob_ptr, void* *record_ptr, UINT16* record_count );
int platdef_meta_record_parse_list( UINT8* blob_ptr, void* *record_ptr, UINT16* record_count, UINT16 max_count );
extern int platdef_meta_load( void );
void platdef_clear_meta( void );
void populate_record( RECORD_TYPE_DATA* rec_data, UINT8* blob_ptr );

void platdef_table_dump(void);
PlatDefTableData* table_data( void );

/*******************************************
    MSIC GET FUNCS
********************************************/

int platdef_get_APML_data(uint32_t *count, Entity *entity_list, uint32_t max_num_entities);


//PlatDefI2C* Access Functions
PlatDefI2CEngine* i2c_engine_by_index( UINT8 n );

/* The theoretical maximum size is 4080 (255*16) bytes - 
   FROM Adaptive ProLiant Management Layer Platform Definition 
   Specification Revision 1.23. */
#define PLATDEF_BMC_RECORD_MAX_SIZE (255*16)

#endif // __PLATDEF_API_H
