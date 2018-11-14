/*
 * common.h
 *
 *  Created on: Nov 14, 2018
 *      Author: bitohw03
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#include "stm32f1xx_hal.h"

#define bit unsigned char
typedef enum {false = 0,true = 1} bool;

int16_t com_str_to_int16(char *str);

#endif /* INC_COMMON_H_ */
