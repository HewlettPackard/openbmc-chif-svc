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
extern void dbPrintf(const char *format, ...);

// Decode functions
UINT8 generic_decode(int, EVT_LOG_ENTRY*, EVT_DECODED_LOG_ENTRY*, UINT8);

int find_index_iml_data_extract(int dataExtractID, UINT16 evtClass, UINT16 evtCode);
int find_index_sl_data_extract(int dataExtractID, UINT16 evtClass, UINT16 evtCode);
int find_index_iel_data_extract(int dataExtractID, UINT16 evtClass, UINT16 evtCode);

UINT8 get_variable_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_VAR_DATA** varData);
UINT8 get_action_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_TEXT_DATA** textData);
UINT8 get_description_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_TEXT_DATA** textData);
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

int find_index_iml_data_extract(int dataExtractID, UINT16 evtClass, UINT16 evtCode)
{
    int rc=-1; // initialize to invalid
    int index; 
    // TODO: Enhance the search logic based on rendering performance. May be use Binary search instead
    // of linear search
    
    switch (dataExtractID) {
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
        default:
            break;
    }
    return rc;
}

int find_index_sl_data_extract(int dataExtractID, UINT16 evtClass, UINT16 evtCode)
{
    int rc=-1; // initialize to invalid
    int index; 
    // TODO: Enhance the search logic based on rendering performance. May be use Binary search instead
    // of linear search
    
    switch (dataExtractID) {
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
        default:
            break;
    }
    return rc;
}

int find_index_iel_data_extract(int dataExtractID, UINT16 evtClass, UINT16 evtCode)
{
    int rc=-1; // initialize to invalid
    int index;
     
    // TODO: Enhance the search logic based on rendering performance. May be use Binary search instead
    // of linear search   
    switch (dataExtractID) {
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
                              char **argVal)
{
    char tempBuf[EVT_MAX_VARIABLE_SIZE] = {0};
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

    if (logType == LOGS_IMLDATA) {
        index = find_index_iml_data_extract(EVT_DESC_DATA_ID, evtClass, evtCode);
        if (index==-1) {
            return LOG_ERROR;
        }
        if (*textData == NULL) {
            *textData = &(List_IML_Desc_Data[index]); 
        }
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
        index = find_index_sl_data_extract(EVT_DESC_DATA_ID, evtClass, evtCode);
        if (index==-1) {
            return LOG_ERROR;
        }
        if (*textData == NULL) {
            *textData = &(List_SL_Desc_Data[index]); 
        }
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

    if (logType == LOGS_IMLDATA) {
        // get event action data
        index = find_index_iml_data_extract(EVT_ACT_DATA_ID, evtClass, evtCode);
        if (index==-1) {
            return LOG_ERROR;
        }
        if (*textData == NULL) {
            *textData = &(List_IML_Action_Data[index]); 
        }
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
        index = find_index_sl_data_extract(EVT_ACT_DATA_ID, evtClass, evtCode);
        if (index==-1) {
            return LOG_ERROR;
        }
        if (*textData == NULL) {
            *textData = &(List_SL_Action_Data[index]); 
        }
    }
    else {
        return LOG_ERROR;
    }
    return LOG_OK;
}

UINT8 get_variable_data(int logType, UINT16 evtClass, UINT16 evtCode, EVT_VAR_DATA** varData)
{
    int index;

    if (logType == LOGS_IMLDATA) {
        // get event variable data
        index = find_index_iml_data_extract(EVT_VAR_DATA_ID, evtClass, evtCode);
        if (index==-1) {
            return DECODE_DATA_NOT_FOUND;//LOG_ERROR;
        }
        if (*varData == NULL) {
            *varData = &(List_IML_Var_Data[index]); 
        }
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
        index = find_index_sl_data_extract(EVT_VAR_DATA_ID, evtClass, evtCode);
        if (index==-1) {
            return DECODE_DATA_NOT_FOUND;//LOG_ERROR;
        }
        if (*varData == NULL) {
            *varData = &(List_SL_Var_Data[index]); 
        }
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
    char buf[512] = {0};
    UINT8 varOffset, varWidth, varRep, varRepDigits;//, ahsFlag;
    char* argVal[16]={0}; // max 16 variable data
    char *str_specifier = (char *)"%s";
    UINT8 enumDescFound = 0;
    char message[EVT_MAX_SIZE] = {0};

    EVT_TEXT_DATA *textData=NULL;
    EVT_VAR_DATA *varData=NULL;

    memset(decodedText, 0x0, maxLen);

    if (textType == TEXT_DESC) {
        rc = get_description_data(logType, evt->hdr.evtClass, evt->hdr.evtCode, &textData);
        if (rc!=LOG_OK || textData==NULL) {
            snprintf(decodedText, maxLen, UNKNOWN_EVENT, evt->hdr.evtClass, evt->hdr.evtCode); 
            printf("Unknown Event Class [%d] or Event Code [%d] received \n", evt->hdr.evtClass, evt->hdr.evtCode);
            return LOG_ERROR;
        }
        else {
            snprintf(message, EVT_MAX_SIZE, "%s", textData->msg);
        }
    }
    else if (textType == TEXT_ACTION) {
        rc = get_action_data(logType, evt->hdr.evtClass, evt->hdr.evtCode, &textData);
        if (rc!=LOG_OK || textData==NULL) {
            printf("Event Class [%d] Code [%d] received with no Action text \n", evt->hdr.evtClass, evt->hdr.evtCode);
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
        dbPrintf("Event Class [%d] Code [%d] received with no variable data description\n", evt->hdr.evtClass, evt->hdr.evtCode);
        snprintf(decodedText, maxLen, "%s", message);
        return LOG_OK; // This seems to be a no variable data description
    }

    buf_ptr = buf;
    ptr = message;
    dbPrintf("Event: %s\n", message);

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
                    rc = extract_variable_value (evt,
                                                 varOffset,
                                                 varWidth,
                                                 varRep,
                                                 varRepDigits,
                                                 &(argVal[argCount]));
                    if (rc != LOG_OK) {
                        dbPrintf("Event Class [%d] Code [%d] extract_variable_value fail\n", evt->hdr.evtClass, evt->hdr.evtCode);
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

    // free
    for (i=0; i<16; i++) {
        if (argVal[i]!=NULL) {
            free(argVal[i]);
        }
    }
    return LOG_OK;
}

