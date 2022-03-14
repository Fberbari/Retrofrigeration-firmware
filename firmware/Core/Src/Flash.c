#pragma once

#include "Flash.h"


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




uint8_t sFLASH_ReadByte(void);
uint8_t sFLASH_SendByte(uint8_t byte);
uint16_t sFLASH_SendHalfWord(uint16_t HalfWord);
void sFLASH_WriteEnable(void);
void sFLASH_WaitForWriteEnd(void);

void sFLASH_Init(void);
void sFLASH_EraseSector(uint32_t SectorAddr);
void sFLASH_EraseBulk(void);
void sFLASH_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void sFLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void sFLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
uint32_t sFLASH_ReadID(void);
void sFLASH_StartReadSequence(uint32_t ReadAddr);


/***********************************************************************************************************************
 * Code
 **********************************************************************************************************************/

void Flash_Init(void)
{
	return;
}

int Flash_LogData(const DataBuffer_t *DataBuffer)
{
    return RETROFRIGERATION_SUCCEEDED;
}

int Flash_PassDataToUSB(void)
{
	return;
}
