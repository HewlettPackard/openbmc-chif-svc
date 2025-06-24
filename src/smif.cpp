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
#include <stddef.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include "ev.hpp"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include "iodnetcfg.h"
#include <phosphor-logging/log.hpp>
#include "strutil.hpp"
#include "platdef_api.hpp"
#include "i2c_topology.hpp"
#include "i2c_return_codes.hpp"
#include "i2c_mapping.hpp"
#include "gpio.h"
#include "DataExtract.h"
#include "logs.h"
#include "misc.hpp"

#define EVT_IML 0x01
#define EVT_IEL 0x02
#define EVT_SL  0x04  // Security Log

//
// Event Severities
//
#define EVT_INFORMATIONAL 2
#define EVT_REPAIRED 6
#define EVT_CAUTION 9
#define EVT_CRITICAL 15

#define SERIAL_NUMBER_LEN 20
#define BUFFER_8153 256
struct pkt_8002 {
	uint32_t ErrorCode;
	uint16_t status_word;
	uint16_t max_users;
	uint16_t firmware_version;
	uint32_t firmware_date;
	uint32_t post_errcode;
	uint32_t datetime;
	uint32_t hw_revision;
	uint8_t board_serial_number[SERIAL_NUMBER_LEN];
	uint16_t cable_status;
	uint32_t latest_event_id;
	uint32_t cfg_writecount;
	uint32_t post_errcode_mask;
	uint8_t   class_;
	uint8_t   subclass;
	uint8_t   rc_in_use;
	uint8_t   application;
	uint8_t   security_state;
	uint64_t  chip_id;          // ASIC ID
    uint8_t patch_version;
	uint8_t   reserved[18];
} __attribute__ ((packed));

struct pkt_0006 {
    uint32_t    ErrorCode;
    uint32_t    reserved;
    uint32_t    reserved2;
    uint32_t    reserved3;
} __attribute__ ((packed));

struct pkt_8006 {
    uint32_t           ErrorCode;
    IODNETCFG_V3_CFG cfg;
} __attribute__ ((packed));

struct pkt_0007 {
    uint32_t           ErrorCode;
    IODNETCFG_V3_CFG cfg;
} __attribute__ ((packed));

struct pkt_8007 {
    uint32_t    ErrorCode;
    uint32_t    reserved;
    uint32_t    reserved2;
    uint32_t    reserved3;
} __attribute__ ((packed));

struct pkt_0008 {
} __attribute__ ((packed));

struct pkt_8008 {
    uint32_t ErrorCode;
} __attribute__ ((packed));

struct pkt_0050 {
        uint32_t  operation;
} __attribute__ ((packed));

struct pkt_8050 {
    uint32_t  ErrorCode;
	uint32_t  FlashSectorSize;
	char FirmwareDate[20];
	char FirmwareTime[20];
	char FirmwareNumber[10];
	char FirmwarePass[10];
	char FirmwareName[20];
	uint8_t AsicMajor;
	uint8_t AsicMinor;
	uint8_t CPLDVersion;
	uint8_t HostCPLDVersion;
	uint32_t AsicRTL;
	char Bootleg[80];
	uint8_t flashManufacturer;
	uint8_t flashDevice;
	uint8_t flashState;
	uint8_t flashStage;
	uint8_t flashPercent;
} __attribute__ ((packed));

struct  pkt_8055{
    uint32_t  ErrorCode;
    uint32_t  datetime;
    int16_t   tz_offset;
    uint8_t   daylight;
    uint8_t   pad2;
} __attribute__ ((packed));

struct stat_route {
	uint32_t  dest;
	uint32_t  gate;
} __attribute__ ((packed));

struct pkt_8063 {
	uint32_t    ErrorCode;
	uint32_t    nic_settings;      /* bitfield defined in CFG_NIC_SET_*   */
	uint32_t    nic_status;         /* bitfield defined in CFG_NIC_STAT_* */
	uint32_t    nic_ipaddr;
	uint32_t    nic_ip_mask;
	uint8_t     nic_mac_addr[6];
	uint32_t    nic_gateway_ip;
	uint32_t    nic_wins_ip[2];
	uint32_t    nic_dns_ip[3];
	uint32_t    dhcp_server_ip_address;
	uint32_t    dhcp_options;      /* bitfield defined in CFG_NIC_*   */
	uint32_t    dhcp_status;      /* bitfield defined in CFG_NIC_DHCPSTAT_* */
	uint32_t    front_nic_ipaddr;
	uint32_t    front_nic_ip_mask;
	uint32_t    front_settings;      /* bitfield defined in CFG_NIC_SET_* applies to front port (if implemented) */
	struct      stat_route  route[3];
	uint8_t        nic_name[50];      /* "gromit" in gromit.eng.acme.com */
	uint8_t        nic_domain_name[128];   /* "eng.acme.com" in gromit.eng.acme.com */
} __attribute__ ((packed));

struct pkt_806e {
    uint32_t ErrCode;
	uint32_t flags;
	uint32_t mask;
	uint32_t installable;
	uint32_t status;
	uint32_t reserved3;
	uint32_t reserved4;
	uint8_t  key[100];
} __attribute__ ((packed));

struct pkt_0072 {
	uint32_t  reserved;
	uint8_t   magic[8];
	uint16_t  address;
	uint8_t   segment;
	uint8_t   write_len;
	uint8_t   read_len;
	uint8_t   data[32];
} __attribute__ ((packed));

struct pkt_8072 {
	uint32_t  ErrorCode;
	uint8_t   reserved1[8];
	uint16_t  address;
	uint8_t   segment;
	uint8_t   reserved2;
	uint8_t   read_len;
	uint8_t   data[32];
} __attribute__ ((packed));

struct pkt_0088 {
    uint32_t  operation;
#define SMIF_GET_MEMID  (1) // read a byte from the memid scan chain
#define SMIF_GET_GPI    (2) // read a byte from GPI
#define SMIF_GET_CPLD   (3) // read a byte from the Gromit PAL
#define SMIF_PUT_GPO    (4) // write a byte to the GPO
#define SMIF_PUT_CPLD   (5) // write a byte to the Gromit PAL
#define SMIF_GET_GPIEN  (6) // read the GPI interrupt enable mask
#define SMIF_GET_GPIST  (7) // read the GPI interrupt status mask
#define SMIF_PUT_GPIEN  (8) // write the GPI interrupt enable mask
#define SMIF_PUT_GPIST  (9) // write the GPI interrupt status mask
#define SMIF_GET_GPO    (10) // read a byte from the GPO
    uint32_t index;
    uint32_t reserved1;
    uint32_t status;
#define SMIF_SUCCESS   (0)
#define SMIF_BAD_OP    (1) // unsupported operation
#define SMIF_BAD_INDEX (2) // bad index
#define SMIF_BAD       (3) // error retrieving value at this time
    uint8_t reserved2[3];
    uint8_t  val;
} __attribute__ ((packed));

struct pkt_8088 {
    uint32_t  operation;
	uint32_t index;
    uint32_t reserved1;
    uint32_t status;
	uint8_t reserved2[3];
    uint8_t  val;
} __attribute__ ((packed));


struct pkt_811c {
    uint32_t  ErrorCode;
} __attribute__ ((packed));

struct pkt_0120 {
} __attribute__ ((packed));

struct pkt_8120 {
    uint32_t ErrorCode; // EV Error/Return Code
	uint32_t IPv6Option; // 1 IPv6 enabled, 2 IPv4 enabled
	uint32_t DHCPv6RA;
	uint8_t IPv6_1[20];
	uint8_t IPv6_2[20];
	uint8_t IPv6_3[20];
	uint8_t IPv6_4[20];
	uint8_t IPv6_5[20];
	uint8_t IPv6_6[20];
	uint8_t IPv6_7[20];
	uint8_t IPv6_8[20];
	uint8_t IPv6_9[20];
	uint8_t IPv6_10[20];
	uint8_t IPv6_11[20];
	uint8_t IPv6_12[20];
	uint32_t reserved[5];
	uint8_t IPv6_DNSPrimary[20];
	uint8_t IPv6_DNSSecondary[20];
	uint8_t IPv6_DNSTertiary[20];
	uint8_t IPv6_Gateway[20];
	uint8_t IPv6_Route1[20];
	uint8_t IPv6_Gate1[20];
	uint8_t IPv6_Route2[20];
    uint8_t IPv6_Gate2[20];
	uint8_t IPv6_Route3[20];
    uint8_t IPv6_Gate3[20];
	uint8_t iLOIPPOST;
	uint8_t reserved2[31];

} __attribute__ ((packed));

struct pkt_012b {
	uint16_t  idx;       // Index of EV
	uint16_t  pad;       // used for byte alignment
} __attribute__ ((packed));

struct pkt_812b {
	uint32_t ErrorCode; // EV Error/Return Code
	uint8_t name[32];   // EV Name
	uint16_t sz_ev;     // Size of EV
	uint8_t buf[4000];     // EV Data
} __attribute__ ((packed));

struct pkt_012c {
	uint8_t   flags;     // Bit Field for command
	uint8_t   rsvd[3];   // Used for growth and byte alignment
	uint8_t   name[32];  // EV Name
	uint16_t  sz_ev;     // Size of EV data
	uint8_t   buf[1];    // EV Data
} __attribute__ ((packed));

struct pkt_812c {
	uint32_t  ErrorCode; // EV Error/Return Code
} __attribute__ ((packed));

struct pkt_812d {
	uint32_t ErrorCode;
	uint32_t ImageAuthStatusVersion;
	uint8_t ImageAuthBitField;
	uint8_t CurrentStatusSideAState;
	uint8_t CurrentStatusSideBState;
	uint8_t CurrentStatusRemediationActionTaken;
	uint32_t SignedRegionStartingOffset;
	uint32_t SignedRegionEndingOffset;
	uint32_t SignatureSize;
	uint8_t SignatureAlgorithemGuid[16];
	uint8_t FvGuid[16];
	uint16_t ExecutionOrdinal;
	uint16_t ValidatingAgent;

	uint8_t Signature[64];
} __attribute__ ((packed));

struct pkt_0130 {
	uint8_t name[32];
} __attribute__ ((packed));

struct pkt_0136 {
} __attribute__ ((packed));

struct pkt_8136 {
    uint32_t ErrorCode;
} __attribute__ ((packed));

struct pkt_8130 {
	uint32_t ErrorCode;
	uint8_t name[32];
	uint16_t sz_ev;  // Size of EV
	uint8_t buf[1];  // EV Data
} __attribute__ ((packed));

struct pkt_8132 {
	uint32_t  ErrorCode;      /* 0: success; !0: error */
	uint32_t  rem_sz;         /* remaining size of EV file */
	uint32_t  present_evs;    /* number of EVs present in file */
	uint32_t  max_sz;         /* Max size of EV file in KB */
};

struct pkt_8133 {
    uint32_t ErrorCode; // 0x00 success
    uint32_t state;     // Virtual Serial port state (0x00 not connected / 0x01 connected )
} __attribute__ ((packed));

struct pkt_8139 {
	uint32_t ErrorCode;          /* 0: success; !0: error */
	uint8_t  security_state; /* ERROR=0,FACTORY=1,WIPE=2,PRODUCTION=3,HIGH_SECURITY=4,FIPS=5,SUITEB=6 */
	uint8_t  resvd[3];
} __attribute__ ((packed));

struct pkt_813a {
    uint32_t ErrorCode;
} __attribute__ ((packed));

struct pkt_0143 {
	uint8_t post_state;
	uint8_t  reserved1;
	uint16_t reserved2;
} __attribute__ ((packed));

struct pkt_8143 {
	uint32_t ErrorCode;
} __attribute__ ((packed));

struct pkt_0146 {
    uint8_t  evtType;              /* 0x01 - IML, 0x02 - IEL, 0x04 - SL */
    uint16_t  matchCode;
    uint16_t evtClass;
    uint16_t evtCode;
    uint8_t severity;
    uint16_t evtVarLen;
    uint8_t   buf[1]; // Event Variable Data
} __attribute__ ((packed));

struct pkt_8146 {
    uint8_t  ErrorCode;             /* 0x03 - watermark reached, 0x01 - failure, 0x00 - success*/
    uint8_t  evtType;              /* 0x01 - IML, 0x02 - IEL, 0x04 - SL */
    uint32_t evtNum;
} __attribute__ ((packed));

struct pkt_0151 {
} __attribute__ ((packed));

struct pkt_8151 {
    uint32_t ErrorCode;
    uint32_t pendingClear;
} __attribute__ ((packed));

struct pkt_0153 {
	uint32_t reserved;
    uint32_t operation;
    uint32_t size;
    uint32_t reserved2;
    uint8_t buffer[256];
} __attribute__ ((packed));

struct pkt_8153 {
	uint32_t ReturnCode;
    uint32_t operation;
	uint32_t size;
	uint32_t reserved;
	uint8_t buffer[BUFFER_8153];
} __attribute__ ((packed));

struct pkt_0159 {
    uint8_t HostIfIndex;
    uint8_t reserved;
    uint16_t reserved2;
} __attribute__ ((packed));

struct pkt_8159 {
    uint32_t ErrorCode;
    uint8_t NbIf;
    uint8_t IfIdx;
	uint8_t devType;
	uint8_t ProtocolId;
	char bString[128];
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t reserved;
	uint16_t vendorId;
	uint16_t productId;
	uint8_t uuid[16];
	uint8_t HostIpAssignment;
	uint8_t HostIpAddrFormat;
	uint8_t RedFishServiceIPDiscovery;
	uint8_t RedFishServiceIPAddrFormat;
	uint8_t hostipaddr[16];
	uint8_t hostipmask[16];
	uint8_t redfishServiceIpaddr[16];
	uint8_t redfishServiceIpmask[16];
	uint16_t redfishIpPort;
	uint16_t redfishServiceVLAN;
	uint32_t reserved2[24];
	char RedfishServiceHostname[256];

} __attribute__ ((packed));

struct pkt_0182 {
} __attribute__ ((packed));

struct pkt_8182 {
    uint8_t discoveryStatus;
    uint8_t tinkerStatus;
} __attribute__ ((packed));

struct pkt_not_implemented {
    uint32_t ErrorCode;
} __attribute__ ((packed));

struct pkt_0200 {
    uint32_t ErrorCode;
    uint16_t op;
    uint16_t flags;
    uint32_t data_size;
    uint32_t data_offset;
    uint32_t timestamp;
    uint16_t recordID;
    uint16_t count;
    uint8_t data[4000]; //??? Max size
} __attribute__ ((packed));

struct pkt_8200 {
	uint32_t ErrorCode;
    uint16_t op;
    uint16_t flags;
    uint32_t data_size;
    uint32_t data_offset;
    uint32_t timestamp;
    uint16_t recordID;
    uint16_t count;
    uint8_t data[4000]; //??? Max size
} __attribute__ ((packed));

struct pkt_0202 {
    uint32_t ErrorCode;
    uint16_t op;
    uint16_t flags;
    uint32_t data_size;
    uint32_t data_offset;
    uint32_t timestamp;
    uint16_t recordID;
    uint16_t count;
    uint8_t data[4000]; //??? Max size
} __attribute__ ((packed));

struct pkt_8202_header {
    uint8_t  MinorVer;
    uint8_t  MajorVer;
    uint8_t  SpecialVer;
    uint8_t  BuildVer;
    uint32_t count;
} __attribute__ ((packed));

typedef struct {
   uint16_t pkt_size;
   uint16_t sequence;
   uint16_t command;
   uint8_t  service_id;
   uint8_t  version;
} smif_pkthdr_type;

//sizeof(smif_pkthdr_type) this size is multiplied by 2 since chif packet header and smif packet header are of same size
#define MAX_NUM_ENTITIES         (CHIF_PKT_MAX_SIZE - (sizeof(smif_pkthdr_type) * 2) - sizeof(pkt_8202_header)) / sizeof(Entity)

struct pkt_8202 {
    pkt_8202_header header;
    Entity Entities[MAX_NUM_ENTITIES];
} __attribute__ ((packed));

struct pkt_0209 {
    uint16_t progressCode;
    uint16_t reserved;
    uint32_t bootTime;  //Seconds, not used for all Progress Codes
} __attribute__ ((packed));

struct pkt_8209 {
   uint32_t ErrorCode;
} __attribute__ ((packed));
	

#define TZ_MAX_ZONE_LENGTH (64)
#define TZ_MAX_ENV_STRING_LENGTH (64)
typedef struct
  {
  uint checksum;
  uint version;
  uint wcount;

  int   gmtoff;       // used only when the year changes and there is no
                      // applicable rule, to find a rule that might work the
                      // same, in seconds
  char  zonename[TZ_MAX_ZONE_LENGTH]; // The value stored in here must be
                                      // null terminated
  char  env_string[TZ_MAX_ENV_STRING_LENGTH]; // The value stored in here must be
                                              // null terminated
  //Note:  This structure has been whitelisted to be recorded in blackbox as part of the NVRAM file recording.
  //       If you add any data *ensure* that it is not customer identifiable.  If you have any questions
  //       contact a blackbox expert.
  } TZ_STORED;


/* These functions convert from the standard C timestamp to the
   legacy iLO RTC timestamp format, still used by RBSU and SMIF.
*/
typedef uint32_t RTC_LEGACY_DATETIME;

typedef struct
{
    RTC_LEGACY_DATETIME datetime;   /* HP defined date and time format */
    int                 isdst;      /* "struct tm" defined isdst. For local times isdst indicates */
                                    /* whether or not Daylight Saving Time is in effect or not */
} RTC_LEGACY_TIME;

#define RTC_LEGACY_TIME_INITIALIZER {0, -1};
const RTC_LEGACY_TIME rtc_legacy_time_initializer = RTC_LEGACY_TIME_INITIALIZER;

static char gHostName[IODNETCFG_HOST_NAME_SIZE];
static char gDomainName[IODNETCFG_DOMAIN_NAME_SIZE];
static char gProductId[BUFFER_8153];
static char gSerialNumber[SERIAL_NUMBER_LEN];

void init_smif(void) {
    FILE *fp;

    dbPrintf("Init SMIF\n");

    errno=0;
    if (gethostname(gHostName, IODNETCFG_HOST_NAME_SIZE)) {
        printf("SMIF: hostname failed: errno: %d : %s\n", errno,  strerror(errno));
    } else {
        dbPrintf("hostname: %s\n", gHostName);
    }

    errno=0;
    if (getdomainname(gDomainName, IODNETCFG_DOMAIN_NAME_SIZE)) {
        printf("SMIF: domainname failed: errno: %d : %s\n", errno,  strerror(errno));
    } else {
        dbPrintf("domain name: %s\n", gDomainName);
    }

    memset(gProductId, 0, sizeof(gProductId));
    fp = fopen("/proc/device-tree/model", "r");
    if (fp) {
        if (fread(gProductId, 1, sizeof(gProductId), fp)) {
            dbPrintf("PID: %s\n", gProductId);
        }

        fclose(fp);
    }

    memset(gSerialNumber, 0, sizeof(gSerialNumber));
    fp = fopen("/proc/device-tree/serial-number", "r");
    if (fp) {
        if (fread(gSerialNumber, 1, sizeof(gSerialNumber), fp)) {
            dbPrintf("SN: %s\n", gSerialNumber);
        }

        fclose(fp);
    }
}

/*
 *
 *   smifpkt_0002()
 *
 *      Accepts "Get Flint Status" command packet and sends back a
 *      "Get Flint Status Response" packet (0x8002).
 *
 */
int SmifPkt_0002(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_8002 *respMsg = (struct pkt_8002 *)&respPkt->msg[0];

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8002);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8002;
	respPkt->header.service_id = 0;

	respMsg->ErrorCode = 0;

	respMsg->firmware_version = 0; // not available in OpenBMC in this format.
	respMsg->hw_revision = 0x07;  // GSC
	respMsg->class_ = 7;
	respMsg->subclass = 0;
	respMsg->application = 5;
	respMsg->security_state = 3; // 3 == production mode.
	respMsg->chip_id = 0;  // not used by BIOS.
	strncpy((char *)respMsg->board_serial_number, gSerialNumber, sizeof(respMsg->board_serial_number));

	return respPkt->header.pkt_size;
}

/*
 *  Returns IPV4 network configuration.
 */
int smifpkt_0006 (void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
    struct pkt_8006 *respMsg = (struct pkt_8006 *)&respPkt->msg[0];

    respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8006);
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = 0x8006;
    respPkt->header.service_id = 0;

    respMsg->ErrorCode = 0;

    // We need to retreive the current Ipv4 address from the NIC
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;

    // Default interface is eth0
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    dbPrintf("Network data: %s %x ", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
          ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr);

    respMsg->cfg.iface[0].ipaddr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
    respMsg->cfg.iface[0].gateway_ip = ((struct sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr.s_addr;
    respMsg->cfg.iface[0].ip_mask = ((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr.s_addr;


    strncpy(&respMsg->cfg.iface[0].host_name[0], gHostName, sizeof(respMsg->cfg.iface[0].host_name));
    strncpy(&respMsg->cfg.iface[0].domain_name[0], gDomainName, sizeof(respMsg->cfg.iface[0].domain_name));
    respMsg->cfg.iface[0].kernel = 0;  // not unused by BIOS.

    return respPkt->header.pkt_size;
}

/* 
 *  BIOS makes this call when the RII/iCRU interface is ready for SMI's.
 */
int SmifPkt_0008(void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
 //   struct pkt_0008 *recvMsg = (struct pkt_0008 *)&recvPkt->msg[0];
    struct pkt_8008 *respMsg = (struct pkt_8008 *)&respPkt->msg[0];

    respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8008);
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = 0x8008;
    respPkt->header.service_id = 0;
	respMsg->ErrorCode = 0x00;
	return respPkt->header.pkt_size;

}

RTC_LEGACY_TIME * rtc_legacy_tm_to_legacy(const struct tm * tm_ptr, RTC_LEGACY_TIME *rtc_legacy_time_ptr)
{
    *rtc_legacy_time_ptr = rtc_legacy_time_initializer;

    rtc_legacy_time_ptr->datetime = ((uint)(tm_ptr->tm_year - 90) << 26) // 1990 (SMIF epoch) - 1900 (C tm struct epoch) = 90
                                  | (((uint)tm_ptr->tm_mon + 1)  << 22)
                                  | ((uint)tm_ptr->tm_mday << 17)
                                  | ((uint)tm_ptr->tm_hour << 12 )
                                  | ((uint)tm_ptr->tm_min  << 6 )
                                  | ((uint)tm_ptr->tm_sec);
    rtc_legacy_time_ptr->isdst = tm_ptr->tm_isdst;

    return rtc_legacy_time_ptr;
}

/*  This function converts standard C time_t (UTC) to the legacy ilO RTC timestamp format
 *  (in local time), still used by RBSU and SMIF.
 */
RTC_LEGACY_TIME * rtc_legacy_localtime(
        time_t              t,
        RTC_LEGACY_TIME *   rtc_legacy_time_ptr
)
{
    struct tm tm;

    /* Convert time_t (UTC) into local time */
    localtime_r(&t, &tm);

    /* Convert the tm structure to the legacy iLO RTC timestamp format */
    rtc_legacy_tm_to_legacy(&tm, rtc_legacy_time_ptr);

    return rtc_legacy_time_ptr;
}

/* 
 *  Get IOP Date and Time" function - no payload
 */
int SmifPkt_0055(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_8055 *respMsg = (struct pkt_8055 *)&respPkt->msg[0];
    time_t now;
    RTC_LEGACY_TIME rtc_legacy_time;
//    TZ_STORED tz;

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8055);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8055;
	respPkt->header.service_id = 0;

	respMsg->ErrorCode = 0;

    time(&now);

    /* Convert time_t (UTC) to local time in legacy format */
    rtc_legacy_localtime(now, &rtc_legacy_time);
    respMsg->datetime = rtc_legacy_time.datetime;
    respMsg->daylight = rtc_legacy_time.isdst ? 1 : 0;

	return respPkt->header.pkt_size;
}

/*
 * Get NIC Configuration function - no payload
 */
int SmifPkt_0063(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_8063 *respMsg = (struct pkt_8063 *)&respPkt->msg[0];

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8063);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8063;
	respPkt->header.service_id = 0;

	respMsg->ErrorCode = 0;
	respMsg->nic_settings = 0x03;
	respMsg->nic_status = 0x00;

	// We need to retreive the current Ipv4 address from the NIC
	int fd;
 	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
 	ifr.ifr_addr.sa_family = AF_INET;

	// Default interface is eth0
 	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

 	ioctl(fd, SIOCGIFADDR, &ifr);

 	dbPrintf("%s %x ", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr);

    // switch to network byte order
    uint32_t temp_addr;
    temp_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
    respMsg->nic_ipaddr = ((temp_addr >> 24) & 0xff) |
                          ((temp_addr >> 8)  & 0xff00) |
                          ((temp_addr << 8)  & 0xff0000) |
                          ((temp_addr << 24) & 0xff000000u);

	if(ioctl(fd,SIOCGIFADDR,&ifr)<0)
		printf("ioctl error.\n");

    // switch to network byte order
    temp_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
    respMsg->nic_ip_mask = ((temp_addr >> 24) & 0xff) |
                           ((temp_addr >> 8)  & 0xff00) |
                           ((temp_addr << 8)  & 0xff0000) |
                           ((temp_addr << 24) & 0xff000000u);

	dbPrintf("%x\n", respMsg->nic_ip_mask);

	close(fd);

	return respPkt->header.pkt_size;
}

/* smifpkt_006E()
 *
 * Get License Key - returns payload
 *
 * Example:
 * C:\proj\ilo_license>ilo_license -d
 * 0x00000001  errcode
 * 0x00000000  reserved1
 * 0x00000037  mask
 * 0x00000021  installable
 * 0x00000001  status
 * 0x00000000  reserved3
 * 0x00000000  reserved4
 * Key:
 * [  0]  33 35 44 50 51 4c 43 50 50 48 52 51 57 47 47 37   35DPQLCPPHRQWGG7
 * [ 16]  56 48 53 51 53 33 58 32 4e 00 48 44 4d 20 41 64   VHSQS3X2N.HDM Ad
 * [ 32]  76 61 6e 63 65 64 20 4c 69 6d 69 74 65 64 20 4e   vanced Limited N
 * [ 48]  46 52 00 00 00 00 00 00 00 00 00 00 00 00 00 00   FR..............
 * [ 64]  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   ................
 * [ 80]  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   ................
 * [ 96]  00 00 00 00                                       ....
 */
int SmifPkt_006e(void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
    struct pkt_806e *respMsg = (struct pkt_806e *)&respPkt->msg[0];

    respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8063);
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = 0x806e;
    respPkt->header.service_id = 0;

    respMsg->ErrCode = 0x1; // license installed
    respMsg->flags = 0x0;
    respMsg->mask  = 0x77;
    respMsg->installable = 0x21;
    respMsg->status = 0x1;
    respMsg->key[0] = 51;
    respMsg->key[1] = 52;
    respMsg->key[2] = 77;
    respMsg->key[3] = 89;
    respMsg->key[4] = 82;
    respMsg->key[5] = 78;
    respMsg->key[6] = 82;
    respMsg->key[7] = 89;
    respMsg->key[8] = 52;
    respMsg->key[9] = 68;
    respMsg->key[10] = 56;
    respMsg->key[11] = 84;
    respMsg->key[12] = 71;
    respMsg->key[13] = 89;
    respMsg->key[14] = 54;
    respMsg->key[15] = 54;
    respMsg->key[16] = 74;
    respMsg->key[17] = 50;
    respMsg->key[18] = 90;
    respMsg->key[19] = 68;
    respMsg->key[20] = 67;
    respMsg->key[21] = 90;
    respMsg->key[22] = 82;
    respMsg->key[23] = 53;
    respMsg->key[24] = 74;
    respMsg->key[25] = 0;
    respMsg->key[26] = 105;
    respMsg->key[27] = 76;
    respMsg->key[28] = 79;
    respMsg->key[29] = 32;
    respMsg->key[30] = 65;
    respMsg->key[31] = 100;
    respMsg->key[32] = 118;
    respMsg->key[33] = 97;
    respMsg->key[34] = 110;
    respMsg->key[35] = 99;
    respMsg->key[36] = 101;
    respMsg->key[37] = 100;
    respMsg->key[38] = 0;

    return respPkt->header.pkt_size;
}



/* 
 *  smifpkt_0050(): flash operation.  Contains payload.
 */
int SmifPkt_0050(void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
//    struct pkt_0050 *recvMsg = (struct pkt_0050 *)&recvPkt->msg[0];
    struct pkt_8050 *respMsg = (struct pkt_8050 *)&respPkt->msg[0];
	uint32_t scratchpad=0x00070101; // content of 0x802000a0
	uint32_t tmp;

    respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8050);
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = 0x8050;
    respPkt->header.service_id = 0;
	respMsg->ErrorCode = 0;
	respMsg->FlashSectorSize = 0x10000;

    /* not sure why, but iLO is returning SE FW info here. */
	strcpy(&respMsg->FirmwareDate[0], "June 1 2025");
	strcpy(&respMsg->FirmwareTime[0], "18:00:00");
	strcpy(&respMsg->FirmwareNumber[0],"0.1");
	strcpy(&respMsg->FirmwarePass[0],"1+");
	strcpy(&respMsg->FirmwareName[0],"OpenBMC");

	// bit 23-16
	tmp = ( scratchpad >> 16 ) & 0xF;
	respMsg->AsicMajor = (uint8_t) tmp;
	tmp = ( scratchpad >> 8 ) & 0xF;
	respMsg->AsicMinor = (uint8_t) tmp;
	respMsg->CPLDVersion = 0x27; // 0xd1000003
	respMsg->HostCPLDVersion = 0x0 ; // FYI, this is stored in the smbios record type 216.
	respMsg->AsicRTL = scratchpad;
	strcpy(&respMsg->Bootleg[0], "GSC compatible");
	respMsg->flashState=0;
	respMsg->flashStage=0;
	respMsg->flashPercent=0;

	return respPkt->header.pkt_size;
};

void dump_apml_segments(){
    int i;

    for (i=0; i<254;i++) {
        dbPrintf("i: %d:  %02x %02x\n", i, apml_segments[i].MuxControl.CPLD.Byte,
           apml_segments[i].MuxControl.CPLD.SelectMask);
    }
}

static int select_bus(uint8_t segment)
{
    int i;

    // re-route the request to the relevant bus
    for ( i = 0 ; i < i2cAllocatedEntries ; i++ ) {
	    if ((apml_segments[segment].MuxControl.CPLD.Byte == i2cSystemEntries[i].cpldReg ) &&
		(apml_segments[segment].MuxControl.CPLD.SelectMask == i2cSystemEntries[i].RegVal )) {
	            return i2cSystemEntries[i].i2cKernelSegment;
	    }
    }

    // We expect a software mux implementation on the linux kernel side
    dbPrintf("select_bus: no match for segment %d \n", segment);
    return -1;
}


/* smifpkt_0072()    I2C Transaction Request - takes payload and returns payload
 *
 * Note: it is not the intention for Agents or other routine customer-facing tools to use this support.
 *
 * 0x8072 - I2C Transaction Response
 * errorcode values:
 * 0:    Success
 *       Return codes other than Success do not return data; transaction may not have been performed.
 * 3:    Bad target address
 * 4:    Read length > 32
 * 5:    Write length > 32
 * 6:    Bad magic word (Factory or FLocked)
 * 7:    Unauthorized. Must be in factory mode.
 *
 * 108:  I2C_SEGMENT_DOES_NOT_EXIST    segment bad
 * 102:  I2C_SIZE_GREATER_THAN_MAX_STANDARD_TRANSACT_ERROR          data size exceeds API support
 * 103:  I2C_TIMEOUT          request timed-out waiting for read data
 * 104:  I2C_BTE_ERR          error processing bulk transaction
 * 105:  I2C_NO_DEV           no ACK from target during address phase
 * 106:  I2C_BTE_BUSY         hardware cannot start new transaction
 * 107:  I2C_BTE_NOT_COMPLETE this should never happen
 * 108:  I2C_BAD_ARG          indicates a bad segment argument
 * 109:  I2C_RESOURCE_EXHAUSTED  init failure creating semaphores
 * 110:  I2C_NO_BUS           root segment requested from bad index
 * 111:  I2C_NO_BUS_OWNERSHIP transaction blocked because of ownership
 * 112:  I2C_VASCOMM_ERR      problem attempting VAS call
 * 113:  I2C_MUTEX_ERR        problem with kernel call
 * 114:  I2C_RESUSCITATE      problem with bus
 * 115:  I2C_API_ERR          Data error - SMBIOS I2C topology records may be suspect
 * 116:  I2C_ARB_ERR          Carbondale Arbitration error
 * 117:  I2C_LOCK_ERR         Target device is locked for exclusive access
 * 118:  I2C_CHECKSUM_ERR     Checksum failed on I2C block transaction
 * 119:  I2C_NOGO             RESERVED Condition prevented transaction
 * 120:  I2C_BLOCK_WR_ERR     RESERVED Block write error
 * 121:  I2C_PROTOCOL_ERR     Detected a protocol error.  Reset the bus.
 */
#define MAX_I2C_RETRIES 1
int SmifPkt_0072(void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
    struct pkt_0072 *recvMsg = (struct pkt_0072 *)&recvPkt->msg[0];
    struct pkt_8072 *respMsg = (struct pkt_8072 *)&respPkt->msg[0];
    std::string i2cTransferCmd = "i2ctransfer -y -f ";

    dbPrintf("pkt_0072->address=0x%02x\n", (uint8_t)recvMsg->address);
    dbPrintf("pkt_0072->segment=0x%02x\n", recvMsg->segment);
    dbPrintf("pkt_0072->write_len=0x%02x\n", recvMsg->write_len);
    dbPrintf("pkt_0072->read_len=0x%02x\n", recvMsg->read_len);

    respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8072);
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = 0x8072;
    respPkt->header.service_id = 0;

    if(recvMsg->address == 0xffff && recvMsg->segment == 0xff) {
        printf("pkt_0072 invalid address = %c\n", recvMsg->data[0]);
        respMsg->ErrorCode = I2C_BAD_ARGUMENT;

    } else {
        // Select the bus first
        int bus;

        bus = select_bus(recvMsg->segment);
        if (bus == -1) {
            respMsg->ErrorCode = I2C_SEGMENT_DOES_NOT_EXIST;

        } else {
            int retry=0;
            int i;
            char temp[256]; // hold at least 32 hex printed like '0xcb '

            // Start building the raw command
            // first add the bus, number of bytes to write, and the 7-bit I2C address
            memset(temp, 0, sizeof(temp));
	        sprintf(temp, "%d w%d@0x%02x", bus, recvMsg->write_len, (uint8_t)(recvMsg->address >> 1));
            i2cTransferCmd += temp;

            // Next add the raw bytes to be written
            memset(temp, 0, sizeof(temp));
            for (i=0; i < recvMsg->write_len; i++) {
                sprintf(&temp[i*5], " 0x%02x", recvMsg->data[i]);
            }
            i2cTransferCmd += temp;

            // last add the number of bytes to read back, if any.
            if (recvMsg->read_len) {
                memset(temp, 0, sizeof(temp));
                sprintf(temp, " r%d", recvMsg->read_len);
                i2cTransferCmd += temp;
            }

            // e.g. i2ctransfer -y -f 21 -w4@0x40 0x34 0xbf 0x00 0xe8

            dbPrintf("    %s\n", i2cTransferCmd.c_str());
            while (retry++ <= MAX_I2C_RETRIES) {

                errno = 0;
                // issue the command and get the output
                FILE *cmd = popen(i2cTransferCmd.c_str(), "r");
                if (cmd) {
                    if (recvMsg->read_len) {
                        memset(temp, 0, sizeof(temp));
                        // check for output
                        if (fgets(temp, sizeof(temp), cmd) !=NULL) {
                            char * hexString;
                            // get each hex byte value and add it to the return data
                            dbPrintf("%s", temp);
                            hexString = strtok(temp, " ");
                            for (i=0; hexString != NULL; i++) {
                                if (strcmp(hexString, "Error:") == 0) {
                                    dbPrintf("%s failed, retry: %d, ", hexString, retry);
                                    respMsg->ErrorCode = I2C_GENERAL_ERROR;
                                    break;
                                }
                                respMsg->data[i] = std::stoi(hexString, 0, 16);
                                hexString = strtok(NULL, " ");
                                retry = 0;
                                respMsg->ErrorCode = I2C_SUCCESS;
                            }
                            hexdump(respMsg->data, i);
                            dbPrintf("\n");
                        } /* fgets */
                    } else {
                        dbPrintf("No output expected for i2ctransfer command\n");
                        retry = 0;
                        respMsg->ErrorCode = I2C_SUCCESS;
                    }
                    (void) pclose(cmd);

                    if (retry) {
                        // delay before retrying
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        continue;
                    } else {
                        break;
                    }

                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    dbPrintf("cmd is Null, popen call failed: %s\n", i2cTransferCmd.c_str());
                }
            }

            if (retry > MAX_I2C_RETRIES) {
                dbPrintf("All retries failed\n");
                respMsg->ErrorCode = I2C_SEGMENT_DOES_NOT_EXIST;
            }
            if (respMsg->ErrorCode == I2C_SUCCESS) {
                respMsg->address = recvMsg->address;
                respMsg->segment = recvMsg->segment;
                respMsg->read_len = recvMsg->read_len;
                respMsg->ErrorCode = 0;
            }
        }
    }

    return respPkt->header.pkt_size;
}

#define GPIO_MEMID_DATA_SIZE (256)

int gpio_xreg_get(uint8_t index, uint8_t *regValue)
{
    char temp[64];
    unsigned long val;
    int rv = 1;

    if ((index != 1) && (index != 2)) {
	    printf("gpio_xreg_get %d bad index\n", index);
	    return rv;
    }

    FILE *fp = fopen("/sys/class/soc/xreg/server_id", "r");
    if (fgets(temp, sizeof(temp), fp) != NULL) {
        val = strtoul(temp, NULL, 16);

        if (index == 1) {
            val = val & 0xff;
        } else if (index == 2) {
            val = (val >> 8) & 0xff;
        }
        *regValue = val;
	rv = 0;
    }
    fclose(fp);
    return rv;
}

/* 0088: "I/O bits access"  - contains payload
 *
 * Used for access to
 *   GPI, GPO, Xregister, MEMID
 * from the host.  In some cases, they can directly access the hardware via PCI,
 * but doing that via iLO avoids an ownership and synchronization problem.
 * In some cases (XRegisters) the host software cannot directly access.
 */
int SmifPkt_0088(void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
    struct pkt_0088 *recvMsg = (struct pkt_0088 *)&recvPkt->msg[0];
    struct pkt_8088 *respMsg = (struct pkt_8088 *)&respPkt->msg[0];
    respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8088);
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = 0x8088;
    respPkt->header.service_id = 0;

    respMsg->operation = recvMsg->operation;
    respMsg->index = recvMsg->index;
    respMsg->status = 0; // we are optimistict everything goes well- FIXME

    respMsg->val = recvMsg->val;
    switch(recvMsg->operation){
       case SMIF_GET_MEMID:
           dbPrintf("SMIF_GET_MEMID: index: %08x, val: %02x\n", recvMsg->index, recvMsg->val);
           break;
 
       case SMIF_GET_GPI:
           dbPrintf("SMIF_GET_GPI: index: %08x, val: %02x\n", recvMsg->index, recvMsg->val);
           break;
 
       case SMIF_GET_CPLD:
           dbPrintf("SMIF_GET_CPLD: index: %08x, val: %02x\n", recvMsg->index, recvMsg->val);
           if (recvMsg->index > RANGE_CPLD) {
               printf("Index %d is out of range.\n", recvMsg->index);
               respMsg->status = SMIF_BAD_INDEX;
               break;
           }

           if (gpio_xreg_get((uint8_t)recvMsg->index, &respMsg->val))
               respMsg->status = SMIF_BAD;
           else
               respMsg->status = SMIF_SUCCESS;
           break;
 
       case SMIF_PUT_GPO:
           dbPrintf("SMIF_PUT_GPO: index: %02x, val: %02x\n", recvMsg->index, recvMsg->val);
           break;
 
       case SMIF_PUT_CPLD:
           dbPrintf("SMIF_PUT_CPLD: index: %08x, val: %02x\n", recvMsg->index, recvMsg->val);
           break;
 
       case SMIF_GET_GPIEN:
           dbPrintf("SMIF_GET_GPIEN: index: %08x, val: %02x\n", recvMsg->index, recvMsg->val);
           break;
 
       case SMIF_GET_GPIST:
           dbPrintf("SMIF_GET_GPIST: index: %08x, val: %02x\n", recvMsg->index, recvMsg->val);
           break;
 
       case SMIF_PUT_GPIEN:
           dbPrintf("SMIF_PUT_GPIEN: index: %08x, val: %02x\n", recvMsg->index, recvMsg->val);
           break;
 
       case SMIF_PUT_GPIST:
           dbPrintf("SMIF_PUT_GPIST: index: %08x, val: %02x\n", recvMsg->index, recvMsg->val);
           break;
 
       case SMIF_GET_GPO:
           dbPrintf("SMIF_GET_GPO: index: %08x, val: %02x\n", recvMsg->index, recvMsg->val);
           break;
 
       default:
           printf("SMIF_BAD_OP\n");
           respMsg->status = SMIF_BAD_OP;
           break;
    }

    return respPkt->header.pkt_size;
}

/* 
 * Get iLO IPv6 configuration
 */
int SmifPkt_0120(void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
//    struct pkt_0120 *recvMsg = (struct pkt_0120 *)&recvPkt->msg[0];
    struct pkt_8120 *respMsg = (struct pkt_8120 *)&respPkt->msg[0];

    respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8120);
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = 0x8120;
    respPkt->header.service_id = 0;

    // no active ipv6 
    respMsg->ErrorCode = 1;
    respMsg->IPv6Option = 0;
	respMsg->iLOIPPOST = 0;

    return respPkt->header.pkt_size;
}

/*
 * Get EV by Index
 */
int SmifPkt_012b(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_012b *recvMsg = (struct pkt_012b *)&recvPkt->msg[0];
	struct pkt_812b *respMsg = (struct pkt_812b *)&respPkt->msg[0];

	struct ev *local_ev;

	dbPrintf("smif_012b: ev index: %d\n", recvMsg->idx);
//	if ( recvMsg->idx == 198 ) system("sleep 60");
	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_812b) - 4000;
	dbPrintf("SizeOf Packet: %d\n", respPkt->header.pkt_size);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x812b;
	respPkt->header.service_id = 0;

	local_ev = getEVbyIndex(recvMsg->idx, (char *)respMsg->buf, EV_MAX_LEN);
	if(EVError<0) {
		switch(EVError) {
			case -1:
				dbPrintf("No such EV");
				respMsg->ErrorCode = 0x02; // Indicate EV not found.
				for ( int i = 0 ; i < 32 ; i++ )
					respMsg->name[i]='\0';
				respMsg->sz_ev = 0;
				for ( int i = 0 ; i < 4000 ; i++ )
					respMsg->buf[i] = 0;
				break;
			default:
				dbPrintf("Other Get EV by index error\n");
				respMsg->ErrorCode = 0x01;
				respMsg->sz_ev = 0;
				break;
		}
	}
	else {
		strncpy((char *)(&respMsg->name[0]), local_ev->name, 32);
		if ( strncmp((char *)&respMsg->name[0], "CQHMEM", 6 ) == 0 )
		{
			respMsg->buf[0]=0x40;
			respMsg->buf[1]=0x60;
			respMsg->buf[2]=0x54;
			respMsg->buf[3]=0x00;
			local_ev->size = 4;
		}
		dbPrintf("Get EV by index Success! EV Name %s : Buffer %s Size %d\n", respMsg->name, respMsg->buf, local_ev->size);
		respMsg->ErrorCode = 0x00;
		respMsg->sz_ev += local_ev->size;
	}
	return respPkt->header.pkt_size + respMsg->sz_ev;
}

/*
 * Set/Delete EV
 */
int SmifPkt_012c(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_012c *recvMsg = (struct pkt_012c *)&recvPkt->msg[0];
	struct pkt_812c *respMsg = (struct pkt_812c *)&respPkt->msg[0];
	int rc;

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_812c);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x812c;
	respPkt->header.service_id = 0;

	dbPrintf("\nsmif_012c: ev name: %s flags:%d sz_ev:%d\n", recvMsg->name, recvMsg->flags, recvMsg->sz_ev);

	switch(recvMsg->flags) {
		case 0x01: //Set EV
			dbPrintf("smif_012c: set EV\n");
			if(recvMsg->sz_ev>EV_DATA_MAX_LEN) {
				respMsg->ErrorCode = 0x04; //EV data too large
				dbPrintf("smif_012c: error_code: 0x04\n");
				break;
			}

			if(recvMsg->sz_ev) {  // only store the EV if it has a size
				rc = setEV((char *)recvMsg->name, (char *)recvMsg->buf, recvMsg->sz_ev);
				if(rc<0) {
					respMsg->ErrorCode = 0x01; // EV Error
					dbPrintf("smif_012c: error_code: 0x01\n");
					break;
				}
				respMsg->ErrorCode = 0x00;
				dbPrintf("smif_012c: ok\n");
				break;

			} else {
				dbPrintf("EV size was 0, did not store\n");
				// Fall through to delete the EV if it exists.
                [[fallthrough]];
			}


		case 0x02: //EV delete
			dbPrintf("smif_012c: delete EV\n");
			if(recvMsg->name[32] != 0x00) {
				respMsg->ErrorCode = 0x03; //EV name too large
				dbPrintf("smif_012c: error_code: 0x03\n");
				break;
			}

			rc = delEV((char *)recvMsg->name);
			if(rc<0) {
				respMsg->ErrorCode = 0x01; // EV Error
				dbPrintf("smif_012c: error_code: 0x01\n");
				break;
			}

			respMsg->ErrorCode = 0x00;
			dbPrintf("smif_012c: delete OK\n");
			break;

		case 0x04: //EV delete all
			dbPrintf("smif_012: delete all\n");
			clearEV();
			respMsg->ErrorCode = 0x00; //Success
			dbPrintf("smif_012: delete all success\n");
			break;
		default:
			respMsg->ErrorCode = 0x05; // EV not supported
			dbPrintf("smif_012: ev not supported\n");
			break;
	}

	return respPkt->header.pkt_size;
}

/* SMIF callback for packet 0x012D - Get BIOS Image Authorization Status */
int SmifPkt_012d(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_812d *respMsg = (struct pkt_812d *)&respPkt->msg[0];

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_812d);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x812d;
	respPkt->header.service_id = 0;

	respMsg->ErrorCode = 0x0;
	respMsg->ImageAuthStatusVersion = 1;
	respMsg->ImageAuthBitField = 0x01;     // only single side A active
	respMsg->CurrentStatusSideAState = 1;  // image authenticated
	respMsg->CurrentStatusSideBState = 1;  // image authenticated
	respMsg->CurrentStatusRemediationActionTaken = 0; // No action taken
	respMsg->ValidatingAgent = 0; // BMC

	return respPkt->header.pkt_size;
}

/* 
 * Get EV by Name
 */
int SmifPkt_0130(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_0130 *recvMsg = (struct pkt_0130 *)&recvPkt->msg[0];
	struct pkt_8130 *respMsg = (struct pkt_8130 *)&respPkt->msg[0];

	int ev_size;

	dbPrintf("\nsmif_0130: ev name: %s\n", recvMsg->name);
	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8130) -1;
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8130;
	respPkt->header.service_id = 0;

	ev_size = getEVbyName((char *)recvMsg->name, (char *)respMsg->buf, EV_MAX_LEN);
	if(ev_size<0) {
		switch(ev_size) {
			case -1:
				dbPrintf("smif_0130: No such EV\n");
				respMsg->ErrorCode = 0x02; //NO such EV
				break;
			default:
        dbPrintf("smif_0130: EV Error\n");
				respMsg->ErrorCode = 0x01; //EV Error
				break;
		}
	}
	else {
		respMsg->ErrorCode = 0x00; //return 0x00 as ok
		respMsg->sz_ev += ev_size; //respMsg->sz_ev is 0 so now this is setting ev_size;
		memcpy(respMsg->name, recvMsg->name, 32);
	}

	return respPkt->header.pkt_size + respMsg->sz_ev;
}

/*
 * EV Get File System Status
 */
int SmifPkt_0132(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_8132 *respMsg = (struct pkt_8132 *)&respPkt->msg[0];

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8132);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8132;
	respPkt->header.service_id = 0;

	respMsg->ErrorCode = 0x00;
	respMsg->max_sz = EV_FILE_MAX_SIZE / 1024;
	respMsg->rem_sz = (EV_FILE_MAX_SIZE - EV_FILE_HEADER) - getSizeOfEVfile();
	respMsg->present_evs = getNumOfAllEV();

	return respPkt->header.pkt_size;
}

/*
 * Get VSP Status
 */
int SmifPkt_0133(void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
    struct pkt_8133 *respMsg = (struct pkt_8133 *)&respPkt->msg[0];

    respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8133);
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = 0x8133;
    respPkt->header.service_id = 0;

    respMsg->ErrorCode = 0x00;
	respMsg->state = 0x01;  // connected

    return respPkt->header.pkt_size;
}

/*
 * "Synch iLO/BIOS Comm
 */
int SmifPkt_0136(void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
    struct pkt_8136 *respMsg = (struct pkt_8136 *)&respPkt->msg[0];

    respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8136);
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = 0x8136;
    respPkt->header.service_id = 0;
	respMsg->ErrorCode = 0x00;
	return respPkt->header.pkt_size;

}
  
/*
 * Returns the current security state
 */
int SmifPkt_0139(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_8139 *respMsg = (struct pkt_8139 *)&respPkt->msg[0];

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8139);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8139;
	respPkt->header.service_id = 0;

    // 1 = Factory,          2 = Wipe,       3 = Production,
    // 4 = High Security,    5 = FIPS,       6 = CNSA.
	respMsg->security_state = 0x03; // return Production mode
	respMsg->ErrorCode = 0x00;

	dbPrintf("smif_0139: error_code:0x%02x\n", respMsg->ErrorCode);

	return respPkt->header.pkt_size;
}

/*
 * BIOS will send us 512 bytes of Entropy
 */
int SmifPkt_013a(void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
    struct pkt_813a *respMsg = (struct pkt_813a *)&respPkt->msg[0];

    respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_813a);
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = 0x813a;
    respPkt->header.service_id = 0;
	respMsg->ErrorCode = 0;
	return respPkt->header.pkt_size;

}

/*
 * Write ROM POST state byte to file on channel 24 only
 */
int SmifPkt_0143(void *recv, void *resp)  
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_0143 *recvMsg = (struct pkt_0143 *)&recvPkt->msg[0];
	struct pkt_8143 *respMsg = (struct pkt_8143 *)&respPkt->msg[0];

	dbPrintf("smif_0143: post_state:0x%02x\n", recvMsg->post_state);

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8143);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8143;
	respPkt->header.service_id = 0;

	respMsg->ErrorCode = 0x00;

	return respPkt->header.pkt_size;
}

static constexpr char const* selMessageId = "b370836ccf2f4850ac5bee185b77893a";

extern void dbus_send(std::string message,
                     uint32_t evtClass, uint32_t  evtCode,
		     int logType, std::string action, int severity);

/*
 *
 * Quick Add Event (IML/IEL/SL)
 */
int SmifPkt_0146(void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
    struct pkt_0146 *recvMsg = (struct pkt_0146 *)&recvPkt->msg[0];
    struct pkt_8146 *respMsg = (struct pkt_8146 *)&respPkt->msg[0];
    char buffer[512];
    char *event_message = (char*)&buffer;
    char *event_severity;
    char *event_ID;
    int event_priority=LOG_INFO;
    EVT_LOG_ENTRY evt;

    evt.hdr.evtClass = recvMsg->evtClass;
    evt.hdr.evtCode = recvMsg->evtCode;

    memcpy(&evt.data, &recvMsg->buf, recvMsg->evtVarLen);
    switch (recvMsg->evtType) {
    case EVT_IML:
            decode_text(LOGS_IMLDATA, &evt, event_message, (unsigned short)sizeof(buffer), TEXT_DESC);
            dbPrintf("smif_0146: quick event add to IML\n%s\n", event_message);
            break;
        case EVT_IEL:
            decode_text(LOGS_GELDATA, &evt, event_message, (unsigned short)sizeof(buffer), TEXT_DESC);
            dbPrintf("smif_0146: quick event add to IEL\n%s\n", event_message);
            break;
        case EVT_SL:
            decode_text(LOGS_SLDATA, &evt, event_message, (unsigned short)sizeof(buffer), TEXT_DESC);
            dbPrintf("smif_0146: quick event add to SL\n%s\n", event_message);

            break;
        default:
            snprintf(event_message, sizeof(buffer), "%s", (char*)"Unknown Log Type");
            dbPrintf("smif_0146: quick event add unknown log specified\n");
    }
    dbPrintf("matchCode : %04x\n", (uint16_t)recvMsg->matchCode);
    dbPrintf("evtClass  : %d\n", (uint32_t)recvMsg->evtClass);
    dbPrintf("evtCode   : %d\n", (uint32_t)recvMsg->evtCode);
    dbPrintf("severity  : %04x\n", (uint16_t)recvMsg->severity);
    dbPrintf("evtVarLen : %04x\n", (uint16_t)recvMsg->evtVarLen);
    hexdump(recvMsg->buf, recvMsg->evtVarLen);

    switch (recvMsg->severity) {
        case EVT_INFORMATIONAL:
            event_severity = (char*)"OK";
            event_priority = LOG_INFO;
            event_ID = (char*)"OpenBMC.0.1.OemEventOk";
            break;
        case EVT_REPAIRED:
            event_severity = (char*)"OK";
            event_priority = LOG_INFO;
            event_ID = (char*)"OpenBMC.0.1.OemEventOk";
            break;
        case EVT_CAUTION:
            event_severity = (char*)"Caution";
            event_priority = LOG_WARNING;
            event_ID = (char*)"OpenBMC.0.1.OemEventCaution";
            break;
        case EVT_CRITICAL:
            event_severity = (char*)"Critical";
            event_priority = LOG_CRIT;
            event_ID = (char*)"OpenBMC.0.1.OemEventCritical";
            break;
    }

    // if there are any commas in the text description, replace them with a hyphen.
    std::string message = event_message;
    std::replace(message.begin(), message.end(), ',', ';');

    sd_journal_send("MESSAGE=%s", message.c_str(),
                    "PRIORITY=%d", event_priority,
                    "REDFISH_MESSAGE_ARGS=%s,%d,%d",
                     message.c_str(), (uint32_t)recvMsg->evtClass, (uint32_t)recvMsg->evtCode,
                    "NAME=EventAdd",
                    "SEVERITY=%s", event_severity,
                    "MESSAGE_ID=%s", selMessageId,
                    "REDFISH_MESSAGE_ID=%s", event_ID, NULL);
    dbus_send(message, recvMsg->evtClass, recvMsg->evtCode, recvMsg->evtType, "", recvMsg->severity);

    respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8146);
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = 0x8146;
    respPkt->header.service_id = 0;

    respMsg->ErrorCode = 0x00;
    respMsg->evtType = recvMsg->evtType;
    respMsg->evtNum = 1234;   // FIXME, need to figure out how to get the event ID back from sd_journal_send.

    return respPkt->header.pkt_size;
}
/*
 * Pending SPD Clear Staus
 */
int SmifPkt_0151(void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
    struct pkt_8151 *respMsg = (struct pkt_8151 *)&respPkt->msg[0];

    respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8151);
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = 0x8151;
    respPkt->header.service_id = 0;

    respMsg->ErrorCode = 0;      // no errrors
    respMsg->pendingClear = 0;   // no pending clear

    return respPkt->header.pkt_size;
}

/* smifpkt_0153() Field Access
 * Provides read and write access to enumerated fields using I/O buffer
 *
 * op defines field and access model
 * sz defines data size
 * On error, text in response buffer describes error.
 */
int SmifPkt_0153(void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
    struct pkt_0153 *recvMsg = (struct pkt_0153 *)&recvPkt->msg[0];
    struct pkt_8153 *respMsg = (struct pkt_8153 *)&respPkt->msg[0];

    respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8153);
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = 0x8153;
    respPkt->header.service_id = 0;

    respMsg->operation = recvMsg->operation;
	respMsg->ReturnCode = 0; // Success
    for ( int i = 0 ; i < 256 ; i++ )
        respMsg->buffer[i] = 0;

    if ( recvMsg->operation == 0x1 ) {
        // Read Serial Number
        respMsg->size = strlen(gSerialNumber);
        strncpy((char *)(&respMsg->buffer[0]), gSerialNumber, sizeof(respMsg->buffer));

    } else if ( recvMsg->operation == 0x3 ) {
        // Read Product ID
        respMsg->size = strlen(gProductId);
        strncpy((char *)(&respMsg->buffer[0]), gProductId, sizeof(respMsg->buffer));

    } else {
        respMsg->size = 0;
    }

    return respPkt->header.pkt_size;
}

/* smifpkt_0182()
 *
 * Tinker discover for gen11.
 * TODO: Backend API need to implement it.
 *
 */
int SmifPkt_0182(void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
    struct pkt_8182 *respMsg = (struct pkt_8182 *)&respPkt->msg[0];

    respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8182);
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = 0x8182;
    respPkt->header.service_id = 0;

    respMsg->discoveryStatus = 1;  // discovery complete
    respMsg->tinkerStatus = 0;     // no tinker found

    return respPkt->header.pkt_size;
}

int SmifPkt_not_implemented(void *recv, void *resp, uint32_t rc)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
    struct pkt_not_implemented *respMsg = (struct pkt_not_implemented *)&respPkt->msg[0];

    respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_not_implemented);
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = recvPkt->header.command | 0x8000;
    respPkt->header.service_id = 0;
	respMsg->ErrorCode = rc;
	return respPkt->header.pkt_size;

}

/* smifpkt_0200()
 *
 * Read/Modify/Write iLO APML data atomically.
 *
 */
/* NOTE: below defines must match those in the doc "SMIF and RIBCL interface for APML" */
#define PLATDEF_CMD_RELOAD_PLATDEF                   (0x0001)
#define PLATDEF_CMD_BEGIN_UPLOAD_PLATDEF             (0x0002)
#define PLATDEF_CMD_UPLOAD_PLATDEF_CHUNK             (0x0003)
#define PLATDEF_CMD_FINISH_UPLOAD_PLATDEF            (0x0004)
#define PLATDEF_CMD_PATCH_PLATDEF_DATA               (0x0005)
#define PLATDEF_CMD_DOWNLD_1_PLATDEF_DATA            (0x0006)
#define PLATDEF_CMD_DOWNLD_PLATDEF_HDRS              (0x0007)
#define PLATDEF_CMD_DOWNLD_PLATDEF_CHUNK             (0x0008)
#define PLATDEF_CMD_DOWNLD_SPEC_PLATDEF_DATA         (0x0009)
#define PLATDEF_CMD_DOWNLD_VALIDN_RESULTS            (0x000A)
#define PLATDEF_CMD_DOWNLD_SPEC_PLATDEF_DATA_BY_TYPE (0x000B)

int SmifPkt_0200(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct pkt_0200 *recvMsg = (struct pkt_0200 *)&recvPkt->msg[0];
	struct pkt_8200 *respMsg = (struct pkt_8200 *)&respPkt->msg[0];


	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8200);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8200;
	respPkt->header.service_id = 0;
    respMsg->ErrorCode = 0x00;

    switch (recvMsg->op) {

	    case PLATDEF_CMD_DOWNLD_SPEC_PLATDEF_DATA:
            {
                uint16_t rec_count;
                uint32_t token;
                uint32_t resp_size;

                resp_size = recvMsg->data_size; 
                rec_count = recvMsg->count; 
                respMsg->ErrorCode = platdef_Download_specific_data ((UINT32*)&resp_size,
                                                              (UINT32)recvMsg->timestamp,
                                                              (UINT16*)&rec_count,
                                                              (PlatDefDataRequest*)(recvMsg->data),
                                                              (UINT8 *)&(respMsg->data),
                                                              4000,
                                                              (UINT32*)&token);
                if ( respMsg->ErrorCode == PLATDEF_SMIF_RC_OK) {
                    dbPrintf("APML Platdef download specific data : Success\n");
                    respMsg->count = rec_count;
                    respMsg->data_size = resp_size;
                    respMsg->timestamp = token;
                } else {
                    dbPrintf("APML Platdef download specific data : Error-%d\n", respMsg->ErrorCode);
                }
            }
            break;

        case PLATDEF_CMD_DOWNLD_SPEC_PLATDEF_DATA_BY_TYPE:
            {
                uint16_t rec_count;
                uint32_t token;
                uint32_t resp_size;

                resp_size = recvMsg->data_size; 
                rec_count = recvMsg->count; 
                UINT32 recType   = recvMsg->recordID; 

                respMsg->ErrorCode = platdef_Download_specific_data_per_type  ((UINT32)recvMsg->timestamp,
                                                              recType,
                                                              (PlatDefDataRequest*)(recvMsg->data),
                                                              (UINT32)rec_count,
                                                              (UINT8 *)&(respMsg->data),
                                                              (UINT16 *)&rec_count,
                                                              (UINT16 *)&resp_size,
                                                              (UINT32 *)&token);
                if ( respMsg->ErrorCode == PLATDEF_SMIF_RC_OK) {
                    dbPrintf("APML Platdef download specific data : Success\n");
                    respMsg->count = rec_count;
                    respMsg->data_size = resp_size;
                    respMsg->timestamp = token;
                } else {
                    dbPrintf("APML Platdef download specific data : Error-%d\n", respMsg->ErrorCode);
                }

            }
            break;

        default:
            strncpy((char *)respMsg->data, (const char *)"No operation match", sizeof(respMsg->data));
            dbPrintf("Unsupported 0x200 request: %x\n", recvMsg->op);
            break;
    }

	return respPkt->header.pkt_size;
}

/**
 * smifpkt_0202()
 * Fetch and send APML records data related to power supply, processor 
 * and system devices requested by BIOS.
 * 
 */
int SmifPkt_0202(void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
    struct pkt_8202 *respMsg = (struct pkt_8202 *)&respPkt->msg[0];
	uint32_t count;
    /*Note: The maximum expected size of the request and response packet is 4096. This includes the chif header*/
    PlatDefTableData*  td;

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8202);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0x8202;
	respPkt->header.service_id = 0;
    //respMsg->ErrorCode = 0x00;  don't think ErrorCode is used for this command.

	memset(respMsg, 0, sizeof(pkt_8202));

    if (platdef_get_APML_data(&count, respMsg->Entities, (uint32_t)MAX_NUM_ENTITIES))
	{
		printf("Error in fetching the BIOS data\n");
        respMsg->header.count = 0;
	}
	else
	{
		dbPrintf("Successfully fetched the BIOS data with count = %d\n", count);

	    respMsg->header.count = count;
	    dbPrintf("Get platdef version\n");
        td = table_data();
        if( td ) {
            respMsg->header.MajorVer = td->MajorVersion;
            respMsg->header.MinorVer = td->MinorVersion;
            respMsg->header.SpecialVer = td->SpecialVersion;
            respMsg->header.BuildVer = td->BuildVersion;
        } else {
            dbPrintf("Get table_data returned NULL\n");
	    }
    }

	return respPkt->header.pkt_size;
}

typedef struct {
    UINT16         progressCode;
    UINT16         reserved;
    UINT32         bootTime;  //Seconds, not used for all Progress Codes
} bootProgressPolicy;

typedef enum {
    BOOTPROGRESS_READ_SUCCESS,
    BOOTPROGRESS_READ_FAILURE,            
    BOOTPROGRESS_WRITE_SUCCESS,
    BOOTPROGRESS_WRITE_FAILURE
} BOOTPROGRESS_STATUS;

BOOTPROGRESS_STATUS svcshost_bootprogress_write_chif(bootProgressPolicy *bootProgress)
{
static bootProgressPolicy BootProgress;

    BootProgress = *bootProgress;

	BOOTPROGRESS_STATUS retval = BOOTPROGRESS_WRITE_SUCCESS;
	return retval;
}

/**
 * smifpkt_0209()
 * Get BootProgress Policy from BIOS. 
 * 
 */
int SmifPkt_0209(void *recv, void *resp)
{
    struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
    struct ChifPkt *respPkt = (struct ChifPkt *)resp;
    struct pkt_0209 *bootProgress = (struct pkt_0209 *)&recvPkt->msg[0];
    struct pkt_8209 *respMsg = (struct pkt_8209 *)&respPkt->msg[0];
    bootProgressPolicy bootProg;

    respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(struct pkt_8209);
    respPkt->header.sequence = recvPkt->header.sequence;
    respPkt->header.command = 0x8209;
    respPkt->header.service_id = 0;

    memset(&bootProg, 0, sizeof(bootProgressPolicy));
    respMsg->ErrorCode = 0x00; //Assume Success

    
    bootProg.bootTime = bootProgress->bootTime;
    bootProg.progressCode = bootProgress->progressCode;

	//TODO : This Temp Workaround - To be fixed
    if(svcshost_bootprogress_write_chif(&bootProg) == BOOTPROGRESS_WRITE_SUCCESS)
    {
        dbPrintf("File write into NAND successful for BootProgress %d\n", bootProg.progressCode);
    }
    else
    {
        respMsg->ErrorCode = 1;
        dbPrintf("File write into NAND failure for BootProgress %d\n", bootProg.progressCode);
    }

    return respPkt->header.pkt_size;
}

int SmifPkt_badcmd(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	uint32_t *pErrorCode = (uint32_t*)&recvPkt->msg[0];
	uint8_t str_badcmd[] = "Bad command";

	printf("Smif bad packet command 0x%04x\n", recvPkt->header.command);

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader) + sizeof(uint32_t) + sizeof(str_badcmd);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = 0xffff;
	respPkt->header.service_id = 0;

	*pErrorCode = 0xffff0000;

	return respPkt->header.pkt_size;
}

int SmifHandler(void *recv, void *resp)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
//	struct ChifPkt *respPkt = (struct ChifPkt *)resp;

    dbPrintf("Command: %04x\n", recvPkt->header.command);

	switch(recvPkt->header.command) {
		case 0x0002:
			dbPrintf("smif_0x0002: Get Status\n");
			return SmifPkt_0002(recv, resp);

        case 0x0006:
            dbPrintf("smif_0x0006: Get Network Info\n");
            return smifpkt_0006(recv, resp);

        case 0x0008:
            dbPrintf("smif_0x0008: Set ICRU ready\n");
            return SmifPkt_0008(recv, resp);

        case 0x0035:
            dbPrintf("smif_0x0035: Set PCI Device Info\n");
            return SmifPkt_not_implemented(recv, resp, 0);

        case 0x0050:
            dbPrintf("smif_0x0050: Flash Command\n");
            return SmifPkt_0050(recv, resp);

        case 0x0055:
            dbPrintf("smif_0x0055: Get IOP Date and Time\n");
            return SmifPkt_0055(recv, resp);

        case 0x0056:
            dbPrintf("smif_0x0056: Set IOP Date and Time\n");
            return SmifPkt_not_implemented(recv, resp, 0);

		case 0x0063:
			dbPrintf("smif_0x0063: Get NIC Config\n");
			return SmifPkt_0063(recv, resp);

        case 0x06e:
            dbPrintf("smif_0x006e: Get License\n");
            return SmifPkt_006e(recv, resp);

		case 0x0072:
			dbPrintf("smif_0x0072: I2C Transaction Request\n");
			return SmifPkt_0072(recv, resp);

        case 0x0076:
            dbPrintf("smif_0x0076: Option ROM milestone\n");
            return SmifPkt_not_implemented(recv, resp, 0);

		case 0x0088:
            dbPrintf("smif_0x0088: I/O bit access\n");
            return SmifPkt_0088(recv, resp);

		case 0x011c:
            dbPrintf("smif_0x011c: RIS Blob store\n");
            return SmifPkt_not_implemented(recv, resp, 0);

		case 0x0120:
            dbPrintf("smif_0x0120: Get IPv6 status\n");
            return SmifPkt_0120(recv, resp);

		case 0x012b:
			dbPrintf("smif_0x012B: Get Status BY EV index\n");
			return SmifPkt_012b(recv, resp);

		case 0x012c:
			dbPrintf("smif_0x012C: EV Set/Delete/Delete All\n");
			return SmifPkt_012c(recv, resp);

		case 0x012d:
			dbPrintf("smif_0x012D: Get BIOS Authorization Status\n");
			return SmifPkt_012d(recv, resp);

		case 0x0130:
			dbPrintf("smif_0x0130: Get EV by name\n");
			return SmifPkt_0130(recv, resp);

		case 0x0132:
			dbPrintf("smif_0x0132: Get EV file sys status\n");
			return SmifPkt_0132(recv, resp);

		case 0x0133:
            dbPrintf("smif_0x0133: Get Virtual UART state \n");
            return SmifPkt_0133(recv, resp);

		case 0x0136:
            dbPrintf("smif_0x0136: BIOS Sync response \n");
            return SmifPkt_0136(recv, resp);

		case 0x0139:
			dbPrintf("smif_0x0139: Get security state response\n");
			return SmifPkt_0139(recv, resp);

        case 0x013a:
            dbPrintf("smif_0x013A: Entropy from BIOS\n");
            return SmifPkt_013a(recv, resp);

		case 0x0143:
			dbPrintf("smif_0x0143: BIOS POST State response\n");
			return SmifPkt_0143(recv, resp);

        case 0x0146:
            dbPrintf("smif_0x0146: Quick Event Add\n");
            return SmifPkt_0146(recv, resp);

		case 0x0150:
			dbPrintf("smif_0x0150: Reponse Read CPLD Shutdown & Power Fault\n");
			return SmifPkt_not_implemented(recv, resp, 0);

        case 0x0151:
            dbPrintf("smif_0x0151: SPD Clear Status Request\n");
            return SmifPkt_0151(recv, resp);

        case 0x0153:
            dbPrintf("smif_0x0153: Field Access Request\n");
            return SmifPkt_0153(recv, resp);

        case 0x0155:
            dbPrintf("smif_0x0155: Field Access Request\n");
            return SmifPkt_not_implemented(recv, resp, 0);

        case 0x0158:
            dbPrintf("smif_0x0158: Send BIOS Security States\n");
            return SmifPkt_not_implemented(recv, resp, 0);

        case 0x0159:
            dbPrintf("smif_0x0159: Get Host Interface information Request\n");
			return SmifPkt_not_implemented(recv, resp, 0);

        case 0x0161:
            dbPrintf("smif_0x0161: BIOS Features\n");
            return SmifPkt_not_implemented(recv, resp, 0);

        case 0x0173:
            dbPrintf("smif_0x0173: send (receive) private data\n");
            return SmifPkt_not_implemented(recv, resp, 0);

        case 0x0174:
            dbPrintf("smif_0x0174: REST Task Command Response\n");
            return SmifPkt_not_implemented(recv, resp, 5);

        case 0x0176:
            dbPrintf("smif_0x0176: Set Power Regulator Configuration Data\n");
            return SmifPkt_not_implemented(recv, resp, 0);

        case 0x0177:
            dbPrintf("smif_0x0177: Set Power Regulator Mode\n");
            return SmifPkt_not_implemented(recv, resp, 0);

        case 0x0178:
            dbPrintf("smif_0x0178: Get Device SPDM Status\n");
            return SmifPkt_not_implemented(recv, resp, 0);

        case 0x0179:
            dbPrintf("smif_0x0179: BIOS sends SPD bytes\n");
            return SmifPkt_not_implemented(recv, resp, 0);

        case 0x0182:
            dbPrintf("smif_0x0182: Tinker discovery status\n");
            return SmifPkt_0182(recv, resp);

        case 0x0200:
            dbPrintf("smif_0x0200: Platform APML IO Handler\n");
            return SmifPkt_not_implemented(recv, resp, 0);

        case 0x0202:
            dbPrintf("smif_0x0202: Fetch and send APML records\n");
            return SmifPkt_0202(recv, resp);

        case 0x0209:
            dbPrintf("smif_0x0209: Get BootProgress Policy from BIOS\n");
            return SmifPkt_0209(recv, resp);

		default:
			break;
	}

	return SmifPkt_badcmd(recv, resp);
}
