#include "wv_telnet.h"

typedef struct WV_TELNET_DEV_E 
{
	WV_THR_HNDL_T thrHndl; 
	WV_U32      open;
	WV_U32      close; 
	WV_S32      mSocket;
    WV_S32      cSocket;
    WV_U32      port;
	WV_U32      cmdNum;
	WV_S8       *pBuf; 
 
} WV_TELNET_DEV_E;
 
static    WV_TELNET_DEV_E  *pTelDev;



/********************************************************

WV_S32 WV_TEL_Init(WV_TELNET_DEV_E  * pDev)

********************************************************/ 
WV_S32   WV_TEL_Init(WV_TELNET_DEV_E  * pDev)
{
    unsigned int reuse;
	socklen_t len;
	struct sockaddr_in addr;
	WV_S32 status;

	pDev -> mSocket = socket(AF_INET,SOCK_STREAM,0);
	if (pDev -> mSocket < 0) 
	{ 
		WV_ERROR("Telnet  socket init error.\r\n");
	    return  WV_EFAIL; 
	 }
	 
	//fcntl(DpDev ->mSocket,F_SETFL,O_NONBLOCK);//no block
	reuse = 1;
	len = sizeof(reuse);
	setsockopt(pDev ->mSocket, SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,len);
	//get sen buff
	
     WV_S32   nSendBufLen;
	 socklen_t optlen = sizeof(int); 
	 nSendBufLen = 0;
     getsockopt(pDev->mSocket, SOL_SOCKET, SO_SNDBUF,(char*)&nSendBufLen, &optlen); 
	 WV_printf("telnet socket SNDBUF  = %d.\r\n",nSendBufLen);
	 
	 
	addr.sin_family = AF_INET;
	addr.sin_port = htons(pDev ->port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);  
	bzero(&(addr.sin_zero), 8);
	status  =  bind(pDev ->mSocket, (struct sockaddr *) &addr, sizeof(addr));
	if( status < 0 )
	{
		close(pDev ->mSocket); 
		pDev ->mSocket = -1; 
		WV_ERROR("SERVER socket bind  error = %d.\r\n",status);
		return WV_EFAIL;
	} 
	 
  return WV_SOK;
}

 
/********************************************************

WV_S32 WV_TEL_DeInit(WV_TELNET_DEV_E  * pDev)

********************************************************/ 

WV_S32 WV_TEL_DeInit(WV_TELNET_DEV_E *pDev)
{
  if(pDev -> cSocket >= 0)
  {
     close(pDev->cSocket);
     pDev->cSocket = -1; 
  }
  
 if(pDev -> mSocket >= 0)
  {
     close(pDev->mSocket);
     pDev->mSocket = -1; 
  }
  WV_printf("Telnet socket DeInit OK\r\n");   
  return  WV_SOK;
}





/********************************************************

WV_S32  WV_TEL_GetChar(WV_S32 socket,WV_S8 *pChar)

********************************************************/ 
WV_S32  WV_TEL_GetChar(WV_S32 socket,WV_S8 *pChar)
{
	WV_S32 ret =0,recvLen; 
	recvLen = recv(socket,pChar,1,0);
	if(recvLen == 1)
		{  
		ret = WV_SOK; 
		} 
	else if(recvLen == 0)
		{
		WV_ERROR("Telnet  CNNECT loss !!!!!!!!!! %d",recvLen);
		ret = 1; 
		}
	else
		{ 
		WV_ERROR("Telnet recv EROOR !!!!!!!!!! %d",recvLen);
		ret = WV_EFAIL; 
		} 
	//printf(" WV_TEL_GetChar : 0x%02x\r\n",*pChar);	
		
 return ret;
}



/*********************************************************

WV_S32  WV_TEL_Print(WV_S32 socket,WV_S8 *pBuf);

*********************************************************/

WV_S32  WV_TEL_Print(WV_S32 socket,WV_S8 *pBuf)
{
 
    WV_S32 len,sendLen;   
	len =  strlen(pBuf);
	
     sendLen = send(socket,pBuf,len,0); 
    if(sendLen != len)
    {  
      	WV_ERROR("telnet send  EROOR ! %d[%d]\r\n",sendLen,len);
     	return WV_EFAIL;  
    }/*
    printf("send %d:\r\n",len);
    WV_S32 i;
    for(i=0;i< len;i++)
    {
      printf("%02x ",pBuf[i]); 
		} 
	  printf("\r\n");*/	
	return WV_SOK;
}



/*********************************************************

WV_S32  WV_TEL_SendChar(WV_S32 socket,WV_S8 key);

*********************************************************/

WV_S32  WV_TEL_SendChar(WV_S32 socket,WV_S8 key)
{
 
    WV_S32 sendLen;   
	 
    sendLen = send(socket,&key,1,0); 
    if(sendLen != 1)
    {  
      	WV_ERROR("telnet send  char EROOR ! %d\r\n",sendLen);
     	return WV_EFAIL;  
    }
    
	return WV_SOK;
}
/*********************************************************

void * WV_TEL_Proc(void * prm);

*********************************************************/
void * WV_TEL_Proc(void * prm)
{
	WV_TELNET_DEV_E   * pDev;
	WV_S32 ret,cursorCnt;
	WV_S8 *pBuf; 
	WV_S8 cmd[WV_CMD_BUF_MAXLEN],buf[20],charBuf,titel[128];
	pDev = (WV_TELNET_DEV_E *) prm; 
	pBuf = pDev -> pBuf;
	//
	if( WV_TEL_Init(pDev) == WV_EFAIL)
	  {
		return NULL;
	  } 
	ret  =  listen(pDev -> mSocket,3);
	if ( ret  < 0 ) 
		{
		close(pDev ->mSocket); 
		pDev ->mSocket = -1; 
			WV_ERROR("Telnet socket listen  error = %d.\r\n",ret);
			return NULL;
		}
	WV_printf("Telnet socket listen OK\r\n");  
	  
	pDev -> open   = 1;
	pDev -> close  = 0; 
	while(pDev -> open  == 1)
	{
		  
		pDev ->cSocket = accept(  pDev ->mSocket,NULL, NULL); 
		if( pDev -> cSocket < 0)
			{
			WV_ERROR("Telnet  SOCKET  ACCEPT   error\r\n");
			continue;
			} 

		//get sen buff
		WV_S32   nSendBufLen;
		socklen_t optlen = sizeof(int); 
		nSendBufLen = 0;
		getsockopt(pDev->cSocket, SOL_SOCKET, SO_SNDBUF,(char*)&nSendBufLen, &optlen); 
		WV_printf("SERVER cSocket SNDBUF  = %d.\r\n",nSendBufLen);
		WV_printf("SERVER  SOCKET  ACCEPT   OK\r\n");
 
		//pBuf[0] = 255;
		//pBuf[1] = 254;
		//pBuf[2] = 1;
		//pBuf[3] = 0; 
		ret = WV_TEL_Print(pDev->cSocket,pBuf); 
		if(ret != WV_SOK)
			{
			close(pDev->cSocket);
			pDev->cSocket = -1; 
			}
		//sleep(1);
	   // WV_S32 flags,flags_b;
		//flags_b = fcntl(pDev->cSocket, F_GETFL, 0);
		//flags |= O_NONBLOCK;
       // fcntl(pDev->cSocket,F_SETFL,flags);//no block
			
		 ret = recv(pDev->cSocket,pBuf,3,0);
		// printf("recev %d\r\n",ret);	
		// fcntl(pDev->cSocket,F_SETFL,flags_b);//no block
		
	 
		WV_CMD_ClrBye();
      sprintf(titel,"%s-%d:",WV_COMM_CMD_TITLE,pDev->cmdNum); 	        
	  WV_TEL_Print(pDev->cSocket,titel); 	 
		while( pDev ->cSocket >=  0 ) 
			{ 
			     if(WV_CMD_GetBye())
			     {
					close(pDev->cSocket);
					pDev->cSocket = -1; 
					break;
			     }
				//get cmd frame 
				ret =  WV_TEL_GetChar(pDev->cSocket,&charBuf);  
				if(ret != WV_SOK)
					{
						WV_printf("telnet  SOCKET  DISconet  GetCmd \r\n"); 
						close(pDev->cSocket);
						pDev->cSocket = -1; 
						break;
					} 
                 
				// proc   
				 ret = WV_CMD_PreProc (charBuf,cmd,buf);
			     if(ret == -1 )
					{
					continue;
					} 
				if(ret != 1 )
				{
			     cursorCnt = sprintf(titel,"\r%s-%d:%s",WV_COMM_CMD_TITLE,pDev->cmdNum,cmd);//print cmd input buff  
	             WV_TEL_Print(pDev->cSocket,titel);  
	             sprintf(titel,"\033[K");  //clear right  
	             WV_TEL_Print(pDev->cSocket,titel); 
			      continue;
				}
			     
				       WV_CMD_Proc(cmd,pBuf);        
		  
						sprintf(titel,"\r\n"); 
						WV_TEL_Print(pDev->cSocket,titel);
						WV_TEL_Print(pDev->cSocket,pBuf); 
						pDev-> cmdNum++;
						pBuf[0] = 0;
				 
					cmd[0]=0; 
				    sprintf(titel,"%s-%d:",WV_COMM_CMD_TITLE,pDev->cmdNum); 	        
					WV_TEL_Print(pDev->cSocket,titel); 
			} 
		  
	}
	 
	WV_TEL_DeInit(pDev); 
	pDev -> close  = 1;  

	return NULL; 
}



/********************************************************************

WV_S32  WV_TEL_Open();

********************************************************************/

WV_S32  WV_TEL_Open()
{

  pTelDev = (WV_TELNET_DEV_E  *)malloc(sizeof(WV_TELNET_DEV_E )); 
  memset(pTelDev,0,sizeof(WV_TELNET_DEV_E));   
  pTelDev -> pBuf = (WV_S8 *)malloc(WV_COMM_CMDPRT_MAXLEN);  
  pTelDev -> port = 2345;
  WV_CHECK_RET( WV_THR_Create(&(pTelDev->thrHndl),WV_TEL_Proc, WV_THR_PRI_DEFAULT, WV_THR_STACK_SIZE_DEFAULT, (void *)pTelDev));
  return WV_SOK;
}



/********************************************************************

WV_S32 WV_TEL_Close();

********************************************************************/

WV_S32  WV_TEL_Close()
{
  
   if(pTelDev->open == 1)
   {
   	  pTelDev->open = 0;
   	  while(pTelDev->close == 1) ;
      WV_THR_Destroy(&(pTelDev->thrHndl));
   }
   free (pTelDev -> pBuf);
   free (pTelDev); 
   return WV_SOK; 
}
