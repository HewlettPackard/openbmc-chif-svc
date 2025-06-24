/*
// Copyright (c) 1996-2025 Hewlett Packard Enterprise Development, LP
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

#ifndef eventlog_h
#define eventlog_h

#include "DataExtract.h"

/****************************************************************************/
/* Common Readability Defines */
/****************************************************************************/

// Not used in the IML source but iLO 4 firmware also uses some of these
#ifndef BOOL
typedef uint8_t BOOL;
#endif
typedef char* PCHAR;
typedef char CHAR;
typedef unsigned char UCHAR;
typedef uint32_t ULONG;
typedef void VOID;
typedef unsigned char* PUCHAR;

/* Eventlog revision supported by this file (hdrRev in the header structure) */
// Changed the Log Revision to 3 in iLO  1.30 as a new field "flags" is carved out in EVT_HEADER structure
#define EVT_LOG_REVISION         0x03  /* TO-DO: GEN10 LOGGING - Move to Jarvis extract file? */

/* Raw return codes (returned in register AH) */
#define EVT_ROM_SUCCESS          0x00        /* All */
#define EVT_ROM_TOO_BIG          0x87        /* Log Event only */
#define EVT_ROM_END_OF_LOG       0x87        /* Read Event only */
#define EVT_ROM_TOO_SMALL        0x88        /* Read Event only */
#define EVT_ROM_ENTRY_NOT_FOUND  0x87        /* Update Event only */
#define EVT_ROM_INVALID_DATA     0x88        /* Log Event, Update Event */
#define EVT_ROM_NVRAM_LOCKED     0x8A        /* All, but only used on iLO systems */
                                             /*   and non-Embedded Health iLO 2 */

#define EVT_SHUTDOWN_FAN               0x01
#define EVT_SHUTDOWN_OVERHEAT          0x02
#define EVT_SHUTDOWN_VRM               0x03
#define EVT_SHUTDOWN_PS                0x04
#define EVT_SHUTDOWN_BBU               0x05
#define EVT_SHUTDOWN_ABORT             0x81
#define EVT_SHUTDOWN_FAN_ABORT         0x82
#define EVT_SHUTDOWN_TEMP_ABORT        0x83
#define EVT_SHUTDOWN_VRM_ABORT         0x84
#define EVT_SHUTDOWN_SOFT_POWER_DOWN   0x85
#define EVT_SOFTWARE_ASR_SHUTDOWN      0x86
#define EVT_SHUTDOWN_HARD_POWER_DOWN   0x87
#define EVT_SHUTDOWN_PS_ABORT          0x88					

//
//  EVT_ENCL_POWER_REQUEST_DENIED
//
#define POWER_REQUEST_DENIED_NOT_ENOUGH_POWER            0x01
#define POWER_REQUEST_DENIED_E_KEY_IO_CFG_ERR            0x02
#define POWER_REQUEST_DENIED_NOT_ENOUGH_COOLING          0x03
#define POWER_REQUEST_DENIED_POWER_HOLD                  0x04
#define POWER_REQUEST_DENIED_ENCLOSURE_BUSY              0x05
#define POWER_REQUEST_DENIED_GENERIC                     0x06

/* Eventlog Power Supply structs */
#define PS_NO_ERROR                           0xFF
#define PS_ERR_GEN_FAILURE                    0x00
#define PS_ERR_BIST_FAILURE                   0x01
#define PS_ERR_FAN_FAILURE                    0x02
// reserved                                   0x03
// reserved                                   0x04
#define PS_ERR_EPROM_FAILED                   0x05
#define PS_ERR_VREF_FAILED                    0x06
#define PS_ERR_DAC_FAILED                     0x07
#define PS_ERR_RAMTEST_FAILED                 0x08
#define PS_ERR_VOLTAGE_CHNL_FAILED            0x09
// reserved                                   0x0A
// reserved                                   0x0B
#define PS_ERR_GIVEUP_ON_STARTUP              0x0C
#define PS_ERR_NVRAM_INVALID                  0x0D
#define PS_ERR_CALIBRATION_TABLE_INVALID      0x0E
#define PS_ERR_NO_POWER_INPUT                 0x0F

/* Match code bits (unused bits are 0) */

#define EVT_BIT(n)               (1U << (n))
#define EVT_MATCH_CLASS          EVT_BIT(0)
#define EVT_MATCH_CODE           EVT_BIT(1)
#define EVT_MATCH_YEAR           EVT_BIT(2)
#define EVT_MATCH_MONTH          EVT_BIT(3)
#define EVT_MATCH_DAY            EVT_BIT(4)
#define EVT_MATCH_HOUR           EVT_BIT(5)
#define EVT_MATCH_MINUTE         EVT_BIT(6)
#define EVT_MATCH_SEVERITY       EVT_BIT(7)
#define EVT_MATCH_ALL_DATA       EVT_BIT(8)  /* All free form data */
#define EVT_MATCH_ITERATED       EVT_BIT(9)  /* All free form data - iLO internal use only */


/* Eventlog Standard match codes */
#define EVT_MATCH_NONE           0
#define EVT_MATCH_CLASS_CODE     (EVT_MATCH_CLASS      | \
                                  EVT_MATCH_CODE)
#define EVT_MATCH_SAME_DAY       (EVT_MATCH_CLASS_CODE | \
                                  EVT_MATCH_SEVERITY   | \
                                  EVT_MATCH_YEAR       | \
                                  EVT_MATCH_MONTH      | \
                                  EVT_MATCH_DAY        | \
                                  EVT_MATCH_ALL_DATA)
#define EVT_MATCH_SAME_HOUR      (EVT_MATCH_SAME_DAY   | \
                                  EVT_MATCH_HOUR)
#define EVT_MATCH_SAME_MIN       (EVT_MATCH_SAME_HOUR  | \
                                  EVT_MATCH_MINUTE)
#define EVT_MATCH_STANDARD       EVT_MATCH_SAME_HOUR

// Eventlog match code for AMS as Reporting Entity
#define EVT_MATCH_AMS_CONF_FLAG  OVERRIDE_REPORTING_ENTITY_AMS << 16

// Eventlog match code for SNMP & REST Override
#define EVT_MATCH_OVERRIDE_SNMP_TRAP  OVERRIDE_SNMP_TRAP << 16
#define EVT_MATCH_OVERRIDE_REST_ALERT  OVERRIDE_REST_ALERT << 16

/* Standard Unknown value */
#define EVT_GENERAL_UNKNOWN      0xFF

/* Standard occurrance count */
#define EVT_OCCUR_STANDARD       1

/* Chassis type defines */
#define EVT_CHASSIS_SYSTEM          0x01
#define EVT_CHASSIS_EXTERN_STORAGE  0x02
#define EVT_CHASSIS_INTERN_STORAGE  0x03
#define EVT_CHASSIS_BLADE           0x04     // New for GARdian/Ice (p-Class)
#define EVT_CHASSIS_POWER           0x05     // New for GARdian/Ice (p-Class)
#define EVT_CHASSIS_BLADE_SYSTEM    0x04     // redefined for existing agents
#define EVT_CHASSIS_DIST_POWER_SYSTEM 0x05   // ''
#define EVT_CHASSIS_ICE_STORAGE     0x04
#define EVT_CHASSIS_DISTRIB_POWER   0x05

/* Fan/Temp Zone locator defines */
#define EVT_LOCALE_SYSTEM           0x00  /* System board (compatibility value) */
#define EVT_LOCALE_SYSTEM_BOARD     0x01  /* Host System board */
#define EVT_LOCALE_IO_BOARD         0x02  /* I/O board */
#define EVT_LOCALE_CPU              0x03  /* CPU board */
#define EVT_LOCALE_MEMORY           0x04  /* Memory board */
#define EVT_LOCALE_STORAGE          0x05  /* Storage bays */
#define EVT_LOCALE_REM_MEDIA        0x06  /* Removable Media Bays */
#define EVT_LOCALE_POWER_SUPPLY     0x07  /* Power Supply Bays */
#define EVT_LOCALE_AMBIENT          0x08  /* Ambient / External / Room */
#define EVT_LOCALE_CHASSIS          0x09  /* Chassis */
#define EVT_LOCALE_BRIDGE_BOARD     0x0A  /* Bridge Card */
#define EVT_LOCALE_MGMT_BOARD       0x0B  /* Management board (driver define) */
#define EVT_LOCALE_MANAGEMENT_BOARD 0x0B  /* Remote Management Card */
#define EVT_LOCALE_BACKPLANE        0x0C  /* Generic Backplane */
#define EVT_LOCALE_NETWORK_SLOT     0x0D  /* Infrastructure Network */
#define EVT_LOCALE_BLADE_SLOT       0x0E  /* Blade Slot in Chassis / Infrastructure */
#define EVT_LOCALE_VIRTUAL          0x0F  /* Software-defined sensor */
#define EVT_LOCALE_UNKNOWN          EVT_GENERAL_UNKNOWN  /* Unknown */

//
//  EVT_ENCL_RACK_INFRASTRUCTURE_CHANGE
//
#define EVT_RACK_NAME_CHANGED         0x01
#define EVT_CHASSIS_NAME_CHANGED      0x02
#define EVT_SERVICE_CHANGED           0x03
#define EVT_RACK_NAME_CONFLICT        0x04
#define EVT_RACK_UID_CHANGED          0x05
// 0x06 is reserved
#define EVT_RACK_LAN_SETTINGS_CHANGED 0x07
#define EVT_RACK_UID_STATE_CHANGED    0x08

//
// EVT_CLASS_INTERLOCK defines
//
#define EVT_IL_DEV_NOT_SEATED_UNKNOWN     0
#define EVT_IL_DEV_NOT_SEATED_STANDBY     1
#define EVT_IL_DEV_NOT_SEATED_RUNTIME     2
#define EVT_IL_DEV_NOT_SEATED_POWERON     3
#define EVT_IL_DEV_NOT_SEATED_UNSPECIFIED 4
#define EVT_IL_DEV_NOT_SEATED_CONFIG      5

#endif /* eventlog_h */
