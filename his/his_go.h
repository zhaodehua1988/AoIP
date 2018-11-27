#ifndef _WV_HIS_GO_H_H__
#define _WV_HIS_GO_H_H__
#include "wv_common.h"
#include "hi_go.h" 
#include "hi_common.h"
#include "his_fb.h"

#define HIS_HIGO_GODEC_NUM  16

typedef struct HIS_DEV_GODEC_E 
{ 
WV_S8    				ena;
WV_S8    				decFileName[256];
WV_S8    				decFileNameCur[266];
HI_HANDLE 				decHndl;
HI_HANDLE 				dstSurfHndl;
HI_HANDLE				bkSurfHndl;
HI_HANDLE				imgSurfHndl;
HIGO_DEC_PRIMARYINFO_S 	decInfo;
HIGO_DEC_IMGINFO_S 		imgInfoA;
HIGO_DEC_IMGINFO_S 		imgInfoB;
WV_U32			 		imgIndex;
WV_U32			 		imgIndexCur;
WV_S32    				imgDelay;
WV_S32             		disX;
WV_S32             		disY; 
WV_S32             		disW;
WV_S32             		disH; 
WV_S32             		bkDisX;
WV_S32             		bkDisY;
WV_S32             		bkDisW;
WV_S32             		bkDisH;
WV_U32			 		imgStamp;
WV_U32			 		colorKeyEna;
WV_U32                  imgClosed; // 1 closed 0 notclose

} HIS_DEV_GODEC_E; 


typedef struct HIS_DEV_HIGO_E 
{ 
WV_S32             		layerId;  
HI_HANDLE 				layerSurfHndl;
WV_S32             		layerX;
WV_S32             		layerY;
WV_S32             		layerW;
WV_S32             		layerH;
WV_S32             		bkColor;
HIS_DEV_GODEC_E         imgDev[HIS_HIGO_GODEC_NUM]; 
} HIS_DEV_HIGO_E; 


WV_S32 HIS_GO_Init();
WV_S32 HIS_GO_DeInit();
WV_S32 HIS_GO_GetConf(HIS_DEV_HIGO_E * pDev,WV_S32  id);

WV_S32 HIS_GO_SetPos(HIS_DEV_HIGO_E * pDev, WV_S32 x,WV_S32 y);
WV_S32 HIS_GO_SetSize(HIS_DEV_HIGO_E * pDev, WV_S32 w,WV_S32 h);
WV_S32 HIS_GO_Open(HIS_DEV_HIGO_E * pDev);
WV_S32 HIS_GO_Close(HIS_DEV_HIGO_E * pDev);

WV_S32 HIS_GODEC_Open(HIS_DEV_GODEC_E * pDev,WV_S8 *pFileName,HI_HANDLE  dstSurfHndl);
WV_S32 HIS_GODEC_Close(HIS_DEV_GODEC_E * pDev );

WV_S32 HIS_GO_RefreshLayer(HIS_DEV_HIGO_E * pDev);
WV_S32 HIS_GODEC_Render(HIS_DEV_GODEC_E * pDev);
WV_S32 HIS_GO_PrintLayer(HIS_DEV_HIGO_E * pDev, WV_S8 * pBuf);
WV_S32 HIS_GO_PrintDec(HIS_DEV_GODEC_E * pDev, WV_S8 * pBuf);

WV_S32 HIS_GODEC_ShowPic(HIS_DEV_GODEC_E * pDev,WV_S32 i);
WV_S32 HIS_GO_SetColorKey(WV_U32 ena,WV_U32 colorKey);

#endif
