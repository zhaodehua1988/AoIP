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
#include "wv_file.h"
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

/****************************************************************************

WV_S32 TSK_GO_GetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 TSK_Player_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{

    WV_U32 w,h;
    WV_S32 ret;
    if (argc < 2)
	{

		prfBuff += sprintf(prfBuff, "set player <w> <h> <name> //cmd like:set player 1920 1080 ./mov/mov0.mp4 \r\n");
		return 0;
	}
    ret = WV_STR_S2v(argv[0], &w);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
        return WV_SOK;
    }
    ret = WV_STR_S2v(argv[1], &h);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
        return WV_SOK;
    }

    if(argc >= 3)
    {
        if(WV_FILE_Access(argv[2]) == 0)
        {
            sprintf(gTskPlayer.fileName, "%s", argv[2]);
        }
    }

    gTskPlayer.winRect.s32Width = w;
    gTskPlayer.winRect.s32Height = h;
    TSK_PLAYER_Destory();
    TSK_PLAYER_Create();

    return WV_SOK;

}

/****************************************************************************

WV_S32 TSK_GO_GetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 TSK_Player_SetVolCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{

    WV_U32 rate,data;
    WV_S32 ret;
    if (argc < 1)
	{

		prfBuff += sprintf(prfBuff, "set audio <cmd> ////cmd like:rate/vol \r\n");
        prfBuff += sprintf(prfBuff, "<rate> like : 0<default> 1<8K>  2<11.025K>  3<12K>  4<16K>  5<22.05K>  6<24K>  7<32K>  8<44.1K>  9<48K>  10<88.2K>  11<96K>  12<176.4K> 13<192K> \r\n");
        prfBuff += sprintf(prfBuff, "vol = [0,100] \r\n");
		return 0;
	}
    if(strcmp(argv[0],"rate") == 0){
        if(argc < 2){
            prfBuff += sprintf(prfBuff, "cmd like: set audio rate 9 \r\n");
            return WV_SOK;
        }
        ret = WV_STR_S2v(argv[1], &rate);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        if(rate > 13){
            prfBuff += sprintf(prfBuff, "input erro! rate > 13\r\n");
            return WV_SOK;       
        }

        switch(rate){
            case 0:
                prfBuff += sprintf(prfBuff, "set audio rate=[0]default \r\n");
                data = 0;
                break;
            case 1:
                prfBuff += sprintf(prfBuff, "set audio rate=[1] 8KHz    \r\n");
                data = 8000;
                break;
            case 2:
                prfBuff += sprintf(prfBuff, "set audio rate=[2] 11.025KHz    \r\n");
                data = 11025;
                break;
            case 3:
                prfBuff += sprintf(prfBuff, "set audio rate=[3] 12KHz    \r\n");
                data = 12000;
                break;
            case 4:
                prfBuff += sprintf(prfBuff, "set audio rate=[4] 16KHz    \r\n");
                data = 16000;
                break;
            case 5:
                prfBuff += sprintf(prfBuff, "set audio rate=[5] 22.05KHz    \r\n");
                data = 22050;
                break;
            case 6:
                prfBuff += sprintf(prfBuff, "set audio rate=[6] 24KHz    \r\n");
                data = 24000;
                break;
            case 7:
                prfBuff += sprintf(prfBuff, "set audio rate=[7] 32KHz    \r\n");
                data = 32000;
                break;
            case 8:
                prfBuff += sprintf(prfBuff, "set audio rate=[8] 44.1KHz    \r\n");
                data = 44100;
                break;
            case 9:
                prfBuff += sprintf(prfBuff, "set audio rate=[9] 48KHz    \r\n");
                data = 48000;
                break;
            case 10:
                prfBuff += sprintf(prfBuff, "set audio rate=[10] 88.2KHz    \r\n");
                data = 88200;
                break;
            case 11:
                prfBuff += sprintf(prfBuff, "set audio rate=[11] 96KHz    \r\n");
                data = 96000;
                break;
            case 12:
                prfBuff += sprintf(prfBuff, "set audio rate=[12] 176.4KHz    \r\n");
                data = 176000;
                break;
            case 13:
                prfBuff += sprintf(prfBuff, "set audio rate=[13] 192KHz    \r\n");
                data = 192000;
                break;
            default:
                data = 48000;
                prfBuff += sprintf(prfBuff, "set audio rate=[%d]default    \r\n",data);

                break;                                              
        }
        HIS_AVP_SetSampleRate(0,data);
    }else if(strcmp(argv[0],"vol") == 0){
        if(argc < 2){
            prfBuff += sprintf(prfBuff, "cmd like: set audio vol 50  \r\n");
        }
        
        ret = WV_STR_S2v(argv[1], &data);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        if(data >100)
        {
             prfBuff += sprintf(prfBuff, "input erro! vol(%d) > 100!!\r\n",data);
            return WV_SOK;           
        }

        HIS_AVP_SetVolume(0,data);
    }

    return WV_SOK;

}

/****************************************************************************

WV_S32 TSK_GO_GetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 TSK_Player_GetVolCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
    WV_U32 rate,bitDept;
    WV_S32 vol;
    WV_S32 ret;
    ret = HIS_AVP_GetSampleRate(0,&rate);
    if(ret != 0 ){
         prfBuff += sprintf(prfBuff, "get vol rate  erro! \r\n");
        return WV_SOK;  
    }

    ret = HIS_AVP_GetStreamInfo(gTskPlayer.avPlayHandl,&bitDept);
    ret = HIS_AVP_GetVolume(0,&vol);
    prfBuff += sprintf(prfBuff, "get audio rate[%d], bitDepth=[%d],  vol[%d] \r\n",rate,bitDept,vol);

    return WV_SOK;
}


/***********************************************************************
WV_S32  TSK_PLAYER_Create();
***********************************************************************/

WV_S32  TSK_PLAYER_Create()
{

    //HIS_PLAYER_Init();
    WV_CHECK_RET(  HIS_AVP_EsOpen(&(gTskPlayer.avPlayHandl),gTskPlayer.vEncType,0));
    WV_CHECK_RET(  HIS_DIS_WinCreat(&(gTskPlayer.winHandl),gTskPlayer.winRect));
    WV_CHECK_RET(  HIS_DIS_WinAttach(&(gTskPlayer.winHandl),&(gTskPlayer.avPlayHandl)));
    WV_CHECK_RET(  HIS_DIS_WinStart(&(gTskPlayer.winHandl)));
    //开始播放
    WV_CHECK_RET(  HIS_PLAYER_Create(&(gTskPlayer.playerHandl) ,&(gTskPlayer.avPlayHandl)));
    WV_CHECK_RET(  HIS_PLAYER_Start(&(gTskPlayer.playerHandl) ,gTskPlayer.fileName));
    printf("create,handle = %d ",gTskPlayer.playerHandl);
    HIS_AVP_SetVolume(0,50);


    //HIS_AVP_SetSampleRate(0,9);
    return WV_SOK;

}
/***********************************************************************
WV_S32  TSK_PLAYER_Destory();
***********************************************************************/

WV_S32  TSK_PLAYER_Destory()
{   printf("destory,handle = %d ",gTskPlayer.playerHandl);
    HIS_PLAYER_Stop(&gTskPlayer.playerHandl);
    HIS_PLAYER_Destory(&(gTskPlayer.playerHandl));
    HIS_DIS_WinFreeze(&(gTskPlayer.winHandl),HI_TRUE,0);
    HIS_DIS_WinStop(&(gTskPlayer.winHandl));
    HIS_DIS_WinDetach(&(gTskPlayer.winHandl),&(gTskPlayer.avPlayHandl));
    HIS_DIS_WinDestroy(&(gTskPlayer.winHandl));
    HIS_AVP_Close(&(gTskPlayer.avPlayHandl),0);
    //HIS_PLAYER_DeInit();
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
    gTskPlayer.winRect.s32Width = 1920;
    gTskPlayer.winRect.s32Height = 2160;
    sprintf(gTskPlayer.fileName, "%s", "./mov/mov0.mp4");


    HIS_PLAYER_Init();
    TSK_PLAYER_Create();
	//WV_CMD_Register("get", "higo", "get higo ", TSK_GO_GetCmd);
	WV_CMD_Register("set", "win", "set win freze ", TSK_WIN_SetCmd);
    WV_CMD_Register("set", "player", "set player ", TSK_Player_SetCmd);
    WV_CMD_Register("set", "audio", "set vol rate ", TSK_Player_SetVolCmd);
    WV_CMD_Register("get", "audio", "get vol rate ", TSK_Player_GetVolCmd);
    return WV_SOK;
}


/***********************************************************************
WV_S32  TSK_PLAYER_Close();
***********************************************************************/

WV_S32  TSK_PLAYER_Close()
{ 
    TSK_PLAYER_Destory();
    HIS_PLAYER_DeInit();
    return WV_SOK;
}


