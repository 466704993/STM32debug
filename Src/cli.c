/*
 * cli.c
 *
 *  Created on: Nov 14, 2018
 *      Author: bitohw03
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>  //for va_list ap
#include "cli.h"
#include "common.h"
#include "config.h"

#define UART_BUFF_LEN 100

static Bit label = false;
GET_CMD_PARAM_INFO_T gCmdParamInfo;

struct
{
	char 			cHandleBuf[UART_BUFF_LEN];
	char 			*read_ptr;     //read pointer
	char 			*write_ptr;	   //write pointer
}gUsbParamInfo;

struct
{
	CLI_Command_List_t command[100];
	CLI_Command_List_t parameter[100];
}clilist;

static void vOutputString(char *str)
{
	printf("%s",str);
}
static void vOutputChar(char ch)
{
	printf("%c",ch);
}
char vGetchar(uint16_t time)
{
		return read_cInput(time);
}
void prvUARTCommandConsoleTask(void const *pvParameters)
{
	CLI_info_init();  //CLI ��ʼ��
	cmd_add_Init();
	while(1)
	{
		CLI_event_handle(); //CLI�ַ�����poll
//		vTaskDelay(30);
	}
}

static void cli_deinit() //�����ն�
{
	gCmdParamInfo.cmdBufTail   = gCmdParamInfo.cmdBuf;
	gCmdParamInfo.cmdBufCursor = gCmdParamInfo.cmdBuf;
	gCmdParamInfo.cmdBuf[0] ='\0';
}

static char *CLIGetParameter(char *str,uint8_t num)  //�ȶԵ����ַ�����ʧ�ܷ���NULL
{
	char *dst = str;
	uint8_t i = 0;
	while(*dst == ' ') dst++;
	if(num == 0 && *dst)
	{
		return dst;
	}
	while(*dst)
	{
		if(*dst == '[')
		{
			dst = strchr(dst, ']') + 1;
			if(dst == NULL)
			{
				vOutputString("\r\n Instruction standard error, please contact software processing!");
				return NULL;
			}
		}
		else if(*dst++ == ' ')
		{
			while(*dst == ' ') dst++;
			i++;
			if(i == num && *dst)
			{
				return dst;
			}
		}
	}
	return NULL;
}

static void cli_format_cmd(char *strtop,char **parameter)  //�ָ��ַ���
{
	char *strend;

	gCmdParamInfo.argc = 0;
	gCmdParamInfo.argv[0] = gCmdParamInfo.argvBuf;
	do
	{
		strend = strchr(strtop,' ');
		if(strend == NULL)
		{
			if(strncmp(strtop,"-all",4))
			{
				strcpy(gCmdParamInfo.argv[gCmdParamInfo.argc],strtop);
				*(gCmdParamInfo.argv[gCmdParamInfo.argc ++] + strlen(strtop)) = '\0';
				gCmdParamInfo.argv[gCmdParamInfo.argc] = gCmdParamInfo.argv[gCmdParamInfo.argc - 1] + strlen(strtop) + 1;
			}
			else
			{
				*parameter = strtop + 1;
				parameter ++;
			}
		}
		else
		{
			if(strncmp(strtop,"-all",4))
			{
				strncpy(gCmdParamInfo.argv[gCmdParamInfo.argc],strtop,strend - strtop);
				*(gCmdParamInfo.argv[gCmdParamInfo.argc ++] + (strend - strtop)) = '\0';
				gCmdParamInfo.argv[gCmdParamInfo.argc] = gCmdParamInfo.argv[gCmdParamInfo.argc - 1] + (strend - strtop) + 1;
			}
			else
			{
				*parameter = strtop + 1;
				parameter ++;
			}
			strtop = strend + 1;
		}
	}
	while(strend);
	gCmdParamInfo.argv[gCmdParamInfo.argc][0] = '\0';
}

static void cli_write_history_cmd(char *str)  //�����ʷ����
{
	static uint8_t i = 0;

	if(strcmp(gCmdParamInfo.historyCmd.cmdBuf[i], str))
	{
		if(gCmdParamInfo.historyCmd.cmdBuf[i][0] != 0)
		{
			if(++i >= HISTORY_CMD_NUM)
			{
				i = 0;
			}
		}
		strcpy(gCmdParamInfo.historyCmd.cmdBuf[i],str);
	}
	gCmdParamInfo.historyCmd.wIndex = i;
}

static void cli_read_history_cmd(uint8_t way) //����ʷ����
{
	int i;

	way = 0;

	while(gCmdParamInfo.historyCmd.cmdBuf[gCmdParamInfo.historyCmd.wIndex][0] == 0 && gCmdParamInfo.historyCmd.wIndex != 0)
	{
		--gCmdParamInfo.historyCmd.wIndex;
	}

	if(gCmdParamInfo.historyCmd.cmdBuf[gCmdParamInfo.historyCmd.wIndex][0] != 0)
	{
		for(i = strlen(gCmdParamInfo.cmdBufCursor); i > 0; i--)
		{
			vOutputChar(' ');
		}

		for(i = strlen(gCmdParamInfo.cmdBuf); i > 0; i--)
		{
			vOutputString("\b \b");
		}

		strcpy(gCmdParamInfo.cmdBuf, &gCmdParamInfo.historyCmd.cmdBuf[gCmdParamInfo.historyCmd.wIndex][0]);
		gCmdParamInfo.cmdBufCursor = gCmdParamInfo.cmdBuf + strlen(&gCmdParamInfo.historyCmd.cmdBuf[gCmdParamInfo.historyCmd.wIndex][0]);
		gCmdParamInfo.cmdBufTail   = gCmdParamInfo.cmdBufCursor;
		vOutputString(gCmdParamInfo.cmdBuf);

		if(gCmdParamInfo.historyCmd.wIndex -- == 0)
		{
			gCmdParamInfo.historyCmd.wIndex = HISTORY_CMD_NUM - 1;
		}
	}
}
static void cli_cInput(char ReadChar)
{
	if(ReadChar == ' ' && (*gCmdParamInfo.cmdBufCursor == ' '|| \
							(gCmdParamInfo.cmdBufCursor == gCmdParamInfo.cmdBuf) || \
							(*(gCmdParamInfo.cmdBufCursor - 1) == ' ')))
	{
		return;
	}
	else
	{
		char str[CMD_MAX_LENG];
		uint16_t i;
		strcpy(str, gCmdParamInfo.cmdBufCursor);
		memcpy((gCmdParamInfo.cmdBufCursor + 1),str,strlen(str) + 1);
		*gCmdParamInfo.cmdBufCursor = ReadChar;
		vOutputChar(ReadChar);
		vOutputString(str);
		for(i = strlen(str) ; i > 0 ; i-- )
		{
			vOutputChar('\b');
		}
		gCmdParamInfo.cmdBufCursor ++;
		gCmdParamInfo.cmdBufTail ++;
	}
}
static void cli_backspace(void)
{
	char str[CMD_MAX_LENG];
	uint16_t i;

	if(gCmdParamInfo.cmdBufCursor != gCmdParamInfo.cmdBuf)
	{
		do
		{
			if(gCmdParamInfo.cmdBufCursor == gCmdParamInfo.cmdBuf)
			{
				vOutputString(" ");
				gCmdParamInfo.cmdBufCursor ++;
			}
			strcpy(str, gCmdParamInfo.cmdBufCursor);
			gCmdParamInfo.cmdBufCursor --;
			gCmdParamInfo.cmdBufTail --;
			memcpy((gCmdParamInfo.cmdBufCursor),str,strlen(str)+1);

			vOutputString("\b \b");
			vOutputString(str);
			vOutputString(" \b");

			for(i = strlen(str); i > 0; i--)
			{
				vOutputChar('\b');
			}
		}
		while((gCmdParamInfo.cmdBufCursor == gCmdParamInfo.cmdBuf)&&(*gCmdParamInfo.cmdBufCursor == ' '));
	}
}

static void cli_direction(char ReadChar)
{
	switch(ReadChar)
	{
		case 'A': //up direction key
		{
			cli_read_history_cmd(0);
			break;
		}
		case 'B': //down direction key
		{
			cli_read_history_cmd(1);
			break;
		}
		case 'C': //right direciton key
		{
			if(*gCmdParamInfo.cmdBufCursor != '\0')
			{
				vOutputChar(0x1b);
				vOutputChar(0x5b);
				vOutputChar('C');
				gCmdParamInfo.cmdBufCursor++;
			}
			break;
		}
		case 'D': //left direciton key
		{
			if(gCmdParamInfo.cmdBufCursor != gCmdParamInfo.cmdBuf)
			{
				vOutputChar(0x1b);
				vOutputChar(0x5b);
				vOutputChar('D');
				gCmdParamInfo.cmdBufCursor--;
			}
			break;
		}
		default :
			break;
	}
}

/*
static CLI_Command_List_t *CLIParameterSeek(char *str)
{
	CLI_Command_List_t *command = NULL;
	uint8_t num = 0;
	return command;
}
*/
static char  *CLI_Charmatching(char *src,char *dst)
{
	char *ptr;

	if(*src == '#')
	{
		return dst;
	}

	if(!dst || !src)
	{
		return NULL;
	}

	if(*src == '&')
	{
		return dst;
	}

	if(*src == '[')
	{
		do
		{
			while(*(++src) == ' ');
			ptr = dst;
			while(*src == *ptr)
			{
				src ++;
				ptr ++;
				if(((*ptr == ' ') || (*ptr == '\0')) && ((*src == ' ') || (*src == '|') || (*src == ']')|| (*src == '\0')))
				{
					return (src - (ptr - dst));
				}
			}
			while((*src != ']')&&(*src != '|'))src++;
		}while(*src != ']');
		return NULL;
	}

	ptr = strchr(src,' ');

	if(((ptr == NULL) && (!strcmp(src,dst))) || \
			(((((ptr != NULL) && (!strncmp(src,dst,ptr - src)))) && ((*(dst + (ptr - src)) == ' ') || (*(dst + (ptr - src)) == '\0')))) || \
			((ptr == NULL) && strchr(dst,'-') &&  !strncmp(src,dst,strlen(src))))
	{
		return src;
	}
	else
	{
		return NULL;
	}

}

static CLI_Command_List_t *CLICommandSeek(char *str)
{
	CLI_Command_List_t *command = &clilist.command[0];
	char * src,  * dst ;
	do
	{
		src = command -> str;
		dst = str;
		while((*src == '#') || (CLI_Charmatching(src,dst) != NULL))
		{
			if(*src == '#' && (!CLIGetParameter(src,1)))
			{
				return command;
			}
			dst = CLIGetParameter(dst,1); //��ȡ�û���һ���ַ���
			src = CLIGetParameter(src,1); //��ȡ������һ���ַ���

			while((dst) && (*dst == '-'))
			dst = CLIGetParameter(dst,1); //��ȡ�û���һ���ַ���

			if(!src && !dst)
			{
				return command;
			}
		}
	}while((++command)->str);
	return NULL;
}

static void cli_ProcessCommand(char *str)
{
	HAL_StatusTypeDef ret = HAL_OK;
	CLI_Command_List_t *command;
	char *parameter[10] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	char **prt = parameter;

	cli_write_history_cmd(str);  //�����ʷ����
	cli_format_cmd(str,prt);	 //�ָ��ַ���  -all����ӵ�char** argv

	command = CLICommandSeek(str);  //�ڱ��в��Ҵ�����
	if(command != NULL)
	{
		while(command != NULL)
		{
			ret = command->p(gCmdParamInfo.argc,gCmdParamInfo.argv); //ִ���ַ���������
			if(strstr(str,"-t") == NULL)
			{
				break;
			}
			if( vGetchar(0) != 0)
			{
				break;
			}
		}
		if(HAL_OK != ret)
		{
			vOutputString("\r\n---ATECMDRESULT--- FAIL");
		}
		else
		{
			vOutputString("\r\n---ATECMDRESULT--- OK");
		}
	}
	else
	{
		vOutputString("\r\nCommand not recognised.  Enter 'help' to view a list of available commands.");
	}
}
static HAL_StatusTypeDef prvDispHelpConfig()  //�����������
{
	CLI_Command_List_t *command = &clilist.command[0];
	do
	{
		vOutputString("\r\n        ");
		vOutputString(command ->str);
	}while((++command) ->str);
	return HAL_OK;
}
void CLI_info_init(void)	//CLI��ʼ������
{
	gUsbParamInfo.read_ptr = gUsbParamInfo.cHandleBuf;
	gUsbParamInfo.write_ptr= gUsbParamInfo.cHandleBuf;
	memset(&gCmdParamInfo,'\0',sizeof(gCmdParamInfo));
	memset(&clilist,'\0',sizeof(clilist));
	gCmdParamInfo.outMessage = "\r\nbitohw : ";
	cli_deinit();
	CLICommandAdd("help",prvDispHelpConfig);  //��Ӱ�������
}

void CLIParameterAdd(char *str,Handle p)  //ע��ɱ��ַ�������
{
	static uint16_t num = 0;
	clilist.parameter[num].str = str;
	clilist.parameter[num].p   = p;
	num ++;
}

uint16_t  *CLICommandAdd(char *str,Handle p)   //ע���������
{
	static uint16_t num = 0;
	clilist.command[num].str = str;
	clilist.command[num].p	 = p;
	num ++;
	return &num;
}

void CLI_event_handle() //CLI������
{
	char ReadChar;

	while((ReadChar = vGetchar(0)) != 0)
	{
		if(ReadChar == '\n' || ReadChar == '\r')
		{
			label = false;
			if((gCmdParamInfo.cmdBuf == gCmdParamInfo.cmdBufTail)||(gCmdParamInfo.cmdBuf[0] == ' ') )
			{
				vOutputString(gCmdParamInfo.outMessage);
			}
			else
			{
				if(*(-- gCmdParamInfo.cmdBufTail) == ' ')
				{
					*gCmdParamInfo.cmdBufTail = '\0';
				}
				cli_ProcessCommand(gCmdParamInfo.cmdBuf);  //����Ԥ���������ַ���
				cli_deinit(); 								//�����ն�
				vOutputString(gCmdParamInfo.outMessage);
			}
		}
		else
		{
			if((ReadChar >= '\b') && (ReadChar < ' '))
			{
				label = false;
				if(ReadChar == '\b')
				{
					cli_backspace();		//�����˸��
				}
				else if(ReadChar == 0x1b && vGetchar(0xffff) == 0x5b)
				{
					cli_direction(vGetchar(0xffff));  //�������
				}
			}
			else if((ReadChar >= ' ') && (ReadChar <= '~' ))
			{
				if(ReadChar == '#' || label == true)
				{
					label = true;
				}
				else if((gCmdParamInfo.cmdBufTail - gCmdParamInfo.cmdBuf) < (CMD_MAX_LENG - 1))
				{
					cli_cInput(ReadChar); //����ַ�����Ԥ��������
				}
			}
		}
	}
}

char read_cInput(uint16_t time) //���жϻ�����
{
	char ReadChar;
	do
	{
		if(	gUsbParamInfo.read_ptr != gUsbParamInfo.write_ptr)
		{
			ReadChar = *gUsbParamInfo.read_ptr;
			if(++ gUsbParamInfo.read_ptr == (gUsbParamInfo.cHandleBuf + UART_BUFF_LEN))
			{
				gUsbParamInfo.read_ptr = gUsbParamInfo.cHandleBuf;
			}
			return ReadChar;
		}
	}while(time-- != 0);

	return 0;
}
void  write_cInput(char* Buf, uint32_t Len)  //д������
{
	do
	  {
		  if(gUsbParamInfo.write_ptr >= gUsbParamInfo.read_ptr)
		  {
			  if(gUsbParamInfo.write_ptr + Len <= (gUsbParamInfo.cHandleBuf + UART_BUFF_LEN))
			  {
				  strncpy(gUsbParamInfo.write_ptr,Buf,(uint16_t)Len);
				  gUsbParamInfo.write_ptr = gUsbParamInfo.write_ptr + Len;
				  Buf = Buf + Len;
				  Len = 0;
				  if(gUsbParamInfo.write_ptr == (gUsbParamInfo.cHandleBuf + UART_BUFF_LEN))
				  {
					  gUsbParamInfo.write_ptr = gUsbParamInfo.cHandleBuf;
				  }
			  }
			  else
			  {

				  strncpy(gUsbParamInfo.write_ptr,Buf,(uint16_t)((gUsbParamInfo.cHandleBuf + UART_BUFF_LEN) - gUsbParamInfo.write_ptr));
				  Buf = Buf + (gUsbParamInfo.cHandleBuf + UART_BUFF_LEN - gUsbParamInfo.write_ptr);
				  Len = Len - (gUsbParamInfo.cHandleBuf + UART_BUFF_LEN - gUsbParamInfo.write_ptr );
				  gUsbParamInfo.write_ptr = gUsbParamInfo.cHandleBuf;
			  }
		  }
		  else
		  {
			  if(((gUsbParamInfo.read_ptr - 1) - gUsbParamInfo.write_ptr) >= Len)
			  {
				  strncpy(gUsbParamInfo.write_ptr,Buf,(uint16_t)Len);
				  Buf = Buf + Len;
				  gUsbParamInfo.write_ptr = gUsbParamInfo.write_ptr + Len;
				  Len = 0;
			  }
			  else
			  {
				  strncpy(gUsbParamInfo.write_ptr,Buf,(uint16_t)((gUsbParamInfo.read_ptr - 1) - gUsbParamInfo.write_ptr));
				  Buf = Buf + ((gUsbParamInfo.read_ptr - 1) - gUsbParamInfo.write_ptr);
				  Len = 0;
				  gUsbParamInfo.write_ptr = gUsbParamInfo.read_ptr - 1;
			  }
		  }
	  }while(Len != 0);
}

