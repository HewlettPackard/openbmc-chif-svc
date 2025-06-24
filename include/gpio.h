/*
// Copyright (c) 2007, 2022-2025 Hewlett Packard Enterprise Development, LP
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

#ifndef _GPIO_H_
#define _GPIO_H_


/* This file is the "C" interface provided to the Client applications .
*/

#ifdef __cplusplus
extern "C" {
#endif

// DCS - Taken from gstl vasregs.h ( Not in offical version in share ) >>>
#define  SERIAL_IN_DATA_LOW                 (R_FNC0_BASE + 0x040)
#define  SERIAL_IN_INT_EN_LOW               (R_FNC0_BASE + 0x044)
#define  SERIAL_IN_INT_STATUS_LOW           (R_FNC0_BASE + 0x048)
#define  SERIAL_IN_DATA_HIGH                (R_FNC0_BASE + 0x060)
#define  SERIAL_IN_INT_EN_HIGH              (R_FNC0_BASE + 0x064)
#define  SERIAL_IN_INT_STATUS_HIGH          (R_FNC0_BASE + 0x068)
#define  MEMID_CONTROL                      (R_FNC0_BASE + 0x0b8)
// DCS - End of defines not in offical vasregs.h                       <<<

//================================================================
// Defines to support "Gpio Service" via vascomm
//================================================================

#define  GPIO_CMD_GROUP    0xE1    // Must be unique

// gpioRequest->CmdCode
#define  GPIO_GET_BIT          0x01
#define  GPIO_SET_BIT          0x02
#define  GPIO_GET_BYTE         0x03
#define  GPIO_SET_BYTE         0x04
#define  GPIO_LIST             0x05
#define  GPIO_GET_MEMID        0x07    // 0x06 (GPIO_INIT) was removed
#define  GPIO_GET_CPLD         0x08
#define  GPIO_GET_MEMID_ALL    0x0a    // 0x09 (GPIO_INIT_BIT) was removed
#define  GPIO_SET_CPLD         0x0b
#define  GPIO_GET_XREG         0x0c
#define  GPIO_RMW_CPLD         0x0d
#define  GPIO_GET_ATTR         0x0E

#define  GPIO_INVALID_CMD_NUM  0x0f //must be last value + 1

// gpioRequest->Type (all possible scanchains)
#define GPIO_GPI       (1)
#define GPIO_GPO       (2)
#define GPIO_CPLD      (3)
#define GPIO_MEMID     (4)
#define GPIO_ALL       (5) //actually infers GPI and GPO - for listing
#define GPIO_GPO_SORST (6)
#define GPIO_GPOOWN    (7)

// gpioRequest->gpioNum max values (based on 'op')
//#define LIMIT_MEMID     (256) // only use GPIO_MEMID_DATA_SIZE everywhere
#define RANGE_CPLD        (0x1ff) // changed CPLD size from '0xff' to '0x1ff'
#define RANGE_GPI_BYTE    (8)   // qty of GPI bytes
#define RANGE_GPO_BYTE    (16)  // qty of GPO bytes
#define RANGE_GPI_BIT     (64)  // qty of GPI bits
#define RANGE_GPO_BIT     (128) // qty of GPO bits
#define RANGE_GPIINT      (7)   // 8 bytes in GPI and all can be interrupt enabled
#define MAX_GPI_LOW_RANGE (3)   // Low reg is bytes 0-3
#define MAX_GPI_HI_RANGE  (7)   // Hi  reg is bytes 4-7
#define MAX_GPO_LOW_RANGE (7)   // Low reg is bytes 0-7
#define MAX_GPO_HI_RANGE  (15)  // Hi  reg is bytes 8-15
#define MAX_MEMID_TO_READ (16)  // Max number of MEMID bytes to read
#define OFFSET_GPI_LOW_BYTE   (0x40)// Starting offset of GPI low (0-3) bytes
#define OFFSET_GPI_HI_BYTE    (0x60)// Starting offset of GPI high (4-7) bytes
#define OFFSET_GPO_LOW_BYTE   (0xB0)// Starting offset of GPO low (0-7) bytes
#define OFFSET_GPO_HI_BYTE    (0xF8)// Starting offset of GPO high (8-15) bytes

// gpioResponse->compCode
#define  GPIO_CMD_SUCCESS      0x00
#define  GPIO_CMD_INVALID      0x01
#define  GPIO_CMD_OUT_OF_RANGE 0x02
#define  GPIO_SERVICE_ERR      0x03
#define  GPIO_CMD_INIT_ERR     0x04
#define  GPIO_INVALID_PARM     0x05
#define  GPIO_HW_ERR           0x06


/* if you add new response codes, adjust gpio_err_str in gpio_client.c */

typedef struct {
    int   pin;
    UINT8 initial;
    char* description;
    UINT8 keep_on_pgood;
} GPIO_DESCR;

typedef struct {
    UINT8 cmdGroup;
    UINT8 cmdCode;
    UINT8 gpioType;
    UINT8 gpioNum;
    UINT8 gpioVal;
    UINT8 gpioOpt;
    int   base;
    int   gpio_count; //need to be able to send 256 for MEMID count
} GPIO_MESSAGE_REQ;

#define GPIO_CPLD_DATA_SIZE  (256) // transfer limit, not register base limit
#define GPIO_CPLD_RANGE    (0x200) // Max Xregister per Gen11 spec
#define GPIO_MEMID_DATA_SIZE (256)

typedef struct {
    UINT8 compCode;
    UINT8 gpioVal;
    UINT8 filler;
    UINT8 memid_data[GPIO_MEMID_DATA_SIZE];
    UINT8 gpio_data8[GPIO_CPLD_DATA_SIZE];
} GPIO_MESSAGE_RESP;


#define GPIO_HANDLER_PRI       100
#define GPIO_HANDLER_STACK     5*1024

//================================================================
// GXE specific macros
//================================================================

#define GXE_ISIIEN(x)       (0x80000108+(x))
#define GXE_ISIISTAT(x)     (0x80000100+(x))

//================================================================
// Exported API
//================================================================
extern UINT8 gpio_get_descriptors(UINT8 type, GPIO_DESCR** table);
extern UINT8 gpio_set(UINT8 type, UINT8 is_a_byte, UINT8 num, UINT8 value);
extern UINT8 gpio_get(UINT8 type, UINT8 is_a_byte, UINT8 num, UINT8 *value);

extern UINT8 gpi_get_bit(UINT8 bit, UINT8 *value);
extern UINT8 gpi_get_byte(UINT8 byte, UINT8 *value);
extern UINT8 gpo_set_bit(UINT8 bit, UINT8 value);
extern UINT8 gpo_get_bit(UINT8 bit, UINT8 *value);
extern UINT8 gpo_set_byte(UINT8 byte, UINT8 value);
extern UINT8 gpo_get_byte(UINT8 byte, UINT8 *value);
extern UINT8 gpo_set_sorst_bit(UINT8 bit, UINT8 value);
extern UINT8 gpo_get_sorst_bit(UINT8 bit, UINT8 *value);
extern UINT8 gpo_set_sorst_byte(UINT8 bit, UINT8 value);
extern UINT8 gpo_get_sorst_byte(UINT8 bit, UINT8 *value);
extern UINT8 gpo_set_gpoown_bit(UINT8 bit, UINT8 value);
extern UINT8 gpo_get_gpoown_bit(UINT8 bit, UINT8 *value);
extern UINT8 gpo_set_gpoown_byte(UINT8 bit, UINT8 value);
extern UINT8 gpo_get_gpoown_byte(UINT8 bit, UINT8 *value);

/* gpio_memid()  Retrieves array (up to count) of MEMID data */
extern UINT8 gpio_get_memid_range(UINT8 value[], int count);
extern UINT8 gpio_get_memid( UINT8 byte, UINT8 *value );

extern UINT32 get_sideband_nic_list();

/* gpio_exp_data()  Retrieves array ( from 0 up to count) of XRegister (CPLD) data */
extern UINT8 gpio_get_xregs(UINT8 *buf, int base, int count);
extern UINT8 gpio_exp_set( int offset, UINT8 value );  // set byte
extern UINT8 gpio_exp_get( int offset, UINT8 *value ); // get byte
extern UINT8 gpio_exp_rmw( int offset, UINT8 clear, UINT8 set ); // read @offset, clear bits, set bits, write @offset
extern UINT8 gpio_get_attr(UINT32 *buf, int base, int count); // read attributes DWORDS into array
extern UINT8 gpio_get_cpu_count(UINT8* cpu_count);

extern UINT8 gpio_i2c_bus0_is_accessible(void); // return 0 when I2C bus 0 is used by system ROM (MRC) or HW (Gen9 Maint_SW Pos. 12)

/* define XRegister offsets for GPIO.  These must be consistent with expbus.h.
 * The offsets are passed into the gpio_exp...() API
 * Updated as per Gen11 XReg Spec Rev 0x02 */
#define GPIO_OFS_EXPBUS_IB0                  (0x00)
#define GPIO_OFS_EXPBUS_IB1                  (0x01)
#define GPIO_OFS_EXPBUS_IB2                  (0x02)
#define GPIO_OFS_EXPBUS_IB3                  (0x03)
#define GPIO_OFS_EXPBUS_IOP_LEDS             (0x04)
#define GPIO_OFS_EXPBUS_UID                  (0x05)
#define GPIO_OFS_EXPBUS_IO_0D                (0x06)
#define GPIO_OFS_EXPBUS_FAULTS               (0x07)
#define GPIO_OFS_EXPBUS_FAULTS2              (0x08)
#define GPIO_OFS_EXPBUS_FW_VALIDATION        (0x09)
#define GPIO_OFS_EXPBUS_AUX_VID              (0x0A)
#define GPIO_OFS_EXPBUS_MISC_MODES           (0x0B)
#define GPIO_OFS_EXPBUS_ACM_CHECKPOINT       (0x0C)
#define GPIO_OFS_EXPBUS_BIOS_CHECKPOINT      (0x0D)
#define GPIO_OFS_EXPBUS_ROM_PORT             (0x0E)

#define GPIO_OFS_EXPBUS_RAS_FEATURES_1       (0x10)
#define GPIO_OFS_EXPBUS_RAS_FEATURES_2       (0x11)
#define GPIO_OFS_EXPBUS_DIMM_ALERT0          (0x13)
#define GPIO_OFS_EXPBUS_DIMM_ALERT1          (0x14)
#define GPIO_OFS_EXPBUS_NODE_INFO            (0x15) // see HW_ATTR_STORAGE_PLATFORM (helper fan monitor = [7], slot_id = [1:0])
#define GPIO_OFS_EXPBUS_RESERVED_16          (0x16) // I2C Req/Gnt Arbitration (referenced in APML)
#define GPIO_OFS_EXPBUS_RESERVED_17          (0x17) // Gen11 Xreg Spec - Free Space - Once PMCI IE cleanup is done, this will be removed
#define GPIO_OFS_EXPBUS_RESERVED_1D          (0x1D)
#define GPIO_OFS_EXPBUS_IE_CTRL              (0x1E) // Gen11 Xreg Spec - Free Space - Once PMCI IE cleanup is done, this will be removed
#define GPIO_OFS_EXPBUS_IE_STATUS            (0x1F) // Gen11 Xreg Spec - Free Space - Once PMCI IE cleanup is done, this will be removed

#define GPIO_OFS_EXPBUS_ILO_MEM_CFG_DATA3    (0x20)
#define GPIO_OFS_EXPBUS_ILO_MEM_CFG_DATA2    (0x21)
#define GPIO_OFS_EXPBUS_ILO_MEM_CFG_DATA1    (0x22)
#define GPIO_OFS_EXPBUS_ILO_MEM_CFG_DATA0    (0x23)
#define GPIO_OFS_EXPBUS_ILO_MEM_CFG_INDEX    (0x24)
#define GPIO_OFS_EXPBUS_SOCL1_PWM_LIMIT_1PS  (0x25)
#define GPIO_OFS_EXPBUS_SOCL1_PWM_LIMIT_2PS  (0x26)
#define GPIO_OFS_EXPBUS_SOCL2_PWM_LIMIT_1PS  (0x27)
#define GPIO_OFS_EXPBUS_SOCL2_PWM_LIMIT_2PS  (0x28)
#define GPIO_OFS_EXPBUS_ENV_STATUS           (0x29)
#define GPIO_OFS_EXPBUS_EMMC_CTRL            (0x2B)
#define GPIO_OFS_EXPBUS_RESERVED_37          (0x2C)

#define GPIO_OFS_EXPBUS_ILO_ATTR_DATA3       (0x30)
#define GPIO_OFS_EXPBUS_ILO_ATTR_DATA2       (0x31)
#define GPIO_OFS_EXPBUS_ILO_ATTR_DATA1       (0x32)
#define GPIO_OFS_EXPBUS_ILO_ATTR_DATA0       (0x33)
#define GPIO_OFS_EXPBUS_ILO_ATTR_INDEX       (0x34)
#define GPIO_OFS_EXPBUS_LOM_STAT             (0x36)
#define GPIO_OFS_EXPBUS_AUX_FAN_SPEED        (0x37)
#define GPIO_OFS_EXPBUS_NET0                 (0x3A)
#define GPIO_OFS_EXPBUS_NET1                 (0x3B)
#define GPIO_OFS_EXPBUS_NET2                 (0x3C)
#define GPIO_OFS_EXPBUS_NET3                 (0x3D)
#define GPIO_OFS_EXPBUS_NET4                 (0x3E)
#define GPIO_OFS_EXPBUS_NET5                 (0x3F)

#define GPIO_OFS_EXPBUS_NET6                 (0x40)
#define GPIO_OFS_EXPBUS_PS_ENABLE            (0x41)
#define GPIO_OFS_EXPBUS_PS_PRESENCE          (0x42)
#define GPIO_OFS_EXPBUS_PS_SID_LED           (0x43)
#define GPIO_OFS_EXPBUS_PS_AC_OK             (0x44)
#define GPIO_OFS_EXPBUS_PS_DC_OK             (0x45)
#define GPIO_OFS_EXPBUS_PS_EFUSE_MISC        (0x46)
#define GPIO_OFS_EXPBUS_RESERVED_47          (0x47)
#define GPIO_OFS_EXPBUS_DISABLE              (0x48)
#define GPIO_OFS_EXPBUS_EFUSE_MISC           (0x49)
#define GPIO_OFS_EXPBUS_PLAT_MISC            (0x4A)
#define GPIO_OFS_EXPBUS_OB2                  (0x4B)
#define GPIO_OFS_EXPBUS_RESERVED_4C          (0x4C)
#define GPIO_OFS_EXPBUS_PLATFORM_GENERATION  (0X4D)
#define GPIO_OFS_EXPBUS_FORCEPR_ILO_CTRL     (0x4E)
#define GPIO_OFS_EXPBUS_BLADE                (0x4F)

#define GPIO_OFS_EXPBUS_CANMIC_ALERT1        (0x50)
#define GPIO_OFS_EXPBUS_PS_ALERT_THRESHOLD   (0x51)
#define GPIO_OFS_EXPBUS_PS_MON               (0x52)
#define GPIO_OFS_EXPBUS_PS_STAT_ILO          (0x53)
#define GPIO_OFS_EXPBUS_PS_STAT_CPLD         (0x54)
#define GPIO_OFS_EXPBUS_MEZZ_8_1_AUXCYCLE    (0x55)
#define GPIO_OFS_EXPBUS_MEZZ_16_9_AUXCYCLE   (0x56)
#define GPIO_OFS_EXPBUS_SN_STATUS_50         (0x57) // Not consistent with Gen11 SPEC. TBD: Update.
#define GPIO_OFS_EXPBUS_SN_UART_51           (0x58) // Not consistent with Gen11 SPEC. TBD: Update.

/*Edgeline and Moonshot xreg space*/
#define GPIO_OFS_EXPBUS_CHASSIS_ATTR         (0x59)
#define GPIO_OFS_EXPBUS_MOONSHOT_PRESENCE    (0x5A)
#define GPIO_OFS_EXPBUS_MOONSHOT_MODE        (0x5B)
#define GPIO_OFS_EXPBUS_RESERVED_5C          (0x5C)
#define GPIO_OFS_EXPBUS_RESERVED_5D          (0x5D)

/*Liquid colling xreg*/
#define GPIO_OFS_EXPBUS_LIQUIDCOOL_PUMP      (0x5E)
#define GPIO_OFS_EXPBUS_LIQUIDCOOL_FAULT     (0x5F)

#define GPIO_OFS_EXPBUS_LIQUIDCOOL_PWM       (0x60)
#define GPIO_OFS_EXPBUS_MEZZ_8_1_DISABLE     (0x62)
#define GPIO_OFS_EXPBUS_MEZZ_16_9_DISABLE    (0x63)
#define GPIO_OFS_EXPBUS_RESERVED_39          (0x39)
#define GPIO_OFS_EXPBUS_RESERVED_2D          (0x68)
#define GPIO_OFS_EXPBUS_RESERVED_2E          (0x69)
#define GPIO_OFS_EXPBUS_RESERVED_2F          (0x6A)
#define GPIO_OFS_EXPBUS_CPLD_PRESENCE_7_0               (0x6C) //CPLD presence indicator. Bit0 is reserved for main CPLD and will always be 1. // 1 = CPLD present,   0 = CPLD not present
#define GPIO_OFS_EXPBUS_CPLD_PRESENCE_15_8              (0x6D) //CPLD presence indicator. Bit0 is reserved for main CPLD and will always be 1. // 1 = CPLD present,   0 = CPLD not present
#define GPIO_OFS_EXPBUS_CPLD_SECSTART_MUX_7_0           (0x6E) //CPLD secure start source select. Correspond to bits CPLD_PRESENCE. A bit set to 1 indicates that CPLD driving secure start register set
#define GPIO_OFS_EXPBUS_CPLD_SECSTART_MUX_15_8          (0x6F) //CPLD secure start source select. Correspond to bits CPLD_PRESENCE. A bit set to 1 indicates that CPLD driving secure start register set

#define GPIO_OFS_EXPBUS_RESERVED_0C          (0x70)
#define GPIO_OFS_EXPBUS_HW_LOG_0             (0x71)
#define GPIO_OFS_EXPBUS_HW_LOG_1             (0x72)
#define GPIO_OFS_EXPBUS_ILO_RESET_CAUSE      (0x73)
#define GPIO_OFS_EXPBUS_POWER_FAULT0         (0x74)
#define GPIO_OFS_EXPBUS_PWRFAULT_ADDR        GPIO_OFS_EXPBUS_POWER_FAULT0
#define GPIO_OFS_EXPBUS_POWER_FAULT1         (0x75)
#define GPIO_OFS_EXPBUS_PWRFAULT_DATA        GPIO_OFS_EXPBUS_POWER_FAULT1
#define GPIO_OFS_EXPBUS_INTERLOCK_SEL        (0x76)
#define GPIO_OFS_EXPBUS_INTERLOCK_ADDR       GPIO_OFS_EXPBUS_INTERLOCK_SEL
#define GPIO_OFS_EXPBUS_INTERLOCK_DATA       (0x77)
#define GPIO_OFS_EXPBUS_FAN_8_1_LED          (0x78)
#define GPIO_OFS_EXPBUS_FAN_16_9_LED         (0x79)
#define GPIO_OFS_EXPBUS_FAN_8_1_INST         (0x7A)
#define GPIO_OFS_EXPBUS_FAN_16_9_INST        (0x7B)
#define GPIO_OFS_EXPBUS_FAN_8_1_FAIL         (0x7C)
#define GPIO_OFS_EXPBUS_FAN_16_9_FAIL        (0x7D)
#define GPIO_OFS_EXPBUS_LIQUIDCOOL_FAN_FAIL  (0x7E)
#define GPIO_OFS_EXPBUS_FAN_8_1_ID           (0x7F)

#define GPIO_OFS_EXPBUS_FAN_16_9_ID          (0x80)
#define GPIO_OFS_EXPBUS_JTAG_MUX_SELECT       (0x81) //MUX select for JTAG star topology to update multiple CPLDs.
#define GPIO_OFS_EXPBUS_I2C_BUS_7_0_RESET    (0x82)
#define GPIO_OFS_EXPBUS_I2C_BUS_15_8_RESET   (0x83)
#define GPIO_OFS_EXPBUS_I2C_MUX_SEL_70       (0x84)
#define GPIO_OFS_EXPBUS_I2C_MUX_SEL_71       (0x85)
#define GPIO_OFS_EXPBUS_I2C_MUX_SEL_72       (0x86)
#define GPIO_OFS_EXPBUS_I2C_MUX_SEL_73       (0x87)
#define GPIO_OFS_EXPBUS_I2C_MUX_SEL_74       (0x88)
#define GPIO_OFS_EXPBUS_I2C_MUX_SEL_75       (0x89)
#define GPIO_OFS_EXPBUS_I2C_MUX_SEL_76       (0x8A)
#define GPIO_OFS_EXPBUS_I2C_MUX_SEL_77       (0x8B)
#define GPIO_OFS_EXPBUS_I2C_MUX_SEL_78       (0x8C)
#define GPIO_OFS_EXPBUS_I2C_MUX_SEL_79       (0x8D)
#define GPIO_OFS_EXPBUS_I2C_MUX_SEL_7A       (0x8E)
#define GPIO_OFS_EXPBUS_I2C_MUX_SEL_7B       (0x8F)

#define GPIO_OFS_EXPBUS_I2C_MUX_SEL_7C       (0x90)
#define GPIO_OFS_EXPBUS_I2C_MUX_SEL_7D       (0x91)
#define GPIO_OFS_EXPBUS_I2C_MUX_SEL_7E       (0x92)
#define GPIO_OFS_EXPBUS_I2C_MUX_SEL_7F       (0x93)

/*Extended xreg space*/
#define GPIO_OFS_EXPBUS_INTERLOCK_INJ0       (0x0100)
#define GPIO_OFS_EXPBUS_INTERLOCK_INJ1       (0x0101)
#define GPIO_OFS_EXPBUS_INTERLOCK_INJ2       (0x0102)
#define GPIO_OFS_EXPBUS_INTERLOCK_INJ3       (0x0103)
#define GPIO_OFS_EXPBUS_POWER_FAULT_INJ0     (0x0104)
#define GPIO_OFS_EXPBUS_POWER_FAULT_INJ1     (0x0105)
#define GPIO_OFS_EXPBUS_POWER_FAULT_INJ2     (0x0106)
#define GPIO_OFS_EXPBUS_POWER_FAULT_INJ3     (0x0107)
#define GPIO_OFS_EXPBUS_RESERVED_6B          (0x010A)
#define GPIO_OFS_EXPBUS_RESERVED_6C          (0x010B)

#define GPIO_OFS_EXPBUS_AMP_1                (0x0118)
#define GPIO_OFS_EXPBUS_AMP_2                (0x0119)
#define GPIO_OFS_EXPBUS_AMP_3                (0x011A)

#define GPIO_OFS_EXPBUS_AMP_4_DEV0_BYTE0     (0x0120)
#define GPIO_OFS_EXPBUS_AMP_4_DEV0_BYTE1     (0x0121)
#define GPIO_OFS_EXPBUS_AMP_4_DEV1_BYTE0     (0x0122)
#define GPIO_OFS_EXPBUS_AMP_4_DEV1_BYTE1     (0x0123)
#define GPIO_OFS_EXPBUS_AMP_4_DEV2_BYTE0     (0x0124)
#define GPIO_OFS_EXPBUS_AMP_4_DEV2_BYTE1     (0x0125)
#define GPIO_OFS_EXPBUS_AMP_4_DEV3_BYTE0     (0x0126)
#define GPIO_OFS_EXPBUS_AMP_4_DEV3_BYTE1     (0x0127)
#define GPIO_OFS_EXPBUS_AMP_4_DEV4_BYTE0     (0x0128)
#define GPIO_OFS_EXPBUS_AMP_4_DEV4_BYTE1     (0x0129)
#define GPIO_OFS_EXPBUS_AMP_4_DEV5_BYTE0     (0x012A)
#define GPIO_OFS_EXPBUS_AMP_4_DEV5_BYTE1     (0x012B)
#define GPIO_OFS_EXPBUS_AMP_4_DEV6_BYTE0     (0x012C)
#define GPIO_OFS_EXPBUS_AMP_4_DEV6_BYTE1     (0x012D)
#define GPIO_OFS_EXPBUS_AMP_4_DEV7_BYTE0     (0x012E)
#define GPIO_OFS_EXPBUS_AMP_4_DEV7_BYTE1     (0x012F)

#define GPIO_OFS_EXPBUS_SOCL1_PWM_LIMIT_3PS  (0x0190)
#define GPIO_OFS_EXPBUS_SOCL1_PWM_LIMIT_4PS  (0x0191)
#define GPIO_OFS_EXPBUS_SOCL1_PWM_LIMIT_5PS  (0x0192)
#define GPIO_OFS_EXPBUS_SOCL1_PWM_LIMIT_6PS  (0x0193)

#define GPIO_OFS_EXPBUS_SOCL2_PWM_LIMIT_3PS  (0x0194)
#define GPIO_OFS_EXPBUS_SOCL2_PWM_LIMIT_4PS  (0x0195)
#define GPIO_OFS_EXPBUS_SOCL2_PWM_LIMIT_5PS  (0x0196)
#define GPIO_OFS_EXPBUS_SOCL2_PWM_LIMIT_6PS  (0x0197)

#define GPIO_OFS_EXPBUS_GPU_RISER_ID_1_2     (0x0199)
#define GPIO_OFS_EXPBUS_GPU_RISER_ID_3_4     (0x019A)
#define GPIO_OFS_EXPBUS_GPU_RISER_ID_5_6     (0x019B)
#define GPIO_OFS_EXPBUS_GPU_RISER_ID_7_8     (0x019C)
#define GPIO_OFS_EXPBUS_GPU_RISER_ID_9_10    (0x019D)
#define GPIO_OFS_EXPBUS_GPU_RISER_ID_11_12   (0x019E)


#define GPIO_OFS_EXPBUS_SHREG_GLOBAL         (0x01AA)
#define GPIO_OFS_EXPBUS_SHREG_MASK_DATA      (0x01AB)
#define GPIO_OFS_EXPBUS_SHERG_SET_VAL        (0x01AC)
#define GPIO_OFS_EXPBUS_SHERG_WR_PROTECT     (0x01AD)
#define GPIO_OFS_EXPBUS_SHERG_CONTROL_ADDR1  (0x01AE)
#define GPIO_OFS_EXPBUS_SHERG_CONTROL_ADDR2  (0x01AF)


// define GPI bit assignments.
// Note: if you use a fixed-position GPI bit in iLO,
// it should be listed here.
#define GPI_BIOS_OWNS_I2C0      (7)  // "GMT DIMM Polling Enable|01" byte 0 bit 7
                                     // (Same for Gen8 and Gen9+) is set by ROM when running MRC:
                                     // ROM owns bus 0 (and PECI) and iLO should stay off
#define GPI_ROM_MRC             (GPI_BIOS_OWNS_I2C0)

#define GPI_CATERR_HOLD_DET_G9  (25) // Gen9+: "CATERR_HOLD_DET|10" bit 1 4th byte

#define GPI_DIMM_CONFIG_ERR     (24) //  Gen8: "DIMM Configureation Error|01" bit 0 4th byte
#define GPI_DIMM_CONFIG_ERR_G9  (15) //  Gen9: "DIMM Configureation Error|01" bit 7 2th byte
#define GPO_DIMM_CONFIG_ERR_G9  GPI_DIMM_CONFIG_ERR_G9

#define GPI_SIDECAR_1           (55) // Gen9: this is for first sidecar
#define GPI_SIDECAR_2           (54) // Gen9: this is for second sidecar

//#define GPI_DISABLE_CATERR_HOLD (31)  // not being referenced

#define GPI_BYTE_CATERR          (3) //  GPI byte 3
#define GPI_BYTE_PS_FAIL_STATUS  (5) //  GPI byte 5 lists the failure status of each power supply. Now supports 8 supplies.
                                     //  Was byte 2 (7:4) in Gen8 and Gen9

// define GPO bit assignments
// Note: if you use a fixed-position GPO bit in iLO,
// it should be listed here.
#define GPO_BIOS_FACT_DEFAULTS  (14) // GPO bit 14 - set to 1 by iLO; cleared later by BIOS; Ampere and Gen 12 common
#define GPO_AUX_CYCLE           (28) // GPO chain 1 byte 3 bit 4.  Set to 1 under AUX

                                     //   if set to 1 results in an aux power cycle at next PGood deassert  - BL ONLY
#define GPO_AUX_CYCLE_BL_G9     (24) // "Force aux EFUSE cycling|01" byte 3 bit 0,
                                     //   if set to 1 results in an aux power cycle at next PGood deassert - BL & ML/DL

#define GPO_DISABLE_MAIN_EFUSE  (26) // "Force main EFUSE OFF|01" Disable the main efuse, leaves AUX efuse alive

#define GPO_EN_CPU_FPT          (64) // Enable Fast Platform Throttle for processors

// Type 10 feature enable bit is not defined in Gen11
//#define GPO_REVELATION_ENABLED  (110) // GPO chain 2 byte 5 bit 6. Indicates Type 10 Persistent Memory Backup is enabled.

#define GPO_ILO_ACK_I2C0        (53) /* GPO Chain 1 byte 6 bit 5 */
#define GPO_ILO_ACK_PECI        (54) /* GPO Chain 1 byte 6 bit 6 */

// MEMID offset definitions
#define SCAN_CHAIN_SYSTEM_MAINTENANCE_BYTE1 0x00
#define SCAN_CHAIN_SYSTEM_MAINTENANCE_BYTE2 0x01
#define SCAN_CHAIN_PLATFORM_ID_PCA          0x03
#define SCAN_CHAIN_CPU_PRESENCE_INDEX       0x04
#define SCAN_CHAIN_MEMID_ROC_ID             0x0A
#define SCAN_CHAIN_PSU_DESIGNATOR           0x10
#define SCAN_CHAIN_CPU0_PROCINFO            0x12 //See spec, these are NDA values from AMD and Intel. Moved from 0x70 in Gen11
// SCAN_CHAIN_TBIRD_BLADE and SCAN_CHAIN_EAGLE have changed to Bytes 0x46, 0x47 and 0x48 in Gen11
//#define SCAN_CHAIN_TBIRD_BLADE            0x12 //This is used on Tbird platforms only currently, on others it is reserved at 0
//#define SCAN_CHAIN_EAGLE                  0x14
#define SCAN_CHAIN_OCP_CARD_ID              0x16 //Moved from 0x6B in Gen11
#define SCAN_CHAIN_VRD_VENDOR               0x1C
#define SCAN_CHAIN_MEMID_VERSION            0x24
#define SCAN_CHAIN_MEMID_CPLD_VERS_OFF      0x25
#define SCAN_CHAIN_MEMID_CPLD_BASE_VERS_OFF 0x26
#define SCAN_CHAIN_REDUNDANT_CPLD_FW_VERSION_INDEX 0x27  // CPLD Version or Bootleg Number
#define SCAN_CHAIN_REDUNDANT_CPLD_BASE_CODE_INDEX  0x28  // CPLD Base Code
#define SCAN_CHAIN_CPLD_SHTDWN_PWR_FAULT_BYTE1 0x3D
#define SCAN_CHAIN_CPLD_SHTDWN_PWR_FAULT_BYTE2 0x3E
#define SCAN_CHAIN_CPLD_SHTDWN_PWR_FAULT_BYTE3 0x3F
#define SCAN_CHAIN_NVRAM_CLEAR_MASK         0x40
#define SCAN_CHAIN_ILO_RESET_CAUSE_LATCHED  0x43 //Moved from 0x44 in Gen11
#define SCAN_CHAIN_EXP_BLADE_PRESENCE_BYTE1 0x46
#define SCAN_CHAIN_EXP_BLADE_PRESENCE_BYTE2 0x47
#define SCAN_CHAIN_EXP_BLADE_PRESENCE_BYTE3 0x48
#define SCAN_CHAIN_BASE_PRESENCE_BYTE1      0x4C //Moved from 0x0D in Gen11
#define SCAN_CHAIN_BASE_PRESENCE_BYTE2      0x4D //Moved from 0x0E in Gen11
#define SCAN_CHAIN_BASE_MEZZ_MATED_BYTE1    0x51 //Moved from 0x33 in Gen11
#define SCAN_CHAIN_BASE_MEZZ_MATED_BYTE2    0x52 //Moved from 0x34 in Gen11
#define SCAN_CHAIN_BASE_MEZZ_PGD_VAUX_BYTE1 0x53 //Moved from 0x35 in Gen11
#define SCAN_CHAIN_BASE_MEZZ_PGD_VAUX_BYTE2 0x54 //Moved from 0x36 in Gen11
#define SCAN_CHAIN_BASE_MEZZ_PGD_MAIN_BYTE1 0x55 //Moved from 0x37 in Gen11
#define SCAN_CHAIN_BASE_MEZZ_PGD_MAIN_BYTE2 0x56 //Moved from 0x38 in Gen11
/* MEMID - Helper fan period readings */
#define SCAN_CHAIN_HELP_FAN1_LWR            0x70
#define SCAN_CHAIN_HELP_FAN1_UPR            0x71
#define SCAN_CHAIN_HELP_FAN2_LWR            0x72
#define SCAN_CHAIN_HELP_FAN2_UPR            0x73

//CPU0 Proc info definitions
#define PROC_ID_MASK                             0x03 //Intel Proc ID's  00 = CooperLake, 01 IceLake (NOTE THIS IS INTEL NDA)
#define COOPER_LAKE_ID                           0x00
#define ICE_LAKE_ID                              0x01

//MEMID mask Definitions
#define M2_PRESENCE                         0x01 //M2 is installed
#define BP_PRESENCE                         0x01 //BP is installed, Changed from Byte 0x12 Bit 1 to Byte 0x47 Bit 0 in Gen11
#define M2_KEY                              0x0C //Mask for M2 padleboard ID value
#define M2_KEY_SHIFT                           2 //Bits rightshfit to bring the key to a 0 based index
#define BP_KEY                              0x0E //Mask for storage backplane ID value
#define BP_KEY_SHIFT                           1 //Bits rightshfit to bring the key to a 0 based index
#define GRAPHICS_EXPANDER                   0x30
#define EXPANDER                            0x01 // Changed from Byte 0x12 Bit 7 to Byte 0x48 Bit 0 in Gen11

//Shared network port options bit set
#define EMBNIC_SIDEBAND_BIT_SET   (0x00000002) /* Embedded NIC */ 
#define OCP1_SIDEBAND_BIT_SET     (0x00000010) /* OCP1 */ 
#define OCP2_SIDEBAND_BIT_SET     (0x00000020) /* OCP2 */ 

// GPIO_OFS_EXPBUS_NODE_INFO (see HW_ATTR_STORAGE_PLATFORM)
#define SLOT_ID_MASK                        0x03
#define ENABLE_HLP_FAN_FAILURE              0x80 // Directs CPLD to OR in a helper fan out of range with PS_FAIL for the FRU

#ifdef __cplusplus
}
#endif

#endif // _GPIO_H
