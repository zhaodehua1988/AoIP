#include "his_dis.h"
#include "his_avplay.h"  
#include "his_player.h" 
#include "tsk_player.h"
#include "hi_unf_avplay.h"
#include "sys/time.h"
#include "hi_unf_common.h"
#include "hi_unf_avplay.h"
#include "hi_unf_sound.h"
#include "hi_unf_disp.h"
#include "hi_unf_vo.h" 
#include "his_fb.h"

#include "hi_unf_common.h"
#include "hi_unf_ecs.h"
#include "hi_unf_avplay.h"
#include "hi_unf_sound.h"
#include "hi_unf_disp.h"
#include "hi_unf_vo.h"
#include "hi_unf_demux.h"
#include "wv_cmd.h"
#include "his_dis.h"
typedef struct TSK_PLAYER_CHNL_E 
{

    HI_HANDLE   playerHandl;
    HI_HANDLE   avPlayHandl;
    HI_UNF_VCODEC_TYPE_E vEncType;
    HI_HANDLE   winHandl;
    HI_RECT_S   winRect;
    WV_S8       fileName[128];

} TSK_PLAYER_CHNL_E; 

TSK_PLAYER_CHNL_E gTskPlayer;


/****************************************************************************

WV_S32 TSK_GO_GetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 TSK_WIN_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{

    WV_U32 val;
    if (argc < 1)
	{

		prfBuff += sprintf(prfBuff, "set win <cmd> //cmd like:freeze\r\n");
		return 0;
	}

	if (strcmp(argv[0], "freeze") == 0)
	{
		if (argc < 2)
		{
			prfBuff += sprintf(prfBuff, "set win freeze <val>//val like:0,1\r\n");
		}
        WV_S32 ret;
        ret = WV_STR_S2v(argv[1], &val);
		if (ret != WV_SOK)
		{
			prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
		}

        if(val == 0 || val == 1){
            HIS_DIS_WinFreeze(&gTskPlayer.winHandl,val,0);
        }else{
            prfBuff += sprintf(prfBuff, "input erro!val=%d\r\n",val);
        }

    }
    return WV_SOK;
}

/***********************************************************************
WV_S32  TSK_PLAYER_Open();
***********************************************************************/

WV_S32  TSK_PLAYER_Open()
{ 
    gTskPlayer.vEncType = HI_UNF_VCODEC_TYPE_HEVC;
    gTskPlayer.winRect.s32X = 0;
    gTskPlayer.winRect.s32Y = 0;
    gTskPlayer.winRect.s32Width = 3840;
    gTskPlayer.winRect.s32Height = 2160;
    sprintf(gTskPlayer.fileName, "%s", "./mov/mov0.mp4");

    HIS_PLAYER_Init();
    WV_CHECK_RET(  HIS_AVP_EsOpen(&(gTskPlayer.avPlayHandl),gTskPlayer.vEncType,0));
    WV_CHECK_RET(  HIS_DIS_WinCreat(&(gTskPlayer.winHandl),gTskPlayer.winRect));
    WV_CHECK_RET(  HIS_DIS_WinAttach(&(gTskPlayer.winHandl),&(gTskPlayer.avPlayHandl)));
    WV_CHECK_RET(  HIS_DIS_WinStart(&(gTskPlayer.winHandl)));

    //开始播放
    WV_CHECK_RET(  HIS_PLAYER_Create(&(gTskPlayer.playerHandl) ,&(gTskPlayer.avPlayHandl)));
    WV_CHECK_RET(  HIS_PLAYER_Start(&(gTskPlayer.playerHandl) ,gTskPlayer.fileName));

    HIS_AVP_SetVolume(0,0);

	//WV_CMD_Register("get", "higo", "get higo ", TSK_GO_GetCmd);
	WV_CMD_Register("set", "win", "set win freze ", TSK_WIN_SetCmd);

    return WV_SOK;
}


/***********************************************************************
WV_S32  TSK_PLAYER_Close();
***********************************************************************/

WV_S32  TSK_PLAYER_Close()
{ 
    HIS_PLAYER_Stop(&gTskPlayer.playerHandl);
    HIS_PLAYER_Destory(&(gTskPlayer.playerHandl));
    HIS_DIS_WinFreeze(&(gTskPlayer.winHandl),HI_TRUE,0);
    HIS_DIS_WinStop(&(gTskPlayer.winHandl));
    HIS_DIS_WinDetach(&(gTskPlayer.winHandl),&(gTskPlayer.avPlayHandl));
    HIS_DIS_WinDestroy(&(gTskPlayer.winHandl));
    HIS_AVP_Close(&(gTskPlayer.avPlayHandl),0);
    HIS_PLAYER_DeInit();
    return WV_SOK;
}


