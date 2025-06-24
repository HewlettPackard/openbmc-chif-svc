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

#include "chif.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "romchfservice.hpp"
#include "romchf_msg.hpp"

#include "smbios.hpp"
#include "strutil.hpp"
#include "platdef.h"

#define SMBIOS_MUXES (96)  /* consistent with dvri2c */
#define FLAG_DBG  1 /* debug instrumentation */
#define FLAG_BUSY 2 /* busy (set on entry; cleared on exit) */
#define FLAG_DEP  4 /* deprecated */

extern smbios_cfg_type smbios_db;

/* smbios_type_string()
 *
 * This routine returns a string corresponding to the SMBIOS record type
 * or "unknown".
 * Record types 0-127 are defined by the SMBIOS specification.
 * Record types 128-254 are defined by the OEMs.
 *
 * Extend as needed.
 * Return string length should be fitted in romchf_msg.h:ROMCHF_REP_TYPE_STRING.string[64]
 */
const char *smbios_type_string(int type)  //used by smbios.o
{
    switch(type) {
        case 0:     return ((char *)"BIOS Information");
        case 1:     return ((char *)"System Information");
        case 2:     return ((char *)"Base Board Information");
        case 3:     return ((char *)"System Enclosure or Chassis");
        case 4:     return ((char *)"Processor Information");
        case 7:     return ((char *)"Cache Information");
        case 8:     return ((char *)"Port Connector Information");
        case 9:     return ((char *)"System Slots");
        case 11:    return ((char *)"OEM Strings");
        case 16:    return ((char *)"Physical Memory Array");
        case 17:    return ((char *)"Memory Device");
        case 18:    return ((char *)"Memory Error Info");
        case 19:    return ((char *)"Memory Array Mapped Address");
        case 20:    return ((char *)"Memory Device Mapped Address");
        case 32:    return ((char *)"System Boot Information");
        case 38:    return ((char *)"IPMI Device Information");
        case 39:    return ((char *)"System Power Supply");
        case 41:    return ((char *)"Onboard Devices Extended Information");
        case 42:    return ((char *)"Management Controller Host Interface");
        case 193:   return ((char *)"Other ROM Info"); /* 0xc1 */
        case 194:   return ((char *)"Super I/O Enable/Disable Features");
        case 195:   return ((char *)"Server System ID");
        case 196:   return ((char *)"Compaq System Information");
        case 197:   return ((char *)"Compaq Processor Information");
        case 198:   return ((char *)"Compaq Management Agent Information");
        case 199:   return ((char *)"CPU Microcode Patch Support Information");
        case 200:   return ((char *)"Service Required SIMM SPD Information");
        case 201:   return ((char *)"ProLiant Locator | SAL Access Table");
        case 202:   return ((char *)"Memory Location");
        case 203:   return ((char *)"HP Device Correlation Record");
        case 204:   return ((char *)"Rack Locator");
        case 205:   return ((char *)"Embedded ROM Virtual Disk Table");
        case 206:   return ((char *)"Memory Hotplug Extensions");
        case 207:   return ((char *)"Hood Sensor Extensions");
        case 208:   return ((char *)"Memory Information");
        case 209:   return ((char *)"Embedded NIC MAC Assignment");
        case 210:   return ((char *)"Net Power Consumption");
        case 211:   return ((char *)"Intel processor TControl");
        case 212:   return ((char *)"64-bit service informaton");
        case 213:   return ((char *)"Environment Variables Support");
        case 214:   return ((char *)"Integrated Management Log Support");
        case 215:   return ((char *)"ECC error threshold");
        case 216:   return ((char *)"Version indicator");
        case 217:   return ((char *)"AMD processor TControl");
        case 218:   return ((char *)"invalidated record");
        case 219:   return ((char *)"ProLiant ROM Power Feature Support");
        case 220:   return ((char *)"Logical processor map");
        case 221:   return ((char *)"HPQ NIC iSCSI MAC Info");
        case 222:   return ((char *)"c-Class Blade Port Map");
        case 223:   return ((char *)"Factory Mode Flag Info");
        case 224:   return ((char *)"TPM Module Status");
        case 225:   return ((char *)"ODM BIOS Locator");
        case 226:   return ((char *)"HPQ Physical Attributes");
        case 227:   return ((char *)"HPQ Module locator (I2C Map)");
        case 228:   return ((char *)"Deprecated HPQ I2C/PECI/SMBUS Segment Map");
        case 229:   return ((char *)"HPQ Reserved Memory Region");
        case 230:   return ((char *)"HPQ Power Supply Information");
        case 231:   return ((char *)"Detailed Flash Information");
        case 232:   return ((char *)"DIMM Attributes");
        case 233:   return ((char *)"NIC Information");
        case 234:   return ((char *)"I2C Device Information");
        case 236:   return ((char *)"Hard Drive Backplane Information");
        case 237:   return ((char *)"DIMM Vendor Part Number Information");
        case 238:   return ((char *)"USB port connector correlation");
        case 239:   return ((char *)"USB Device Correlation");
        case 240:   return ((char *)"Firmware Inventory Record");
        case 242:   return ((char *)"Hard Drive Inventory Record");
        case 243:   return ((char *)"Firmware GUID Correlation Record");
        case 244:   return ((char *)"DIMM Current Config Record");
        case 245:   return ((char *)"Extension Board Inventory Record");
        default:    break;
    }
    return((char*)"unknown");
}

/* smbios_rec_s()
 *
 * This routine returns a pointer to a string in the unstructured data area of
 * an SMBIOS record.  The ID corresponds to the string handle to be retrieved.
 * An empty string is returned for bad IDs.  IDs are 1-based.
 *
 * The string is valid for the duration of the input buffer.
 */
const char *smbios_rec_s(const void *p, int id)
{
    char *s;
    const SMBIOS_HDR *h = (SMBIOS_HDR *)p;
    int i;

    if (p && id)
    {
        s = (char *) p + h->len;
        if ( *s || *(s+1) ) // not at the end
        {
            for (i=1; i!=id; i++) // move forward through the strings (starting at 1) until id
            {
                while (*s) // move s forward until NULL
                {
                    s++;
                }
                /* end of data? */
                if (*(s+1)==0)
                { // end
                    break;
                }
                s++; // move s past first NULL
            }
            if ( (i == id) && *s )
            {
                return(s);
            }
        }
    }
    return("");
}


/* smbios_get_rec_by_handle()
 *
 * Retrieve record using the handle.  Caller provides buffer to receive
 * a copy of the record.
 *
 * returns:
 * 0: success
 * 1: not found
 * 2: bad parm
 * 3: result clipped
 */
int smbios_get_rec_by_handle(UINT16 handle, UINT8 *buf, UINT32 szbuf)
{
   int i;
   UINT32 sz;

   if (!buf || !szbuf) return SMBIOS_RETV_BAD_PARAM; // bad parm
   
   for (i=0; i<SMBIOS_NUM_REC; i++) {
      if ((smbios_db.index[i].length) &&
          (smbios_db.index[i].handle == handle)) {
         sz = (unsigned)smbios_db.index[i].length;
         if (sz>szbuf) {
            memcpy( buf, &smbios_db.data[smbios_db.index[i].offset], szbuf);
            return SMBIOS_RETV_CLIPPED; // clipped
         }
         memcpy( buf, &smbios_db.data[smbios_db.index[i].offset], sz);
         return SMBIOS_RETV_SUCCESS; // found, success
      }
   }
   return SMBIOS_RETV_NOT_FOUND; // not found
}


/* smbios_get_rec_by_type_and_handle()
 *
 * Not sure this makes sense - handles are unique.  See ...by_type_and_position
 * SNMP uses this, needs to be corrected
 */
int smbios_get_rec_by_type_and_handle(UINT8 type, UINT16 handle, UINT8 *buf, UINT32 szbuf )
{
   int i;
   UINT32 sz;

   if (!buf || !szbuf) return SMBIOS_RETV_BAD_PARAM; // bad parm
   
   for (i=0; i<SMBIOS_NUM_REC; i++) {
      if ((smbios_db.index[i].length) &&
          (smbios_db.index[i].type == type) &&
          (smbios_db.index[i].handle == handle)) {
         sz = (unsigned)smbios_db.index[i].length;
         if (sz>szbuf) {
            memcpy( buf, &smbios_db.data[smbios_db.index[i].offset], szbuf);
            return 3; // clipped
         }
         memcpy( buf, &smbios_db.data[smbios_db.index[i].offset], sz);
         return 0; // found, success
      }
   }
   return 1; // not found
}


/* smbios_get_rec_by_type_and_position()
 *
 * Retrieve record using the type and position (i.e. n-th occurence; n is zero-based).
 * Caller provides buffer to receive a copy of the record.
 *
 * returns:
 * 0: success
 * 1: not found
 * 2: bad parm
 * 3: result clipped
 */
int smbios_get_rec_by_type_and_position(UINT8 type, int position, UINT8 *buf, UINT32 szbuf)
{
   int i;
   UINT32 sz;

   if (!buf || !szbuf) return SMBIOS_RETV_BAD_PARAM; // bad parm

   for (i=0; i<SMBIOS_NUM_REC; i++) {
      if ((smbios_db.index[i].length) &&
          (smbios_db.index[i].type == type)) {
         if (position) {
            position--;
            continue;
         }
         sz = (unsigned)smbios_db.index[i].length;
         if (sz>szbuf) {
            memcpy( buf, &smbios_db.data[smbios_db.index[i].offset], szbuf);
            return SMBIOS_RETV_CLIPPED; // clipped
         }
         memcpy( buf, &smbios_db.data[smbios_db.index[i].offset], sz);
         return SMBIOS_RETV_SUCCESS; // found, success
      }
   }
   return SMBIOS_RETV_NOT_FOUND; // not found
}

/* smbios_get_module()
 *
 * This routine returns a composite combining parts of multiple SMBIOS record types.
 *
 * 227: OEM segment map locating I2C/PECI segment and address of the target
 *      The "type" field must match the input "matchtype" parameter
 * 202: OEM module location record
 * 197: OEM procesor location record
 *  17: corresponding memory module record handle, status, and interleave set
 *   4: corresponding CPU record handle, and presence
 *
 * DIMMs and Millbrooks (memory buffers with temperature sensors) both use
 * OEM type 227 records to describe their locations in the topology.  However
 * DIMMs use "type 1" and Millbrooks use "type 2" to differentiate the type.
 * Type 1 corresponds to the on-DIMM Microchip 98242 I2C EEPROM/SPD/temp sensor.
 *
 * Returns 0: module found
 *        !0: error - instance not available
 */
int smbios_get_module(UINT8 matchtype, int num, type_dimm *composite)
{
   type_4        r4;
   type_17       r17;
   type_197      r197;
   type_202      r202;
   type_227      r227;
   type_227     *p227;
   type_dimm     module;
   int           i;
   int           rc;

   /* num corresponds to the nth entry.  The order is that of SMBIOS */
   for (i=0; i<SMBIOS_NUM_REC; i++) {
      if ((SMBIOS_TYPE_DIMM_I2C == smbios_db.index[i].type) && (smbios_db.index[i].length)) {
         /*lint -e826 Suspicious pointer-to-pointer conversion (area too small) */
         p227 = (type_227 *) &smbios_db.data[smbios_db.index[i].offset];
         /*lint -restore */
         if (p227->type == matchtype) {
            if (num) {
               num--;
               continue;
            } else {
               break;
            }
         }
      }
   }
   // check to see if the scan has exceeded the limits
   if (i >= SMBIOS_NUM_REC) {
      return SMBIOS_RETV_NOT_FOUND;
   }
   // if nothing matches, we scanned all available records.
   if ((SMBIOS_TYPE_DIMM_I2C != smbios_db.index[i].type) || (!smbios_db.index[i].length)) {
      return SMBIOS_RETV_NOT_FOUND;
   }

   /* wipe the scratch results buffer */
   memset(&module, 0, sizeof(module));

   /* retrieve the module I2C map record */
   rc = smbios_get_rec_by_handle(smbios_db.index[i].handle&0xFFFF, (UINT8 *)&r227, sizeof(r227));
   if (rc==0 || rc==3) {
      module.dimm.hndl_type_17 = r227.hndl_type_17;
      module.cpu.hndl_type_4   = r227.hndl_type_4;
      module.i2c.seg           = r227.seg;
      module.i2c.addr          = r227.addr;
      module.i2c.type          = r227.type;  // sensor type
      module.dimm.group        = r227.group;
   }

   /* retrieve the corresponding memory module record */
   rc = smbios_get_rec_by_handle(r227.hndl_type_17, (UINT8 *)&r17, sizeof(r17));
   if (rc==0 || rc==3) {
      module.dimm.status   = (r17.size) ? 1 : 0; // If the module has a size, it is populated (status !0).
      module.dimm.set      = r17.set;   // Interleave group or set (sometimes used for error logging)
      module.dimm.mem_tech = r17.mem_tech;
   }

   /* set the spd size based on module type */
   switch(r17.type) {
     case 0x18:         // DDR3
         module.i2c.spd_size = 256;
         break;
     case 0x1A:         // DDR4
         module.i2c.spd_size = 512;
         break;
     case 0x22:         // DDR5
         module.i2c.spd_size = 1024;
         break;
      case 0x1f:         // CPS DDR5
         module.i2c.spd_size = 1024;
         break;
     default:           // Pre-DDR3
         module.i2c.spd_size = 256;
         break;
   }

   /* retrieve the corresponding processor record */
   rc = smbios_get_rec_by_handle(r227.hndl_type_4, (UINT8 *)&r4, sizeof(r4));
   if (rc==0 || rc==3) {
      module.cpu.status = (r4.cpu_status)?1:0;
   }

   /* find matching memory location record */
   for (i=0; i<SMBIOS_NUM_REC; i++) {
      if ((SMBIOS_TYPE_MEM_LOC == smbios_db.index[i].type) && (smbios_db.index[i].length)) {
         rc = smbios_get_rec_by_handle(smbios_db.index[i].handle&0xFFFF, (UINT8 *)&r202, sizeof(r202));
         if (rc==0 || rc==3) {
            if (r202.hndl_module == module.dimm.hndl_type_17) {
               module.dimm.slot   = r202.slot;
               module.dimm.socket = r202.socket;
               module.dimm.ie_dimm  = r202.ie_dimm;
               module.dimm.ie_sensor = r202.ie_sensor;
               module.dimm.dimm_index = r202.dimm_index;
               break;
            }
         }
      }
   }

   /* find matching processor information record */
   for (i=0; i<SMBIOS_NUM_REC; i++) {
      if ((SMBIOS_TYPE_CPQ_PROC == smbios_db.index[i].type) && (smbios_db.index[i].length)) {
         rc = smbios_get_rec_by_handle(smbios_db.index[i].handle&0xFFFF, (UINT8 *)&r197, sizeof(r197));
         if (rc==0 || rc==3) {
            if (r197.hndl_type_4 == module.cpu.hndl_type_4) {
               module.cpu.slot   = r197.slot;
               module.cpu.socket = r197.socket;
               break;
            }
         }
      }
   }

   *composite = module;
   return (SMBIOS_RETV_SUCCESS);
}

int smbios_get_dimm(int num, type_dimm *returndimm)
{
    return(smbios_get_module( SMBIOS_T227_MCP98242, num, returndimm));
}

int smbios_get_mb(int num, type_dimm *returndimm, UINT8 type)
{
    return(smbios_get_module(type, num, returndimm));
}

/* smbios_decode_216()
 *
 * Version Indicator Record type 216 - carries name and version fo a programmable.
 * Extract the programmable name (string) and programmable version (string) from a type 216 record.
 *
 * INPUT
 *      fw_name     : Pointer to buffer to hold name of FW.  Should be NULL terminated.
 *      fw_name_len : Size of fw_name buffer.  Recommend minimum size 48
 *      fw_ver      : Pointer to buffer to hold version string of FW.  Should be NULL terminated.
 *      fw_ver_len  : Size of fw_ver_len.  Recommend minimum size 32
 *
 * Returns:
 *      fw_name is a 1 or more byte null terminated string. strlen == 0 if empty
 *      fw_ver  is a 1 or more byte null terminated string. strlen == 0 if empty
 *
 *      0: success
 *      1: don't understand
 */
int smbios_decode_216(type_216 * t216,
                      char *fw_name, int fw_name_len,
                      char *fw_ver,  int fw_ver_len)
{
    UINT16 word;
    int    i;
    /* this is the lookup table for type_216 firmare types.
     * It is used only when the SMBIOS record lacks the label string (default)
     * It ideally is updated as new firmware types are defined in the HPE SMBIOS OEM records spec */
    struct {
       UINT16 type;
       char   *name;
    } type[]= {
          { FW_SYS_ROM,            (char*)"System ROM"                                            },
          { FW_REDUNDANT_SYS_ROM,  (char*)"Redundant System ROM"                                  },
          { FW_SYS_ROM_BBLK,       (char*)"Bootblock"                                             },
          { FW_PWR_PIC,            (char*)"Power Management Controller Firmware"                  },
          { FW_PWR_PIC_BOOTLDR,    (char*)"Power Management Controller Firmware Bootloader"       },
          { FW_SL_CHAS,            (char*)"SL Chassis Firmware"                                   },
          { FW_SL_CHAS_BOOTLDR,    (char*)"SL Chassis Firmware Bootloader"                        },
          { FW_PAL_CPLD,           (char*)"Hardware PAL/CPLD"                                     },
          { FW_SPS_FIRMWARE,       (char*)"SPS Firmware (ME Firmware)"                            },
          { FW_SL_CHAS_PAL_CPLD,   (char*)"SL Chassis Programmable Logic Device"                  },
          { FW_CSM,                (char*)"Compatibility Support Module (CSM)"                    },
          { FW_SYS_ROM_PLATDEF,    (char*)"Intelligent Platform Abstraction Data"                 },
          { FW_STORAGE_BATT,       (char*)"Smart Storage Battery (Megacell) Firmware"             },
          { FW_TPM_TCM,            (char*)"TM Firmware"                                           },
          { FW_NVME_PIC,           (char*)"NVMe Backplane Firmware"                               },
          { FW_IP,                 (char*)"Intelligent Provisioning"                              },
          { FW_SPI,                (char*)"SPI Descriptor Version"                                },
          { FW_IE,                 (char*)"IE Firmware"                                           },
          { FW_UBM_PIC,            (char*)"UBM PIC"                                               },
          { FW_ELCH_ABS,           (char*)"EL Chassis Abstraction Revision"                       },
          { FW_ELCH_FW,            (char*)"EL Chassis Firmware Revision"                          },
          { FW_ELCH_PAL,           (char*)"EL Chassis PAL/CPLD"                                   },
          { FW_ELCART_ABS,         (char*)"EL Cartridge Abstraction Revision"                     },
          { FW_EMB_VID_CTRL,       (char*)"Embedded Video Controller"                             },
          { FW_PCI_RISER_CPLD,     (char*)"PCIe Riser CPLD"                                       },
          { FW_PCI_CARD_CPLD,      (char*)"PCIe Card with a programmable CPLD"                    },
          { FW_INTEL_NVME_VROC,    (char*)"Intel NVME VROC FW"                                    },
          { FW_INTEL_SATA_VROC,    (char*)"Intel SATA VROC FW"                                    },
          { FW_INTEL_SPS,          (char*)"Intel SPS firmware"                                    },
          { FW_SEC_CPLD,           (char*)"Secondary System Programmable Logic Device"            },
          { FW_CPU_MEZZ_CPLD,      (char*)"CPU MEZZ System Programmable Logic Device"             },
          { FW_INTEL_ATSM,         (char*)"Intel Artic Sound -M (ATS-M) dual FW"                  },
          { FW_PAL_SCP,            (char*)"Ampere System Control Processor (SCP - PMPro + SMPro)" },
          { FW_INTEL_CFR,          (char*)"Intel CFR (CPU Fault Resiliency) "                     },
       };
 

    // Firmware Name - use the label from the SMBIOS record when it is present
    if (fw_name_len) {
       memset(fw_name, 0, fw_name_len);
       if (t216->sid_fw_name) {
          strncpy(fw_name, smbios_rec_s(t216, t216->sid_fw_name), (fw_name_len - 1));
       } else {
          /* default */
          strncpy(fw_name, "Firmware Component", fw_name_len-1);
          /* attempt lookup */
          for (i=0; i<(int)(sizeof(type)/sizeof(type[0])); i++) {
             if (t216->fw_type == type[i].type) {
                strncpy(fw_name, type[i].name, fw_name_len-1);
                break;
             }
          }
       }
    }

    // Firmware version
    if (!fw_ver_len) {
       return 0;
    }

    memset(fw_ver, 0, fw_ver_len);

    /* Use the version string from the SMBIOS record when it is present
     * Was "HP ISS ProLiant Server SMBIOS OEM Extensions for UEFI Systems v3.46"
     * Now "HPE Specification"
     *     "ProLiant Server SMBIOS OEM Extensions for UEFI Systems v3.83", section
     *     "Version Indicator Record (Type 216 Version Indicator Record (Type 216 )"
     */
    if (t216->sid_fw_ver) {
        strncpy(fw_ver, smbios_rec_s(t216, t216->sid_fw_ver), (fw_ver_len - 1));
        return 0;
    }

    switch (t216->ver_format) {
       case FORMAT_NONE: // display as string
          break;

       case FORMAT_PAL:  // 2 bytes PAL/CPLD versioning.  If 0x80 of first byte is set, bootleg.  Else release
          if (t216->ver.b[0] & 0x80) {
             snprintf(fw_ver, fw_ver_len, "0x%02X (bootleg 0x%02X)", (t216->ver.b[1] & 0x7f), (t216->ver.b[0] & 0x7f));
          } else {
             snprintf(fw_ver, fw_ver_len, "0x%02X", (t216->ver.b[1] & 0x7f));
          }
          break;

       case FORMAT_XY1:  // encoded as one byte (XY) displayed as X.Y
          snprintf(fw_ver, fw_ver_len, "%X.%X", ((t216->ver.b[0] & 0xf0)>>4), (t216->ver.b[0] & 0x0f));
          break;

       case FORMAT_SPS:  // (03) AAAA.BBBB.CCCC.DDDD.000E.0000 - SPS/ME FW Formatted  example 1.23.45.1234.15.0
          snprintf(fw_ver, fw_ver_len, "%d.%d.%d.%d.%d.%d", t216->ver.w[0], t216->ver.w[1], t216->ver.w[2], t216->ver.w[3], t216->ver.b[8], 0);
          break;

       case FORMAT_XYZ1: // (04) Gen9 PMC firmware where x, y are 4-bit hex numbers, z is 7-bit (major.minor.pass)
          snprintf(fw_ver, fw_ver_len, "%d.%d.%d", (t216->ver.b[0]&0xf0)>>4, (t216->ver.b[0]&0x0f), (t216->ver.b[1]&0x7F));
          break;

       case FORMAT_XYZ2: // (05) x, y are 4-bit hex numbers, z is 7-bit hex (Gen 9 SL/XL Chassis FW)
          snprintf(fw_ver, fw_ver_len, "%d.%d.%d", (t216->ver.b[0]&0xf0)>>4, (t216->ver.b[0]&0x0f), (t216->ver.b[1]&0x7F));
          break;

       case FORMAT_XYZ3: // (06) x, y are 8-bit hex numbers (APML/Megacell/NVMe backplane)
          snprintf(fw_ver, fw_ver_len, "%d.%d", t216->ver.b[1], t216->ver.b[0]);
          break;

       case FORMAT_XYD:  // (07) x, y are 8-bit hex numbers, Month (byte; 1:Jan), Day (byte); Year (16 bits)  (ROM/Redundant ROM)
          snprintf(fw_ver, fw_ver_len, "%d.%d (%02d/%02d/%04d)", t216->ver.b[0], t216->ver.b[1],
                t216->ver.b[2], t216->ver.b[3], t216->ver.w[2]);
          break;

       case FORMAT_XY2:  // (08) X (dword) y (dword) (i.e. TPM 1.2)
          snprintf(fw_ver, fw_ver_len, "%u.%u", t216->ver.d[0], t216->ver.d[1]);
          break;

       case FORMAT_XYZ4: // (09) x (byte), y (byte), z (word) (i.e. IP 2.30.1234)
          snprintf(fw_ver, fw_ver_len, "%d.%02d.%d", t216->ver.b[0], t216->ver.b[1], t216->ver.w[1]);
          break;

       case FORMAT_ABCD: // (0A) A (byte), B (byte), C (byte), D (build, omit if zero)  i.e. APML 1.2.3 or APML 1.2.3 build 5"
          if (t216->ver.b[3]) {
             snprintf(fw_ver, fw_ver_len, "%d.%d.%d build %d", t216->ver.b[0], t216->ver.b[1], t216->ver.b[2], t216->ver.b[3]);
          } else {
             snprintf(fw_ver, fw_ver_len, "%d.%d.%d", t216->ver.b[0], t216->ver.b[1], t216->ver.b[2]);
          }
          break;

       case FORMAT_YX_T:  // (0B) Y (word) X (word) time (uint32) Linux Epoch time displayed as X.Y t
          snprintf(fw_ver, fw_ver_len, "%u.%u %u", t216->ver.w[1], t216->ver.w[0], t216->ver.d[1]);
          break;

       case FORMAT_MmPB:  // (0C) AAAA.BBBB.CCCC.DDDD.[EEEE] Major.Minor.Patch.Build.[Debug] (all words, omit EEEE/Debug if zero)
          if (t216->ver.w[4])
          {
             snprintf(fw_ver, fw_ver_len - 1, "%d.%d.%d.%d %d", t216->ver.w[0], t216->ver.w[1], t216->ver.w[2], t216->ver.w[3], t216->ver.w[4]);
          }
          else
          {
             snprintf(fw_ver, fw_ver_len, "%d.%d.%d.%d", t216->ver.w[0], t216->ver.w[1], t216->ver.w[2], t216->ver.w[3]);
          }
          break;

       case FORMAT_X:  // (0D) X (1 byte) If bit 0x80 is set, bootleg
          snprintf(fw_ver, fw_ver_len - 1, "0x%02X%s", (t216->ver.b[0] & 0x7f), (t216->ver.b[0] & 0x80)? "(bootleg)":"");
          break;

       case FORMAT_VROC:      // 0x0E Intel VROC Versioning as A.B.C.D ex 7.5.0.1000; byte, byte, byte, word
          memcpy(&word, &t216->ver.b[3], sizeof(word));
          snprintf(fw_ver, fw_ver_len-1, "%d.%d.%d.%d", (t216->ver.b[0]), (t216->ver.b[1]), (t216->ver.b[2]), word);
          break;

       case FORMAT_INTEL_SPS: // 0x0F AAAA.BBBB.CCCC.DDDD (XX/YY/ZZZZ) four UINT16 words then MM/DD/YYYY (1-based)
          snprintf(fw_ver, fw_ver_len-1, "%d.%d.%d.%d (%02d/%02d/%04d)", t216->ver.w[0], t216->ver.w[1], t216->ver.w[2], t216->ver.w[3], t216->ver.b[8], t216->ver.b[9], t216->ver.w[5]);
          break;

       case FORMAT_ATSM:      // 0x10 Intel Artic Sound -M Accelerator example, DG02.1.3092 format string "%1c%1c%1c%1c_%d.%d"; byte, byte, byte, byte, word, word
          snprintf(fw_ver, fw_ver_len-1, "%1c%1c%1c%1c_%d.%d", (t216->ver.b[0]), (t216->ver.b[1]), (t216->ver.b[2]), (t216->ver.b[3]), (t216->ver.w[2]), (t216->ver.w[3]));
          break;

       case FORMAT_HEX32:     // 0x11 Intel Artic Sound -M Accelerator example, 0x11223344 format string "0x%X" (dword)  no leading zeroes
          snprintf(fw_ver, fw_ver_len-1, (char*)"0x%x", (t216->ver.d[0]));
          break;

       case FORMAT_CENTS:     // 0x12 Dollars and cents %d.%02d (leading zero for cents) - 2.06 not 2.6 (two bytes)
          snprintf(fw_ver, fw_ver_len-1, "%d.%02d", (t216->ver.b[0]), (t216->ver.b[1]));
          break;

       default:
          printf("smbios_decode_216_s: Unknown format %d handle 0x%04x\n", t216->ver_format, t216->hdr.handle);
          return 1;
    }

    return (0);
}

