#ifndef  _HIS_MODE_FB_H_H_
#define  _HIS_MODE_FB_H_H_
#include "wv_common.h" 
#include "hi_common.h"
WV_S32  HIS_FB_Open();
WV_S32  HIS_FB_Close();
WV_S32  HIS_FB_SetFpga( WV_U8 * pCmdData ,WV_U8 cmdType, WV_U8 cmdChnl,WV_U32 cmdW ,WV_U32 cmdH); 
WV_S32  HIS_FB_ClrFpga(); 
WV_S32  HIS_FB_TestBuf( WV_U32 data);
WV_S32  HIS_FB_SurfGet( HI_HANDLE * pScreenSurf);
WV_S32  HIS_FB_SurfFresh();
WV_S32  HIS_FB_clrFillRect();
WV_S32  HIS_FB_SetColorKey(WV_U32 ena,WV_U32 colorKey);
WV_S32  HIS_FB_SurfGetForPic( HI_HANDLE * pScreenSurf);
WV_S32 HIS_FB_SetAlpha(WV_U8 alpha);
#endif
