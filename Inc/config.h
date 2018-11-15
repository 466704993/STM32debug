/*
 * config.h
 *
 *  Created on: Nov 15, 2018
 *      Author: bitohw03
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_



#include "stm32f1xx_hal.h"
#include "cli.h"
#include "mpu9250.h"
#include "cmd_app.h"


#define Bit char
typedef enum {false = 0,true = 1} bool;

extern I2C_HandleTypeDef hi2c1;


#endif /* INC_CONFIG_H_ */
