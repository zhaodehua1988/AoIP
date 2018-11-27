#ifndef _HIS_MODE_DISPLAY_H_H_
#define _HIS_MODE_DISPLAY_H_H_
#include "wv_common.h"
#include "hi_unf_video.h"
WV_S32  HIS_DIS_Init(HI_UNF_ENC_FMT_E disFormat,WV_S32 mode);
WV_S32  HIS_DIS_DeInit();

WV_S32  HIS_DIS_WinCreat(HI_HANDLE *  pHndlWin,HI_RECT_S  winRect);
WV_S32  HIS_DIS_WinDestroy(HI_HANDLE *  pHndlWin);

WV_S32  HIS_DIS_WinStart(HI_HANDLE *  pHndlWin);
WV_S32  HIS_DIS_WinStop(HI_HANDLE *  pHndlWin);


WV_S32  HIS_DIS_WinAttach(HI_HANDLE *  pHndlWin,HI_HANDLE *  pHndlSrc);
WV_S32  HIS_DIS_WinDetach(HI_HANDLE *  pHndlWin,HI_HANDLE *  pHndlSrc);

WV_S32  HIS_DIS_InitCast(HI_HANDLE * pHandlCast);
WV_S32  HIS_DIS_DeInitCast(HI_HANDLE * pHandlCast); 
WV_S32  HIS_DIS_StartCast(HI_HANDLE handlCast);
WV_S32  HIS_DIS_EndCast(HI_HANDLE handlCast);
WV_S32  HIS_DIS_SetVirtualScreen(WV_U32 mode);
WV_S32  HIS_DIS_SetCustomTiming(WV_U32 mode);

WV_S32  HIS_DIS_WinFreeze(HI_HANDLE *  pHndlWin,HI_BOOL bEnable,WV_S32 mode);
WV_S32  HIS_DIS_GetWinFreezeStatus(HI_HANDLE * pHndlWin,HI_BOOL *pbEnable);
WV_S32  HIS_DIS_GetWinFreezeStatus(HI_HANDLE * pHndlWin,HI_BOOL *pbEnable);
#endif
