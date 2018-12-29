/******************************************************************************
  File Name     :
  Version       :
  Author        :
  Created       :
  Description   :
  History       :
  1.Date        :
    Author      :
    Modification: Created file

******************************************************************************/

#include<stdio.h>
#include"wv_cmd.h"
#include"wv_ver.h" 

// printf wrapper that can be turned on and off by defining/undefining

#define CMD_DEBUG_MODE 
#ifdef  CMD_DEBUG_MODE 
#define CMD_printf(...)  do { printf("[%s]-%d:",__FUNCTION__, __LINE__);printf(__VA_ARGS__); fflush(stdout); } while(0)
#else 
#define CMD_printf(...)
#endif

static WV_CMD_FUNC_E  mCmdFunc[WV_CMD_ITEM0_MAXLEN][WV_CMD_ITEM1_MAXLEN]; 
static WV_S32 mCmdSysExit = 0;
static WV_S32 mCmdSysBye =0;
static WV_S8  mCmdList[WV_CMD_LIST_MAXLEN][WV_CMD_BUF_MAXLEN];
static WV_S32  mListNumCur=0,mListNumGet=0,mListNumTemp;

/******************************************************************************
WV_S32 WV_CMD_Help(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)
******************************************************************************/
WV_S32 WV_CMD_Help(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)
{
    WV_S32 i=0,j=0;

    if(argc == 0)
    {
        prfBuff += sprintf( prfBuff,"***************** help  ****************\r\n");
        for(i=0; i<WV_CMD_ITEM0_MAXLEN; i++)
        {
            if(strlen(mCmdFunc[i][0].cmdL0) == 0)
                break;
            if(strlen(mCmdFunc[i][0].cmdComment) == 0)
                break;
            prfBuff += sprintf(prfBuff,"%-16s:%s\r\n",mCmdFunc[i][0].cmdL0,mCmdFunc[i][0].cmdComment);
        }
    }

    else
    {
        prfBuff += sprintf( prfBuff,"***************** %s  ****************\r\n",argv[0]);

        for(i=0; i<WV_CMD_ITEM0_MAXLEN; i++)
        {
            if(strcmp(mCmdFunc[i][0].cmdL0,argv[0]) == 0)
                break;
        }


        if(i == WV_CMD_ITEM0_MAXLEN)
        {
            prfBuff += sprintf( prfBuff,"no command [%s]\r\n",argv[0]);
            return -1;
        }

        for(j=0; j< WV_CMD_ITEM1_MAXLEN; j++)
        {
            if(strlen(mCmdFunc[i][j].cmdL1) == 0 && strlen(mCmdFunc[i][j].cmdComment) == 0)
                break;

            prfBuff += sprintf(prfBuff,"%-10s%-10s: %s\r\n",mCmdFunc[i][j].cmdL0,mCmdFunc[i][j].cmdL1,mCmdFunc[i][j].cmdComment);
        }
    }

    prfBuff += sprintf(prfBuff,"***************************************\r\n");
    return WV_SOK;
}

/******************************************************************************
WV_S32   WV_CMD_Register(WV_S8 *cmdL0,WV_S8 *cmdL1,WV_S8 *cmdComment,WV_CMD_PROFUNC pFunc);
******************************************************************************/
WV_S32   WV_CMD_Register(WV_S8 *cmdL0,WV_S8 *cmdL1,WV_S8 *cmdComment,WV_CMD_PROFUNC cmdFunc) 
{
    WV_S32 i,j;

    if(cmdL0 == NULL || cmdComment == NULL)
    {
        CMD_printf("input erro\r\n");
        return WV_EFAIL;
    }

    for(i=0; i< WV_CMD_ITEM0_MAXLEN; i++)
    {
        if(strlen(mCmdFunc[i][0].cmdL0) == 0)
            break;

        if(strcmp(mCmdFunc[i][0].cmdL0,cmdL0) == 0)
            break;
    }

    j=0;
    if(cmdL1 != NULL)
    {
        for(j=1; j< WV_CMD_ITEM1_MAXLEN; j++)
        {
            if(strlen(mCmdFunc[i][j].cmdL1) == 0)
                break;

            if(strcmp(mCmdFunc[i][j].cmdL1,cmdL1) == 0)
                break;
        }
    }


    strcpy(mCmdFunc[i][j].cmdL0,cmdL0);
    if(cmdL1 != NULL)
    {
        if(strlen(mCmdFunc[i][0].cmdL0) == 0)
        {
            strcpy(mCmdFunc[i][0].cmdL0,cmdL0);
            strcpy(mCmdFunc[i][0].cmdComment,"********");
        }
        strcpy(mCmdFunc[i][j].cmdL1,cmdL1);
    }
    strcpy(mCmdFunc[i][j].cmdComment,cmdComment);
    mCmdFunc[i][j].cmdFunc = cmdFunc;
    if(i == WV_CMD_ITEM1_MAXLEN  || j == WV_CMD_ITEM1_MAXLEN)
    {
        CMD_printf("list is full\r\n");
        return WV_EFAIL;
    }

    return WV_SOK;


}


/******************************************************************************
WV_S32   WV_CMD_PreProc (WV_S8 *buff,WV_S8 *cmd);
******************************************************************************/
WV_S32 WV_CMD_PreProc (WV_S8 key,WV_S8 *cmd,WV_S8 *buf)
{ 
    WV_S32 ret = -1;// -1 erro   0 char  1 \r\n   2 key buf;
    WV_S32 cmdLen   = strlen(cmd);
    WV_S32 bufLen   = strlen(buf);

    switch (key)
    {   //  key = /r /n
    // case '\r':
    case '\n':
        //if(cmdLen > 0)  	ret = 1;
        ret = 1;
        cmd[cmdLen] = 0;
        break;
        //	key == del
    case 0x7f:
    case 0x08:
        if(cmdLen > 0)   cmdLen--;
        cmd[cmdLen] = 0;
        ret =0;
        break;
    case 0x1b:
        buf[0] = 0x1b;
        buf[1] = 0;
        break;
    case 0x5b:
        if( buf[0] == 0x1b  && bufLen == 1)
        {
            buf[1] = 0x5b;
            buf[2] = 0;
            break;
        }
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
        if(buf[0] == 0x1b  && buf[1] == 0x5b && bufLen == 2)
        {
            buf[bufLen] =key;
            buf[bufLen+1] =0;
            ret = 2;
            if(key == 0x41)
            {
                //printf("page UP  NOW lis[%d] = %s\r\n",mListNumGet,mCmdList[mListNumGet]);
                strcpy(cmd,mCmdList[mListNumGet]);
                mListNumTemp = mListNumGet;
                mListNumTemp++;
                mListNumTemp %= WV_CMD_LIST_MAXLEN;

                if(strlen(mCmdList[mListNumTemp]) != 0)
                {
                    mListNumGet = mListNumTemp;
                }
                // printf("page UP  NEXT lis[%d] = %s\r\n",mListNumGet,mCmdList[mListNumGet]);
            }
            if(key == 0x42)
            {
                //printf("page DOWN lis[%d] = %s\r\n",mListNumGet,mCmdList[mListNumGet]);
                strcpy(cmd,mCmdList[mListNumGet]);
                mListNumTemp = mListNumGet;
                if(mListNumTemp == 0)
                {
                    mListNumTemp = WV_CMD_LIST_MAXLEN -1;
                }
                else
                {
                    mListNumTemp--;
                }
                if(strlen(mCmdList[mListNumTemp]) != 0)
                {
                    mListNumGet = mListNumTemp;
                }
                //printf("page DOWN pre lis[%d] = %s\r\n",mListNumGet,mCmdList[mListNumGet]);
            }
            buf[0] = 0;
            break;
        }
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:

        if(buf[0] == 0x1b  && buf[1] == 0x5b && bufLen < 6)
        {
            buf[bufLen] = key;
            buf[bufLen+1] = 0;
            break;
        }
    case  0x7e:
        if(buf[0] == 0x1b  && buf[1] == 0x5b && bufLen < 6)
        {
            buf[bufLen] = key;
            buf[bufLen+1] = 0;
            ret = 2;
            break;
        }

        // ASIIC
    default:
        if(key >= 0x20 )
        {
            if(cmdLen < WV_CMD_BUF_MAXLEN -2)
            {
                cmd[cmdLen] = key;
                cmdLen++;
                cmd[cmdLen] = 0;
                buf[0] = 0;
                ret = 0;
            }
        }
        break;
    }

    return ret;

}



/******************************************************************************
WV_S32   WV_CMD_arsProc (WV_S8 *cmd ,WV_S8 **args,WV_S32 *argsNum);
******************************************************************************/
WV_S32   WV_CMD_arsProc (WV_S8 *cmd ,WV_S8 **args,WV_S32 *argsNum)
{

    WV_S32 cmdLen, argStart, argEnd, num;

    if(NULL == cmd || NULL == args || NULL == argsNum)
    {
        CMD_printf("input erro\r\n");
        return WV_EFAIL;
    }
    num = 0;
    *argsNum = num;

    cmdLen  = strlen(cmd);
    argStart = 0;
    argEnd   = argStart;
    while ( argStart < cmdLen )
    {
        //find arg start pos
        for(; argStart<cmdLen; argStart++)
        {
            if ( cmd[argStart] == 0X20 ) { continue; }  //0x20  = " "
            break;
        }
        if ( argStart >= cmdLen) { break; }

        //find arg end pos
        argEnd = argStart;
        for(; argEnd<cmdLen; argEnd++)
        {
            if ( cmd[argEnd] == 0X20) { break; }
        }
        if ( argEnd == argStart) { break;}

        if ( (argEnd-argStart) >= (WV_CMD_NAME_MAXLEN) ) { break;}
        if ( num >= (WV_CMD_ARGS_MAXNUM) ) { break; }
        memcpy(args[num], cmd+argStart, argEnd-argStart);
        args[num][argEnd-argStart] = 0;
        num++;
        argStart = argEnd;
    }

    *argsNum = num;
    return 0;

}
/******************************************************************************
WV_S32   WV_CMD_Proc(WV_S8 *cmd,WV_S8 *prfBuf);
******************************************************************************/
WV_S32   WV_CMD_Proc(WV_S8 *cmd,WV_S8 *prfBuf)
{
    WV_S32  cmdLen, argc;
    WV_S32 i,j;

    WV_S8 args[WV_CMD_ARGS_MAXNUM][WV_CMD_NAME_MAXLEN];
    WV_S8 *argv[WV_CMD_ARGS_MAXNUM];

    if(NULL == cmd || NULL == prfBuf)
    {
        CMD_printf("input erro\r\n");
        return WV_EFAIL;
    }
    for(i=0; i < WV_CMD_ARGS_MAXNUM; i++)
    {
        argv[i] = args[i];
        args[i][0] = 0;
    }

    cmdLen = strlen(cmd);
    while ( cmdLen )
    {
        if ( (cmd[cmdLen-1] == '\r') || (cmd[cmdLen-1] == '\n') || (cmd[cmdLen-1] == 0X20))
        {
            cmd[cmdLen-1] = 0; cmdLen--;
        }
        else { break; }
    }

    WV_CMD_arsProc(cmd, argv, &argc);
    if ( argc == 0 )
    {
        return WV_EFAIL;
    }

    mListNumCur++;
    mListNumCur %= WV_CMD_LIST_MAXLEN;
    mListNumGet  = mListNumCur;
    strcpy(mCmdList[mListNumCur],cmd);

    mListNumTemp = mListNumCur;
    mListNumTemp++;
    mListNumTemp %= WV_CMD_LIST_MAXLEN;
    mCmdList[mListNumTemp][0] = 0;
    // printf("list cur =%d ,get = %d cmd=[%s]\r\n",mListNumCur,mListNumGet,mCmdList[mListNumCur]);

    for(i=0; i< WV_CMD_ITEM0_MAXLEN; i++)
    {

        if(0 == strcmp(argv[0],mCmdFunc[i][0].cmdL0))
        {
            if(mCmdFunc[i][0].cmdFunc != NULL)
            {

                return mCmdFunc[i][0].cmdFunc(argc-1,(WV_S8 **)(argv+1),prfBuf);
            }

            if(argc < 2)
            {
                return WV_CMD_Help(argc, (WV_S8 **)argv,prfBuf);
            }
            //find childCommand

            for(j=1; j< WV_CMD_ITEM1_MAXLEN; j++)
            {
                if((0 == strcmp(argv[1],mCmdFunc[i][j].cmdL1))
                        && (mCmdFunc[i][j].cmdFunc != NULL))
                {


                    return mCmdFunc[i][j].cmdFunc(argc-2,(WV_S8 **)(argv+2),prfBuf);
                }
            }
            sprintf(prfBuf,"no child cmd: %s\r\n",argv[1]);
            return WV_EFAIL;
        }
    }

    sprintf(prfBuf,"no cmd : %s\r\n",argv[0]);

    return  WV_EFAIL;
}


/******************************************************************************
WV_S32 	 WV_CMD_SysCmd(WV_S32 argc, WV_S8 ** args,WV_S8 *prfBuff)
******************************************************************************/

WV_S32   WV_CMD_SysCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)
{
    if(argc <=  0)
    {
        prfBuff += sprintf(prfBuff,"no comand get!!!\r\n");
        return WV_SOK;
    }

    WV_S8 cmdBuf[1024] = {0};
    WV_S32 i = 0;
    for(i = 0; i < argc; i++)
    {
        if(NULL ==argv[i])
            break;

        if(0 != i)
            strcpy(cmdBuf + strlen(cmdBuf), " ");
        strcpy(cmdBuf + strlen(cmdBuf), argv[i]);

    }
    prfBuff += sprintf(prfBuff,"call system cmd [%s] \r\n\r\n", cmdBuf);
    return system(cmdBuf);
}



/******************************************************************************
WV_S32 	 WV_CMD_SysExit(WV_S32 argc, WV_S8 ** args,WV_S8 *prfBuff)
******************************************************************************/

WV_S32 	 WV_CMD_SysExit(WV_S32 argc, WV_S8 ** args,WV_S8 *prfBuff)
{


    prfBuff += sprintf(prfBuff,"exiting\r\n");
    mCmdSysExit = 1;
    return  WV_SOK;
}

/******************************************************************************
WV_S32 	 WV_CMD_SysBye(WV_S32 argc, WV_S8 ** args,WV_S8 *prfBuff)
******************************************************************************/

WV_S32 	 WV_CMD_SysBye(WV_S32 argc, WV_S8 ** args,WV_S8 *prfBuff)
{


    prfBuff += sprintf(prfBuff,"bye\r\n");
    mCmdSysBye = 1;
    return  WV_SOK;
}

/******************************************************************************
WV_S32 	 WV_CMD_GetVer(WV_S32 argc, WV_S8 ** args,WV_S8 *prfBuff)
******************************************************************************/

WV_S32 	 WV_CMD_GetVer(WV_S32 argc, WV_S8 ** args,WV_S8 *prfBuff)
{

    prfBuff += sprintf(prfBuff,"%s:%s(%s-%s)\r\n",SOFTVER_NAME,SOFTVER_MAIN,SOFTVER_DATE,SOFTVER_TIME );
    return  WV_SOK;
}
/******************************************************************************
WV_S32 	 WV_CMD_SysReboot(WV_S32 argc, WV_S8 ** args,WV_S8 *prfBuff)
******************************************************************************/

WV_S32 	 WV_CMD_SysReboot(WV_S32 argc, WV_S8 ** args,WV_S8 *prfBuff)
{

    prfBuff += sprintf(prfBuff,"rebooting!!!!!\r\n");
    system("reboot");
    return  WV_SOK;
}


/******************************************************************************
WV_S32 	 WV_CMD_GetExit() 
******************************************************************************/

WV_S32 	 WV_CMD_GetExit()
{

    return  mCmdSysExit;
}

/******************************************************************************
WV_S32 	 WV_CMD_GetBye() 
******************************************************************************/

WV_S32 	 WV_CMD_GetBye()
{

    return  mCmdSysBye;
}

/******************************************************************************
WV_S32 	 WV_CMD_ClrBye() 
******************************************************************************/

WV_S32 	 WV_CMD_ClrBye()
{
    mCmdSysBye = 0;
    return  WV_SOK;
}
/******************************************************************************
WV_S32 	 WV_CMD_GetReset() 
******************************************************************************/

WV_S32 	 WV_CMD_GetReset()
{

    return system("reboot");
}
/******************************************************************************
WV_S32 	 WV_CMD_Create();
注册一些基本命令到控制台
******************************************************************************/

WV_S32 	 WV_CMD_Create()
{
    
    memset(mCmdFunc,0x0,sizeof(mCmdFunc));
    WV_CMD_Register("sys",NULL,"admin tools",NULL);
    WV_CMD_Register("sys","exit","exit app",WV_CMD_SysExit);
    WV_CMD_Register("sys","cmd","execute sys  cmd",WV_CMD_SysCmd);
    WV_CMD_Register("help",NULL,"print help info",WV_CMD_Help);
    WV_CMD_Register("bye",NULL,"bye telnet",WV_CMD_SysBye);
    WV_CMD_Register("reboot",NULL,"reboot",WV_CMD_SysReboot);
    WV_CMD_Register("get",NULL,"get infomation of module",NULL);
    WV_CMD_Register("get","ver","get app version",WV_CMD_GetVer);
    WV_CMD_Register("set",NULL,"set parameter of module",NULL);
    WV_CMD_Register("save",NULL,"save parameter of module",NULL);
    return WV_SOK;
}

/******************************************************************************
WV_S32 	 WV_CMD_Destroy();
******************************************************************************/

WV_S32 	 WV_CMD_Destroy()
{

    return WV_SOK;
}




