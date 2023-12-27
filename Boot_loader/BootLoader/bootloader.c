#include "bootloader.h"

/* static function declearation */
static void Bootloader_Get_Version(uint8 *Host_Buffer);
static void Bootloader_Get_Help(uint8 *Host_Buffer);
static void Bootloader_Get_Chip_Identification_Number(uint8 *Host_Buffer);
static void Bootloader_Read_Protection_Level(uint8 *Host_Buffer);
static void Bootloader_Jump_To_Address(uint8 *Host_Buffer);
static void Bootloader_Erase_Flash(uint8 *Host_Buffer);
static void Bootloader_Memory_Write(uint8 *Host_Buffer);
static void Bootloader_Enable_RW_Protection(uint8 *Host_Buffer);
static void Bootloader_Memory_Read(uint8 *Host_Buffer);
static void Bootloader_Get_Sector_Protection_Status(uint8 *Host_Buffer);
static void Bootloader_Read_OTP(uint8 *Host_Buffer);
static void Bootloader_Disable_RW_Protection(uint8 *Host_Buffer);
static void Bootloader_Change_Read_Protection_Level(uint8 *Host_Buffer);
static void Jump_From_BL_To_APP(void);

static void BL_Sending_ACK(uint8_t Replay_Len);
static void BL_Sending_NACK(void);
static uint8_t CRC_Verification(uint8_t *ptr,uint32_t data_lentgh,uint32 host_crc);
static void Bootloader_Get_Version(uint8 *Host_Buffer);

uint8_t BL_Buffer[BL_SIZE_BUFER];


void BL_Print_Massege_tarek(uint8 counter,uint8 *ptr,...){

	if (NULL != ptr) {
        uint8_t *message = ptr;
        va_list list;
        
        va_start(list, ptr);
        for (int i = 0; i < counter; ++i) {
        HAL_UART_Transmit(BL_CHANNEL_3, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
            message = va_arg(list, uint8*);
        }
        va_end(list);
    }

}


void BL_Print_massege(char *format, ...){
	char Messsage[100] = {0};
	/* holds the information needed by va_start, va_arg, va_end */
	va_list args;
	/* Enables access to the variable arguments */
	va_start(args, format);
	/* Write formatted data from variable argument list to string */
	vsprintf(Messsage, format, args);
#if (BL_DEBUG_METHOD == BL_ENABLE_UART_DEBUG_MESSAGE)
	/* Trasmit the formatted data through the defined UART */
	HAL_UART_Transmit(BL_CHANNEL_2, (uint8_t *)Messsage, sizeof(Messsage), HAL_MAX_DELAY);
#elif (BL_DEBUG_METHOD == BL_ENABLE_SPI_DEBUG_MESSAGE)
	/* Trasmit the formatted data through the defined SPI */
#elif (BL_DEBUG_METHOD == BL_ENABLE_CAN_DEBUG_MESSAGE)
	/* Trasmit the formatted data through the defined CAN */
#endif
	/* Performs cleanup for an ap object initialized by a call to va_start */
	va_end(args);
}


BL_statuse BL_Featch_Command(void){
	
BL_statuse bl_statuse=BL_NACK;
HAL_StatusTypeDef return_statuse=HAL_ERROR;
uint8_t	Data_Length = 0;
memset(BL_Buffer,0,BL_SIZE_BUFER);
return_statuse=HAL_UART_Receive(BL_CHANNEL_3,BL_Buffer,FIRIST_BYTE_OF_BUFFER,HAL_MAX_DELAY);
	
	
	if(HAL_ERROR != return_statuse){
		Data_Length=BL_Buffer[0];
		return_statuse=	HAL_UART_Receive(BL_CHANNEL_3,&BL_Buffer[1],Data_Length,HAL_MAX_DELAY);
		if(HAL_ERROR !=return_statuse){
				switch(BL_Buffer[1]){
					case CBL_GET_VER_CMD :
						BL_Print_massege(" CBL_GET_VER_CMD \r\n");
					bl_statuse=BL_ACK;
					Bootloader_Get_Version(BL_Buffer);
					break;
				case CBL_GET_HELP_CMD :
						BL_Print_massege(" CBL_GET_HELP_CMD \r\n");
				bl_statuse=BL_ACK;
				Bootloader_Get_Help(BL_Buffer);
				Jump_From_BL_To_APP();
					break;
				case CBL_GET_CID_CMD :
						BL_Print_massege(" CBL_GET_CID_CMD \r\n");
				bl_statuse=BL_ACK;
				Bootloader_Get_Chip_Identification_Number(BL_Buffer);
					break;
				case CBL_GET_RDP_STATUS_CMD :
						BL_Print_massege(" CBL_GET_RDP_STATUS_CMD \r\n");
				bl_statuse=BL_ACK;
				Bootloader_Read_Protection_Level(BL_Buffer);
					break;
				case CBL_GO_TO_ADDR_CMD :
						BL_Print_massege(" CBL_GO_TO_ADDR_CMD \r\n");
				bl_statuse=BL_ACK;
				Bootloader_Jump_To_Address(BL_Buffer);
					break;
				case CBL_FLASH_ERASE_CMD :
						BL_Print_massege(" CBL_FLASH_ERASE_CMD \r\n");
				bl_statuse=BL_ACK;
				Bootloader_Erase_Flash(BL_Buffer);
					break;
				case CBL_MEM_WRITE_CMD :
						BL_Print_massege(" CBL_MEM_WRITE_CMD \r\n");
				bl_statuse=BL_ACK;
				Bootloader_Memory_Write(BL_Buffer);
					break;
				case CBL_ED_W_PROTECT_CMD:
						BL_Print_massege(" CBL_ED_W_PROTECT_CMD \r\n");
				bl_statuse=BL_ACK;
					break;
				case CBL_MEM_READ_CMD :
						BL_Print_massege(" CBL_MEM_READ_CMD \r\n");
				bl_statuse=BL_ACK;
					break;
				case CBL_READ_SECTOR_STATUS_CMD:
						BL_Print_massege(" CBL_READ_SECTOR_STATUS_CMD \r\n");
				bl_statuse=BL_ACK;
					break;
				case CBL_OTP_READ_CMD:
						BL_Print_massege(" CBL_OTP_READ_CMD \r\n");
				bl_statuse=BL_ACK;
					break;
				case CBL_CHANGE_ROP_Level_CMD:
						BL_Print_massege(" CBL_CHANGE_ROP_Level_CMD \r\n");
				bl_statuse=BL_ACK;
				Bootloader_Change_Read_Protection_Level(BL_Buffer);
					break;
				default:
					BL_Print_massege("Error choosing Command \r\n");
				bl_statuse=BL_NACK;
					break;
				}
		}
	}
	
	
	return bl_statuse;
}





static uint8_t CRC_Verification(uint8_t *ptr,uint32_t data_lentgh,uint32 host_crc){
uint8_t checking =CRC_NOT_CORRECT;
	uint32_t CRC_Calc=0;
	uint32_t buffer=0;
	for(uint8_t counter=0;counter<data_lentgh;counter++){
	buffer=(uint32_t)ptr[counter];
	CRC_Calc=HAL_CRC_Accumulate(BL_CRC,&buffer,1);
	}
	
	/* reset crc after calulating  */
	__HAL_CRC_DR_RESET(BL_CRC);
	
	if(host_crc == CRC_Calc){
		checking =CRC_CORRECT;
	}
	else{
	checking =CRC_NOT_CORRECT;
	}
	
	return checking;
}

static void BL_Sending_ACK(uint8_t Replay_Len){
uint8_t Ack_Value[2] = {0};
	Ack_Value[0] = BL_STATUSE_ACK;
	Ack_Value[1] = Replay_Len;
	HAL_UART_Transmit(BL_CHANNEL_3, (uint8_t *)Ack_Value, 2, HAL_MAX_DELAY);
}
static void BL_Sending_NACK(void){
uint8_t Ack_Value = BL_STATUSE_NACK;
	HAL_UART_Transmit(BL_CHANNEL_3, &Ack_Value, 1, HAL_MAX_DELAY);
}

static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len){
	HAL_UART_Transmit(BL_CHANNEL_3, Host_Buffer, Data_Len, HAL_MAX_DELAY);
}



static void Bootloader_Get_Version(uint8 *Host_Buffer){

		if(NULL!=Host_Buffer){
		uint8_t BL_Version[4] = { BL_VENDOR_ID, BL_MAJOR_VERSION, BL_MINIOR_VERSION, BL_PATCH_VERSION };
		/* Extract the CRC32 and packet length sent by the HOST */
	uint16_t Host_CMD_Packet_Len =  Host_Buffer[0] + 1;
  uint32_t Host_CRC32 = *((uint32_t *)((Host_Buffer+2)));
	
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("Read the bootloader version from the MCU \r\n");
#endif
	
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == CRC_Verification((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_massege("CRC Verification Passed \r\n");
#endif
		BL_Sending_ACK(4);
		Bootloader_Send_Data_To_Host((uint8_t *)(&BL_Version[0]), 4);
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
    BL_Print_massege("Bootloader Ver. %d.%d.%d \r\n", BL_Version[1], BL_Version[2], BL_Version[3]);
#endif  
			
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("CRC Verification Failed \r\n");
#endif
		BL_Sending_NACK();
	}
		}
		else{
		// do nuthing 
		}
	
}


static void Bootloader_Get_Help(uint8 *Host_Buffer){
if(NULL!=Host_Buffer){
	
	uint8_t BL_CMD_Commands[12]={
 CBL_GET_VER_CMD  ,
 CBL_GET_HELP_CMD          ,
 CBL_GET_CID_CMD           ,
/* Get Read Protection Status */
 CBL_GET_RDP_STATUS_CMD    ,
 CBL_GO_TO_ADDR_CMD        ,
 CBL_FLASH_ERASE_CMD       ,
 CBL_MEM_WRITE_CMD         ,
/* Enable/Disable Write Protection */
 CBL_ED_W_PROTECT_CMD      ,
 CBL_MEM_READ_CMD          ,
/* Get Sector Read/Write Protection Status */
 CBL_READ_SECTOR_STATUS_CMD,
 CBL_OTP_READ_CMD          ,
/* Change Read Out Protection Level */
 CBL_CHANGE_ROP_Level_CMD  ,
};
	
	/* Extract the CRC32 and packet length sent by the HOST */
	uint16_t Host_CMD_Packet_Len =  Host_Buffer[0] + 1;
  uint32_t Host_CRC32 = *((uint32_t *)((Host_Buffer+2)));
	
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("Read the bootloader version from the MCU \r\n");
#endif
	
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == CRC_Verification((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_massege("CRC Verification Passed \r\n");
#endif
		
		BL_Sending_ACK(12);
		Bootloader_Send_Data_To_Host((uint8_t *)(&BL_CMD_Commands[0]), 12);
 
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("CRC Verification Failed \r\n");
#endif
		BL_Sending_NACK();
	}
	}
}




static void Bootloader_Get_Chip_Identification_Number(uint8 *Host_Buffer){
if(NULL!=Host_Buffer){
	uint16_t ID_Code= DBGMCU->IDCODE & 0xfff;
	/* Extract the CRC32 and packet length sent by the HOST */
	uint16_t Host_CMD_Packet_Len =  Host_Buffer[0] + 1;
  uint32_t Host_CRC32 = *((uint32_t *)((Host_Buffer+2)));
	
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("Read the bootloader version from the MCU \r\n");
#endif
	
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == CRC_Verification((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_massege("CRC Verification Passed \r\n");
#endif
		
		BL_Sending_ACK(2);
		Bootloader_Send_Data_To_Host((uint8_t *)(&ID_Code), 2);
 
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("CRC Verification Failed \r\n");
#endif
		BL_Sending_NACK();
	}
	}
}




static void Bootloader_Read_Protection_Level(uint8 *Host_Buffer){
if(NULL!=Host_Buffer){
		/* Extract the CRC32 and packet length sent by the HOST */
	uint16_t Host_CMD_Packet_Len =  Host_Buffer[0] + 1;
  uint32_t Host_CRC32 = *((uint32_t *)((Host_Buffer+2)));
	
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("Bootloader_Read_Protection_Level from the MCU \r\n");
#endif
	
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == CRC_Verification((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_massege("CRC Verification Passed \r\n");
#endif
		BL_Sending_ACK(1);
		uint8_t read_Protection_Number=0;
		FLASH_OBProgramInitTypeDef FLASH_OBProgram={0};
		HAL_FLASHEx_OBGetConfig(&FLASH_OBProgram);
		read_Protection_Number=(uint8_t)(FLASH_OBProgram.RDPLevel);
		Bootloader_Send_Data_To_Host(&read_Protection_Number, 1);
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
    BL_Print_massege("Bootloader Protection_Level is =%d \r\n",read_Protection_Number);
#endif  
			
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("CRC Verification Failed \r\n");
#endif
		BL_Sending_NACK();
	}
	}
}




static uint8_t Adress_Checking(uint32_t adress){
uint8_t statuse_adress=ADRESS_NOT_VALED;
	if((adress >=FLASH_BASE) && (adress<=FLASH_END_) ){
	 statuse_adress=ADRESS_VALED;
	}
	else if((adress >=SRAM2_BASE) && (adress<=SRAM2_END_)){
	 statuse_adress=ADRESS_VALED;
	}
		else if((adress >=SRAM1_BASE) && (adress<=SRAM1_END_)){
	 statuse_adress=ADRESS_VALED;
	}
			else if((adress >=CCMDATARAM_BASE) && (adress<=CCMDATARAM_END_)){
	 statuse_adress=ADRESS_VALED;
	}
			else{
			 statuse_adress=ADRESS_NOT_VALED;
			}
	return statuse_adress ;
}
static void Bootloader_Jump_To_Address(uint8 *Host_Buffer){
if(NULL!=Host_Buffer){
	/* Extract the CRC32 and packet length sent by the HOST */
	uint16_t Host_CMD_Packet_Len =  Host_Buffer[0] + 1;
  uint32_t Host_CRC32 = *((uint32_t *)((Host_Buffer+Host_CMD_Packet_Len)-4));
	
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("Bootloader_Jump_To_Address in the MCU \r\n");
#endif
	
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == CRC_Verification((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_massege("CRC Verification Passed \r\n");
#endif
		BL_Sending_ACK(1);
		uint32_t adress=*((uint32_t *)&Host_Buffer[2]);
		uint8_t statuse_adress=ADRESS_NOT_VALED;
		statuse_adress=Adress_Checking(adress);
		if(ADRESS_VALED==statuse_adress){
			#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("Adress Verification Sucsess \r\n");
#endif
		Ptr_Fun Ptr= (Ptr_Fun) (adress +1);
		Bootloader_Send_Data_To_Host(&statuse_adress, 1);
			Ptr();
		}
		else{
			#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("Adress Verification Failed \r\n");
#endif
		Bootloader_Send_Data_To_Host(&statuse_adress, 1);
		}

	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("CRC Verification Failed \r\n");
#endif
		BL_Sending_NACK();
	}
	}
}

static uint8_t Sector_Flash_Checking(uint8_t sector_number,uint8_t number_of_sectors){
uint8_t sector_statuse=SECTOR_NOT_VALID;
	
	if(FLASH_MASS_ERASE==sector_number){
	sector_statuse=FLASH_MASS_ERASE;
	}
	else{
	 if(((sector_number>=SECTOR_MIN) && (sector_number<=SECTOR_MAX)) && (((sector_number+number_of_sectors)>=SECTOR_MIN)&& ((sector_number+number_of_sectors)<=SECTOR_MAX))){
	sector_statuse=SECTOR_VALID;
	}
else{
uint8_t sector_statuse=SECTOR_NOT_VALID;
}
	}
	
return sector_statuse;
}

static void Bootloader_Erase_Flash(uint8 *Host_Buffer){
if(NULL!=Host_Buffer){
		/* Extract the CRC32 and packet length sent by the HOST */
	uint16_t Host_CMD_Packet_Len =  Host_Buffer[0] + 1;
  uint32_t Host_CRC32 = *((uint32_t *)((Host_Buffer+Host_CMD_Packet_Len)-4));
	
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("Read the bootloader version from the MCU \r\n");
#endif
	
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == CRC_Verification((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_massege("CRC Verification Passed \r\n");
#endif
		BL_Sending_ACK(1);
		uint8_t sector_statuse=SECTOR_NOT_VALID;
		sector_statuse=Sector_Flash_Checking(Host_Buffer[2],Host_Buffer[3]);
		Bootloader_Send_Data_To_Host(&sector_statuse, 1);
		
		if(FLASH_MASS_ERASE==sector_statuse){
		#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
    BL_Print_massege("Bootloader Erase_Flash all  \r\n");
			#endif 
			/* Erase_Flash all */
			uint32_t SectorError=0;
			uint8_t Erase_statuse=0;
			FLASH_EraseInitTypeDef pEraseInit={
			.TypeErase=FLASH_TYPEERASE_MASSERASE,
				.Banks=FLASH_BANK_1,
				.VoltageRange=FLASH_VOLTAGE_RANGE_3
				
			};
			HAL_FLASH_Unlock();
		Erase_statuse=HAL_FLASHEx_Erase(&pEraseInit,&SectorError);
			HAL_FLASH_Lock();
					#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
       BL_Print_massege("Bootloader Erase_Flash %d Erase_statuse = %d \r\n",SectorError,Erase_statuse);
#endif  
		}
		else if(SECTOR_VALID==sector_statuse){
		#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
       BL_Print_massege("Bootloader Erase_Flash accepted specefic all  \r\n");
#endif  
			/* specefic Erase_Flash  */			
			uint32_t SectorError=0;
			uint8_t Erase_statuse=0;
			FLASH_EraseInitTypeDef pEraseInit={
			.TypeErase=FLASH_TYPEERASE_SECTORS,
				.Sector=Host_Buffer[2],
				.NbSectors=Host_Buffer[3],
				.VoltageRange=FLASH_VOLTAGE_RANGE_3
				
			};
			HAL_FLASH_Unlock();
		Erase_statuse=	HAL_FLASHEx_Erase(&pEraseInit,&SectorError);			
			HAL_FLASH_Lock();
								#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
       BL_Print_massege("Bootloader Erase_Flash %d Erase_statuse = %d \r\n",SectorError,Erase_statuse);
#endif 
		}
		else{
		#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
        BL_Print_massege("Bootloader Erase_Flash Declined  \r\n");
#endif  
		}

		
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("CRC Verification Failed \r\n");
#endif
		BL_Sending_NACK();
	}
	}
}







/* Helper function to Bootloader_Memory_Write */
static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_Payload,uint32_t Payload_start_adress,uint8_t Payload_Length){
uint8_t Memory_Write_Payload_Statuse=MEMORY_WRITE_NOT_SYCSESS;
HAL_StatusTypeDef Function_Statuse=HAL_ERROR;
	Function_Statuse=HAL_FLASH_Unlock();
	/* unlocking statuse */
	if(HAL_OK==Function_Statuse){
		#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_massege("FLASH Unlock Success \r\n");
#endif
	for(uint8_t counter=0;counter<Payload_Length;counter++){
		
	Function_Statuse=HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,Payload_start_adress+counter,Host_Payload[counter]);
	/* writing statuse */
		if(HAL_OK!=Function_Statuse){
			#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_massege("Writng has been errored \r\n");
#endif
		Memory_Write_Payload_Statuse=MEMORY_WRITE_NOT_SYCSESS;
		break;
	}
		/* else of writing statuse */
	else{

		Memory_Write_Payload_Statuse=MEMORY_WRITE_SYCSESS;
	}
	}
		if(MEMORY_WRITE_SYCSESS ==Memory_Write_Payload_Statuse){
		HAL_FLASH_Lock();
		}
		else{
		
		}
	}
		/* else of unlocking statuse */
	else{
	Memory_Write_Payload_Statuse=MEMORY_WRITE_NOT_SYCSESS;
	}
	
	return Memory_Write_Payload_Statuse;
}


static void Bootloader_Memory_Write(uint8 *Host_Buffer){
if(NULL!=Host_Buffer){
	/* Extract the CRC32 and packet length sent by the HOST */
	uint16_t Host_CMD_Packet_Len =  Host_Buffer[0] + 1;
  uint32_t Host_CRC32 = *((uint32_t *)((Host_Buffer+Host_CMD_Packet_Len)-4));
	
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("Bootloader_Memory_Write \r\n");
#endif
	
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == CRC_Verification((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_massege("CRC Verification Passed \r\n");
#endif
		BL_Sending_ACK(1);
uint32_t Host_Adress=*((uint32_t *)(&Host_Buffer[2]));
	uint8_t Payload_Length=Host_Buffer[6];	
		uint8_t statuse_adress=ADRESS_NOT_VALED;
		uint8_t Memory_Write_Payload_Statuse=MEMORY_WRITE_NOT_SYCSESS;
	statuse_adress=Adress_Checking(Host_Adress);
	
		if(ADRESS_VALED==statuse_adress){
			#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("Adress is Correct \r\n");
#endif
Memory_Write_Payload_Statuse=	Flash_Memory_Write_Payload((uint8_t *)&Host_Buffer[7],Host_Adress,Payload_Length);
		if(MEMORY_WRITE_SYCSESS==Memory_Write_Payload_Statuse){
	#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("Final Writng has been Success \r\n");
#endif
			Bootloader_Send_Data_To_Host(&Memory_Write_Payload_Statuse, 1);
		
		}
		else{
	#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("Final Writng has been errored \r\n");
#endif
			
		Bootloader_Send_Data_To_Host(&Memory_Write_Payload_Statuse, 1);
		}
		}
		else{
	#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("Adress is not Correct \r\n");
#endif
				Bootloader_Send_Data_To_Host(&statuse_adress, 1);
		}
	}
	
	/* else of CRC checking */
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("CRC Verification Failed \r\n");
#endif
		BL_Sending_NACK();
	}
	}

}






static uint8_t Change_RP(uint32_t Read_Protection_Level){
HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	FLASH_OBProgramInitTypeDef FLASH_OBProgramInit;
	uint8_t ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
	
	/* Unlock the FLASH Option Control Registers access */
	HAL_Status = HAL_FLASH_OB_Unlock();
	if(HAL_Status != HAL_OK){
		ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("Failed -> Unlock the FLASH Option Control Registers access \r\n");
#endif
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_massege("Passed -> Unlock the FLASH Option Control Registers access \r\n");
#endif
		FLASH_OBProgramInit.OptionType = OPTIONBYTE_RDP; /* RDP option byte configuration */
		FLASH_OBProgramInit.Banks = FLASH_BANK_1;
		FLASH_OBProgramInit.RDPLevel = Read_Protection_Level;
		/* Program option bytes */
		HAL_Status = HAL_FLASHEx_OBProgram(&FLASH_OBProgramInit);
		if(HAL_Status != HAL_OK){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_massege("Failed -> Program option bytes \r\n");
#endif
			HAL_Status = HAL_FLASH_OB_Lock();
			ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
		}
		else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_massege("Passed -> Program option bytes \r\n");
#endif
			/* Launch the option byte loading */
			HAL_Status = HAL_FLASH_OB_Launch();
			if(HAL_Status != HAL_OK){
				ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
			}
			else{
				/* Lock the FLASH Option Control Registers access */
				HAL_Status = HAL_FLASH_OB_Lock();
				if(HAL_Status != HAL_OK){
					ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
				}
				else{
					ROP_Level_Status = ROP_LEVEL_CHANGE_VALID;
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
					BL_Print_massege("Passed -> Program ROP to Level : 0x%X \r\n", Read_Protection_Level);
#endif
				}
			}
		}
	}
	return ROP_Level_Status;
}

static void Bootloader_Change_Read_Protection_Level(uint8 *Host_Buffer){
if(NULL!=Host_Buffer){
		/* Extract the CRC32 and packet length sent by the HOST */
	uint16_t Host_CMD_Packet_Len =  Host_Buffer[0] + 1;
  uint32_t Host_CRC32 = *((uint32_t *)((Host_Buffer+Host_CMD_Packet_Len)-4));
	
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("Read the bootloader version from the MCU \r\n");
#endif
	
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == CRC_Verification((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_massege("CRC Verification Passed \r\n");
#endif
		BL_Sending_ACK(1);
		uint8_t RP_Changing_Statuse=RP_CHANGING_FAILED;
		uint8_t protection_level=Host_Buffer[2];
		if(2==protection_level){
		// do nothing for now 
		}
		else if(1==protection_level){
		protection_level=0x55;
		}
		else if(0==protection_level){
		protection_level=0xAA;
		}
		else{
		
		}
		RP_Changing_Statuse=Change_RP(protection_level);
		if(RP_CHANGING_SUCSSES==RP_Changing_Statuse){
		#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
    BL_Print_massege("Bootloader Succsess to change protection \r\n");
#endif 
		Bootloader_Send_Data_To_Host((uint8_t *)&RP_Changing_Statuse, 1);
		}
		else{
	#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
    BL_Print_massege("Bootloader Failed to change protection \r\n");
#endif 
		Bootloader_Send_Data_To_Host((uint8_t *)&RP_Changing_Statuse, 1);
		}
		
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
    BL_Print_massege("Bootloader  \r\n");
#endif  
			
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_massege("CRC Verification Failed \r\n");
#endif
		BL_Sending_NACK();
	}
		}
		else{
		// do nuthing 
		}
}
static void Bootloader_Enable_RW_Protection(uint8 *Host_Buffer){
if(NULL!=Host_Buffer){
	
	}
}


static void Bootloader_Memory_Read(uint8 *Host_Buffer){
if(NULL!=Host_Buffer){
	
	}
}


static void Bootloader_Get_Sector_Protection_Status(uint8 *Host_Buffer){
if(NULL!=Host_Buffer){
	
	}
}


static void Bootloader_Read_OTP(uint8 *Host_Buffer){
if(NULL!=Host_Buffer){
	
	}
}


static void Bootloader_Disable_RW_Protection(uint8 *Host_Buffer){
if(NULL!=Host_Buffer){
	
	}
}


static void Jump_From_BL_To_APP(void){

			
		uint32_t	MSP_Value	=*((volatile uint32_t *)SECTOR2_BASE_ADRESS_APP);
	uint32_t	Reset_Handler_Adress_value=*((volatile uint32_t *)(SECTOR2_BASE_ADRESS_APP+4));
	Ptr_Fun	Ptr_Reset_handler= (Ptr_Fun)Reset_Handler_Adress_value;
	
	__set_MSP(MSP_Value);
	HAL_RCC_DeInit();
	Ptr_Reset_handler();
}
