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
#include "cmd_app.h"
#include "config.h"


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
			result = HAL_I2C_Mem_Read(&hi2c1, i2c_data.devAddr, i2c_data.offsetAddr, I2C_MEMADD_SIZE_8BIT, i2c_data.readData, i2c_data.rwByteNum,1000);
	}
	else
	{
			result = HAL_I2C_Mem_Write(&hi2c1, i2c_data.devAddr, i2c_data.offsetAddr, I2C_MEMADD_SIZE_8BIT, i2c_data.readData, i2c_data.rwByteNum,1000);
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
/*
 *
 * //获取 X/Y/Z 磁力计值
extern void getmag(void);

//读取DMP FIFO
extern void readdmp(void);

//获取加速度、角速度
extern void getAcc_gyro(void);
 */
static HAL_StatusTypeDef prvDispmpu9250Config(int argc, char** argv)
{
	uint8_t result = HAL_ERROR;
	if(!strcmp(argv[1],"init"))
	{
		Init_MPU9250_With_DMP();//初始化MPU9250
	}
	else if(!strcmp(argv[1],"getmag"))
	{
		getmag();
		printf("\r\n mag_x: %d mag_y: %d mag_z: %d ",(int)(mag[0]),(int)(mag[1]),(int)(mag[2]));
	}
	else if(!strcmp(argv[1],"getAcc_gyro"))
	{
		getAcc_gyro();
		printf("\r\n acc_x: %d acc_y: %d acc_z: %d",(int)(accelf[0]*100),(int)(accelf[1]*100),(int)(accelf[2]*100));
		printf("\r\n gyro_x: %d gyro_y: %d gyro_z: %d",(int)(gyrof[0]*100),(int)(gyrof[1]*100),(int)(gyrof[2]*100));
	}
	else if(!strcmp(argv[1],"pose"))
	{
		uint16_t fifo_count = 0;
		while(1)//使用循环是为了等待DMP转换完成
		{
			fifo_count = MPU9250_getFIFOCount();//读取FIFO计数

			if(fifo_count >= 0x0200)//如果FIFO值>0x0200，此时DMP的结果错误，直接复位FIFO
			{
				MPU9250_resetFIFO();
			}
			else
			{

				if(fifo_count >= 0x2a)//如果FIFO值 > 0x2a,此时DMP转换完成并且数值正常
				{
					readdmp(); //首先要读取DMP FIFO，读取之后才能进行计算姿态的操作
					MPU9250_resetFIFO();
					getyawpitchroll();//计算并且获取yaw、pitch、roll，结果保存在yprf[3]数组中
					printf("\r\n yaw : %f pitch : %f  roll : %f",(yprf[0]),(yprf[1]),(yprf[2]));
					break;//转换并且计算完成之后退出循环
				}
			}
		}
	}
	return result;
}

void cmd_add_Init(void)
{
	CLICommandAdd("[opr|opw] &baseAddr &OffAddr #ParmNum(0~0xff)" \
					  ,prvDispOprwConfig);
	CLICommandAdd("mpu9250 [init|getmag|getAcc_gyro|pose]" \
					  ,prvDispmpu9250Config);
}

