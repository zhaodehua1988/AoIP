#ifndef  _HIS_MODE_FB_H_H_
#define  _HIS_MODE_FB_H_H_
#include "wv_common.h" 
#include "hi_common.h"

#define HIS_FB_VITURE_SCEEN_W 1920
#define HIS_FB_VITURE_SCEEN_H 1080

WV_S32  HIS_FB_Open();
WV_S32  HIS_FB_Close();
WV_S32  HIS_FB_SurfGet( HI_HANDLE * pScreenSurf);
WV_S32  HIS_FB_SurfFresh();
WV_S32  HIS_FB_SetColorKey(WV_U32 ena,WV_U32 colorKey);
WV_S32  HIS_FB_Cursor();
#endif
