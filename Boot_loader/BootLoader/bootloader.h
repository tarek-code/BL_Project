#ifndef __STM32F4xx_BOOT_LOADER_H
#define __STM32F4xx_BOOT_LOADER_H



// ----------------Includes-----------------------
#include "usart.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "std_types.h"
#include "crc.h"
// ----------------Includes end-----------------------


// ---------------macro---------------------------
#define SECTOR2_BASE_ADRESS_APP			0x08008000U
typedef void(*Ptr_Fun)(void);
#define BL_CHANNEL_2  &huart2
#define BL_CHANNEL_3  &huart3
#define BL_CRC				&hcrc

#define BL_SIZE_BUFER 200
#define FIRIST_BYTE_OF_BUFFER  (1)

#define BL_USING_USART		(0x00)
#define BL_USING_SPI			(0x01)
#define BL_USING_CAN			(0x02)

#define CRC_CORRECT						(0x01)
#define CRC_NOT_CORRECT				(0x00)

#define ADRESS_VALED				(1)
#define ADRESS_NOT_VALED				(0)



#define CCMDATARAM_SIZE		(64*1024)
#define SRAM1_SIZE				(112*1024)
#define SRAM2_SIZE				(16*1024)
#define FLASH_SIZE				(1024*1024)

#define CCMDATARAM_END_		(CCMDATARAM_BASE+CCMDATARAM_SIZE)
#define SRAM1_END_				(SRAM1_BASE+SRAM1_SIZE)
#define SRAM2_END_				(SRAM2_BASE+SRAM2_SIZE)
#define FLASH_END_				(FLASH_BASE+FLASH_SIZE)


#define FLASH_MASS_ERASE	(0xFF)
#define	SECTOR_MIN				(0)
#define SECTOR_MAX				(11)
#define SECTOR_VALID		(1)
#define SECTOR_NOT_VALID		(2)

/*Memory Write Statuse*/
#define MEMORY_WRITE_SYCSESS		(1)
#define MEMORY_WRITE_NOT_SYCSESS		(0)

#define RP_CHANGING_SUCSSES		(1)
#define RP_CHANGING_FAILED		(0)
#define ROP_LEVEL_CHANGE_INVALID     0x00
#define ROP_LEVEL_CHANGE_VALID       0X01

#define CRC_VERIFICATION_FAILED      0x00
#define CRC_VERIFICATION_PASSED      0x01

#define BL_VENDOR_ID			(100)
#define BL_MAJOR_VERSION	(1)
#define BL_MINIOR_VERSION	(1)
#define BL_PATCH_VERSION	(0)

#define BL_STATUSE_ACK		(0xCD)
#define BL_STATUSE_NACK		(0xAB)

#define CRC_TYPE_SIZE_BYTE           4

#define BL_METHODE_USING		(BL_USING_USART)

#define CBL_GET_VER_CMD              0x10
#define CBL_GET_HELP_CMD             0x11
#define CBL_GET_CID_CMD              0x12
/* Get Read Protection Status */
#define CBL_GET_RDP_STATUS_CMD       0x13
#define CBL_GO_TO_ADDR_CMD           0x14
#define CBL_FLASH_ERASE_CMD          0x15
#define CBL_MEM_WRITE_CMD            0x16
/* Enable/Disable Write Protection */
#define CBL_ED_W_PROTECT_CMD         0x17
#define CBL_MEM_READ_CMD             0x18
/* Get Sector Read/Write Protection Status */
#define CBL_READ_SECTOR_STATUS_CMD   0x19
#define CBL_OTP_READ_CMD             0x20
/* Change Read Out Protection Level */
#define CBL_CHANGE_ROP_Level_CMD     0x21


// ---------------macro end---------------------------


//---------------macro function-------------------

//---------------macro function end-------------------


//----------------Data type-----------------------

typedef enum{
BL_NACK=0,
	BL_ACK
}BL_statuse;

//----------------Data type end-----------------------


//----------------Interfaces---------------------

void BL_Print_massege(char *format, ...);
void BL_Print_Massege_tarek(uint8 counter,uint8 *ptr,...);
BL_statuse BL_Featch_Command(void);



//----------------Interfaces end---------------------



#endif /* __STM32F4xx_BOOT_LOADER_H */