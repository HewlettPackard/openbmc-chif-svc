/*
// Copyright (c) 2021-2025 Hewlett Packard Enterprise Development, LP
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

#ifndef _I2C_TOPOLOGY_H
#define _I2C_TOPOLOGY_H

typedef enum
{
    I2C_TOPOLOGY_NONE_STATE,
    I2C_TOPOLOGY_INIT_STATE,
    I2C_TOPOLOGY_APML_PHASE_ONE_STATE,
    I2C_TOPOLOGY_APML_COMPLETE_STATE
} I2C_TOPOLOGY_STATE;

/*
The enum introduced for the flag which handles segment 
release procedure for error handling during data transfer
*/
typedef enum
{
    I2C_RELEASE_SEGMENT_ERROR_STATE_NONE,
    I2C_RELEASE_SEGMENT_ERROR_STATE_NEED_CBTE_COMPLETE,
    I2C_RELEASE_SEGMENT_ERROR_STATE_GENERAL_CASE
} I2C_RELEASE_SEGMENT_ERROR_STATE_FLAG;


/*
The following data will be used during I2C_MUX_SETUP_STATE
to setup the muxes so we can access the requested segment
*/
#define MAX_SEGMENT_DEPTH			10
#define MAX_MUX_I2C_WRITE			4

typedef struct _I2C_MUX_RESET_DATA
{
    /*This variable is required for taking back-up of write count before mux reset. It is required for snoop to work correctly*/
    unsigned char                           master_bte_write_count_backup;
    /*This variable store the segment ID in descending order in the array for which the mux reset is required*/
    unsigned char                           muxes_to_reset[MAX_SEGMENT_DEPTH];
    /* This will be set to the current mux in the above array to be reset. When it hits -1 we are done*/
    signed char                             mux_to_reset;    
    /*This flag is to indicate whether inline i2c mux rest is in progress or not*/
    unsigned int                            is_inline_i2c_mux_reset;
    /*The flag is needed to indicate recovery thread that mux reset is required after recovery or not */
    unsigned int                            mux_reset_after_recovery;
    /*This flag is required to indicate the segment release state that mux reset is completed and service thread needs to be alerted in error scenario*/
    unsigned int                            is_service_thread_alert_required;
    /*This flag is required to indicate the segment release state that what triggered the mux reset and based on that decide next interrupt state after mux reset completed*/
    I2C_RELEASE_SEGMENT_ERROR_STATE_FLAG    seg_release_error_state_flag;                                
} I2C_MUX_RESET_DATA;

typedef struct _I2C_MUX_SETUP_DATA
{
    /* This will be used to check if the requested segment matches the current one. If it does no mux setup is required */
    unsigned char                           current_segment_selected;
    /* This will be set up with the muxes to set to get to the requested segment */
    unsigned char                           muxes_to_set[MAX_SEGMENT_DEPTH];
    /* This will be set to the current mux in the above array to be set. When it hits -1 we are done*/
    signed char                             mux_to_set;
    /* This will be set with the data to be written to the mux */
    //char                                    mux_setup_write_buffer[MAX_MUX_I2C_WRITE];
    /* The length of the data buffer that will be written to the mux */
    //unsigned char                           mux_setup_write_length;
    /* The segment of the mux to be written to. Should only be needed for error checking */
    //unsigned char                           mux_segment;
    /* The address of the mux we are writing to */
    //unsigned char                           mux_address;
} I2C_MUX_SETUP_DATA;


/* Prototypes */
//unsigned int is_topology_state_apml_phase_one(void);
//void i2c_topology_clear_engines_current_mux_selected(void);
//unsigned int is_ddc_engine(unsigned int engine_num);
//void init_i2c_topology(void);
void update_i2c_topology(void);
//unsigned int i2c_engine_mux_reset(unsigned int engine_num);
//unsigned int set_engines_mux_setup_data_and_check_for_topology_errors(unsigned int engine_num);
//unsigned int i2c_acquire_segment(unsigned int engine_num);
//unsigned int i2c_release_segment(unsigned int engine_num);
//I2C_RETURN_CODES i2c_dump_apml_topology(void);
//I2C_RETURN_CODES i2c_create_apml_segment(I2C_CREATE_APML_SEGMENT_REQUEST_S * new_apml_segment);
//I2C_RETURN_CODES i2c_edit_mux_snoop(unsigned char enable_disable);
//I2C_RETURN_CODES i2c_change_segment_flag_s(I2C_SEGMENT_FLAG_CHANGE_REQUEST_S* seg_flag_change_info);
//I2C_RETURN_CODES i2c_check_aero_engine_revision(unsigned int engine_num);
//void i2c_release_segment_for_recovery(unsigned int engine_num);

extern unsigned char                   segment_to_engine[];
extern PlatDefI2CEngine                apml_root_engines[];
extern PlatDefI2CSegment               apml_segments[];

#endif