#ifndef _HIS_MODE_AVPLAYER_H_H_
#define _HIS_MODE_AVPLAYER_H_H_
#include "wv_common.h"
#include "hi_unf_video.h"
#include "hi_unf_sound.h"

WV_S32  HIS_AVP_Init();
WV_S32  HIS_AVP_DeInit();
WV_S32  HIS_AVP_EsOpen(HI_HANDLE *avHandl,HI_UNF_VCODEC_TYPE_E vDecType,WV_U32 playerID);
WV_S32  HIS_AVP_Close(HI_HANDLE *avHandl,WV_U32 playerID);
WV_S32  HIS_AVP_SetVolume(WV_U32 playerID,WV_S32 volume);
WV_S32  HIS_AVP_GetVolume(WV_U32 playerID,WV_S32 *pVolume);
#endif
