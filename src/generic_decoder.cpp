/*
// Copyright 2001-2025 Hewlett Packard Enterprise Development, LP
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

#include "chif.hpp"
#include <stddef.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <unistd.h>

#include "generic_decoder.h"
extern "C" {
#include "DataExtract.h"
}
//#include "oem.h"

//#define MAX_AHS_EVT_CHARACTERS      (48)
//#define AHS_EVT_HASH_PREFIX_CHARS   (2)
//#define AHS_EVT_HASH_CHARS          (MAX_AHS_EVT_CHARACTERS - AHS_EVT_HASH_PREFIX_CHARS - 1) // -1 for the NULL.
//#define b64_ntop __b64_ntop
//extern int b64_ntop(u_char *target, size_t targsize, char const *src, int srclen);

// Structure pointers
//extern BINARY_HEADER *binHeader; // Binary file binHeaderdata
//extern EVT_GENERAL_DATA *gData; // General Meta Data
//extern EVT_TEXT_DATA *dData; // Description Data
//extern EVT_TEXT_DATA *aData; // Action Data
//extern EVT_VAR_DATA *vData; // Variable Meta Data
//extern EVT_TRAP_DATA *tData; // Trap Meta Data
//extern EVT_RIS_ALERT_DATA *rData; // RIS Alert Data
//extern EVT_SERVICE_EVENT_DATA *sData; // Service Event Data

//extern UINT32 startUTCTime;

// Decode functions
UINT8 generic_decode(int, EVT_LOG_ENTRY*, EVT_DECODED_LOG_ENTRY*, UINT8);

int find_index_iml_data_extract(int dataExtractID, UINT16 evtClass, UINT16 evtCode);
//int find_index_supp_data_extract(int dataExtractID, UINT16 evtClass, UINT16 evtCode);
int find_index_sl_data_extract(int dataExtractID, UINT16 evtClass, UINT16 evtCode);
int find_index_iel_data_extract(int dataExtractID, UINT16 evtClass, UINT16 evtCode);

//UINT8 get_trap_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_TRAP_DATA** trapData);
//UINT8 get_ris_alert_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_RIS_ALERT_DATA** risData);
//UINT8 get_service_event_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_SERVICE_EVENT_DATA** serData);
UINT8 get_variable_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_VAR_DATA** varData);
UINT8 get_action_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_TEXT_DATA** textData);
UINT8 get_description_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_TEXT_DATA** textData);
//UINT8 get_general_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_GENERAL_DATA** genData);
UINT8 extract_variable_value (EVT_LOG_ENTRY* record,
                              UINT8 varOffset,
                              UINT8 varWidth,
                              UINT8 varRep,
                              UINT8 varRepDigits,
                              char **argVal,
                              UINT8 safe);

//externs

extern EVT_GENERAL_DATA List_IML_General_Data [];
extern EVT_TEXT_DATA List_IML_Action_Data [];
extern EVT_TEXT_DATA List_IML_Desc_Data [];
extern EVT_VAR_DATA List_IML_Var_Data [];
extern EVT_TRAP_DATA List_IML_Trap_Data [];
extern EVT_RIS_ALERT_DATA List_IML_RIS_Alert_Data [];
extern EVT_SERVICE_EVENT_DATA List_IML_Service_Event_Data [];

extern EVT_GENERAL_DATA List_IEL_General_Data [];
extern EVT_TEXT_DATA List_IEL_Action_Data [];
extern EVT_TEXT_DATA List_IEL_Desc_Data [];
extern EVT_VAR_DATA List_IEL_Var_Data [];
extern EVT_TRAP_DATA List_IEL_Trap_Data [];
extern EVT_RIS_ALERT_DATA List_IEL_RIS_Alert_Data [];
extern EVT_SERVICE_EVENT_DATA List_IEL_Service_Event_Data [];

extern EVT_GENERAL_DATA List_SL_General_Data [];
extern EVT_TEXT_DATA List_SL_Action_Data [];
extern EVT_TEXT_DATA List_SL_Desc_Data [];
extern EVT_VAR_DATA List_SL_Var_Data [];
extern EVT_TRAP_DATA List_SL_Trap_Data [];
extern EVT_RIS_ALERT_DATA List_SL_RIS_Alert_Data [];
extern EVT_SERVICE_EVENT_DATA List_SL_Service_Event_Data [];

//extern UINT64 ahs_data_extract_salt;

#if 0
/***********************************************************************
* decode_meta_data_consolidated() - Source Function
*   reads the general meta data for IML/IEL/SL log entry
*
* Input Parameters:
*
* 1. int logType - IML or IEL or SL
*
* 2. UINT16 evtClass
*   Identifies the event class
* 
* 3. UINT16 evtCode
*   Identifies the event code
* 
* 4. UINT8 severity
*  Identifies the severity
*
* Output Parameters:
* 
*   EVT_CONSOLIDATED_META_DATA *metaData
*   contains three structures
*    EVT_GENERAL_DATA        gen_data;
*    LOCAL_TRAP_DETAILS      trap_data;
*    LOCAL_VAR_DATA          var_data;
* 
* Return Values:
*
* 1. LOG_OK for success
* 2. LOG_ERROR for failure
*
************************************************************************/
UINT8 decode_meta_data_consolidated(int logType, UINT16 evtClass, UINT16 evtCode, UINT8 severity, EVT_CONSOLIDATED_META_DATA *metaData)
{

    int rc = 0;
    rc = decode_variable_meta_data(logType, evtClass, evtCode, &metaData->var_data);
    if(rc != LOG_OK)
        return rc;

    rc = decode_trap_meta_data(logType, evtClass, evtCode, severity, &metaData->trap_data);
    if(rc != LOG_OK)
        return rc;
    rc = decode_general_meta_data(logType, evtClass, evtCode, &metaData->gen_data);
    if(rc != LOG_OK)
        return rc;

    return rc;

}

/***********************************************************************
* decode_general_meta_data() - Source Function
*   reads the general meta data for IML/IEL/SL log entry
*
* Input Parameters:
*
* 1. int logType - IML or IEL or SL
*
* 2. UINT16 evtClass
*   Identifies the event class
* 
* 3. UINT16 evtCode
*   Identifies the event code
*
* Output Parameters:
*
* 1. PEVT_GENERAL_DATA pEvtGenMetaData
*    returns the event general meta data in EVT_GENERAL_DATA structure
*
* Return Values:
*
* 1. LOG_OK for success
* 2. LOG_ERROR for failure
*
************************************************************************/
UINT8 decode_general_meta_data (int logType,
                                UINT16 evtClass,
                                UINT16 evtCode,
                                PEVT_GENERAL_DATA pEvtGenMetaData)
{
    UINT8 rc;
    EVT_GENERAL_DATA *genData=NULL;
    rc = get_general_data(logType, evtClass, evtCode, &genData);
    if (rc!=LOG_OK) {
        return rc;
    }
    if (genData == NULL) {
        return LOG_ERROR;
    }
    memcpy(pEvtGenMetaData, genData, sizeof(EVT_GENERAL_DATA)); 

    return LOG_OK;
}

/***********************************************************************
* decode_trap_meta_data() - Source Function
*   reads the trap meta data for IML/IEL/SL log entry
*
* Input Parameters:
*
* 1. int logType - IML or IEL or SL
*
* 2. UINT16 evtClass
*   Identifies the event class
* 
* 3. UINT16 evtCode
*   Identifies the event code
*
* Output Parameters:
*
* 1. LOCAL_TRAP_DETAILS *trapDetails
*    returns the event trap meta data in LOCAL_TRAP_DETAILS structure
*
* Return Values:
*
* 1. LOG_OK for success
* 2. LOG_ERROR for failure
*
************************************************************************/
UINT8 decode_trap_meta_data (int logType,
                             UINT16 evtClass,
                             UINT16 evtCode,
                             UINT8 severity,
                             LOCAL_TRAP_DETAILS *trapDetails)
{
    int i, j;
    UINT8 rc;
    EVT_TRAP_DATA *trapData=NULL;
    rc = get_trap_data(logType, evtClass, evtCode, &trapData);
    if (rc!=LOG_OK) {
        return rc;
    }
    if (trapData == NULL) {
        return LOG_ERROR;
    }
    for (i=0;i<trapData->trapCount;i++) {
        if (trapData->trapDetails[i].mappedSeverity == severity) {
            trapDetails->trapId = trapData->trapDetails[i].trapId;
            trapDetails->hostTrapFlag = trapData->trapDetails[i].hostTrapFlag;
            trapDetails->startTime = startUTCTime;
            trapDetails->varBindCount = trapData->trapDetails[i].varBindCount;

            for (j=0;j<trapData->trapDetails[i].varBindCount;j++) {
                if (j>=MAX_TRAP_VARBINDS) {
                    break;
                }
                trapDetails->localVarBinds[j].oidDigitCount = trapData->trapDetails[i].varBinds[j].oidDigitCount;
                // memcpy
                if (trapData->trapDetails[i].varBinds[j].oidDigitCount > 0) {
                    memcpy(trapDetails->localVarBinds[j].oid,
                           trapData->trapDetails[i].varBinds[j].oid,
                           (trapData->trapDetails[i].varBinds[j].oidDigitCount)*sizeof(WORD));
                }

                trapDetails->localVarBinds[j].oidIdxMappedVarCount = trapData->trapDetails[i].varBinds[j].oidIdxMappedVarCount;
                // memcpy
                if (trapData->trapDetails[i].varBinds[j].oidIdxMappedVarCount > 0) {
                    memcpy(trapDetails->localVarBinds[j].oidIdxMappedVarNum,
                           trapData->trapDetails[i].varBinds[j].oidIdxMappedVarNum,
                           trapData->trapDetails[i].varBinds[j].oidIdxMappedVarCount);
                }

                trapDetails->localVarBinds[j].oidType = trapData->trapDetails[i].varBinds[j].oidType;
                trapDetails->localVarBinds[j].mappedVarNum = trapData->trapDetails[i].varBinds[j].mappedVarNum;
            }
            break;
        }
    }
    return LOG_OK;
}

/***********************************************************************
* decode_ris_alert_meta_data() - Source Function
*   reads the ris alert meta data for IML/IEL/SL log entry
*
* Input Parameters:
*
* 1. int logType - IML or IEL or SL
*
* 2. UINT16 evtClass
*   Identifies the event class
* 
* 3. UINT16 evtCode
*   Identifies the event code
*
* Output Parameters:
*
* 1. LOCAL_RIS_ALERT_DETAILS *risAlertMetaData
*    returns the event ris alert meta data in LOCAL_RIS_ALERT_DETAILS structure
*
* Return Values:
*
* 1. LOG_OK for success
* 2. LOG_ERROR for failure
*
************************************************************************/
UINT8 decode_ris_alert_meta_data (int logType,
                                  UINT16 evtClass,
                                  UINT16 evtCode,
                                  UINT8 severity,
                                  LOCAL_RIS_ALERT_DETAILS *risAlertDetails)
{
    int i;
    UINT8 rc;
    EVT_RIS_ALERT_DATA *risAlertData=NULL;
    rc = get_ris_alert_data(logType, evtClass, evtCode, &risAlertData);
    if (rc!=LOG_OK) {
        return rc;
    }
    if (risAlertData == NULL) {
        return LOG_ERROR;
    }
    for (i=0;i<risAlertData->alertCount;i++) {
        if (risAlertData->alertDetails[i].mappedSeverity == severity) {
            risAlertDetails->paramCount = risAlertData->alertDetails[i].paramCount;
            if ((risAlertData->alertDetails[i].paramCount <= MAX_EVT_VARIABLES) && (risAlertData->alertDetails[i].paramCount > 0)) {
                    memcpy(risAlertDetails->mappedVarNumbers,
                           risAlertData->alertDetails[i].mappedVarNumbers,
                           risAlertData->alertDetails[i].paramCount);
            }
            if (STRNLEN_S(risAlertData->alertDetails[i].risAlertID, sizeof(risAlertDetails->risAlertID))) {
                STRNCPY_S(risAlertDetails->risAlertID,sizeof(risAlertDetails->risAlertID), risAlertData->alertDetails[i].risAlertID, EVT_CATEGORY_USR_INPUT); 
	    }
            return LOG_OK;
        }
    }
    return LOG_ERROR;
}

/***********************************************************************
* decode_service_event_meta_data() - Source Function
*   reads the service event meta data for IML/IEL/SL log entry
*
* Input Parameters:
*
* 1. int logType - IML or IEL or SL
*
* 2. UINT16 evtClass
*   Identifies the event class
* 
* 3. UINT16 evtCode
*   Identifies the event code
*
* Output Parameters:
*
* 1. PEVT_SERVICE_EVENT_DATA pSerEventMetaData
*    returns the event service event meta data in EVT_SERVICE_EVENT_DATA structure
*
* Return Values:
*
* 1. LOG_OK for success
* 2. LOG_ERROR for failure
*
************************************************************************/
UINT8 decode_service_event_meta_data (int logType,
                                      UINT16 evtClass,
                                      UINT16 evtCode,
                                      PEVT_SERVICE_EVENT_DATA pSerEventMetaData)
{
    UINT8 rc;
    EVT_SERVICE_EVENT_DATA *serData=NULL;
    rc = get_service_event_data(logType, evtClass, evtCode, &serData);
    if (rc!=LOG_OK) {
        return rc;
    }
    if (serData == NULL) {
        return LOG_ERROR;
    }
    memcpy(pSerEventMetaData, serData, sizeof(EVT_SERVICE_EVENT_DATA));

    return LOG_OK;
}

/***********************************************************************
* decode_variable_meta_data() - Source Function
*   reads the variable meta data for IML/IEL/SL log entry
*
* Input Parameters:
*
* 1. int logType - IML or IEL or SL
*
* 2. UINT16 evtClass
*   Identifies the event class
* 
* 3. UINT16 evtCode
*   Identifies the event code
*
* Output Parameters:
*
* 1. LOCAL_VAR_DATA *varMetaData
*    returns the event variable meta data in LOCAL_VAR_DATA structure
*
* Return Values:
*
* 1. LOG_OK for success
* 2. LOG_ERROR for failure
* 3. DECODE_DATA_NOT_FOUND - no variable data for the event
*
************************************************************************/
UINT8 decode_variable_meta_data (int logType,
                                 UINT16 evtClass,
                                 UINT16 evtCode,
                                 LOCAL_VAR_DATA *varMetaData)
{
    int i;
    UINT8 rc;
    EVT_VAR_DATA *varData=NULL;
    if (varMetaData == NULL) {
        return LOG_ERROR;
    }
    rc = get_variable_data(logType, evtClass, evtCode, &varData); 
    if (rc!=LOG_OK) {
        return rc;
    }
    if (varData == NULL) {
        return LOG_ERROR;
    }
    varMetaData->varCount = varData->varDataCount;
    for (i=0; i<varData->varDataCount; i++) {
        if (i>=MAX_EVT_VARIABLES) {
            break;
        }
        varMetaData->localVarDetails[i].varWidth = varData->varDetails[i].varWidth; 
        varMetaData->localVarDetails[i].varOffset = varData->varDetails[i].varOffset;
        varMetaData->localVarDetails[i].varRepresentation = varData->varDetails[i].varRepresentation;
        varMetaData->localVarDetails[i].varRepDigits = varData->varDetails[i].varRepDigits;
        varMetaData->localVarDetails[i].ahsRedactionFlag = varData->varDetails[i].ahsRedactionFlag;
    }
    return LOG_OK;
}


/***********************************************************************
* read_decoded_log_entry()
*   reads the decoded (with decoded 'description' and 'action' text) IML/IEL/SL log entry
* 
* Input Parameters:
* 
* 1. int logType - IML or IEL or SL
*
* 2. UINT32 eventID
*   Identifies the event entry that needs to be decoded
* 
* 3. UINT8 safe
*   Specifies if PII needs to be hidden
* 
* Output Parameters:
* 
* 1. EVT_DECODED_LOG_ENTRY* record
*    returns the decoded event data in the EVT_DECODED_LOG_ENTRY structure
* 
* Return Values:
*
* 1. LOG_OK for success
* 2. LOG_ERROR for failure
*
************************************************************************/
UINT8 read_decoded_log_entry (int logType, 
                              UINT32 eventID, 
                              EVT_DECODED_LOG_ENTRY* record,
                              UINT8 safe)
{
    EVT_LOG_ENTRY* evt;
    int rc;
    char buf[EVT_MAX_SIZE] = {0};
    UINT16 maxLen;
    evt = (EVT_LOG_ENTRY*)buf;
    maxLen = sizeof(buf);
    memset (record, 0x00, sizeof(EVT_DECODED_LOG_ENTRY));

    rc = logs_entry_read(logType, eventID, evt, &maxLen, 0);
    if (rc == LOG_EVENT_FOUND) {
        rc = generic_decode(logType, evt, record, safe);
    }
    return rc;
}
#endif //0

int find_index_iml_data_extract(int dataExtractID, UINT16 evtClass, UINT16 evtCode)
{
    int rc=-1; // initialize to invalid
    int index; 
    // TODO: Enhance the search logic based on rendering performance. May be use Binary search instead
    // of linear search
    
    switch (dataExtractID) {
        case EVT_GEN_DATA_ID:
            // Linear Search
            for (index=0;index<EVT_IML_GEN_DATA_ENTRIES;index++) {
                if (List_IML_General_Data[index].evtClass == evtClass &&
                    List_IML_General_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break; 
        case EVT_ACT_DATA_ID:
            for (index=0;index<EVT_IML_ACT_DATA_ENTRIES;index++) {
                if (List_IML_Action_Data[index].evtClass == evtClass &&
                    List_IML_Action_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        case EVT_DESC_DATA_ID:
            for (index=0;index<EVT_IML_DESC_DATA_ENTRIES;index++) {
                if (List_IML_Desc_Data[index].evtClass == evtClass &&
                    List_IML_Desc_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        case EVT_VAR_DATA_ID:
            for (index=0;index<EVT_IML_VAR_DATA_ENTRIES;index++) {
                if (List_IML_Var_Data[index].evtClass == evtClass &&
                    List_IML_Var_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        case EVT_TRAP_DATA_ID:
            for (index=0;index<EVT_IML_TRAP_DATA_ENTRIES;index++) {
                if (List_IML_Trap_Data[index].evtClass == evtClass &&
                    List_IML_Trap_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        case EVT_RIS_ALERT_DATA_ID:
            for (index=0;index<EVT_IML_RIS_ALERT_DATA_ENTRIES;index++) {
                if (List_IML_RIS_Alert_Data[index].evtClass == evtClass &&
                    List_IML_RIS_Alert_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        case EVT_SER_EVENT_DATA_ID:
            for (index=0;index<EVT_IML_SER_EVENT_DATA_ENTRIES;index++) {
                if (List_IML_Service_Event_Data[index].evtClass == evtClass &&
                    List_IML_Service_Event_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        default:
            break;
    }
    return rc;
}

#if 0
int find_index_supp_data_extract(int dataExtractID, UINT16 evtClass, UINT16 evtCode)
{
    int rc=-1; // initialize to invalid
    int index; 
    // TODO: Enhance the search logic based on rendering performance. May be use Binary search instead
    // of linear search
    
    switch (dataExtractID) {
        case EVT_GEN_DATA_ID:
            // Linear Search
            for (index=0;index<(binHeader->genDetails.evtGeneralDataCount);index++) {
                if (gData[index].evtClass == evtClass &&
                    gData[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break; 
        case EVT_ACT_DATA_ID:
            for (index=0;index<(binHeader->actDetails.evtActionTextDataCount);index++) {
                if (aData[index].evtClass == evtClass &&
                    aData[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        case EVT_DESC_DATA_ID:
            for (index=0;index<(binHeader->descDetails.evtDescTextDataCount);index++) {
                if (dData[index].evtClass == evtClass &&
                    dData[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        case EVT_VAR_DATA_ID:
            for (index=0;index<(binHeader->varDetails.evtVarDataCount);index++) {
                if (vData[index].evtClass == evtClass &&
                    vData[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        case EVT_TRAP_DATA_ID:
            for (index=0;index<(binHeader->trapDetails.evtTrapDataCount);index++) {
                if (tData[index].evtClass == evtClass &&
                    tData[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        case EVT_RIS_ALERT_DATA_ID:
            for (index=0;index<(binHeader->risDetails.evtRISAlertDataCount);index++) {
                if (rData[index].evtClass == evtClass &&
                    rData[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        case EVT_SER_EVENT_DATA_ID:
            for (index=0;index<(binHeader->serDetails.evtServiceEventDataCount);index++) {
                if (sData[index].evtClass == evtClass &&
                    sData[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;

        default:
            break;
    }
    return rc;
}
#endif // 0

int find_index_sl_data_extract(int dataExtractID, UINT16 evtClass, UINT16 evtCode)
{
    int rc=-1; // initialize to invalid
    int index; 
    // TODO: Enhance the search logic based on rendering performance. May be use Binary search instead
    // of linear search
    
    switch (dataExtractID) {
        case EVT_GEN_DATA_ID:
            // Linear Search
            for (index=0;index<EVT_SL_GEN_DATA_ENTRIES;index++) {
                if (List_SL_General_Data[index].evtClass == evtClass &&
                    List_SL_General_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break; 
        case EVT_ACT_DATA_ID:
            for (index=0;index<EVT_SL_ACT_DATA_ENTRIES;index++) {
                if (List_SL_Action_Data[index].evtClass == evtClass &&
                    List_SL_Action_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        case EVT_DESC_DATA_ID:
            for (index=0;index<EVT_SL_DESC_DATA_ENTRIES;index++) {
                if (List_SL_Desc_Data[index].evtClass == evtClass &&
                    List_SL_Desc_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        case EVT_VAR_DATA_ID:
            for (index=0;index<EVT_SL_VAR_DATA_ENTRIES;index++) {
                if (List_SL_Var_Data[index].evtClass == evtClass &&
                    List_SL_Var_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        case EVT_TRAP_DATA_ID:
            for (index=0;index<EVT_SL_TRAP_DATA_ENTRIES;index++) {
                if (List_SL_Trap_Data[index].evtClass == evtClass &&
                    List_SL_Trap_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        case EVT_RIS_ALERT_DATA_ID:
            for (index=0;index<EVT_SL_RIS_ALERT_DATA_ENTRIES;index++) {
                if (List_SL_RIS_Alert_Data[index].evtClass == evtClass &&
                    List_SL_RIS_Alert_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        case EVT_SER_EVENT_DATA_ID:
            for (index=0;index<EVT_SL_SER_EVENT_DATA_ENTRIES;index++) {
                if (List_SL_Service_Event_Data[index].evtClass == evtClass &&
                    List_SL_Service_Event_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        default:
            break;
    }
    return rc;
}

#if 0
/*******************************************************************
*  Function name: get_event_hdr_for_trap
*
*  Description:
*       This function get the iML header using the trap id
*
*  Parameters:
*       trapID:           trap Id
*       evlogHrd:         pointer to the event header
*
*  Returns:
*       -1 = Failure
*       0  = Success
*
*******************************************************************/
int get_event_hdr_for_trap(int trapID, EVT_HEADER *evlogHrd ) // TODO: Add arrays for IEL
{
    int evt_index, trap_index;

    for (evt_index=0;evt_index<EVT_IML_TRAP_DATA_ENTRIES;evt_index++)
    {
        if (List_IML_Trap_Data[evt_index].trapDetails)
        {
            // If trapDetails is not null trapCount will not be zero
            for (trap_index = 0; trap_index <  List_IML_Trap_Data[evt_index].trapCount; trap_index++) 
            {
                if (trapID == List_IML_Trap_Data[evt_index].trapDetails[trap_index].trapId )
                {
                    evlogHrd->severity = List_IML_Trap_Data[evt_index].trapDetails[trap_index].mappedSeverity;
                    evlogHrd->evtClass = List_IML_Trap_Data[evt_index].evtClass;
                    evlogHrd->evtCode = List_IML_Trap_Data[evt_index].evtCode ;
                    return 0;
                }
            }
        } // End of List_IML_Trap_Data[evt_index].trapDetails check
    }

    //try to search in Security Events list
    for (evt_index=0;evt_index<EVT_SL_TRAP_DATA_ENTRIES;evt_index++)
    {
        if (List_SL_Trap_Data[evt_index].trapDetails)
        {
            // If trapDetails is not null trapCount will not be zero
            for (trap_index = 0; trap_index <  List_SL_Trap_Data[evt_index].trapCount; trap_index++) 
            {
                if (trapID == List_SL_Trap_Data[evt_index].trapDetails[trap_index].trapId )
                {
                    evlogHrd->severity = List_SL_Trap_Data[evt_index].trapDetails[trap_index].mappedSeverity;
                    evlogHrd->evtClass = List_SL_Trap_Data[evt_index].evtClass;
                    evlogHrd->evtCode = List_SL_Trap_Data[evt_index].evtCode ;
                    return 0;
                }
            }
        } // End of List_SL_Trap_Data[evt_index].trapDetails check
    }
    
    return -1;
}
#endif // 0

int find_index_iel_data_extract(int dataExtractID, UINT16 evtClass, UINT16 evtCode)
{
    int rc=-1; // initialize to invalid
    int index;
     
    // TODO: Enhance the search logic based on rendering performance. May be use Binary search instead
    // of linear search   
    switch (dataExtractID) {
        case EVT_GEN_DATA_ID:
            for (index=0;index<EVT_IEL_GEN_DATA_ENTRIES;index++) {
                if (List_IEL_General_Data[index].evtClass == evtClass &&
                    List_IEL_General_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
    case EVT_ACT_DATA_ID:
        //Check if any IEL event has action details.
        //This condition added to avoid coverity check.
        if (EVT_IEL_ACT_DATA_ENTRIES != 0) {
            for (index = 0; index < EVT_IEL_ACT_DATA_ENTRIES; index++) {
                    if (List_IEL_Action_Data[index].evtClass == evtClass &&
                        List_IEL_Action_Data[index].evtCode == evtCode) {
                        rc = index;
                        break;
                    }
                }
        }
            break;
        case EVT_DESC_DATA_ID:
            for (index=0;index<EVT_IEL_DESC_DATA_ENTRIES;index++) {
                if (List_IEL_Desc_Data[index].evtClass == evtClass &&
                    List_IEL_Desc_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
        case EVT_VAR_DATA_ID:
            for (index=0;index<EVT_IEL_VAR_DATA_ENTRIES;index++) {
                if (List_IEL_Var_Data[index].evtClass == evtClass &&
                    List_IEL_Var_Data[index].evtCode == evtCode) {
                    rc = index;
                    break;
                }
            }
            break;
    case EVT_TRAP_DATA_ID:
        //Check if any IEL event has SNMP trap details.
        //This condition added to avoid coverity check.
         if (EVT_IEL_TRAP_DATA_ENTRIES != 0) {
            for (index = 0; index < EVT_IEL_TRAP_DATA_ENTRIES; index++) {
                    if (List_IEL_Trap_Data[index].evtClass == evtClass &&
                        List_IEL_Trap_Data[index].evtCode == evtCode) {
                        rc = index;
                        break;
                    }
                }
          }
          break;
    case EVT_RIS_ALERT_DATA_ID:
          //Check if any IEL event has RIS alert details.
          //This condition added to avoid coverity check.
          if (EVT_IEL_RIS_ALERT_DATA_ENTRIES != 0) {
            for (index = 0; index < EVT_IEL_RIS_ALERT_DATA_ENTRIES; index++) {
                    if (List_IEL_RIS_Alert_Data[index].evtClass == evtClass &&
                        List_IEL_RIS_Alert_Data[index].evtCode == evtCode) {
                        rc = index;
                        break;
                    }
                }
          }
          break;
    case EVT_SER_EVENT_DATA_ID:
          //Check if any IEL event has Service event details.
          //This condition added to avoid coverity check.
          if (EVT_IEL_SER_EVENT_DATA_ENTRIES != 0) {
            for (index = 0; index < EVT_IEL_SER_EVENT_DATA_ENTRIES; index++) {
                    if (List_IEL_Service_Event_Data[index].evtClass == evtClass &&
                        List_IEL_Service_Event_Data[index].evtCode == evtCode) {
                        rc = index;
                        break;
                    }
                }
          }
          break;
        default:
            break;
    }
    return rc;
}

#pragma GCC diagnostic ignored "-Wstringop-overread"
UINT8 extract_variable_value (EVT_LOG_ENTRY* record, 
                              UINT8 varOffset, 
                              UINT8 varWidth, 
                              UINT8 varRep,
                              UINT8 varRepDigits,
//                              UINT8 ahsFlag,
//                              char* varName, 
                              char **argVal)
{
    char tempBuf[EVT_MAX_VARIABLE_SIZE] = {0};
//    char tmpVarName[128] = {0};
    char *ptr = NULL;
    int k;
    int len;
    int tempInt;
    int size;
    UINT8* start=NULL;

    start = (UINT8*)(&(record->data));
    if (start == NULL) {
        return LOG_ERROR;
    }
    if (varRep == EVT_REP_INTEGER || varRep == EVT_REP_HEX) { // Integer or Hex
        size = (varWidth * 3) + 1; // Max 1 byte value in decimal can be 255. 2 byte 65535 and so on. Additional byte for NULL char.
                                   // This calculation doesn't represent the optimum size required (little more than required for 2,4,8 byte int)
                                   // but is simple fix without changing much of the logic and flow.
        ptr = (char*)malloc(size);

        if (ptr !=NULL)	{
            memset(ptr,0,size);
            memcpy(tempBuf, (start+varOffset), varWidth);
            tempBuf[varWidth] = '\0';
            tempInt = *((int*)tempBuf);
            if (varRep == EVT_REP_INTEGER){
                if (varRepDigits) {
                    snprintf(ptr, size, "%0*u", varRepDigits, tempInt); 
                }
                else {
                    snprintf(ptr, size, "%u", tempInt);
                }
            }
            else {
                if (varRepDigits) {
                    snprintf(ptr, size, "%0*X", varRepDigits, tempInt);
                }
                else {
                    snprintf(ptr, size, "%X", tempInt);
                }
            }
            *argVal = ptr;	
        }
        else {
            return LOG_ERROR;
        }
    }
    else if (varRep == EVT_REP_STRING) { // String
        int strOffset = varOffset;
        for (k=1;k<varWidth;k++) {
            strOffset += strnlen((char*)(start + strOffset), (EVT_MAX_SIZE - sizeof(EVT_HEADER)));
            strOffset += 1;
        }
        len = strnlen ((char*)(start + strOffset), (EVT_MAX_SIZE - sizeof(EVT_HEADER)));
		
        if (len>MAX_EVT_VAR_STR_LEN) {
            len= MAX_EVT_VAR_STR_LEN;
        }

        ptr = (char*)malloc(len + 1);
        if (ptr != NULL) {
            memset(ptr, 0, len + 1);
            strncpy(ptr, (char *)(start + strOffset), len + 1);
            ptr[len] = '\0';
            *argVal = ptr;
    }
        else {
            return LOG_ERROR;
        }
    } 
    else {
        return LOG_ERROR; // unknown representation
    } 
    return LOG_OK; // Success
}

UINT8 get_description_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_TEXT_DATA** textData)
{
    int index;
//    int supp_index;
    if (logType == LOGS_IMLDATA) {
        // get event description data
//        if (supplemental_data_intialized == 1 && supp_extract_mutex_lock() == Success) {
//            supp_index = find_index_supp_data_extract(EVT_DESC_DATA_ID, evtClass, evtCode);
//            supp_extract_mutex_unlock();
//            if (supp_index==-1) {
//                index = find_index_iml_data_extract(EVT_DESC_DATA_ID, evtClass, evtCode); 
//                if (index==-1) {
//                    return LOG_ERROR;
//                }
//                if (*textData == NULL) {
//                    *textData = &(List_IML_Desc_Data[index]); 
//                }
//            }
//            else {
//                FYI(FYI_INFORM, "EVLOG: Desc - Entry found in supplemental Data Extract. Class [0x%x] and Code [0x%x]\n", evtClass, evtCode);
//                if (*textData == NULL) {
//                    *textData = &(dData[supp_index]);
//                }
//            }
//        }
//        else { 
            index = find_index_iml_data_extract(EVT_DESC_DATA_ID, evtClass, evtCode);
            if (index==-1) {
                return LOG_ERROR;
            }
            if (*textData == NULL) {
                *textData = &(List_IML_Desc_Data[index]); 
            }
//        }
    }
    else if (logType == LOGS_GELDATA) {
        // get event description data 
        index = find_index_iel_data_extract(EVT_DESC_DATA_ID, evtClass, evtCode);
        if (index==-1) {
            return LOG_ERROR;
        }
        if (*textData == NULL) {
            *textData = &(List_IEL_Desc_Data[index]); 
        }
    }
    else if (logType == LOGS_SLDATA) {
        // get event description data
//        if (supplemental_data_intialized == 1 && supp_extract_mutex_lock() == Success) {
//            supp_index = find_index_supp_data_extract(EVT_DESC_DATA_ID, evtClass, evtCode);
//            supp_extract_mutex_unlock();
//            if (supp_index==-1) {
//                index = find_index_sl_data_extract(EVT_DESC_DATA_ID, evtClass, evtCode); 
//                if (index==-1) {
//                    return LOG_ERROR;
//                }
//                if (*textData == NULL) {
//                    *textData = &(List_SL_Desc_Data[index]); 
//                }
//            }
//            else {
//                printf("EVLOG: Desc - Entry found in SL supplemental Data Extract. Class [0x%x] and Code [0x%x]\n", evtClass, evtCode);
//                if (*textData == NULL) {
//                    *textData = &(dData[supp_index]);
//                }
//            }
//        }
//        else { 
            index = find_index_sl_data_extract(EVT_DESC_DATA_ID, evtClass, evtCode);
            if (index==-1) {
                return LOG_ERROR;
            }
            if (*textData == NULL) {
                *textData = &(List_SL_Desc_Data[index]); 
            }
//        }
    }
    else if (logType == LOGS_DPUDATA) {
        index = find_index_iml_data_extract(EVT_DESC_DATA_ID, evtClass, evtCode);
        if (index==-1) {
            return LOG_ERROR;
        }
        if (*textData == NULL) {
            *textData = &(List_IML_Desc_Data[index]); 
        }
    }
    else {
        return LOG_ERROR;
    }
    return LOG_OK;
}

UINT8 get_action_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_TEXT_DATA** textData)
{
    int index;
//    int supp_index;
    if (logType == LOGS_IMLDATA) {
        // get event action data
//        if (supplemental_data_intialized == 1 && supp_extract_mutex_lock() == Success) {
//            supp_index = find_index_supp_data_extract(EVT_ACT_DATA_ID, evtClass, evtCode);
//            supp_extract_mutex_unlock();
//            if (supp_index==-1) {
//                index = find_index_iml_data_extract(EVT_ACT_DATA_ID, evtClass, evtCode); 
//                if (index==-1) {
//                    return LOG_ERROR;
//                }
//                if (*textData == NULL) {
//                    *textData = &(List_IML_Action_Data[index]); 
//                }
//            }
//            else {
//                FYI(FYI_INFORM, "EVLOG: Action - Entry found in supplemental Data Extract. Class [0x%x] and Code [0x%x]\n", evtClass, evtCode);
//                if (*textData == NULL) {
//                    *textData = &(aData[supp_index]);
//                }
//            }
//        }
//        else {
            index = find_index_iml_data_extract(EVT_ACT_DATA_ID, evtClass, evtCode);
            if (index==-1) {
                return LOG_ERROR;
            }
            if (*textData == NULL) {
                *textData = &(List_IML_Action_Data[index]); 
            }
//        }
    }
    else if (logType == LOGS_GELDATA) {
        // get event action data 
        index = find_index_iel_data_extract(EVT_ACT_DATA_ID, evtClass, evtCode);
        if (index==-1) {
            return LOG_ERROR;
        }
        if (*textData == NULL) {
            *textData = &(List_IEL_Action_Data[index]); 
        }
    }
    else if (logType == LOGS_SLDATA) {
        // get event action data
//        if (supplemental_data_intialized == 1 && supp_extract_mutex_lock() == Success) {
//            supp_index = find_index_supp_data_extract(EVT_ACT_DATA_ID, evtClass, evtCode);
//            supp_extract_mutex_unlock();
//            if (supp_index==-1) {
//                index = find_index_sl_data_extract(EVT_ACT_DATA_ID, evtClass, evtCode); 
//                if (index==-1) {
//                    return LOG_ERROR;
//                }
//                if (*textData == NULL) {
//                    *textData = &(List_SL_Action_Data[index]); 
//                }
//            }
//            else {
//                printf("EVLOG: Action - Entry found in SL supplemental Data Extract. Class [0x%x] and Code [0x%x]\n", evtClass, evtCode);
//                if (*textData == NULL) {
//                    *textData = &(aData[supp_index]);
//                }
//            }
//        }
//        else {
            index = find_index_sl_data_extract(EVT_ACT_DATA_ID, evtClass, evtCode);
            if (index==-1) {
                return LOG_ERROR;
            }
            if (*textData == NULL) {
                *textData = &(List_SL_Action_Data[index]); 
            }
//        }
    }
    else {
        return LOG_ERROR;
    }
    return LOG_OK;
}

UINT8 get_variable_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_VAR_DATA** varData)
{
    int index;
//    int supp_index;
    if (logType == LOGS_IMLDATA) {
        // get event variable data
//        if (supplemental_data_intialized == 1 && supp_extract_mutex_lock() == Success) {
//            supp_index = find_index_supp_data_extract(EVT_VAR_DATA_ID, evtClass, evtCode);
//            //supp_extract_mutex_unlock();
//            if (supp_index==-1) {
//                index = find_index_iml_data_extract(EVT_VAR_DATA_ID, evtClass, evtCode); 
//                if (index==-1) {
//                    return DECODE_DATA_NOT_FOUND;//LOG_ERROR;
//                }
//                if (*varData == NULL) {
//                    *varData = &(List_IML_Var_Data[index]); 
//                }
//            }
//            else {
//                FYI(FYI_INFORM, "EVLOG: Var - Entry found in supplemental Data Extract. Class [0x%x] and Code [0x%x]\n", evtClass, evtCode);
//                if (*varData == NULL) {
//                    *varData = &(vData[supp_index]);
//                }
//            }
//        }
//        else {
            index = find_index_iml_data_extract(EVT_VAR_DATA_ID, evtClass, evtCode);
            if (index==-1) {
                return DECODE_DATA_NOT_FOUND;//LOG_ERROR;
            }
            if (*varData == NULL) {
                *varData = &(List_IML_Var_Data[index]); 
            }
//        }
    }
    else if (logType == LOGS_GELDATA) {
        // get event variable data 
        index = find_index_iel_data_extract(EVT_VAR_DATA_ID, evtClass, evtCode);
        if (index==-1) {
            return DECODE_DATA_NOT_FOUND;//LOG_ERROR;
        }
        if (*varData == NULL) {
            *varData = &(List_IEL_Var_Data[index]); 
        }
    }
    else if (logType == LOGS_SLDATA) {
        // get event variable data
//        if (supplemental_data_intialized == 1 && supp_extract_mutex_lock() == Success) {
//            supp_index = find_index_supp_data_extract(EVT_VAR_DATA_ID, evtClass, evtCode);
//            //supp_extract_mutex_unlock();
//            if (supp_index==-1) {
//                index = find_index_sl_data_extract(EVT_VAR_DATA_ID, evtClass, evtCode); 
//                if (index==-1) {
//                    return DECODE_DATA_NOT_FOUND;//LOG_ERROR;
//                }
//                if (*varData == NULL) {
//                    *varData = &(List_SL_Var_Data[index]); 
//                }
//            }
//            else {
//                FYI(FYI_INFORM, "EVLOG: Var - Entry found in SL supplemental Data Extract. Class [0x%x] and Code [0x%x]\n", evtClass, evtCode);
//                if (*varData == NULL) {
//                    *varData = &(vData[supp_index]);
//                }
//            }
//        }
//        else {
            index = find_index_sl_data_extract(EVT_VAR_DATA_ID, evtClass, evtCode);
            if (index==-1) {
                return DECODE_DATA_NOT_FOUND;//LOG_ERROR;
            }
            if (*varData == NULL) {
                *varData = &(List_SL_Var_Data[index]); 
            }
//        }
    }
    else if (logType == LOGS_DPUDATA) {
        index = find_index_iml_data_extract(EVT_VAR_DATA_ID, evtClass, evtCode);
        if (index==-1) {
             return DECODE_DATA_NOT_FOUND;//LOG_ERROR;
        }
        if (*varData == NULL) {
            *varData = &(List_IML_Var_Data[index]); 
        }
    }
    else {
        return LOG_ERROR;
    }
    return LOG_OK;
}

#if 0
UINT8 get_trap_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_TRAP_DATA** trapData)
{
    int index, supp_index;
    if (logType == LOGS_IMLDATA) {
        // get event trap data 
        if (supplemental_data_intialized == 1 && supp_extract_mutex_lock() == Success) {
            supp_index = find_index_supp_data_extract(EVT_TRAP_DATA_ID, evtClass, evtCode);
            supp_extract_mutex_unlock();
            if (supp_index==-1) {
                index = find_index_iml_data_extract(EVT_TRAP_DATA_ID, evtClass, evtCode); 
                if (index==-1) {
                    return LOG_ERROR;
                }
                if (*trapData == NULL) {
                    *trapData = &(List_IML_Trap_Data[index]); 
                }
            }
            else {
                if (*trapData == NULL) {
                    *trapData = &(tData[supp_index]);
                }
            }
        }
        else {
            index = find_index_iml_data_extract(EVT_TRAP_DATA_ID, evtClass, evtCode);
            if (index==-1) {
                return LOG_ERROR;
            }
            if (*trapData == NULL) {
                *trapData = &(List_IML_Trap_Data[index]); 
            }
        }
    }
    else if (logType == LOGS_GELDATA) {
        // get event trap data 
        index = find_index_iel_data_extract(EVT_TRAP_DATA_ID, evtClass, evtCode);
        if (index==-1) {
            return LOG_ERROR;
        }
        if (*trapData == NULL) {
            *trapData = &(List_IEL_Trap_Data[index]);
        }
    }
    else if (logType == LOGS_SLDATA) {
        // get event trap data 
        if (supplemental_data_intialized == 1 && supp_extract_mutex_lock() == Success) {
            supp_index = find_index_supp_data_extract(EVT_TRAP_DATA_ID, evtClass, evtCode);
            supp_extract_mutex_unlock();
            if (supp_index==-1) {
                index = find_index_sl_data_extract(EVT_TRAP_DATA_ID, evtClass, evtCode); 
                if (index==-1) {
                    return LOG_ERROR;
                }
                if (*trapData == NULL) {
                    *trapData = &(List_SL_Trap_Data[index]); 
                }
            }
            else {
                if (*trapData == NULL) {
                    *trapData = &(tData[supp_index]);
                }
            }
        }
        else {
            index = find_index_sl_data_extract(EVT_TRAP_DATA_ID, evtClass, evtCode);
            if (index==-1) {
                return LOG_ERROR;
            }
            if (*trapData == NULL) {
                *trapData = &(List_SL_Trap_Data[index]); 
            }
        }
    }
    else {
        return LOG_ERROR;
    }
    return LOG_OK;
}

UINT8 get_ris_alert_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_RIS_ALERT_DATA** risData)
{
    int index, supp_index;
    if (logType == LOGS_IMLDATA) {
        // get event's ris alert data 
        if (supplemental_data_intialized == 1 && supp_extract_mutex_lock() == Success) {
            supp_index = find_index_supp_data_extract(EVT_RIS_ALERT_DATA_ID, evtClass, evtCode);
            supp_extract_mutex_unlock();
            /* if supplement (BIOS) DataExtract minor version is less than iLO DataExtract minor version or if the version
               matches then check of supplement errata veriosn and supplement errata veriosn is less than iLO errata version 
               then use iLO DataExtract IML data else use the rData which is BIOS DataExtract supplement IML data*/
            if (supp_index==-1 || !((binHeader->verDetails.dataExtractVersion.minor > DATA_EXTRACT_MINOR_NUMBER)
                               || ((binHeader->verDetails.dataExtractVersion.minor == DATA_EXTRACT_MINOR_NUMBER)
                               && (binHeader->verDetails.dataExtractVersion.errata > DATA_EXTRACT_ERRATA_NUMBER)))) {
                index = find_index_iml_data_extract(EVT_RIS_ALERT_DATA_ID, evtClass, evtCode);
                if (index==-1) {
                    return LOG_ERROR;
                }
                if (*risData == NULL) {
                    *risData = &(List_IML_RIS_Alert_Data[index]);
                }
            }
            else {
                FYI(FYI_INFORM, "EVLOG: Ris - Entry found in supplemental Data Extract. Class [0x%x] and Code [0x%x]\n", evtClass, evtCode);
                if (*risData == NULL) {
                    *risData = &(rData[supp_index]);
                }
            }
        }
        else {
            index = find_index_iml_data_extract(EVT_RIS_ALERT_DATA_ID, evtClass, evtCode); 
            if (index==-1) {
                return LOG_ERROR;
            }
            if (*risData == NULL) {
                *risData = &(List_IML_RIS_Alert_Data[index]);
            }
        }
    }
    else if (logType == LOGS_GELDATA) {
        // get event general data
        index = find_index_iel_data_extract(EVT_RIS_ALERT_DATA_ID, evtClass, evtCode);
        if (index==-1) {
            return LOG_ERROR;
        }
        if (*risData == NULL) {
            *risData = &(List_IEL_RIS_Alert_Data[index]);
        }
    }
    else if (logType == LOGS_SLDATA) {
        // get event's ris alert data 
        if (supplemental_data_intialized == 1 && supp_extract_mutex_lock() == Success) {
            supp_index = find_index_supp_data_extract(EVT_RIS_ALERT_DATA_ID, evtClass, evtCode);
            supp_extract_mutex_unlock();
            if (supp_index==-1) {
                index = find_index_sl_data_extract(EVT_RIS_ALERT_DATA_ID, evtClass, evtCode);
                if (index==-1) {
                    return LOG_ERROR;
                }
                if (*risData == NULL) {
                    *risData = &(List_SL_RIS_Alert_Data[index]);
                }
            }
            else {
                FYI(FYI_INFORM, "EVLOG: Ris - Entry found in SL supplemental Data Extract. Class [0x%x] and Code [0x%x]\n", evtClass, evtCode);
                if (*risData == NULL) {
                    *risData = &(rData[supp_index]);
                }
            }
        }
        else {
            index = find_index_sl_data_extract(EVT_RIS_ALERT_DATA_ID, evtClass, evtCode); 
            if (index==-1) {
                return LOG_ERROR;
            }
            if (*risData == NULL) {
                *risData = &(List_SL_RIS_Alert_Data[index]);
            }
        }
    }
    else {
        return LOG_ERROR;
    }
    return LOG_OK;
}

UINT8 get_service_event_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_SERVICE_EVENT_DATA** serData)
{
    int index, supp_index;
    if (logType == LOGS_IMLDATA) {
        // get event's service event data 
        if (supplemental_data_intialized == 1 && supp_extract_mutex_lock() == Success) {
            supp_index = find_index_supp_data_extract(EVT_SER_EVENT_DATA_ID, evtClass, evtCode);
            supp_extract_mutex_unlock();
            if (supp_index==-1) {
                index = find_index_iml_data_extract(EVT_SER_EVENT_DATA_ID, evtClass, evtCode);
                if (index==-1) {
                    return LOG_ERROR;
                }
                if (*serData == NULL) {
                    *serData = &(List_IML_Service_Event_Data[index]);
                }
            }
            else {
                FYI(FYI_INFORM, "EVLOG: Ser - Entry found in supplemental Data Extract. Class [0x%x] and Code [0x%x]\n", evtClass, evtCode);
                if (*serData == NULL) {
                    *serData = &(sData[supp_index]);
                }
            }
        }
        else {
            index = find_index_iml_data_extract(EVT_SER_EVENT_DATA_ID, evtClass, evtCode);
            if (index==-1) {
                return LOG_ERROR;
            }
            if (*serData == NULL) {
                *serData = &(List_IML_Service_Event_Data[index]);
            }
        }
    }
    else if (logType == LOGS_GELDATA) {
        // get event's service event data
        index = find_index_iel_data_extract(EVT_SER_EVENT_DATA_ID, evtClass, evtCode);
        if (index==-1) {
            return LOG_ERROR;
        }
        if (*serData == NULL) {
            *serData = &(List_IEL_Service_Event_Data[index]);
        }
    }
    else if (logType == LOGS_SLDATA) {
        // get event's service event data 
        if (supplemental_data_intialized == 1 && supp_extract_mutex_lock() == Success) {
            supp_index = find_index_supp_data_extract(EVT_SER_EVENT_DATA_ID, evtClass, evtCode);
            supp_extract_mutex_unlock();
            if (supp_index==-1) {
                index = find_index_sl_data_extract(EVT_SER_EVENT_DATA_ID, evtClass, evtCode);
                if (index==-1) {
                    return LOG_ERROR;
                }
                if (*serData == NULL) {
                    *serData = &(List_SL_Service_Event_Data[index]);
                }
            }
            else {
                FYI(FYI_INFORM, "EVLOG: Ser - Entry found in SL supplemental Data Extract. Class [0x%x] and Code [0x%x]\n", evtClass, evtCode);
                if (*serData == NULL) {
                    *serData = &(sData[supp_index]);
                }
            }
        }
        else {
            index = find_index_sl_data_extract(EVT_SER_EVENT_DATA_ID, evtClass, evtCode);
            if (index==-1) {
                return LOG_ERROR;
            }
            if (*serData == NULL) {
                *serData = &(List_SL_Service_Event_Data[index]);
            }
        }
    }
    else {
        return LOG_ERROR;
    }
    return LOG_OK;
}


UINT8 get_general_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_GENERAL_DATA** genData)
{
    int index, supp_index;
    if (logType == LOGS_IMLDATA) {
        // get event general data 
        if (supplemental_data_intialized == 1 && supp_extract_mutex_lock() == Success) {
            supp_index = find_index_supp_data_extract(EVT_GEN_DATA_ID, evtClass, evtCode);
            supp_extract_mutex_unlock();
            if (supp_index==-1) {
                index = find_index_iml_data_extract(EVT_GEN_DATA_ID, evtClass, evtCode); 
                if (index==-1) {
                    return LOG_ERROR;
                }
                if (*genData == NULL) {
                    *genData = &(List_IML_General_Data[index]); 
                }
            }
            else {
                FYI(FYI_INFORM, "EVLOG: Gen - Entry found in supplemental Data Extract. Class [0x%x] and Code [0x%x]\n", evtClass, evtCode);
                if (*genData == NULL) {
                    *genData = &(gData[supp_index]);
                }
            }
        }
        else {
            index = find_index_iml_data_extract(EVT_GEN_DATA_ID, evtClass, evtCode); 
            if (index==-1) {
                return LOG_ERROR;
            }
            if (*genData == NULL) {
                *genData = &(List_IML_General_Data[index]); 
            }
        }
    }
    else if (logType == LOGS_GELDATA) {
        // get event general data 
        index = find_index_iel_data_extract(EVT_GEN_DATA_ID, evtClass, evtCode);
        if (index==-1) {
            return LOG_ERROR;
        }
        if (*genData == NULL) {
            *genData = &(List_IEL_General_Data[index]); 
        }
    }
    else if (logType == LOGS_SLDATA) {
        // get event general data 
        if (supplemental_data_intialized == 1 && supp_extract_mutex_lock() == Success) {
            supp_index = find_index_supp_data_extract(EVT_GEN_DATA_ID, evtClass, evtCode);
            supp_extract_mutex_unlock();
            if (supp_index==-1) {
                index = find_index_sl_data_extract(EVT_GEN_DATA_ID, evtClass, evtCode); 
                if (index==-1) {
                    return LOG_ERROR;
                }
                if (*genData == NULL) {
                    *genData = &(List_SL_General_Data[index]); 
                }
            }
            else {
                FYI(FYI_INFORM, "EVLOG: Gen - Entry found in SL supplemental Data Extract. Class [0x%x] and Code [0x%x]\n", evtClass, evtCode);
                if (*genData == NULL) {
                    *genData = &(gData[supp_index]);
                }
            }
        }
        else {
            index = find_index_sl_data_extract(EVT_GEN_DATA_ID, evtClass, evtCode); 
            if (index==-1) {
                return LOG_ERROR;
            }
            if (*genData == NULL) {
                *genData = &(List_SL_General_Data[index]); 
            }
        }
    }
    else if (logType == LOGS_DPUDATA) {
        index = find_index_iml_data_extract(EVT_GEN_DATA_ID, evtClass, evtCode); 
        if (index==-1) {
           return LOG_ERROR;
        }
        if (*genData == NULL) {
           *genData = &(List_IML_General_Data[index]); 
        }
    }
    else {
        return LOG_ERROR;
    }
    return LOG_OK;
}

/* oem_profile_in_use()
 *
 * Return 0: Not rebranded
 *        1: OEM profile active
 *        2: OEM degraded
 *        3: OEM NAND problem
 */
int oem_profile_in_use_s(const char *name)
{
   return 0;  // no OEM branding yet
   //OEM_REQ req;
   //OEM_RSP rsp;
   //
   //memset(&rsp, 0, sizeof(rsp));
   //
   //req.cmd = OEM_EFFECT;
   //SNPRINTF_S(req.name, sizeof(req.name), "%s", name);
   //
   //CHECK_ERROR(VComClientSync_Call(OEM_SVCNAME, (char *)&req, sizeof(req), (char *)&rsp, sizeof(rsp)));
   //
   //return rsp.rc;
}

// FIXME: need to figure out what this will be for openbmc !!!

/* oem_branding_token_swap()
 * oem_branding_token_swap_s()
 *
 * Given input string, replace hashtag branding tokens into supplied output buffer.
 * If not OEM mode, receives default Hewlett-Packard values
 *
 * #ILO     -> iLO
 * #PRODABR -> iLO 4
 * #VENDABR -> HPE
 * #VENDNAM -> Hewlett Packard Enterprise
 * #WWWBMC  -> www.hpe.com/info/ilo
 * #BLADESYSTEM -> BladeSystem
 * 
 * if #OMIT is found and OEM is in use, then remove text up to next #OMIT or end of text
 *
 * str:   original input string
 * buf:   output buffer
 * szbuf: output buffer capacity
 *
 * Returns: output buffer
 */
char *oem_branding_token_swap(char *str, char *buf, size_t szbuf)
{
   int    i, mi;
   char  *ms, *cur, *out;
   size_t sz;
   static char omit_token[] = "#OMIT";
   static int oem = -1;
   static struct {
      char  *token;
      char  *val;
      UINT32 prop;
   } swap[]= {
      { (char*)"#ILO",         NULL, OEM_PROP_BMC         }, // iLO
      { (char*)"#PRODABR",     NULL, OEM_PROP_PRODABR     }, // iLO 5
      { (char*)"#VENDABR",     NULL, OEM_PROP_VENDABR     }, // HPE
      { (char*)"#VENDNAM",     NULL, OEM_PROP_VENDNAM     }, // Hewlett Packard Enterprise
      { (char*)"#ADVLIC",      NULL, OEM_PROP_ADVLIC      }, // iLO Advanced
      { (char*)"#WWWBMC",      NULL, OEM_PROP_WWWBMC      }, // www.hpe.com/info/ilo
      { (char*)"#BLADESYSTEM", NULL, OEM_PROP_BLADESYSTEM }, // BladeSystem
      { (char*)"#IPROV",       NULL, OEM_PROP_IP          }, // Intelligent Provisioning
      { (char*)"#WWWLML",      NULL, OEM_PROP_WWWLML      }, // www.hpe.com/support  (learn more links)
      { (char*)"#WWW",         NULL, OEM_PROP_WWW         }, // www.hpe.com MUST BE LAST PLACE
   };

   if (!str) return str;
   if (!buf) return str;
   if (!szbuf) return str;
   sz = strnlen(str, szbuf);
   if (sz > szbuf) {
      printf("input string size [%d] eclipses output buffer size [%d]\n  >>%s\n", sz, szbuf, str);
      return str;
   }

   memset(buf, 0, szbuf); // in some cases, the input buffer contains debris.
   out = buf;
   if (oem == -1) {
       // one time init
       oem = oem_profile_in_use();
   }

   while (*str) {
      ms = NULL; // pointer in source string to earliest token
      mi = 0;    // index of earliest modification token
      for (i=0; i<sizeof(swap)/sizeof(swap[0]); i++) {
         cur = strstr(str, swap[i].token);
         /* locate the token for substitution that is closest to the start of the string */
         if (cur) {
            if (ms) {
               if (cur < ms) {
                  ms = cur;
                  mi = i;
               }
            } else {
               ms = cur;
               mi = i;
            }
         }
      }

      // check for omit token separately
      cur = strstr(str, omit_token);
      if (cur) {
          if (ms) {
             if (cur < ms) {
                ms = cur;
                mi = -1;    // mark as omit token
             } // else leave ms, mi pointing to earlier swap token
          } else {
             ms = cur;
             mi = -1;
          }
      }

      /* if there is a substitution token, process that and continue */
      if (ms) {
         /* copy characters from the source into the buffer until reaching the token.
          * advance the string pointer past the token
          * copy the substitute into the output buffer
          * advance the output buffer
          * reduce remaining length
          */
         while (str != ms) {
            *buf = *str;
            buf ++;
            str ++;
            szbuf--;
            if (!szbuf) {
               break;
            }
         }
         if (mi == -1) {
             // process omit token
             sz = strnlen(omit_token , sizeof(omit_token) );
             str += sz;
             if (oem) {
                 // locate ending #OMIT
                 cur = strstr(str, omit_token);
                 if (cur) {
                     // set str after 2nd #OMIT
                     str = cur + sz;
                 } else {
                     // 2nd #OMIT not found, remaining text removed by ending loop
                     break;
                 }
             } // else not oem, token removed as str advanced over #OMIT
         } else {
             // process swap token
             sz = strnlen(swap[mi].token, MAX_TOKEN_LEN);
             str += sz;
             /* pay one-time init penalty if we do not have cached value */
             if (!swap[mi].val) {
                swap[mi].val = (char*)oem_prop_get_init(swap[mi].prop);
             }
             sz = strnlen(swap[mi].val, szbuf);
             if (sz > szbuf) {
                /* error case - would overflow */
                break;
             }
             strncat(buf, swap[mi].val, szbuf);
             buf += sz;
             szbuf -= sz;
         }
         if (!szbuf) {
            break;
         }

         continue;
      }
      /* no more tokens.  Copy the rest of the text */
      if (szbuf) {
         strncat(buf, str, szbuf);
      }
      break;
   }

   return out;
}
#endif // 0


/***********************************************************************
* decode_text() - Source Function
*   translates the text ('description' or 'action' text) IML/IEL/SL log entry
*
* Input Parameters:
*
* 1. int logType - IML or IEL or SL
*
* 2. EVT_LOG_ENTRY *evt
*   Event log entry
* 
* 3. UINT16 len - length of output decoded text buffer
* 
* 4. int textType - 'description' or 'action' text
* 
* Output Parameters:
*
* 1. char *decodedText
*    returns the decoded text in this buffer
*
* Return Values:
*
* 1. LOG_OK for success
* 2. LOG_ERROR for failure
*
************************************************************************/
UINT8 decode_text(LOG_ID logType,
                  EVT_LOG_ENTRY* evt,
                  char* decodedText,
                  unsigned short maxLen,
                  TEXT_TYPE textType)
{
    int i,j;
    UINT8 varDescLen;
    UINT8 rc;
    int argCount = 0;
    UINT8 varNum;
    const char* ptr;
    char *buf_ptr;
//    char *varName=NULL;
    char buf[512] = {0};
    UINT8 varOffset, varWidth, varRep, varRepDigits;//, ahsFlag;
    char* argVal[16]={0}; // max 16 variable data
    char *str_specifier = (char *)"%s";
    UINT8 enumDescFound = 0;
    char oemString[512];
    char message[EVT_MAX_SIZE] = {0};

    EVT_TEXT_DATA *textData=NULL;
    EVT_VAR_DATA *varData=NULL;

    memset(decodedText, 0x0, maxLen);
    memset(oemString,0,sizeof(oemString));

    if (textType == TEXT_DESC) {
        rc = get_description_data(logType, evt->hdr.evtClass, evt->hdr.evtCode, &textData);
        if (rc!=LOG_OK || textData==NULL) {
//            if (((evt->hdr.flags) & EVT_HEADER_FLAGS_SIMULATED_EVENT) && (evt->hdr.hdrRev >= 0x03)) { // simulated event
//                snprintf(decodedText, maxLen, SIMULATED_UNKNOWN_EVENT, evt->hdr.evtClass, evt->hdr.evtCode);
//            }
//            else {
                snprintf(decodedText, maxLen, UNKNOWN_EVENT, evt->hdr.evtClass, evt->hdr.evtCode); 
//            }
            printf("Unknown Event Class [%x] or Event Code [%x] received \n", evt->hdr.evtClass, evt->hdr.evtCode);
            return LOG_ERROR;
        }
        else {
//            if (((evt->hdr.flags) & EVT_HEADER_FLAGS_SIMULATED_EVENT) && (evt->hdr.hdrRev >= 0x03)) { // simulated event
//                SNPRINTF_S(message, EVT_MAX_SIZE, "%s%s", SIMULATED_EVT_TEXT, textData->msg);
//            }
//            else {
                snprintf(message, EVT_MAX_SIZE, "%s", textData->msg);
//            }
        }
    }
    else if (textType == TEXT_ACTION) {
        rc = get_action_data(logType, evt->hdr.evtClass, evt->hdr.evtCode, &textData);
        if (rc!=LOG_OK || textData==NULL) {
            printf("Event Class [%x] Code [%x] received with no Action text \n", evt->hdr.evtClass, evt->hdr.evtCode);
            return LOG_ERROR;
        }
        else {
             snprintf(message, EVT_MAX_SIZE, "%s", textData->msg);
        }
    }
    else if (textType == TEXT_CAUSE) {
        return LOG_ERROR;
    }
    else {
        return LOG_ERROR;
    }

    rc = get_variable_data (logType, evt->hdr.evtClass, evt->hdr.evtCode, &varData);
    if (rc!=LOG_OK || varData==NULL) {
        //FIXME:  Add if supported later.
//        // There is no variable data so just copy the description/action text
//        oem_branding_token_swap(message, decodedText, maxLen);
//        if (strnlen(decodedText, maxLen) == 0)
//            strnlen(decodedText,maxLen,"%s",message);
        return LOG_OK; // This seems to be a no variable data description
    }

    buf_ptr = buf;
    ptr = message;
    printf("Event: %s\n", message);

    // TODO: Add a check for %% in this code - to display % in the text output
    for (; *ptr; ptr++) {
        if (*ptr=='%') {
            if (argCount < textData->argCount) {
                varNum = textData->argsToVarDet[argCount].varNum;
                if (varNum<=varData->varDataCount) {
                    varOffset = varData->varDetails[varNum-1].varOffset;
                    varWidth = varData->varDetails[varNum-1].varWidth;
                    varRep = varData->varDetails[varNum-1].varRepresentation;
                    varRepDigits = varData->varDetails[varNum-1].varRepDigits;
//                    varName = varData->varDetails[varNum-1].varName;
//                    ahsFlag = varData->varDetails[varNum-1].ahsRedactionFlag;
                    rc = extract_variable_value (evt,
                                                 varOffset,
                                                 varWidth,
                                                 varRep,
                                                 varRepDigits,
//                                                 ahsFlag,
//                                                 varName,
                                                 &(argVal[argCount]));
//                    printf("Found argument: varName: %s, value:%s\n", varName, argVal[argCount]);
                    if (rc != LOG_OK) {
                        // FIXME: Add if supported later.
//                        // free
//                        for (i=0; i<16; i++) {
//                            if (argVal[i]!=NULL) {
//                                free(argVal[i]);
//                            }
//                        }
//                        oem_branding_token_swap(message, decodedText, maxLen);
//                        if (strnlen(decodedText, maxLen) == 0)
//                            snprintf(decodedText,maxLen,"%s",message);
                        return rc;
                    }
                    // TODO: display integer values min 2 digits
                    if (textData->argsToVarDet[argCount].replaceWith == REPLACE_DESC) {
                        // replace with variable's description
                        for (j=0;j<varData->varDetails[varNum-1].valDescCount; j++) {
                            if (varData->varDetails[varNum-1].valDesc[j].value == atoi(argVal[argCount])) {
                                free(argVal[argCount]);
                                varDescLen = strnlen(varData->varDetails[varNum-1].valDesc[j].desc, EVT_MAX_DESC_TEXT);
                                argVal[argCount] = (char*)malloc(varDescLen+1);
                                if (argVal[argCount] != NULL) {
                                    memset(argVal[argCount], 0x00, varDescLen + 1); 
                                    memcpy(argVal[argCount],
                                           varData->varDetails[varNum-1].valDesc[j].desc,
                                           varDescLen);
                                }
                                enumDescFound = 1;
                                break;
                            }
                        }

                        if (!enumDescFound) {// enum description not found using default value
                            free(argVal[argCount]);
                            argVal[argCount] = (char*)malloc(UNKNOWN_STR_LEN+1);
                            if (argVal[argCount] != NULL) {
                                memset(argVal[argCount], 0x00, UNKNOWN_STR_LEN + 1); 
                                memcpy(argVal[argCount],
                                       UNKNOWN_STR,
                                       UNKNOWN_STR_LEN);
                            }
                        }
                    }
                    memcpy(buf_ptr, str_specifier, strnlen(str_specifier, sizeof("%s")));
                           buf_ptr += strnlen(str_specifier, sizeof("%s"));
                    if (argCount<9) ptr++;
                    else {
                        ptr++;
                        ptr++;
                    }
                }
                else {
                    *buf_ptr++ = *ptr;
                }
            }
            else {
                *buf_ptr++ = *ptr;
            }
            argCount++; 
        }
        else {
            *buf_ptr++ = *ptr;
        }
    }
    snprintf(decodedText,
             maxLen-1,
             buf,
             argVal[0],
             argVal[1],
             argVal[2],
             argVal[3],
             argVal[4],
             argVal[5],
             argVal[6],
             argVal[7],
             argVal[8],
             argVal[9],
             argVal[10],
             argVal[11],
             argVal[12],
             argVal[13],
             argVal[14],
             argVal[15]);

    //FIXME: Add later if supported.
    //oem_branding_token_swap(decodedText, oemString, sizeof(oemString));
    if (strnlen(oemString , sizeof(oemString) ) != 0)
        snprintf(decodedText,maxLen,"%s",oemString);

    // free
    for (i=0; i<16; i++) {
        if (argVal[i]!=NULL) {
            free(argVal[i]);
        }
    }
    return LOG_OK; 
}

#if 0
UINT8 generic_decode(int logType, EVT_LOG_ENTRY* evt, EVT_DECODED_LOG_ENTRY* record, UINT8 safe)
{
    UINT8 rc;
    char buf[512];
    EVT_GENERAL_DATA *genData=NULL;

    record->dwEvtNumber = evt->hdr.evtNumber;
    record->bSeverity = evt->hdr.severity;
    record->wClass = evt->hdr.evtClass;
    record->wCode = evt->hdr.evtCode;
    record->evtInitTime = evt->hdr.evtInitTime;
    record->evtUpdatedTime = evt->hdr.evtUpdatedTime;
    record->dwCount = evt->hdr.count;
     /*Bit 1 and bit 2  stores Action information which applies to SL*/
    if(logType==LOGS_SLDATA)
        record->UserAction = (evt->hdr.flags & SL_READ_BIT1_BIT2) >> 1;

    rc = get_general_data(logType, evt->hdr.evtClass, evt->hdr.evtCode, &genData);
    if (rc!=LOG_OK || genData == NULL) {
        record->evtCategory = 0;
    }
    else {
        record->evtCategory = genData->evtCategory; 
    }     
   decode_text(logType, evt, buf, sizeof(buf), TEXT_DESC, safe);
   SNPRINTF_S(record->description, sizeof(record->description), "%s", buf);
   decode_text(logType, evt, buf, sizeof(buf), TEXT_ACTION, safe);
   SNPRINTF_S(record->action, sizeof(record->action), "%s", buf); 

    return LOG_OK;
}
#endif // 0

