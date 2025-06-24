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

/****************************************************************************
*  UEFI.h
*
*  Header file for UEFI functionality
*
****************************************************************************/
#ifndef UEFI_H
   #define UEFI_H


   /****************************************************************************
   * Includes
   ****************************************************************************/
//#include "..\..\blobstore\include\blobstore.h"

   typedef struct {
       UINT32  Data1;
       UINT16  Data2;
       UINT16  Data3;
       UINT8   Data4[8];
   } EFI_GUID;

#include "uefi_file_def.h"
         
   /****************************************************************************
   * Defines
   ****************************************************************************/
   typedef enum {
       UEFI_RC_OK,
       UEFI_RC_ERROR,
       UEFI_RC_BADPARAM,
       UEFI_RC_POWER_ON,
       UEFI_RC_NOTFOUND,
       UEFI_RC_CHECKSUM_ERR,
       UEFI_RC_BADCOMMAND,
       UEFI_RC_NOMEMORY,
       UEFI_RC_NOTMODIFIED,
       UEFI_RC_BADDATA,
       UEFI_RC_COULDNOTOBTAINLOCK,
       UEFI_RC_MEDIA_ERROR,
       UEFI_RC_PARTITION_ERROR,
       UEFI_RC_END_OF_MEDIA,
       UEFI_RC_FILE_ERROR,
       UEFI_RC_PATH_ERROR,
       UEFI_RC_SIZE_ERROR,
       UEFI_RC_SPI_ERROR
   } UEFI_RC;

   typedef enum
   {
       UEFI_FS_NONE = 0,
       UEFI_FS_ILO,
       UEFI_FS_PTID,
       UEFI_FS_EEPROM,
       UEFI_FS_MAX
   } UEFI_PTID_FILE_SYSTEM;

   typedef enum
   {
       UEFI_OP_END = 0,
       UEFI_OP_MAKE_DIR,
       UEFI_OP_REMOVE_DIR,
       UEFI_OP_WRITE_FILE,
       UEFI_OP_WRITE_BLOB,
       UEFI_OP_REMOVE_FILE,
       UEFI_OP_RECURSIVE_REMOVE,
       UEFI_OP_STAIN,
       UEFI_OP_MAX
   } UEFI_PTID_OPERATION;


  /****************************************************************************
   * Structures and Types
   ****************************************************************************/
   /* Simulated boolean since there doesn't seem to be a standard one */
   typedef UINT8   uefi_bool;
#define UEFI_FALSE    0
#define UEFI_TRUE     1

#define MAX_PATH             (160)

#define VOL_DE_READ_RETRIES  (1)   // ROM Data Extract read retry count
#define VOL_DE_READ_MS_DELAY (100) // ROM Data Extract read retry delay mSecs

#define AMP_SROM_PARTS_UEFI_MAP_FILE  "i:/vol0/cfg/sromMap_details.bin"

#define MAX_RETRY_APML_FILE_ACCESS   3 //Used only in ARM based platforms.

   typedef struct {
       int      active;         // side index of active side (per NVRAM header byte)
       struct {
        int             valid;
        char            rom_sysid[8];
        unsigned int    rom_major_ver;
        unsigned int    rom_minor_ver;
        unsigned int    rom_pass;
        char            rom_date[16];
        char            rom_ver_str[32];
       } side[2];   // Side A/B  (A = 0, B = 1)
   } UEFI_BIOS_VERSION;

   // which side of the ROM do we want to address?
   typedef enum {
       UEFI_REDROM_SIDE_A,         // Side A of ROM (regardless of active/passive)
       UEFI_REDROM_SIDE_B,         // Side B of ROM (regardless of active/passive)
       UEFI_REDROM_SIDE_ACTIVE,    // Active side of ROM (based upon NVRAM header byte)
       UEFI_REDROM_SIDE_BACKUP     // Passive side of ROM (based upon NVRAM header byte)
   } UEFI_REDROM_SIDE;

   typedef struct {
      UINT32       file_position;
      union {
         struct {
             EFI_FIRMWARE_VOLUME_HEADER     volHdr;
             EFI_FIRMWARE_VOLUME_EXT_HEADER volExtHdr;
             char                           extHdr[40];
         } volInfo;
         struct {
             EFI_FFS_FILE_HEADER        fileHdr;
             EFI_COMMON_SECTION_HEADER  comHdr;
         } fileInfo;
      } header;
   } UEFI_PTID_FILE_INFO;

   // NVRAM file that holds the Platdef Offset and Size
   typedef struct
   {
       UINT32 valid;
       UINT32 offset;          //offset of bios part on BIOS ROM
       UINT32 size;            //size of bios part data
       UINT32 uefi_NOR_offset; //start addr of bios part on  UEFI ROM
       UINT32 platdef_version; //bios part version
       UINT32 reserved;
   } HPE_PLATDEFN_INFO;
   typedef struct
   {
       UINT32 valid;
       UINT32 offset;          //offset of bios part on BIOS ROM
       UINT32 size;            //size of bios part data
       UINT32 uefi_NOR_offset; //start addr of bios part on  UEFI ROM
       UINT32 version; //bios part version
       UINT32 reserved;
   } HPE_overlay_INFO;

   typedef struct
   {
       UINT32 valid;
       UINT32 offset;          //offset of bios part on BIOS ROM
       UINT32 size;            //size of bios part data
       UINT32 uefi_NOR_offset; //start addr of bios part on  UEFI ROM
       UINT32 version; //bios part version
       UINT32 reserved;
   } HPE_embed_INFO;

  typedef struct
   {
       UINT32 valid;
       UINT32 offset;          //offset of bios part on BIOS ROM
       UINT32 size;            //size of bios part data
       UINT32 uefi_NOR_offset; //start addr of bios part on  UEFI ROM
       UINT32 version; //bios part version
       UINT32 reserved;
   } HPE_EVTLOG_INFO;

typedef struct
{
    HPE_PLATDEFN_INFO platinfo_cfg;
    HPE_overlay_INFO overlay_prebootinfo_cfg;
    HPE_overlay_INFO overlay_oem_prebootinfo_cfg;
    HPE_overlay_INFO overlay_PCDinfo_cfg;
    HPE_embed_INFO embedinfo4GB_cfg;//4GB
    HPE_embed_INFO embedinfo2GB_cfg;//2G
    HPE_EVTLOG_INFO evtlog_cfg;
    UINT8 reserved[32];//for future
}HPE_BIOS_PARTS_NVRAM_CFG;

  /****************************************************************************
   * Well Known GUIDs
   ****************************************************************************/
//   extern const EFI_GUID EFIGUID_FV_iLO;      // iLO firmware volume
//   extern const EFI_GUID EFIGUID_File_SMSCFW; // SMSC USX2065 firmware image
//   extern const EFI_GUID EFIGUID_File_BezelImage; // Bezel image
//   extern const EFI_GUID EFIGUID_File_FWBuildManifest; // UEFI System Firmware Build Manifest File
//   extern const EFI_GUID EFIGUID_File_Bootblock; // BIOS bootblock ($SIGFLSH)
//   extern const EFI_GUID EFIGUID_File_UEFIRomRisBiosClassesGz; // UEFI System Firmware RIS Bios Classes
//   extern const EFI_GUID EFIGUID_File_UEFIRomRisBiosClassesJson; // UEFI System Firmware RIS Bios Classes Descriptor info
//   extern const EFI_GUID EFIGUID_Volume_ROMSigManifest; // HP_SIG_MANIFEST_FV_GUID
//   extern const EFI_GUID EFIGUID_File_ROMSigManifest; // HP_SIG_MANIFEST_FILE_GUID
//   extern const EFI_GUID EFIGUID_FV_APML;      // APML FV 
//   extern const EFI_GUID EFIGUID_File_APML;    // APML binary
//   extern const EFI_GUID EFIGUID_FV_OEM_APML;  // Optional OEM APML FV
//   extern const EFI_GUID EFIGUID_File_OEM_APML;// Optional OEM APML binary
//   extern const EFI_GUID EFIGUID_File_DataExtract; // ROM's Supplemental Data Extract Binary File GUID
//   extern const EFI_GUID EFIGUID_FV_BIOS_IMAGES; // ROM BMP FV
//   extern const EFI_GUID EFIGUID_FV_OEM_BIOS_IMAGES; // OEM ROM BMP FV
//   extern const EFI_GUID EFIGUID_File_PCD_Structures;      // ROM PCD Structures
//   extern const EFI_GUID EFIGUID_File_BIOS_HPLogoRightBanner; // ROM BMP Images
//   extern const EFI_GUID EFIGUID_File_BIOS_ProLiantLogo; // ROM BMP Images
//   extern const EFI_GUID EFIGUID_File_BIOS_PreBootHealthSummary;
//   extern const EFI_GUID EFIGUID_File_OEM_BIOS_HPLogoRightBanner; // ROM BMP Images
//   extern const EFI_GUID EFIGUID_File_OEM_BIOS_ProLiantLogo; // ROM BMP Images
//   extern const EFI_GUID EFIGUID_File_OEM_BIOS_PreBootHealthSummary;

/****************************************************************************
   * Function Prototypes
   ****************************************************************************/

   // Find offset and size in SPI ROM of UEFI file (by firmware volume/file name)
   // returns UEFI_OK or UEFI_NOT_FOUND
//   extern UEFI_RC uefi_file_find(   const UEFI_REDROM_SIDE, const EFI_GUID* pFwVolGUID, const EFI_GUID* pFwFileGUID,
//                                    UINT32* pOffset, UINT32* pSize, UINT8* pChecksum,
//                                    const unsigned int retry_count, const unsigned int ms_retry_interval);
//
//   // Find offset and size in SPI ROM of UEFI FV
//   // returns UEFI_OK or UEFI_NOT_FOUND
//   extern UEFI_RC uefi_fv_find(     const UEFI_REDROM_SIDE, const EFI_GUID* pFwVolGUID,
//                                    UINT32* pOffset, UINT32* pSize);
//
//   // Cache a UEFI file to the blob store
//   extern UEFI_RC uefi_file_to_blobstore_copy( const UEFI_REDROM_SIDE, const EFI_GUID* pFwVolGUID, const EFI_GUID* pFwFileGUID,
//                                    const blobstore_namespace nspace, const char* blob_name,
//                                    const char* content_type, const char* content_encoding, const char* description,
//                                    const unsigned int retry_count, const unsigned int ms_retry_interval );
//
//   extern UEFI_RC uefi_rom_version_get( UEFI_BIOS_VERSION* );
//
//   extern UEFI_RC uefi_util_read_bmp_image_from_bios(const UEFI_REDROM_SIDE, const EFI_GUID* pFwVolGUID, const EFI_GUID* pFwFileGUID,
//                                    UINT32* pOffset, UINT32* pSize, UINT8* pChecksum,
//                                    const unsigned int retry_count, const unsigned int ms_retry_interval, UINT8* data_buffer);
//
//   extern UEFI_RC uefi_create_ilostore_file (const char *filename, const char *pdata);
//
//   extern UEFI_RC uefi_find_ptid_guid (const EFI_GUID *guid, UEFI_PTID_FILE_INFO *fileInfo);
//   extern UEFI_RC uefi_find_ptid_file (const char *filename, UEFI_PTID_FILE_INFO *fileInfo);
//   extern UEFI_RC uefi_create_ptid_file (const EFI_GUID *guid, const char *filename);
//   extern UEFI_RC uefi_list_ptid (UINT32 file_position, UEFI_PTID_FILE_INFO *fileInfo);
//   extern UEFI_RC uefi_process_ptid (const char *filename);
//   extern UEFI_RC platdef_data_store(UINT8 bios_flash);
   

#define PLATDEF_UPDATE_BUF_SZ               (256*1024)  // Platdef buffer size //PLATDEF_UPDATE_BUF_SIZE//
#define MAGIC_NUM                           (0x5A5A)
#define PLATDEF_OFFSET_UEFISTORE            (0x10000)////160KB
#define NO_OEM_PREBOOT_OFFSET_UEFISTORE     (0x60000)//100KB
#define OEM_PREBOOT_OFFSET_UEFISTORE        (0x80000)//100KB
#define PDC_OFFSET_UEFISTORE                (0xa0000) //500bytes
#define EMBED_NAND4G_OFFSET_UEFISTORE       (0xa1000) //1056 bytes
#define EMBED_NAND2G_OFFSET_UEFISTORE       (0xa2000)//1056 bytes
#define SUPPL_DATA_OFFSET_UEFISTORE         (0xa3000) //128KB supplemental data extract from ROM FV
#define RESERVE_DATA_OFFSET_UEFISTORE       (0xc3000) //128KB supplemental data extract from ROM FV

#define ALLOCATED_PLATDEF_DATA_SIZE         0X50000
#define ALLOCATED_PREBOOT_DATA_SIZE         0X20000
#define ALLOCATED_PDC_DATA_SIZE             0X1000
#define ALLOCATED_PARTITION_DATA_SIZE       0X1000
#define ALLOCATED_SUPPL_DATA_SIZE           0X20000

#endif
