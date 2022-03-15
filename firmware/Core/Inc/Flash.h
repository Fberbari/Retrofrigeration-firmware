#pragma once

/**
 * This module holds the interface to the flash chip
 * Author(s): David Liu
 */

#include "Common.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

#define sFLASH_TIMEOUT			  0x01
#define sFLASH_WIP_FLAG           0x01  /* Write In Progress (WIP) flag */
#define sFLASH_DUMMY_BYTE         0xA5
#define sFLASH_SPI_PAGESIZE       0x100


/* GD25D05C SPI Flash supported commands */
#define sFLASH_CMD_WREN           0x06  /* Write enable instruction */
#define sFLASH_CMD_WRDI           0x04  /* Write disable instruction */

#define sFLASH_CMD_RDSR           0x05  /* Read Status Register instruction  */
#define sFLASH_CMD_WRSR           0x01  /* Write Status Register instruction */

#define sFLASH_CMD_READ           0x03  /* read byte instruction */
#define sFLASH_CMD_READF          0x0B  /* read byte at faster speed instruction */
#define sFLASH_CMD_DUAL           0x3B  /* Dual Output Fast Read instruction (uses both MISO and MOSI for data) */

#define sFLASH_CMD_PPRO           0x02  /* Page Program instruction */

#define sFLASH_CMD_SE             0x20  /* Sector Erase instruction */
#define sFLASH_CMD_32BE           0x52  /* 32KB Block Erase instruction */
#define sFLASH_CMD_64BE           0xD8  /* 64KB Block Erase instruction */
#define sFLASH_CMD_CE             0x60  /* Chip Erase instruction */

#define sFLASH_CMD_DPD            0xB9  /* Deep Power Down instruction*/
#define sFLASH_CMD_LDPD           0xAB  /* Leave Deep Power Down instruction*/

#define sFLASH_CMD_REMS           0x90  /* Read Manufacture ID*/
#define sFLASH_CMD_RDID           0x9F  /* Read Identification */
#define sFLASH_CMD_RUID           0x4B  /* Read Unique ID */


/* SPI Interface pins  */

/*
 * Nucleo Pins
 * SCK: PA5
 * MISO: PA6
 * MOSI: PA7
 * CS: PB0
 */



/***********************************************************************************************************************
 * Prototypes
 **********************************************************************************************************************/

/**
* Initializes internal parameters.
* Should be called exactly once before anything is attempted to be done with this module.
* Failing to call this function, or calling it more than once, voids all guarantees made by this module.
* Notes for configuring SPI in IOC:
* - do not use NSS, use a dedicated GPIO for CS
* - make sure that "data size" is set to 8 bits
*/
void Flash_Init(void);

/**
* Stores collected logs in the flash
* @param[in]        DataBuffer          struct with data to be logged
* @return           RETROFRIGERATION_SUCCEEDED, RETROFRIGERATION_BUSY or RETROFRIGERATION_FAILED
*/
int Flash_LogData(const DataBuffer_t *DataBuffer);
int Flash_PassDataToUSB(void);

