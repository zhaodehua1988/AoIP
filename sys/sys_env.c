#include"sys_env.h"
#include "wv_ver.h"
#define   SYS_ENV_FILE_NAME    "conf.ini"
#define   SYS_ENV_FILE_PATH    "./env/"
// 64 scene
#define   SYS_ENV_FILE_NAME_64    "conf64.ini"

static WV_CONF_DEV_E  *pSysEnv;


/**********************************************************************

WV_S32  SYS_ENV_GetOldFile();

**********************************************************************/
WV_S32  SYS_ENV_GetOldFile()
{


	WV_S32 ret;
	WV_S8 name[128];
	WV_S8 name64[128];
	memset(name,0,sizeof(name));
	sprintf(name,"%s%s",SYS_ENV_FILE_PATH,SYS_ENV_FILE_NAME);
	ret = access(name,F_OK);
	if(ret != 0)
	{
		return WV_SOK;
	}
	memset(name64,0,sizeof(name64));
	sprintf(name64,"%s%s",SYS_ENV_FILE_PATH,SYS_ENV_FILE_NAME_64);

	ret = access(name64,F_OK);
	if(ret == 0)
	{
		WV_printf(" ****** %s is exist**********\n",name64);
		return WV_SOK;
	}

	FILE *fp;
	FILE *fp64;
	WV_S32 readLen=0,writeLen=0;
	WV_S8 buf[1028];

	fp = fopen(name,"rb+");	
	if(fp == NULL)
	{
		WV_printf(" fopen %s error\n",name);	
		return WV_SOK;
	}
		
	fp64 = fopen(name64,"wb+");	
	if(fp64 == NULL)
	{
		WV_printf(" fopen %s error\n",name64);	
		return WV_SOK;
	}
	
	while(1)
	{
		readLen = fread(buf,1,1024,fp);
		if(readLen <= 0)
		{
			break;
		}

		writeLen = fwrite(buf,1,readLen,fp64);
		if(writeLen != readLen)
		{
			break;
		}

	}
		

	fclose(fp);
	fclose(fp64);
	return WV_SOK;
  
}


/**********************************************************************

WV_S32  SYS_ENV_Register(WV_S8 type ,WV_S8 * pName,WV_S8 *pDefault,WV_S8  *pNote);

**********************************************************************/
WV_S32  SYS_ENV_Register(WV_S8 type ,WV_S8 * pName,WV_S8 *pDefault,WV_S8  *pNote)
{
 return WV_CONF_Register(pSysEnv,type ,pName,pDefault,pNote); 
  
}



/**********************************************************************

WV_S32  SYS_ENV_Get(WV_S8 *pName ,WV_S8 *pValue);

**********************************************************************/
WV_S32  SYS_ENV_Get(WV_S8 *pName ,WV_S8 *pValue) 
{
 return  WV_CONF_Get(pSysEnv,pName,pValue);  
 
}
/**********************************************************************

WV_S32  SYS_ENV_GetU32(WV_S8 *pName ,WV_U32 * pValue);

**********************************************************************/
WV_S32  SYS_ENV_GetU32(WV_S8 *pName ,WV_U32 * pValue)
{

 WV_S8  Value[WV_CONF_VALUE_MAX_LEN];
 WV_S32 ret = 0;
ret = WV_CONF_Get(pSysEnv,pName,Value);
if(ret == WV_EFAIL)
{
  WV_ERROR("no the env  %s\r\n",pName);
  return WV_EFAIL;
}
   
 WV_STR_S2v(Value,pValue); 
 
 return WV_SOK;
 
}


/**********************************************************************

WV_S32  SYS_ENV_Set(WV_S8 *pName ,WV_S8 *pValue);

**********************************************************************/

WV_S32  SYS_ENV_Set(WV_S8 *pName ,WV_S8 *pValue)

{
  return  WV_CONF_Set(pSysEnv,pName,pValue); 
} 



/**********************************************************************

WV_S32  SYS_ENV_SetU32(WV_S8 *pName ,WV_U32 dat);

**********************************************************************/

WV_S32  SYS_ENV_SetU32(WV_S8 *pName ,WV_U32 data)

{
   WV_S8  Value[WV_CONF_VALUE_MAX_LEN];
   sprintf(Value,"%d",data);
    
  return  WV_CONF_Set(pSysEnv,pName,Value); 
} 
/**********************************************************************

WV_S32  SYS_ENV_SetDefaultAll();

**********************************************************************/

WV_S32  SYS_ENV_SetDefaultAll()

{
 
  return  WV_CONF_SetDefalutAll(pSysEnv);
} 



/**********************************************************************

WV_S32  SYS_ENV_SetDefault();

**********************************************************************/

WV_S32  SYS_ENV_SetDefault(WV_S8 *pName)

{
 
  return  WV_CONF_SetDefalut(pSysEnv,pName);
} 

/**********************************************************************

WV_S32  SYS_ENV_Save();

**********************************************************************/
WV_S32  SYS_ENV_Save()
{
  return WV_CONF_Save(pSysEnv);  
}


/****************************************************************************

WV_S32 SYS_ENV_CmdGet(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 SYS_ENV_CmdGet(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)
{
   WV_S32 i,j,len;
   WV_S8 lineBuf[WV_CONF_ITEM_MAX_LEN], * pTemp;
   if(argc > 0)
   {
      WV_S8  value[WV_CONF_VALUE_MAX_LEN];
      i =  SYS_ENV_Get(argv[0],value); 
      if(i == WV_EFAIL)
		{
		  prfBuff += sprintf(prfBuff,"no env %s \r\n",argv[0]);
		  return WV_SOK;     
		}
	  if(pSysEnv->pItem[i].type == 1)
	  {
	     memset(lineBuf, 0x20, WV_CONF_ITEM_MAX_LEN);  
	     len = strlen(pSysEnv->pItem[i].name);
	     memcpy(lineBuf,pSysEnv->pItem[i].name,len);
	     lineBuf[WV_CONF_NAME_MAX_LEN -1] = '=';
	     
	     pTemp = lineBuf + WV_CONF_NAME_MAX_LEN;
	     len = strlen(pSysEnv->pItem[i].value);
	     memcpy(pTemp,pSysEnv->pItem[i].value,len);
	     
	     lineBuf[WV_CONF_NAME_MAX_LEN + WV_CONF_VALUE_MAX_LEN -1] = ':'; 
	     pTemp =lineBuf + WV_CONF_NAME_MAX_LEN + WV_CONF_VALUE_MAX_LEN;
	     sprintf(pTemp,"%s\r\n",pSysEnv->pItem[i].note); 
	     lineBuf[WV_CONF_ITEM_MAX_LEN - 1] = 0;
	     prfBuff += sprintf(prfBuff,"%s",lineBuf);  
	     return WV_SOK;	       
	  } 
	  else
	  {
	  
		  for(j = i+1;j< pSysEnv->numItem;j++)
		  {
		     if(pSysEnv->pItem[j].type == 1)
		     {
				memset(lineBuf, 0x20, WV_CONF_ITEM_MAX_LEN);  
				len = strlen(pSysEnv->pItem[j].name);
				memcpy(lineBuf,pSysEnv->pItem[j].name,len);
				lineBuf[WV_CONF_NAME_MAX_LEN -1] = '=';

				pTemp = lineBuf + WV_CONF_NAME_MAX_LEN;
				len = strlen(pSysEnv->pItem[j].value);
				memcpy(pTemp,pSysEnv->pItem[j].value,len);

				lineBuf[WV_CONF_NAME_MAX_LEN + WV_CONF_VALUE_MAX_LEN -1] = ':'; 
				pTemp = lineBuf + WV_CONF_NAME_MAX_LEN + WV_CONF_VALUE_MAX_LEN;
				sprintf(pTemp,"%s\r\n",pSysEnv->pItem[j].note); 
				lineBuf[WV_CONF_ITEM_MAX_LEN - 1] = 0;
				prfBuff += sprintf(prfBuff,"%s",lineBuf);   
		     }
		     else
		     {
		       return WV_SOK;
		     } 
		      
		  }
	  }	  
       return WV_SOK;    
    } 
   prfBuff += sprintf(prfBuff,"env:\r\n"); 
   for( i = 0; i< pSysEnv-> numItem ;i++)
   {
     if(pSysEnv->pItem[i].type == 0 )
     {
      prfBuff += sprintf(prfBuff,"%s\r\n",pSysEnv->pItem[i].name);  
     }
     else
     {
         memset(lineBuf, 0x20, WV_CONF_ITEM_MAX_LEN);  
	     len = strlen(pSysEnv->pItem[i].name);
	     memcpy(lineBuf,pSysEnv->pItem[i].name,len);
	     lineBuf[WV_CONF_NAME_MAX_LEN -1] = '=';
	     
	     pTemp = lineBuf + WV_CONF_NAME_MAX_LEN;
	     len = strlen(pSysEnv->pItem[i].value);
	     memcpy(pTemp,pSysEnv->pItem[i].value,len);
	     
	     lineBuf[WV_CONF_NAME_MAX_LEN + WV_CONF_VALUE_MAX_LEN -1] = ':'; 
	     pTemp = lineBuf +WV_CONF_NAME_MAX_LEN + WV_CONF_VALUE_MAX_LEN;
	     sprintf(pTemp,"%s\r\n",pSysEnv->pItem[i].note); 
	     lineBuf[WV_CONF_ITEM_MAX_LEN - 1] = 0;
	     prfBuff += sprintf(prfBuff,"%s",lineBuf);     
     }
   }
    
 return WV_SOK;
}

/****************************************************************************

WV_S32  SYS_ENV_GetVersion(WV_U16 *pVer);

****************************************************************************/
WV_S32  SYS_ENV_GetVersion(WV_U16 *pVer)
{
		WV_U8 name[20];
		memset(name,0,sizeof(name));
                SYS_ENV_Get("EnvVer",name);
                strcpy(name,SYS_ENV_VERSION_NO);      //这里是获取的宏定义的版本号，而不是从文件里写的版本号了
		printf("the Version is %s\n",name);
		WV_S32 i,j=3,temp;		
		WV_U16 data = 0;
		WV_S8 *p=name;
		//sprintf(name,"%s",SOFTVER_MAIN);
		for(i=0;i<strlen(name);i++)
		{

			if(name[i] == '.')
			{
				
				sscanf(p,"%d",&temp);
				data = data | ((temp &0xf) << (4*j));
				j--;
				p = &name[i+1];		
			}
			
		}
		sscanf(p,"%d",&temp);
		data = data | ((temp &0xf) << (4*j));
                
		*pVer = data;
		return 0;
}





/****************************************************************************

WV_S32 SYS_ENV_CmdSet(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 SYS_ENV_CmdSet(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)
{
   WV_S32 i;
   WV_S8  value[WV_CONF_VALUE_MAX_LEN];
   
   
   if(argc < 2) 
   {
      prfBuff += sprintf(prfBuff,"set env <name> <value>\r\n");   
      return WV_SOK; 
   }  
   
   i=SYS_ENV_Get(argv[0],value);
   if(i == WV_EFAIL)
    {
     prfBuff += sprintf(prfBuff,"no env %s \r\n",argv[0]);
      return WV_SOK;     
    }
    if(pSysEnv->pItem[i].type == 1)
     {
        i=SYS_ENV_Set(argv[0],argv[1]); 
        if(i == WV_EFAIL)
		{
		  prfBuff += sprintf(prfBuff,"set %s erro\r\n",argv[0]);
		 // return WV_SOK;     
		}
		else
		{
        	prfBuff += sprintf(prfBuff,"set %s  =  %s\r\n",pSysEnv->pItem[i].name,pSysEnv->pItem[i].value);  
        	
        }	
     } 
     else
     {
      prfBuff += sprintf(prfBuff,"%s is type module\r\n",argv[0]);
     }
      
 return WV_SOK;
}




/****************************************************************************

WV_S32 SYS_ENV_CmdSet(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 SYS_ENV_CmdSetDefault(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)
{
    WV_S32 ret;
   WV_S8  value[WV_CONF_VALUE_MAX_LEN];
  
  
   if(argc > 0) 
   {
     
     ret = SYS_ENV_SetDefault(argv[0]);
     if(ret == WV_EFAIL)
     {
        prfBuff += sprintf(prfBuff,"argv[0]  default Erro!!!\r\n");   
     }
     else
     {
         WV_CHECK_FAIL(SYS_ENV_Get(argv[0] ,value) );
        prfBuff += sprintf(prfBuff,"%s =  %s \r\n",argv[0],value);   
     }
     
   } 
   else
   {
     ret =   SYS_ENV_SetDefaultAll();
     if(ret == WV_EFAIL)
     {
        prfBuff += sprintf(prfBuff,"default all Erro!!!\r\n");   
     }
     else
     {  
        prfBuff += sprintf(prfBuff,"default all ok \r\n",argv[0],value);   
     } 
   }  
   return WV_SOK;
} 

/****************************************************************************

WV_S32 SYS_ENV_CmdSave(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 SYS_ENV_CmdSave(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)
{
  WV_S32 ret;
  prfBuff += sprintf(prfBuff,"save env .....\r\n");
  
  ret =  SYS_ENV_Save();
  
  if(ret == WV_SOK)
  {
  		prfBuff += sprintf(prfBuff,"save env ok\r\n");
  } 
  else
  {
   		prfBuff += sprintf(prfBuff,"save env erro!!!!\r\n");       
  }
  
  return WV_SOK; 
}



/**********************************************************************

WV_S32  SYS_ENV_Open();

**********************************************************************/
WV_S32  SYS_ENV_Open()
{ 
   pSysEnv =  (WV_CONF_DEV_E *)malloc( sizeof(WV_CONF_DEV_E));
	//get old 16 scene file 	
	SYS_ENV_GetOldFile();
   WV_CHECK_RET( WV_CONF_Creat(pSysEnv,SYS_ENV_FILE_NAME_64,SYS_ENV_FILE_PATH));
   WV_ASSERT_RET( WV_CONF_Load(pSysEnv)); 
   //for env test
   WV_CHECK_FAIL( WV_CONF_Register(pSysEnv,0,"Env",NULL,NULL));
   WV_CHECK_FAIL( WV_CONF_Register(pSysEnv,1,"EnvName","DC98M","Env name"));
   WV_CHECK_FAIL( WV_CONF_Register(pSysEnv,1,"EnvVer","1.0.0.0","Env version"));

   
   //
   WV_CHECK_FAIL( WV_CONF_Register(pSysEnv,0,"Board",NULL,NULL));
   WV_CHECK_FAIL( WV_CONF_Register(pSysEnv,1,"BdName","DC98M","Board name"));
   WV_CHECK_FAIL( WV_CONF_Register(pSysEnv,1,"BdVer","0x11","Board version"));
   WV_CHECK_FAIL( WV_CONF_Register(pSysEnv,1,"UpdateInfo","20170424a","UpdateInfo"));

   //注册一些跟env相关的控制台命令
   WV_CMD_Register("env",NULL,"sys env model cmd",NULL);
   WV_CMD_Register("env","get","get the sys env",SYS_ENV_CmdGet); 
   WV_CMD_Register("env","set","set the sys env",SYS_ENV_CmdSet);
   WV_CMD_Register("env","default","set the sys env default",SYS_ENV_CmdSetDefault);
   WV_CMD_Register("env","save","save all env",SYS_ENV_CmdSave);
   
 return  WV_SOK;
}

/**********************************************************************

WV_S32  SYS_ENV_Close();

**********************************************************************/
WV_S32  SYS_ENV_Close()
{  
 if(pSysEnv-> numItemBuf  != pSysEnv-> numItem)
  {
    WV_ASSERT_RET(WV_CONF_Save(pSysEnv));
  } 
   WV_CONF_Destry(pSysEnv);
  free(pSysEnv);
  pSysEnv = NULL; 
 return  WV_SOK;
}


