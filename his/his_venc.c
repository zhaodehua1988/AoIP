#include "hi_unf_venc.h"
#include "his_venc.h"


#define  HIS_CAST_WIDTH   	1280 
#define  HIS_CAST_HEIGHT   	720 

/****************************************************************************
WV_S32  HIS_VENC_Create(HI_HANDLE  *pHandlVenc,HI_HANDLE  *pHandlSrc);
****************************************************************************/

WV_S32  HIS_VENC_Create(HI_HANDLE  *pHandlVenc,HI_HANDLE  *pHandlSrc)
{
    HI_S32  ret = 0;
    HI_UNF_VENC_CHN_ATTR_S  stVencChnAttr;
    
    
    WV_RET_ADD( HI_UNF_VENC_GetDefaultAttr(&stVencChnAttr) ,ret);
    
    stVencChnAttr.enVencType = HI_UNF_VCODEC_TYPE_H264;
    stVencChnAttr.enCapLevel = HI_UNF_VCODEC_CAP_LEVEL_720P;
    stVencChnAttr.u32Width = HIS_CAST_WIDTH;
    stVencChnAttr.u32Height = HIS_CAST_HEIGHT;
    stVencChnAttr.u32StrmBufSize   = HIS_CAST_WIDTH * HIS_CAST_HEIGHT * 2;
    stVencChnAttr.u32TargetBitRate = 4 * 1024 * 1024; 
    stVencChnAttr.u32InputFrmRate  = 30;
    stVencChnAttr.u32TargetFrmRate = 30;
    
    WV_RET_ADD(HI_UNF_VENC_Create(pHandlVenc, &stVencChnAttr),ret);
 
    WV_RET_ADD(HI_UNF_VENC_AttachInput(*pHandlVenc,*pHandlSrc),ret);
  return   ret;
}


/****************************************************************************
WV_S32  HIS_VENC_Destroy(HI_HANDLE  *pHandlVenc);
****************************************************************************/

WV_S32  HIS_VENC_Destroy(HI_HANDLE  *pHandlVenc)
{
   HI_S32  ret = 0;
    
   WV_RET_ADD(HI_UNF_VENC_DetachInput(*pHandlVenc),ret);
   WV_RET_ADD(HI_UNF_VENC_Destroy (*pHandlVenc),ret);
   return ret; 
}

/********************************************************************************************

WV_S32  HIS_VENC_GetStream(HI_HANDLE  *pHandlVenc , HI_U8  *pBuf, HI_U32 *len, HI_U32 * ptsMs);

********************************************************************************************/
WV_S32  HIS_VENC_GetStream(HI_HANDLE  *pHandlVenc , HI_U8  *pBuf, HI_U32  *len, HI_U32 *ptsMs)
{
    HI_S32  ret = 0;
    HI_UNF_VENC_STREAM_S  stVencStream;
	ret = HI_UNF_VENC_AcquireStream(*pHandlVenc,&stVencStream,0);
	if ( HI_SUCCESS == ret )
	{ 
	  memcpy(pBuf+12,stVencStream.pu8Addr,stVencStream.u32SlcLen);
	 *len =  stVencStream.u32SlcLen +12;
	 *ptsMs =  stVencStream.u32PtsMs;
     *((WV_U64 *)(pBuf)) = ((WV_U64)(stVencStream.u32PtsMs)) * 1000; 
     *((WV_U32 *)(pBuf+8)) = stVencStream.u32SlcLen +12;
      HI_UNF_VENC_ReleaseStream(*pHandlVenc,&stVencStream); 
     // WV_printf("GetStream pack len [%d]  pts[ %d]  \r\n", stVencStream.u32SlcLen,stVencStream.u32PtsMs); 
      
	}
	
   return ret;	 
}

/****************************************************************************

WV_S32  HIS_VENC_Init(); 
 
****************************************************************************/

WV_S32  HIS_VENC_Init()
{
  return  HI_UNF_VENC_Init( );
}
/****************************************************************************

WV_S32  HIS_VENC_DeInit(); 
 
****************************************************************************/

WV_S32  HIS_VENC_DeInit()
{
  return  HI_UNF_VENC_DeInit();
}
