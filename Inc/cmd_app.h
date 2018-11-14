/*
 * cmd_app.h
 *
 *  Created on: Nov 14, 2018
 *      Author: bitohw03
 */

#ifndef INC_CMD_APP_H_
#define INC_CMD_APP_H_

typedef enum
{
	 READ_CMD,
	 WRITE_CMD,
	 UNKNOW_CMD,
}CMD_RW_TYPE_T;

typedef struct {
	uint8_t       *readData;
	uint8_t       *writeData;
	uint8_t        rwByteNum;
	uint8_t        devAddr;
	uint8_t        i2c_dbg_mode;  //0:normal i2c mode,1:dbg i2c mode
	uint16_t  	   offsetAddr;
	CMD_RW_TYPE_T  rwType;
}I2C_DATA_T;

void cmd_add_Init(void);


#endif /* INC_CMD_APP_H_ */
