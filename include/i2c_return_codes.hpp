/*
// Copyright (c) 2021, 2025 Hewlett Packard Enterprise Development, LP
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

#ifndef I2C_RETURN_CODES_H
#define I2C_RETURN_CODES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    I2C_SUCCESS=100,
    I2C_GENERAL_ERROR,
    I2C_CONNECTION_ERROR,
    I2C_NEED_INIT_ERROR,
    I2C_BAD_ARGUMENT,
    I2C_BUS_TIMEOUT,
    I2C_TRANSACTION_TIMEOUT,
    I2C_SEGMENT_REQUEST_ON_WRONG_ENGINE,
    I2C_SEGMENT_DOES_NOT_EXIST,
    I2C_BAD_TOPOLOGY,
    I2C_MUX_SET_FAILURE,
    I2C_MUX_ADDRESS_NACK,
    I2C_NEED_TOPOLOGY_UPDATE,
    I2C_ENGINE_DISABLED_ON_AUX_POWER,
    I2C_ENGINE_HUNG,
    I2C_SEGMENT_HUNG,
    I2C_ADDRESS_NACK,
    I2C_DATA_NACK,
    I2C_ENGINE_LOCKED_BY_SOMEONE_ELSE,
    I2C_ENGINE_NOT_LOCKED, 
    I2C_ENGINE_OUT_OF_LOCKS,
    I2C_SNA_NO_CONNECTION,
    I2C_SNA_ENGINE_NOT_ENABLED,
    I2C_SNA_ENGINE_ALREADY_ENABLED,
    I2C_SNA_ENGINE_ALREADY_DISABLED,
    I2C_SNA_FAILED_TO_ENABLE_ENGINE,
    I2C_SNA_FAILED_TO_DISABLE_ENGINE,
    I2C_SNA_NOT_AVAILABLE_ON_AUX,
    I2C_SNOOP_TARGET_ALREADY_EXISTS,
    I2C_SNOOP_TARGETS_FULL,
    I2C_SNOOP_TARGET_DIDNT_EXIST,
    I2C_SNOOP_INVALID_ENTRY,
    I2C_NO_PERMISSION_TO_RUN_DURING_APML_PHASE_ONE,
    I2C_INVALID_REGISTER_DUMP_REQUEST,
    I2C_NULL_REGISTER_PTR,
    I2C_ENGINE_IS_NOT_IDLE,
    I2C_SIZE_GREATER_THAN_MAX_STANDARD_TRANSACT_ERROR,
    I2C_SIZE_GREATER_THAN_AVAILABLE_BUFFER_ERROR,
    I2C_COBOURG_REQUEST_WITH_WAIT_FOR_REPLY_SET, 
    I2C_NO_COBOURG_ENGINE,
    I2C_COBOURG_DEBUG_ALREADY_ON,
    I2C_COBOURG_DEBUG_ALREADY_OFF,
    I2C_MCTP_TIMEOUT_ERROR,
    I2C_NOT_A_DEBUG_TRANSACTION_TYPE, 
    I2C_UNSUPPORTED_TRANSACTION_TYPE,
    I2C_UNSUPPORTED_TRANSACTION_MODE,
    I2C_UNSUPPORTED_MUX_TYPE,
    I2C_ENGINE_DOES_NOT_EXIST,
    I2C_REQUEST_NOT_VALID_ON_VIRTUAL_SEGMENT,
    I2C_INTERRUPT_TIMEOUT_ERROR,
    I2C_RESPONSE_LARGER_THAN_BUFFER,
    I2C_UNKNOWN_ADMIN_REQUEST,
    I2C_ILO_DOES_NOT_OWN_BUS,
    I2C_LOST_ARBITRATION,
    I2C_ERROR_DURING_MUX_RESET,
    I2C_SSIF_GOT_ANOTHER_INT,
    I2C_IPMB_TIMEOUT_ERROR
} I2C_RETURN_CODES;

#ifdef __cplusplus
}
#endif

#endif
