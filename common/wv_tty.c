#include <termios.h>   
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include "wv_common.h"
#include "wv_cmd.h"

#define WV_TTY_PRFBUF_MAXLEN     WV_COMM_CMDPRT_MAXLEN
#define WV_TTY_PASKEY_MAXLEN     WV_COMM_ENVPASS_MAXLEN

typedef struct WV_TTY_DEV_E 
{
 WV_THR_HNDL_T hndl; 
 WV_U32      open;
 WV_U32      close; 
 WV_U32      cmdNum;  
 WV_S8       passWord[WV_TTY_PASKEY_MAXLEN];
 WV_S32      logEnd;
 WV_S32      logCnt;
 WV_S32      logNum; 
 
} WV_TTY_DEV_E ;

static    WV_S8  mPrfBuf[WV_TTY_PRFBUF_MAXLEN];
static    WV_TTY_DEV_E  mTtyDev;

/******************************************************************************

WV_S32  WV_TTY_GetChar();
******************************************************************************/

WV_S32  WV_TTY_GetChar() 
{   
        WV_S32 c=0;   
        struct termios orgOpts, newOpts;  
        //-----  store old settings -----------   
        WV_CHECK_RET( tcgetattr(STDIN_FILENO,&orgOpts) );   
          
        //---- set new terminal parms --------   
        memcpy(&newOpts, &orgOpts, sizeof(newOpts));   
        newOpts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL); 
      
        WV_CHECK_RET(tcsetattr(STDIN_FILENO, TCSANOW, &newOpts));   
        c=getchar(); 

        //------  restore old settings ---------   
        WV_CHECK_RET(tcsetattr(STDIN_FILENO, TCSANOW, &orgOpts)); 
        
        return c;   
}

/******************************************************************************

void *WV_TTY_Proc(Void * prm);
******************************************************************************/
void * WV_TTY_Proc(void * prm)
{   
	WV_S8 cmd[WV_CMD_BUF_MAXLEN],buf[20];
	WV_S32  c,ret;
	WV_U32 cursorCnt;
	mTtyDev.open   = 1; 
	mTtyDev.close  = 0;  
	mTtyDev.logNum = 0;
	mTtyDev.logCnt = 0; 
	mTtyDev.logEnd = 0; 
	mTtyDev.cmdNum = 0;
	cmd[0]=0;
	cursorCnt = printf("\r\n%s-%d:",WV_COMM_CMD_TITLE,mTtyDev.cmdNum); 
    while(mTtyDev.open == 1)
    {   
    	//
    	ret =WV_CMD_GetExit();
        if(ret == 1)  break;
        //
    	c =  WV_TTY_GetChar();
       // printf("char=[0x%2x]\r\n",c);
    	if(c < 0 ) continue; 
    	//
    	ret = WV_CMD_PreProc (c,cmd,buf);
    	if(ret == -1 )
    	{
    	 continue;
    	}
    	
    	//printf("\033[%dD",cursorCnt);  //cursor go to start
	   // printf("\033[K");  //clear line                                             			 
	   // cursorCnt = printf("%s-%d:%s",WV_COMM_CMD_TITLE,mTtyDev.cmdNum,cmd);//print cmd input buff    	 
    	
    	if(ret != 1 )
    	{
    	printf("\r%s-%d:%s",WV_COMM_CMD_TITLE,mTtyDev.cmdNum,cmd);//print cmd input buff  
    	printf("\033[K");  //clear line 
    	 continue;
    	}
    	//
       ret = WV_CMD_Proc(cmd,mPrfBuf);        
       // if(ret != WV_EFAIL)
        	//{  
        	    printf("\r\n%s",mPrfBuf);
           		mTtyDev.cmdNum++;
           		mPrfBuf[0] = 0;
    			
    		//}
    //	else
    		//{
    		//  printf("\r\n%s",mPrfBuf);
           	  //mTtyDev.cmdNum++;
           	//  mPrfBuf[0] = 0; 
    		//}	
    	cmd[0]=0;		        
        cursorCnt = printf("%s-%d:",WV_COMM_CMD_TITLE,mTtyDev.cmdNum); 
      
    }
    mTtyDev.open   = 0; 
	mTtyDev.close  = 1; 
return NULL;    
}


/******************************************************************************

WV_S32 WV_TTY_Create();

******************************************************************************/
WV_S32 WV_TTY_Create()
{
   
  memset(&mTtyDev,0,sizeof(mTtyDev)); 
  
   WV_CHECK_RET(WV_THR_Create(&mTtyDev.hndl,WV_TTY_Proc, WV_THR_PRI_DEFAULT, WV_THR_STACK_SIZE_DEFAULT, NULL));
   
 return 0;	
}


/******************************************************************************

WV_S32 WV_TTY_Destroy();

******************************************************************************/
WV_S32 WV_TTY_Destroy()
{
   
   if(mTtyDev.open == 1)
   {
   	  mTtyDev.open = 0;
   	  while(mTtyDev.close == 1)
   	   {
   	     usleep(10000);
   	   }
 	   WV_CHECK_RET( WV_THR_Destroy(&mTtyDev.hndl));	
   } 
 return 0;	
}
