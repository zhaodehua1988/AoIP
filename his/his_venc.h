#ifndef  _HIS_VENC_VENC_H_H_
#define  _HIS_VENC_VENC_H_H_
#include "hi_type.h" 
#include "wv_common.h" 
WV_S32  HIS_VENC_Init();
WV_S32  HIS_VENC_DeInit();
WV_S32  HIS_VENC_Create(HI_HANDLE  *pHandlVenc ,HI_HANDLE  *pHandlSrc);
WV_S32  HIS_VENC_Destroy(HI_HANDLE  *pHandlVenc);
WV_S32  HIS_VENC_GetStream(HI_HANDLE  *pHandlVenc , HI_U8  *pBuf, HI_U32 *len, HI_U32 * ptsMs);
#endif
