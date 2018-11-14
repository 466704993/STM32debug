/*
 * cli.h
 *
 *  Created on: Nov 14, 2018
 *      Author: bitohw03
 */

#ifndef INC_CLI_H_
#define INC_CLI_H_



#define CMD_MAX_LENG  			(256)
#define HISTORY_CMD_NUM 		(5)
#define CMD_MAX_PARAMETER_NUM  	(20)

typedef  HAL_StatusTypeDef (*Handle)(int argc, char** argv);//uint8_t
typedef struct xCOMMAND_LINE_INPUT
{
	char *str;
	Handle p;
}CLI_Command_List_t;

typedef struct
{
	char  cmdBuf[HISTORY_CMD_NUM][CMD_MAX_LENG];
	uint8_t  wIndex;
}HISTORY_CMD_T;

typedef struct
{
	int     		argc;
	char 			*cmdBufTail;   //����ĩ��
	char			*cmdBufCursor; //���λ��
	char 			*outMessage;
	char			*argv[CMD_MAX_PARAMETER_NUM];
	char 			argvBuf[CMD_MAX_LENG];
	char	   		cmdBuf[CMD_MAX_LENG];
	HISTORY_CMD_T	historyCmd;
}GET_CMD_PARAM_INFO_T;//gCmdParamInfo;

char     vGetchar(uint16_t time);
void	 CLI_info_init(void);
void 	 CLI_event_handle(void);
void 	 CLIParameterAdd(char *str,Handle p);
uint16_t *CLICommandAdd(char *str,Handle p);
char 	 read_cInput(uint16_t time);
void  	 write_cInput(char* Buf, uint32_t Len);

void     prvUARTCommandConsoleTask(void const *pvParameters);


#endif /* INC_CLI_H_ */
