/*
// Copyright (c) 2003-2025 Hewlett Packard Enterprise Development, LP
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

#ifndef SMBIOS_H
#define SMBIOS_H

#define SMBIOS_TYPE_ROM             0
#define SMBIOS_TYPE_ROM_VERSION     0x01
#define SMBIOS_TYPE_ROM_DATE        0x02
#define SMBIOS_TYPE_ROM_CMPL        (SMBIOS_TYPE_ROM_VERSION | SMBIOS_TYPE_ROM_DATE)


#define SMBIOS_TYPE_HOST            1
#define SMBIOS_TYPE_HOST_NAME       0x01
#define SMBIOS_TYPE_HOST_NUM        0x02
#define SMBIOS_TYPE_HOST_VSN        0x04
#define SMBIOS_TYPE_HOST_CMPL       (SMBIOS_TYPE_HOST_NAME | SMBIOS_TYPE_HOST_NUM | SMBIOS_TYPE_HOST_VSN)

#define SMBIOS_TYPE_CPU             4
#define SMBIOS_TYPE_CPU_SPEED       0x01
#define SMBIOS_TYPE_CPU_L1CACHE     0x02
#define SMBIOS_TYPE_CPU_L2CACHE     0x04
#define SMBIOS_TYPE_CPU_L3CACHE     0x08
#define SMBIOS_TYPE_CPU_CMPL        (SMBIOS_TYPE_CPU_SPEED | SMBIOS_TYPE_CPU_L1CACHE \
                                     | SMBIOS_TYPE_CPU_L2CACHE | SMBIOS_TYPE_CPU_L3CACHE)


#define SMBIOS_TYPE_SLOT            9
#define SMBIOS_TYPE_SLOT_TYPE       0x01
#define SMBIOS_TYPE_SLOT_WIDTH      0x02
#define SMBIOS_TYPE_SLOT_CMPL       (SMBIOS_TYPE_SLOT_TYPE | SMBIOS_TYPE_SLOT_WIDTH)

#define SMBIOS_TYPE_MEMORY          17
#define SMBIOS_TYPE_MEMORY_SIZE     0x01
#define SMBIOS_TYPE_MEMORY_SPEED    0x02
#define SMBIOS_TYPE_MEMORY_LOC      0x04
#define SMBIOS_TYPE_MEMORY_CMPL     (SMBIOS_TYPE_MEMORY_SIZE | SMBIOS_TYPE_MEMORY_SPEED \
                                     | SMBIOS_TYPE_MEMORY_LOC)


#define SMBIOS_TYPE_PROC            (  4)
#define SMBIOS_TYPE_PHY_MEM_ARRAY   ( 16)
#define SMBIOS_TYPE_MEM_MOD         ( 17)
#define SMBIOS_TYPE_CPQ_PROC        (197)
#define SMBIOS_TYPE_MEM_LOC         (202)
#define SMBIOS_TYPE_NIC             (209)
#define SMBIOS_TYPE_PWR_FEATURES    (219)
#define SMBIOS_TYPE_OPTION_ROM      (224)
#define SMBIOS_TYPE_DIMM_I2C        (227)
#define SMBIOS_TYPE_PECI            (228)
#define SMBIOS_TYPE_DIMM_ATTRIB     (232)
#define SMBIOS_TYPE_DIMM_VENDOR_INFO (237)
#define SMBIOS_TYPE_DIMM_CONFIG     (244)
#define SMBIOS_TYPE_EXT_BOARD_INV   (245)

// Structures must be packed
#pragma pack(1)

//HPE Proliant Server SMBIOS Extensions for UEFI Systems v3.54
#define UNSPECIFIED_MODULE   0
#define TPM_12_MODULE        1
#define TPM_20_MODULE        2
#define INTEL_PTT_MODULE     3

typedef struct {
   struct {
      UINT16 hndl_type_17; // SMBIOS DIMM handle
      UINT8  status;       // presence 1=present; 0=absent   (source: type 17; derived from size)
      UINT8  group;        // thermal group                  (source: type 227)
      UINT8  slot;         // 0xff if the slot is on the PCA (source: type 202)
      UINT8  socket;       // 1-based socket number          (source: type 202)
      UINT8  ie_dimm;      // 0-based IE dimm number
      UINT8  ie_sensor;    // IE sensor ID
      UINT8  set;          // interleave group/set           (source: type 17)
      UINT8  mem_tech;     // Memory technology              (source: type 17)
      UINT8  dimm_index;
   } dimm;
   struct {
      UINT16 hndl_type_4;  // SMBIOS CPU handle
      UINT8  status;       // presence 1=present; 0=absent      (source: type 4)
      UINT8  slot;         // 1-based slot (0xff is not used)   (source: type 197)
      UINT8  socket;       // 1-based socket (0xff if not used) (source: type 197)
      UINT8  res;          // reserved for alignment
   } cpu;
   struct {
      UINT8  seg;          // segment number                 (source: type 227)
      UINT8  addr;         // target address (other addresses may be derived from this)
      UINT8  type;         // type of thermal sensor
      UINT8  mstr;         // I2C or PECI master             (source: derived from type 228)
                           //     (SMBIOS_T228_TYPE_I2C or SMBIOS_T228_TYPE_PECI)
      UINT16 spd_size;     // Size of the SPD, based on module type (DDR3: 256, DDR4: 512 DDR5: 1024)
   } i2c;
   struct {
      UINT8   domain;            //domain inside cpu
      UINT8   pci_bus;           //smbus (1)
      UINT8   pci_dev_fnc;    //smbus dev/function(15/0)
      UINT16  smbus_offset;  //smbus register offset
      UINT8   res;
    }peci;  //for G8 peci smbus controller
   struct {
      UINT8   bus;
      UINT8   dev;
      UINT8   fn;
      UINT8   chan;
      UINT16  reg;
   } cltt; // for Intel closed-loop temperature throttling output (writing temp to CPU for mem throttling)
} type_dimm;

typedef struct {
    char        socket[50];
    UINT16      cpu_speed;
    UINT32      l1_size;
    UINT32      l2_size;
    UINT32      l3_size;
    UINT16      n_cores; // here and below added in version 2.4+ SMBIOS
    UINT16      n_cores_ena;
    UINT16      n_thrd;  // updated cores, cores_ena and thrd to 16bit in SMBIOS 3.0
    UINT16      cpu_cap;
    char        cpu_name[256];
} SMBIOS_proc_info;

typedef struct {
    UINT8 type;
    UINT8 width;
} SMBIOS_slot_info;

#define SMBIOS_RETV_SUCCESS       (0x0000)
#define SMBIOS_RETV_NOT_FOUND     (0x0001)
#define SMBIOS_RETV_BAD_PARAM     (0x0002)
#define SMBIOS_RETV_CLIPPED       (0x0003)
#define SMBIOS_RETV_NO_ROOT       (0x0006)

extern void  smbios_data_begin(void);
extern void  smbios_data_end(void);
extern void  smbios_data_record(void *p, int len);
extern const char *smbios_type_string (int type);
extern const char *smbios_rec_s(const void *p, int id);
extern int smbios_get_rec_by_type_and_position(UINT8 type, int position, UINT8 *buf, UINT32 szbuf);

/* debug support */
extern int   smbios_rec_dump(void *p);
extern int   smbios_rec_decode(void *p);

/* specific requests */
extern int   smbios_get_dimm(int index, type_dimm *dimm);
extern int   smbios_get_mb(int index, type_dimm *dimm, UINT8 type);
extern int   smbios_get_rec_by_handle(UINT16 handle, UINT8 *buf, UINT32 szbuf);

/* the format of known SMBIOS records are below.
 * Each structure represents the structured data known to be in the SMBIOS record.
 * Unstructured strings are not part of the length field in the structure header.
 */
/* Every SMBIOS record begins with the following 4-byte header.
 *
 * type:   SMBIOS record type defined in Systems Management BIOS Specification
 * len:    the length of the formatted area of the structure.
 *         Strings typically follow the structure, and each record ends with
 *         a double-NULL.
 * handle: A unique handle for this structure instance.  Make no assumptions
 *         about handle numbering: they are not contiguous, do not begin at 0,
 *         and may be reassigned across boots.
 */
typedef struct {
    UINT8  type;
    UINT8  len;
    UINT16 handle;
} SMBIOS_HDR;

#define SMBIOS_T0_UEFI_SPEC_SUPP     (0x00000008)
typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       sid_vendor;
    UINT8       sid_bios_version;
    UINT16      bios_start_addr_seg;
    UINT8       sid_bios_date;
    UINT8       sz_rom;
    UINT64      bios_char;
    UINT8       bios_char_ext[2];
    UINT8       bios_maj;
    UINT8       bios_min;
    UINT8       ecfw_maj; // embedded controller firmware major release
    UINT8       ecfw_min;
    /* there are possibly more BIOS characteristic words here */
} type_0;

#define SMBIOS_SERNUM_LEN  (25)
#define SMBIOS_UUID_BIN_LEN (16)
typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       sid_manu;
    UINT8       sid_prod;
    UINT8       sid_ver;
    UINT8       sid_sn;
    UINT8       uuid[SMBIOS_UUID_BIN_LEN];
    UINT8       wakeup;
    UINT8       sid_sku_num;
    UINT8       sid_family;
} type_1;

typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       sid_manu;
    UINT8       sid_prod;
    UINT8       sid_ver;
    UINT8       sid_sn;
    UINT8       sid_asset_tag;
    UINT8       feature_flags;
    UINT8       sid_chassis_loc;
    UINT16      t3_hndl;          // Handle to associated Type 3 chassis Record
    UINT8       board;
    UINT8       n_handles;
    UINT8       handle[1];        // 0 (or more) associated handles
} type_2;

typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       sid_manu;
    UINT8       type;
    UINT8       sid_ver;
    UINT8       sid_sn;
    UINT8       sid_atag;
    UINT8       boot;
    UINT8       ps;
    UINT8       therm;
    UINT8       sec;
    UINT32      oeminfo;
    UINT8       height;
    UINT8       n_pwr_cords;
    UINT8       n_elem;
    UINT8       sz_elem;
    //NT8       0 or more contained element records.  BS from DMTF clowns.  Easy to generate, sucks to parse.
    UINT8       sid_sku;
} type_3;

typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       sid_sock;
    UINT8       cpu_type;
    UINT8       cpu_fam;
    UINT8       sid_cpu_manuf;
    UINT16      cpuid[4];
    UINT8       sid_cpu_ver;
    UINT8       cpu_voltage;
    UINT16      cpu_clock;
    UINT16      cpu_max_speed;
    UINT16      cpu_speed;
    UINT8       cpu_status;
    UINT8       cpu_upgrade;
    UINT16      hndl_l1;
    UINT16      hndl_l2;
    UINT16      hndl_l3;
    UINT8       sid_cpu_sn;
    UINT8       sid_cpu_atag;
    UINT8       sid_cpu_pn;
    UINT8       n_cores; // here and below added in version 2.4+ SMBIOS
    UINT8       n_cores_ena;
    UINT8       n_thrd;
    UINT16      cpu_cap;
    UINT16      cpu2_fam; // added in version 2.6 SMBIOS
    UINT16      n2_cores; // here and below added in 3.0 SMBIOS
    UINT16      n2_cores_ena;
    UINT16      n2_thrd;
} type_4;
// Macros to decode when to use UINT16 SMBIOS 2.6/3.0 entries
#define cpu_fam_type4_p(t4p)     ((t4p->cpu2_fam) ? t4p->cpu2_fam : t4p->cpu_fam)
#define n_cores_type4_p(t4p)     ((t4p->n2_cores) ? t4p->n2_cores : t4p->n_cores)
#define n_cores_ena_type4_p(t4p) ((t4p->n2_cores_ena) ? t4p->n2_cores_ena : t4p->n_cores_ena)
#define n_thrd_type4_p(t4p)      ((t4p->n2_thrd) ? t4p->n2_thrd : t4p->n_thrd)

typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       sid_sock;
    UINT16      cfg;
    UINT16      max;
    UINT16      size;
    UINT16      sram_s;
    UINT16      sram;
    UINT8       speed;
    UINT8       ecc;
    UINT8       type;
    UINT8       associativity;
} type_7;

typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       sid_ird;
    UINT8       int_connector;
    UINT8       sid_u_ref;
    UINT8       ext_connector;
    UINT8       port;
} type_8;

typedef struct {
    UINT16      seg_grp_num;
    UINT8       bus_number;
    UINT8       dev_func_num;
    UINT8       data_bus_width;
} peer_group;

#define VER_2_6_SZ 17
#define T9_MAX_PEER_GROUP 8
typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       sid_slot;
    UINT8       type;
        // From DSP0134 System Management BIOS (SMBIOS) Reference Specification 3.5.0, 2021-09-15
        //              7.10.1 System Slots - Slot Type
        //              Table 45 � System Slots: Slot Type field
        #define T9_SLOT_TYPE_OTHER                    0x01 // Other
        #define T9_SLOT_TYPE_UNKOWN                   0x02 // Unknown
        #define T9_SLOT_TYPE_ISA                      0x03 // ISA
        #define T9_SLOT_TYPE_MCA                      0x04 // MCA
        #define T9_SLOT_TYPE_EISA                     0x05 // EISA
        #define T9_SLOT_TYPE_PCI                      0x06 // PCI
        #define T9_SLOT_TYPE_PCMCIA                   0x07 // PC Card (PCMCIA)
        #define T9_SLOT_TYPE_VL_VESA                  0x08 // VL-VESA
        #define T9_SLOT_TYPE_PROPRIETARY              0x09 // Proprietary
        #define T9_SLOT_TYPE_PROC_CRD_SLT             0x0A // Processor Card Slot
        #define T9_SLOT_TYPE_PROPRIETARY_MEM_CRD_SLT  0x0B // Proprietary Memory Card Slot
        #define T9_SLOT_TYPE_IO_RISER_CRD             0x0C // I/O Riser Card Slot
        #define T9_SLOT_TYPE_NUBUS                    0x0D // NuBus
        #define T9_SLOT_TYPE_PCI_66_MHZ               0x0E // PCI � 66MHz Capable
        #define T9_SLOT_TYPE_AGP                      0x0F // AGP
        #define T9_SLOT_TYPE_AGP_2X                   0x10 // AGP 2X
        #define T9_SLOT_TYPE_AGP_4X                   0x11 // AGP 4X
        #define T9_SLOT_TYPE_PCI_X                    0x12 // PCI-X
        #define T9_SLOT_TYPE_AGP_8X                   0x13 // AGP 8X
        #define T9_SLOT_TYPE_M2_SCKT_1DP_KEY_A        0x14 // M.2 Socket 1-DP (Mechanical Key A)
        #define T9_SLOT_TYPE_M2_SCKT_1SD_KEY_E        0x15 // M.2 Socket 1-SD (Mechanical Key E)
        #define T9_SLOT_TYPE_M2_SCKT_3___KEY_B        0x16 // M.2 Socket 2 (Mechanical Key B)
        #define T9_SLOT_TYPE_M2_SCKT_3___KEY_M        0x17 // M.2 Socket 3 (Mechanical Key M)
        #define T9_SLOT_TYPE_MXM_TYPE_I               0x18 // MXM Type I
        #define T9_SLOT_TYPE_MXM_TYPE_II              0x19 // MXM Type II
        #define T9_SLOT_TYPE_MXM_TYPE_III_STD         0x1A // MXM Type III (standard connector)
        #define T9_SLOT_TYPE_MXM_TYPE_III_HE          0x1B // MXM Type III (HE connector)
        #define T9_SLOT_TYPE_MXM_TYPE_IV              0x1C // MXM Type IV
        #define T9_SLOT_TYPE_MXM_3_0_TYPE_A           0x1D // MXM 3.0 Type A
        #define T9_SLOT_TYPE_MXM_3_0_TYPE_B           0x1E // MXM 3.0 Type B
        #define T9_SLOT_TYPE_PCI_E_GEN2_U_2           0x1F // PCI Express Gen 2 SFF-8639 (U.2)
        #define T9_SLOT_TYPE_PCI_E_GEN3_U_2           0x20 // PCI Express Gen 3 SFF-8639 (U.2)
        #define T9_SLOT_TYPE_PCI_E_MINI_52P_BSKO      0x21 // PCI Express Mini 52-pin (CEM spec. 2.0) with bottom-side keep-outs. 
                                                           // Use Slot Length field value 03h (short length) for "half-Mini card"-only support, 
                                                           // Use Slot Length field value 04h (long length) for "full-Mini card" or dual support.
        #define T9_SLOT_TYPE_PCI_E_MINI_52P           0x22 // PCI Express Mini 52-pin (CEM spec. 2.0) without bottom-side keep-outs. 
                                                           // Use Slot Length field value 03h (short length) for "half-Mini card"-only support, 
                                                           // Use Slot Length field value 04h (long length) for "full-Mini card" or dual support.
        #define T9_SLOT_TYPE_PCI_E_MINI_72P           0x23 // PCI Express Mini 76-pin (CEM spec. 2.0) Corresponds to Display-Mini card.
        #define T9_SLOT_TYPE_PCI_E_GEN4_U_2           0x24 // PCI Express Gen 4 SFF-8639 (U.2)
        #define T9_SLOT_TYPE_PCI_E_GEN5_U_2           0x25 // PCI Express Gen 5 SFF-8639 (U.2)
        #define T9_SLOT_TYPE_OCP_NIC_3_0_SFF          0x26 // OCP NIC 3.0 Small Form Factor (SFF)
        #define T9_SLOT_TYPE_OCP_NIC_3_0_LFF          0x27 // OCP NIC 3.0 Large Form Factor (LFF)
        #define T9_SLOT_TYPE_OCP_NIC                  0x28 // OCP NIC Prior to 3.0
        #define T9_SLOT_TYPE_CXL_FLEXBUS_1_0          0x30 // CXL Flexbus 1.0 (deprecated, see note below)
        #define T9_SLOT_TYPE_PC_98_C20                0xA0 // PC-98/C20 
        #define T9_SLOT_TYPE_PC_98_C24                0xA1 // PC-98/C24 
        #define T9_SLOT_TYPE_PC_98_E                  0xA2 // PC-98/E 
        #define T9_SLOT_TYPE_PC_98_LOCAL              0xA3 // PC-98/Local Bus 
        #define T9_SLOT_TYPE_PC_98_CARD               0xA4 // PC-98/Card 
        #define T9_SLOT_TYPE_PCI_E                    0xA5 // PCI Express (see note below)
        #define T9_SLOT_TYPE_PCI_E_X1                 0xA6 // PCI Express x1
        #define T9_SLOT_TYPE_PCI_E_X2                 0xA7 // PCI Express x2
        #define T9_SLOT_TYPE_PCI_E_X4                 0xA8 // PCI Express x4
        #define T9_SLOT_TYPE_PCI_E_X8                 0xA9 // PCI Express x8
        #define T9_SLOT_TYPE_PCI_E_X16                0xAA // PCI Express x16
        #define T9_SLOT_TYPE_PCI_E_GEN2               0xAB // PCI Express Gen 2 (see note below)
        #define T9_SLOT_TYPE_PCI_E_GEN2_X1            0xAC // PCI Express Gen 2 x1
        #define T9_SLOT_TYPE_PCI_E_GEN2_X2            0xAD // PCI Express Gen 2 x2
        #define T9_SLOT_TYPE_PCI_E_GEN2_X4            0xAE // PCI Express Gen 2 x4
        #define T9_SLOT_TYPE_PCI_E_GEN2_X8            0xAF // PCI Express Gen 2 x8
        #define T9_SLOT_TYPE_PCI_E_GEN2_X16           0xB0 // PCI Express Gen 2 x16
        #define T9_SLOT_TYPE_PCI_E_GEN3               0xB1 // PCI Express Gen 3 (see note below)
        #define T9_SLOT_TYPE_PCI_E_GEN3_X1            0xB2 // PCI Express Gen 3 x1
        #define T9_SLOT_TYPE_PCI_E_GEN3_X2            0xB3 // PCI Express Gen 3 x2
        #define T9_SLOT_TYPE_PCI_E_GEN3_X4            0xB4 // PCI Express Gen 3 x4
        #define T9_SLOT_TYPE_PCI_E_GEN3_X8            0xB5 // PCI Express Gen 3 x8
        #define T9_SLOT_TYPE_PCI_E_GEN3_X16           0xB6 // PCI Express Gen 3 x16
        #define T9_SLOT_TYPE_PCI_E_GEN4               0xB8 // PCI Express Gen 4 (see note below)
        #define T9_SLOT_TYPE_PCI_E_GEN4_X1            0xB9 // PCI Express Gen 4 x1
        #define T9_SLOT_TYPE_PCI_E_GEN4_X2            0xBA // PCI Express Gen 4 x2
        #define T9_SLOT_TYPE_PCI_E_GEN4_X4            0xBB // PCI Express Gen 4 x4
        #define T9_SLOT_TYPE_PCI_E_GEN4_X8            0xBC // PCI Express Gen 4 x8
        #define T9_SLOT_TYPE_PCI_E_GEN4_X16           0xBD // PCI Express Gen 4 x16
        #define T9_SLOT_TYPE_PCI_E_GEN5               0xBE // PCI Express Gen 5 (see note below)
        #define T9_SLOT_TYPE_PCI_E_GEN5_X1            0xBF // PCI Express Gen 5 x1
        #define T9_SLOT_TYPE_PCI_E_GEN5_X2            0xC0 // PCI Express Gen 5 x2
        #define T9_SLOT_TYPE_PCI_E_GEN5_X4            0xC1 // PCI Express Gen 5 x4
        #define T9_SLOT_TYPE_PCI_E_GEN5_X8            0xC2 // PCI Express Gen 5 x8
        #define T9_SLOT_TYPE_PCI_E_GEN5_X16           0xC3 // PCI Express Gen 5 x16

    UINT8       width;
    UINT8       current;
        // From DSP0134 System Management BIOS (SMBIOS) Reference Specification 3.5.0, 2021-09-15
        //              7.10.3 System Slots - Current Usage
        //              Table 47 � System Slots: Current Usage field
        #define T9_CURRENT_USAGE_OTHER                0x01 // other
        #define T9_CURRENT_USAGE_UNKOWN               0x02 // Unknown
        #define T9_CURRENT_USAGE_AVAILABLE            0x03 // Available
        #define T9_CURRENT_USAGE_IN_USE               0x04 // In use
        #define T9_CURRENT_USAGE_UNAVAILABLE          0x05 // Unavailable, For example, connected to a processor that is not installed.

    UINT8       length;
    UINT16      id;                               // 09h; 2.0+; Slot ID
    UINT8       properties[2];                    // 0Bh; 2.0+
    UINT16      seg_group_number;                 // 0Dh; 2.6+; Segment Group Number
    UINT8       bus;                              // 0Fh; 2.6+; Bus Number
    UINT8       dev_fn;                           // 10h; 2.6+; Device (Bits 7:3) and Function (Bits 2:0) Numbers
    UINT8       data_bus_width;                   // 11h; 3.2;  Electrical Width of Bus
    UINT8       peer_num;                         // 12h; 3.2;  Number of peer groups that follow  (Max: 8)
    peer_group  peer_grp[T9_MAX_PEER_GROUP];      // 13h; 3.2;  Peer Segment/Bus/Device/Function present in slot
} type_9;

/* type 11 is a collection of strings.
 * HP has some specific indicators: (ISS SMBIOS for UEFI v3.33)
 * PSF:        <<Information>> Permanent Storage Field
 * Product ID: <<Information>> This string will be used to store the Product ID
 * CPN:        <<Information>> This string will be used to store the Chassis Product Name
 */
typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       num;
} type_11;

typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       location;
    UINT8       use;
    UINT8       ecc;
    UINT32      max_kb;
    UINT16      hndl_err;
    UINT16      sockets;
    UINT64      cap;
} type_16;

#define SMBIOS_TYPE17_DETAIL_BIT_NVDIMM    0x1000

#define SMBIOS_TYPE17_MEMTECH_OTHER        0x01
#define SMBIOS_TYPE17_MEMTECH_UNKNOWN      0x02
#define SMBIOS_TYPE17_MEMTECH_DRAM         0x03
#define SMBIOS_TYPE17_MEMTECH_NVDIMM_N     0x04
#define SMBIOS_TYPE17_MEMTECH_NVDIMM_F     0x05
#define SMBIOS_TYPE17_MEMTECH_NVDIMM_P     0x06
#define SMBIOS_TYPE17_MEMTECH_INTEL_P      0x07

typedef struct {
    SMBIOS_HDR  hdr;
    UINT16      hndl_array;
    UINT16      hndl_error;
    UINT16      width_tot;      //if 0xffff then unknown
    UINT16      width_dat;      //if 0xffff unknown
    UINT16      size;           //if 0xffffh unknown.  if 0x7fff look at the extended size field below
    UINT8       ff;
    UINT8       set;
    UINT8       sid_device;
    UINT8       sid_bank;
    UINT8       type;
    UINT16      detail;
    UINT16      speed;          //max capable speed in MT/s (MHz in spec rev 3.0.0 and earlier)
    UINT8       sid_manu;
    UINT8       sid_sn;
    UINT8       sid_atag;
    UINT8       sid_pn;
    UINT8       attr;           //bits 0-3 rank, 0 = unknown
    UINT32      extended_size;  //Used to compliment the size field above for larger than 32GB devices
    UINT16      clock_speed;    //configured speed in MT/s. 0=unknown (MHz in spec rev 3.0.0 and earlier)
    UINT16      v_min;
    UINT16      v_max;
    UINT16      v_cfg;          // last word for SMBIOS Spec 3.1.1
    UINT8       mem_tech;
    UINT16      mem_omcap;
    UINT8       fw_ver;
    UINT16      mmanuf_id;
    UINT16      mprod_id;
    UINT16      mscmanuf_id;
    UINT16      mscprod_id;
    UINT64      nvol_sz;
    UINT64      vol_sz;
    UINT64      cach_sz;
    UINT64      logical_sz;
} type_17;

typedef struct {
    SMBIOS_HDR  hdr;
    UINT32      b_paddr;
    UINT32      e_paddr;
    UINT16      hndl_array;
    UINT8       width;
    UINT64      ext_start_addr;
    UINT64      ext_end_addr;
} type_19;

typedef struct {
    SMBIOS_HDR  hdr;
    UINT32      b_paddr;
    UINT32      e_paddr;
    UINT16      hndl_device;
    UINT16      hndl_array;
    UINT8       row;
    UINT8       interleave;
    UINT8       depth;
    UINT64      esa;
    UINT64      eea;
} type_20;

typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       reserved[6];
    UINT8       status[10];
} type_32;

typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       bmc_type;
    UINT8       rev;
    UINT8       bmc_addr;
    UINT8       nv_bus;
    UINT64      base_addr;
    UINT8       bam;
    UINT8       intr;
} type_38;

typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       pugroup;
    UINT8       sid_loc;
    UINT8       sid_devname;
    UINT8       sid_manu;
    UINT8       sid_sn;
    UINT8       sid_atag;
    UINT8       sid_model;
    UINT8       sid_rev;
    UINT16      cap;
    UINT16      attr;
    UINT16      hndl_26; // input voltage probe
    UINT16      hndl_27; // cooling device
    UINT16      hndl_29; // input current probe
} type_39;

typedef struct {
    SMBIOS_HDR  hdr;                  // length is 0Bh for version 2.6 and later
    UINT8       sid_ref_designation;  // onboard device reference designation
    UINT8       dev_type;             // Bit 7 - Device Status: 1-Enabled, 0-Disabled
                                      // Bit 6:0 - Device type
    UINT8       dev_type_instance;    // Device type instance
    UINT16      seg_group_number;     // Segment Group Number
    UINT8       bus;                  // PCI Bus number
    UINT8       dev_fn;               // PCI device (bits 7:3) and function (bits 2:0) numbers
} type_41;

#define TYPE_41_DEV_TYPE_OTHER          0x01
#define TYPE_41_DEV_TYPE_UNKNOWN        0x02
#define TYPE_41_DEV_TYPE_VIDEO          0x03
#define TYPE_41_DEV_TYPE_SCSI_CTLR      0x04
#define TYPE_41_DEV_TYPE_ETHERNET       0x05
#define TYPE_41_DEV_TYPE_TOKEN_RING     0x06
#define TYPE_41_DEV_TYPE_SOUND          0x07
#define TYPE_41_DEV_TYPE_PATA_CTLR      0x08
#define TYPE_41_DEV_TYPE_SATA_CTLR      0x09
#define TYPE_41_DEV_TYPE_SAS_CTLR       0x0A



typedef struct {
    SMBIOS_HDR  hdr;                  // DSP0134 3.3.0 SMBIOS spec definition
    UINT8       intf_type;            // enum TYPE_42_INTF_
    UINT8       sz_dat;               // bytes of data before protocol header begins
    UINT8       dat[1];
} type_42;

typedef struct {
    UINT8       protocol_records;     // number of protocol records
    UINT8       dat[1];               // start of first record
} type_42_proto_header;

typedef struct {
   UINT8        type;                 // Enumerated TYPE_42_PROT_
   UINT8        len;                  // protol type specific data length
   UINT8        dat[1];               // procotol type specific data
} type_42_protocol;

/* type 42 interface type identifiers
 * 0x00..0x3F: MCTP Host Interface type identifiers from DSP0239 1.6.0
 * 0x40..0xEF: Redfish Host Interface Specification / Network Host interface DSP0270 1.2.0
 * 0xF0:       OEM-defined
 * others      reserved
 */
#define TYPE_42_INTF_UNK     0x00
#define TYPE_42_INTF_UNKN    0x01
#define TYPE_42_INTF_KCS     0x02
#define TYPE_42_INTF_8250    0x03
#define TYPE_42_INTF_16450   0x04
#define TYPE_42_INTF_16550   0x05
#define TYPE_42_INTF_16650   0x06
#define TYPE_42_INTF_16750   0x07
#define TYPE_42_INTF_16850   0x08
/* 0x09..0x3F: reserved */
#define TYPE_42_INTF_REDFISH 0x40

/* type 42 protocol types from SMBIOS DSP0134 v3.3.0
 * 0x00    reserved
 * 0x01    reserved
 * 0x02    IPMI            Refer to IPMI appendix C1
 * 0x03    MCTP            Refer to DSP0236
 * 0x04    Redfish over IP Refer to DSP0270
 * 0xF0    OEM-defined
 */
#define TYPE_42_PROT_IPMI   0x02
#define TYPE_42_PROT_MCTP   0x03
#define TYPE_42_PROT_RESTIP 0x04
#define TYPE_42_PROT_OEM    0xF0



#if 0  /* DEPRECATED */
typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       version;
    UINT8       slot_number;
    UINT32      slot_prop;
} type_192;
#endif

typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       rrom_inst;
    UINT8       sid_rrom;
    UINT8       sid_bblk;
    UINT8       sid_oem_rom_file;
    UINT8       sid_oem_rom_date;
#if 0  /* use type 216 */
    UINT8       sid_pmc;
    struct {                 /* six PAL/CPLD records in type 193 */
       UINT16  ver[6];       /* PAL Revision read from scan chain; 0000 if not implemented */
       UINT8   sid_label[6]; /* pal location or description string (NULL if not implemented) */
    } pal;
#endif
} type_193;

typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       status;
} type_194;

#define VER_195_PRE_3_71 0x07
typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       sid_ssid;    // example "$0E11YYZZ" wher YY and ZZ are hardware-assigned system IDs.
    UINT8       plat_id[2];  // XRegister
    struct {
        UINT8       guid[16];    // personality GUID
    } ver_3_71;
} type_195;

#define VER_196_PRE_3_69 0x0F
typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       lcd_prop;
    UINT8       rack_u_offset;
    UINT32      efuse_thresholds;
    UINT8       boot_mode_indicator;
    UINT16      supported_socket_count;  //Processor Sockets supported by the server (not how many processors are installed).
                                         //If a server requires an optional board to allow support for more processors,
                                         //then the total possible for the server will be indicated even if the optional board(s) are not installed.
    UINT8       secure_boot_state;       //Current state of UEFI secure boot option
    UINT8       feature_state;           // deature enable/disable state
    struct {
        UINT8       sid_pch_name;            // String ID for PCH Name
        UINT8       pch_rev;                 // PCH Revision
    } ver_3_69;
} type_196;

#define VER_197_PRE_3_68 0x1A
typedef struct {
    SMBIOS_HDR  hdr;
    UINT16      hndl_type_4;
    UINT8       apic_id;
    UINT8       oem_status;
    UINT8       slot;
    UINT8       socket;
    UINT16      max_watt;
    UINT32      x2apic_id;
    UINT8       proc_uuid[8];
    UINT16      upi_speed;
    struct {
        UINT8       proc_qdf_num[6];
    } ver_3_68;
} type_197;

typedef struct {
    SMBIOS_HDR  hdr;
    UINT16      security;
    UINT16      ecc_thresh;
    UINT16      ecc_interval;
    UINT8       sys_type;
#define SYS_UNK          0
#define SYS_SERVER       1
#define SYS_WORKSTATION  2
#define SYS_DESKTOP      3
#define SYS_MOBILE       4
    UINT8       icru_delay;
    UINT8       min_ps;
    UINT8       min_ps_red;
} type_198;

typedef struct {
    SMBIOS_HDR  hdr;
    struct {
        UINT32      id;
        UINT32      date;
        UINT32      cpuid;
    } patch[1];
} type_199;

#if 0  /* DEPRECATED */
typedef struct {
    SMBIOS_HDR  hdr;
    UINT16      hndl_module;
    struct  {
        UINT8 offs;
        UINT8 data;
    } spd[1];
} type_200;
#endif

typedef struct {
    SMBIOS_HDR  hdr;
    union {
       struct {
          UINT8       version;
          UINT32      address[2];
       } SAL_table;  // Proposed IA64
       struct {
            UINT32      membist_nvram_offset;
            UINT8       membist_length;
            UINT16      membist_format; /* bits [15:12]:unused; [11:8]:Revision; [7:0]:Sub-revision */
            UINT32      reboot_nvram_offset;
            UINT8       reboot_length;
       } proliant;    // used Gen8+
    } member_union;
} type_201;

#define VER_202_PRE_3_61 0x19
#define VER_202_PRE_3_71 0x1A
typedef struct {
    SMBIOS_HDR  hdr;
    UINT16      hndl_module; // type 17
    UINT8       slot;
    UINT8       socket;
    UINT8       processor;
    UINT8       logical_dimm_id;
    UINT8       sid_uefi_dev_path;
    UINT8       sid_uefi_dev_structured_name;
    UINT8       sid_uefi_device_name;
    UINT8       ctrl; // 1-based memory controller number
    UINT8       chan; // 1-based memory channel number
    UINT8       ie_dimm;     // 0-based DIMM number for Innovation Engine, if suppoerted; else 0xFF
    UINT8       ie_sensor;   // 0-based PLDM semsor ID if Innovation Engine; else 0xff
    UINT16      id_vend;     // module mfg ID code from SPD
    UINT16      id_dev;      // (NVDIMM only) module product ID code from SPD
    UINT16      id_ctrlvend; // (NVDIMM only) Controller mfg ID from SPD
    UINT16      id_ctrldev;  // (NVDIMM only) Controller product ID from SPD
    struct {
        UINT8       bestInterleavedSet; // 1-based set ID the dimm belongs to. 0 if not part of a set.
    } ver_3_61;
    struct {
        UINT8       sid_part_num;
    } ver_3_71;
    UINT8       dimm_index;
} type_202;

#if 0 /* DEPRECATED format */
typedef struct {
    SMBIOS_HDR  hdr;
    struct {
        UINT8   maj;
        UINT8   min;
    } family[1];
} type_203;
#endif
#define INVALID_HANDLE 0xFFFE
#define VER_203_PRE_3_66 0x22
#define VER_203_PRE_3_72 0x24
#define PCIE_BIFURCATED_MASK_BP0 0x01         // Bit[0] = Peer Indicator in T203 flags ; 0 = NOT a Peer bifurcated device; 1 = Peer bifurcated device.

typedef struct {
    SMBIOS_HDR  hdr;
    UINT16      assoc_dev_hndl;     // This is the Type 9 or Type 41 Record for this entry OR 0xFFFE
    UINT16      assoc_smbus_hndl;   // This is the Type 228 (I2C topology) Record for this device OR 0xFFFE
    UINT16      pci_vend_id;        // IFF a PCIe device
    UINT16      pci_dev_id;
    UINT16      pci_sub_vend_id;
    UINT16      pci_sub_dev_id;
    /*The smbios 203 "PCI Class Code" macro -- start*/
    /*The "pci_class_code" field in type_203 structure
    * See ProLiant Server SMBIOS OEM extensions for all enumerations */
    #define PCI_CLASS_CODE_PRE_PCI_2            0x00
    #define PCI_CLASS_CODE_MASS_STRG_CTRL       0x01
    #define PCI_CLASS_CODE_NW_CTRL              0x02
    #define PCI_CLASS_CODE_VIDEO_CTRL           0x03
    #define PCI_CLASS_CODE_MULTIMEDIA           0x04
    #define PCI_CLASS_CODE_MEM_CTRL             0x05
    #define PCI_CLASS_CODE_BRIDGE               0x06
    #define PCI_CLASS_CODE_COMM_CTRL            0x07
    #define PCI_CLASS_CODE_SYS_PERIPHERALS      0x08
    #define PCI_CLASS_CODE_INPUT                0x09
    #define PCI_CLASS_CODE_DOCK_STATION         0x0A
    #define PCI_CLASS_CODE_PROCESSORS           0x0B
    #define PCI_CLASS_CODE_SERIAL_BUS_CTRL      0x0C
    #define PCI_CLASS_CODE_WIRELESS_CTRL        0x0D //Wireless controllers
    #define PCI_CLASS_INTELLIGENT_IO_CTRL       0x0E //Intelligent I/O controllers
    #define PCI_CLASS_SATCOM_CTRL               0x0F //Satellite communication controllers
    #define PCI_CLASS_ENCRYPT_DECRYPT_CTRL      0x10 //Encryption/Decryption controllers
    #define PCI_CLASS_DAQ_SIGNAL_PROC_CTRL      0x11 //Data acquisition and signal processing controllers
    #define PCI_CLASS_PROC_ACCELERATOR          0x12 //Processing accelerator
    #define PCI_CLASS_NE_INSTRUMENTATION        0x13 //Non-Essential Instrumentation
    #define PCI_CLASS_CODE_MISC                 0xFF
    /*The smbios 203 PCI Class Code macro -- end*/
    UINT8       pci_class_code;
    #define PCI_SUBCLASS_CODE_COPROCESSOR       0x40
    UINT8       pci_sub_class_code;
    UINT16      parent_hndl;        // Parent record handle, 0xFFFE for no children
    UINT16      flags;
    struct {
        /*The smbios 203 "Device Type" macro -- start*/
        /*The "device_type" field in type_203 structure
         * See ProLiant Server SMBIOS OEM extensions for all enumerations */
        #define SMBIOS_UNKNOWN_DEV_TYPE             0x00
        #define SMBIOS_FLX_LOM_NIC_DEV_TYPE         0x03
        #define SMBIOS_EMBED_LOM_DEV_TYPE           0x04
        #define SMBIOS_NIC_SLOT_DEV_TYPE            0x05
        #define SMBIOS_STRG_CTRL_DEV_TYPE           0x06
        #define SMBIOS_SMRT_ARR_STRG_CTRL_DEV_TYPE  0x07
        #define SMBIOS_USB_HD_DRV_DEV_TYPE          0x08
        #define SMBIOS_OTHER_PCI_DEV_TYPE           0x09
        #define SMBIOS_RAM_DISK_DEV_TYPE            0x0A
        #define SMBIOS_FW_VOL_DEV_TYPE              0x0B
        #define SMBIOS_UEFI_SHELL_DEV_TYPE          0x0C
        #define SMBIOS_UEFI_USB_BOOT_ENTRY_DEV_TYPE 0x0D
        #define SMBIOS_DSA_CTRL_DEV_TYPE            0x0E
        #define SMBIOS_FILE_DEV_TYPE                0x0F
        #define SMBIOS_NVME_HD_DRV_DEV_TYPE         0x10
        #define SMBIOS_NVDIMM_DEV_TYPE              0x11
        /*The smbios 203 Device Type macro -- end*/
        UINT8   device_type;
        /*The "device_location" field in type_203 structure */
        #define SMBIOS_UNKNOWN_LOC 0x00
        #define SMBIOS_EMBEDDED_LOC 0x01
        #define SMBIOS_ILO_VIRT_MEDIA_LOC 0x02
        #define SMBIOS_FRONT_USB_LOC 0x03
        #define SMBIOS_REAR_USB_LOC 0x04
        #define SMBIOS_INTERNAL_USB_LOC 0x05
        #define SMBIOS_INTERNAL_SD_LOC 0x06
        #define SMBIOS_INTERNAL_NAND_LOC 0x07
        #define SMBIOS_EMBED_SATA_LOC 0x08
        #define SMBIOS_EMBED_SA_LOC 0x09
        #define SMBIOS_PCI_SLOT_LOC 0x0A
        #define SMBIOS_RAM_MEM_LOC 0x0B
        #define SMBIOS_USB_LOC 0x0C
        #define SMBIOS_DSA_LOC 0x0D
        #define SMBIOS_URL_LOC 0x0E
        #define SMBIOS_NVME_DRIVE_BAY_LOC 0x0F
        #define SMBIOS_NVDIMM_PROCESSOR_LOC 0x10
        #define SMBIOS_NVDIMM_BOARD_LOC 0x11
        #define SMBIOS_NVME_RISER_LOC 0x12
        #define SMBIOS_NVDRIMM_NAME_SPACE_LOC 0x13
        #define SMBIOS_VROC_SATA_LOC 0x14
        #define SMBIOS_VROC_NVME_LOC 0x15
        /*The smbios 203 Device Location macro -- end*/
        UINT8   device_location;  /* enumeration */
        UINT8   device_inst;      /* UEFI device instance value or 0 */
        UINT8   device_sub_inst;
        UINT8   bay;
        UINT8   enclosure;
        UINT8   sid_dev_path;
        UINT8   sid_dev_structured_name;
        UINT8   sid_device_name;
        UINT8   sid_location;
    } uefi;
    UINT16  bifurcated_hndl_t9;            // associated "real and physical" record which includes physical slot number
    struct {
        UINT8   sid_dev_part_number;
        UINT8   sid_dev_serial_number;
    } pcie_vpd;
    struct {
        UINT16  seg_grp_num;
        UINT8   pci_bus;
        UINT8   pci_dev_func;
    } ver_3_72;
} type_203;

typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       sid_rack;
    UINT8       sid_encl;
    UINT8       sid_encl_model;
    UINT8       sid_bay;
    UINT8       encl_bays;
    UINT8       blade_bays;
    UINT8       sid_encl_sernum;
    UINT8       sid_ilo_bmc_ipaddr;
    UINT8       solutions[8];
} type_204;

#if 0 /* DEPRECATED */
typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       version;
    UINT8       images;
    struct {
        UINT32      sig;
        UINT32      addr[2];
        UINT32      length;
    } img[1];
} type_205;
#endif

#if 0 /* DEPRECATED */
typedef struct {
    SMBIOS_HDR  hdr;
    UINT16      hndl_module;
    UINT8       status;
    UINT8       reserved;
} type_206;
#endif

#if 0 /* DEPRECATED */
typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       status;
    UINT32      datetime;
} type_207;
#endif

#if 0 /* DEPRECATED */
typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       status;
} type_208;
#endif

typedef struct {
    SMBIOS_HDR  hdr;
    struct {
        UINT8   dev_fn;
        UINT8   bus;
        UINT8   mac[6];
    } nic[1];
} type_209;                    // type 209 only used c-Class Blades in Gen9 and beyond

typedef struct {
    SMBIOS_HDR  hdr;
    UINT16      watts;         // Estimated watts consumed
    UINT16      min_watts;     // Estimated watts when forced to lowest p-state
    UINT16      watts_phot_h;  // Estimated watts highest p-state (PROCHOT)
    UINT16      watts_phot_l;  // Estimated watts lowest p-state (PROCHOT)
    UINT8       unknown[3];    // Padding so we don't throw a buffer undersized error. You can redefine these bytes but DO NOT remove them.
} type_210;

typedef struct {
    SMBIOS_HDR  hdr;
    UINT16      hndl_type_4;
    UINT8       tcontrol;
} type_211;

#if 0 /* DEPRECATED */
typedef struct {
    SMBIOS_HDR  hdr;
    UINT32      sig;
    UINT32      addr[2];
    UINT32      len;
    UINT32      offs;
} type_212;
#endif

#if 0 /* DEPRECATED */
typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       ctrl;
    UINT8       type;
    UINT32      size;
    UINT32      start;
    UINT32      status;
    UINT32      addr[2];
    UINT16      len;
} type_213;
#endif

#if 0 /* DEPRECATED */
typedef struct {
    SMBIOS_HDR  hdr;
    UINT16      ver;
    UINT8       type;
    UINT8       size_ptr;
    UINT32      offs_sig;
    UINT32      offs_base;
    UINT32      offs_end;
    UINT32      offs_head;
    UINT32      offs_tail;
    UINT32      offs_high;
    UINT32      offs_low;
    UINT32      offs_last;
    UINT32      offs_counter;
} type_214;
#endif

typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       flags;
    UINT8       rate;
} type_215;

/* 26 August 2022 - type 216 updated to:
 * SMBIOS OEM Extension, v3.83
 */
typedef struct {
    SMBIOS_HDR  hdr;
    UINT16      fw_type;
#define FW_SYS_ROM           1
#define FW_REDUNDANT_SYS_ROM 2
#define FW_SYS_ROM_BBLK      3
#define FW_PWR_PIC           4
#define FW_PWR_PIC_BOOTLDR   5
#define FW_SL_CHAS           6
#define FW_SL_CHAS_BOOTLDR   7
#define FW_PAL_CPLD          8
#define FW_SPS_FIRMWARE      9 /* a.k.a. ME firmware */
#define FW_SL_CHAS_PAL_CPLD 10
#define FW_CSM              11 /* compatibility support module */
#define FW_SYS_ROM_PLATDEF  12 /* APML thermal definitions as of Gen9 carried in Sys ROM */
#define FW_STORAGE_BATT     13 /* Smart Storage Battery - Megacell */
#define FW_TPM_TCM          14 /* TPM or TCM firmware */
#define FW_NVME_PIC         15 /* NVMe Backplane PIC */
#define FW_IP               16 /* Intelligent Provisioning */
#define FW_SPI              17 /* SPI Descriptor version */
#define FW_IE               18 /* Innovation Engine Firmware */
#define FW_UBM_PIC          19 /* UBM PIC */
#define FW_ELCH_ABS         32 /* EL Chassis Abstraction Revision   */
#define FW_ELCH_FW          33 /* EL Chassis Firmware Revision      */
#define FW_ELCH_PAL         34 /* EL Chassis PAL/CPLD               */
#define FW_ELCART_ABS       35 /* EL Cartridge Abstraction Revision */
#define FW_EMB_VID_CTRL     48 /* Embedded Video Controller (iLO) */
#define FW_PCI_RISER_CPLD   49 /* PCIe Riser CPLD */
#define FW_PCI_CARD_CPLD    50 /* PCIe Card with a programmable CPLD */
#define FW_INTEL_NVME_VROC  51 /* Intel NVME VROC FW */
#define FW_INTEL_SATA_VROC  52 /* Intel SATA VROC FW */
#define FW_INTEL_SPS        53 /* Intel SPS firmware*/
#define FW_SEC_CPLD         54 /* Secondary System Programmable Logic Device */
#define FW_CPU_MEZZ_CPLD    55 /* CPU MEZZ System Programmable Logic Device */
#define FW_INTEL_ATSM       56 /* Intel Artic Sound -M (ATS-M) dual FW */
#define FW_PAL_SCP          57 /* Ampere System Control Processor (SCP - PMPro + SMPro) */
#define FW_INTEL_CFR        58 /* Intel CFR (CPU Fault Resiliency information */

/* 64..79 (0x40-0x4F) - CPLD reservations */
#define FW_CPLD_RESERVED1   64
#define FW_CPLD_RESERVED2   65
#define FW_CPLD_RESERVED3   66
#define FW_CPLD_RESERVED4   67
#define FW_CPLD_RESERVED5   68
#define FW_CPLD_RESERVED6   69
#define FW_CPLD_RESERVED7   70
#define FW_CPLD_RESERVED8   71
#define FW_CPLD_RESERVED9   72
#define FW_CPLD_RESERVED10   73
#define FW_CPLD_RESERVED11   74
#define FW_CPLD_RESERVED12   75
#define FW_CPLD_RESERVED13   76
    UINT8       sid_fw_name;
    UINT8       sid_fw_ver;
    UINT8       ver_format;
#define FORMAT_NONE       0 // display as string
#define FORMAT_PAL        1 // 2 bytes PAL/CPLD versioning
#define FORMAT_XY1        2 // encoded as one byte (XY) displayed as X.Y
#define FORMAT_SPS        3 // AAAA.BBBB.CCCC.DDDD.000E.0000 v2.07 - SPS/ME FW Formatted
#define FORMAT_XYZ1       4 // Gen9 PMC firmware where x, y, z are 4-bit hex numbers (major.minor.pass)
#define FORMAT_XYZ2       5 // x, y are 4-bit hex numbers, z is 7-bit hex
#define FORMAT_XYZ3       6 // x, y, z are 8-bit hex numbers
#define FORMAT_XYD        7 // x, y are 8-bit hex numbers, Month (byte; 1:Jan), Day (byte); Year (16 bits)
#define FORMAT_XY2        8 // X (dword) y (dword)
#define FORMAT_XYZ4       9 // x (byte), y (byte), z (word)
#define FORMAT_ABCD      10 // A (byte), B (byte), C (byte), D (build, omit if zero)
#define FORMAT_YX_T      11 // Y (word), X (word), timestamp (dword - Linux epoch) displayed as X.Y t
#define FORMAT_MmPB      12 // AAAA.BBBB.CCCC.DDDD Major.Minor.Patch.Build (all words)
#define FORMAT_X         13 // Version X (1 byte)
#define FORMAT_VROC      14 // Intel VROC Versioing as A.B.C.D ex 7.5.0.1000
#define FORMAT_INTEL_SPS 15 // AAAA.BBBB.CCCC.DDDD (XX/YY/ZZZZ) four UINT16 words then MM/DD/YYYY (1-based)
#define FORMAT_ATSM      16 // Intel Artic Sound -M Accelerator example, DG02.1.3092 format string "%1c%1c%1c%1c_%d.%d"
#define FORMAT_HEX32     17 // Intel Artic Sound -M Accelerator example, 0x11223344 format string "0x%08X" (dword)
#define FORMAT_CENTS     18 // Dollars and cents %d.%02d (leading zero for cents) - 2.06 not 2.6 (two bytes)

    union {
        UINT8   b[12]; // byte
        UINT16  w[6];  // word
        UINT32  d[3];  // dword
    } ver;

    UINT16      uniq_id;
} type_216;

#if 0 /* DEPRECATED */
typedef struct {
    SMBIOS_HDR  hdr;
    UINT16      hndl_type_4;
    UINT8       tcontrol;
} type_217;
#endif

#if 0 /* DEPRECATED */
typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       data[1];
} type_218;
#endif

/* type 219 power feature flags bitfield */
#define SMBIOS_T219_STATIC_LO           (0x00000001)
#define SMBIOS_T219_DYNAMIC             (0x00000002)
#define SMBIOS_T219_OS_CTRL             (0x00000004)
#define SMBIOS_T219_PSTATE_MON          (0x00000008)
#define SMBIOS_T219_PSTATE_FORCE        (0x00000010)
#define SMBIOS_T219_QSTATE_CAP          (0x00000020)
#define SMBIOS_T219_PROC_UTIL           (0x00000040)
#define SMBIOS_T219_STATIC_HI           (0x00000080)
#define SMBIOS_T219_PWR_METER_ROM       (0x00000100)
#define SMBIOS_T219_PWR_METER           (0x00000200)
#define SMBIOS_T219_AMD_ULTRALO         (0x00000400)
#define SMBIOS_T219_ADV_CAP             (0x00000800)
#define SMBIOS_T219_JITTER_CTRL_SUPPORT (0x00001000)
#define SMBIOS_T219_JITTER_CTRL_CONFIG  (0x00002000)
#define SMBIOS_T219_TELEM               (0x00004000)
#define SMBIOS_T219_PERF_ADV            (0x00008000)
#define SMBIOS_T219_PSYS_CAP            (0x00010000)
/* type 219 Omega/Triton Feature Flags */
#define SMBIOS_T219_GAIUS_EMBEDDED     (0x00000001)
#define SMBIOS_T219_BLACKBOX_SUPPORT   (0x00000002)
#define SMBIOS_T219_VIRT_USB_INST_DISK (0x00000004)
#define SMBIOS_T219_EMBEDDED_ACU       (0x00000008)
#define SMBIOS_T219_NO_OS_INST         (0x00000010)
#define SMBIOS_T219_IPROV_SMB          (0x00000020)
/* type 219 Misc Feature Flags */
#define SMBIOS_T219_ICRU_SUPPORT        (0x00000001)
#define SMBIOS_T219_MEMBIST_RANK        (0x00000002)
#define SMBIOS_T219_MEMBIST_HW_MEM      (0x00000004)
#define SMBIOS_T219_SABINE_CACHE        (0x00000008)
#define SMBIOS_T219_LIC_OD_FREQ_PLAT    (0x00000010)
#define SMBIOS_T219_LIC_OD_CORE_PLAT    (0x00000020)
#define SMBIOS_T219_LIC_OD_FREQ_CONF    (0x00000040)
#define SMBIOS_T219_LIC_OD_CORE_CONF    (0x00000080)
#define SMBIOS_T219_LIC_OD_FREQ_ON      (0x00000100)
#define SMBIOS_T219_LIC_OD_CORE_ON      (0x00000200)
#define SMBIOS_T219_CLASS_3_SYSTEM      (0x00000400) /* UEFI Boot only */
#define SMBIOS_T219_DRIVE_CRYPT         (0x00000800)
#define SMBIOS_T219_CLASS_2_SYSTEM      (0x00001000) /* UEFI and Legacy boot */
#define SMBIOS_T219_UEFI_DIAGS          (0x00002000)
#define SMBIOS_T219_REST_MASK           (0x0000C000)
  #define SMBIOS_T219_REST_CFG          (0x00004000)
  #define SMBIOS_T219_REST_CFG_BOOT     (0x00008000)
/* type 219 iLO cap flags */
#define SMBIOS_T219_ILO_DRVCRYPT        (0x00000001)
/* type 219 memory features flags masks - 00 notsup; 01 sup; 10 resv; 11 sup&&config */
#define SMBIOS_T219_MF_STD_ECC_MASK     (0x00000003)
#define SMBIOS_T219_MF_ADV_ECC_MASK     (0x0000000C)
#define SMBIOS_T219_MF_RANK_SPARING     (0x00000030)
#define SMBIOS_T219_MF_MIRRORING        (0x000000C0)
#define SMBIOS_T219_MF_A3DC             (0x00000300)
#define SMBIOS_T219_MF_1LM              (0x00000C00)
#define SMBIOS_T219_MF_2LM              (0x00003000)
#define SMBIOS_T219_MF_1LM_HBM          (0x0000C000)
#define SMBIOS_T219_MF_2LM_HBM          (0x00030000)

typedef struct {
    SMBIOS_HDR  hdr;
    UINT32      flags;
    UINT32      omega_triton_flags;
    UINT32      omega_triton_rsvd;
    UINT32      misc_features_flags;
    UINT32      misc_features_rsvd;
    UINT64      ilo_cap;
    UINT32      mem_features_flags;
} type_219;

#if 0 /* DEPRECATED */
typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       n_lproc;
    struct {
        UINT8       apicid;
        UINT16      hndl_type_197;
        UINT8       core;
        UINT8       threadNum;
    } proc_map[1];
} type_220;
#endif

#if 0 /* DEPRECATED */
typedef struct {
    SMBIOS_HDR  hdr;
    struct {
        UINT8   dev_fn;
        UINT8   bus;
        UINT8   mac[6];
    } nic[1];
} type_221;
#endif

typedef struct {
    SMBIOS_HDR hdr;
    UINT16 gbx_tbl_ver;
    struct {
        struct {
            UINT8 mezz;
            UINT8 swi;
        } bay[8];
    } con[1];
} type_222;

typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       index;
    UINT8       data;
    UINT8       base;
    UINT8       factory;
    UINT8       addr;
    UINT8       ctrl;
    UINT8       cfg;
} type_223;

/* 224 TPM Status bits
 * 1:0  00  Not present
 *      01  Present enabled
 *      10  present disabled
 *   2   0  No option ROM measuring
 *       1  Option ROM measuring
 * 7:3      reserved
 *
 * Extended Status (if record length indicates is present)
 * 1:0   00   Not specified
 *       01   User disabled
 *       10   Error condition (see bits 5:2)
 *       11   Reserved
 * 5:2 0000   Unspecified error
 *     0001   Self-test
 *    others  reserved
 * 7:6        reserved
 *
 * module type
 * 3:0 0000   Not specified
 *     0001   TPM 1.2
 *     0010   TPM 2.0
 *     0011   TCM 1.0 (call simply TM)
 *    others  reserved
 *   4    0   unspecified
 *        1   standard algorithms supported
 *   5    0   unspecified
 *        1   Chinese algorithms supported
 * 7:6        reserved
 *
 * module attributes
 * 1:0   00   unspecified
 *       01   pluggable and optional
 *       10   pluggable and standard
 *       11   integrated
 * 3:2   00   FIPS cert not speficied
 *       01   Not FIPS certified
 *       10   FIPS certified
 *       11   reserved
 * 7:4        reserved
 *
 * fw version handle: type 216 firmware version handle or 0xFFFE
 */
#define SMBIOS_T224_TPM_PRESENCE_MASK (0x3)
#define SMBIOS_T224_TPM_ABSENT          (0)
#define SMBIOS_T224_TPM_ENABLED         (1)
#define SMBIOS_T224_TPM_DISABLED        (2)
#define SMBIOS_T224_TPM_OPTROM_MEAS     (4)
typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       status;
    UINT8       ext_status;
    UINT8       module_type;
    UINT8       module_attributes;
    UINT16      hndl_type_216;
    UINT16      chip_vendor_id; //HPE Proliant Server SMBIOS Extensions for UEFI Systems v3.54
} type_224;

#if 0 /* DEPRECATED */
typedef struct {
    SMBIOS_HDR  hdr;
    UINT32      marker;
    UINT32      flag;
} type_225;
#endif

typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       uuid[SMBIOS_UUID_BIN_LEN];
    UINT8       sid_sn;
} type_226;

// the type tells how to read and interpret the module sensor
#define SMBIOS_T227_MCP98242  (1)
#define SMBIOS_T227_MILLBROOK (2)
#define SMBIOS_T227_JORDANCREEK  (3)

// NVDIMM Flags Value
#define NVDIMM_FLAGS_PRESENT        0x01            // This is an NVDIMM

// NVDIMM Media Controller Types - nvdimm_mc_type
#define NVDIMM_MC_TYPE_RESVD            0x00
#define NVDIMM_MC_TYPE_SMART_MODULAR    0x01
#define NVDIMM_MC_TYPE_NETLIST_4GB      0x02
#define NVDIMM_MC_TYPE_AGIGATECH        0x03
#define NVDIMM_MC_TYPE_TRUMPKIN         0x04
#define NVDIMM_MC_TYPE_AEP              SMBIOS_TYPE17_MEMTECH_INTEL_P
#define NVDIMM_MC_TYPE_REVELATION       0x7F

#define NVDIMM_TYPE_INTEL_BPS           0x1F

typedef struct {
    SMBIOS_HDR  hdr;
    UINT16      hndl_type_4;  // CPU
    UINT16      hndl_type_17; // DIMM
    UINT8       seg;
    UINT8       addr;
    UINT8       type;
    UINT8       group;        // temperature group index
    UINT8       bus; // bus number                      0xFF   if not used
    UINT8       dfn; // PCI Device/function (DDDD DFFF) 0xFF   if not used
    UINT16      reg; // PCI register offset             0xFFFF if not used
    UINT8       chan; // DIMM channel (0-based)
    UINT16      nvdimm_flags;   // 0b = Type 1, [02b - 31b] reserved
    UINT8       nvdimm_mc_addr; // NVDIMM Media Contrller I2C Slave Address
    UINT16      nvdimm_mc_type; // NVDIMM Media Ctlr Type
} type_227;

/************************************************************
 * type_228 is deprecated in Gen10 / iLO 5 in favor of APML *
 ************************************************************
 * the types tell now to interpret the union fields including
 * the method and signals used to alter the MUX
 * and information about transitions between PECI/I2C */
#define SMBIOS_T228_TYPE_CPLD     (1)
#define SMBIOS_T228_TYPE_GPO      (2)
#define SMBIOS_T228_TYPE_SCHAIN   (3)
#define SMBIOS_T228_TYPE_I2C      (4)
#define SMBIOS_T228_TYPE_PECI_I2C (5)
#define SMBIOS_T228_TYPE_PECI     (6)
#define SMBIOS_T228_TYPE_I2CEX    (7)
#define SMBIOS_T228_TYPE_PECI_G8  (8)
#define SMBIOS_T228_TYPE_CARB     (9)
#define SMBIOS_T228_TYPE_IE       (10)
#define SMBIOS_T228_TYPE_IPMB     (11)
#define SMBIOS_T228_TYPE_ARM      (12)
/* Info is a bitflag indicating what might be present on the segment */
#define SMBIOS_T228_INFO_HDD_BP (0x01)
#define SMBIOS_T228_INFO_MB     (0x02)
#define SMBIOS_T228_INFO_DISCO  (0x04)
#define SMBIOS_T228_INFO_VMAIN  (0x08)   // Only Main Power Access
/************************************************************
 * type_228 is deprecated in Gen10 / iLO 5 in favor of APML *
 ************************************************************/
typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       seg;
    struct  {
       UINT8   type;
       union {
          struct {
             UINT8   xreg;
             UINT8   mask_clr;
             UINT8   mask_set;
             UINT8   res[2];
          }  cpld;
          struct {
             UINT8   byte;
             UINT8   mask_clr;
             UINT8   mask_set;
             UINT8   res[2];
          }  gpo;
          struct {
             UINT8   byte;
             UINT8   mask_clr;
             UINT8   mask_set;
             UINT8   res[2];
          }  schain;
          struct {
             UINT8   addr;
             UINT8   offs;
             UINT8   mask_clr;
             UINT8   mask_set;
             UINT8   res;
          }  i2c;
          struct {
             UINT8   seg;
             UINT8   addr;
             UINT8   offs;
             UINT8   mask_clr;
             UINT8   mask_set;
          }  i2cex;
          struct {
             UINT8   mux_addr;
             UINT8   mux_offs;
             UINT8   mask_clr;
             UINT8   mask_set;
             UINT8   dev_addr;
          }  peci;
          struct {
             UINT8   domain;        //domain cpu target addr(30,31...)
             UINT8   pci_bus;       //smbus (1)
             UINT8   pci_dev_fnc;   //smbus dev/function(15/0)
             UINT16  smbus_offset;  //smbus regsiter offset(0x180/190)
          }  peci_2;    //G8 peci type 8
          struct {
             UINT8   byt[5];
          } unknown;
       } select;
    } mux;
    UINT8       parent;
    UINT8       info;
    UINT8       res;
} type_228;


#define SMBIOS_T229_SIG_HDD  (0x44444824) /* $HDD */
#define SMBIOS_T229_SIG_OCSD (0x53434f24) /* $OCS */
#define SMBIOS_T229_SIG_WHEA (0x45485724) /* $WHE */
/* $OCB  (OCBB)
 * $CRP  (CONREP)
 * $DFC  (DFLT_CFG)
 * $SAE  (Array Crypt)
 * $SMV  (SMI count value)
 * $ZXT  (ZX table)
 * $XFB  (Xfer buffer)
 * $RAC  (RAID cache)
 */
typedef struct {
    SMBIOS_HDR  hdr;
    struct {
       UINT32    sig;
       UINT32    loc[2];
       UINT32    size;
    } entry[1];
} type_229;

#define SMBIOS_T230_FRU_NA   (0)
#define SMBIOS_T230_FRU_IPMI (1)
#define SMBIOS_T230_FRU_ILO  (2)
typedef struct {
    SMBIOS_HDR  hdr;
    UINT16      hndl_39;
    UINT8       sid_manu;
    UINT8       sid_rev;
    UINT8       fru;
    UINT8       bus;
    UINT8       addr;
} type_230;

#if 0 /* DEPRECATED */
typedef struct {
    SMBIOS_HDR  hdr;
    UINT8       bios_type;
#define BIOS_LEGACY    0
#define BIOS_UEFI      1
    UINT8       spi;
#define SPI_INTEL      0
#define SPI_AMD_ATI    1
#define SPI_AMD_NVIDIA 2
    UINT16      sz_rom;
    UINT16      sz_img;
    UINT16      sz_nonbios;
    UINT16      sz_updatable;
    UINT16      sz_bblk;
    UINT16      ver_me;
    UINT8       regions;
#define REGION_FDT     0x01
#define REGION_GBE     0x02
#define REGION_PD      0x04
#define REGION_ME      0x08
#define REGION_BIOS    0x10
    UINT32      mpm_settings;
    UINT32      mpm_int15;
} type_231;
#endif

typedef struct {
    SMBIOS_HDR  hdr;
    UINT16      hndl_17;           // 04h
#define T232_ATT_SMARTMEM (0x00000001)
#define T232_ATT_STANDMEM (0x00000030)
    UINT32      att;               // 06h
    UINT16      v_min;             // 0Ah
    UINT16      v_cfg;             // 0Ch
    UINT16      mode;              // 0Eh DIMM persistence type - supported modes
    UINT16      cfg;               // 10h DIMM persistence type - configured
    UINT16      inc_sz;            // 12h Allocation
    UINT16      align_sz;          // 14h Allocation
    UINT32      reg_sz_max_vol;    // 16h Region Size Max Volatile
    UINT32      reg_sz_max_non;    // 1Ah Region Size Max Non-Volatile
    UINT8       region_cnt;        // 1Eh Configured Region Count
    UINT8       region_limit_pers; // 1Fh Region Count Limit (Persistent)
    UINT8       region_limit_vol;  // 20h Region Count Limit (Volatile)
    UINT8       security_st;       // 21h DIMM Security State
    UINT8       map_out;           // 22h DIMM Map-Out Reasons
#define T232_DIMM_NOT_ENCRYPTED            (0x00)   
#define T232_DIMM_ENCRYPTED                (0x01)   
#define T232_DIMM_ENCRYPTED_STATUS_UNKNOWN (0x02)   
#define T232_DIMM_ENCRYPTION_UNSUPPORTED   (0x03)   
    UINT8       encrypt_status;    // 23h DIMM Encryption status as reported by BIOS.
    UINT8       null_term[2];      // Unused Null characters which conclude every SMBIOS record.
} type_232;

typedef struct {
    SMBIOS_HDR   hdr;
    UINT16       seg_grp_num;
    UINT8        bus_num;
    UINT8        dev_fn_num;
    UINT8        mac_addr[32];
    UINT8        port;
} type_233;

typedef struct {
    SMBIOS_HDR   hdr;
    UINT16       hndl_228;
    UINT8        i2c_addr;
#define T234_DEV_MC        (0x0) // Megacell (mc_data)
#define T234_DEV_EFUSE     (0x1) // E-Fuse   (pmbus_data)
#define T234_DEV_Q_COMP    (0x2) // Quad Comparator (pmbus_data)
#define T234_DEV_VRD       (0x3) // VRD Controller  (pmbus_data)
#define T234_DEV_MULTI_VRD (0x4) // MultiPhase VRD Controller (pmbus_data)
#define T234_DEV_BP_PIC    (0x5) // NVMe BP PIC - premium backplane PIC (no additional data)
    UINT8        dev_type;
    union {
        struct {
            UINT8    ver;
            UINT8    fru_i2c_addr;
            UINT16   load_cap;
            UINT16   tot_req;
#define MC_INFO_POST_PRESENT (0x1) // Megacell present during POST
#define MC_INFO_POST_ENABLED (0x2) // Megacell output enabled during POST
#define MC_INFO_CENTRALIZED  (0x4) // Megacell resides on Carbondale
            UINT8    info;
            UINT8    rsvd;
        } mc_data;
        struct { /* PMBus Specific - deprecated Gen 11 and later as of spec v3.83 */
            UINT8    ver;
            UINT8    flt_table_class;
            UINT8    flt_table_byte;
            UINT8    flt_table_bit;
        } pmbus_data;
    } dev_data;
} type_234;

/* type 235: This structure type is not used on HP ProLiant Servers.
 * See ProLiant Server SMBIOS OEM Extensions for UEFI Systems v3.12
 * or later for details */

// This record (236) is used to communicate the HardDrive Backplane FRU information to the host.
// This information is provided to the System ROM by a UEFI storage device via the UEFI Adapter
// Information Protocol. This record will be duplicated for each instance of a HDD FRU detected in the server.
typedef struct {
    SMBIOS_HDR   hdr;
    UINT8        bp_fru_i2c_addr;
    UINT16       bp_box_num;
    UINT16       bp_nvram_id;
    UINT8        sas_expander_wwid[8]; // SAS Expander (Shriner) WWID (binary data.) 00h when no Shriner is present.
    UINT8        total_sas_bays;
    UINT8        port0xA0_sas_bay_count;    // removed in HPE Specification ProLiant Server SMBIOS OEM Extensions for UEFI Systems Version 3.69
    UINT8        port0xA2_sas_bay_count;    // removed in HPE Specification ProLiant Server SMBIOS OEM Extensions for UEFI Systems Version 3.69
    UINT8        sid_bp_name;               // removed in HPE Specification ProLiant Server SMBIOS OEM Extensions for UEFI Systems Version 3.69
} type_236;

// This record (237) is used to communicate additional DIMM information to HP
// Service Support. The Type 17 is the "Public Data". This is more private.
typedef struct {
    SMBIOS_HDR   hdr;
    UINT16       t17_hndl;    // Handle to associated Type 17 Record
    UINT8        sid_mfg;     // String ID to Manufacturer name
    UINT8        sid_part;    // String ID to Manufacturer part Number
    UINT8        sid_vsn;     // Vendor serial number
    UINT16       mfg_date;    // DIMM manufacture date YYXX in BCD where
                              // YY is year and XX is week number - 0x2133 is week 33 of year 2021
} type_237;

/* type 238 HP USB Port Connector Correlation Record
 * provides a mechanism for software to correlate USB Port information from Type 8.
 * It additionally provides device specific data that is typically not available */
typedef struct {
    SMBIOS_HDR   hdr;
    UINT16       t8_hndl;           // Handle to associated Type 8 (port connector) Record
    UINT8        bus;               // PCI bus for host controller for the port
    UINT8        dev_fn;            // PCI device and function; Device (Bits 7:3); Function (Bits 2:0)
    UINT8        loc;               // location enumeration (others reserved)
#define T238_LOC_INT      0 // internal
#define T238_LOC_FRONT    1 // front of server
#define T238_LOC_REAR     2 // rear of server
#define T238_LOC_INTERNAL 3 // Embedded internal SD Card
#define T238_LOC_ILO      4 // iLO USB
#define T238_LOC_NAND     5 // HP NAND Controller (USX 2065 or other)
#define T238_LOC_DEBUG    7 // Debug Port
#define T238_LOC_OCP      9 // OCP USB
    UINT16       flags;             // bits 1:0 == 00:not shared for mgmt; 01:shared via MUX; 10:auto MUX; 11:resv
    UINT8        inst;              // port instance (1-based)
    UINT8        hub;               // hub instance
    UINT8        speed;             // 00:resv; 01:1.1; 02:2.0; 03:3.0 else reserved
    UINT8        sid_path;          // string ID UEFI device path of USB endpoint
} type_238;

/* type 239: HPE USB Device Correlation Record */
typedef struct {
    SMBIOS_HDR   hdr;
    UINT16       t238_hndl;
    UINT16       vendor;
    UINT16       flags;     // 0x0001: SD card in the embedded SD controller
    UINT8        d_class;
    UINT8        d_sub;
    UINT8        d_prot;
    UINT16       prodid;
    UINT32       capacity;  // capacity in MB
    UINT8        sid_path;  // UEFI device path
    UINT8        sid_sname; // UEFI device structured name
    UINT8        sid_name;  // device name
    UINT8        sid_loc;   // device location
} type_239;


/* type 240: HP Firmware Inventory Record (ISS SMBIOS for UEFI v3.33)
 * mechanism for software to report firmware version information for devices
 * that report their firmware using their UEFI drivers. */
typedef struct {
    SMBIOS_HDR   hdr;
    UINT16       hndl_assoc;        // Handle to associated Type 202 or 203 (device correlation) or 224 Record
    UINT32       ver;               // numerical version
    UINT8        sid_fw_ver;        // string for firmware version
    UINT64       sz;                // firmware size, in bytes
    UINT32       att_def[2];        // firmware attributes defined
    UINT32       att_val[2];        // firmware attributes settings
#define IMAGE_ATTRIBUTE_IMAGE_UPDATABLE          0x0000000000000001
#define IMAGE_ATTRIBUTE_RESET_REQUIRED           0x0000000000000002
#define IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED  0x0000000000000004
#define IMAGE_ATTRIBUTE_IN_USE                   0x0000000000000008
#define IMAGE_ATTRIBUTE_UEFI_IMAGE               0x0000000000000010
    UINT32       min;               // lowest supported version
} type_240;

/* type 242: Hard Drive Inventory Record for direct-attached NVMe and SATA drives */
#define VER_242_PRE_3_63 0x2A
#define VER_242_PRE_3_67 0x2B
#define VER_242_PRE_3_75 0x3D
typedef struct {
    SMBIOS_HDR   hdr;
    UINT16       hndl_assoc;  // type 203 device association rec
    UINT8        type;        // 0:unk; 1:NVMe; 2:SATA; 3:SAS; 4:SSD
#define T242_TYPE_UNKNOWN   0 // unknown
#define T242_TYPE_NVME      1 // NVME
#define T242_TYPE_SATA      2 // SATA
#define T242_TYPE_SAS       3 // SAS
#define T242_TYPE_SSD       4 // SSD
    UINT64       uid;         // HDD unique identifier
    UINT32       cap;         // device capacity in MB
    UINT8        poh[16];     // power on hours
    UINT8        used;        // percentage used; vendor specfic
    UINT8        watts;       // wattage; 0:unknown
#define T242_FF_3_5     0x02
#define T242_FF_2_5     0x03
#define T242_FF_1_8     0x04
#define T242_FF_LESS_1_8 0x05
#define T242_FF_MSATA   0x06
#define T242_FF_M2      0x07
#define T242_FF_MICRO_SSD 0x08
#define T242_FF_CFAST   0x09
    UINT8        ff;          // form factor: 2:3.5; 3:2.5; 4:1.8
#define T242_HEALTH_OK      0x00
#define T242_HEALTH_WARNING 0x01
#define T242_HEALTH_CRITICAL 0x02
#define T242_HEALTH_UNKNOWN 0xFF
    UINT8        health;      // HDD health: 0:ok, 1:warn, 2:critical
    UINT8        sid_sn;
    UINT8        sid_model;
    UINT8        sid_rev;
    struct
    {
        UINT8    sid_loc;
    } ver_3_63;
#define T242_NOT_ENCRYPTED              0x00
#define T242_ENCRYPTED                  0x01
#define T242_ENCRYPTION_STATUS_UNKNOWN  0x02
#define T242_ENCRYPTION_NOT_SUPPORTED   0x03
    struct
    {
        UINT8    encrypt_status;
    } ver_3_67;
    struct
    {
        UINT64       capacity_bytes;
        UINT32       block_size;
        UINT16       rotational_speed;
        UINT16       negotiated_speed;
        UINT16       capable_speed;
    } ver_3_75;
} type_242;

/* type 243: Correlates a type 216 FW Version to Device/Target GUIDs */
typedef struct {
    SMBIOS_HDR   hdr;
    UINT16       t216_hndl;    // type 216 device association rec
    UINT8        device_guid[16];
    UINT8        target_guid[16];
} type_243;

/* type 244: DIMM current configuration record (NVDIMM) */
#define VER_244_PRE_3_62 0x0E
typedef struct {
    SMBIOS_HDR   hdr;
    UINT16       t17_hndl;    // type 17 DIMM (physical)
    UINT8        region_id;
    UINT16       region_config;  // Region Config Type
    UINT64       region_mem_sz;
    UINT8        passphrase_st;
    UINT16       interleave_idx;
    struct
    {
        UINT8        interleave_cnt;
#define T244_INT_HEALTH_HEALTHY      0x00
#define T244_INT_HEALTH_DIMM_MISSING 0x01
#define T244_INT_HEALTH_CFG_INACTIVE 0x02
#define T244_INT_HEALTH_SPA_MISSING  0x03
#define T244_INT_HEALTH_NEW_GOAL     0x04
#define T244_INT_HEALTH_LOCKED       0x05
        UINT8        interleave_health;
    } ver_3_62;
} type_244;

/* type 245: Extension Board Inventory Record*/
typedef struct {
    SMBIOS_HDR   hdr;
#define T245_EXT_BOARD_TYPE_PCIE_RISER                  0
    UINT8        ext_board_type;
    union {
       struct {
#define T245_EXT_BOARD_TYPE_PCIE_RISER_PRIMARY          1
#define T245_EXT_BOARD_TYPE_PCIE_RISER_SECONDARY        2
#define T245_EXT_BOARD_TYPE_PCIE_RISER_TERTIARY         3
#define T245_EXT_BOARD_TYPE_PCIE_RISER_QUATERNARY       4
#define T245_EXT_BOARD_TYPE_PCIE_RISER_FRONT            10
          UINT8       riser_position;
          UINT8       riser_id;
#define T245_EXT_BOARD_TYPE_PCIE_RISER_NO_CPLD          0
#define T245_EXT_BOARD_TYPE_PCIE_RISER_CPLD_VER_MASK    0x3F   //bit[6:0] CPLD version
#define T245_EXT_BOARD_TYPE_PCIE_RISER_CPLD_IND         0x80  //bit7 0-formal 1-bootleg
          UINT8       riser_version;
          UINT8       riser_index;
       }pcie_riser;
    } member_union;//Board_spec_data
} type_245;

#pragma pack()

extern int smbios_decode_216(type_216 * t216, char *fw_name, int fw_name_len, char *fw_ver,  int fw_ver_len);
extern int smbios_get_i2c_loc_str(UINT16 t228_hndl, UINT8 *loc_str, size_t loc_str_sz, UINT8 *rec_type, UINT8 *smbios_rec);

#endif
