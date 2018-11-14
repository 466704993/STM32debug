/*
 * cmd_app.c
 *
 *  Created on: Nov 14, 2018
 *      Author: bitohw03
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stm32f1xx_hal.h"
#include "common.h"
//#include "cli.c"
#include "cmd_app.h"



static HAL_StatusTypeDef prvDispOprwConfig(int argc, char** argv)
{
	I2C_DATA_T i2c_data;
	uint8_t databuff[256];
	uint8_t result = HAL_ERROR;

	if (!strcmp(argv[0],"opw") )
	{
		i2c_data.rwType = WRITE_CMD;
	}
	else if (!strcmp(argv[0],"opr") )
	{
		i2c_data.rwType = READ_CMD;
	}

	i2c_data.devAddr 	= com_str_to_int16(argv[1]);
	i2c_data.offsetAddr = com_str_to_int16(argv[2]);
	i2c_data.readData   = databuff;
	i2c_data.writeData  = databuff;
	i2c_data.i2c_dbg_mode = false;  //false:normal i2c mode,true:dbg i2c mode

	if(i2c_data.rwType == READ_CMD)
	{
		if(argc == 3)
		{
			i2c_data.rwByteNum = 1;
		}
		else
		{
			i2c_data.rwByteNum = com_str_to_int16(argv[3]);
		}
	}
	else
	{
		uint8_t i = 0, index = 0;

		if(argc == 0x4)
		{
			i2c_data.rwByteNum = argc - 3;
			index = 3;

		}
		else if(argc > 0x4)
		{
			i2c_data.rwByteNum = argc - 4;
			index = 4;
		}

		for(i= 0; i < i2c_data.rwByteNum;i++ )
		{
			databuff[i] = com_str_to_int16(argv[index + i]);
		}
	}

	if(i2c_data.rwType == READ_CMD)
	{
	//		result = smb_read_multi_byte( i2c_data.devAddr, i2c_data.offsetAddr,  i2c_data.readData, i2c_data.rwByteNum);
	}
	else
	{
	//		result = smb_write_multi_byte( i2c_data.devAddr, i2c_data.offsetAddr,  i2c_data.readData, i2c_data.rwByteNum);
	}

	if(result == HAL_OK)
	{
		printf(" \r\n The value on page (0x%x), index(0x%x) : ",i2c_data.devAddr,i2c_data.offsetAddr);
		while(i2c_data.rwByteNum --)
		{
			printf(" 0x%x",*(i2c_data.readData++));
		}
	}
	return result;
}

void cmd_add_Init(void)
{
//	CLICommandAdd("[opr|opw] &baseAddr &OffAddr #ParmNum(0~0xff)" \
//					  ,prvDispOprwConfig);
}

