// printf wrapper that can be turned on and off by defining/undefining
#define WV_ESRX_DEBUG_MODE 
#ifdef WV_ESRX_DEBUG_MODE 
  #define WV_ESRX_printf(...)  do { printf("\n\rESRX:" __VA_ARGS__); fflush(stdout); } while(0)
#else 
  #define WV_ESRX_printf(...) 
#endif
 


#include "wv_esrx.h"
/***********************************************

WV_S32  WV_ESRX_Analyze(WV_S8 *pBuf,WV_ESRX_PACK_INFO_S *pInfo,WV_U32 len); 

************************************************/
WV_S32  WV_ESRX_Analyze(WV_S8 *pBuf,WV_ESRX_PACK_INFO_S *pInfo,WV_S32 len)
{


 // set all 0
 memset(pInfo,0,sizeof(struct WV_ESRX_PACK_INFO_S));
 // check sync is 0x47 
 if(pBuf[0] != 0x47 )
  {
    return WV_EFAIL;
  } 
  if(len < WV_ESRX_PAG_HEAD_LEN   ||  len > WV_ESRX_PAG_LEN)
  {
    return WV_EFAIL;
  }  
 pInfo->sync = 1;  
 if( (pBuf[1] &  0XF0)== 0X80)
 {
   pInfo->endFlag = 1;  
 } 
  // get payload start flag
  
 pInfo->pid  =  ((pBuf[1] & 0X1F)>>8)  |(pBuf[2] & 0XFF);
 
 //get cc 
 pInfo->cc  = pBuf[3] & 0XFF;
 
 pInfo->loadOffset   = WV_ESRX_PAG_HEAD_LEN ;
 pInfo->loadLen     = len - WV_ESRX_PAG_HEAD_LEN ; 
 pInfo->pLoad = pBuf +  WV_ESRX_PAG_HEAD_LEN;
 pInfo->pts = *((WV_U64 *)(pBuf+4));
 return 0;
   
}

