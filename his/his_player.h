#ifndef _HIS_PLAYER_H_H_
#define _HIS_PLAYER_H_H_
#include "wv_common.h"
#include "hi_type.h"
WV_S32  HIS_PLAYER_Init();
WV_S32  HIS_PLAYER_DeInit();
WV_S32  HIS_PLAYER_Create(HI_HANDLE  * pHndlPlayer, HI_HANDLE * pHndlAVplay);
WV_S32  HIS_PLAYER_Destory(HI_HANDLE  * pHndlPlayer);
WV_S32  HIS_PLAYER_Start(HI_HANDLE  * pHndlPlayer ,WV_S8 * pFileName);
WV_S32  HIS_PLAYER_Play(HI_HANDLE  * pHndlPlayer);
WV_S32  HIS_PLAYER_Pause(HI_HANDLE  * pHndlPlayer);
WV_S32  HIS_PLAYER_Resume(HI_HANDLE  * pHndlPlayer);
WV_S32  HIS_PLAYER_Stop(HI_HANDLE  * pHndlPlayer );
WV_S32  HIS_PLAYER_Seek(HI_HANDLE  * pHndlPlayer,HI_U64 s64TimeInMs);
WV_S32  HIS_PLAYER_GetTimePlayed(HI_HANDLE  * pHndlPlayer ,HI_U64  *u64TimePlayed);
WV_S32  HIS_PLAYER_Replay(HI_HANDLE  * pHndlPlayer);
WV_S32  HIS_PLAYER_playRoll(HI_HANDLE  * pHndlPlayer);
WV_S32  HIS_PLAYER_PlayRandom(HI_HANDLE  * pHndlPlayer);
#endif
