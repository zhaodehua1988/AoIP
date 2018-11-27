#ifndef _WV_COMMON_COMMAND_H_
#define _WV_COMMON_COMMAND_H_
#include"wv_type.h"
#include"wv_common.h"


#define WV_CMD_NAME_MAXLEN	 		 20
#define WV_CMD_COMMENT_MAXLEN		 64 

#define WV_CMD_ITEM0_MAXLEN			64
#define WV_CMD_ITEM1_MAXLEN			64

#define WV_CMD_ARGS_MAXNUM			30
#define WV_CMD_BUF_MAXLEN			128
#define WV_CMD_LIST_MAXLEN			16

typedef WV_S32 (*WV_CMD_PROFUNC)( WV_S32 argc, WV_S8 ** args,WV_S8 *prfBuff);

typedef struct WV_CMD_FUNC_E{
	WV_S8  cmdL0[WV_CMD_NAME_MAXLEN];				//命令 level 0
	WV_S8  cmdL1[WV_CMD_NAME_MAXLEN];			    //子命令 level 1
	WV_S8  cmdComment[WV_CMD_COMMENT_MAXLEN];		//comment
	WV_CMD_PROFUNC  cmdFunc;					    //function
}WV_CMD_FUNC_E;

WV_S32 	 WV_CMD_Create();
WV_S32	 WV_CMD_Destroy();
WV_S32   WV_CMD_PreProc (WV_S8  key,WV_S8 *cmd,WV_S8 *buf);
WV_S32   WV_CMD_Proc(WV_S8 *cmd,WV_S8 *prfBuf); 
WV_S32   WV_CMD_Register(WV_S8 *cmdL0,WV_S8 *cmdL1,WV_S8 *cmdComment,WV_CMD_PROFUNC pFunc); 
WV_S32 	 WV_CMD_GetExit();
WV_S32 	 WV_CMD_GetBye();
WV_S32 	 WV_CMD_GetReset(); 
WV_S32 	 WV_CMD_ClrBye(); 
#endif /* _WV_COMMAND_H_*/
