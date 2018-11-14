/*
 * common.c
 *
 *  Created on: Nov 14, 2018
 *      Author: bitohw03
 */

#include <stdarg.h>  //for va_list ap
#include <stdio.h>   //for printf
#include <string.h>  //for strlen
#include <stdlib.h>  //for atoi
#include <math.h>
#include "common.h"

static uint16_t _com_int_pow(uint8_t m, uint8_t n)
{
	uint8_t i = 0;
	uint16_t result = 1;

	for ( i = 0; i < n; i++)
	{
		result *= m;
	}
	return result;
}

int16_t com_str_to_int16(char *str)
{
	char *ptr = str,tmp[5];
	uint8_t  i = 0, len = 0;
	int16_t  temp = 0;
	bool IsHex = false;

	memset(tmp,'\0',5);

	if ((strchr(str,'x') != NULL) || (strchr(str,'X') != NULL))
	{
		IsHex = true;
		ptr += 2;
		for (i = 0; i < 4 ; i++)
		{
			tmp[i] = *ptr;
			ptr++;
		}
		ptr = tmp;
	}
	else
	{
		ptr = str;
	}

	if (IsHex)
	{
		len = strlen(ptr);
		for (i = 0; i < len; i++)
		{
			if (*ptr >= '0' && *ptr <= '9')
			{
				temp += (*ptr-'0') * _com_int_pow(16,len-1-i);
			}
			else if (*ptr >= 'a' && *ptr <= 'f')
			{
				temp += (*ptr-'a'+ 10) * _com_int_pow(16,len-1-i);
			}
			else if (*ptr >= 'A' && *ptr <= 'F')
			{
				temp += (*ptr-'A' + 10) * _com_int_pow(16,len-1-i);
			}
			ptr++;
		}
	}
	else
	{
		temp = (int16_t)atoi(ptr);
	}

	return temp;
}




