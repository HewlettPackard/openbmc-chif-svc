/*
// Copyright (c) 2016, 2019, 2021-2025 Hewlett Packard Enterprise Development, LP
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

/* Module definitions */

#ifndef  IODNETCFG_H
#define  IODNETCFG_H

//#include <integrity.h>

//#include "phydefs.h"


#define IODNETCFG_NUMBER_OF_INTFS           (2)
#define IODNETCFG_NUMBER_OF_STATIC_ROUTES   (3)
#define IODNETCFG_NUMBER_OF_DNS_SERVERS     (3)
#define IODNETCFG_NUMBER_OF_WINS_SERVERS    (2)
#define IODNETCFG_NUMBER_OF_SNTP_SERVERS    (2)
#define IODNETCFG_VERSION                   (0x00000003)
#define IODNETCFG_HOST_NAME_SIZE            (63)
#define IODNETCFG_DOMAIN_NAME_SIZE          (193)
#define IODNETCFG_MAIN_RESERVED_BYTES       (999)
/*
    Should be defined identically to NICTXRX_NUM_UMACS. Defined here
    rather than including nictxrx.h to prevent the double declaration
    of EnetDevice, defined in ethdriver.h and enet_iodevice.h
*/
#define IODNETCFG_NUM_UMACS               (2)

/* bitmasks and values */
/* bitmask definitions for cfg_nic.iface[].settings */
#define IODNETCFG_F_RESERVED0             (0x00000001)   /* unused */
#define IODNETCFG_F_AUTOSELECT            (0x00000002)   /* autoselect network settings (speed and duplex) */
#define IODNETCFG_F_FORCE_100MB           (0x00000004)   /* if no autoselect, use 100MB else 10 */
#define IODNETCFG_F_FORCE_1GB             (0x00000008)   /* if no autoselect, use 1000MB else 10 */
#define IODNETCFG_F_FORCE_FULL_DUPLEX     (0x00000010)   /* if no autoselect, use full else half */
#define IODNETCFG_F_RESERVED1             (0x00000020)   /* unused */
#define IODNETCFG_F_VLAN_ENABLED          (0x00000040)   /* if enable VLAN filtering on sideband systems */
#define IODNETCFG_F_IPV4_ENABLED          (0x00000080)   /* IPv4 enabled by default */
#define IODNETCFG_F_IPV6_ENABLED          (0x00000100)   /* IPv6 enabled by default */
#define IODNETCFG_F_REGISTER_WITH_WINS    (0x00000200)   /* register with WINS server (not DHCP related) */
#define IODNETCFG_F_REGISTER_WITH_DNS     (0x00000400)   /* register with dynamic DNS */
#define IODNETCFG_F_PING_GATEWAY_ON_BOOT  (0x00000800)   /* ping gateway on boot */
#define IODNETCFG_F_RESERVED2             (0x00001000)   /* unused */
#define IODNETCFG_F_IGNORE_SLAAC_DURING_RA (0x00002000)   /* Do not configure a SLAAC address during router discovery (IPv6)*/
#define IODNETCFG_F_USE_DUPLICATE_IP_ADDRESS_DETECTION  (0x00004000)
#define IODNETCFG_F_DEFAULTS_CONFIGURE    (0x80000000)   /* This bit indicates the dynamic download has made adjustments to
                                                            the settings to adjust default settings that had been overwritten
                                                            due to corrupted memory or other kernel action to reset to
                                                            kernel based defaults.  The kernel will clear this bit when
                                                            setting defaults. */

/* bitmask definitions for cfg_nic_type.dhcp_options */
#define IODNETCFG_F_DHCP_ENABLED          (0x00000001) /* use DHCP supplied IP address instead of static */
#define IODNETCFG_F_DHCP_GATEWAY          (0x00000002) /* use DHCP supplied gateway instead of static */
#define IODNETCFG_F_DHCP_DNS_SERVERS      (0x00000004) /* use DHCP supplied DNS servers instead of static */
#define IODNETCFG_F_DHCP_STATIC_ROUTES    (0x00000008) /* use DHCP supplied static routes */
#define IODNETCFG_F_DHCP_WINS_SERVERS     (0x00000010) /* use DHCP supplied WINS servers */
#define IODNETCFG_F_DHCP_DOMAIN_NAME      (0x00000020) /* use DHCP supplied domain name */
#define IODNETCFG_F_DHCP_NTP_SERVERS      (0x00000040) /* use DHCP supplied NTP servers */
#define IODNETCFG_F_DHCP_CLIENTID_OVR     (0x00000080) /* DHCP ClientID Override*/

/* bitmask definitions for cfg_nic_type.dhcp_status */
/* THIS MUST MATCH UP WITH BITFIELD DEFINITIONS FOR DHCP_STATUS TYPE*/
#define IODNETCFG_F_DHCPSTAT_IPADDR     (0x00000001)   /* Got an IP address from DHCP */
#define IODNETCFG_F_DHCPSTAT_GATEWAY    (0x00000002)   /* Got a Gateway Router address from DHCP */
#define IODNETCFG_F_DHCPSTAT_DNS        (0x00000004)   /* Got DNS server addresses from DHCP */
#define IODNETCFG_F_DHCPSTAT_ROUTES     (0x00000008)   /* Got Static Routes information from DHCP */
#define IODNETCFG_F_DHCPSTAT_WINS       (0x00000010)   /* Got WINS server addresses from DHCP */
#define IODNETCFG_F_DHCPSTAT_DOMAIN     (0x00000020)   /* Got domain name from DHCP */
#define IODNETCFG_F_DHCPSTAT_NTP        (0x00000040)   /* Got NTP server from DHCP */


/* bitmask definitions for cfg_nic_type.status */
/* THIS MUST MATCH UP WITH BITFIELD DEFINITIONS FOR NET_STATUS TYPE*/
#define IODNETCFG_F_IPADDR              (0x00000001)   /* IP address and netmask configured */
#define IODNETCFG_F_GATEWAY             (0x00000002)   /* Default gateway configured */
#define IODNETCFG_F_DNS                 (0x00000004)   /* DNS servers configured */
#define IODNETCFG_F_ROUTES              (0x00000008)   /* Static Routes configured */
#define IODNETCFG_F_WINSREG             (0x00000010)   /* Registered with WINS servers */
#define IODNETCFG_F_DDNSREG             (0x00000020)   /* Registered name with DDNS server */
#define IODNETCFG_F_DUPIPDETECTED       (0x00000040)   /* Duplicate IP address detected on this interface */

//
// These control whether or not the kernel will perform this action
//
#define IODNETCFG_K_STATIC_ROUTES                  (0x00000001)
#define IODNETCFG_K_DHCP_FINISH_UPDATE_NVRAM       (0x00000002)
#define IODNETCFG_K_DHCP_HOSTNAME_SAME_ALL_INTF    (0x00000004)
#define IODNETCFG_K_STACK_ENABLE                   (0x00000008)
#define IODNETCFG_K_NTP_SERVERS                    (0x00000010)
// #define IODNETCFG_K_             (0x00000020)
// #define IODNETCFG_K_             (0x00000040)
#define IODNETCFG_K_LINK_STATUS_EFFECTS_DHCP       (0x00000080) // either interface
#define IODNETCFG_K_KERNEL_FTPD                    (0x00000100) // Only valid on first interface
#define IODNETCFG_K_IPV6_INACTIVE                  (0x00000200) // either interface
#define IODNETCFG_K_LINK_STATUS_EFFECTS_DUP_IP     (0x00000400) // either interface
#define IODNETCFG_K_TESTNCSI_INACTIVE              (0x00000800) // Only valid on first interface
//
// These defines describe the link status word
//
#define IODNETCFG_F_LINK_UP             (0x00000001)
#define IODNETCFG_F_LINK_DOWN           (0x00000002)
#define IODNETCFG_F_LINK_AUTO           (0x00000004)
#define IODNETCFG_F_LINK_10MBPS         (0x00000010)
#define IODNETCFG_F_LINK_100MBPS        (0x00000020)
#define IODNETCFG_F_LINK_1000MBPS       (0x00000040)
#define IODNETCFG_F_LINK_10GBPS         (0x00000080)
#define IODNETCFG_F_LINK_DUPLEX_HALF    (0x00000100)
#define IODNETCFG_F_LINK_DUPLEX_FULL    (0x00000200)
#define IODNETCFG_F_LINK_100GBPS        (0x00000400)
#define IODNETCFG_F_LINK_40GBPS         (0x00000800)

#define IODNETCFG_PARAMS_SIZE           (64)
#define IODNETCFG_DHCP_CLIENTID_MAX         (16)
#define IODNETCFG_IFACE_RESERVED_BYTES      (75)

//
//  sideband_sel values
//
#define IODNETCFG_SIDEBAND_NONE     (0)
#define IODNETCFG_SIDEBAND_ALOM     (1)
#define IODNETCFG_SIDEBAND_LOM      (2)
#define IODNETCFG_SIDEBAND_RESERVED (3)

//  Gen11 sideband_sel values
// [1:0] - Meant for OCP1/OCP2/EMBEDDED_NIC
#define IODNETCFG_SIDEBAND_DISABLED  (0)
#define IODNETCFG_SIDEBAND_OCP1      (1)
#define IODNETCFG_SIDEBAND_OCP2      (2)
#define IODNETCFG_SIDEBAND_EMB_NIC   (3)

//Used in the array containing the details of OCP/SMARTNIC 
#define OCP1_ID       0
#define OCP2_ID       1
/*
  Bitmask definitions for the main settings field, (IODNETCFG_V3_CFG.settings), the one
  outside of the per interface structure
*/
#define IODNETCFG_M_PRIMARY_ACTIVE       (0x00000001)   /* Is the primary interface active */
#define IODNETCFG_M_SECONDARY_ACTIVE     (0x00000002)   /* Is the secondary interface active */
/* 
  This next bit is the install status, at the previous iLO boot, of the optional dedicated
  NIC card, for servers that have the Networking enablement option platform attribute set.
  These servers require that we rewrite SRAM to change the enabled NIC interface whenever
  that card is installed or removed.
 
  When the card is installed we switch to the Dedicated NIC interface. When the card is
  removed we disable both NIC interfaces.
 
  Once we change the enabled interface we update this bit so that SRAM is rewritten only
  once per optional NIC card installation status change.
 
  When this bit is:
    0 - Card was not installed
    1 - Card was installed
 
  The idea behind this is that the customer has to buy this card to use any iLO NIC.
*/
#define IODNETCFG_M_LAST_NIC_OPTION_STATUS (0x00000004)  /* The install status for optional NIC card */


/* structures */

typedef struct
    {
    uint32_t   dest;
    uint32_t   gate;
    uint32_t   mask;
    } IODNETCFG_ROUTE;

typedef struct // 0x200 bytes
    {                                                           // offset from beginning
    uint32_t           settings;                                   // 0
    uint32_t           status;                                     // 4
    uint32_t           dhcp_options;                               // 8
    uint32_t           dhcp_status;                                // 12
    uint32_t           ipaddr;                                     // 16
    uint32_t           ip_mask;                                    // 20
    uint32_t           gateway_ip;                                 // 24
    uint32_t           wins_ip[IODNETCFG_NUMBER_OF_WINS_SERVERS];  // 28
    uint32_t           dns_ip[IODNETCFG_NUMBER_OF_DNS_SERVERS];    // 36
    uint32_t           ntp_ip[IODNETCFG_NUMBER_OF_SNTP_SERVERS];   // 48
    IODNETCFG_ROUTE    route[IODNETCFG_NUMBER_OF_STATIC_ROUTES];   // 56
    uint16_t           vlan_id;                                    // 92
    uint16_t           QoS;                //unused currently      // 94
/*
  62 bytes for the name and a trailing null to make strcpy work. Per the relevant RFC
  a label can be 63 octets, one of which is the length. Since the length isn't stored
  here, but the null is, 62 characters are allowed for the host name.
*/
    char            host_name[IODNETCFG_HOST_NAME_SIZE];        // 96
/*
  63 octets for the name leaves 192 octets left for the maximum size of the rest of the
  DNS field. We've also allowed a trailing null to make strcpy usable.
*/
    char              domain_name[IODNETCFG_DOMAIN_NAME_SIZE];    // 159
    uint8_t           dhcp_params[IODNETCFG_PARAMS_SIZE];         // 352
    uint8_t           dhcp_clientid_len;                          // 416
    uint8_t           dhcp_clientid_value[IODNETCFG_DHCP_CLIENTID_MAX]; //417
    uint8_t           reserved[IODNETCFG_IFACE_RESERVED_BYTES];   // 433
    uint32_t           kernel;                                     // 508
    } IODNETCFG_V3_INTF;                                        // 512 total

typedef struct // 0x400 + (2 * 0x200)
    {
    uint32_t       magic;
    uint32_t       version;
    uint32_t       checksum;
    uint32_t       sequence;
    uint32_t       settings;
/*
  physel only valid in failover mode
*/
    uint8_t       physel;       // 0 - UMAC 0 connected to PHY 0
                              // 1 - UMAC 0 connected to PHY 1
    uint8_t       sideband_sel; // valid values are 0 - no sideband selected
                              //                  1 - ALOM/OCP1(>=Gen11)
                              //                  2 - EmbeddedNIC-LOM/OCP2(>=Gen11)
                              //                  3 - no sideband selected/EmbeddedNIC(>=Gen11)
    uint8_t       channel;      // Contains the package/channel number.
                              // Valid values are 0 thru max_channels-1
    uint8_t       package;      // Currently only package 0 is supported, so this field is
                              // ignored
    uint8_t       hidden_port;  // Which port to hide from the OS for NC-SI devices
    uint8_t       reserved[IODNETCFG_MAIN_RESERVED_BYTES];
    IODNETCFG_V3_INTF    iface[IODNETCFG_NUMBER_OF_INTFS]; // starts 0x400 from the beginning of the structure
    } IODNETCFG_V3_CFG, IODNETCFG_CFG;

#define IODNETCFG_CFG_MAGIC (0x439cd202)

#define IODNETCFG_DHCP_BUFFER_SIZE (1600)

#define IODNETCFG_SRAM_LENGTH  (2 * sizeof (IODNETCFG_CFG))

extern IODNETCFG_V3_CFG   iodnetcfg_storage;


typedef struct
    {
    uint32_t link;
    uint32_t speed;
    uint32_t duplex;
    } IODNETCFG_LINK_BUFFER;

//-------------------------------------------
// Smart NIC related details

#define MAX_SMARTNICS           0x2

// Register number for getting details of Smart NICs
#define SMARTNIC                0xA 

#define SMARTNIC_ID(id) (SMARTNIC+id)

// Smart NIC Operations
#define SMARTNIC_STOP           0x0 
#define SMARTNIC_START          0x1
#define SMARTNIC_SWIPE          0x2
#define SMARTNIC_GSHUTDOWN      0x3
#define SMARTNIC_GRESTART       0x4
#define SMARTNIC_PROGRESS_AEN   0x5
#define SMARTNIC_SWIPE_STS      0x6
#define SMARTNIC_OS_INST_STS    0x7
#define SMARTNIC_LIST           0x8
#define SMARTNIC_USER_CREDENTIALS 0x9
#define SMARTNIC_RESET          0xA

#define USERNAME_LEN            32
#define TOKEN_LEN               39

//Smart NIC Vendor
#define    SNIC_VENDOR_UNKNOWN    0x0
#define    SNIC_VENDOR_PENSANDO   0x1
#define    SNIC_VENDOR_NVIDIA     0x2

//Smart NIC Status
#define    SNIC_STATUS_NOTREADY      0x0
#define    SNIC_STATUS_IDLE          0x1
#define    SNIC_STATUS_INITIALIZING  0x2
#define    SNIC_STATUS_OPERATIONAL   0x3
#define    SNIC_STATUS_ERROR         0x4

//Smart NIC Wipe Status
#define    SNIC_WIPE_SUCCESS         0x0
#define    SNIC_WIPE_NOT_STARTED     0x1
#define    SNIC_WIPE_IN_PROGRESS     0x3
#define    SNIC_WIPE_FAILED          0x5

//OS Installation Status
#define    OS_INSTALL_NOT_STARTED    0x0
#define    OS_INSTALL_IN_PROGRESS    0x1
#define    OS_INSTALL_COMPLETE       0x2

//OS Intallation Type
#define    INVALID_TYPE              0x0
#define    IMAGE                     0x1
#define    BOOT_PROGRESS             0x2

#pragma pack(1)
typedef struct 
{
  uint8_t    id;
  uint8_t    vendor; // 0 - SNIC_VENDOR_UNKNOWN; 1 - SNIC_VENDOR_PENSANDO; 2 - SNIC_VENDOR_NVIDIA;
  uint8_t    status; // 0 - SNIC_STATUS_NOTREADY; 1 - SNIC_STATUS_IDLE; 2 - SNIC_STATUS_INITIALIZING; 3 - SNIC_STATUS_OPERATIONAL 4 - SNIC_STATUS_ ERROR;
  uint8_t    reserved;
} SMARTNIC_CFG;

typedef struct
{
    union
    {
        uint16_t value;
        struct
        {
            uint16_t smart_nic_caps        : 1;
            uint16_t esxio_capable         : 1;
            uint16_t supply_mac_address    : 1;
            uint16_t receive_mac_address   : 1;
            uint16_t ipv6_slac             : 1;
        }s1;
    }u1;
} EXT_FLAGS;

typedef struct
{
    uint8_t      no_of_channels;
    uint8_t      mgmt_channel;
    EXT_FLAGS    ext_flags;
    uint8_t      mgmt_mac [6] ; 
    uint16_t     reserved1;
    uint16_t     reserved2;
} EXT_CAPS;

typedef struct
{
  uint8_t type ;
  uint8_t progress_sts ;
  uint16_t progress_code ;
} PROGRESS_AEN;

typedef struct
{
  uint8_t  progress_sts ;
  uint16_t progress_code ;
  uint8_t  reserved;
} SWIPE_STATUS;

typedef struct
{
  uint8_t user_name[USERNAME_LEN];
  uint8_t token[TOKEN_LEN];
  uint8_t reserved;
} USER_CREDENTIALS;

#pragma pack()

//-------------------------------------------

extern void iodnetcfg_link_status_update (int intf, uint32_t link, uint32_t speed, uint32_t duplex);

#endif


