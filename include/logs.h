/*
// Copyright (c) 2002-2019, 2021-2025 Hewlett Packard Enterprise Development, LP
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

/***********************************************************************
* LOGS.H
*
* Header file to support the IML, the SL and the GEL for Gromit
*
************************************************************************/
#ifndef _LOGS_H
#define _LOGS_H

#include "eventlog.h"
#include "DataExtract.h"

#define MAX_SIZE 764 // sizeof(EVT_CONSOLIDATED_META_DATA)
#define EVT_MAX_VARIABLE_SIZE (EVT_MAX_SIZE - sizeof(EVT_HEADER))
#define COPY_SIZE(b_l,s_l) (b_l) > s_l ? s_l: (b_l)
#define EVNT_MAX_CONNECTIONS 8

#define UNKNOWN_STR "[UNKNOWN]"
#define UNKNOWN_STR_LEN 9

#define UNKNOWN_EVENT "UNKNOWN EVENT - Class [0x%x] and Code [0x%x]"
#define SIMULATED_UNKNOWN_EVENT "SIMULATED EVENT - UNKNOWN EVENT - Class [0x%x] and Code [0x%x]"
#define SIMULATED_EVT_TEXT "SIMULATED EVENT - " // test to be displayed in description for Simulated events

#define HIDE_PII 1 // Hide customer's personally identifiable data. This is used while storing IML/IEL in AHS
#define SHOW_PII 0 // Show customer's personally identifiable data. This is used while rendering IML/IEL in various interfaces

#define RESET_EVENTID_TRUE    1 // Reset event id after clearing IML/IEL logs.
#define RESET_EVENTID_FALSE   0 // Continue with existing event id after clearing IML/IEL logs.

#define LOG_CLEAR_EVENT_FALSE  0    // Do not log IML/IEL event after clearing IML/IEL log.
#define LOG_CLEAR_EVENT_TRUE   1    // Log IML/IEL event after clearing IML/IEL log.

#define LOGS_CFG_FILE "i:/vol0/cfg/evlog.bin"
#define LOGS_CFG_VERSION 1
#define PENDING_SPD_CLEAR_TRUE 1 // SPD clear is pending
#define PENDING_SPD_CLEAR_FALSE 0 // SPD clear is not pending
#define SL_MASK_BIT1_BIT2 0x9
#define SL_READ_BIT1_BIT2 0x6

#define EVLOG_STRING32 32

typedef enum {
    SL_ACTION_NA = 0,
    SL_ACTION_NONE,
    SL_ACTION_IGNORED
} USER_ACTION_ID;

typedef enum {
    LOGS_IMLDATA = 0,
    LOGS_GELDATA = 1,
    LOGS_ERSDATA=2,
    LOGS_PLDATA=3,
    LOGS_SLDATA=4,
    LOGS_DPUDATA=5,
    LOGS_EVENTDATA=6,
    LOGS_MAX
} LOG_ID;

typedef enum {
    TEXT_DESC = 0,
    TEXT_ACTION,
    TEXT_CAUSE,
    TEXT_MAX
} TEXT_TYPE;
#define UNKNOW_USER "Unknown User"
#pragma pack(1)

typedef struct _localVarDetails
{
    BYTE varWidth; // VarWidth
    BYTE varOffset; // Offset
    BYTE varRepresentation;
    BYTE varRepDigits;
    BYTE ahsRedactionFlag;
} LOCAL_VAR_DETAILS;

typedef struct _localVarData
{
    BYTE varCount; // No. of variable data
    LOCAL_VAR_DETAILS localVarDetails[MAX_EVT_VARIABLES]; // Array of variable details
} LOCAL_VAR_DATA;

// Trap Structure to read trap details
typedef struct _localVarBinds
{
    BYTE oidDigitCount; // OID digits size
    WORD oid[MAX_OID_DIGITS]; // OID
    BYTE oidIdxMappedVarCount; // No. of OID index mapping
    BYTE oidIdxMappedVarNum[MAX_OID_DIGITS_MAPPED];
    BYTE oidType; // OID Type - 0x04(String Type), 0x05(Integer Type)
    BYTE mappedVarNum; // Mapped variable number
} LOCAL_VARBINDS;

typedef struct _localTrapDetails
{
    DWORD trapId; // Trap ID
    DWORD hostTrapFlag; // indicating the trap condition
    DWORD startTime; // evlog VAS start time used to calulate time ticks by SNMP VAS
    BYTE varBindCount; // No. of varbinds
    LOCAL_VARBINDS localVarBinds[MAX_TRAP_VARBINDS]; // Array of varbinds
} LOCAL_TRAP_DETAILS;

typedef struct _localRISAlertDetails
{
    BYTE paramCount;            // No. of parameters
    BYTE mappedVarNumbers[MAX_EVT_VARIABLES];     // Array of mapped variables
    char risAlertID[128];           // RIS alert ID
} LOCAL_RIS_ALERT_DETAILS;

typedef struct {
    UINT8 version;
    UINT8 pending_spd_clear;
} logs_config_type;

typedef struct {
    UINT32        param_number;
    char          param_string[128];
} EVT_DNS_REC;


typedef struct {
    UINT32        in_use;
    UINT32        logIndex;
    UINT32        match_code;
    EVT_DNS_REC   dns_rec;
    char log_entry[EVT_MAX_SIZE];
} EVNT_REC;

#pragma pack()

// Decoded Log Entry Structure
typedef struct _evtDecodedLogEntry
{
   DWORD dwEvtNumber;
   BYTE   bSeverity;           /* Event severity code */
   WORD   wClass;              /* Event class or sub-system */
   WORD   wCode;              /* Event Code */
   DWORD  evtInitTime;      /* Time of the event */
   DWORD  evtUpdatedTime;      /* Time of the event */
   DWORD  dwCount;             /* Occurrence count (at least 1) */
   QWORD evtCategory;       /* Event category */
   BYTE UserAction;
   char   description[EVT_MAX_DESC_TEXT];       /* Null terminated Translated Description */
   char   action[EVT_MAX_ACTION_TEXT];                    /* Null terminated Action string */
} EVT_DECODED_LOG_ENTRY, *PEVT_DECODED_LOG_ENTRY;

/***********************************************************************
* definitions
************************************************************************/
typedef UINT8 LOG_STATUS;

#define LOG_OK                   0x00
#define LOG_ERROR                0x01
#define LOG_NO_SUCH_EVENT        0x02
#define LOG_EVENT_FOUND          0x03
#define LOG_EVENT_NEXT           0x04
#define LOG_LENGTHS_DIFFER       0x05
#define LOG_BUFFER_TOO_SMALL     0x06
#define LOG_NOT_INITIALIZED      0x07
#define LOG_WATERMARK_REACHED    0x08
#define LOG_FULL                 0x09
#define SUBSCRIPTION_OK          0x0A
#define SUBSCRIPTION_FAILED      0x0B
#define SUBSCRIPTION_UNAVAILABLE 0x0C
#define DECODE_OK                0x0D
#define DECODE_ERROR             0x0E
#define DECODE_DATA_NOT_FOUND    0x0F
#define TRAP_DATA_NOT_FOUND      0x10
#define LOG_DNS_THREADS_BUSY     0x11
#define LOG_CORRUPTED            0x12

#define LOG_MAX_NOTIFY_QUEUES       (8)
#define LOG_DEFAULT_NOTIFY_QUEUES   (7)
#define MAX_SUBSCRIPTION            LOG_MAX_NOTIFY_QUEUES

#define LOG_CHANGE_QUEUE_HEALTH     (0)
#define LOG_CHANGE_QUEUE_AHS        (1)
#define LOG_CHANGE_QUEUE_AMAIL      (2)
#define LOG_CHANGE_QUEUE_SYSLOG     (3)
#define LOG_CHANGE_QUEUE_LIVELOGS   (4)

#define LOG_CQ_DEPTH      (4)
// NOTE: The below number, with other fields in the struct needs to be a power of 2 for the queue
#define LOG_CHANGES_MAX_DATA  (512-14)

#define SL_ILO_SUBSYSTEM 1

// PMS_PJFS_TAG QXCR1001189058
/*
** Special Macro for extended IML / GEL reading
** ============================================
** We have an issue where if we have a very, very large IML (> 400 entries),
** too many things try to read the IML and/or GEL and this appears to cause
** the PJFS worker threads to thrash, consuming more than 50% of the CPU.
** This macro is to be used for large, sequential reads that are INTERNAL or
** generated by outside forces (IPMI, CHIF, etc). This macro will provide a throttle
** based on the entry ID.
**/
#define LOG_THROTTLE(x) if ( !(x % 10) ) usleep(100000);
/*
filterCriteria_ - 32 bit filter criteria
*   Filter Criteria (32 bits)
*   |  15  |  14  |  13  |  12  |  11  |  10  |  9  |  8  |  7  |  6  |   5   |  4  |  3   |  2  |  1   |  0  |
*   | IML  |  IEL |  SL  |  DPU |-----------RESERVED------------------|RSyslog|AMail| AHS  | RIS | SNMP | eRS |
*   | 31 - 16 |
*   |RESERVED |
*/
// Subscription Filter Criteria definitions
#define NO_CRITERIA                            0x00000000
#define FILTER_CRITERIA_IML_BITMASK            0x00008000 // IML Events
#define FILTER_CRITERIA_IEL_BITMASK            0x00004000 // IEL Events
#define FILTER_CRITERIA_SL_BITMASK             0x00002000 // SL Events
#define FILTER_CRITERIA_DPU_BITMASK            0x00001000 // DPU Events
#define FILTER_CRITERIA_RIS_EVENT_BITMASK      0x00000004 // RIS Event Enabled
#define FILTER_CRITERIA_SNMP_TRAP_BITMASK      0x00000002 // SNMP Trap Enabled
#define FILTER_CRITERIA_SERVICE_EVENT_BITMASK  0x00000001 // Service Event Enabled
#define FILTER_CRITERIA_AHS_LOGGING_BITMASK    0x00000008 // AHS Logging Enabled
#define FILTER_CRITERIA_AMAIL_BITMASK          0x00000010 // Alert Mail Enabled
#define FILTER_CRITERIA_RSYSLOG_BITMASK        0x00000020 // RSyslog Enabled

// override config flags bitmask - refer to struct LogChangeMessage
#define OVERRIDE_REPORTING_ENTITY_AMS          0x0001 // Use AMS as Reporting Entity
#define OVERRIDE_REPORTING_ENTITY_BIOS         0x0002 // Use BIOS as Reporting Entity
#define OVERRIDE_REPORTING_ENTITY_STORAGE      0x0003 // Use STORAGE as Reporting Entity
#define OVERRIDE_REPORTING_ENTITY_NIC          0x0004 // Use NIC as Reporting Entity
#define OVERRIDE_REPORTING_ENTITY_ILO          0x0005 // Use iLO as Reporting Entity

#define OVERRIDE_REST_ALERT                    0x0008 // Do not send Rest Alert
#define OVERRIDE_SNMP_TRAP                     0x0010 // Do not not send SNMP trap
#define OVERRIDE_SERVICE_EVENT                 0x0020 // Do not send service event
#define OVERRIDE_EVENT_LOGGING                 0x0040 // Do not write event to NVRAM
#define OVERRIDE_BIOS_RENDERING                0x0080 // Do not log to Rsyslog

#define OVERRIDE_ALERT_MAIL                    0x0100 // Do not log to Alert Mail
#define OVERRIDE_RSYSLOG                       0x0200 // Do not log to Rsyslog

#define AHS_LOG_DECODED_EVT                    0x0400 // Log Decoded logs to AHS
#define AHS_LOG_RAW_EVT                        0x0800 // Log Raw logs to AHS
#define AHS_LOG_DECODED_RAW_EVT                0x0C00 // Log Decoded and Raw logs to AHS
#define OVERRIDE_AHS_LOGGING                   0x1C00 // Do not log to AHS

#define EVT_HEADER_FLAGS_SIMULATED_EVENT 0x1 // Simulated event

#define ILO_HOST_NAME     0
#define SERVER_HOST_NAME  1

typedef enum {
    LOG_CHANGE_NONE = 0, // invalid
    LOG_CHANGE_ENTRY_ADD = 1,
    LOG_CHANGE_ENTRY_MODIFY,
    LOG_CHANGE_LOG_CLEAR
} LOG_CHANGE_TYPE;

typedef enum {
    SUBSCRIPTION_NONE = 0, // invalid
    HEALTH_SUBSCRIPTION,
    AHS_SUBSCRIPTION,
    RIS_SUBSCRIPTION,
    SNMP_SUBSCRIPTION,
    eRS_SUBSCRIPTION,
    AMAIL_SUBSCRIPTION,
    RSYSLOG_SUBSCRIPTION,
    SNMP_BUBBLE_LIST_SUBSCRIPTION
} SUBSCRIPTION_SOURCE;

typedef enum {
    HEALTH_SUBSCRIPTION_HANDLE=0,
    AHS_SUBSCRIPTION_HANDLE,
    RIS_SUBSCRIPTION_HANDLE,
    SNMP_SUBSCRIPTION_HANDLE,
    eRS_SUBSCRIPTION_HANDLE,
    AMAIL_SUBSCRIPTION_HANDLE,
    RSYSLOG_SUBSCRIPTION_HANDLE
} DEFAULT_SUBSCRIPTION_HANDLE;

typedef enum {
    LOG_TAIL_CORRUPT = 0,
    LOG_LOW_CORRUPT,
    LOG_HIGH_CORRUPT,
    LOG_LAST_CORRUPT
} LOG_CORRUPT_TYPE;


/***********************************************************************
* Structures
************************************************************************/
#pragma pack(1)

typedef struct LogHeader {
    UINT16 head;
    UINT16 tail;
    UINT16 high;
    UINT16 low;
    UINT32 eventNumber;
    UINT16 last;
    UINT16 signature;
    UINT16 reserved;
} LOG_HEADER;
#pragma pack()

//typedef struct LogInformationTag {
//    int lognum;   // either LOGS_IMLDATA or LOGS_GELDATA or LOGS_SLDATA
//    int fd;
//    UINT16 size;
//    UINT32 fullWaterMark;
////    Semaphore intSemaphore;     // Integrity semaphore for MUTEX
//    UINT32 semaphore;       // ASM style semaphore for host interaction
//    UINT8  log_is_full;
//    UINT8  recount;
//    UINT16 eventcount;
//} LOG_INFO;

typedef struct LogChangeMessage {
    // Change header information
    LOG_ID          log_id;
    UINT32          event_id;
    LOG_CHANGE_TYPE change_type;
    // Event (needs to be cast to EVT_LOG_ENTRY (limited length)
    /* 
    ConfigFlags bit definition 
    Bit    | Definition
    ------   -----------
    2-0      Reporting entity 
    -------------------------- 
     001    AMS
     010    BIOS
     011    Storage
     100    NIC
     101    iLO
     Others RESERVED
    --------------------------
    3        RIS Event override 
    4        SNMP Trap override 
    5        Service Event Override
    6        IML override
    7        BIOS Rendering override
    8        Alert Mail Settings Override
    9        RSysLog Override
    12-10    AHS Logging Override
    15-13    RESERVED
    */
    UINT16          configFlags;  //  sum 14 bytes
    char            event[LOG_CHANGES_MAX_DATA];
    // Only store up-to the above amount, and the structure in the queue needs to be x^2
} LOG_CHANGE;


/********* POST IML FOR BIOS RENDERING **********/
#define POST_IML_DESC_LEN    EVT_MAX_DESC_TEXT
#define POST_IML_ACTION_LEN  EVT_MAX_ACTION_TEXT

#define POST_IML_Q_SUCCESS   0
#define POST_IML_Q_ERROR     1
#define POST_IML_Q_EMPTY     2
#define POST_IML_MAX_Q_SIZE  24

typedef struct evtPostIml {
    WORD evtClass;
    WORD evtCode;
    BYTE severity;
    char msgText[POST_IML_DESC_LEN]; // null terminated string
    char actionText[POST_IML_ACTION_LEN];
} EVT_POST_IML, *PEVT_POST_IML;

//Circular Queue for storing POST IML (For BIOS Rendering)
typedef struct PostImlQueue
{
   EVT_POST_IML evt_post_iml;
   struct PostImlQueue *que_next;
}EVT_POST_IML_QUEUE, *PEVT_POST_IML_QUEUE;


/********* ACTION TAG FOR AMAIL, RSYSLOG, AHS and POST IML ********/
typedef enum {
    NEWLINE_SEPARATOR =0,
    SPACE_SEPARATOR
} ACTION_TAG_SEPARATOR;

typedef struct{
        EVT_GENERAL_DATA        gen_data;
        LOCAL_TRAP_DETAILS      trap_data;
        LOCAL_VAR_DATA          var_data;
}EVT_CONSOLIDATED_META_DATA;

#define MAX_FORMAT_TEXT      EVT_MAX_DESC_TEXT + \
                             EVT_MAX_ACTION_TEXT + \
                             ACTION_TAG_LEN + 1     //1 byte for NULL

#define ACTION_TAG_SPACE     " ACTION: "
#define ACTION_TAG_NEWLINE   "\nACTION: "
#define ACTION_TAG_LEN       9

/*****************************************************************************
** As a result of enormous thrashing on PJFS when seeking log entries,
** we are creating a Linear Index to point to the offsets for records as
** they are added. This was originally done for the IML but now we have a
** need to do this for the GEL.
******************************************************************************/
#define LOGS_MAX_INDICES     ( 0x4000 / (sizeof(EVT_HEADER) + sizeof(int)) )
typedef struct LogsEntryIndex {
    UINT32      event_id;
    UINT32      entry_offset;
} LOGS_ENTRY_INDEX;

// this is a repeat from EVENTLOG.H - why do we duplicate it!!!
///#pragma pack(1)
///typedef struct IMLRecord {
///    UINT16 length;
///    UINT32 number;
///    UINT8  headerVersion;
///    UINT8  severity;
///    UINT16 class;
///    UINT16 code;
///    UINT8  initCentury;
///    UINT8  initYear;
///    UINT8  initMonth;
///    UINT8  initDay;
///    UINT8  initHour;
///    UINT8  initMinute;
///    UINT8  eventCentury;
///    UINT8  eventYear;
///    UINT8  eventMonth;
///    UINT8  eventDay;
///    UINT8  eventHour;
///    UINT8  eventMinute;
///    UINT32 count;
///} EVT_LOG_ENTRY;
///#pragma pack()
///


/***********************************************************************
* Function Prototypes
************************************************************************/
// Initializer
extern void  logs_init( void );

// Basic Operations
extern UINT8 logs_entry_add         ( int logIndex, EVT_LOG_ENTRY * record, UINT32 match );
extern UINT8 logs_entry_add_dns_name( int logIndex, EVT_DNS_REC * dns_rec, EVT_LOG_ENTRY * record, UINT32 match ); // Client
extern UINT8 logs_entry_add_oneshot ( int logIndex, EVT_LOG_ENTRY * record, UINT32 match ); // cleans up vascom context, for dynamic threads
extern UINT8 logs_entry_read        ( int logIndex, UINT32 eventID, EVT_LOG_ENTRY * record,
                                        UINT16 * maxLen, UINT32 match );
extern UINT8 logs_entry_add_dns_name_s( int logIndex, const BYTE * buffer, UINT32 match ); // Service
extern UINT8 logs_entry_read_sort   ( int logIndex, UINT32 eventID, EVT_LOG_ENTRY * record,
                                        UINT16 * maxLen, UINT32 match );
extern UINT8 logs_entry_modify  ( int logIndex, UINT32 eventID, EVT_LOG_ENTRY * record, UINT32 match );
extern UINT8 logs_log_clear     ( int logIndex, UINT8 flag, UINT8 logEvent, char *userName);
extern UINT8 logs_log_corrupt     ( int logIndex, UINT8 flag, UINT16 offset);
extern UINT8 logs_get_spd_clear_status(UINT8 *pending_spd_clear);
extern UINT8 logs_entry_readlast( int logIndex, EVT_LOG_ENTRY * record, UINT16 len );

//GEN10 Logging
extern UINT8 logs_entry_quickAdd(int logIndex, UINT8 severity, UINT16 evtClass, UINT16 evtCode, void* data, UINT16 dataSize, UINT32 match, UINT32 *evtNum);
extern UINT8 logs_entry_quickModify(int logIndex, UINT32 eventID, UINT8 severity, UINT16 evtClass, UINT16 evtCode, void* data, UINT16 dataSize);
extern int write_decoded_post_iml (EVT_LOG_ENTRY *record_in);
extern int read_decoded_post_iml (EVT_POST_IML *record_in);
extern void clear_decoded_post_iml_queue (void);
extern BOOL logs_get_hpsc_link(UINT16 evtClass, UINT16 evtCode, UINT8 severity, char* link_out_buf, UINT16 buf_size);
extern int logs_host_name_get(char *sysName, int buflen, int type);
extern int logs_get_start_time (UINT32* start_time);
extern int get_event_hdr_for_trap(int trapID, EVT_HEADER *evlogHrd );
extern void evlog_format_text(char * desc, char *action, char *fmtbuf, int *buflen, UINT8 flag);

// special version for HEALTH VAS to use, avoids deadlock.
extern UINT8 logs_log_clear_noalert  ( int logIndex, UINT8 flag, UINT8 logEvent, char *userName, size_t userName_size);

// special function for HEALTH VAS to learn about changes to the logs
extern UINT8 logs_change_wait( int q_number, LOG_CHANGE* change);

extern UINT32 logs_get_min_event_id(int logIndex);
extern UINT32 logs_get_max_event_id(int logIndex);
extern UINT16 logs_get_event_count(int logIndex);
extern UINT8 logs_get_log_full( int logIndex );
extern UINT8 logs_get_log_empty( int logIndex );
extern UINT16 logs_event_get( int logIndex, UINT16 offset, EVT_LOG_ENTRY * evt );

// Function to subscribe for IML notifications
extern UINT8 add_notify_subscription(UINT32 filterCriteria_, UINT8 subSource_, UINT8* handle_);
extern UINT8 update_notify_subscription(UINT32 filterCriteria_, UINT8 subSource_);

// Functions for decoding
extern UINT8 read_decoded_log_entry (int logType, 
                                     UINT32 eventID, 
                                     EVT_DECODED_LOG_ENTRY* record,
                                     UINT8 safe);

extern UINT8 decode_text(LOG_ID logType,
                         EVT_LOG_ENTRY* evt,
                         char* decodedText,
                         unsigned short maxLen,
                         TEXT_TYPE textType);

extern UINT8 decode_meta_data_consolidated (int logType,
                                       UINT16 evtClass,
                                       UINT16 evtCode,
                                       UINT8 severity,
                                       EVT_CONSOLIDATED_META_DATA *metaData);

extern UINT8 decode_general_meta_data (int logType,
                                       UINT16 evtClass,
                                       UINT16 evtCode,
                                       PEVT_GENERAL_DATA pEvtGenMetaData);
extern int get_event_hdr_for_trap(int trapID,
                                       EVT_HEADER *hdr );

extern UINT8 decode_trap_meta_data (int logType,
                                    UINT16 evtClass,
                                    UINT16 evtCode,
                                    UINT8 severity,
                                    LOCAL_TRAP_DETAILS *trapDetails);

extern UINT8 decode_variable_meta_data (int logType,
                                        UINT16 evtClass,
                                        UINT16 evtCode,
                                        LOCAL_VAR_DATA *varMetaData);

extern UINT8 decode_ris_alert_meta_data (int logType,
                                         UINT16 evtClass,
                                         UINT16 evtCode,
                                         UINT8 severity,
                                         LOCAL_RIS_ALERT_DETAILS *risAlertDetails);

extern UINT8 decode_service_event_meta_data (int logType,
                                             UINT16 evtClass,
                                             UINT16 evtCode,
                                             EVT_SERVICE_EVENT_DATA *serEventMetaData);

/***********************************************************************
* Defines for iml_ and gel_ functions
************************************************************************/
// Basic Operations
//#define iml_addEntry( a, b )           logs_entry_add( LOGS_IMLDATA, a, b )
//#define iml_modifyEntry( a, b )        logs_entry_modify( LOGS_IMLDATA, a, b )
//#define iml_clearLog( a )              logs_log_clear( LOGS_IMLDATA, a )
//#define iml_readEntry( a, b, c, d )    logs_entry_read( LOGS_IMLDATA, a, b, c, d )

// Basic Operations
//#define gel_addEntry( a, b )           logs_entry_add( LOGS_GELDATA, a, b )
//#define gel_modifyEntry( a, b )        logs_entry_modify( LOGS_GELDATA, a, b )
//#define gel_clearLog( a )              logs_log_clear( LOGS_GELDATA, a )
//#define gel_readEntry( a, b, c, d )    logs_entry_read( LOGS_GELDATA, a, b, c, d )

//#define iml_readLastEntry( a, b )      logs_entry_readlast( LOGS_IMLDATA, a, b )
//#define gel_readLastEntry( a, b )      logs_entry_readlast( LOGS_GELDATA, a, b )

// Macro Operations
// These could include operations like;
//  deleteEntry
//  markAsFixed, etc.
//
//  they really are just modifies with special structures
//
// STILL HAVE to do the special Search that ignores certain fields.
// check logs.c if this is done or not!


#endif // _LOGS_H

/***********************************************************************
* End of File
************************************************************************/
