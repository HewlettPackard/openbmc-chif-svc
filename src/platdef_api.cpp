/*
// Copyright (c) 2013-2025 Hewlett Packard Enterprise Development, LP
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
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/statvfs.h>

#include "platdef_api.hpp"
#include "i2c_topology.hpp"
#include "uefi.hpp"
#include "uefi_util.hpp"
#include "misc.hpp"

UINT8   platdef[PLATDEF_UPDATE_BUF_SZ + PLATDEF_BLOB_START];  // storage for platdef and meta)
PLATDEF_METADATA *meta;

void init_platdef(void) {

    meta = (PLATDEF_METADATA*)platdef;  //make meta data live at start of PlatDef mem

    if (uefi_util_platdef_store() != UEFI_RC_OK) {
        printf("Failed to initialize platdef data from ROM image!\n");
    }

    update_i2c_topology();

    platdef_table_dump();
}

void handle_legacy_platdef() {
    UINT8* blob_ptr = platdef + PLATDEF_BLOB_START;  // data storage location
    PlatDefTableData* td = (PlatDefTableData*) blob_ptr;

    dbPrintf("handle_legacy_platdef\n");
    if( td->Header.Type != RecordType_TableData ) { 
        printf("PLATDEF: A-RecordType_TableData not first entry, found Record_ID: %d instead\n",(int) *blob_ptr);
        return;
    }

    if( td->Flags & TableFlag_LegacyTable ) {

        if( !td->TotalSize || 
            (blob_ptr + td->TotalSize) > (platdef + PLATDEF_SIZE) ) {
            printf("PLATDEF: Legacy platdef indicated but TotalSize field is incorrect\n");
            return;
        }

        blob_ptr += td->TotalSize;
        td = (PlatDefTableData*) blob_ptr;

        if( td->Header.Type != RecordType_TableData ) {
            printf("PLATDEF: B-RecordType_TableData not first entry of secondary platdef, found Record_ID: %d instead\n",(int) *blob_ptr);
            return;
        }
        
        // Move new table to spot of old table
        blob_ptr = platdef + PLATDEF_BLOB_START;
        if( ((Address)td + td->TotalSize) < ((Address)platdef + PLATDEF_MEM_REGION_SIZE) ) {
            dbPrintf("PLATDEF: Legacy platdef flag handled, using new platdef\n");
            memmove( blob_ptr, td, td->TotalSize );
        } else {
            printf("PLATDEF: Legacy platdef indicated but TotalSize field of seconary platdef is incorrect\n");
        }

    }
}

// Platdef Metadata Functions
void populate_record_counts( void ) {
    UINT8* blob_ptr = platdef + PLATDEF_BLOB_START;  // data storage location
    PlatDefRecordHeader* hdr = (PlatDefRecordHeader*) blob_ptr;
    int status = PLATDEF_RC_OK;

    dbPrintf("populate_record_counts\n");

    if( hdr->Type != RecordType_TableData ) { 
        status = PLATDEF_RC_BADCONFIG;
        printf("PLATDEF_META: RecordType_TableData not first entry, found Record_ID: %d instead\n",(int) *blob_ptr);
    }

    while( hdr->Type != RecordType_EndOfTable && 
           (status == PLATDEF_RC_OK || status == PLATDEF_RC_BADCONFIG) ) {
//        printf("PLATDEF_LOAD: Record at %lx of type %x found\n", (UINT32)blob_ptr, (UINT16)*blob_ptr);
        status = PLATDEF_RC_OK;
        meta->record_count++;
        // If the platdef has too many records, we have to increase the max or fix the platdef
        //  or we might overrun the record arrays
        if (meta->record_count > PLATDEF_MAX_RECORDS) {
            printf("PLATDEF_META: record count is too many! %d\n", meta->record_count);
            break;
        }
        switch ( hdr->Type ) {
            case RecordType_Undefined:
                status = PLATDEF_RC_UNIMPLEMENTED;
                printf("PLATDEF_META: Reached RecordType_Undefined, no logic here yet\n");
                break;
            case RecordType_TableData:
                meta->table_data.count++;
                break;
            case RecordType_TempSensor:
                if( hdr->Flags & HeaderFlag_HideFromUI ) { 
                    meta->hidden_temp_sensor.count++;
                } else {
                    meta->visible_temp_sensor.count++;
                }
                break;
            case RecordType_FanPWM:
                meta->fan_pwm.count++;
                break;
            case RecordType_FanDevice:
                meta->fan_device.count++;
                break;
            case RecordType_PowerSupply:
                meta->power_supply.count++;
                break;
            case RecordType_RedundancyRule:
                meta->redundancy_rule.count++;
                break;
            case RecordType_Indicator:
                meta->indicator.count++;
                break;
            case RecordType_PowerMeter:
                meta->power_meter.count++;
                break;
            case RecordType_Processor:
                meta->processor.count++;
                break;
            case RecordType_Status:
                meta->status.count++;
                break;
            case RecordType_SatelliteBMC:
                meta->satellite_bmc.count++;
                break;
            case RecordType_FRU:
                meta->fru.count++;
                break;
            case RecordType_Association:
                meta->association.count++;
                break;
            case RecordType_I2CEngine:
                meta->i2c_engine.count++;
                break;
            case RecordType_DIMMMapping:
                meta->dimm_mapping.count++;
                break;
            case RecordType_AltConfig:
                meta->alt_config.count++;
                break;
            case RecordType_Validation:
                meta->validation.count++;
                break;
            case RecordType_SensorGroup:
                meta->sensor_group.count++;
                break;
            case RecordType_LookupTable:
                meta->lookup_table.count++;
                break;
            case RecordType_Throttle:
                meta->throttle.count++;
                break;
            case RecordType_SystemDevice:
                meta->system_device.count++;
                break;
            case RecordType_PeciSegment:
                meta->peci_segment.count++;
                break;
            case RecordType_DeltaPatch:
            case RecordType_Replacement:
                meta->patch_and_replacement.count++;
                break;
            default:
                // reached a record not in spec
                status = PLATDEF_RC_BADCONFIG;
                printf("PLATDEF_META: Reached Unknown Record with Type: %d\n",hdr->Type);
                break;
        } // end of switch(record_type)

        // increment blob ptr to next record based on record header size field
        if ( status == PLATDEF_RC_OK || status == PLATDEF_RC_BADCONFIG ) {
            blob_ptr += hdr->Size * 16;
            hdr = (PlatDefRecordHeader*) blob_ptr;
        }
    } // end of counting records of each type

    if( hdr->Type == RecordType_EndOfTable ) {
        dbPrintf("PLATDEF_META: Reached RecordType_EndOfTable\n");
        meta->end_of_table = (UINT8*)((Address)blob_ptr - (Address)platdef);
    } else {
        printf("PLATDEF_META: Parsing ended before EndOfTable record found\n");
    }
}

void assign_first_index( UINT16* index, RECORD_TYPE_DATA* rec_data ) {
    rec_data->first_index = *index;
    *index += rec_data->count;
}

void clear_record_counts() {
    dbPrintf("clear_record_counts\n");
    meta->table_data.count = 0;
    meta->visible_temp_sensor.count = 0;
    meta->hidden_temp_sensor.count = 0;
    meta->fan_pwm.count = 0;
    meta->fan_device.count = 0;
    meta->power_supply.count = 0;
    meta->redundancy_rule.count = 0;
    meta->indicator.count = 0;
    meta->power_meter.count = 0;
    meta->processor.count = 0;
    meta->status.count = 0;
    meta->satellite_bmc.count = 0;
    meta->fru.count = 0;
    meta->association.count = 0;
    meta->i2c_engine.count = 0;
    meta->dimm_mapping.count = 0;
    meta->alt_config.count = 0;
    meta->validation.count = 0;
    meta->sensor_group.count = 0;
    meta->lookup_table.count = 0;
    meta->throttle.count = 0;
    meta->system_device.count = 0;
    meta->peci_segment.count = 0;
    meta->patch_and_replacement.count = 0;
}

void clear_record_first_index() {
    dbPrintf("clear_record_first_index\n");
    meta->table_data.first_index = 0;
    meta->visible_temp_sensor.first_index = 0;
    meta->hidden_temp_sensor.first_index = 0;
    meta->fan_pwm.first_index = 0;
    meta->fan_device.first_index = 0;
    meta->power_supply.first_index = 0;
    meta->redundancy_rule.first_index = 0;
    meta->indicator.first_index = 0;
    meta->power_meter.first_index = 0;
    meta->processor.first_index = 0;
    meta->status.first_index = 0;
    meta->satellite_bmc.first_index = 0;
    meta->fru.first_index = 0;
    meta->association.first_index = 0;
    meta->i2c_engine.first_index = 0;
    meta->dimm_mapping.first_index = 0;
    meta->alt_config.first_index = 0;
    meta->validation.first_index = 0;
    meta->sensor_group.first_index = 0;
    meta->lookup_table.first_index = 0;
    meta->throttle.first_index = 0;
    meta->system_device.first_index = 0;
    meta->peci_segment.first_index = 0;
    meta->patch_and_replacement.first_index = 0;
}

void populate_record( RECORD_TYPE_DATA* rec_data, UINT8* blob_ptr ) {
    UINT16 index = rec_data->first_index + rec_data->count;
    PlatDefRecordHeader* hdr = (PlatDefRecordHeader*)((Address)blob_ptr - (Address)platdef);
    
    rec_data->count++;
    meta->records[index] = (Address)hdr;
}

void populate_record_pointers( void ) {
    UINT8* blob_ptr = platdef + PLATDEF_BLOB_START;  // data storage location
    PlatDefRecordHeader* hdr = (PlatDefRecordHeader*) blob_ptr;
    int status = PLATDEF_RC_OK;
    dbPrintf("populate_record_pointers\n");
    while( hdr->Type != RecordType_EndOfTable && 
           (status == PLATDEF_RC_OK || status == PLATDEF_RC_BADCONFIG) ) {
//        printf("PLATDEF: populate_record_pointers: Record at %lx of type %d found\n", (UINT32)blob_ptr, (UINT16)*blob_ptr);
        status = PLATDEF_RC_OK;
        switch ( hdr->Type ) {
            case RecordType_Undefined:
                status = PLATDEF_RC_UNIMPLEMENTED;
                printf("PLATDEF_META: Reached RecordType_Undefined, no logic here yet\n");
                break;
            case RecordType_TableData:
                populate_record( &meta->table_data, blob_ptr );
                break;
            case RecordType_TempSensor:
                if( hdr->Flags & HeaderFlag_HideFromUI ) { 
                    populate_record( &meta->hidden_temp_sensor, blob_ptr );
                } else {
                    populate_record( &meta->visible_temp_sensor, blob_ptr );
                }
                break;
            case RecordType_FanPWM:
                populate_record( &meta->fan_pwm, blob_ptr );
                break;
            case RecordType_FanDevice:
                populate_record( &meta->fan_device, blob_ptr );
                break;
            case RecordType_PowerSupply:
                populate_record( &meta->power_supply, blob_ptr );
                break;
            case RecordType_RedundancyRule:
                populate_record( &meta->redundancy_rule, blob_ptr );
                break;
            case RecordType_Indicator:
                populate_record( &meta->indicator, blob_ptr );
                break;
            case RecordType_PowerMeter:
                populate_record( &meta->power_meter, blob_ptr );
                break;
            case RecordType_Processor:
                populate_record( &meta->processor, blob_ptr );
                break;
            case RecordType_Status:
                populate_record( &meta->status, blob_ptr );
                break;
            case RecordType_SatelliteBMC:
                populate_record( &meta->satellite_bmc, blob_ptr );
                break;
            case RecordType_FRU:
                populate_record( &meta->fru, blob_ptr );
                break;
            case RecordType_Association:
                populate_record( &meta->association, blob_ptr );
                break;
            case RecordType_I2CEngine:
                populate_record( &meta->i2c_engine, blob_ptr );
                break;
            case RecordType_DIMMMapping:
                populate_record( &meta->dimm_mapping, blob_ptr );
                break;
            case RecordType_AltConfig:
                populate_record( &meta->alt_config, blob_ptr );
                break;
            case RecordType_Validation:
                populate_record( &meta->validation, blob_ptr );
                break;
            case RecordType_SensorGroup:
                populate_record( &meta->sensor_group, blob_ptr );
                break;
            case RecordType_LookupTable:
                populate_record( &meta->lookup_table, blob_ptr );
                break;
            case RecordType_Throttle:
                populate_record( &meta->throttle, blob_ptr );
                break;
            case RecordType_SystemDevice:
                populate_record( &meta->system_device, blob_ptr );
                break;
            case RecordType_PeciSegment:
                populate_record( &meta->peci_segment, blob_ptr );
                break;
            case RecordType_DeltaPatch:
            case RecordType_Replacement:
                populate_record( &meta->patch_and_replacement, blob_ptr );
                break;
            default:
                // reached a record not in spec
                status = PLATDEF_RC_BADCONFIG;
                printf("PLATDEF_META: Reached Unknown Record with Type: %d\n",hdr->Type);
                break;
        } // end of switch(record_type)

        // increment blob ptr to next record based on record header size field
        if ( status == PLATDEF_RC_OK || status == PLATDEF_RC_BADCONFIG ) {
            blob_ptr += hdr->Size * 16;
            hdr = (PlatDefRecordHeader*) blob_ptr;
        }
    } // end of counting records of each type

    if( hdr->Type == RecordType_EndOfTable ) {
        dbPrintf("PLATDEF_META: Reached RecordType_EndOfTable\n");
        meta->end_of_table = (UINT8*)((Address)blob_ptr - (Address)platdef);
        dbPrintf("PLATDEF_META: End of table is 0x%lx\n", (uint64_t)meta->end_of_table);
    } else {
        printf("PLATDEF_META: Parsing ended before EndOfTable record found\n");
    }
}

void assign_all_first_indexes() {
   UINT16 index = 0;

   dbPrintf("assign_all_first_indexes\n");

   // Assign first_index values
   assign_first_index( &index, &meta->table_data );
   assign_first_index( &index, &meta->visible_temp_sensor );
   assign_first_index( &index, &meta->hidden_temp_sensor );
   assign_first_index( &index, &meta->fan_pwm );
   assign_first_index( &index, &meta->fan_device );
   assign_first_index( &index, &meta->power_supply );
   assign_first_index( &index, &meta->redundancy_rule );
   assign_first_index( &index, &meta->indicator );
   assign_first_index( &index, &meta->power_meter );
   assign_first_index( &index, &meta->processor );
   assign_first_index( &index, &meta->status );
   assign_first_index( &index, &meta->satellite_bmc );
   assign_first_index( &index, &meta->fru );
   assign_first_index( &index, &meta->association );
   assign_first_index( &index, &meta->i2c_engine );
   assign_first_index( &index, &meta->dimm_mapping );
   assign_first_index( &index, &meta->alt_config );
   assign_first_index( &index, &meta->validation );
   assign_first_index( &index, &meta->sensor_group );
   assign_first_index( &index, &meta->lookup_table );
   assign_first_index( &index, &meta->throttle );
   assign_first_index( &index, &meta->system_device );
   assign_first_index( &index, &meta->peci_segment );
   assign_first_index( &index, &meta->patch_and_replacement );
}


int platdef_meta_load() {
    int status = PLATDEF_RC_OK;   // handles unknown record types
    UINT16 num_temp_sensors, num_devices;

    platdef_clear_meta();

    // check for legacy flag and handle it
    handle_legacy_platdef();

    // Loop through all records and count each type
    populate_record_counts();

    if( meta->record_count > PLATDEF_MAX_RECORDS ) {
        printf("PLATDEF: Platdef records exceed max supported");
        status = PLATDEF_RC_ERROR;
        return status;
    }

    assign_all_first_indexes();

    // Clear counts
    clear_record_counts();

    // Loop through records again and fill out records[] array
    populate_record_pointers();

    // reduce the fan_pwm count by 1 (last pointer is a global pwm value)
    if(meta->fan_pwm.count > 0) {
        meta->fan_pwm.count--;
    }

    // Check that we're not going to overrun our parallel structures
    if( meta->fan_pwm.count > PLATDEF_MAX_FAN_PWM ) {
        //Truncate number of fan pwms to avoid structure overruns
        printf("PLATDEF: FanPWM records exceed max supported");
        meta->fan_pwm.count = PLATDEF_MAX_FAN_PWM;
    }

    num_temp_sensors = meta->visible_temp_sensor.count + meta->hidden_temp_sensor.count;
    if( num_temp_sensors > PLATDEF_MAX_TEMP_SENSOR ) {
        //Truncate number of temp sensors to avoid structure overruns
        printf("PLATDEF: TempSensor records exceed max supported");
        meta->visible_temp_sensor.count -= (num_temp_sensors - PLATDEF_MAX_TEMP_SENSOR);
    }

    if( meta->power_supply.count > PLATDEF_MAX_POWER_SUPPLY ) {
        //Truncate number of fan pwms to avoid structure overruns
        printf("PLATDEF: PowerSupply records exceed max supported");
        meta->power_supply.count = PLATDEF_MAX_POWER_SUPPLY;
    }

    if( meta->redundancy_rule.count > PLATDEF_MAX_REDUNDANCY_RULE ) {
        //Truncate number of redundancy rules to avoid structure overruns
        printf("PLATDEF: RedundancyRule records exceed max supported");
        meta->redundancy_rule.count = PLATDEF_MAX_REDUNDANCY_RULE;
    }

    num_devices = meta->indicator.count + num_temp_sensors + meta->fan_device.count + 
        meta->power_supply.count + meta->power_meter.count + meta->status.count;
    if( num_devices > PLATDEF_MAX_HEALTH_DEVICES ) {
        printf("PLATDEF: Device records exceed max supported");
        status = PLATDEF_RC_ERROR;
    }

    return status;
}

void platdef_clear_meta() {
    dbPrintf("platdef_clear_meta: addr:%lx size:%lx\n", (uint64_t)meta, sizeof(PLATDEF_METADATA));
    int bc = meta->build_count;
    UINT8* dyn_start = meta->dynamic_record_start;
    memset( meta, 0x00, sizeof(PLATDEF_METADATA) );
    meta->build_count = bc;
    meta->hood_index = PLATDEF_INDEX_UNINIT;
    meta->battery_index = PLATDEF_INDEX_UNINIT;
    meta->dynamic_record_start = dyn_start;
}


PLATDEF_METADATA* platdef_meta_get() {
    return meta;
}

void increment_platdef_build_count( void ) {
    if (meta) {
        meta->build_count++;
    }
}

int platdef_build_count( void ) {
    if (meta) {
        return meta->build_count;
    } else {
        return 0;
    }
}

PlatDefRecordHeader* get_record( RECORD_TYPE_DATA* rec_data, UINT16 index ) {
    if( index < rec_data->count ) {
        return (PlatDefRecordHeader*)(meta->records[rec_data->first_index + index] + (Address)platdef);
    } else {
        return NULL;
    }
}


// Platdef Record Functions
PlatDefTableData* table_data() {
    return (PlatDefTableData*)get_record(&meta->table_data,0);
}

UINT16 temp_sensor_count( void ) {
    return meta->visible_temp_sensor.count + meta->hidden_temp_sensor.count;
}

/**************************************************************************************** 
   PlatDefPowerSupply Access Functions
      Definitions: 
        + n - index of power supply record to return
****************************************************************************************/
PlatDefPowerSupply* power_supply( UINT16 n ) {
    return (PlatDefPowerSupply*)get_record(&meta->power_supply,n);
}

UINT16 power_supply_count( void ) {
    if(meta==NULL)
    {
        dbPrintf("Meta is unititalized func\n");
        return PLATDEF_RC_NO_SUCH_RECORD;
    }

    return meta->power_supply.count;
}

/**************************************************************************************** 
   PlatDefProcessor Access Functions
****************************************************************************************/
PlatDefProcessor* processor( UINT16 n ) {
   return (PlatDefProcessor*)get_record(&meta->processor,n);
}

UINT16 processor_count( void ) {
   return meta->processor.count;
}

/**************************************************************************************** 
   System Device Record Access Functions
****************************************************************************************/
PlatDefSystemDevice* system_device( UINT8 n ) {
    return (PlatDefSystemDevice*)get_record(&meta->system_device,n);
}

/**************************************************************************************** 
   I2C Engine/Segment Record Access Functions
****************************************************************************************/
PlatDefI2CEngine* i2c_engine_by_index( UINT8 n ) {
    return (PlatDefI2CEngine*)get_record(&meta->i2c_engine,n);
}

/******************************************************** 
  UTILITY FUNCS - find data by various methods
*********************************************************/

//search via APML pointer for specified record_id - return record size and contents
platdef_rc platdef_get_record_by_id(UINT32 rec_id, UINT32 *size, UINT8* record_data, UINT8** record_ptr)
{
    UINT8* blob_ptr = platdef + PLATDEF_BLOB_START;  // location of platdef data
    PlatDefRecordHeader* hdr = (PlatDefRecordHeader*) blob_ptr;
    platdef_rc status = PLATDEF_RC_OK;   // handles unknown record types

    if ((record_data!=NULL) && (size!=NULL)) {  // blob init or bad param?
        // until EndOfTable reached
        while ( hdr->Type != RecordType_EndOfTable && status == PLATDEF_RC_OK) {
            //if found it 
            if (hdr->RecordID == rec_id) {
                *size = hdr->Size *16;
                if((record_data[0] == 0x0b) && (*size>2048))
                   *size = 2048;
                memcpy(record_data, blob_ptr, *size);
                *record_ptr = blob_ptr;
                break;
            }

            // if this record has no size we need to avoid an infinite loop
            if( !hdr->Size ) {
                printf("PLATDEF: A-Record %u has 0 size, cannot continue looking for record %u\n", 
                    hdr->RecordID, rec_id);
                status = PLATDEF_RC_BADCONFIG;
            } else {
                //keep looking - increment blob ptr to next record based on record header size field
                blob_ptr += hdr->Size * 16;
                hdr = (PlatDefRecordHeader*) blob_ptr;
            }
        } // end of while(record != EndOfTable)  

        //if never found record_id but hit end of table record
        if ( hdr->Type == RecordType_EndOfTable ) {
            //handle case where where recordID is 0xFFFF (i.e. EndOfTable)
            if (hdr->RecordID == rec_id) {
                *size = hdr->Size * 16;
                if((record_data[0] == 0x0b) && (*size>2048))
                   *size = 2048;
                memcpy(record_data, blob_ptr, *size);
                *record_ptr = blob_ptr;
                status = PLATDEF_RC_OK;
            }
            else {
                status = PLATDEF_RC_NO_SUCH_RECORD;
            }
        }
    }  // end of if(blob init)
    else {
        //not allowed to search if APML not init'ed
        status = PLATDEF_RC_MEM_ACCESS_FAIL;
        printf("platdef mem region is not initalized\n");
    }

    return status;
}

platdef_rc platdef_get_record_by_type(UINT32 rec_type, UINT32 *size, UINT8* record_data, UINT8** record_ptr, UINT16*recordID)
{
    PlatDefRecordHeader* hdr = (PlatDefRecordHeader*) *record_ptr;
    platdef_rc status = PLATDEF_RC_OK;   // handles unknown record types
    UINT8* blob_ptr = *record_ptr;

    if ((record_data!=NULL) && (size!=NULL)) {  // blob init or bad param?
        // until EndOfTable reached
        while ( hdr->Type != RecordType_EndOfTable && status == PLATDEF_RC_OK) {
            //if found it 
            if (hdr->Type == rec_type) {
                *size = hdr->Size *16;
                *recordID = hdr->RecordID;
                memcpy(record_data, blob_ptr, *size);
                blob_ptr += hdr->Size * 16;
                *record_ptr = blob_ptr;
                break;
            }

            // if this record has no size we need to avoid an infinite loop
            if( !hdr->Size ) {
                printf("PLATDEF: B-Record %u has 0 size, cannot continue looking for record %u\n", hdr->Type, rec_type);
                status = PLATDEF_RC_BADCONFIG;
            } else {
                //keep looking - increment blob ptr to next record based on record header size field
                blob_ptr += hdr->Size * 16;
                hdr = (PlatDefRecordHeader*) blob_ptr;
            }
        } // end of while(record != EndOfTable)  

        //if never found record_id but hit end of table record
        if ( hdr->Type == RecordType_EndOfTable ) {
            //handle case where where recordID is 0xFFFF (i.e. EndOfTable)
            if (hdr->Type == rec_type) {
                *size = hdr->Size * 16;
                memcpy(record_data, blob_ptr, *size);
                blob_ptr += hdr->Size * 16;
                *record_ptr = blob_ptr;
                status = PLATDEF_RC_OK;
            }
            else {
                status = PLATDEF_RC_NO_SUCH_RECORD;
            }
        }
    }  // end of if(blob init)
    else {
        //not allowed to search if APML not init'ed
        status = PLATDEF_RC_MEM_ACCESS_FAIL;
        printf("PLATDEF: get record by type bad parameters!\n");
    }

    return status;
}

platdef_smif_rc platdef_Download_specific_data (UINT32 *data_size, UINT32 timestamp, UINT16 *req_count, PlatDefDataRequest* req_data, UINT8 *resp, UINT16 resp_buf_size, UINT32 *token)
{
    int i;
    UINT8* blob_ptr;  // current location in blob
    UINT32 rec_size = 0;  //throwaway var needed for API call
    UINT8 rec_data[2048];  // buffer to get one record - how big can one record really be (spec sez 4096) ?
    UINT16 curr_buf_offset = 0; //offset into response buffer
    platdef_rc status = PLATDEF_RC_OK;   // handles unknown record types
    UINT16 count_in;
    UINT16 count_out;

//    PLATDEF_CHIF_MESSAGE_REQ  req;
//    PLATDEF_CHIF_MESSAGE_RESP rsp;

    {
        if ( (!data_size)||(*data_size==0)||(*data_size>PLATDEF_CHUNK_SIZE) || 
            (!req_count)||(*req_count == 0) ||
            (!req_data) || 
            (!resp))
        { 
            printf("PLATDEF: bad or too large parameter\n");
            return PLATDEF_SMIF_RC_BADREQUEST;
        }
        if (*req_count > MAX_PLATDEF_SPECIFIC_DATA_REQ)
        {
            printf("PLATDEF: bad parameter - request count limited to %d - but got %d\n", 
                  MAX_PLATDEF_SPECIFIC_DATA_REQ, *req_count );
            return PLATDEF_SMIF_RC_BADREQUEST;
        }

        curr_buf_offset = 0;
        count_in = *req_count;
        count_out = 0;
        *data_size = 0;
        memset(resp,0,resp_buf_size);

        for (i=0;i<count_in;i++) {
            if (curr_buf_offset + req_data[i].length > resp_buf_size) {
                printf("PLATDEF: response buffer overflow\n");
                return PLATDEF_SMIF_RC_BADREQUEST;
            }
            memset(rec_data,0,2048);
            rec_data[0] = 0x0b; // 2^11 = 2048, buffer size info for platdef_get_record_by_id()
            status = platdef_get_record_by_id(req_data[i].RecordID, &rec_size, rec_data, &blob_ptr);
            if (status) {
                printf("PLATDEF: invalid request - record ID %d not found - \n", req_data[i].RecordID);
                continue; //skip bad records and keep going
            }

            //copy record ID (UINT16) into response buf
            *(UINT16 *)((Address)resp + (Address)curr_buf_offset) = req_data[i].RecordID;
            curr_buf_offset += RECORD_ID_LENGTH;
            *data_size += RECORD_ID_LENGTH;

            //copy offset into response buf
            *(UINT16 *)((Address)resp + (Address)curr_buf_offset) = req_data[i].Offset;
            curr_buf_offset += RECORD_ID_LENGTH; // Offset is also 2 bytes
            *data_size += RECORD_ID_LENGTH;

            //copy specific record data requested into response buf
            memcpy((void*)((Address)resp + (Address)curr_buf_offset), (void*)((Address)rec_data + (Address)req_data[i].Offset), req_data[i].length);
            *data_size += req_data[i].length; //save size used so far for response
            curr_buf_offset +=  req_data[i].length;
            count_out++; //count number of valid records found for response
        }
        *req_count = count_out; //how many actually found
        //-----------------------------------------------
        // TODO: For now we using the locally stored timestamp now.
        // IN future we may get the timestamp stored in NVM.
        //-----------------------------------------------
        *token = timestamp;

        return PLATDEF_SMIF_RC_OK;
    }
}

platdef_smif_rc platdef_Download_specific_data_per_type(UINT32 timestamp, UINT32 recType, PlatDefDataRequest *req_data, UINT32 req_count,
                                            void* resp, UINT16 *resp_count, UINT16 * data_size, UINT32 *token)
{
    UINT32 i;
    UINT8* blob_ptr;  // current location in blob
    UINT32 rec_size = 0;  //throwaway var needed for API call
    UINT8 rec_data[2048];  // buffer to get one record - how big can one record really be (spec sez 4096) ?
    UINT32 curr_buf_offset = 0; //offset into response buffer
    platdef_rc status = PLATDEF_RC_OK;   // handles unknown record types
    UINT32 count_out;
    UINT16 recordID = 0;
    UINT16 resp_size = 0;
    UINT32 totalReqOffsetLength = 0;

//    PLATDEF_CHIF_MESSAGE_REQ  req;
//    PLATDEF_CHIF_MESSAGE_RESP rsp;

    {
        if ((!req_data) || (!resp))
        {
            printf("PLATDEF: bad or too large parameter\n");
            return PLATDEF_SMIF_RC_BADREQUEST;
        }

        if (req_count > MAX_PLATDEF_SPECIFIC_DATA_REQ)
        {
            printf("PLATDEF: bad parameter - request count limited to %d - but got %d\n",
                  MAX_PLATDEF_SPECIFIC_DATA_REQ, req_count );
            return PLATDEF_SMIF_RC_BADREQUEST;
        }
        else if (!req_count)
        {
            // Special Case for BIOS.  Use Max to check in the loop below.
            req_count = MAX_PLATDEF_SPECIFIC_DATA_REQ; // Output all of PlatDef recordid when req_count is 0
            req_data[0].Offset = 0;
            totalReqOffsetLength = RECORD_ID_LENGTH*2; // Record ID and Offset=0 are returned for each record
        }
        else // req_count is non-zero.  There are offsets to process.
        {
            // Sum the length of all offset readings for each record
            // 2 bytes for Record Id, 2 bytes for Offset, plus length
            for (i=0; i<req_count; i++)
            {
                totalReqOffsetLength += (2*RECORD_ID_LENGTH) + req_data[i].length;
            }
        }

        curr_buf_offset = 0;
        count_out = 0;
        blob_ptr = platdef + PLATDEF_BLOB_START;  // location of platdef data

        while (curr_buf_offset + totalReqOffsetLength < PLATDEF_CHUNK_SIZE) {
            memset(rec_data, 0, 2048);
            // Get first record of recType (into rec_data), and advance blob_ptr to following record
            status = platdef_get_record_by_type(recType, &rec_size, rec_data, &blob_ptr, &recordID);
            // Exit while-loop when no such records remain.
            if (status == PLATDEF_RC_NO_SUCH_RECORD) {
                break;
            } else if (status) {
                printf("PLATDEF: invalid request - record type %d not found - \n", recType);
                return PLATDEF_SMIF_RC_NOTFOUND;
            }

            // Each offset response is formatted: RecordId (2 bytes), Offset (2 bytes), data (length bytes)
            for (i=0; i<req_count; i++)
            {
                count_out++; //count number of data records returned

                // Copy recordId
                *(UINT16 *)((UINT8*)resp + curr_buf_offset) = recordID;
                curr_buf_offset += RECORD_ID_LENGTH;
                resp_size += RECORD_ID_LENGTH;

                // Copy offset
                *(UINT16 *)((UINT8*)resp + curr_buf_offset) = req_data[i].Offset;
                curr_buf_offset += RECORD_ID_LENGTH; // Offset is also size of 2
                resp_size += RECORD_ID_LENGTH;

                // Special case for BIOS. SVN 47808. Just Record ID and 00. No other data.
                if (req_count == MAX_PLATDEF_SPECIFIC_DATA_REQ)
                {
                    break;  // end for-loop
                }

                // Copy record data
                memcpy ((void*)((UINT8*)resp + curr_buf_offset), (void*)(rec_data + req_data[i].Offset), req_data[i].length);
                resp_size += req_data[i].length;
                curr_buf_offset += req_data[i].length;
            }

            // This should never be true, since length is checked at while entry
            if (curr_buf_offset > PLATDEF_CHUNK_SIZE) {
                printf("PLATDEF: response buffer overflow\n");
                return PLATDEF_SMIF_RC_BADREQUEST;
            }
        } // while curr_buf_offset < PLATDEF_CHUNK_SIZE
        *resp_count = count_out; //how many data chunks actually found
        *data_size = resp_size;
        //-----------------------------------------------
        // TODO: For now we using the locally stored timestamp now.
        // IN future we may get the timestamp stored in NVM.
        //-----------------------------------------------
        *token = timestamp;

        return PLATDEF_SMIF_RC_OK;
    }
}
    
/******************************************************** 
  DATA PRINTING FUNCS - debug dumps
*********************************************************/

void platdef_table_dump(void) {
    PlatDefTableData* table = table_data();
    PLATDEF_METADATA* meta = platdef_meta_get();
    char ver[32] = "TBD";

    if (table != NULL && meta) {
        dbPrintf("============= PlatDef Metadata =============\n");
        dbPrintf("Build Count:\t\t\t\t%d\n", platdef_build_count());
        dbPrintf("RECORDTYPE\t\tITEM\t\tVALUE\n");
        dbPrintf("TableData\t\tDescription\t%s\n", table->Description);
        dbPrintf("TableData\t\tVersion\t\t%s\n", ver);
        dbPrintf("TableData\t\tBuildTimestamp\t%d\n", table->BuildTimestamp);
        dbPrintf("TableData\t\tTotalSize\t%d\n", table->TotalSize);
        dbPrintf("TableData\t\tRecordCount\t%d\n", table->RecordCount);
        dbPrintf("Metadata\t\tBuild Count\t%d\n", meta->build_count);
        dbPrintf("TempSensor\t\tCount\t\t%d\n", temp_sensor_count());
        dbPrintf("\t\t\tVisible\t\t%d\n", meta->visible_temp_sensor.count);
        dbPrintf("\t\t\tHidden\t\t%d\n", meta->hidden_temp_sensor.count);
        dbPrintf("FanPWM\t\t\tCount\t\t%d\n", meta->fan_pwm.count);
        dbPrintf("FanDevice\t\tCount\t\t%d\n", meta->fan_device.count);
        dbPrintf("PowerSupply\t\tCount\t\t%d\n", meta->power_supply.count);
        dbPrintf("RedundancyRule\t\tCount\t\t%d\n", meta->redundancy_rule.count);
        dbPrintf("Indicator\t\tCount\t\t%d\n", meta->indicator.count);
        dbPrintf("PowerMeter\t\tCount\t\t%d\n", meta->power_meter.count);
        dbPrintf("Processor\t\tCount\t\t%d\n", meta->processor.count);
        dbPrintf("Status\t\t\tCount\t\t%d\n", meta->status.count);
        dbPrintf("SatelliteBMC\t\tCount\t\t%d\n", meta->satellite_bmc.count);
        dbPrintf("FRU\t\t\tCount\t\t%d\n", meta->fru.count);
        dbPrintf("Association\t\tCount\t\t%d\n", meta->association.count);
        dbPrintf("I2CEngine\t\tCount\t\t%d\n", meta->i2c_engine.count);
        dbPrintf("DIMMMapping\t\tCount\t\t%d\n", meta->dimm_mapping.count);
        dbPrintf("AltConfig\t\tCount\t\t%d\n", meta->alt_config.count);
        dbPrintf("Validation\t\tCount\t\t%d\n", meta->validation.count);
        dbPrintf("SensorGroup\t\tCount\t\t%d\n", meta->sensor_group.count);
        dbPrintf("LookupTable\t\tCount\t\t%d\n", meta->lookup_table.count);
        dbPrintf("Throttle\t\tCount\t\t%d\n", meta->throttle.count);
        dbPrintf("SystemDevice\t\tCount\t\t%d\n", meta->system_device.count);
        dbPrintf("PeciSegment\t\tCount\t\t%d\n", meta->peci_segment.count);
        dbPrintf("Patch/Replace\t\tCount\t\t%d\n", meta->patch_and_replacement.count);
        //ddbPrintf("NOTE: TableData.RecordCount = SUM(All_Counts) + TableData + EndofTable + FanPWM_Global\n");
        dbPrintf("=============================================\n");
    }
    else {
        printf("PLATDEF: PlatDef Metadata Empty\n");
    }
}

/**
 * platdef_get_APML_data()
 * Fetch APML records data related to power supply, processor 
 * and system devices, update it to the recieved structure.
 * 
 */
int platdef_get_APML_data(uint32_t *count, Entity *entity_list, uint32_t max_num_entities)
{
    int i;
    PlatDefSystemDevice *p;
    int ps_count = power_supply_count();
    int proc_count = processor_count();

    if(!entity_list || !count) {
        return 1;
    }

    *count = 0;

    for(i = 0; i < ps_count; i++) {
        PlatDefPowerSupply* ps = power_supply(i);
    
        if(*count >= max_num_entities || ps == NULL)
            return 1;

        entity_list[*count].entity_type = POWER_SUPPLY;
        entity_list[*count].entity_data.power_supply.bus = ps->Interrogate.NoisyI2C.Bus;
        entity_list[*count].entity_data.power_supply.address = ps->Interrogate.NoisyI2C.Address;
        entity_list[*count].entity_data.power_supply.domain = ps->DomainType;
        *count += 1;
    }

    for(i = 0; i < proc_count; i++) {
        PlatDefProcessor *proc = processor(i);
        if(*count >= max_num_entities || proc == NULL)
            return 1;

        entity_list[*count].entity_type = PIROM;
        entity_list[*count].entity_data.pirom.segment = proc->PiROMSegment;
        entity_list[*count].entity_data.pirom.address = proc->PiROMAddress;
        *count += 1;
    }

    i = 0;
    while ((p = system_device(i)) != NULL) {
        enum SystemDeviceType t = (enum SystemDeviceType) p->Data.DeviceType;

        if(*count >= max_num_entities)
            return 1;

        switch (t) {
            case SystemDeviceType_Slot:
                entity_list[*count].entity_type= SLOT;
                entity_list[*count].entity_data.slot.segment = p->Data.SlotDevice.Segment;
                entity_list[*count].entity_data.slot.slot_num = p->Data.SlotDevice.Slot;
                *count += 1;
                break;

            case SystemDeviceType_MegaCell:
                entity_list[*count].entity_type = MEGACELL;
                entity_list[*count].entity_data.mega_cell.segment = p->Data.MegaCellDevice.Segment;
                entity_list[*count].entity_data.mega_cell.address = p->Data.MegaCellDevice.Address;
                entity_list[*count].entity_data.mega_cell.fru_address = p->Data.MegaCellDevice.FRUAddress;
                *count += 1;
                break;

            default:
                break;
        }
        i++;
    }

    return 0;
}
