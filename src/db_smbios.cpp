/*
// Copyright (c) 2021-2025 Hewlett-Packard Enterprise Development, LP
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
#include <chrono>
#include <thread>

#include "romchf_msg.hpp"
#include "smbios.hpp"
#include "strutil.hpp"
#include "uuid_gen.hpp"
#include "misc.hpp"

/* db_guid_to_text()
 *
 * From binary GUID, generate printable text using standard format
 *
 * GUID format is a little strange:
 * (4 byte LE)-(2 byte LE)-(2 byte LE)-((2)-(6) byte byte order)
 */
void db_guid_to_text(UINT8 *g, char *name)
{
   /* convert it to a string for telbug */
   dbPrintf("%s:          %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                    name,
                    g[3], g[2], g[1], g[0],
                    g[5], g[4],
                    g[7], g[6],
                    g[8], g[9],
                    g[10],g[11],g[12],g[13],g[14],g[15]);
}

/* smbios_rec_dump()
 *
 * Hex dumps the contents of an SMBIOS record
 * includes:
 *    type
 *    label
 *    computed actual length
 */
int  smbios_rec_dump(void *p)
{
    SMBIOS_HDR h_data;
    SMBIOS_HDR *h = &h_data;
    char *c = (char *)p;
    int i;

    if (!p) return(-1);

    memcpy(&h_data, p, sizeof(h_data)); //fixed the alignment problem (access violation)

    /* seek the true length of the record - start at the end of the record and seek two consecutive NULLs */
    i = h->len+2;
    while ((*(c+i-1)) || (*(c+i-2))) {
        i++;
    }

    dbPrintf( "HEXDUMP SMBIOS Record type:0x%02x handle:0x%04x(%u) sublength:0x%02x fullength[0x%02x]\n",
            h->type, h->handle, (unsigned)h->handle, h->len, i);
    hexdump_b(p, i);
    return(0);
}


/* from SMBIOS spec DSP0134_3.3.0
 * 7.43.1 Management Controller Host Interface Types */
char *t42_intf_type(UINT8 type)
{
   switch (type) {
      case TYPE_42_INTF_UNK:     return (char*)"unk";
      case TYPE_42_INTF_UNKN:    return (char*)"unkn";
      case TYPE_42_INTF_KCS:     return (char*)"KCS";
      case TYPE_42_INTF_8250:    return (char*)"8250";
      case TYPE_42_INTF_16450:   return (char*)"16450";
      case TYPE_42_INTF_16550:   return (char*)"16550";
      case TYPE_42_INTF_16650:   return (char*)"16650";
      case TYPE_42_INTF_16750:   return (char*)"16750";
      case TYPE_42_INTF_16850:   return (char*)"16850";
      case TYPE_42_INTF_REDFISH: return (char*)"Redfish";
   }
   return (char*)"unknown";
}

/* from SMBIOS spec DSP0134_3.3.0
 * 7.43.2 Management Controller Host Interface - Protocol Types */
char *t42_prot_type(UINT8 type)
{
   switch (type) {
      case 0:                    return (char*)"reserved";
      case 1:                    return (char*)"reserved";
      case TYPE_42_PROT_IPMI:    return (char*)"IPMI";
      case TYPE_42_PROT_MCTP:    return (char*)"MCTP";
      case TYPE_42_PROT_RESTIP:  return (char*)"Redfish over IP";
      case TYPE_42_PROT_OEM:     return (char*)"OEM-defined";
   }
   return (char*)"unknown";
}

typedef struct {
    union {
        type_0      t0;
        type_1      t1;
        type_2      t2;
        type_3      t3;
        type_4      t4;
        type_7      t7;
        type_8      t8;
        type_9      t9;
        type_11     t11;
        type_16     t16;
        type_17     t17;
        type_19     t19;
        type_20     t20;
        type_32     t32;
        type_38     t38;
        type_39     t39;
        type_41     t41;
        type_42     t42;
        type_193    t193;
        type_194    t194;
        type_195    t195;
        type_196    t196;
        type_197    t197;
        type_198    t198;
        type_199    t199;
        type_201    t201;
        type_202    t202;
        type_203    t203;
        type_204    t204;
        type_209    t209;
        type_210    t210;
        type_211    t211;
        type_215    t215;
        type_216    t216;
        type_219    t219;
        type_222    t222;
        type_223    t223;
        type_224    t224;
        type_226    t226;
        type_227    t227;
        type_228    t228;
        type_229    t229;
        type_230    t230;
        type_232    t232;
        type_233    t233;
        type_234    t234;
        type_236    t236;
        type_237    t237;
        type_238    t238;
        type_239    t239;
        type_240    t240;
        type_242    t242;
        type_243    t243;
        type_244    t244;
        type_245    t245;
    };
} SMBIOS_TYPES;

/* smbios_rec_decode()
 *
 * Decode the contents of an SMBIOS record or invokes hexdump if unknown.
 *
 * Extend as needed.
 */
int  smbios_rec_decode(void *p)
{
    SMBIOS_HDR h_data;
    SMBIOS_HDR *h = &h_data;

    char *t9_slot1_char[] =
    {
        (char*)"Characteristics Unknown.",
        (char*)"Provides 5.0 volts.",
        (char*)"Provides 3.3 volts.",
        (char*)"Slot's opening is shared with another slot.",
        (char*)"PC Card slot supports PC Card-16.",
        (char*)"PC Card slot supports CardBus.",
        (char*)"PC Card slot supports Zoom Video.",
        (char*)"PC Card slot supports Modem Ring Resume"
    };

    char *t9_slot2_char[] =
    {
        (char*)"PCI slot supports Power Management Event Signal.",
        (char*)"Slot supports hot-plug devices.",
        (char*)"PCI slot supports SMBus signal",
        (char*)"PCI slot supports bifurcation.",
        (char*)"Reserved = 0",
        (char*)"Reserved = 0",
        (char*)"Reserved = 0",
        (char*)"Reserved = 0"
    };

    UINT16 u16;
    int i, j, rc;
    char buf[40];

    char label[100];
    char ver[60];
    SMBIOS_TYPES *r;

    r = (SMBIOS_TYPES *)p;

    if (!p) return(-1);

    memcpy(&h_data, p, sizeof(h_data)); //fixed the alignment problem (access violation)

    dbPrintf("\r\n\"%s\" (type %d) handle=0x%04x sublength:0x%x\n",
            smbios_type_string(h->type), h->type, h->handle, h->len );

    switch(h->type) {
//#ifdef DECODE
        case 0:
            dbPrintf("Vendor:      \"%s\"\n", smbios_rec_s(p, r->t0.sid_vendor));
            dbPrintf("BIOS Version:\"%s\"\n", smbios_rec_s(p, r->t0.sid_bios_version));
            dbPrintf("BIOS Starting address segment: 0x%04x\n", r->t0.bios_start_addr_seg);
            dbPrintf("BIOS Date:   \"%s\"\n", smbios_rec_s(p, r->t0.sid_bios_date));
            dbPrintf("ROM Size:    64K*(%d+1)\n", r->t0.sz_rom);
            dbPrintf("BIOS characteristics:\t0x%08x 0x%08x\n",
                    (UINT32) (r->t0.bios_char >> 32), 
                    (UINT32) (r->t0.bios_char & 0xFFFFFFFF));
            dbPrintf("BIOS major:  %d\n", r->t0.bios_maj);
            dbPrintf("BIOS minor:  %d\n", r->t0.bios_min);
            dbPrintf("ECFW major:  %d\n", r->t0.ecfw_maj);
            dbPrintf("ECFW minor:  %d\n", r->t0.ecfw_min);
            break;

        case 1:
            dbPrintf("Manufacturer: \"%s\"\n", smbios_rec_s(p, r->t1.sid_manu));
            dbPrintf("Product:      \"%s\"\n", smbios_rec_s(p, r->t1.sid_prod));
            dbPrintf("Version:      \"%s\"\n", smbios_rec_s(p, r->t1.sid_ver));
            dbPrintf("Serial Number:\"%s\"\n", smbios_rec_s(p, r->t1.sid_sn));
            /* note that the byte ordering is consistent with DMTF SMBIOS 2.6a spec */
            sprintf((char*)buf, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                  r->t1.uuid[3], r->t1.uuid[2], r->t1.uuid[1], r->t1.uuid[0],
                  r->t1.uuid[5], r->t1.uuid[4],
                  r->t1.uuid[7], r->t1.uuid[6],
                  r->t1.uuid[8], r->t1.uuid[9],
                  r->t1.uuid[10], r->t1.uuid[11], r->t1.uuid[12], r->t1.uuid[13], r->t1.uuid[14], r->t1.uuid[15]);
            dbPrintf("UUID:         \"%s\" ", buf);

            /* check for special cases */
            memcpy(buf, r->t1.uuid, 16);
            buf[16]=0;
            /* UUID attributes:
             * Universal Unique ID number.
             * If the value is all FFh,
             *  the ID is not currently present in the system, but is settable.
             * If the value is all 00h, the ID is not present in the system.
             */
            // scan for all FFs
            for (i=0;i<16;i++) {
                if (buf[i]!=(char)0xff) break;
            }
            if (i==16) { // all FFs
               dbPrintf(">> not set <<");
            } else {
                // scan for all 0s
                for (i=0;i<16;i++) {
                    if (buf[i]!=0) {
                       dbPrintf("%s", "");
                       break;
                    }
                }
                if (i==16) { // all 0s
                   dbPrintf(">> not present <<\n");
                }
            }
            dbPrintf("wakeup:       0x%02x\n", r->t1.wakeup);
            dbPrintf("SKU number:   \"%s\"\n", smbios_rec_s(p, r->t1.sid_sku_num));
            dbPrintf("Family:       \"%s\"\n", smbios_rec_s(p, r->t1.sid_family));
            break;

        case 2:
            dbPrintf("Manufacturer: \"%s\"\n", smbios_rec_s(p, r->t2.sid_manu));
            dbPrintf("Product:      \"%s\"\n", smbios_rec_s(p, r->t2.sid_prod));
            dbPrintf("Version:      \"%s\"\n", smbios_rec_s(p, r->t2.sid_ver));
            dbPrintf("Serial Number:\"%s\"\n", smbios_rec_s(p, r->t2.sid_sn));
            dbPrintf("Asset Tag:    \"%s\"\n", smbios_rec_s(p, r->t2.sid_asset_tag));
            dbPrintf("Feature Flags: %02x\n",  r->t2.feature_flags);
            dbPrintf("Chassis Loc:  \"%s\"\n", smbios_rec_s(p, r->t2.sid_chassis_loc));
            dbPrintf("type 3:       0x%04x\n", r->t2.t3_hndl);
            dbPrintf("board:        %d",     r->t2.board);
            dbPrintf("%d handles:",          r->t2.n_handles);
            for (i=0; i<r->t2.n_handles; i++) {
               dbPrintf("[%d]:       0x%04x\n", i, (UINT16)r->t2.handle[i]);
            }
            break;
            
        case 3:
            dbPrintf("Manufacturer:   \"%s\"\n", smbios_rec_s(p, r->t3.sid_manu));
            dbPrintf("Type:            0x%x\n", r->t3.type);
            dbPrintf("Version:        \"%s\"\n", smbios_rec_s(p, r->t3.sid_ver));
            dbPrintf("Serial Number:  \"%s\"\n", smbios_rec_s(p, r->t3.sid_sn));
            dbPrintf("Asset Tag:      \"%s\"\n", smbios_rec_s(p, r->t3.sid_atag));
            dbPrintf("Boot:            0x%x\n", r->t3.boot);
            dbPrintf("Power Supply:    0x%x\n", r->t3.ps);
            dbPrintf("Thermal:         0x%x\n", r->t3.therm);
            dbPrintf("Security:        0x%x\n", r->t3.sec);
            dbPrintf("OEM Information: 0x%x\n", r->t3.oeminfo);
            dbPrintf("height:          0x%x\n", r->t3.height);
            dbPrintf("num power cords: 0x%x\n", r->t3.n_pwr_cords);
            dbPrintf("num elements:    0x%x\n", r->t3.n_elem);
            dbPrintf("size element:    0x%x\n", r->t3.sz_elem);
            break;

        case 4:
            dbPrintf("Socket:         \"%s\"\n", smbios_rec_s(p, r->t4.sid_sock));
            dbPrintf("CPU Type:       0x%x\n", r->t4.cpu_type);
            dbPrintf("CPU Fam:        0x%x\n", r->t4.cpu_fam);
            dbPrintf("CPU Manufacturer:\"%s\"\n", smbios_rec_s(p, r->t4.sid_cpu_manuf));
            dbPrintf("CPUID:          0x%04x%04x%04x%04x\n",
                    r->t4.cpuid[0], r->t4.cpuid[1], r->t4.cpuid[2], r->t4.cpuid[3]);
            dbPrintf("CPU Version:    \"%s\"\n", smbios_rec_s(p, r->t4.sid_cpu_ver));
            dbPrintf("CPU Voltage:    0x%x\n", r->t4.cpu_voltage);
            dbPrintf("CPU Clock:      0x%x\n", r->t4.cpu_clock);
            dbPrintf("CPU Max Speed:  0x%x\n", r->t4.cpu_max_speed);
            dbPrintf("CPU Speed:      0x%x\n", r->t4.cpu_speed);
            dbPrintf("CPU Status:     0x%x\n", r->t4.cpu_status);
            dbPrintf("CPU Upgrade:    0x%x\n", r->t4.cpu_upgrade);
            dbPrintf("CPU L1 Cache handle:    0x%04x\n", r->t4.hndl_l1);
            dbPrintf("CPU L2 Cache handle:    0x%04x\n", r->t4.hndl_l2);
            dbPrintf("CPU L3 Cache handle:    0x%04x\n", r->t4.hndl_l3);
            dbPrintf("CPU Serial Number:      \"%s\"\n", smbios_rec_s(p, r->t4.sid_cpu_sn));
            dbPrintf("CPU Asset Tag:          \"%s\"\n", smbios_rec_s(p, r->t4.sid_cpu_atag));
            dbPrintf("CPU Part Number:        \"%s\"\n", smbios_rec_s(p, r->t4.sid_cpu_pn));
            dbPrintf("Number of cores:         0x%x\n", r->t4.n_cores);
            dbPrintf("Number of enabled cores: 0x%x\n", r->t4.n_cores_ena);
            dbPrintf("Number of threads:       0x%x\n", r->t4.n_thrd);
            dbPrintf("CPU capabilities:        0x%04x %s\n", r->t4.cpu_cap,
                             (r->t4.cpu_cap & 0x4)?"64-bit capable":"\n");
            dbPrintf("CPU family:              0x%04x\n", r->t4.cpu2_fam);
            dbPrintf("cores:                   0x%04x\n", r->t4.n2_cores);
            dbPrintf("cores enabled:           0x%04x\n", r->t4.n2_cores_ena);
            dbPrintf("threads:                 0x%04x\n", r->t4.n2_thrd);
            break;

        case 7:
            dbPrintf("Socket:             \"%s\"\n", smbios_rec_s(p, r->t7.sid_sock));
            dbPrintf("Cache Configuration: 0x%04x\n", r->t7.cfg);
            dbPrintf("Max Cache Size:      0x%04x\n", r->t7.max);
            dbPrintf("Installed Cache Size:0x%04x\n", r->t7.size);
            dbPrintf("Supported SRAM type: 0x%04x\n", r->t7.sram_s);
            dbPrintf("Current SRAM type:   0x%04x\n", r->t7.sram);
            dbPrintf("Cache Speed:         0x%02x\n", r->t7.speed);
            dbPrintf("ECC:                 0x%02x\n", r->t7.ecc);
            dbPrintf("Cache Type:          0x%02x\n", r->t7.type);
            dbPrintf("Associativity:       0x%02x\n", r->t7.associativity);
            break;
            
        case 8:
            dbPrintf("Internal Reference Designator:\t%s\n", smbios_rec_s(p, r->t8.sid_ird));
            dbPrintf("Internal Connector:\t%d\n", r->t8.int_connector);
            dbPrintf("User Reference Designator:\t%s\n", smbios_rec_s(p, r->t8.sid_u_ref)); 
            dbPrintf("External Connector:\t%d\n", r->t8.ext_connector);
            dbPrintf("Port Type:\t%d\n", r->t8.port);
            break;

        case 9:
            dbPrintf("Slot:                \"%s\"\n", smbios_rec_s(p, r->t9.sid_slot));
            dbPrintf("Type:                 %d\n", r->t9.type);
            dbPrintf("Data Width:           %d\n", r->t9.width);
            dbPrintf("Current usage:        %d\n", r->t9.current);
            dbPrintf("Length:               %d\n", r->t9.length);
            dbPrintf("Slot ID:              0x%04x\n", r->t9.id);
            dbPrintf("Properties:           0x%02x %02x\n", r->t9.properties[0], r->t9.properties[1]);
            dbPrintf("Segment Group Number: %d\n", (int)r->t9.seg_group_number);
            dbPrintf("Bus/Device/Function:  %02X/%02X/%02X\n", (int)r->t9.bus,
                              (int)((r->t9.dev_fn>>3)&0x1f),
                              (int)((r->t9.dev_fn)&0x07));
            dbPrintf("Data Bus Width:       %d\n", r->t9.data_bus_width);
            if (r->t9.hdr.len > VER_2_6_SZ)
            {
                dbPrintf("Number of Peers:      %d\n", r->t9.peer_num);
                for (i = 0; (i < r->t9.peer_num) && (i < T9_MAX_PEER_GROUP); i++)
                {
                    dbPrintf("Peer:                 %d\n", i);
                    dbPrintf("  Segment Group Num:  %d\n", r->t9.peer_grp[i].seg_grp_num);
                    dbPrintf("  Bus Number:         %d\n", r->t9.peer_grp[i].bus_number);
                    dbPrintf("  Dev/Func Number:    %d\n", r->t9.peer_grp[i].dev_func_num);
                    dbPrintf("  Data Bus Width:     %d\n", r->t9.peer_grp[i].data_bus_width);
                }
            }
            dbPrintf("Slot 1 Characteristics:      0x%02x\n", r->t9.properties[0]);
            for (i = 0; i < 8; i++) // Loop through Slot 1 Characteristics bitmask (SMBIOS v3.2 sec 7.10.6)
            {
                if ((r->t9.properties[0] & (0x01 << i)) == (0x01 << i))
                    dbPrintf("  %s\n", t9_slot1_char[i]);
            }
            dbPrintf("Slot 2 Characteristics:      0x%02x\n", r->t9.properties[1]);
            for (i = 0; i < 8; i++) // Loop through Slot 2 Characteristics bitmask (SMBIOS v3.2 sec 7.10.7)
            {
                if ((r->t9.properties[1] & (0x01 << i)) == (0x01 << i))
                    dbPrintf("  %s\n", t9_slot2_char[i]);
            }
            break;

        case 11:
            dbPrintf("Number of OEM Strings: %d\n", r->t11.num);
            for (i=1; i <= r->t11.num; i++) {
               dbPrintf("[%2d]: \"%s\"\n", i, smbios_rec_s(p, i));
            }
            break;

        case 16:
            dbPrintf("Location:            %d\n", r->t16.location);
            dbPrintf("Use:                 %d\n", r->t16.use);
            dbPrintf("ECC mechanism:       %d\n", r->t16.ecc);
            dbPrintf("Max capacity (KB):   0x%x\n", r->t16.max_kb);
            dbPrintf("Error Handle:        0x%04x\n", r->t16.hndl_err);
            dbPrintf("Available Sockets:   %d\n", r->t16.sockets);
            dbPrintf("Capabilities:        0x%08x %08x\n", (UINT32)(r->t16.cap >> 32), (UINT32) (r->t16.cap & 0xFFFFFFFF));
            break;

        case 17:
            dbPrintf("Array Handle:     0x%04x\n", r->t17.hndl_array);
            dbPrintf("Error Handle:     0x%04x\n", r->t17.hndl_error);
            dbPrintf("Bit Width:        0x%04x\n", r->t17.width_tot);
            dbPrintf("Data Width:       0x%04x\n", r->t17.width_dat);
            if (r->t17.size == 0xffff)
            {
                dbPrintf("Size:             0x%016lx", r->t17.nvol_sz);
            }
            else
            {
                dbPrintf("Size:             0x%04x\n", r->t17.size);
            }                
            dbPrintf("Form Factor:      %d\n", r->t17.ff);
            dbPrintf("Device Set:       %d\n", r->t17.set);
            dbPrintf("Device Locator:   %s\n", smbios_rec_s(p, r->t17.sid_device));
            dbPrintf("Bank Locator:    \"%s\"\n", smbios_rec_s(p, r->t17.sid_bank));
            dbPrintf("Type:             0x%02x\n", r->t17.type);
            dbPrintf("Type Detail:      0x%04x\n", r->t17.detail);
            dbPrintf("Speed:            %d MHz\n", r->t17.speed);
            dbPrintf("Manufacturer:    \"%s\"\n", smbios_rec_s(p, r->t17.sid_manu));
            dbPrintf("Serial Number:   \"%s\"\n", smbios_rec_s(p, r->t17.sid_sn));
            dbPrintf("Asset Tag:       \"%s\"\n", smbios_rec_s(p, r->t17.sid_atag));
            dbPrintf("Part Number:     \"%s\"\n", smbios_rec_s(p, r->t17.sid_pn));
            dbPrintf("Attributes:       %u\n", r->t17.attr);
            dbPrintf("Extended Size:    %u\n", r->t17.extended_size);
            dbPrintf("Clock Speed:      %u MHz\n", r->t17.clock_speed);
            dbPrintf("Minimum mV:       %u\n", r->t17.v_min);
            dbPrintf("Maximum mV:       %u\n", r->t17.v_max);
            dbPrintf("Configured mV:    %u\n", r->t17.v_cfg);
            if (r->t17.hdr.len >= offsetof(type_17, mem_tech))
            {
                if (r->t17.mem_tech != 0)
                {
                    dbPrintf("\033[1;33mMemory Tech:      %u\033[0m\n", r->t17.mem_tech);
                }
                else
                {
                    dbPrintf("Memory Tech:      %u\n", r->t17.mem_tech);
                }
                dbPrintf("Memory Op Mode:   0x%04x\n", r->t17.mem_omcap);
                dbPrintf("FW Version:       \"%s\"\n", smbios_rec_s(p, r->t17.fw_ver));
                dbPrintf("Manf ID:          0x%04x\n", r->t17.mmanuf_id);
                dbPrintf("Prod ID:          0x%04x\n", r->t17.mprod_id);
                dbPrintf("Cntrl Manf ID:    0x%04x\n", r->t17.mscmanuf_id);
                dbPrintf("Cntrl Prod ID:    0x%04x\n", r->t17.mscprod_id);
                dbPrintf("Non-Vol Size:     %ld MiB\n", r->t17.nvol_sz);
                dbPrintf("Vol Size:         %ld MiB\n", r->t17.vol_sz);
                dbPrintf("Cache Size:       %ld MiB\n", r->t17.cach_sz);
                dbPrintf("Logical Size:     %ld MiB\n", r->t17.logical_sz);
            }
            break;

        case 19:
            dbPrintf("Starting address:\t0x%08x\n", r->t19.b_paddr);
            dbPrintf("Ending address:\t0x%08x\n", r->t19.e_paddr);
            dbPrintf("Array handle:\t0x%04x\n", r->t19.hndl_array);
            dbPrintf("Width:\t%d\n", r->t19.width);
            dbPrintf("Start Address:\t%ld\n", r->t19.ext_start_addr);
            dbPrintf("End Address:\t%ld\n", r->t19.ext_end_addr);
            break;

        case 20:
            dbPrintf("Starting address:\t0x%08x\n", r->t20.b_paddr);
            dbPrintf("Ending address:\t0x%08x\n", r->t20.e_paddr);
            dbPrintf("Device handle:\t0x%04x\n", r->t20.hndl_device);
            dbPrintf("Array handle:\t0x%04x\n", r->t20.hndl_array);
            dbPrintf("Row:\t%d\n", r->t20.row);
            dbPrintf("Interleave:\t%d\n", r->t20.interleave);
            dbPrintf("Depth:\t%d\n", r->t20.depth);
            
            break;

        case 32:
            dbPrintf("Reserved: 0x%02x %02x %02x %02x %02x %02x\n",
                r->t32.reserved[0], r->t32.reserved[1], r->t32.reserved[2],
                r->t32.reserved[3], r->t32.reserved[4], r->t32.reserved[5] );
            dbPrintf("Status:   0x%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                r->t32.status[0], r->t32.status[1], r->t32.status[2], r->t32.status[3],
                r->t32.status[4], r->t32.status[5], r->t32.status[6], r->t32.status[7],
                r->t32.status[8], r->t32.status[9] );
            break;

        case 38:
            dbPrintf("BMC type:            %d:%s\n", r->t38.bmc_type,
               (r->t38.bmc_type == 0)?"unknown":
               (r->t38.bmc_type == 1)?"KCS":
               (r->t38.bmc_type == 2)?"SMIC":
               (r->t38.bmc_type == 3)?"BT":"rsv:future");
            dbPrintf("IPMI Spec level:     %d.%d\n", (r->t38.rev>>4)&0xF, r->t38.rev&0xF);
            dbPrintf("I2C Slave Address:   0x%02x\n", r->t38.bmc_addr);
            dbPrintf("NV storage bus ID:   0x%02x\n", r->t38.nv_bus);
            dbPrintf("Base Address:        0x%08x %08x (%s)\n",
                  (UINT32) (r->t38.base_addr >>32),
                  (UINT32) (r->t38.base_addr & 0xFFFFFFFF),
                  (r->t38.base_addr%2)?"I/O":"Memory-mapped");
            dbPrintf("Base Addr Modifier:  0x%02x\n", r->t38.bam);
            dbPrintf("Interrupt Number:    0x%02x\n", r->t38.intr);
            break;

        case 39:
            dbPrintf("Power Unit Group: %d\n", r->t39.pugroup);
            dbPrintf("Location:        \"%s\"\n", smbios_rec_s(p, r->t39.sid_loc));
            dbPrintf("Device Name:     \"%s\"\n", smbios_rec_s(p, r->t39.sid_devname));
            dbPrintf("Manufacturer:    \"%s\"\n", smbios_rec_s(p, r->t39.sid_manu));
            dbPrintf("Serial Number:   \"%s\"\n", smbios_rec_s(p, r->t39.sid_sn));
            dbPrintf("Asset Tag:       \"%s\"\n", smbios_rec_s(p, r->t39.sid_atag));
            dbPrintf("Model:           \"%s\"\n", smbios_rec_s(p, r->t39.sid_model));
            dbPrintf("Revision:        \"%s\"\n", smbios_rec_s(p, r->t39.sid_rev));
            dbPrintf("Capacity:         %d %s\n", r->t39.cap, (r->t39.cap==0x8000)?"(unk)":" watts");
            dbPrintf("Characteristics:  0x%04x\n", r->t39.attr);
            dbPrintf("handle-Volts:     0x%04x\n", r->t39.hndl_26);
            dbPrintf("handle-Cooling:   0x%04x\n", r->t39.hndl_27);
            dbPrintf("handle-Amps:      0x%04x\n", r->t39.hndl_29);
            break;

        case 41:
            dbPrintf("Reference Designation: \"%s\"\n", smbios_rec_s(p, r->t41.sid_ref_designation));
            dbPrintf("Device Status:         \"%s\"\n", (r->t41.dev_type & 0x80)?"Enabled":"Disabled");
            dbPrintf("Device Type (0x%02x):    \"%s\"\n", r->t41.dev_type & 0x7f,
               ((r->t41.dev_type & 0x7f) == 0x01)?  "Other":
               ((r->t41.dev_type & 0x7f) == 0x02)?  "Unknown":
               ((r->t41.dev_type & 0x7f) == 0x03)?  "Video":
               ((r->t41.dev_type & 0x7f) == 0x04)?  "SCSI Controller":
               ((r->t41.dev_type & 0x7f) == 0x05)?  "Ethernet":
               ((r->t41.dev_type & 0x7f) == 0x06)?  "Token Ring":
               ((r->t41.dev_type & 0x7f) == 0x07)?  "Sound":
               ((r->t41.dev_type & 0x7f) == 0x08)?  "PATA Controller":
               ((r->t41.dev_type & 0x7f) == 0x09)?  "SATA Controller":
               ((r->t41.dev_type & 0x7f) == 0x0A)?  "SAS Controller": "Unknown: value > 0Ah");
            dbPrintf("Device Type Instance:  0x%02x\n", r->t41.dev_type_instance);
            dbPrintf("Segment Group Number:  %02X\n", (int)r->t41.seg_group_number);
            dbPrintf("Bus/Device/Function:   %02X/%02X/%02X\n", (int)r->t41.bus,
                              (int)((r->t41.dev_fn>>3)&0x1f),
                              (int)((r->t41.dev_fn)&0x07));
            break;
        case 42:
            {
               type_42_proto_header *ph;
               type_42_protocol     *prot;

               dbPrintf("Management Controller Interface Type:\t0x%02x: %s\n",
                     r->t42.intf_type, t42_intf_type(r->t42.intf_type));
               dbPrintf("Interface Data size:\t\t\t0x%02x\n", r->t42.sz_dat);
               hexdump_b_hind(r->t42.dat, r->t42.sz_dat, (char *)"Interface Type Specific Data    ");
               /* calculate protocol records header offset using size of data */
               ph = (type_42_proto_header *)(r->t42.dat + r->t42.sz_dat);
               dbPrintf("Number of Protocol Records:\t\t%d\n", ph->protocol_records);
               j = (int) ph->protocol_records;
               /* calculate protocol record offset from records header data */
               prot = (type_42_protocol *)ph->dat;
               for (i=0; i<j; i++) {
                  dbPrintf("Protocol Record [%d]\n", i);
                  dbPrintf(" Protocol Type:\t\t\t0x%02x:%s\n", prot->type, t42_prot_type(prot->type));
                  dbPrintf(" Protocol Data Length:\t\t0x%02x\n", prot->len);
                  hexdump_b_hind(prot->dat, prot->len, (char *)" Protocol Data ");
                  /* advance protocol record pointer to start of next record */
                  prot = (type_42_protocol *)(prot->dat + prot->len);
               }
            }

            break;
#if 0
        case 192:
            dbPrintf("Version:\t%d\n", r->t192.version);
            dbPrintf("Slot Number:\t%d\n", r->t192.slot_number);
            dbPrintf("Slot Characteristics:\t0x%08x\n", r->t192.slot_prop);
            break;
#endif
        case 193:
            dbPrintf("Redundant ROM installed: %d\n", r->t193.rrom_inst);
            dbPrintf("Redundant ROM version:  \"%s\"\n", smbios_rec_s(p, r->t193.sid_rrom));
            dbPrintf("Boot Block ROM version: \"%s\"\n", smbios_rec_s(p, r->t193.sid_bblk));
            dbPrintf("OEM ROM file:           \"%s\"\n", smbios_rec_s(p, r->t193.sid_oem_rom_file));
            dbPrintf("OEM ROM date:           \"%s\"\n", smbios_rec_s(p, r->t193.sid_oem_rom_date));

            break;

        case 194:
            dbPrintf("Status: 0x%02x\n", r->t194.status);
            break;

        case 195:
            dbPrintf("Server System ID:   %s\n", smbios_rec_s(p, r->t195.sid_ssid));
            memcpy(&u16, r->t195.plat_id, sizeof(u16));
            dbPrintf("Server platform ID: 0x%04x\n", u16);
            if (r->t195.hdr.len > VER_195_PRE_3_71)
            {
                uuid_get_canonical((char*)r->t195.ver_3_71.guid, sizeof(r->t195.ver_3_71.guid), (PUUID_T)buf);
                dbPrintf("GUID:              %s\n", buf);
                dbPrintf("                  ");
                for (i = 0; i < (int)sizeof(r->t195.ver_3_71.guid); i++)
                {
                    dbPrintf(" %02X\n", r->t195.ver_3_71.guid[i]);
                }
                dbPrintf("%s\n", "");
            }
            break;

        case 196:
            dbPrintf("LCD Characteristics:    0x%02x\n", r->t196.lcd_prop);
            dbPrintf("rack_u_offset:          0x%02x\n", r->t196.rack_u_offset);
            dbPrintf("efuse_thresholds:       0x%08x\n", r->t196.efuse_thresholds);
            dbPrintf("boot_mode_indicator:    0x%02x\n", r->t196.boot_mode_indicator);
            dbPrintf("supported_socket_count: 0x%04x\n", r->t196.supported_socket_count);
            dbPrintf("secure_boot_state:      0x%02x\n", r->t196.secure_boot_state);
            dbPrintf("Feature state:          0x%02x\n", r->t196.feature_state);
            if (r->t196.hdr.len > VER_196_PRE_3_69)
            {
                dbPrintf("PCH Name:               %s\n", smbios_rec_s(p, r->t196.ver_3_69.sid_pch_name));
                dbPrintf("PCH Revision:           0x%02X\n", r->t196.ver_3_69.pch_rev);
            }
            break;

        case 197:
            dbPrintf("type 4 handle: 0x%04x\n", r->t197.hndl_type_4);
            dbPrintf("APIC ID:       0x%02x\n", r->t197.apic_id);
            dbPrintf("OEM status:    0x%02x\n", r->t197.oem_status);
            dbPrintf("Slot:          0x%02x\n", r->t197.slot);
            dbPrintf("Socket:        0x%02x\n", r->t197.socket);
            dbPrintf("Max Wattage:   0x%04x\n", r->t197.max_watt);
            dbPrintf("x2APIC ID:     0x%08x\n", r->t197.x2apic_id);
            sprintf(buf, "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                r->t197.proc_uuid[0], r->t197.proc_uuid[1], r->t197.proc_uuid[2], r->t197.proc_uuid[3], 
                r->t197.proc_uuid[4], r->t197.proc_uuid[5], r->t197.proc_uuid[6], r->t197.proc_uuid[7]);
            dbPrintf("Proc UUID:     %s\n", buf);
            dbPrintf("UPI Speed:     0x%04x\n", r->t197.upi_speed);
            if (r->t197.hdr.len > VER_197_PRE_3_68)
            {
                dbPrintf("Proc QDF Num:  ");
                for (i = 0; i < (int)sizeof(r->t197.ver_3_68.proc_qdf_num); i++)
                {
                    dbPrintf("%02X\n", r->t197.ver_3_68.proc_qdf_num[i]);
                }
                dbPrintf("%s\n", "");
            }
            break;

        case 198:
            dbPrintf("Security:      0x%04x\n", r->t198.security);
            dbPrintf("ECC Threshold: %d\n",     r->t198.ecc_thresh);
            dbPrintf("ECC Interval:  %d\n",     r->t198.ecc_interval);
            dbPrintf("Sys Type:      %d\n",     r->t198.sys_type);
            dbPrintf("iCRU minDly:   %d ms\n",  r->t198.icru_delay);
            dbPrintf("min PS:        %d\n",     r->t198.min_ps);
            dbPrintf("min PS redund: %d\n",     r->t198.min_ps_red);
            break;

        case 199:
            /* calculate number of patch records: (record size-header)/record size */
            i = (h->len - sizeof(SMBIOS_HDR))/12;
            for (j=0; j<i; j++) {
                dbPrintf("Patch[%d] ID:    0x%08x\n", j, r->t199.patch[j].id);
                dbPrintf("          Date:  0x%08x\n",    r->t199.patch[j].date);
                dbPrintf("          CPUID: 0x%08x\n",    r->t199.patch[j].cpuid);
            }
            break;
#if 0
        case 200:
            dbPrintf("Memory Module Handle:\t0x%04x\n", r->t200.hndl_module);
            /* calculate number of SPD pairs */
            j = (((r->t200.hdr.len-sizeof(type_200))+2)/2);
            for (i=0; i<j; i++) {
                dbPrintf("SPD Data: %02x %02x, \n", r->t200.spd[i].offs, r->t200.spd[i].data);
            }
            dbPrintf("");
            break;
#endif
        case 201:
            dbPrintf("MEMBIST NVRAM offset:0x%08x\n", r->t201.member_union.proliant.membist_nvram_offset);
            dbPrintf("              length:0x%02x\n", r->t201.member_union.proliant.membist_length);
            dbPrintf("              format:0x%04x\n", r->t201.member_union.proliant.membist_format);
            dbPrintf("Scratchpad Offset:   0x%08x\n", r->t201.member_union.proliant.reboot_nvram_offset);
            dbPrintf("Scratchpad size:     0x%02x\n", r->t201.member_union.proliant.reboot_length);
#if 0 
            dbPrintf("\nIA64 interpretation:\r\n "
                             " Version: %d\r\n"
                             " Address: 0x%08x %08x\n",
                             r->t201.member_union.SAL_table.version,
                             r->t201.member_union.SAL_table.address[0],
                             r->t201.member_union.SAL_table.address[1]);
#endif
            break;

        case 202:
            dbPrintf("DIMM handle:           0x%04x\n", r->t202.hndl_module);
            dbPrintf("slot:                  0x%02x\n", r->t202.slot);
            dbPrintf("socket:                0x%02x\n", r->t202.socket);
            dbPrintf("processor:             0x%02x\n", r->t202.processor);
            dbPrintf("logical ID:            0x%02x\n", r->t202.logical_dimm_id);
            dbPrintf("path:                  %s\n",     smbios_rec_s(p, r->t202.sid_uefi_dev_path));
            dbPrintf("structured name:       %s\n",     smbios_rec_s(p, r->t202.sid_uefi_dev_structured_name));
            dbPrintf("name:                  %s\n",     smbios_rec_s(p, r->t202.sid_uefi_device_name));
            dbPrintf("controller number:     0x%02x\n", r->t202.ctrl);
            dbPrintf("channel number:        0x%02x\n", r->t202.chan);
            dbPrintf("IE number:             0x%02x\n", r->t202.ie_dimm);
            dbPrintf("IE PLDM sensor ID:     0x%02x\n", r->t202.ie_sensor);
            dbPrintf("Vendor ID code:        0x%04x\n", r->t202.id_vend);
            dbPrintf("Device ID code:        0x%04x (NVDIMM only)\n", r->t202.id_dev);
            dbPrintf("Subsys ctrl vendor ID: 0x%04x (NVDIMM only)\n", r->t202.id_ctrlvend);
            dbPrintf("Subsys ctrl dev ID:    0x%04x (NVDIMM only)\n", r->t202.id_ctrldev);
            dbPrintf("dimm number:           0x%02x \n", r->t202.dimm_index);
            if (r->t202.hdr.len > VER_202_PRE_3_61)
            {
                dbPrintf("Interleaved Set:       0x%02x\n", r->t202.ver_3_61.bestInterleavedSet);
                if (r->t202.hdr.len > VER_202_PRE_3_71)
                {
                    dbPrintf("Part Number:           %s\n", smbios_rec_s(p, r->t202.ver_3_71.sid_part_num));
                }
            }
            break;

        case 203:
#if 0 /* DEPRECATED format */
            dbPrintf("ROM Family Data:\t");
            j = ((r->t203.hdr.len-sizeof(SMBIOS_HDR))/2);
            for (i=0; i<j; i++) {
                dbPrintf("%c%d, \n",
                    r->t203.family[i].maj,
                    r->t203.family[i].min);
            }
            dbPrintf("");
#else
            {
                SMBIOS_HDR      *smb_h;
                unsigned char   buffer[256];
                UINT16          h;
                type_9          *t9;
                type_41         *t41;

                t9    = (type_9 *)buffer;
                t41   = (type_41 *)buffer;
                smb_h = (SMBIOS_HDR *)buffer;

                h = r->t203.assoc_dev_hndl;
                dbPrintf("Associated Device Handle:    0x%04x\n", h);
                if (h != INVALID_HANDLE) {
                   rc = smbios_get_rec_by_handle(h, buffer, sizeof(buffer));
                   if (!rc) {
                      switch (smb_h->type) {
                         case 9:
                            dbPrintf(" Type:%2d, Slot:                  \"%s\"\n", smb_h->type, smbios_rec_s(smb_h, t9->sid_slot));
                            break;

                        case 41:
                            dbPrintf(" Type:%2d, Reference Designation: \"%s\"\n", smb_h->type, smbios_rec_s(smb_h, t41->sid_ref_designation));
                            break;

                        default:
                            dbPrintf(" INVALID RECORD TYPE!  %2d\n", smb_h->type);
                            break;
                      }
                   } else {
                      dbPrintf(" Handle not found!  %d\n", rc);
                   }
                } else if (r->t203.assoc_dev_hndl == INVALID_HANDLE &&
                           r->t203.assoc_smbus_hndl == INVALID_HANDLE &&
                           r->t203.parent_hndl != INVALID_HANDLE) {
                    dbPrintf(" N/A for child 203 record, see parent handle 0x%04x\n", r->t203.parent_hndl);
                } else {
                    dbPrintf("  *** INVALID Associated Device Handle ***\n");
                }

                dbPrintf("Associated SMBus Handle:     0x%04x\n", r->t203.assoc_smbus_hndl);

                if (r->t203.assoc_smbus_hndl != INVALID_HANDLE) {
                   rc = smbios_get_rec_by_handle(r->t203.assoc_smbus_hndl, buffer, sizeof(buffer));
                   if (!rc) {
                        dbPrintf(" **** Corresponding Record *****\n");
                        if( smb_h->type == 228 ) {
                            // Yep, this is recursive but it should work.
                            smbios_rec_decode(smb_h);
                        } else {
                            dbPrintf(" Expected 228, handle points to type: %d\n", smb_h->type);
                        }
                   } else {
                      dbPrintf(" Associated handle not found!  rc = %d\n", rc);
                   }
                } else if (r->t203.assoc_dev_hndl == INVALID_HANDLE &&
                           r->t203.assoc_smbus_hndl == INVALID_HANDLE &&
                           r->t203.parent_hndl != INVALID_HANDLE) {
                    dbPrintf(" N/A for child 203 record, see parent handle 0x%04x\n", r->t203.parent_hndl);
                } else {
                    dbPrintf(" Type 228 Handle 0xFFFE. Not an I2C accessible device\n");
                }

                dbPrintf("PCIe Vend ID:                0x%04x\n", r->t203.pci_vend_id);
                dbPrintf("PCIe Dev ID:                 0x%04x\n", r->t203.pci_dev_id);
                dbPrintf("PCIe Sub-Vend ID:            0x%04x\n", r->t203.pci_sub_vend_id);
                dbPrintf("PCIe Sub-Dev ID:             0x%04x\n", r->t203.pci_sub_dev_id);
                dbPrintf("PCIe Class Code:             0x%02x\n", r->t203.pci_class_code);
                dbPrintf("PCIe Sub-Class Code:         0x%02x\n", r->t203.pci_sub_class_code);
                dbPrintf("Parent 203 Handle:           0x%04x\n", r->t203.parent_hndl);
                dbPrintf("Flags:                       0x%04x\n", r->t203.flags);

                dbPrintf("Device Type:                 0x%02x\n", r->t203.uefi.device_type);
                dbPrintf("Device Location:             0x%02x\n", r->t203.uefi.device_location);
                dbPrintf("Device Instance:             0x%02x\n", r->t203.uefi.device_inst);
                dbPrintf("Device Sub-Instance:         0x%02x\n", r->t203.uefi.device_sub_inst);
                dbPrintf("Bay:                         0x%02x\n", r->t203.uefi.bay);
                dbPrintf("Enclosure:                   0x%02x\n", r->t203.uefi.enclosure);
                dbPrintf("UEFI Device Path:            %s\n", smbios_rec_s(r, r->t203.uefi.sid_dev_path));
                dbPrintf("UEFI Device Structured Name: %s\n", smbios_rec_s(r, r->t203.uefi.sid_dev_structured_name));
                dbPrintf("UEFI Device Name:            %s\n", smbios_rec_s(r, r->t203.uefi.sid_device_name));
                dbPrintf("UEFI Location:               %s\n", smbios_rec_s(r, r->t203.uefi.sid_location));
                dbPrintf("Associated handle:           0x%04x\n", r->t203.bifurcated_hndl_t9);
                if (r->t203.hdr.len > VER_203_PRE_3_66)
                {
                    dbPrintf("Device VPD Part Number:      %s\n", smbios_rec_s(r, r->t203.pcie_vpd.sid_dev_part_number));
                    dbPrintf("Device VPD Serial Number:    %s\n", smbios_rec_s(r, r->t203.pcie_vpd.sid_dev_serial_number));
                    if (r->t203.hdr.len > VER_203_PRE_3_72)
                    {
                        dbPrintf("Segment Group Num:           0x%02x\n", r->t203.ver_3_72.seg_grp_num);
                        dbPrintf("PCI Bus:                     0x%02x\n", r->t203.ver_3_72.pci_bus);
                        dbPrintf("PCI Dev/Func:                0x%02x\n", r->t203.ver_3_72.pci_dev_func);
                    }
                }
            }
#endif
            break;

        case 204:
            dbPrintf("Rack Name:\t\"%s\"\n", smbios_rec_s(r, r->t204.sid_rack));
            dbPrintf("Enclosure Name:\t\"%s\"\n", smbios_rec_s(r, r->t204.sid_encl));
            dbPrintf("Enclosure Model:\t\"%s\"\n", smbios_rec_s(r, r->t204.sid_encl_model));
            dbPrintf("Bay:\t\"%s\"\n", smbios_rec_s(r, r->t204.sid_bay));
            dbPrintf("Enclosure Bays:\t%d\n", r->t204.encl_bays);
            dbPrintf("Blade Bays:\t%d\n", r->t204.blade_bays);
            dbPrintf("Enclosure sernum:\t%s\n", smbios_rec_s(r, r->t204.sid_encl_sernum));
            dbPrintf("BMC IP address:\t%s\n", smbios_rec_s(r, r->t204.sid_ilo_bmc_ipaddr));
            hexdump_b_hind(r->t204.solutions, sizeof(r->t204.solutions), (char*)"Solutions:\t\t");
            break;
#if 0
        case 205:
            dbPrintf("Version: %d\n", r->t205.version);
            dbPrintf("Images:  %d\n", r->t205.images);
            dbPrintf("Image Data:\r\n  Sig          ---Address---         Length");
            for (i=0; i<r->t205.images; i++) {
                dbPrintf("0x%08x 0x%08x 0x%08x 0x%08xx\n",
                    r->t205.img[i].sig,
                    r->t205.img[i].addr[0],
                    r->t205.img[i].addr[1],
                    r->t205.img[i].length);
            }
            break;

        case 206:
            dbPrintf("Memory Module Handle:\t0x%04x\n", r->t206.hndl_module);
            dbPrintf("Status:\t%d\n", r->t206.status);
            //printf("Reserved:\t%d\n", r->t206.reserved);
            break;

        case 207:
            i = r->t207.status;
            dbPrintf("Status: 0x%x:%s\n", i, (i==0)?"disabled":
                                           (i==1)?"notify user":
                                           (i==2)?"reserved":
                                           (i==3)?"setup password":
                                           (i==0xff)?"not supported":"unknown");
            dbPrintf("time/date:0x%08x\n", r->t207.datetime);
            break;

        case 208:
            i = r->t208.status;
            dbPrintf("Status: 0x%x:%s\n", i, (i==0)?"single channel":
                                           (i==1)?"dual channel":
                                           (i==0xff)?"not supported":"unknown");
            break;
#endif

        case 209:
            if ((r->t209.nic[0].dev_fn)||(r->t209.nic[0].bus)) {
                dbPrintf("NIC\tdev_fn\tbus\tMAC");
            }
            /*lint -e661  out of bounds access possible (by design)
             * This SMBIOS record exists if there is at least one NIC of this type,
             * but the length varies depending on the number of NICs.  The structure
             * is designed to the size of the smallest entry, and some array abuse
             * is intentional to support longer record types.
             */
            for (i=0; (r->t209.nic[i].dev_fn)||(r->t209.nic[i].bus); i++) {
                printf ("%d\t0x%02x\t%d\t%02X-%02X-%02X-%02X-%02X-%02X\n",
                        i+1,
                        r->t209.nic[i].dev_fn,
                        r->t209.nic[i].bus,
                        r->t209.nic[i].mac[0], r->t209.nic[i].mac[1], r->t209.nic[i].mac[2],
                        r->t209.nic[i].mac[3], r->t209.nic[i].mac[4], r->t209.nic[i].mac[5] );
            }
            /*lint -restore*/
            break;

        case 210:
            dbPrintf("Net Watts:\t%d\n", r->t210.watts);
            dbPrintf("Min Watts:\t%d\n", r->t210.min_watts);
            dbPrintf("Prochot hiWatts:\t%d\n", r->t210.watts_phot_h);
            dbPrintf("PRochit loWatts:\t%d\n", r->t210.watts_phot_l);
            break;

        case 211:
            dbPrintf("Proc record Handle: 0x%04x\n", r->t211.hndl_type_4);
            dbPrintf("TControl:           %d\n", r->t211.tcontrol);
            break;
#if 0
        case 212:
            dbPrintf("Address: 0x%08x%08x\n", r->t212.addr[0], r->t212.addr[1]);
            dbPrintf("Length:  0x%x\n", r->t212.len);
            dbPrintf("Offset:  0x%x\n", r->t212.offs);
            break;

        case 213:
            dbPrintf("Misc control:\t0x%x\n", r->t213.ctrl);
            dbPrintf("Storage type:\t0x%x\n", r->t213.type);
            dbPrintf("Offset to logsize&csum:\t0x%x\n", r->t213.size);
            dbPrintf("EV Log Start:\t0x%x\n", r->t213.start);
            dbPrintf("Offset to status word:\t0x%x\n", r->t213.status);
            dbPrintf("EV Look-up table address:\t0x%08x%08x\n", r->t213.addr[0], r->t213.addr[1]);
            dbPrintf("EV look-up table length:\t0x%x\n", r->t213.len);
            break;

        case 214:
            dbPrintf("IML Version:\t0x%x\n", r->t214.ver);
            dbPrintf("storage type:\t0x%x\n", r->t214.type);
            dbPrintf("pointer size:\t0x%x\n", r->t214.size_ptr);
            dbPrintf("signature pointer:\t0x%x\n", r->t214.offs_sig);
            dbPrintf("IML base addr:\t0x%x\n", r->t214.offs_base);
            dbPrintf("IML end addr:\t0x%x\n", r->t214.offs_end);
            dbPrintf("head pointer:\t0x%x\n", r->t214.offs_head);
            dbPrintf("tail pointer:\t0x%x\n", r->t214.offs_tail);
            dbPrintf("high pointer:\t0x%x\n", r->t214.offs_high);
            dbPrintf("low pointer:\t0x%x\n", r->t214.offs_low);
            dbPrintf("last pointer:\t0x%x\n", r->t214.offs_last);
            dbPrintf("counter pointer:\t0x%x\n", r->t214.offs_counter);
            break;
#endif
        case 215:
            dbPrintf("SHAFT flags:                  0x%02x\n", r->t215.flags);
            dbPrintf("MCA warranty rate (ECC/hour): %d\n", r->t215.rate);
            break;

        case 216:
            /* decodes according to HP OEM SMBIOS extensions */
            dbPrintf("Type:     %d:%s\n",
                  r->t216.fw_type,
                  (r->t216.fw_type == FW_SYS_ROM           )?"Sys ROM":
                  (r->t216.fw_type == FW_REDUNDANT_SYS_ROM )?"Redundant ROM":
                  (r->t216.fw_type == FW_SYS_ROM_BBLK      )?"Boot Block":
                  (r->t216.fw_type == FW_PWR_PIC           )?"PwrPIC":
                  (r->t216.fw_type == FW_PWR_PIC_BOOTLDR   )?"PwrPIC bootldr":
                  (r->t216.fw_type == FW_SL_CHAS           )?"Carb":
                  (r->t216.fw_type == FW_SL_CHAS_BOOTLDR   )?"Carb bootldr":
                  (r->t216.fw_type == FW_PAL_CPLD          )?"PAL":
                  (r->t216.fw_type == FW_SPS_FIRMWARE      )?"SPS/ME":
                  (r->t216.fw_type == FW_SL_CHAS_PAL_CPLD  )?"SL Chas PAL/CPLD":
                  (r->t216.fw_type == FW_STORAGE_BATT      )?"Storage Battery":
                  (r->t216.fw_type == FW_CSM               )?"CSM":
                  (r->t216.fw_type == FW_SYS_ROM_PLATDEF   )?"APML":
                  (r->t216.fw_type == FW_TPM_TCM           )?"TPM/TCM/TM":
                  (r->t216.fw_type == FW_NVME_PIC          )?"NVMe":
                  (r->t216.fw_type == FW_IP                )?"IP":
                  (r->t216.fw_type == FW_SPI               )?"SPI":
                  (r->t216.fw_type == FW_IE                )?"IE":
                  (r->t216.fw_type == FW_UBM_PIC           )?"UBM":
                  (r->t216.fw_type == FW_ELCH_ABS          )?"EL Chassis Abstraction":
                  (r->t216.fw_type == FW_ELCH_FW           )?"EL Chassis Firmware":
                  (r->t216.fw_type == FW_ELCH_PAL          )?"EL Chassis PAL/CPLD":
                  (r->t216.fw_type == FW_ELCART_ABS        )?"EL Cartridge Abstraction":
                  (r->t216.fw_type == FW_EMB_VID_CTRL      )?"Embedded Video Controller (iLO)":
                  (r->t216.fw_type == FW_PCI_RISER_CPLD    )?"PCIe Riser CPLD":
                  (r->t216.fw_type == FW_PCI_CARD_CPLD     )?"PCIe Card CPLD":
                  (r->t216.fw_type == FW_INTEL_SPS         )?"Intel SPS firmware":
                  (r->t216.fw_type == FW_SEC_CPLD          )?"Secondary CPLD":
                  (r->t216.fw_type == FW_CPU_MEZZ_CPLD     )?"CPU MEZZ programmable logic device":
                  (r->t216.fw_type == FW_INTEL_ATSM        )?"Intel Artic Sound-M Accelerator Models Firmware":
                  (r->t216.fw_type == FW_PAL_SCP           )?"Ampere System Control Processor":
                  (r->t216.fw_type == FW_INTEL_CFR         )?"Intel CPU Fault Resilience (CFR)":

                  (r->t216.fw_type == 64                   )?"CPLD reserved": /* reserved range 0x40..0x4F */
                  (r->t216.fw_type == 65                   )?"CPLD reserved":
                  (r->t216.fw_type == 66                   )?"CPLD reserved":
                  (r->t216.fw_type == 67                   )?"CPLD reserved":
                  (r->t216.fw_type == 68                   )?"CPLD reserved":
                  (r->t216.fw_type == 69                   )?"CPLD reserved":
                  (r->t216.fw_type == 70                   )?"CPLD reserved":
                  (r->t216.fw_type == 71                   )?"CPLD reserved":
                  (r->t216.fw_type == 72                   )?"CPLD reserved":
                  (r->t216.fw_type == 73                   )?"CPLD reserved":
                  (r->t216.fw_type == 74                   )?"CPLD reserved":
                  (r->t216.fw_type == 75                   )?"CPLD reserved":
                  (r->t216.fw_type == 76                   )?"CPLD reserved":
                  (r->t216.fw_type == 77                   )?"CPLD reserved":
                  (r->t216.fw_type == 78                   )?"CPLD reserved":
                  (r->t216.fw_type == 79                   )?"CPLD reserved":
                  "other");
            dbPrintf("Name:    \"%s\"\n", smbios_rec_s(r, r->t216.sid_fw_name));
            dbPrintf("Ver:     \"%s\"\n", smbios_rec_s(r, r->t216.sid_fw_ver));
            dbPrintf("Format:  %2d\n", r->t216.ver_format);
            hexdump_b_hind(r->t216.ver.b, sizeof(r->t216.ver.b), (char*)"Data:    ");

            std::this_thread::sleep_for(std::chrono::microseconds(20));
            //usleep(20);
            smbios_decode_216((type_216 *)r, label, sizeof(label), ver, sizeof(ver));
            dbPrintf(" decoded: %-48s %s\n", label, ver);
            dbPrintf("Unique ID:%d\n", r->t216.uniq_id);
            break;
#if 0
        case 217:
            dbPrintf("Proc record Handle:\t0x%04x\n", r->t217.hndl_type_4);
            dbPrintf("TControl:\t%d\n", r->t217.tcontrol);
            break;

        case 218:
            dbPrintf("this record is a nullified record placeholder.");
            break;
#endif
        case 219:
            dbPrintf("ROM Power Support Feature Flags:           0x%08x\n", r->t219.flags);
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_STATIC_LO,    (r->t219.flags & SMBIOS_T219_STATIC_LO    )?"Yes":"no", "Power Regulator Static Low");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_DYNAMIC,      (r->t219.flags & SMBIOS_T219_DYNAMIC      )?"Yes":"no", "Power Regulator Dynamic");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_OS_CTRL,      (r->t219.flags & SMBIOS_T219_OS_CTRL      )?"Yes":"no", "OS Level Power Management");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_PSTATE_MON,   (r->t219.flags & SMBIOS_T219_PSTATE_MON   )?"Yes":"no", "iLO Pstate monitoring");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_PSTATE_FORCE, (r->t219.flags & SMBIOS_T219_PSTATE_FORCE )?"Yes":"no", "iLO Pstate forcing");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_QSTATE_CAP,   (r->t219.flags & SMBIOS_T219_QSTATE_CAP   )?"Yes":"no", "iLO Qstate capping");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_PROC_UTIL,    (r->t219.flags & SMBIOS_T219_PROC_UTIL    )?"Yes":"no", "Proc Utilization reporting");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_STATIC_HI,    (r->t219.flags & SMBIOS_T219_STATIC_HI    )?"Yes":"no", "Static High Performance Mode");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_PWR_METER_ROM,(r->t219.flags & SMBIOS_T219_PWR_METER_ROM)?"Yes":"no", "Pwr Mtr Availability controlled by ROM");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_PWR_METER,    (r->t219.flags & SMBIOS_T219_PWR_METER    )?"Yes":"no", "Pwr Mtr Support Available");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_AMD_ULTRALO,  (r->t219.flags & SMBIOS_T219_AMD_ULTRALO  )?"Yes":"no", "AMD Ultra Low Power mode");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_ADV_CAP,      (r->t219.flags & SMBIOS_T219_ADV_CAP      )?"Yes":"no", "Advanced Power Capping available");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_JITTER_CTRL_SUPPORT, (r->t219.flags & SMBIOS_T219_JITTER_CTRL_SUPPORT)?"Yes":"no", "Jitter Control Supported");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_JITTER_CTRL_CONFIG, (r->t219.flags & SMBIOS_T219_JITTER_CTRL_CONFIG)?"Yes":"no", "Jitter Control Configurable Threshold/Rate Supported");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_TELEM,        (r->t219.flags & SMBIOS_T219_TELEM        )?"Yes":"no", "Telemetry Supported");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_PERF_ADV,     (r->t219.flags & SMBIOS_T219_PERF_ADV     )?"Yes":"no", "Performance Advisory Supported");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_PSYS_CAP,     (r->t219.flags & SMBIOS_T219_PSYS_CAP     )?"Yes":"no", "Intel PSYS Power Capping Supported");
            
            dbPrintf("ROM Omega/Triton Support Feature Flags:    0x%08x\n", r->t219.omega_triton_flags);
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_GAIUS_EMBEDDED,    (r->t219.omega_triton_flags & SMBIOS_T219_GAIUS_EMBEDDED)?"Yes":"no", "Embedded Gaius Support");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_BLACKBOX_SUPPORT,  (r->t219.omega_triton_flags & SMBIOS_T219_BLACKBOX_SUPPORT)?"Yes":"no", "Blackbox Support");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_VIRT_USB_INST_DISK,(r->t219.omega_triton_flags & SMBIOS_T219_VIRT_USB_INST_DISK)?"Yes":"no", "Virtual USB Install Disk Support");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_EMBEDDED_ACU,      (r->t219.omega_triton_flags & SMBIOS_T219_EMBEDDED_ACU)?"Yes":"no", "Embedded ACU Support");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_NO_OS_INST,        (r->t219.omega_triton_flags & SMBIOS_T219_NO_OS_INST)?"Yes":"no", "No OS inst");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_IPROV_SMB,         (r->t219.omega_triton_flags & SMBIOS_T219_IPROV_SMB)?"Yes":"no", "Intelligent Provisioning SMB mode supported");

            dbPrintf("ROM Omega/Triton Reserved:                 0x%08x\n", r->t219.omega_triton_rsvd);
            
            dbPrintf("ROM Miscellaneous Support Feature Flags:   0x%08x\n", r->t219.misc_features_flags);
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_ICRU_SUPPORT,     (r->t219.misc_features_flags & SMBIOS_T219_ICRU_SUPPORT)?"Yes":"no", "iCRU BIOS Support");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_MEMBIST_RANK,     (r->t219.misc_features_flags & SMBIOS_T219_MEMBIST_RANK)?"Yes":"no", "MEMBIST Rank Margining Tool Supported");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_MEMBIST_HW_MEM,   (r->t219.misc_features_flags & SMBIOS_T219_MEMBIST_HW_MEM)?"Yes":"no", "MEMBIST HW Memory Test Supported");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_SABINE_CACHE,     (r->t219.misc_features_flags & SMBIOS_T219_SABINE_CACHE)?"Yes":"no", "Sabine Cache Module Supported");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_LIC_OD_FREQ_PLAT, (r->t219.misc_features_flags & SMBIOS_T219_LIC_OD_FREQ_PLAT)?"Yes":"no", "OD_FREQ_PLAT");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_LIC_OD_CORE_PLAT, (r->t219.misc_features_flags & SMBIOS_T219_LIC_OD_CORE_PLAT)?"Yes":"no", "OD_CORE_PLAT");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_LIC_OD_FREQ_CONF, (r->t219.misc_features_flags & SMBIOS_T219_LIC_OD_FREQ_CONF)?"Yes":"no", "OD_FREQ_CONF");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_LIC_OD_CORE_CONF, (r->t219.misc_features_flags & SMBIOS_T219_LIC_OD_CORE_CONF)?"Yes":"no", "OD_CORE_CONF");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_LIC_OD_FREQ_ON,   (r->t219.misc_features_flags & SMBIOS_T219_LIC_OD_FREQ_ON)?"Yes":"no", "u,OD_FREQ_ON");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_LIC_OD_CORE_ON,   (r->t219.misc_features_flags & SMBIOS_T219_LIC_OD_CORE_ON)?"Yes":"no", "OD_CORE_ON");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_CLASS_3_SYSTEM,   (r->t219.misc_features_flags & SMBIOS_T219_CLASS_3_SYSTEM)?"Yes":"no", "CLASS_3_SYSTEM (UEFI boot only)");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_DRIVE_CRYPT,      (r->t219.misc_features_flags & SMBIOS_T219_DRIVE_CRYPT)?"Yes":"no", "DRIVEE_CRYPT");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_CLASS_2_SYSTEM,   (r->t219.misc_features_flags & SMBIOS_T219_CLASS_2_SYSTEM)?"Yes":"no", "CLASS_2_SYSTEM (UEFI or Legacy boot)");
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_UEFI_DIAGS,       (r->t219.misc_features_flags & SMBIOS_T219_UEFI_DIAGS)?"Yes":"no", "UEFI_DIAGS");
            dbPrintf(" 0x%04x %3x %s\n", SMBIOS_T219_REST_MASK,        (r->t219.misc_features_flags & SMBIOS_T219_REST_MASK)>>14, "REST_MASK");
            
            dbPrintf("iLO Capabilities mask:                     0x%016lx\n", r->t219.ilo_cap);
            dbPrintf(" 0x%04x %3s %s\n", SMBIOS_T219_ILO_DRVCRYPT,     (r->t219.ilo_cap & SMBIOS_T219_ILO_DRVCRYPT)?"Yes":"no", "iLO Drive crypto");
            
            dbPrintf("Memory Features Flags:                     0x%08x\n", r->t219.mem_features_flags);

            break;
#if 0
        case 220:
            dbPrintf("Number of logical processors:\t%d", r->t220.n_lproc);
            if (r->t220.n_lproc) {
               dbPrintf("       APIC    physical  logical  logical\r\n"
                      " num    ID     CPU_hndl    core    thread");
            }
            for (i=0; i<r->t220.n_lproc; i++) {
                dbPrintf("[%2d]   0x%02x     0x%04x     0x%02x     0x%02x",
                      i, r->t220.proc_map[i].apicid, r->t220.proc_map[i].hndl_type_197,
                      r->t220.proc_map[i].core, r->t220.proc_map[i].threadNum);
            }
            break;

        case 221:
            if ((r->t221.nic[0].dev_fn)||(r->t221.nic[0].bus)) {
                dbPrintf("NIC\tdev_fn\tbus\tMAC");
            }
            /*lint -e661  out of bounds access possible (by design) See type 209 comments for background */
            for (i=0; (r->t221.nic[i].dev_fn)||(r->t221.nic[i].bus); i++) {
                printf ("%d\t0x%02x\t%d\t%02X-%02X-%02X-%02X-%02X-%02X",
                        i+1,
                        r->t221.nic[i].dev_fn,
                        r->t221.nic[i].bus,
                        r->t221.nic[i].mac[0], r->t221.nic[i].mac[1], r->t221.nic[i].mac[2],
                        r->t221.nic[i].mac[3], r->t221.nic[i].mac[4], r->t221.nic[i].mac[5] );
            }
            /*lint -restore */
            break;
#endif

        case 222:
            dbPrintf("GbX Table Version: %d\n", r->t222.gbx_tbl_ver);
            /* compute number of connector entries.
             * This is the size of the record (minus overhead and non-repeats)
             * divided by the size of each repeating entry */
            j = ((r->t222.hdr.len - sizeof(SMBIOS_HDR)) - sizeof(UINT16)) / 16;
            dbPrintf("%d GbX Connector Entries:\n", j);
            dbPrintf("       bay-1  bay-2  bay-3  bay-4  bay-5  bay-6  bay-7  bay-8");
            for (i=0; i<j; i++) {
               dbPrintf("[%2d]: %02x %02x  %02x %02x  %02x %02x  %02x %02x  "
                      "%02x %02x  %02x %02x  %02x %02x  %02x %02x\n", i,
                      r->t222.con[i].bay[0].mezz, r->t222.con[i].bay[0].swi,
                      r->t222.con[i].bay[1].mezz, r->t222.con[i].bay[1].swi,
                      r->t222.con[i].bay[2].mezz, r->t222.con[i].bay[2].swi,
                      r->t222.con[i].bay[3].mezz, r->t222.con[i].bay[3].swi,
                      r->t222.con[i].bay[4].mezz, r->t222.con[i].bay[4].swi,
                      r->t222.con[i].bay[5].mezz, r->t222.con[i].bay[5].swi,
                      r->t222.con[i].bay[6].mezz, r->t222.con[i].bay[6].swi,
                      r->t222.con[i].bay[7].mezz, r->t222.con[i].bay[7].swi
                     );
            }
            break;

        case 223:
            dbPrintf("Index:    0x%02x\n", r->t223.index);
            dbPrintf("Data:     0x%02x\n", r->t223.data);
            dbPrintf("Base:     0x%02x\n", r->t223.base);
            dbPrintf("Factory:  0x%02x\n", r->t223.factory);
            dbPrintf("Addr:     0x%02x\n", r->t223.addr);
            dbPrintf("Ctrl:     0x%02x\n", r->t223.ctrl);
            dbPrintf("Confg:    0x%02x\n", r->t223.cfg);
            break;
            
        case 224:
            /* consistent with HP ISS ProLiant Server SMBIOS OEM Extensions for UEFI Systems v3.36 */
            dbPrintf("  0x%02x: TPM Status\n", r->t224.status);
            switch (r->t224.status & 0x03) {
               case 0:  dbPrintf("        .... ..00: not present");              break;
               case 1:  dbPrintf("        .... ..01: present, enabled");         break;
               case 2:  dbPrintf("        .... ..10: present, disabled");        break;
               case 3:  dbPrintf("        .... ..11: reserved");                 break;
            }
            switch (r->t224.status & 0x04) {
               case 0:  dbPrintf("        .... .0..: No option ROM measuring");  break;
               case 4:  dbPrintf("        .... .1..: measure option ROMs");      break;
            }

            if (r->t224.hdr.len < 6) {
               dbPrintf("        Extended Status not implemented");
            } else {
               dbPrintf("  0x%02x: Extended Status\n", r->t224.ext_status);
               switch (r->t224.ext_status & 0x03) {
                  case 0:  dbPrintf("        .... ..00: Disable reason not specified");   break;
                  case 1:  dbPrintf("        .... ..01: Disable reason user disabled");   break;
                  case 2:  dbPrintf("        .... ..10: Disable reason error condition"); break;
                  case 3:  dbPrintf("        .... ..11: Disable reason reserved");        break;
               }
               switch (r->t224.ext_status & 0x3C) {
                  case 0:  dbPrintf("        ..00 00..: Error type not specified or no error");  break;
                  case 4:  dbPrintf("        ..00 01..: Self-Test error");                       break;
                  default: dbPrintf("        ..?? ??..: reserved err");                          break;
               }
            }

            if (r->t224.hdr.len < 7) {
               dbPrintf("        Trusted Module Type not implemented");
            } else {
               //HPE Proliant Server SMBIOS Extensions for UEFI Systems v3.54
               dbPrintf("  0x%02x: Trusted Module Type\n", r->t224.module_type);
               switch (r->t224.module_type & 0x0F) {
                  case 0:  dbPrintf("        .... 0000: Not Specified");  break;
                  case 1:  dbPrintf("        .... 0001: TPM 1.2");        break;
                  case 2:  dbPrintf("        .... 0010: TPM 2.0");        break;
                  case 3:  dbPrintf("        .... 0011: Intel PTT fTPM"); break;
                  default: dbPrintf("        .... ????: unknown");        break;
               }
               switch (r->t224.module_type & 0x10) {
                  case    0:  dbPrintf("        ...0 ....: Standard Algorithms not specified"); break;
                  case 0x10:  dbPrintf("        ...1 ....: Standard Algorithms support");       break;
               }
               switch (r->t224.module_type & 0x20) {
                  case    0:  dbPrintf("        ..0. ....: Chinese Algorighms not specified");  break;
                  case 0x20:  dbPrintf("        ..1. ....: Chinese Algorithms support");        break;
               }
            }

            if (r->t224.hdr.len < 8) {
               dbPrintf("        Trusted Module Attributes not implemented");
            } else {
               dbPrintf("  0x%02x: Trusted Module Attributes\n", r->t224.module_attributes);
               switch (r->t224.module_attributes & 0x03) {
                  case 0:  dbPrintf("        .... ..00: Not Specified");          break;
                  case 1:  dbPrintf("        .... ..01: Pluggable and Optional"); break;
                  case 2:  dbPrintf("        .... ..10: Pluggable and Standard"); break;
                  case 3:  dbPrintf("        .... ..11: Soldered Down");          break;
               }
               switch (r->t224.module_attributes & 0x0C) {
                  case 0x00:  dbPrintf("        .... 00..: Not Specified");       break;
                  case 0x04:  dbPrintf("        .... 01..: Not FIPS certified");  break;
                  case 0x08:  dbPrintf("        .... 10..: FIPS certified");      break;
                  case 0x0C:  dbPrintf("        .... 11..: reserved");            break;
               }
            }

            if (r->t224.hdr.len < 10) {
               dbPrintf("        Trusted Module FW Version handle not implemented");
            } else {
               dbPrintf("0x%04x: Trusted Module FW Version Handle\n", r->t224.hndl_type_216);
               if (r->t224.hdr.len >= 12) { //HPE Proliant Server SMBIOS Extensions for UEFI Systems v3.54
                  dbPrintf("0x%04x: Chip Identifier Value\n", r->t224.chip_vendor_id);
                  switch (r->t224.chip_vendor_id & 0x0003) {
                     case 0:  dbPrintf("        .... ..00: None");                break;
                     case 1:  dbPrintf("        .... ..01: STMicro Gen10 TPM");   break;
                     case 2:  dbPrintf("        .... ..10: Intel firmware TPM");  break;
                     case 3:  dbPrintf("        .... ..11: Nationz TPM");         break;
                  }
               }
            }
            break;

#if 0
        case 225:
            dbPrintf("0x%08x: SLP Activation Marker Locator\n", r->t225.marker);
            dbPrintf("0x%08x: SLP Enable/Disable Flag Locator\n", r->t225.flag);
            break;
#endif           
        case 226:
            /* note that the byte ordering is consistent with DMTF SMBIOS 2.6a spec */
            sprintf(buf, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                  r->t226.uuid[3], r->t226.uuid[2], r->t226.uuid[1], r->t226.uuid[0],
                  r->t226.uuid[5], r->t226.uuid[4],
                  r->t226.uuid[7], r->t226.uuid[6],
                  r->t226.uuid[8], r->t226.uuid[9],
                  r->t226.uuid[10], r->t226.uuid[11], r->t226.uuid[12], r->t226.uuid[13], r->t226.uuid[14], r->t226.uuid[15]);
            dbPrintf("UUID:           \"%s\"  \n", buf);

            /* check for special cases */
            memcpy(buf, r->t226.uuid, 16);
            buf[16]=0;
            /* UUID attributes:
             * Universal Unique ID number.
             * If the value is all FFh,
             *  the ID is not currently present in the system, but is settable.
             * If the value is all 00h, the ID is not present in the system.
             */
            // scan for all FFs
            for (i=0;i<16;i++) {
                if (buf[i]!=(char)0xff) break;
            }
            if (i==16) { // all FFs
               dbPrintf(">> not set <<");
            } else {
                // scan for all 0s
                for (i=0;i<16;i++) {
                    if (buf[i]!=0) {
                       dbPrintf("%s", "");
                       break;
                    }
                }
                if (i==16) { // all 0s
                   dbPrintf(">> not present <<\n");
                }
            }
            dbPrintf("Serial Number: \"%s\"\n", smbios_rec_s(p, r->t226.sid_sn));
            break;

        case 227:
            sprintf(label, "%d:%s\n", r->t227.type,
                  (r->t227.type==SMBIOS_T227_MCP98242)?"microchip":
                  (r->t227.type==SMBIOS_T227_MILLBROOK)?"millbrook":
                  (r->t227.type==SMBIOS_T227_JORDANCREEK)?"jordancreek": "\?\?");

            /* structure expansion - some fields are only populated when the structure
             * includes the channel */
            dbPrintf("%-9s %-9s %-3s %-4s %-13s %-5s %-3s %-3s %-2s %6s %-4s %5s %5s %5s\n",
                            "CPU_hnd", "DIMM_hnd", "seg", "addr", "type", "group", "bus", "dev", "fn", "reg", "chan", "flags", "uAdr", "type");
               dbPrintf(" 0x%04x    0x%04x  0x%02x 0x%02x %-13s %3d   %3d %3d %2d 0x%04x %4d  %04x    %02x  %04x\n",
                   r->t227.hndl_type_4,
                   r->t227.hndl_type_17,
                   r->t227.seg, 
                   r->t227.addr,
                   label,
                   r->t227.group,
                   r->t227.bus,
                   (r->t227.dfn & 0xf8) >> 3, // device
                   r->t227.dfn & 0x7,         // function
                   r->t227.reg,
                   r->t227.chan,
                   r->t227.nvdimm_flags,
                   r->t227.nvdimm_mc_addr,
                   r->t227.nvdimm_mc_type);
            break;
 
        case 228:
            switch (r->t228.mux.type) {
               case 0: // Root
                  dbPrintf("  seg  [root: addr  offs  mask  set   res]  parent info  res\r\n"
                         " 0x%02x         0x%02x  0x%02x  0x%02x  0x%02x  0x%02x   0x%02x   0x%02x  0x%02x",
                         r->t228.seg,
                         r->t228.mux.select.i2c.addr,
                         r->t228.mux.select.i2c.offs,
                         r->t228.mux.select.i2c.mask_clr,
                         r->t228.mux.select.i2c.mask_set,
                         r->t228.mux.select.i2c.res,
                         r->t228.parent,
                         r->t228.info,
                         r->t228.res);
                  break;
               case SMBIOS_T228_TYPE_CPLD: // XRegister-controlled MUX
                  dbPrintf("  seg  [XReg: byte  mask  set   res0  res1] parent info  res\r\n"
                         " 0x%02x         0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x    0x%02x  0x%02x",
                         r->t228.seg,
                         r->t228.mux.select.cpld.xreg,
                         r->t228.mux.select.cpld.mask_clr,
                         r->t228.mux.select.cpld.mask_set,
                         r->t228.mux.select.cpld.res[0],
                         r->t228.mux.select.cpld.res[1],
                         r->t228.parent,
                         r->t228.info,
                         r->t228.res);
                  break;
               case SMBIOS_T228_TYPE_I2C: // I2C inline MUX
                  dbPrintf("  seg  [I2C: addr  offs  mask  set   res]  parent info  res\r\n"
                         " 0x%02x        0x%02x  0x%02x  0x%02x  0x%02x  0x%02x   0x%02x    0x%02x  0x%02x",
                         r->t228.seg,
                         r->t228.mux.select.i2c.addr,
                         r->t228.mux.select.i2c.offs,
                         r->t228.mux.select.i2c.mask_clr,
                         r->t228.mux.select.i2c.mask_set,
                         r->t228.mux.select.i2c.res,
                         r->t228.parent,
                         r->t228.info,
                         r->t228.res);
                  break;
               case SMBIOS_T228_TYPE_GPO: // GPO-controlled MUX
                  dbPrintf("  seg  [GPO: byte  mask  set   res0  res1] parent info  res\r\n"
                         " 0x%02x        0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x    0x%02x  0x%02x",
                         r->t228.seg,
                         r->t228.mux.select.gpo.byte,
                         r->t228.mux.select.gpo.mask_clr,
                         r->t228.mux.select.gpo.mask_set,
                         r->t228.mux.select.gpo.res[0],
                         r->t228.mux.select.gpo.res[1],
                         r->t228.parent,
                         r->t228.info,
                         r->t228.res);
                  break;
               case SMBIOS_T228_TYPE_SCHAIN: // MID Scan Chain
                  dbPrintf("  seg  [MID: byte  mask  set   res0  res1] parent info  res\r\n"
                         " 0x%02x        0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x    0x%02x  0x%02x",
                         r->t228.seg,
                         r->t228.mux.select.gpo.byte,
                         r->t228.mux.select.gpo.mask_clr,
                         r->t228.mux.select.gpo.mask_set,
                         r->t228.mux.select.gpo.res[0],
                         r->t228.mux.select.gpo.res[1],
                         r->t228.parent,
                         r->t228.info,
                         r->t228.res);
                  break;
               case SMBIOS_T228_TYPE_PECI: // MUX behind PECI
                  dbPrintf("  seg  [PECI: addr  offs  mask   set   dev ] parent info  res\r\n"
                         " 0x%02x         0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x   0x%02x  0x%02x",
                         r->t228.seg,
                         r->t228.mux.select.peci.mux_addr,
                         r->t228.mux.select.peci.mux_offs,
                         r->t228.mux.select.peci.mask_clr,
                         r->t228.mux.select.peci.mask_set,
                         r->t228.mux.select.peci.dev_addr,
                         r->t228.parent,
                         r->t228.info,
                         r->t228.res);
                  break;
               case SMBIOS_T228_TYPE_PECI_I2C: // I2C segment behind PECI
                  dbPrintf("  seg  [PECI_I2C: addr  offs  mask  set   dev ] parent info  res\r\n"
                         " 0x%02x             0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x   0x%02x  0x%02x",
                         r->t228.seg,
                         r->t228.mux.select.peci.mux_addr,
                         r->t228.mux.select.peci.mux_offs,
                         r->t228.mux.select.peci.mask_clr,
                         r->t228.mux.select.peci.mask_set,
                         r->t228.mux.select.peci.dev_addr,
                         r->t228.parent,
                         r->t228.info,
                         r->t228.res);
                  break;
               case SMBIOS_T228_TYPE_I2CEX: // I2C I/O expander on a different segment
                  dbPrintf("  seg  [I2CEX: seg   addr  offs  mask  set ] parent info  res\r\n"
                         " 0x%02x          0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x    0x%02x  0x%02x",
                         r->t228.seg,
                         r->t228.mux.select.i2cex.seg,
                         r->t228.mux.select.i2cex.addr,
                         r->t228.mux.select.i2cex.offs,
                         r->t228.mux.select.i2cex.mask_clr,
                         r->t228.mux.select.i2cex.mask_set,
                         r->t228.parent,
                         r->t228.info,
                         r->t228.res);
                  break;
               case SMBIOS_T228_TYPE_PECI_G8:  // Gen 8 PECI to SMBUS controllers within a domain
                  dbPrintf("  seg  [PECI2: byt0  byt1  byt2  byt3  byt4] parent info  res\r\n"
                         " 0x%02x        0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x    0x%02x  0x%02x",
                         r->t228.seg,
                         r->t228.mux.select.unknown.byt[0],
                         r->t228.mux.select.unknown.byt[1],
                         r->t228.mux.select.unknown.byt[2],
                         r->t228.mux.select.unknown.byt[3],
                         r->t228.mux.select.unknown.byt[4],
                         r->t228.parent,
                         r->t228.info,
                         r->t228.res);
                  break;
               case SMBIOS_T228_TYPE_CARB:  // Carbondale-arbitrated I2C in SL enclosure
                  dbPrintf("  seg  [CARB:  byt0  byt1  byt2  byt3  byt4] parent info  res\r\n"
                         " 0x%02x          0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x    0x%02x  0x%02x",
                         r->t228.seg,
                         r->t228.mux.select.unknown.byt[0],
                         r->t228.mux.select.unknown.byt[1],
                         r->t228.mux.select.unknown.byt[2],
                         r->t228.mux.select.unknown.byt[3],
                         r->t228.mux.select.unknown.byt[4],
                         r->t228.parent,
                         r->t228.info,
                         r->t228.res);
                  break;
               default:
                  dbPrintf("Unknown MUX control type 0x%x:", r->t228.mux.type);
                  dbPrintf("  seg  [ ! : byt0  byt1  byt2  byt3  byt4] parent info  res\r\n"
                         " 0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x    0x%02x  0x%02x",
                         r->t228.seg,
                         r->t228.mux.type,
                         r->t228.mux.select.unknown.byt[0],
                         r->t228.mux.select.unknown.byt[1],
                         r->t228.mux.select.unknown.byt[2],
                         r->t228.mux.select.unknown.byt[3],
                         r->t228.mux.select.unknown.byt[4],
                         r->t228.parent,
                         r->t228.info,
                         r->t228.res);
            }
            dbPrintf("%s", "");
            break;
            
        case 229:
            /* compute number of connector entries.
             * This is the size of the record (minus overhead and non-repeats)
             * divided by the size of each repeating entry */
            j = (r->t229.hdr.len - sizeof(SMBIOS_HDR)) / 16;
            dbPrintf("%d Reserved Memory Ranges:\n", j);
            dbPrintf("       Signature              Address  Size\n");
            for (i=0; i<j; i++) {
               dbPrintf("[%2d]: 0x%08x  0x%08x.%08x  0x%08x\n", i,
                      r->t229.entry[i].sig,
                      r->t229.entry[i].loc[1], r->t229.entry[i].loc[0],
                      r->t229.entry[i].size);
            }
            break;
            
        case 230:
            dbPrintf("P/S handle:   0x%04x\n", r->t230.hndl_39);
            dbPrintf("Manufacturer:\"%s\"\n", smbios_rec_s(p, r->t230.sid_manu));
            dbPrintf("Revision:    \"%s\"\n", smbios_rec_s(p, r->t230.sid_rev));
            dbPrintf("FRU:          %d (%s)\n", r->t230.fru, (r->t230.fru==SMBIOS_T230_FRU_NA)?"N/A":
                                                                   (r->t230.fru==SMBIOS_T230_FRU_IPMI)?"IPMI I2C":
                                                                   (r->t230.fru==SMBIOS_T230_FRU_ILO)?"iLO":"!?");
            dbPrintf("Segment:      %d\n", r->t230.bus);
            dbPrintf("Address:      0x%02x\n", r->t230.addr);
            break;

#if 0
        case 231:
            dbPrintf("       0x%02x: bios type\n",  r->t231.bios_type);
            dbPrintf("       0x%02x: spi\n",        r->t231.spi);
            dbPrintf("     0x%04x: sz_rom\n",       r->t231.sz_rom);
            dbPrintf("     0x%04x: sz_img\n",       r->t231.sz_img);
            dbPrintf("     0x%04x: sz_nonbios\n",   r->t231.sz_nonbios);
            dbPrintf("     0x%04x: sz_updatable\n", r->t231.sz_updatable);
            dbPrintf("     0x%04x: sz_bblk\n",      r->t231.sz_bblk);
            dbPrintf("     0x%04x: ver_me\n",       r->t231.ver_me);
            dbPrintf("       0x%02x: regions\n",    r->t231.regions);
            dbPrintf(" 0x%08x: mpm_settings\n",     r->t231.mpm_settings);
            dbPrintf(" 0x%08x: mpm_int15\n",        r->t231.mpm_int15);
            break;
#endif
        case 232:
            dbPrintf("DIMM handle:       0x%04x\n",  r->t232.hndl_17);
            dbPrintf("Attributes:        0x%08x\n",  r->t232.att);
            dbPrintf("min Voltage:       %d mv\n",   r->t232.v_min);
            dbPrintf("cfg Voltage:       %d mv\n",   r->t232.v_cfg);
            dbPrintf("modes:             0x%04x\n",  r->t232.mode);
            dbPrintf("cfg mode:          0x%04x\n",  r->t232.cfg);
            dbPrintf("Increment Sz:      0x%04x\n",  r->t232.inc_sz);
            dbPrintf("Alignment Sz:      0x%04x\n",  r->t232.align_sz);
            dbPrintf("Region Cnt:        0x%04x\n",  r->t232.region_cnt);
            dbPrintf("Region Lim (Pers): 0x%04x\n",  r->t232.region_limit_pers);
            dbPrintf("Region Lim (Vol):  0x%04x\n",  r->t232.region_limit_vol);
            dbPrintf("Security State:    0x%04x\n",  r->t232.security_st);
            dbPrintf("Map Out:           0x%02x\n",  r->t232.map_out);
            dbPrintf("Encryption Status: %d:%s\n", r->t232.encrypt_status,
                (r->t232.encrypt_status == T232_DIMM_NOT_ENCRYPTED)?"Not Encrypted":
                (r->t232.encrypt_status == T232_DIMM_ENCRYPTED)?"Encrypted":
                (r->t232.encrypt_status == T232_DIMM_ENCRYPTED_STATUS_UNKNOWN)?"Encryption Status Unknown":
                (r->t232.encrypt_status == T232_DIMM_ENCRYPTION_UNSUPPORTED)?"Encryption Unsupported":"?");
            break;

        case 233:
            dbPrintf("Segment Group Number:   0x%04x\n",   r->t233.seg_grp_num);
            dbPrintf("Bus Number:             0x%02x\n",   r->t233.bus_num);
            dbPrintf("Device/Function Number: 0x%02x\n",   r->t233.dev_fn_num);
            hexdump_b_hind(r->t233.mac_addr, sizeof(r->t233.mac_addr), (char*)"MAC Address:            ");
            dbPrintf("Port Number:            0x%02x\n",   r->t233.port);
            break;

        case 234:
            dbPrintf("Type 228 handle: 0x%04x\n",   r->t234.hndl_228);
            dbPrintf("I2C address:     0x%02x\n",   r->t234.i2c_addr);
            dbPrintf("Device Type:     %d:%s\n",    r->t234.dev_type,
                  (r->t234.dev_type == 0)?"Megacell":
                  (r->t234.dev_type == 1)?"eFuse":
                  (r->t234.dev_type == 2)?"Quad Comparator":
                  (r->t234.dev_type == 3)?"VRD":
                  (r->t234.dev_type == 4)?"Multiphase VRD":
                  (r->t234.dev_type == 5)?"backplane PIC":"??");
            switch (r->t234.dev_type) {
               case T234_DEV_MC:
                  dbPrintf("version:       0x%02x\n", r->t234.dev_data.mc_data.ver);
                  dbPrintf("FRU I2C:       0x%02x\n", r->t234.dev_data.mc_data.fru_i2c_addr);
                  dbPrintf("Load Capacity: 0x%04x\n", r->t234.dev_data.mc_data.load_cap);
                  dbPrintf("Total Loads:   0x%04x\n", r->t234.dev_data.mc_data.tot_req);
                  dbPrintf("Info:          0x%02x\n", r->t234.dev_data.mc_data.info);
                  break;
               case T234_DEV_EFUSE:
               case T234_DEV_Q_COMP:
               case T234_DEV_VRD:
               case T234_DEV_MULTI_VRD:
                  dbPrintf("PMBUS version:     0x%02x\n", r->t234.dev_data.pmbus_data.ver);
                  dbPrintf("Fault Table Class: 0x%02x\n", r->t234.dev_data.pmbus_data.flt_table_class);
                  dbPrintf("Fault Table Byte:  0x%02x\n", r->t234.dev_data.pmbus_data.flt_table_byte);
                  dbPrintf("Fault Table Bit:   0x%02x\n", r->t234.dev_data.pmbus_data.flt_table_bit);
                  break;
               default:
                  hexdump_b_hind(&(r->t234.dev_data), sizeof(r->t234.dev_data), (char*)"Data               ");
            }
            break;

        case 236:
            dbPrintf("Backplane FRU I2C address: 0x%02x\n", r->t236.bp_fru_i2c_addr);
            dbPrintf("Backplane Box Number:      0x%04x\n", r->t236.bp_box_num);
            dbPrintf("Backplane Box NVRAM ID:    0x%04x\n", r->t236.bp_nvram_id);
            hexdump_b_hind(&(r->t236.sas_expander_wwid), sizeof(r->t236.sas_expander_wwid), (char*)"SAS Expander WWID          ");
            dbPrintf("Total SAS Bays:            0x%02x\n", r->t236.total_sas_bays);
            dbPrintf("Port 0xA0 SAS Bay Count:   0x%02x (deprecated)\n", r->t236.port0xA0_sas_bay_count);
            dbPrintf("Port 0xA2 SAS Bay Count:   0x%02x (deprecated)\n", r->t236.port0xA2_sas_bay_count);
            dbPrintf("Backplane Name:           \"%s\" (deprecated)\n",  smbios_rec_s(p, r->t236.sid_bp_name));
            break;

        case 237:
        {
            unsigned char   buffer[256];
            type_17          *t17 = NULL;

            t17 = (type_17 *)buffer;
            rc = smbios_get_rec_by_handle(r->t237.t17_hndl, buffer, sizeof(buffer));
            dbPrintf("type 17 handle:      0x%04x\n", r->t237.t17_hndl);

            rc = smbios_get_rec_by_handle(r->t237.t17_hndl, buffer, sizeof(buffer));
            if (rc == 0) {
                dbPrintf(" Location:           %s (%s)\n", smbios_rec_s(buffer, t17->sid_device), ((t17->size || t17->extended_size) ? ("Occupied") : ("Empty")));
            } else {
                dbPrintf(" Location:           %s (%s)\n", "??", "invalid");
            }
            dbPrintf("DIMM Manufacturer:   %s\n", smbios_rec_s(p, r->t237.sid_mfg));
            dbPrintf("DIMM Part Number:    %s\n", smbios_rec_s(p, r->t237.sid_part));
            dbPrintf("DIMM Vendor SerNum:  %s\n", smbios_rec_s(p, r->t237.sid_vsn));
            dbPrintf("DIMM Mfg Date:     0x%04X Week %X of year 20%02X\n", r->t237.mfg_date, r->t237.mfg_date%0xFF, r->t237.mfg_date&0xFF00>>8);
        }
        break;

        case 238:
           dbPrintf("type 8 handle:        0x%04x\n",     r->t238.t8_hndl);
           dbPrintf("Host ctrl B/D/Fn:       %d/%d/%d\n", r->t238.bus, (r->t238.dev_fn>>3), (r->t238.dev_fn & 0x7));
           dbPrintf("Location:               %d:%s\n",    r->t238.loc, r->t238.loc==0?"internal":r->t238.loc==1?"front":
                 r->t238.loc==2?"rear":r->t238.loc==3?"embedded SD-card":r->t238.loc==4?"iLO USB":r->t238.loc==5?"NAND (USX 2065)":r->t238.loc==7?"Debug":r->t238.loc==9?"OCP":"?");
           dbPrintf("Flags:                0x%04x\n",     r->t238.flags);
           dbPrintf("Instance:               %d\n",       r->t238.inst);
           dbPrintf("Hub:                    %d\n",       r->t238.hub);
           dbPrintf("Speed:                  %d:%s\n",    r->t238.speed, (r->t238.speed==1)?"1.1":r->t238.speed==2?"2.0":r->t238.speed==3?"3.0":"??");
           dbPrintf("Path:                   %s\n",       smbios_rec_s(p, r->t238.sid_path));
           break;

        case 239:
           dbPrintf("type 238 handle:        0x%04x\n",   r->t239.t238_hndl);
           dbPrintf("Vendor ID:              0x%04x\n",   r->t239.vendor);
           dbPrintf("Flags:                  0x%04x\n",   r->t239.flags);
           dbPrintf("USB Device Class code:     %d\n",    r->t239.d_class);
           dbPrintf("USB Device SubClass code:  %d\n",    r->t239.d_sub);
           dbPrintf("USB Device Protocol code:  %d\n",    r->t239.d_prot);
           dbPrintf("Product ID:             0x%04x\n",   r->t239.prodid);
           dbPrintf("Capacity (MB):          %u\n",       r->t239.capacity);
           dbPrintf("UEFI Device Path:       %s\n",       smbios_rec_s(p, r->t239.sid_path));
           dbPrintf("UEFI Device StructName: %s\n",       smbios_rec_s(p, r->t239.sid_sname));
           dbPrintf("UEFI Device Name:       %s\n",       smbios_rec_s(p, r->t239.sid_name));
           dbPrintf("UEFI Device Location:   %s\n",       smbios_rec_s(p, r->t239.sid_loc));
           break;

        case 240:
           dbPrintf("associated handle:    0x%04x\n",      r->t240.hndl_assoc); /* type 202, 203, 224 */
           dbPrintf("Version:                %d\n",        r->t240.ver);
           dbPrintf("Version String:         %s\n",        smbios_rec_s(p, r->t240.sid_fw_ver));
           dbPrintf("Size:                 0x%016lx %016lx\n", r->t240.sz>>32, r->t240.sz&0xFFFFFFFF);
           dbPrintf("Defined Attributes:   0x%08x %08x\n", r->t240.att_def[1], r->t240.att_def[0]);
           dbPrintf("Effective Attributes: 0x%08x %08x\n", r->t240.att_val[1], r->t240.att_val[0]);
           dbPrintf("Minimum:                %d\n",        r->t240.min);
           break;

        case 242:
           dbPrintf("associated handle:    0x%04x\n",      r->t242.hndl_assoc);
           dbPrintf("type:                   %d:%s\n",     r->t242.type,
                                                                (r->t242.type==T242_TYPE_UNKNOWN)?"undetermined":
                                                                (r->t242.type==T242_TYPE_NVME)?"NVMe":
                                                                (r->t242.type==T242_TYPE_SATA)?"SATA":
                                                                (r->t242.type==T242_TYPE_SAS)?"SAS":
                                                                (r->t242.type==T242_TYPE_SSD)?"SSD":"?");
           dbPrintf("Unique Identifier:    0x%016lx %016lx\n", r->t242.uid>>32, r->t242.uid&0xFFFFFFFF);
           dbPrintf("Capacity:               %d MB\n",     r->t242.cap);
           hexdump_b_hind(r->t242.poh, sizeof(r->t242.poh), (char*)"Power On Hours:         ");
           dbPrintf("Percentage Used:        %d:(deprecated)\n",       r->t242.used);
           dbPrintf("Wattage:                %d\n",        r->t242.watts);
           dbPrintf("Form Factor:            %d:%s\n",     r->t242.ff,
                                                                (r->t242.ff == T242_FF_3_5)?"3.5\"":
                                                                (r->t242.ff == T242_FF_2_5)?"2.5\"":
                                                                (r->t242.ff == T242_FF_1_8)?"1.8\"":
                                                                (r->t242.ff == T242_FF_LESS_1_8)?"<1.8\"":
                                                                (r->t242.ff == T242_FF_MSATA)?"mSATA":
                                                                (r->t242.ff == T242_FF_M2)?"m.2":
                                                                (r->t242.ff == T242_FF_MICRO_SSD)?"microSSD":
                                                                (r->t242.ff == T242_FF_CFAST)?"CFast":"?");
           dbPrintf("Health:                 %d:%s\n",     r->t242.health, 
                                                                (r->t242.health == T242_HEALTH_OK)?"OK":
                                                                (r->t242.health == T242_HEALTH_WARNING)?"warning":
                                                                (r->t242.health == T242_HEALTH_CRITICAL)?"critical":
                                                                (r->t242.health == T242_HEALTH_UNKNOWN)?"Unknown":"?");
           dbPrintf("Serial Number:          %s\n",        smbios_rec_s(p, r->t242.sid_sn));
           dbPrintf("Model Number:           %s\n",        smbios_rec_s(p, r->t242.sid_model));
           dbPrintf("FW Revision:            %s\n",        smbios_rec_s(p, r->t242.sid_rev));
           if (r->t242.hdr.len > VER_242_PRE_3_63)
           {
               dbPrintf("Hard Drive Location:    %s\n", smbios_rec_s(p, r->t242.ver_3_63.sid_loc));
               if (r->t242.hdr.len > VER_242_PRE_3_67)
               {
                   dbPrintf("Encryption Status:      %d:%s\n", r->t242.ver_3_67.encrypt_status,
                       (r->t242.ver_3_67.encrypt_status == T242_NOT_ENCRYPTED) ? "Not Encrypted" :
                        (r->t242.ver_3_67.encrypt_status == T242_ENCRYPTED) ? "Encrypted" :
                        (r->t242.ver_3_67.encrypt_status == T242_ENCRYPTION_STATUS_UNKNOWN) ? "Encryption Status Unknown" :
                        (r->t242.ver_3_67.encrypt_status == T242_ENCRYPTION_NOT_SUPPORTED) ? "Encryption Not Supported" : "?");
                    if (r->t242.hdr.len > VER_242_PRE_3_75)
                    {
                        dbPrintf("Device Capacity Bytes:  %lu Bytes\n",    r->t242.ver_3_75.capacity_bytes);
                        dbPrintf("Block Size:             %d\n",            r->t242.ver_3_75.block_size);
                        dbPrintf("Rotational Speed RPM:   %d RPM\n",        r->t242.ver_3_75.rotational_speed);
                        dbPrintf("Negotiated Speed:       %ld Gbit/s\n",     r->t242.ver_3_75.capacity_bytes);
                        dbPrintf("Capable Speed:          %ld Gbit/s\n",     r->t242.ver_3_75.capacity_bytes);
                    }
               }
           }
           break;
        case 243:
           dbPrintf("Associated handle:    0x%04x\n",      r->t243.t216_hndl);
           db_guid_to_text(r->t243.device_guid, (char*)"Device GUID");
           db_guid_to_text(r->t243.target_guid, (char*)"Target GUID");
           break;
        case 244:
           dbPrintf("Type 17 Handle:    0x%04x\n",      r->t244.t17_hndl);
           dbPrintf("Region ID:         %d    \n",      r->t244.region_id);
           dbPrintf("Region Type:       0x%04x\n",      r->t244.region_config);
           dbPrintf("Region Size:       %ld MiB\n",    r->t244.region_mem_sz);  /// Print 64 byte value in hex
           dbPrintf("Passphrase State:  0x%04x\n",      r->t244.passphrase_st);
           dbPrintf("Interleave Index   %d\n",          r->t244.interleave_idx);
           if (r->t244.hdr.len > VER_244_PRE_3_62)
           {
               dbPrintf("Interleave Count:  %d\n", r->t244.ver_3_62.interleave_cnt);
               dbPrintf("Interleave Health: %d:%s\n", r->t244.ver_3_62.interleave_health,
                   (r->t244.ver_3_62.interleave_health == T244_INT_HEALTH_HEALTHY) ? "Healthy" :
                    (r->t244.ver_3_62.interleave_health == T244_INT_HEALTH_DIMM_MISSING) ? "DIMM Missing" :
                    (r->t244.ver_3_62.interleave_health == T244_INT_HEALTH_CFG_INACTIVE) ? "Config Inactive" :
                    (r->t244.ver_3_62.interleave_health == T244_INT_HEALTH_SPA_MISSING) ? "SPA Missing" :
                    (r->t244.ver_3_62.interleave_health == T244_INT_HEALTH_NEW_GOAL) ? "New Goal" :
                    (r->t244.ver_3_62.interleave_health == T244_INT_HEALTH_LOCKED) ? "Locked" : "?");
           }
           break;
        case 245:
           dbPrintf("EXT Board Type:   0x%02x:%s\n",     r->t245.ext_board_type,
           (r->t245.ext_board_type == T245_EXT_BOARD_TYPE_PCIE_RISER)?"PCIe Riser":"?");
           if(r->t245.ext_board_type == T245_EXT_BOARD_TYPE_PCIE_RISER)
           {
                dbPrintf("Riser Postion:    0x%02x:%s\n", r->t245.member_union.pcie_riser.riser_position,
                (r->t245.member_union.pcie_riser.riser_position == T245_EXT_BOARD_TYPE_PCIE_RISER_PRIMARY)?"Primary":
                (r->t245.member_union.pcie_riser.riser_position == T245_EXT_BOARD_TYPE_PCIE_RISER_SECONDARY)?"Secondary":
                (r->t245.member_union.pcie_riser.riser_position == T245_EXT_BOARD_TYPE_PCIE_RISER_TERTIARY)?"Tertiary":
                (r->t245.member_union.pcie_riser.riser_position == T245_EXT_BOARD_TYPE_PCIE_RISER_QUATERNARY)?"Quaternary":
                (r->t245.member_union.pcie_riser.riser_position == T245_EXT_BOARD_TYPE_PCIE_RISER_FRONT)?"Front":"?");
                
                dbPrintf("Riser ID:         0x%02x\n",r->t245.member_union.pcie_riser.riser_id);
                dbPrintf("Riser Version:    0x%02x\n",r->t245.member_union.pcie_riser.riser_version);
                dbPrintf("Riser Name:       %s\n",       smbios_rec_s(p, r->t245.member_union.pcie_riser.riser_index));
           }
           break;
//#endif

        default:
            dbPrintf("Raw data:");
            smbios_rec_dump(p);
    }
    return(0);
}



/* db_smbios_help() telbug high-level help text */
void db_smbios_help(void)
{
   dbPrintf("SMBIOS INDEX        Received records\r\n"
                    "SMBIOS ALL          Decode all stored records\r\n"
                    "SMBIOS TYPES        Known types\r\n"
                    "SMBIOS STAT         Received records statistics\r\n"
                    "SMBIOS MAP          I2C/PECI segments and multiplexers map\r\n"
                    "SMBIOS MB           Millbrooks summary data\r\n"
                    "SMBIOS DIMM         DIMM summary data\r\n"
                    "SMBIOS TYPE <type>  Dump records of type <type>\r\n"
                    "SMBIOS HANDLE <hnd> Dump record with handle <hnd>\r\n");
   
   return;
}

/* db_smbios_handler()
 *
 * Telbug command handler registered for "SMBIOS"
 * argv[0] == "SMBIOS"      && argc >=1
 * argv[1] == "command..."  && argc >=2
 * argv[3] == "1"           && argc >=3
 */
void db_smbios_handler( int argc, char *argv[] ) 
{
   int i, n;
   int h=0, t=0, p=0, c=0; // handle, type, position, command
   UINT8  buf[SMBIOS_MAX_RECSIZE];
//   UINT8  b2[64];
//   UINT8  b3[64];
//   UINT8  v8, vv8;
//   UINT16 v16;
//   UINT32 v32;
//   SMBIOS_proc_info pi;
//   SMBIOS_slot_info si;

   SMBIOS_HDR* hdr = (SMBIOS_HDR *)buf;
   type_dimm d;
#define N_MUX (64)  //see also I2C_MUX and SMBIOS_NUM_MUX_SEGMENTS
   struct {
      type_228  t228;
      UINT8 buf[16];
   } mux[N_MUX];
   
   memset(buf, 0, sizeof(buf));

   if ((argc < 2) || (!strutil_icmp(argv[1], "HELP")) || (!strutil_icmp(argv[1], "?"))) {
      db_smbios_help();
      return;
   }

   if (!strutil_icmp(argv[1], "ALL")) {
      dbPrintf("\r\nAll Stored SMBIOS Records");
      for (t=0; t<255; t++) { // max type 255
         i = 0; // reset instance number each new type
         while ( (!(smbios_get_rec_by_type_and_position( (UINT8)t, i, buf, sizeof(buf)))) && (hdr->len) ) {
            smbios_rec_decode(buf);
            i++;
         }
      }
      return;
   }
   if (!strutil_icmp(argv[1], (char*)"INDEX")) {
      dbPrintf("Index of stored SMBIOS records\r\n"
                       "NUM type len handle description");
      c = 0; // retain count of records
      for (t=0; t<255; t++) { // max type 255
         i = 0; // reset instance number each new type
         while ( (!(smbios_get_rec_by_type_and_position( (UINT8)t, i, buf, sizeof(buf)))) && (hdr->len) ) {
            c++;
            dbPrintf("%3d: %3d 0x%02x 0x%04x \"%s\"\n", c,
                  hdr->type, hdr->len, hdr->handle, smbios_type_string(hdr->type));
            i++;
         }
      }
      dbPrintf("Listed %d entries\n\n", c);
      return;
   }
   if (!strutil_icmp(argv[1], (char*)"TYPES")) {
      for (i=0;i<255; i++) {
         if (strutil_icmp(smbios_type_string(i), (char*)"UNKNOWN")) {
            dbPrintf("%3d: %s\n\n", i, smbios_type_string(i));
         }
      }
      return;
   }

   if ((!strutil_icmp(argv[1], "MUX")) || (!strutil_icmp(argv[1], (char*)"MAP"))) {
      memset(mux, 0, sizeof(mux));
      n = 0;
      while ((n<N_MUX) &&
             (!(smbios_get_rec_by_type_and_position( 228, n, (UINT8 *)&mux[n], sizeof(mux[0])))) &&
             (mux[n].t228.hdr.len)) {
         n++;
      }
      dbPrintf("Number of Segments: %d\r\n"
                       "<num> seg  parent type     hex_data         label                    info   res\n", n);
      for (i=0; i<n; i++) {
         dbPrintf("[%2d]%3d -> %3d   %d:%-6s[ %02x %02x %02x %02x %02x] %-24s  0x%02x  0x%02x\n",
               i, mux[i].t228.seg, mux[i].t228.parent,
               mux[i].t228.mux.type,
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_CPLD)?    "CPLD":
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_GPO)?     "GPO":
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_SCHAIN)?  "SChain":
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_I2C)?     "I2C":
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_PECI_I2C)?"PECI2C":
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_PECI)?    "PECI":
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_PECI_G8)? "PECI2":
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_I2CEX)?   "I2CEX":
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_CARB)?    "CARB":
               (mux[i].t228.mux.type==0)?                        "I2C":
                                                                 "unk",
               mux[i].t228.mux.select.unknown.byt[0],
               mux[i].t228.mux.select.unknown.byt[1],
               mux[i].t228.mux.select.unknown.byt[2],
               mux[i].t228.mux.select.unknown.byt[3],
               mux[i].t228.mux.select.unknown.byt[4],
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_CPLD)?    "[xreg mask set res res]":
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_GPO)?     "[offs mask set res res]":
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_SCHAIN)?  "[offs mask set res res]":
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_I2C)?     "[addr offs mask set res]":
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_I2CEX)?   "[seg addr offs mask set]":
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_PECI_I2C)?"[addr offs mask set dev]":
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_PECI_G8)? "[dom bus d/fn offset]":
               (mux[i].t228.mux.type==SMBIOS_T228_TYPE_CARB)?    "[res res res res res]":
               "[? ? ? ? ?]",
               mux[i].t228.info, mux[i].t228.res);
      }
      return;
   }
   if ((!strutil_icmp(argv[1], (char*)"DIMM")) || (!strutil_icmp(argv[1], (char*)"DIMMS"))) {
      if (argc > 2) {
         dbPrintf("SMBIOS DIMM field details:\r\n"
                      " num:  enumeration of the instance, determined by order of records\r\n"
                      " DIMM: Houses multiple fields pertinent to the memory module\r\n"
                      "   t_17: SMBIOS type 17 handle number\r\n"
                      "   st:   Module status; 0: vacant\r\n"
                      "   gr:   Thermal group number\r\n"
                      "   slot: board slot number\r\n"
                      "   sock: module socket number\r\n"
                      "   set:  interleave set number\r\n\n"
                      " CPU:  Houses multiple fields pertinent to the CPU hosting the module\r\n"
                      "   t_4:  SMBIOS type 4 handle number\r\n"
                      "   st:   processor status; 0: vacant;\r\n"
                      "   slot: slot number or 0xFF if not used\r\n"
                      "   sock: socket number or 0xFF if not used\r\n\n"
                      " I2C:  Houses system internal bus routine and addressing information\r\n"
                      "   seg:  segment number the temperature sensor, SPD, and EEPROM are on\r\n"
                      "   addr: base address of the device (extrapolate temp sensor from this\r\n"
                      "   type: temperature sensor enumerator.  1: Microchip\r\n"
                      "   mstr: master segment type: I2C or PECI\r\n"
                      " reg:    register\r\n"
               );
         return;
      }
      dbPrintf("DIMM record summary");
      for (i=0; !smbios_get_dimm(i, &d); i++) {
         if (i==0) {
            dbPrintf("     -----------DIMM-------------  ---------CPU--------  --------I2C------------  \r\n"
                             "num  t_17 st  gr  slot  sock  set   t_4  st  slot  sock  seg  addr  type    mstr   reg");
         }
         if (d.dimm.status) {
            dbPrintf("\033[0;32m[%2d] \033[0m\n",i); 
         }
         else {
            dbPrintf("[%2d] \n",i);                       
         }         
         dbPrintf(
                          "%04X %d  %3d   %02x    %02x   %2d   "
                          "%04x  %d    %02x    %02x    "
                          "%02x   %02x    %02x  %-7s  %04x\n",
                          d.dimm.hndl_type_17, d.dimm.status, d.dimm.group, d.dimm.slot, d.dimm.socket, d.dimm.set,
                          d.cpu.hndl_type_4, d.cpu.status, d.cpu.slot, d.cpu.socket,
                          d.i2c.seg, d.i2c.addr, d.i2c.type,
                          (d.i2c.mstr==SMBIOS_T228_TYPE_I2C) ?"I2C":
                          (d.i2c.mstr==SMBIOS_T228_TYPE_PECI)?"PECI":
                          (d.i2c.mstr==SMBIOS_T228_TYPE_PECI_G8)?"PECI2":
                          (d.i2c.mstr==SMBIOS_T228_TYPE_ARM)?"LM75I2C":" ?? \n",
                          d.cltt.reg
               );
      }
      return;
   }
   if ((!strutil_icmp(argv[1], (char*)"MB"))) {
      if (argc > 2) {
         dbPrintf("SMBIOS Memory buffer field details:\r\n"
                      " num:  enumeration of the instance, determined by order of records\r\n"
                      " Module: Houses multiple fields pertinent to the Millbrook\r\n"
                      "   t_17: SMBIOS type 17 handle number\r\n"
                      "   st:   Module status; 0: vacant\r\n"
                      "   gr:   Thermal group number\r\n"
                      "   slot: board slot number\r\n"
                      "   sock: module socket number\r\n"
                      "   set:  interleave set number\r\n\n"
                      " CPU:  Houses multiple fields pertinent to the CPU hosting the memory bugger\r\n"
                      "   t_4:  SMBIOS type 4 handle number\r\n"
                      "   st:   processor status; 0: vacant;\r\n"
                      "   slot: slot number or 0xFF if not used\r\n"
                      "   sock: socket number or 0xFF if not used\r\n\n"
                      " I2C:  Houses system internal bus routine and addressing information\r\n"
                      "   seg:  segment number of the Millbrook\r\n"
                      "   addr: base address of the device\r\n"
                      "   type: temperature sensor enumerator.  2: Millbrook, 3: Jordan Creek\r\n"
                      "   mstr: master segment type: I2C or PECI\r\n"
               );
         return;
      }
      dbPrintf("Millbrook record summary");
      for (i=0; !smbios_get_mb(i, &d, SMBIOS_T227_MILLBROOK); i++) {
         if (i==0) {
            dbPrintf("     ---------Millbrook----------  ---------CPU--------  --------I2C---------\r\n"
                             "num  t_17 st  gr  slot  sock  set   t_4  st  slot  sock  seg  addr  type mstr");
         }
         dbPrintf("[%2d] "
                          "%04X %d  %3d   %02x    %02x   %2d   "
                          "%04x  %d    %02x    %02x    "
                          "%02x   %02x    %02x  %-4s\n",
                          i, 
                          d.dimm.hndl_type_17, d.dimm.status, d.dimm.group, d.dimm.slot, d.dimm.socket, d.dimm.set,
                          d.cpu.hndl_type_4, d.cpu.status, d.cpu.slot, d.cpu.socket,
                          d.i2c.seg, d.i2c.addr, d.i2c.type,
                          (d.i2c.mstr==SMBIOS_T228_TYPE_I2C) ?"I2C":
                          (d.i2c.mstr==SMBIOS_T228_TYPE_PECI)?"PECI":
                          (d.i2c.mstr==SMBIOS_T228_TYPE_PECI_G8)?"PECI2":" ?? "
               );
      }
      dbPrintf("Jordan Creek record summary");
      for (i=0; !smbios_get_mb(i, &d, SMBIOS_T227_JORDANCREEK); i++) {
         if (i==0) {
            dbPrintf("     -------Jordan Creek---------  ---------CPU--------  --------I2C---------\r\n"
                             "num  t_17 st  gr  slot  sock  set   t_4  st  slot  sock  seg  addr  type mstr");
         }
         dbPrintf("[%2d] "
                          "%04X %d  %3d   %02x    %02x   %2d   "
                          "%04x  %d    %02x    %02x    "
                          "%02x   %02x    %02x  %-4s\n",
                          i, 
                          d.dimm.hndl_type_17, d.dimm.status, d.dimm.group, d.dimm.slot, d.dimm.socket, d.dimm.set,
                          d.cpu.hndl_type_4, d.cpu.status, d.cpu.slot, d.cpu.socket,
                          d.i2c.seg, d.i2c.addr, d.i2c.type,
                          (d.i2c.mstr==SMBIOS_T228_TYPE_I2C) ?"I2C":
                          (d.i2c.mstr==SMBIOS_T228_TYPE_PECI)?"PECI":
                          (d.i2c.mstr==SMBIOS_T228_TYPE_PECI_G8)?"PECI2":" ?? "
               );
      }
      return;
   }
   if (!strutil_icmp(argv[1], (char*)"TYPE")) {
      if (argc>2) {
         t = strtol(argv[2], 0, 0);
         dbPrintf("SMBIOS records of type %d: \"%s\"\n", t, smbios_type_string(t));
         p = 0;
         while ( (!(smbios_get_rec_by_type_and_position( (UINT8)t, p, buf, sizeof(buf)))) && (hdr->len) ) {
            p++;
            smbios_rec_decode(buf);
            
            /* Need a small throttle */
            if (0 == (p%10))
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
//                usleep(100000);
         }
         dbPrintf("Listed %d entries of type %d\n", p, t);
      } else {
         dbPrintf("Missing record type");
      }
      return;
   }
   if (!strutil_icmp(argv[1], (char*)"HANDLE")) {
      if (argc>2) {
         h = strtol(argv[2], 0, 0);
         if (smbios_get_rec_by_handle( (UINT16)h, buf, sizeof(buf))) {
            dbPrintf("Not found\n");
         } else {
            smbios_rec_decode(buf);
            smbios_rec_dump(buf);
         }
      } else {
         dbPrintf("Missing Handle");
      }
      return;
   }

   dbPrintf("Unsupported %s %s\n", argv[0], argv[1]);
}

