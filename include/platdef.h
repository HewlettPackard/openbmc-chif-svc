/*
// Copyright (c) 2012-2025 Hewlett Packard Enterprise Development, LP
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

/*
        *** CRITICAL ***

        The following must be kept in sync:

        docs\APML Platform Definition Specification.docx
        frostbyte\Compiler\Records\*.cs
        frostbyte\Compiler\Primitives\*.cs
        frostbyte\PlatDefBinary\Records\*.cs
        frostbyte\PlatDefBinary\Primitives\*.cs
        frostbyte\PlatDefData\Records\*.cs
        frostbyte\PlatDefData\Primitives\*.cs
        inc\platdef.h                                         <-- You are here
*/

#pragma once

#ifndef INCL_APML_PLATDEF_H
#define INCL_APML_PLATDEF_H


#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push,1)

//#if !defined(WIN32) && !defined(APML_BASIC_TYPES_DEFINED)
//typedef signed char    INT8;
//typedef signed short   INT16;
//typedef signed int     INT32;
//typedef unsigned char  UINT8;
//typedef unsigned short UINT16;
//typedef unsigned int   UINT32;
//#define APML_BASIC_TYPES_DEFINED
//#endif

//////////////////////////////////////////////////////////////////////////////
//
//  Misc
//
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//  FP100
//----------------------------------------------------------------------------

typedef INT16 FP100;

//----------------------------------------------------------------------------
//  PlatDefReaction
//----------------------------------------------------------------------------

typedef UINT32 PlatDefReaction;

#define MAXIMUM_NUMBER_REACTIONS 24
#define NUM_THROTTLE_REACTIONS   16
// The BOX ID info is from EE, the maximum is 9 for current Gen11 design
#define NUM_THROTTLE_BOXIDS 16

enum PlatDefReactionBits
{
    Reaction_DedicatedIndicator     = 0x00000001,
    Reaction_DegradedHealthLED      = 0x00000002,
    Reaction_CriticalHealthLED      = 0x00000004,
    Reaction_Blowout                = 0x00000008,
    Reaction_GracefulShutdown       = 0x00000010,
    Reaction_GracefulShutdownIfEAAS = 0x00000020,
    Reaction_ImmediatePowerOff      = 0x00000040,
    Reaction_ImmediateReset         = 0x00000080,
    Reaction_Throttle1              = 0x00000100,
    Reaction_Throttle2              = 0x00000200,
    Reaction_Throttle3              = 0x00000400,
    Reaction_Throttle4              = 0x00000800,
    Reaction_Throttle5              = 0x00001000,
    Reaction_Throttle6              = 0x00002000,
    Reaction_Throttle7              = 0x00004000,
    Reaction_Throttle8              = 0x00008000,
    Reaction_Throttle9              = 0x00010000,
    Reaction_Throttle10             = 0x00020000,
    Reaction_Throttle11             = 0x00040000,
    Reaction_Throttle12             = 0x00080000,
    Reaction_Throttle13             = 0x00100000,
    Reaction_Throttle14             = 0x00200000,
    Reaction_Throttle15             = 0x00400000,
    Reaction_Throttle16             = 0x00800000,
    Reaction_PowerRelated           = Reaction_GracefulShutdown       |
                                      Reaction_GracefulShutdownIfEAAS |
                                      Reaction_ImmediatePowerOff      |
                                      Reaction_ImmediateReset
};

//----------------------------------------------------------------------------
//  PlatDefDevInitData
//----------------------------------------------------------------------------

typedef struct {

    UINT8   Register;
    UINT8   Data;

} PlatDefDevInitData;

//----------------------------------------------------------------------------
//  IPMISensorOwner
//----------------------------------------------------------------------------

typedef union {

    struct {

        UINT8   SystemSoftwareOwnsSensor : 1;
        UINT8   SensorOwnerID : 7;

        UINT8   SensorOwnerLUN : 2;
        UINT8   __Byte_07_Bit_2_3 : 2;
        UINT8   SensorChannelNumber : 4;

    } SensorOwnerData;

    UINT8       RawBytes[2];

} IPMISensorOwner;

//----------------------------------------------------------------------------
//  PrimitiveCommonFields
//----------------------------------------------------------------------------

typedef struct {

   UINT16   Flags;
   UINT8    Type;

} PrimitiveCommonFields;

//----------------------------------------------------------------------------
//  PlatDefPrimitiveFlag
//----------------------------------------------------------------------------

enum PlatDefPrimitiveFlag
{
   PrimitiveFlag_MaskUsage        = 0x1000|0x2000,
   PrimitiveFlag_ReturnsFP100     = 0x4000,
   PrimitiveFlag_UsePositiveLogic = 0x8000
};

//----------------------------------------------------------------------------
//  MaskUsage
//----------------------------------------------------------------------------

enum MaskUsage
{
    Mask_None    = 0,
    Mask_AND     = 0x1000,
    Mask_OR      = 0x2000,
    Mask_XOR     = 0x3000,
    Mask_Divisor = 0x3000
};

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefPrimitive
//
//  Also includes:
//
//  ConstantPrimitive
//  GPIPrimitive
//  GPOPrimitive
//  ScanChainPrimitive
//  DoubleBitGPIPrimitive
//  I2CPrimitive
//  NoisyI2CPrimitive
//  DeviceRegisterPrimitive
//  GeneralPrimitive
//  IntelIOHPrimitive
//  IntelPCHPrimitive
//  SpecialI2CPrimitive
//  PSGroupsPrimitive
//  OCSDPrimitive
//  PLDMPrimitive
//  IPMISatCtrlPrimitive
//  APMLDataPrimitive
//  BlockTransferPrimitive
//
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//  ConstantPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   Value;

} ConstantPrimitive;

//----------------------------------------------------------------------------
//  GPIPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   Bit;
    UINT8   DebounceDelay;

} GPIPrimitive;

enum GPIPrimitiveFlag
{
    GPIFlag_UsePositiveLogic = PrimitiveFlag_UsePositiveLogic
};

//----------------------------------------------------------------------------
//  GPOPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   Bit;
    UINT8   Byte;
    UINT8   Mask;

} GPOPrimitive;

enum GPOPrimitiveFlag
{
    GPOFlag_ByteMode         = 0x0001,
    GPOFlag_MaskUsage        = PrimitiveFlag_MaskUsage,
    GPOFlag_UsePositiveLogic = PrimitiveFlag_UsePositiveLogic
};

//----------------------------------------------------------------------------
//  ScanChainPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   Byte;
    UINT8   Mask;
    UINT8   DebounceDelay;

} ScanChainPrimitive;

enum ScanChainPrimitiveFlag
{
    ScanChainFlag_PerformComparison = 0x0001,
    ScanChainFlag_DoubleBit         = 0x0002,
    ScanChainFlag_BooleanNegate     = 0x0004,
    ScanChainFlag_MaskUsage         = PrimitiveFlag_MaskUsage,
    ScanChainFlag_UsePositiveLogic  = PrimitiveFlag_UsePositiveLogic
};

//----------------------------------------------------------------------------
//  DoubleBitGPIPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   Byte;
    UINT8   Mask;
    UINT8   DebounceDelay;

} DoubleBitGPIPrimitive;

enum DoubleBitGPIPrimitiveFlag
{
    DoubleBitGpiFlag_UsePositiveLogic = PrimitiveFlag_UsePositiveLogic
};

//----------------------------------------------------------------------------
//  I2CPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   Bus;
    UINT8   Address;
    UINT8   Register;
    UINT8   Register2;
    UINT8   Mask;
    UINT8   Shift;

} I2CPrimitive;

enum I2CPrimitiveFlag
{
    I2CFlag_FP100Style       = 0x0001|0x0002|0x0004,
    I2CFlag_ReadStyle        = 0x0040|0x0020|0x0010|0x0008,
    I2CFlag_BooleanNegate    = 0x0080,
    I2CFlag_DiscoveryPhase   = 0x0100,
    I2CFlag_UnsignedRead     = 0x0200,
    I2CFlag_MaskUsage        = PrimitiveFlag_MaskUsage,
    I2CFlag_ReturnsFP100     = PrimitiveFlag_ReturnsFP100,
    I2CFlag_UsePositiveLogic = PrimitiveFlag_UsePositiveLogic
};

enum I2CFP100Style
{
    I2CFP100Style_Native       = 0,
    I2CFP100Style_EFuse        = 1,
    I2CFP100Style_AMDProcessor = 2
};

enum I2CReadStyle
{
    I2CReadStyle_OneRegister  = 0x0000,
    I2CReadStyle_TwoRegisters = 0x0008,
    I2CReadStyle_LittleEndian = 0x0018,
    I2CReadStyle_BigEndian    = 0x0028,
    I2CReadStyle_AckPoll      = 0x0040
};

//----------------------------------------------------------------------------
//  NoisyI2CPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   Bus;
    UINT8   Address;
    UINT8   Register;
    UINT8   Mask;

} NoisyI2CPrimitive;

enum NoisyI2CPrimitiveFlag
{
    NoisyI2CFlag_UseAddressInChecksum = 0x0001,
    NoisyI2CFlag_UseOffsetAsGeneral   = 0x0002,
    NoisyI2CFlag_UINT16Action         = 0x0004|0x0008,
    NoisyI2CFlag_UINT16_Action        = NoisyI2CFlag_UINT16Action,
      NoisyI2CFlag_UseOnlyLSB           = 0x0004,
      NoisyI2CFlag_UseOnlyMSB           = 0x0008,
    NoisyI2CFlag_UseCommandByte       = 0x0010,
    NoisyI2CFlag_BooleanNegate        = 0x0020,
    NoisyI2CFlag_DiscoveryPhase       = 0x0040,
    NoisyI2CFlag_BrokenMegacell       = 0x0080,
    NoisyI2CFlag_MaskUsage            = PrimitiveFlag_MaskUsage,
    NoisyI2CFlag_ReturnsFP100         = PrimitiveFlag_ReturnsFP100,
    NoisyI2CFlag_UsePositiveLogic     = PrimitiveFlag_UsePositiveLogic
};

//----------------------------------------------------------------------------
//  DeviceRegisterPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   Device;
    UINT8   Mask;
    UINT8   ReadType;
    UINT16  Register;

} DeviceRegisterPrimitive;

enum DevRegPrimitiveFlag
{
    DevRegFlag_Use16Bits        = 0x0001,
    DevRegFlag_DoubleBit        = 0x0002,
    DevRegFlag_BooleanNegate    = 0x0004,
    DevRegFlag_MaskUsage        = PrimitiveFlag_MaskUsage,
    DevRegFlag_ReturnsFP100     = PrimitiveFlag_ReturnsFP100,
    DevRegFlag_UsePositiveLogic = PrimitiveFlag_UsePositiveLogic
};

enum DeviceType
{
    DeviceType_CPLD                  = 1,
    DeviceType_PECIRegisters         = 2,
    DeviceType_PECIGroups            = 3,
    DeviceType_DIMMRegisters         = 4,
    DeviceType_DIMMGroups            = 5,
    DeviceType_DiskDriveRegisters    = 6,
    DeviceType_DiskDriveGroups       = 7,
    DeviceType_IntelDTS              = 8,
    DeviceType_MemoryBufferRegisters = 9,
    DeviceType_MemoryBufferGroups    = 10,
    DeviceType_IntelDTSTemp          = 11,
    DeviceType_LRDIMMGroups          = 12,
    DeviceType_FanRegisters          = 13,
    DeviceType_TachRegisters         = 14,
    DeviceType_SensorGroup           = 15,
    DeviceType_CANmic                = 16,
    DeviceType_iLO                   = 17,
    DeviceType_NVMe                  = 18,
    DeviceType_AHCIDriveGroups       = 19,
    DeviceType_IntelNM               = 20
};

enum ReadType
{
    // Use with DeviceType_PECIRegisters or DeviceType_PECIGroups

    ReadType_PECITemperature        = 0,
    ReadType_PECIHeatScale          = 1,
    ReadType_PECICount              = 2,
    ReadType_TControlHeatScale      = 3,

    // Use with DeviceType_IntelDTS(except PCHTemperature)
    // DTSOffset, DTS2Point0 and PCHTemperature used with DeviceType.IntelNM

    ReadType_DTSOffset              = 0,
    ReadType_TControl               = 1,
    ReadType_DTSCount               = 2,
    ReadType_UITemp                 = 3,
    ReadType_UITemp_UINT8           = 4,
    ReadType_TCCA                   = 5,
    ReadType_DTSNet                 = 6,
    ReadType_DTS2Point0             = 7,
    ReadType_CPURealTemperature     = 8,
    ReadType_IPMBSensorReading      = 9,

    // Use with DeviceType_DIMMRegisters or DeviceType_DIMMGroups

    ReadType_DIMMTemperature        = 0,
    ReadType_DIMMHeatScale          = 1,
    ReadType_DIMMCount              = 2,
    ReadType_DIMMHighTemp           = 3,
    ReadType_HottestDIMMID          = 4,
    ReadType_STDIMMTemp             = 5,
    ReadType_STDIMMCount            = 6,
    ReadType_NVDIMMPTemp            = 7,
    ReadType_NVDIMMPCount           = 8,
    ReadType_NumDIMMReadTypes       = 9,  //Needs to be the last item in enum

    // Use with DeviceType_DiskDriveRegisters or DeviceType_DiskDriveGroups

    ReadType_HDDTemperature         = 0,
    ReadType_HDDHottestRaw          = 4,
    ReadType_HDDCount               = 8,

    // Use with DeviceType_MemoryBufferRegisters or DeviceType_MemoryBufferGroups

    ReadType_MBTemperature          = 0,
    ReadType_HottestMBID            = 1,
    ReadType_MBCount                = 2,
    ReadType_HighTemp               = 3,
    ReadType_NumMBReadTypes         = 4,

    // Use with DeviceType_LRDIMMGroups

    ReadType_LRDIMMTemperature      = 0,
    ReadType_LRDIMMCount            = 2,

    // Use with DeviceType_SensorGroup

    ReadType_SensorGroupValue       = 0,
    ReadType_SensorGroupActive      = 2,
    ReadType_SensorGroupTotal       = 3,
    ReadType_SensorGroupStatus      = 4,

    // Use with DeviceType_iLO

    ReadType_iLOCoreTemperature     = 0,

    // Use with DeviceType_NVMe

    ReadType_NVMeTemperature        = 0,
    ReadType_NVMeCount              = 1,
    ReadType_NVMeThrottleBoxID      = 2,
    ReadType_NVMeM2                 = 3,

    // Use with DeviceType_CPLD

    //ReadType_Generic                = 0,
    ReadType_eFuseTempZone          = 1,

    // Use with all other device types

    ReadType_Generic                = 0
};

typedef union {

    UINT16  AsUINT16;

    struct {

        UINT8   Identifier;
        UINT8   Flags;

    } Data;

} DiskDriveGroupsRegister;

enum DiskDriveGroupsRegisterFlag
{
    DiskGroupFlag_IdentifierType = 0x01|0x02|0x04,
      DiskGroupFlag_Enclosure      = 0,
      DiskGroupFlag_I2CBox         = 1,
    DiskGroupFlag_AllExcept      = 0x08
};

//----------------------------------------------------------------------------
//  GeneralPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   Offset;
    UINT8   Mask;

} GeneralPrimitive;

enum GeneralPrimitiveFlag
{
    GenRegFlag_BooleanNegate    = 0x0001,
    GenRegFlag_MaskUsage        = PrimitiveFlag_MaskUsage,
    GenRegFlag_UsePositiveLogic = PrimitiveFlag_UsePositiveLogic
};

enum GeneralRegister
{
    GenReg_OA_TEMP          = 0x00,
    GenReg_OA_STATUS        = 0x01,
    GenReg_HDD_STATUS       = 0x30,
    GenReg_HDD_COUNT        = 0x31,
    GenReg_CTLR_BASE_TEMPS  = 0x40,
    GenReg_CTLR_BASE_DELTAS = 0x48,
    GenReg_PSUPOWER         = 0x70,
    GenReg_IDA_STATUS       = 0x80
};

//----------------------------------------------------------------------------
//  IntelIOHPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   IOHBus;
    UINT8   IOHAddress;
    UINT8   PCIBus;
    UINT8   PCIDevFunc;
    UINT8   Register1;
    UINT8   Register2;

} IntelIOHPrimitive;

enum IOHPrimitiveFlag
{
    IOHFlag_MathOp         = 0x0001|0x0002,
    IOHFlag_Temperature    = 0x0004,
    IOHFlag_Reg2IsConstant = 0x0008
};

enum IOHMathOp
{
    IOHMathOp_Add      = 0,
    IOHMathOp_Subtract = 1,
    IOHMathOp_Multiply = 2,
    IOHMathOp_Divide   = 3
};

//----------------------------------------------------------------------------
//  IntelPCHPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   Bus;
    UINT8   Address;
    UINT8   Register;
    UINT8   Mask;

} IntelPCHPrimitive;

//----------------------------------------------------------------------------
//  SpecialI2CPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   Bus;
    UINT8   Address;
    UINT8   Slave;
    UINT8   Command;
    UINT8   WriteRegister;
    UINT8   GPUCount;
    UINT8   Page;

} SpecialI2CPrimitive;

enum SpecialI2CPrimitiveFlag
{
    SpecialI2CFlag_DeviceType   = 0x0001|0x0002|0x0004|0x0008,
    SpecialI2CFlag_VRDPageWrite = 0x0010,
    SpecialI2CFlag_VRDSecSrcChkDone = 0x0020,           // set by iLO after checking if VRD sensor has secondary source
    SpecialI2CFlag_VRDHasSecSrc     = 0x0040            // set by iLO if the VRD sensor has secondary source
};

enum SpecialI2CDeviceType
{
    SpecialDevice_Volterra         = 0,
    SpecialDevice_Mellanox         = 1,
    SpecialDevice_NVidia           = 2,
    SpecialDevice_InfineonExtender = 3,
    SpecialDevice_VRD              = 4,
    SpecialDevice_WriteRead        = 5,
    SpecialDevice_BroadcomSWB      = 6
};

//----------------------------------------------------------------------------
//  LM75I2CPrimitive
//----------------------------------------------------------------------------

typedef struct
{
     UINT16 Flags;
     UINT8  Type;
     UINT8  Procnum;
     UINT8  Bus;
     UINT8  Address;
     UINT8  Register;
     UINT8  Register2;
     UINT8  Misc;

}LM75I2CPrimitive;

//----------------------------------------------------------------------------
//  PSGroupsPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   DataType;
    UINT8   Supplies;

} PSGroupsPrimitive;

enum PSDataType
{
    PSDataType_TempPercent         = 0,
    PSDataType_Temperature         = 1,
    PSDataType_InternalTempPercent = 2,
    PSDataType_InternalTemperature = 3
};

//----------------------------------------------------------------------------
//  OCSDPrimitive (deprecated)
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   SensorType;
    UINT8   SlotNumber;
    UINT8   DevType;
    UINT8   DevTypeInstance;
    UINT8   DevTypeHandle;

} OCSDPrimitive;

enum OCSDSensorType
{
    OCSDSensorType_Other              = 1,
    OCSDSensorType_CardInlet          = 2,
    OCSDSensorType_OffBoardSmartArray = 3
};

enum OCSDDeviceType
{
    OCSDDeviceType_OptionCard = 0,
    OCSDDeviceType_Other      = 1,
    OCSDDeviceType_Unknown    = 2,
    OCSDDeviceType_Video      = 3,
    OCSDDeviceType_SCSI       = 4,
    OCSDDeviceType_Ethernet   = 5,
    OCSDDeviceType_TokenRing  = 6,
    OCSDDeviceType_Sound      = 7,
    OCSDDeviceType_PATA       = 8,
    OCSDDeviceType_SATA       = 9,
    OCSDDeviceType_SAS        = 10
};

//----------------------------------------------------------------------------
//  PLDMPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   DeviceType;
    UINT16  Slot;
    UINT8   OnboardType;
    UINT8   OnboardInstance;
    UINT8   SensorType;

} PLDMPrimitive;

enum PLDMSensorType
{
    PLDMSensorType_Other              = 0,
    PLDMSensorType_CardInlet          = 1,
    PLDMSensorType_CardHighMCOT       = 2,
    PLDMSensorType_CardLowMCOT        = 3
};

//----------------------------------------------------------------------------
//  AcceleratorPrimitive
//----------------------------------------------------------------------------

typedef struct {

	UINT16  Flags;
	UINT8   Type;
	UINT8	_Reserved1;
	UINT16  Slot;
	UINT8   SensorType;

} AcceleratorPrimitive;

enum AcceleratorSensorType
{
	AcceleratorSensorType_StandardGPU = 0
};

//----------------------------------------------------------------------------
//  IPMISatCtrlPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   Bus;
    UINT8   Address;
    UINT8   SensorIndex;
    UINT16  Mask;

} IPMISatCtrlPrimitive;

enum IPMISatCtrlPrimitiveFlag
{
    IPMISatFlag_DeviceType       = 0x0001|0x0002|0x0004|0x0008,
    IPMISatFlag_DataType         = 0x0010|0x0020|0x0040,
    IPMISatFlag_BooleanNegate    = 0x0080,
    IPMISatFlag_MaskUsage        = PrimitiveFlag_MaskUsage,
    IPMISatFlag_UsePositiveLogic = PrimitiveFlag_UsePositiveLogic
};

enum IPMISatCtrlDeviceType
{
    IPMISatDevice_KNC = 0
};

enum IPMISatCtrlDataType
{
    IPMISatDataType_Analog           = (0 << 4),
    IPMISatDataType_Discrete         = (1 << 4),
    IPMISatDataType_OperationalState = (2 << 4)
};

//----------------------------------------------------------------------------
//  APMLDataPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   Size;
    UINT16  RecordID;
    UINT16  Offset;
    UINT16  Mask;

} APMLDataPrimitive;

enum APMLDataPrimitiveFlag
{
    APMLDataFlag_MaskUsage        = PrimitiveFlag_MaskUsage,
    APMLDataFlag_ReturnsFP100     = PrimitiveFlag_ReturnsFP100,
    APMLDataFlag_UsePositiveLogic = PrimitiveFlag_UsePositiveLogic
};

enum APMLDataPrimitiveSize
{
    APMLDataSize_Byte   = 1,
    APMLDataSize_UINT16 = 2
};

//----------------------------------------------------------------------------
//  BlockTransferPrimitive
//----------------------------------------------------------------------------

typedef struct {

    UINT16  Flags;
    UINT8   Type;
    UINT8   Size;
    UINT16  Block;
    UINT16  Offset;
    UINT32  Mask;
    UINT8   Bus;
    UINT8   Address;
    UINT8   Shift;

} BlockTransferPrimitive;

enum BlkXferPrimitiveFlag
{
    BlkXferFlag_DeviceType       = 0x0001|0x0002|0x0004,
    BlkXferFlag_BooleanNegate    = 0x0008,
    BlkXferFlag_BigEndian        = 0x0010,
    BlkXferFlag_MaskUsage        = PrimitiveFlag_MaskUsage,
    BlkXferFlag_ReturnsFP100     = PrimitiveFlag_ReturnsFP100,
    BlkXferFlag_UsePositiveLogic = PrimitiveFlag_UsePositiveLogic
};

enum BlkXferDevType
{
    BlkXferDevType_Unspecified = 0,
    BlkXferDevType_Cobourg     = 1
};

enum BlkXferPrimitiveSize
{
    BlkXferSize_Byte   = 1,
    BlkXferSize_UINT16 = 2,
    BlkXferSize_UINT32 = 3
};

//----------------------------------------------------------------------------
//  PlatDefPrimitive
//----------------------------------------------------------------------------

typedef union {

    UINT8   RawBytes[16];

    PrimitiveCommonFields   Global;
      //Contains UINT16 Flags and UINT8 Type for access to all primitives

    ConstantPrimitive       Constant;
    GPIPrimitive            GPI;
    GPOPrimitive            GPO;
    ScanChainPrimitive      ScanChain;
    DoubleBitGPIPrimitive   DoubleBitGPI;
    I2CPrimitive            I2C;
    NoisyI2CPrimitive       NoisyI2C;
    DeviceRegisterPrimitive DeviceRegister;
    GeneralPrimitive        General;
    IntelIOHPrimitive       IntelIOH;
    IntelPCHPrimitive       IntelPCH;
    SpecialI2CPrimitive     SpecialI2C;
    PSGroupsPrimitive       PSGroups;
    OCSDPrimitive           OCSD;  //deprecated
    PLDMPrimitive           PLDM;
	AcceleratorPrimitive    Accelerator;
    IPMISatCtrlPrimitive    IPMISatCtrl;
    APMLDataPrimitive       APMLData;
    BlockTransferPrimitive  BlockTransfer;
    LM75I2CPrimitive        LM75I2C;

} PlatDefPrimitive;

enum PlatDefPrimitiveType
{
    PrimitiveType_Constant       = 0,
    PrimitiveType_GPI            = 1,
    PrimitiveType_GPO            = 2,
    PrimitiveType_ScanChain      = 3,
    PrimitiveType_DoubleBitGPI   = 6,
    PrimitiveType_I2C            = 4,
    PrimitiveType_NoisyI2C       = 11,
    PrimitiveType_DeviceRegister = 12,
    PrimitiveType_General        = 13,
    PrimitiveType_IntelIOH       = 18,
    PrimitiveType_SpecialI2C     = 23,
    PrimitiveType_PSGroups       = 26,
    PrimitiveType_OCSD           = 27,  //deprecated
    PrimitiveType_IntelPCH       = 28,
    PrimitiveType_IPMISatCtrl    = 30,
    PrimitiveType_PLDM           = 31,
    PrimitiveType_APMLData       = 32,
    PrimitiveType_BlockTransfer  = 33,
    PrimitiveType_Accelerator    = 34,
    PrimitiveType_LM75I2C        = 35
};
                                        /* 0123456789012 */
#define PLATDEF_PRIMITIVE_TYPE_S(x) (                   \
    (x==PrimitiveType_Constant)         ? "BOOLEAN":    \
    (x==PrimitiveType_GPI)              ? "GPI":        \
    (x==PrimitiveType_GPO)              ? "GPO":        \
    (x==PrimitiveType_ScanChain)        ? "MEMID":      \
    (x==PrimitiveType_DoubleBitGPI)     ? "DualBit":    \
    (x==PrimitiveType_I2C)              ? "I2C":        \
    (x==PrimitiveType_NoisyI2C)         ? "Noisy":      \
    (x==PrimitiveType_DeviceRegister)   ? "DevReg":     \
    (x==PrimitiveType_General)          ? "General":    \
    (x==PrimitiveType_IntelIOH)         ? "IntelIOH":   \
    (x==PrimitiveType_SpecialI2C)       ? "Special I2C":\
    (x==PrimitiveType_PSGroups)         ? "PS Group":   \
    (x==PrimitiveType_OCSD)             ? "OCSD":       \
    (x==PrimitiveType_IntelPCH)         ? "I2C PCH":    \
    (x==PrimitiveType_IPMISatCtrl)      ? "IPMI SAT MSTR": \
    (x==PrimitiveType_PLDM)             ? "PLDM":       \
    (x==PrimitiveType_APMLData)         ? "APMLData":   \
    (x==PrimitiveType_BlockTransfer)    ? "BlkXfer":    \
    (x==PrimitiveType_Accelerator)      ? "Accelerator": "Unknown" )

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefRecordHeader
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    UINT8       Type;           /* Record type */

    UINT8       Size;           /* Record size (including this header) */

    UINT16      RecordID;       /* Unique record ID */

    UINT16      Flags;          /* Flag bits */

    UINT8       EntityID;       /* IPMI Entity ID */
    UINT8       EntityInstance; /* IPMI Entity Instance */

    UINT8       FeatureIndex;   /* Feature index */
#if 1 //GEN12_BOARD
    UINT8       _Reserved[15];   /* Reserved for expansion */
    UINT64      RuntimeData;    /* Available for iLO firmware */
#else
    UINT8       _Reserved[3];   /* Reserved for expansion */
    UINT32      RuntimeData;    /* Available for iLO firmware */
#endif
#define PLATDEF_REC_HDR_NAME_LEN    16
    char        Name[PLATDEF_REC_HDR_NAME_LEN];       /* Record name */

} PlatDefRecordHeader;

enum PlatDefRecordType
{
    RecordType_Undefined      = 0,
    RecordType_TableData      = 1,
    RecordType_TempSensor     = 2,
    RecordType_FanPWM         = 3,
    RecordType_FanDevice      = 4,
    RecordType_PowerSupply    = 5,
    RecordType_RedundancyRule = 6,
    RecordType_Indicator      = 7,
    RecordType_PowerMeter     = 8,
    RecordType_Processor      = 9,
    RecordType_Status         = 10,
    RecordType_SatelliteBMC   = 11,
    RecordType_FRU            = 12,
    RecordType_Association    = 13,
    RecordType_I2CEngine      = 14,
    RecordType_DIMMMapping    = 15,  //deprecated
    RecordType_AltConfig      = 16,
    RecordType_Validation     = 17,  //deprecated
    RecordType_SensorGroup    = 18,
    RecordType_LookupTable    = 19,
    RecordType_Throttle       = 20,
    RecordType_SystemDevice   = 21,
    RecordType_PeciSegment    = 22,
    RecordType_DeltaPatch     = 240,
    RecordType_Replacement    = 241,
    RecordType_EndOfTable     = 255
};

#define PLATDEF_RECORD_TYPE_NAME_LEN  16
/* MACRO to report human readable enum value */
                                     /*0123456789012345*/
#define PLATDEF_RECORD_TYPE_NAME(x)    ( \
    (x==RecordType_Undefined)       ? "Undefined":      \
    (x==RecordType_TableData)       ? "TableData":      \
    (x==RecordType_TempSensor)      ? "TempSensor":     \
    (x==RecordType_FanPWM)          ? "FanPWM":         \
    (x==RecordType_FanDevice)       ? "FanDevice":      \
    (x==RecordType_PowerSupply)     ? "PowerSupply":    \
    (x==RecordType_RedundancyRule)  ? "RedundancyRule": \
    (x==RecordType_Indicator)       ? "Indicator":      \
    (x==RecordType_PowerMeter)      ? "PowerMeter":     \
    (x==RecordType_Processor)       ? "Processor":      \
    (x==RecordType_Status)          ? "Status":         \
    (x==RecordType_SatelliteBMC)    ? "SatelliteBMC":   \
    (x==RecordType_FRU)             ? "FRU":            \
    (x==RecordType_Association)     ? "Association":    \
    (x==RecordType_I2CEngine)       ? "I2CEngine":      \
    (x==RecordType_DIMMMapping)     ? "_DIMMMapping":   \
    (x==RecordType_AltConfig)       ? "AltConfig":      \
    (x==RecordType_Validation)      ? "Validation":     \
    (x==RecordType_SensorGroup)     ? "SensorGroup":    \
    (x==RecordType_LookupTable)     ? "LookupTable":    \
    (x==RecordType_Throttle)        ? "Throttle":       \
    (x==RecordType_SystemDevice)    ? "SystemDevice":   \
    (x==RecordType_PeciSegment)     ? "PeciSegment":    \
    (x==RecordType_DeltaPatch)      ? "DeltaPatch":     \
    (x==RecordType_Replacement)     ? "Replacement":    \
    (x==RecordType_EndOfTable)      ? "END" : "Unknown" )

enum PlatDefHeaderFlag
{
    HeaderFlag_VersionMask    = 0x000F,
    HeaderFlag_Disabled       = 0x0010,
    HeaderFlag_HideFromIPMI   = 0x0020,
    HeaderFlag_HideFromUI     = 0x0040,
    HeaderFlag_AuxPower       = 0x0080,
    HeaderFlag_AfterPOST      = 0x0100,
    HeaderFlag_CommonFields   = 0x0200,
    HeaderFlag_ExposedToIPMI  = 0x2000,
    HeaderFlag_Patched        = 0x4000,
    HeaderFlag_Dynamic        = 0x8000
};

enum PlatDefRecordVersion
{
    RecordVersion_Undefined = 0,
    RecordVersion_V1        = 1,
    RecordVersion_V2        = 2,
    RecordVersion_V3        = 3,
    RecordVersion_V4        = 4,
    RecordVersion_V5        = 5,
    RecordVersion_V6        = 6,
    RecordVersion_V7        = 7,
    RecordVersion_V8        = 8,
    RecordVersion_V9        = 9,
    RecordVersion_V10       = 10,
    RecordVersion_V11       = 11,
    RecordVersion_V12       = 12,
    RecordVersion_V13       = 13,
    RecordVersion_V14       = 14,
    RecordVersion_V15       = 15
};

enum IPMIEntityID
{
    IPMIEntityID_Unspecified                  = 0,
    IPMIEntityID_Other                        = 1,
    IPMIEntityID_Unknown                      = 2,
    IPMIEntityID_Processor                    = 3,
    IPMIEntityID_Disk                         = 4,
    IPMIEntityID_PeripheralBay                = 5,
    IPMIEntityID_SystemManagementModule       = 6,
    IPMIEntityID_SystemBoard                  = 7,
    IPMIEntityID_MemoryModule                 = 8,
    IPMIEntityID_ProcessorModule              = 9,
    IPMIEntityID_PowerSupply                  = 10,
    IPMIEntityID_AddInCard                    = 11,
    IPMIEntityID_FrontPanel                   = 12,
    IPMIEntityID_BackPanel                    = 13,
    IPMIEntityID_PowerSupplyBoard             = 14,
    IPMIEntityID_DriveBackplane               = 15,
    IPMIEntityID_InternalExpansionBoard       = 16,
    IPMIEntityID_OtherSystemBoard             = 17,
    IPMIEntityID_ProcessorBoard               = 18,
    IPMIEntityID_PowerUnit                    = 19,
    IPMIEntityID_PowerModule                  = 20,
    IPMIEntityID_PowerDistributionBoard       = 21,
    IPMIEntityID_ChassisBackPanelBoard        = 22,
    IPMIEntityID_SystemChassis                = 23,
    IPMIEntityID_SubChassis                   = 24,
    IPMIEntityID_OtherChassisBoard            = 25,
    IPMIEntityID_DiskDriveBay                 = 26,
    IPMIEntityID_PeripheralBay2               = 27,
    IPMIEntityID_DeviceBay                    = 28,
    IPMIEntityID_Fan                          = 29,
    IPMIEntityID_CoolingUnit                  = 30,
    IPMIEntityID_Cable                        = 31,
    IPMIEntityID_MemoryDevice                 = 32,
    IPMIEntityID_SystemManagementSoftware     = 33,
    IPMIEntityID_BIOS                         = 34,
    IPMIEntityID_OS                           = 35,
    IPMIEntityID_SystemBus                    = 36,
    IPMIEntityID_Group                        = 37,
    IPMIEntityID_RemoteManagementCommDevice   = 38,
    IPMIEntityID_ExternalEnvironment          = 39,
    IPMIEntityID_Battery                      = 40,
    IPMIEntityID_ProcessingBlade              = 41,
    IPMIEntityID_ConnectivitySwitch           = 42,
    IPMIEntityID_ProcessorMemoryModule        = 43,
    IPMIEntityID_IOModule                     = 44,
    IPMIEntityID_ProcessorIOModule            = 45,
    IPMIEntityID_ManagementControllerFirmware = 46,
    IPMIEntityID_IPMIChannel                  = 47,
    IPMIEntityID_InletTemp                    = 64,   // DCMI
    IPMIEntityID_ProcessorTemp                = 65,   // DCMI
    IPMIEntityID_BaseboardTemp                = 66    // DCMI
};

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefCommonFields_V1
//
//  Several PlatDef records start with the same set of fields,
//  allowing common code to be used with all of them.
//
//  Used by:
//
//  PlatDefTempSensor_V1
//  PlatDefFanDevice_V1
//  PlatDefPowerSupply_V1
//  PlatDefPowerMeter_V1
//  PlatDefProcessor_V1
//  PlatDefStatus_V1
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    PlatDefRecordHeader Header;

    PlatDefPrimitive    Detect;
    PlatDefPrimitive    Monitor;
    PlatDefPrimitive    Interrogate;
    PlatDefPrimitive    Indicator;

    PlatDefPrimitive    Configure;
    PlatDefDevInitData  CfgRegisters[4];

    PlatDefReaction     DegradedReaction;
    PlatDefReaction     FailedReaction;

    UINT16              Flags;
    UINT16              Status;

    UINT8               _CommonReserved[12];

} PlatDefCommonFields_V1;

typedef PlatDefCommonFields_V1 PlatDefCommonFields;

/* Enums for the different usages of PlatDefPrimitive structure
** - enum order modeled on health.c: char *health_desc_name[] content AND 
**    // Descriptors definition (in order in the extension structure)
**    #define HEALTH_DESC_GENERAL         (0)
**    #define HEALTH_DESC_MONITOR         (1)
**    #define HEALTH_DESC_DETECT          (2)
**    #define HEALTH_DESC_INTERROGATE     (3) 
** - rational for enum creation: 
**   QXCR1001812321: False iLO OS Shutdown or Loss of Temp Sensors 
*/ 
typedef enum {
    PLATDEF_PRIMITIVE_USE_GENERAL       = 0,
    PLATDEF_PRIMITIVE_USE_MONITOR       = 1,
    PLATDEF_PRIMITIVE_USE_DETECT        = 2,
    PLATDEF_PRIMITIVE_USE_INTERROGATE   = 3,
    PLATDEF_PRIMITIVE_USE_INDICATE      = 4,
    PLATDEF_PRIMITIVE_USE_CONFIGURE     = 5,
    PLATDEF_PRIMITIVE_USE_END           // always keep this last 
} PlatDefPrimitiveUsage;

/* MACRO for enum to string */
#define PRIMITIVE_USAGE_NAME_LEN  8
#define PLATDEF_PRIMITIVE_USE_NAME(x)   (                 \
    (x==PLATDEF_PRIMITIVE_USE_GENERAL)      ? "General ": \
    (x==PLATDEF_PRIMITIVE_USE_MONITOR)      ? "Monitor ": \
    (x==PLATDEF_PRIMITIVE_USE_DETECT)       ? "Detect  ": \
    (x==PLATDEF_PRIMITIVE_USE_INTERROGATE)  ? "Interro ": \
    (x==PLATDEF_PRIMITIVE_USE_INDICATE)     ? "Indicate": \
    (x==PLATDEF_PRIMITIVE_USE_CONFIGURE)    ? "Config  ": \
    (x==PLATDEF_PRIMITIVE_USE_END)          ? "END" : "Unknown " )

enum PlatDefCommonFlags
{
    CommonFlag_SupportsRedundancy = 0x0001,
    CommonFlag_SupportsHotPlug    = 0x0002
};

enum PlatDefCommonStatus
{
    CommonStatus_Present  = 0x0001,
    CommonStatus_Failed   = 0x0002,
    CommonStatus_Degraded = 0x0004,
    CommonStatus_Error    = 0x0008
};

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefTableData_V1
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    PlatDefRecordHeader Header;             /* Standard header */

    char                Description[32];    /* Table description */

    UINT16              Flags;              /* Flag bits */

    UINT8               MajorVersion;       /* Table major version number */
    UINT8               MinorVersion;       /* Table minor version number */

    UINT32              BuildTimestamp;     /* Table build time */

    UINT32              RecordCount;        /* Total number of records */
    UINT32              TotalSize;          /* Size of entire PlatDef */

    UINT8               MD5Hash[16];        /* MD5 hash of the entire table */

    UINT32              CompressedSize;     /* Compressed size of PlatDef */

    UINT8               SpecialVersion;     /* Table special version number */
    UINT8               BuildVersion;       /* Table build version number */

    UINT8               _Reserved[10];      /* Reserved for future expansion */

} PlatDefTableData_V1;

typedef PlatDefTableData_V1 PlatDefTableData;

enum PlatDefTableFlag
{
    TableFlag_PatchesApplied   = 0x0001,
    TableFlag_LegacyTable      = 0x0002,
    TableFlag_TemporaryOEM     = 0x0004,
    TableFlag_PlatdefValidated = 0x0008,
    TableFlag_CompressionMask  = 0x0070,
      TableFlag_NotCompressed    = 0x0000,
      TableFlag_ZLib             = 0x0010
};

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefTempSensor_V1
//
//  Also includes:
//
//  PlatDefFanAlgorithm
//  FanAlgorithm_MultiSegment
//  FanAlgorithm_PID
//
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//  FanAlgorithm_MultiSegment
//----------------------------------------------------------------------------

#define PLATDEF_ALGORITHM_NUM_SEGMENTS 6

typedef struct {

    struct {

        UINT8   Type;

        UINT8   Flags;

        FP100   LowTemp;
        FP100   HighTemp;

        UINT8   LowPWM;
        UINT8   HighPWM;

    } Segment[PLATDEF_ALGORITHM_NUM_SEGMENTS];

} FanAlgorithm_MultiSegment;

enum FanAlgSegmentType
{
    FanAlgSegmentType_Disabled    = 0x00,
    FanAlgSegmentType_Constant    = 0x01,
    FanAlgSegmentType_Linear      = 0x02,
    FanAlgSegmentType_Exponential = 0x03
};

enum FanAlgMultiSegmentFlag
{
    FanAlgSegmentFlag_UseFanPWM = 0x01,
    FanAlgSegmentFlag_UseMinPWM = 0x02,
    FanAlgSegmentFlag_UseMaxPWM = 0x04
};

//----------------------------------------------------------------------------
//  FanAlgorithm_PID
//----------------------------------------------------------------------------

typedef struct {

    FP100   Setpoint;

    FP100   LowLimit;
    FP100   HighLimit;

    FP100   PGain;
    FP100   IGain;
    FP100   DGain;

    FP100   IntegralMin;
    FP100   IntegralMax;

    FP100   M;             // For setpoint adjustment
    FP100   B;             // SP = Mx + B

    UINT8   Flags;

} FanAlgorithm_PID;

enum FanAlgPIDFlag
{
    FanAlgPIDFlag_ModifySetPoint = 0x01,
    FanAlgPIDFlag_Delta          = 0x02
};

//----------------------------------------------------------------------------
//  PlatDefFanAlgorithm
//----------------------------------------------------------------------------

typedef struct {

    UINT8               Type;

    UINT8               Output;

    UINT16              Flags;

    INT16               Delta;

    UINT8               _Reserved[2];

    PlatDefPrimitive    Modifier;

    union
    {
        FanAlgorithm_MultiSegment MultiSegment;
        FanAlgorithm_PID          PID;
        UINT8                     _RawBytes[48];
    };

} PlatDefFanAlgorithm;

enum FanAlgType
{
    FanAlgType_None         = 0,
    FanAlgType_MultiSegment = 1,
    FanAlgType_PID          = 3
};

enum FanAlgFlag
{
    FanAlgFlag_NoModifier       = 0x0000,
    FanAlgFlag_AddModifier      = 0x0001,
    FanAlgFlag_SubtractModifier = 0x0002,
    FanAlgFlag_ModifierMask     = 0x0003
};

//----------------------------------------------------------------------------
//  PlatDefTempSensor_V1
//----------------------------------------------------------------------------

typedef struct {

    // BEGIN PlatDefCommonFields_V1

    PlatDefRecordHeader Header;

    PlatDefPrimitive    Detect;
    PlatDefPrimitive    Monitor;
    PlatDefPrimitive    Interrogate;
    PlatDefPrimitive    Indicator;

    PlatDefPrimitive    Configure;
    PlatDefDevInitData  CfgRegisters[4];

    PlatDefReaction     DegradedReaction;
    PlatDefReaction     FailedReaction;

    UINT16              Flags;
    UINT16              Status;

    UINT8               _CommonReserved[12];

    // END PlatDefCommonFields_V1

    FP100               Raw;
    FP100               Adjusted;

    FP100               CriticalThreshold;
    FP100               CautionThreshold;
    FP100               StartupThreshold;
    FP100               ThrottleThreshold;

    FP100               Offset;
    FP100               MinDisplayTemp;
    UINT8               Hysteresis;
    UINT8               MaxChange;
    UINT8               LocationXY;
    UINT8               CfgHysteresisRegister;
    UINT8               CfgCriticalRegister;
    UINT8               OCSDID;    //deprecated
    UINT8               Units;
    UINT8               _Reserved1;
    UINT16              FallbackSensorID;
    IPMISensorOwner     SensorOwner;
    UINT8               SDRByte45;
	UINT8               CustomType;
    UINT8               FilterDeltaTemp;
    UINT8               MaxFilterTimes;
    UINT16              ThrottleMask;

    UINT8               _Reserved2[6];

    PlatDefFanAlgorithm Algorithm;

} PlatDefTempSensor_V1;

typedef PlatDefTempSensor_V1 PlatDefTempSensor;

enum PlatDefTempFlag
{
    TempFlag_SupportsRedundancy      = CommonFlag_SupportsRedundancy,
    TempFlag_SupportsHotPlug         = CommonFlag_SupportsHotPlug,
    TempFlag_CriticalPowerOff        = 0x0010,
    TempFlag_AmbientIntake           = 0x0020,
    TempFlag_ThrottleAnchor          = 0x0040,
    TempFlag_IndependentThrottle     = 0x0080,
    TempFlag_TreatErrorAsNotPresent  = 0x0100,
    TempFlag_FailSafeBlowout         = 0x0200,
    TempFlag_POSTOnlyFanDriver       = 0x0400,
    TempFlag_AuxPowerOnly            = 0x0800,
    TempFlag_SensorNameReplaceEnable = 0x1000
};

enum PlatDefTempStatus
{
    TempStatus_Present         = CommonStatus_Present,
    TempStatus_Failed          = CommonStatus_Failed,
    TempStatus_Error           = CommonStatus_Error,
    TempStatus_Critical        = 0x0010,
    TempStatus_Caution         = 0x0020,
    TempStatus_Startup         = 0x0040,
    TempStatus_Throttle        = 0x0080,
    TempStatus_FailSafeBlowout = 0x0100
};

enum PlatDefTempUnits
{
    TempUnits_Celsius  = 1,
    TempUnits_IntelDTS = 2
};

enum PlatDefCustomType
{
	CustomType_NoCustomize = 0
};



//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefFanPWM_V1
//
//  Also includes:
//
//  PlatDefPWMInput
//
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//  PlatDefPWMInput
//----------------------------------------------------------------------------

typedef struct {

    UINT16              TempSensorID;
    UINT8               Weight;
    UINT8               Flags;

} PlatDefPWMInput;

//----------------------------------------------------------------------------
//  PlatDefFanPWM_V1
//----------------------------------------------------------------------------

typedef struct {

    PlatDefRecordHeader Header;

    PlatDefPrimitive    Enable;
    PlatDefPrimitive    SetPWM;

    PlatDefPrimitive    Configure;
    PlatDefDevInitData  CfgRegisters[4];

    UINT16              Flags;
    UINT16              Status;

    UINT8               CurrentPWM;
    UINT8               ResultPWM;
    UINT16              DrivingSensorID;
    UINT8               MinimumPWM;
    UINT8               MaximumPWM;
    UINT8               BlowoutPWM;
    UINT8               LockedPWM;
    UINT8               PowerOffPWM;
    UINT8               DecreasePercent;
    UINT8               Hysteresis;
    UINT8               MaxChange;
    UINT8               BlowoutPercent;
    UINT8               SystemOffPWM;
    UINT8               Multiplier;
    UINT8               SpecialHandling;

    UINT8               SegmentHighPWM[4];

    UINT8               InputCount;

    UINT8               _Reserved[7];

    FP100               M;
    FP100               B;

    PlatDefPWMInput     Inputs[1];

} PlatDefFanPWM_V1;

typedef PlatDefFanPWM_V1 PlatDefFanPWM;

enum PlatDefPWMFlag
{
    PWMFlag_GroupingMask       = 0x000F,
      PWMFlag_Single             = 0x0001,
      PWMFlag_Fastest            = 0x0002,
    PWMFlag_SetPercentage      = 0x0010,
    PWMFlag_Blowout            = 0x0020,
    PWMFlag_IgnoreMaxPWM       = 0x0040,
    PWMFlag_UseLockedPWM       = 0x0080,
    PWMFlag_ForceZero          = 0x0100,
    PWMFlag_UseMultiplier      = 0x0200,
    PWMFlag_Global             = 0x0400,
    PWMFlag_SetPWMAtPowerOff   = 0x0800,
    PWMFlag_UseGlobalIncCoolMB = 0x1000,
    PWMFlag_AuxPowerOnly       = 0x2000
};

enum PlatDefPWMStatus
{
    PWMStatus_Present = CommonStatus_Present,
    PWMStatus_Error   = CommonStatus_Error
};

enum PlatDefPWMSpecial
{
    PWMSpecial_Default            = 0,
    PWMSpecial_DecreasePercent    = 1,
    PWMSpecial_SpecialHandlingOff = 255
};

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefFanDevice_V1
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    // BEGIN PlatDefCommonFields_V1

    PlatDefRecordHeader Header;

    PlatDefPrimitive    Detect;
    PlatDefPrimitive    Monitor;
    PlatDefPrimitive    Interrogate;
    PlatDefPrimitive    Indicator;

    PlatDefPrimitive    Configure;
    PlatDefDevInitData  CfgRegisters[4];

    PlatDefReaction     DegradedReaction;
    PlatDefReaction     FailedReaction;

    UINT16              Flags;
    UINT16              Status;

    UINT8               _CommonReserved[12];

    // END PlatDefCommonFields_V1

    UINT16              Raw;
    UINT16              Adjusted;

    UINT16              PWMID;
    UINT16              FaultThreshold;
    UINT8               Hysteresis;
    UINT8               MaxChange;
    UINT8               LocationXY;
    UINT8               _Reserved1;
    IPMISensorOwner     SensorOwner;

    UINT8               _Reserved2[18];

} PlatDefFanDevice_V1;

typedef PlatDefFanDevice_V1 PlatDefFanDevice;

enum PlatDefFanDevFlag
{
    FanDevFlag_SupportsRedundancy = CommonFlag_SupportsRedundancy,
    FanDevFlag_SupportsHotPlug    = CommonFlag_SupportsHotPlug,
    FanDevFlag_Tachometer         = 0x0010,
    FanDevFlag_Counter            = 0x0020,
    FanDevFlag_FFFFisError        = 0x0040,
    FanDevFlag_FanBlock           = 0x0080,
    FanDevFlag_Virtual            = 0x0100,
    FanDevFlag_ThrottleAnchor     = 0x0200,
    FanDevFlag_MakeFirstSDR       = 0x0400,
    FanDevFlag_Optional           = 0x0800,
	FanDevFlag_LiquidCoolingPump  = 0x1000
};

enum PlatDefFanDevStatus
{
    FanDevStatus_Present  = CommonStatus_Present,
    FanDevStatus_Failed   = CommonStatus_Failed,
    FanDevStatus_Error    = CommonStatus_Error,
    FanDevStatus_Degraded = CommonStatus_Degraded
};

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefPowerSupply_V1
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    // BEGIN PlatDefCommonFields_V1

    PlatDefRecordHeader Header;

    PlatDefPrimitive    Detect;
    PlatDefPrimitive    Monitor;
    PlatDefPrimitive    Interrogate;
    PlatDefPrimitive    Indicator;

    PlatDefPrimitive    Configure;
    PlatDefDevInitData  CfgRegisters[4];

    PlatDefReaction     DegradedReaction;
    PlatDefReaction     FailedReaction;

    UINT16              Flags;
    UINT16              Status;

    UINT8               _CommonReserved[12];

    // END PlatDefCommonFields_V1

    UINT8               Type;
    UINT8               LocationXY;
    UINT8               ShutdownRegister;
    UINT8               WarningRegister;
    UINT16              CurrentReading;
    IPMISensorOwner     SensorOwner;
    UINT8               RawToWatts;
    UINT8               WattsToIPMI;
    UINT8               DomainType;

    UINT8               _Reserved[5];

} PlatDefPowerSupply_V1;

typedef PlatDefPowerSupply_V1 PlatDefPowerSupply;

enum PlatDefPowerSupplyFlag
{
    PowerSupplyFlag_SupportsRedundancy = CommonFlag_SupportsRedundancy,
    PowerSupplyFlag_SupportsHotPlug    = CommonFlag_SupportsHotPlug,
    PowerSupplyFlag_PowerMeter         = 0x0010
};

enum PlatDefPowerSupplyStatus
{
    PowerSupplyStatus_Present  = CommonStatus_Present,
    PowerSupplyStatus_Failed   = CommonStatus_Failed,
    PowerSupplyStatus_Error    = CommonStatus_Error,
    PowerSupplyStatus_Degraded = CommonStatus_Degraded
};

enum PlatDefPowerSupplyType
{
    PSType_Unknown     = 0,
    PSType_Trees       = 1
};

enum PlatDefPowerSupplyDomainType
{
    PSDomainType_System = 0,
    PSDomainType_GPU    = 1,
    PSDomainType_GPU_2    = 2,
    PSDomainType_System_1 = 3,
    PSDomainType_System_2 = 4
    //Update ps_domain_names array in pwrmgr.h if a new domain type is added
};

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefRedundancyRule_V1
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    UINT8               RequiredForRedundancy;
    UINT8               RequiredForOperation;
    UINT8               CurrentCount;
    UINT8               PriorCount;

} PlatDefRedundancyAlg_Count;

typedef struct {

    UINT8               Type;
    UINT8               Flags;
    UINT8               _Reserved[2];

    union
    {
        PlatDefRedundancyAlg_Count  Count;
        UINT8                       _RawBytes[28];
    };

} PlatDefRedundancyAlgorithm;

enum RedundancyAlgType
{
    RedundancyAlgType_None  = 0,
    RedundancyAlgType_Count = 1
};

typedef struct {

    PlatDefRecordHeader         Header;

    PlatDefPrimitive            Indicator;

    UINT16                      Flags;
    UINT16                      Status;

    UINT8                       MemberCount;
    UINT8                       GroupType;

    IPMISensorOwner             SensorOwner;

    PlatDefReaction             DegradedReaction;
    PlatDefReaction             FailedReaction;

    UINT8                       _Reserved[16];

    PlatDefRedundancyAlgorithm  Algorithm;

    UINT16                      Members[1];

} PlatDefRedundancyRule_V1;

typedef PlatDefRedundancyRule_V1 PlatDefRedundancyRule;

enum PlatDefRedundancyFlag
{
    RedundancyFlag_RedundancyNotSupported = 0x0001
};

enum PlatDefRedundancyStatus
{
    RedundancyStatus_Failed   = CommonStatus_Failed,
    RedundancyStatus_Degraded = CommonStatus_Degraded
};

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefIndicator_V1
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    PlatDefRecordHeader Header;

    PlatDefPrimitive    Bit1;
    PlatDefPrimitive    Bit2;

    PlatDefPrimitive    Interrogate;

    UINT8               Type;
    UINT8               _UnusedPadding;

    UINT16              Flags;
    UINT16              Status;

    IPMISensorOwner     SensorOwner;

    UINT8               _Reserved[8];

} PlatDefIndicator_V1;

typedef PlatDefIndicator_V1 PlatDefIndicator;

enum PlatDefIndicatorType
{
    IndicatorType_Unknown = 0,
    IndicatorType_UID     = 1,
    IndicatorType_Health  = 2
};

enum PlatDefIndicatorStatus
{
    IndicatorStatus_Error = CommonStatus_Error,
    IndicatorStatus_On    = 0x0010
};

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefPowerMeter_V1
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    // BEGIN PlatDefCommonFields_V1

    PlatDefRecordHeader Header;

    PlatDefPrimitive    Detect;
    PlatDefPrimitive    Monitor;
    PlatDefPrimitive    Interrogate;
    PlatDefPrimitive    Indicator;

    PlatDefPrimitive    Configure;
    PlatDefDevInitData  CfgRegisters[4];

    PlatDefReaction     DegradedReaction;
    PlatDefReaction     FailedReaction;

    UINT16              Flags;
    UINT16              Status;

    UINT8               _CommonReserved[12];

    // END PlatDefCommonFields_V1

    UINT16              Raw;
    UINT16              Watts;

    UINT8               MeterType;
    UINT8               Scope;
    UINT8               RawToWatts;
    UINT8               WattsToIPMI;

    IPMISensorOwner     SensorOwner;

    UINT8               _Reserved[6];

} PlatDefPowerMeter_V1;

typedef PlatDefPowerMeter_V1 PlatDefPowerMeter;

enum PlatDefPowerMeterStatus
{
    PowerMeterStatus_Present = CommonStatus_Present,
    PowerMeterStatus_Failed  = CommonStatus_Failed,
    PowerMeterStatus_Error   = CommonStatus_Error
};

enum PlatDefPowerMeterType
{
    PowerMeterType_Unknown           = 0,
    PowerMeterType_Virtual           = 1,
    PowerMeterType_PowerPIC          = 2,
    PowerMeterType_ChassisController = 3
};

enum PlatDefPowerMeterScope
{
    PowerMeterScope_Unknown   = 0,
    PowerMeterScope_Enclosure = 1,
    PowerMeterScope_Server    = 2,
    PowerMeterScope_PCISlot   = 3,
    PowerMeterScope_SubSystem = 4
};

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefProcessor_V1
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    // BEGIN PlatDefCommonFields_V1

    PlatDefRecordHeader Header;

    PlatDefPrimitive    Detect;
    PlatDefPrimitive    Monitor;
    PlatDefPrimitive    Interrogate;
    PlatDefPrimitive    Indicator;

    PlatDefPrimitive    Configure;
    PlatDefDevInitData  CfgRegisters[4];

    PlatDefReaction     DegradedReaction;
    PlatDefReaction     FailedReaction;

    UINT16              Flags;
    UINT16              Status;

    UINT8               _CommonReserved[12];

    // END PlatDefCommonFields_V1

    UINT16              Type4Record;

    IPMISensorOwner     SensorOwner;

    UINT8               CPUNumber;

    UINT8               PiROMSegment;

    UINT8               PiROMAddress;

    UINT8               SBRMISegment;
    UINT8               SBRMIAddress;
    UINT8               SBRMIInstance;

    UINT8               _Reserved[6];
} PlatDefProcessor_V1;

typedef PlatDefProcessor_V1 PlatDefProcessor;

enum PlatDefProcessorStatus
{
    ProcessorStatus_Present  = CommonStatus_Present,
    ProcessorStatus_Failed   = CommonStatus_Failed
};

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefStatus_V1
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    // BEGIN PlatDefCommonFields_V1

    PlatDefRecordHeader Header;

    PlatDefPrimitive    Detect;
    PlatDefPrimitive    Monitor;
    PlatDefPrimitive    Interrogate;
    PlatDefPrimitive    Indicator;

    PlatDefPrimitive    Configure;
    PlatDefDevInitData  CfgRegisters[4];

    PlatDefReaction     DegradedReaction;
    PlatDefReaction     FailedReaction;

    UINT16              Flags;
    UINT16              Status;

    UINT8               _CommonReserved[12];

    // END PlatDefCommonFields_V1

    UINT8               ItemType;

    UINT8               _Reserved1;

    UINT16              CurrentReading;

    UINT8               _Reserved2[12];

    union
    {
#ifdef INCL_APML_IPMIRECORD_H
        SensorDataRecord    SDR;
#endif
        UINT8               SDRBytes[64];
    };

} PlatDefStatus_V1;

typedef PlatDefStatus_V1 PlatDefStatus;

enum PlatDefItemTypes
{
    ItemType_Generic           = 0,
    ItemType_ProcHot           = 1,
    ItemType_Memory            = 2,
    ItemType_Temperature       = 3,
    ItemType_Fan               = 4,
    ItemType_PowerSupply       = 5,
    ItemType_EnclosureType     = 6,
    ItemType_EnclosureHealth   = 7,
    ItemType_NICLink           = 8,
    ItemType_NICSpeed          = 9,
    ItemType_StorageController = 10,
    ItemType_DriveBackplane    = 11,
    ItemType_PhysicalDisk      = 12,
    ItemType_LogicalDisk       = 13,
    ItemType_Battery           = 14,
    ItemType_SuperCap          = 15,
    ItemType_IPMI              = 16,
    ItemType_FastCapWrite      = 17,
    ItemType_HoodSensor        = 18,
    ItemType_CPUUtilization    = 19
};

enum PlatDefItemFlag
{
    ItemFlag_FFisError              = 0x0010,
    ItemFlag_FFFFisError            = 0x0020,
    ItemFlag_TreatErrorAsNotPresent = 0x0040,
    ItemFlag_TreatErrorAsFailed     = 0x0080
};

enum PlatDefItemStatus
{
    ItemStatus_Present  = CommonStatus_Present,
    ItemStatus_Failed   = CommonStatus_Failed,
    ItemStatus_Degraded = CommonStatus_Degraded,
    ItemStatus_Error    = CommonStatus_Error
};

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefSatelliteBMC_V1
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    PlatDefRecordHeader Header;

    UINT16              Flags;
    UINT16              Status;

    UINT8               _Reserved[12];

    union
    {
#ifdef INCL_APML_IPMIRECORD_H
        ManagementControllerDeviceLocatorRecord Locator;
#endif
        UINT8                                   LocatorBytes[32];
    };

} PlatDefSatelliteBMC_V1;

typedef PlatDefSatelliteBMC_V1 PlatDefSatelliteBMC;

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefFRU_V1
//
//  Also includes:
//
//  PlatDefFRUDevice
//
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//  PlatDefFRUDevice
//----------------------------------------------------------------------------

typedef struct {

    UINT8       Type;
    UINT8       Format;
    UINT8       I2CSegment;
    UINT8       I2CAddress;
    UINT32      FRUOffset;
    UINT32      FRUSize;

    UINT32      DeviceSize;
    UINT16      PageSize;
    UINT8       NumAddressBytes;
    UINT8       SlaveAddressMask;
    UINT8       MaxReadSize;

    UINT8       _Reserved[3];

} PlatDefFRUDevice;

enum PlatDefFRUDeviceType
{
    FRUDeviceType_Unspecified              = 0x00,
    FRUDeviceType_Virtual                  = 0x10,
      FRUDeviceType_Virtual_DLSysFRU       = 0x11,
    FRUDeviceType_I2CEEPROM                = 0x20,
      FRUDeviceType_I2C_24C01              = 0x21,
      FRUDeviceType_I2C_24C02              = 0x22,
      FRUDeviceType_I2C_24C04              = 0x23,
      FRUDeviceType_I2C_24C08              = 0x24,
      FRUDeviceType_I2C_24C16              = 0x25,
      FRUDeviceType_I2C_24C17              = 0x26,
      FRUDeviceType_I2C_24C32              = 0x27,
      FRUDeviceType_I2C_24C64              = 0x28,
      FRUDeviceType_I2C_24C128             = 0x29,
      FRUDeviceType_I2C_24C256             = 0x2A,
      FRUDeviceType_I2C_24C512             = 0x2B,
      FRUDeviceType_I2C_Tbird_Mezz_Generic = 0x2C,
      FRUDeviceType_I2C_24CM01             = 0x2D,
      FRUDeviceType_I2C_24CM02             = 0x2E,
      FRUDeviceType_I2C_CClass_Generic     = 0x2F,
    FRUDeviceType_Mask                     = 0xF0
};

enum PlatDefFRUDeviceFormat
{
    FRUDeviceFormat_Unspecified = 0,
    FRUDeviceFormat_IPMI        = 1,
    FRUDeviceFormat_HPQ         = 2,
    FRUDeviceFormat_SPD         = 3,
    FRUDeviceFormat_PCA         = 4
};

//----------------------------------------------------------------------------
//  PlatDefFRU_V1
//----------------------------------------------------------------------------

typedef struct {

    PlatDefRecordHeader Header;

    UINT16              Flags;
    UINT16              Status;

    PlatDefFRUDevice    Device;

    UINT8               FRUClass;
    UINT8               AssociatedRecordCount;

    UINT8               _Reserved[10];

    union
    {
#ifdef INCL_APML_IPMIRECORD_H
        FRUDeviceLocatorRecord  Locator;
#endif
        UINT8                   LocatorBytes[32];
    };

    UINT16              AssociatedRecords[1];

} PlatDefFRU_V1;

typedef PlatDefFRU_V1 PlatDefFRU;

enum PlatDefFRUFlag
{
    FRUFlag_WriteProtected = 0x0010
};

enum PlatDefFRUClass
{
    FRUClass_Unspecified = 0x00,
    FRUClass_Base        = 0x01,
    FRUClass_Mezz        = 0x02,
    FRUClass_CANmic      = 0x03,
    FRUClass_Riser       = 0x04
};

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefAssociation_V1
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    PlatDefRecordHeader Header;

    UINT16              Flags;
    UINT16              Status;

    UINT8               _Reserved[12];

    union
    {
#ifdef INCL_APML_IPMIRECORD_H
        EntityAssociationRecord EAR;
#endif
        UINT8                   EARBytes[16];
    };

} PlatDefAssociation_V1;

typedef PlatDefAssociation_V1 PlatDefAssociation;

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefI2CEngine_V1
//
//  Also includes:
//
//  PlatDefI2CSegment
//  PlatDefI2CMux
//
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//  PlatDefI2CMux
//----------------------------------------------------------------------------

typedef union {

    UINT8   MuxType;

    UINT8   RawBytes[16];

    struct {

        UINT8   MuxType;
        UINT16   Byte;
        UINT8   InitMask;
        UINT8   SelectMask;

    } CPLD;

    struct {

        UINT8   MuxType;
        UINT8   Byte;
        UINT8   InitMask;
        UINT8   SelectMask;

    } GPO;

    struct {

        UINT8   MuxType;
        UINT8   Address;
        UINT8   Offset;
        UINT8   InitMask;
        UINT8   SelectMask;
        UINT8   AlertMask;

    } InlineI2C;

    struct {

        UINT8   MuxType;
        UINT8   Address;

    } GPIOExpander;

    struct {

        UINT8   MuxType;

    } Virtual;

} PlatDefI2CMux;

enum I2CMuxType
{
    I2CMuxType_None         = 0,
    I2CMuxType_CPLD         = 1,
    I2CMuxType_GPO          = 2,
    I2CMuxType_InlineI2C    = 3,
    I2CMuxType_GPIOExpander = 4,
    I2CMuxType_Virtual      = 5,
    I2CMuxType_ArbCPLD      = 6
};

//----------------------------------------------------------------------------
//  PlatDefI2CSegment
//----------------------------------------------------------------------------

typedef struct {

    UINT32              Flags;
    UINT8               Speed;

    UINT8               ID;
    UINT8               ParentID;

    UINT8               GPURiserNumber;

    UINT8               _Reserved[8];

    PlatDefI2CMux       MuxControl;

} PlatDefI2CSegment;

//----------------------------------------------------------------------------
//  PlatDefI2CEngine_V1
//----------------------------------------------------------------------------

typedef struct {

    PlatDefRecordHeader Header;

    PlatDefPrimitive    Reset;
    PlatDefPrimitive    Alert;

    UINT32              Flags;
    UINT8               Speed;

    UINT8               ID;
    UINT8               Count;

    UINT8               _Reserved[9];

    PlatDefI2CSegment   Segments[1];

} PlatDefI2CEngine_V1;

enum PlatDefI2CFlag
{
    I2CFlag_HasChassisController  = 0x00000001,
    I2CFlag_Dynamic               = 0x00000002,
    I2CFlag_AvailableOnAuxPower   = 0x00000004,
    I2CFlag_HasDedicatedGPUSlot   = 0x00000008,
    I2CFlag_IgnoreSegment         = 0x00000010,
    I2CFlag_HasUBMDriveBay        = 0x00000020,
    I2CFlag_UBMSingleModeNVMeOK   = 0x00000040,
    I2CFlag_UBMSingleModeSATAOK   = 0x00000080,
    I2CFlag_UBMMultiModeNVMeOK    = 0x00000100,
    I2CFlag_UBMMultiModeSATAOK    = 0x00000200
};

enum PlatDefI2CSpeed
{
    I2CSpeed_Unspecified = 0x00,
    I2CSpeed_Max100KHz   = 0x01,
    I2CSpeed_Max400KHz   = 0x02,
    I2CSpeed_Max1MHz     = 0x03
};

typedef PlatDefI2CEngine_V1 PlatDefI2CEngine;

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefDIMMMapping_V1(deprecated)
//
//  Also includes:
//
//  PlatDefDIMMInfo
//
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//  PlatDefDIMMInfo(deprecated)
//----------------------------------------------------------------------------

typedef struct {

    UINT8               BoardDIMM;
    UINT8               BoardChannel;
    UINT8               BoardLettering;
    UINT8               LogicalCPU;
    UINT16              IESensorID;
    UINT8               IEDIMMID;
    UINT8               GroupID;
    UINT8               PECISegment;
    UINT8               Address;

    UINT8               _Reserved[6];

} PlatDefDIMMInfo;

//----------------------------------------------------------------------------
//  PlatDefDIMMMapping_V1(deprecated)
//----------------------------------------------------------------------------

typedef struct {

    PlatDefRecordHeader Header;

    UINT16              Flags;
    UINT8               Count;

    UINT8               _Reserved[13];

    PlatDefDIMMInfo     DIMM[1];

} PlatDefDIMMMapping_V1;

enum PlatDefDIMMMappingFlag
{
    DIMMMappingFlag_HasIntelIE  = 0x0001,
    DIMMMappingFlag_UseGrouping = 0x0002
};

typedef PlatDefDIMMMapping_V1 PlatDefDIMMMapping;

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefAltConfig_V1
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    PlatDefRecordHeader Header;

    UINT32              Value;

    UINT8               Flags;
    UINT8               Status;

    UINT16              BaseConfigID;

    UINT16              Length;

    UINT8               _Reserved[6];

    UINT8               Expression[1];

} PlatDefAltConfig_V1;

typedef PlatDefAltConfig_V1 PlatDefAltConfig;

enum PlatDefAltConfigFlagBits
{
    AltCfgFlag_Primary    = 0x01,
    AltCfgFlag_Base       = 0x02,
    AltCfgFlag_Validation = 0x04,
    AltCfgFlag_First      = 0x08,
    AltCfgFlag_Partial    = 0x10
};

enum PlatDefAltConfigStatusBits
{
    AltCfgStatus_Evaluating = 0x01,
    AltCfgStatus_Evaluated  = 0x02,
    AltCfgStatus_Selected   = 0x04
};

enum PlatDefConfigExprOpCodes
{
    OpCode_Const0           = 0,
    OpCode_Const1           = 1,
    OpCode_Const2           = 2,
    OpCode_Const3           = 3,
    OpCode_Const4           = 4,
    OpCode_Const5           = 5,
    OpCode_Const6           = 6,
    OpCode_Const7           = 7,
    OpCode_Const8           = 8,
    OpCode_Const9           = 9,
    OpCode_Const10          = 10,
    OpCode_Const11          = 11,
    OpCode_Const12          = 12,
    OpCode_Const13          = 13,
    OpCode_Const14          = 14,
    OpCode_Const15          = 15,
    OpCode_Const16          = 16,
    OpCode_Const17          = 17,
    OpCode_Const18          = 18,
    OpCode_Const19          = 19,
    OpCode_Const20          = 20,
    OpCode_Const21          = 21,
    OpCode_Const22          = 22,
    OpCode_Const23          = 23,
    OpCode_Const24          = 24,
    OpCode_Const25          = 25,
    OpCode_Const26          = 26,
    OpCode_Const27          = 27,
    OpCode_Const28          = 28,
    OpCode_Const29          = 29,
    OpCode_Const30          = 30,
    OpCode_Const31          = 31,

    OpCode_ConstUINT8       = 0x20,     // Followed by UINT8
    OpCode_ConstUINT16      = 0x21,     // Followed by UINT16
    OpCode_ConstUINT32      = 0x22,     // Followed by UINT32

    OpCode_ConstString      = 0x23,     // Followed by UINT8 length and NUL terminated string

    OpCode_Add              = 0x24,
    OpCode_Subtract         = 0x25,
    OpCode_Multiply         = 0x26,
    OpCode_Divide           = 0x27,
    OpCode_Mod              = 0x28,
    OpCode_Negate           = 0x29,

    OpCode_BitAND           = 0x2A,
    OpCode_BitOR            = 0x2B,
    OpCode_BitXOR           = 0x2C,
    OpCode_BitNEG           = 0x2D,

    OpCode_ShiftLeft        = 0x2E,
    OpCode_ShiftRight       = 0x2F,

    OpCode_LogicalAND       = 0x30,
    OpCode_LogicalOR        = 0x31,
    OpCode_LogicalNOT       = 0x32,

    OpCode_LT               = 0x33,
    OpCode_LE               = 0x34,
    OpCode_GT               = 0x35,
    OpCode_GE               = 0x36,
    OpCode_EQ               = 0x37,
    OpCode_NE               = 0x38,

    OpCode_Config           = 0x39,     // Followed by UINT16 PlatDef record ID
    OpCode_FunctionCall     = 0x3A,     // Follwed by UINT8 function ID and UINT8 parameter count

    OpCode_End              = 0xFE,
    OpCode_NOP              = 0xFF
};

enum PlatDefConfigExprFunctions
{
    Function_NOP            = 0x00,
    Function_XReg           = 0x01,
    Function_MEMID          = 0x02,
    Function_GPI            = 0x03,
    Function_GPO            = 0x04,
    Function_EV             = 0x05,
    Function_SMBIOS         = 0x06,
    Function_HostCall       = 0x07,
    Function_CPUCount       = 0x08,
    Function_CPUType        = 0x09,
    Function_PSCount        = 0x0A,
    Function_PSType         = 0x0B,
    Function_FanCount       = 0x0C,
    Function_FanType        = 0x0D,
    Function_Lookup         = 0x0E,
    Function_HostState      = 0x0F,
    Function_PCIDevice      = 0x10,
    Function_DeviceStatus   = 0x11,
    Function_FRUDevice      = 0x12,
    Function_SMBIOSCount    = 0x13,
    Function_iLOVer         = 0x14,
    Function_NoisyI2C       = 0x15,
    Function_I2C            = 0x16,
    Function_BlockTransfer  = 0x17,
    Function_BackPlane      = 0x18,
    Function_BackplaneConfig= 0x19
};

enum PlatDefConfigExprHostState
{
    HostState_Off  = 0,
    HostState_POST = 1,
    HostState_On   = 2
};

enum PlatDefConfigPCIDeviceReqType
{
    PCIDevReqType_Slot           = 0,
    PCIDevReqType_Count          = 1,
    PCIDevReqType_IsDeviceInSlot = 2
};

enum PlatDefConfigExprDeviceStatus
{
    DevStatus_InvalidRequest = 0,
    DevStatus_Unknown        = 1,
    DevStatus_NotPresent     = 2,
    DevStatus_Failed         = 3,
    DevStatus_Degraded       = 4,
    DevStatus_OK             = 5
};

enum PlatDefConfigFRUDeviceField
{
    FRUDevField_DeviceName = 0
};

enum PlatDefConfigFRUDeviceReqType
{
    FRUDevReqType_Slot           = 0,
    FRUDevReqType_Count          = 1,
    FRUDevReqType_IsDeviceInSlot = 2
};

enum PlatDefConfigBackPlaneReqType
{
    BackPlaneReqType_Id           = 0,
    BackPlaneReqType_Type         = 1,
    BackPlaneReqType_DrvCnt       = 2
};

enum PlatDefConfigBackplaneConfigReqType
{
    BackplaneReqType_HasBackplaneName         = 0,
    BackplaneReqType_HasBoxNum                = 1,
    BackplaneReqType_BackplaneNameWithBoxNum  = 2
};

typedef union {

    UINT32  AsUINT32;

    struct {

        UINT8   Pass;
        UINT8   Minor;
        UINT8   Major;
        UINT8   Generation;

    } Version;

} PlatDefConfigILOVersion;

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefValidation_V1(deprecated)
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    PlatDefRecordHeader Header;

    UINT8               Flags;
    UINT8               Status;

    UINT16              TextLength;

    UINT16              BaseValidationID;
    UINT16              AltConfigID;

    UINT16              ErrorCode;
    UINT16              ErrorData1;
    UINT16              ErrorData2;

    UINT8               _Reserved[6];

    UINT8               ErrorText[1];

} PlatDefValidation_V1;

typedef PlatDefValidation_V1 PlatDefValidation;

enum PlatDefValidationFlagBits
{
    ValidationFlag_ReactionMask = 0x01|0x02|0x04,
      ValidationFlag_Internal = 0,
      ValidationFlag_Warning  = 1,
      ValidationFlag_Error    = 2,
      ValidationFlag_Critical = 3
};

enum PlatDefValidationStatusBits
{
    ValidationStatus_Valid      = 0x01,
    ValidationStatus_Ignore     = 0x02,
    ValidationStatus_Evaluating = 0x04,
    ValidationStatus_Evaluated  = 0x08
};

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefSensorGroup_V1
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    PlatDefRecordHeader Header;

    UINT16              Flags;
    UINT16              Status;

    UINT8               MemberCount;
    UINT8               ActiveCount;
    UINT16              Value;

    UINT8               _Reserved[8];

    UINT16              Members[1];

} PlatDefSensorGroup_V1;

typedef PlatDefSensorGroup_V1 PlatDefSensorGroup;

enum PlatDefGroupFlagBits
{
    GroupFlag_GroupTypeMask = 0x000F,
      GroupType_HottestOf     = 0x0001
};

enum PlatDefGroupStatusBits
{
    GroupStatus_Failed   = CommonStatus_Failed,
    GroupStatus_Degraded = CommonStatus_Degraded
};

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefLookupTable_V1
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    PlatDefRecordHeader Header;

    UINT8               KeyType;
    UINT8               ValueType;
    UINT16              Count;

    UINT8               _Reserved[4];

    UINT8               Data[1];

} PlatDefLookupTable_V1;

typedef PlatDefLookupTable_V1 PlatDefLookupTable;

enum PlatDefLookupTableDataType
{
    LookupTableDataType_None     = 0x00,
    LookupTableDataType_UInt8    = 0x01,
    LookupTableDataType_UInt16   = 0x02,
    LookupTableDataType_UInt32   = 0x03,
    LookupTableDataType_String   = 0x04,
    LookupTableDataType_PCIID    = 0x05,
    LookupTableDataType_RecordID = 0x06
};

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefThrottle_V1
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    PlatDefRecordHeader Header;

    PlatDefPrimitive    Throttle1;
    PlatDefPrimitive    Throttle2;
    PlatDefPrimitive    Throttle3;
    PlatDefPrimitive    Throttle4;
    PlatDefPrimitive    Throttle5;
    PlatDefPrimitive    Throttle6;
    PlatDefPrimitive    Throttle7;
    PlatDefPrimitive    Throttle8;
    PlatDefPrimitive    Throttle9;
    PlatDefPrimitive    Throttle10;
    PlatDefPrimitive    Throttle11;
    PlatDefPrimitive    Throttle12;
    PlatDefPrimitive    Throttle13;
    PlatDefPrimitive    Throttle14;
    PlatDefPrimitive    Throttle15;
    PlatDefPrimitive    Throttle16;

} PlatDefThrottle_V1;

typedef PlatDefThrottle_V1 PlatDefThrottle;

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefSystemDevice_V1
//
//  Also includes:
//
//  PlatDefSystemDeviceData
//
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//  PlatDefSystemDeviceData
//----------------------------------------------------------------------------

typedef union {

    UINT8   DeviceType;

    UINT8   RawBytes[32];

    struct {

        UINT8   DeviceType;
        UINT8   Segment;
        UINT16  Slot;
        UINT16  ParentSysDevID;
        UINT16  ProcessorNumberConfigID1;
        UINT8   SlotType;
        UINT16  ProcessorNumberConfigID2;

    } SlotDevice;

    struct {

        UINT8   DeviceType;
        UINT8   Segment;
        UINT8   OnboardType;
        UINT8   OnboardInstance;
        UINT16  ParentSysDevID;

    } OnboardDevice, OnboardSlotDevice;

    struct {

        UINT8   DeviceType;
        UINT8   Segment;
        UINT8   Address;
        UINT8   FRUAddress;
        UINT16  LoadCapacity;
        UINT16  TotalRequestedLoad;
        UINT8   Flags;

    } MegaCellDevice;

    struct {

        UINT8   DeviceType;
        UINT8   Segment;
        UINT8   Address;
        UINT8   Page;
        UINT8   FaultTableClass;
        UINT8   FaultTableByte;
        UINT8   FaultTableBit;
        UINT8   Flags;

    } VRDDevice, EFuseDevice, QuadComparatorDevice, VRDControllerDevice, MultiPhaseVRDControllerDevice;

    struct {

        UINT8   DeviceType;
        UINT8   Segment;
        UINT8   Address;

    } PowerPICDevice;

    struct {

        UINT8   DeviceType;
        UINT8   Segment;
        UINT8   Address;

    } UBMBackplanePICDevice;

    struct {

        UINT8   DeviceType;
        UINT8   Segment;
        UINT8   Address;

    } PCHDevice;

    struct {
        UINT8   DeviceType;   //SystemDeviceType_PDB
        UINT8   FRUi2cSegment;
        UINT8   FRUi2cAddress;   //Used of FRU authentication
        UINT8   CPLDi2cSegment; 
        UINT8   CPLDi2cAddress;  //Used for CPLD image flashing
    } PDBDevice, OCPInterposerDevice;

} PlatDefSystemDeviceData;

enum SystemDeviceType
{
    SystemDeviceType_Unspecified             = 0,
    SystemDeviceType_Slot                    = 1,
    SystemDeviceType_Onboard                 = 2,
    SystemDeviceType_OnboardSlot             = 3,
    SystemDeviceType_MegaCell                = 4,
    SystemDeviceType_EFuse                   = 5,
    SystemDeviceType_QuadComparator          = 6,
    SystemDeviceType_VRDController           = 7,
    SystemDeviceType_MultiPhaseVRDController = 8,
    SystemDeviceType_PowerPIC                = 9,
    SystemDeviceType_UBMBackplanePIC         = 10,
    SystemDeviceType_PCH                     = 11,
    SystemDeviceType_CPU                     = 12,
    SystemDeviceType_DIMM                    = 13,
    SystemDeviceType_GPU                     = 14,
    SystemDeviceType_PDB                     = 15,
    SystemDeviceType_OCPInterposer           = 16
};

enum SlotType
{
    SlotType_Unspecified = 0x00,
    SlotType_PCIe        = 0x01,
    SlotType_MXM         = 0x02,
    SlotType_Mezz        = 0x03,
    SlotType_AROC        = 0x04,
    SlotType_OCP30       = 0x05,
    SlotType_NVMe        = 0x06,
    SlotType_SXM2        = 0x07,
    SlotType_M_2         = 0x08,
    SlotType_Board_FPGA  = 0x09
};

enum OnboardDeviceType
{
    OnboardDeviceType_Unspecified    = 0x00,
    OnboardDeviceType_Other          = 0x01,
    OnboardDeviceType_Unknown        = 0x02,
    OnboardDeviceType_Video          = 0x03,
    OnboardDeviceType_SCSIController = 0x04,
    OnboardDeviceType_Ethernet       = 0x05,
    OnboardDeviceType_TokenRing      = 0x06,
    OnboardDeviceType_Sound          = 0x07,
    OnboardDeviceType_PATAController = 0x08,
    OnboardDeviceType_SATAController = 0x09,
    OnboardDeviceType_SASController  = 0x0A,
	OnboardDeviceType_EthernetModule = 0x0B,
	OnboardDeviceType_EthernetHybrid = 0x0C,
    OnboardDeviceType_CPU_PKG_TEMP            = 0x0D,
    OnboardDeviceType_CPU_PROC_VRD            = 0x0E,
    OnboardDeviceType_CPU_SOC_VRD             = 0x0F,    
    OnboardDeviceType_DIMM_0123_VRD_TEMP      = 0x10,
    OnboardDeviceType_DIMM_4567_VRD_TEMP      = 0x11,
    OnboardDeviceType_GPU                     = 0x12
};

enum MCFlag
{
    MCFlag_PresentInPOST       = 0x01,
    MCFlag_OutputEnabledInPOST = 0x02,
    MCFlag_Centralized         = 0x04
};

enum PMBusFlag
{
    PMBusFlag_VRDPageWrite = 0x01
};

//----------------------------------------------------------------------------
//  PlatDefSystemDevice_V1
//----------------------------------------------------------------------------

typedef struct {

    PlatDefRecordHeader      Header;

    PlatDefPrimitive         Detect;

    UINT16                   Flags;
    UINT16                   Status;

    UINT8                    _Reserved[12];

    PlatDefSystemDeviceData  Data;

    char                     AlternativeName[64];

} PlatDefSystemDevice_V1;

enum PlatDefSystemDeviceStatus
{
    SystemDeviceStatus_Present = CommonStatus_Present
};

typedef PlatDefSystemDevice_V1 PlatDefSystemDevice;

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefPECISegment_V1 
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    PlatDefRecordHeader      Header;

    UINT16                   Flags;
    UINT16                   SMBusOffset;

    UINT8                    Segment;
    UINT8                    Domain;
    UINT8                    PCIBus;
    UINT8                    PCIDeviceFunction;

    UINT8                    _Reserved[8];

} PlatDefPECISegment_V1;

typedef PlatDefPECISegment_V1 PlatDefPECISegment;

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefDeltaPatch_V1
//
//////////////////////////////////////////////////////////////////////////////

#define LENGTH_MASK 0xF000
#define OFFSET_MASK 0x0FFF

typedef struct {

    UINT16              RecordID;

    UINT16              OffsetAndLength;

    UINT8               Data[1];

} PlatDefDeltaPatchEntry_V1;

typedef PlatDefDeltaPatchEntry_V1 PlatDefDeltaPatchEntry;

typedef struct {

    PlatDefRecordHeader     Header;

    UINT16                  ConfigurationID;

    UINT16                  Count;

    PlatDefDeltaPatchEntry  Patch[1];

} PlatDefDeltaPatch_V1;

typedef PlatDefDeltaPatch_V1 PlatDefDeltaPatch;

//////////////////////////////////////////////////////////////////////////////
//
//  PlatDefReplacement_V1
//
//////////////////////////////////////////////////////////////////////////////

typedef struct {

    PlatDefRecordHeader Header;

    UINT16              ConfigurationID;
    UINT16              RecordID;
    UINT16              Length;

    UINT8               _Reserved[10];

    UINT8               Data[1];

} PlatDefReplacement_V1;

typedef PlatDefReplacement_V1 PlatDefReplacement;

//////////////////////////////////////////////////////////////////////////////
//
//  Bundled Platform Definition
//
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//  PlatDefBundleHeader
//----------------------------------------------------------------------------

typedef struct {

    char        Signature[16];
    UINT32      TotalSize;
    UINT16      Flags;
    UINT8       HeaderLength;
    UINT8       Count;
    UINT16      PlatformId[1];

} PlatDefBundleHeader;

#define PlatDefBundleSignature "$PlatdefBundle1$"
#define PlatDefBundleEndMarker "$EndOfTheBundle$"

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif // INCL_APML_PLATDEF_H
