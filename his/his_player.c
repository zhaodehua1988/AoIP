#include"his_player.h"
#include "hi_svr_player.h" 

#include "hi_audio_codec.h"
#include "hi_unf_common.h"
#include "hi_unf_avplay.h"
#include "hi_unf_demux.h"
#include "hi_unf_so.h"
#include "hi_unf_sound.h"
#include "hi_unf_disp.h"
#include "hi_unf_vo.h"
#include "hi_go.h"
#include "hi_svr_format.h"
#include "hi_svr_metadata.h"
#include "hi_svr_assa.h"
#include "hi_unf_edid.h"
#include "tsk_player.h"
#include <time.h>
#define  HIS_PLAYER_STATUS_INIT          	0
#define  HIS_PLAYER_STATUS_DEINIT        	1
#define  HIS_PLAYER_STATUS_PLAY        		2
#define  HIS_PLAYER_STATUS_FORWAR        	3
#define  HIS_PLAYER_STATUS_BACKWARD        	4
#define  HIS_PLAYER_STATUS_PAUSE        	5
#define  HIS_PLAYER_STATUS_STOP        		6
#define  HIS_PLAYER_STATUS_BUTT        		7

#define HIS_PLAYER_MOV_MAXNUM 128
typedef struct HIS_PLAYER_MOVFILE
{
    WV_S8 name[30];
}HIS_PLAYER_MOVFILE;

//static HIS_PLAYER_MOVFILE gHisPlayMov[HIS_PLAYER_MOV_MAXNUM];
/***************************************************************

WV_S32  HIS_PLAYER_Init();

***************************************************************/

WV_S32  HIS_PLAYER_Init()
{
    WV_S32  s32Ret;
    s32Ret = HI_SVR_PLAYER_Init();

    if (HI_SUCCESS != s32Ret)
    {
        WV_ERROR("HI_SVR_PLAYER_Init() fail, ret = 0x%x \e[0m \n", s32Ret);
        return WV_EFAIL;
    }

    s32Ret = HI_SVR_PLAYER_RegisterDynamic(HI_SVR_PLAYER_DLL_PARSER, "libformat.so");

    if (HI_SUCCESS != s32Ret)
    {
        WV_ERROR(" register file parser libformat.so fail, ret = 0x%x \e[0m \n", s32Ret);
    }
    s32Ret = HI_SVR_PLAYER_RegisterDynamic(HI_SVR_PLAYER_DLL_PARSER, "libffmpegformat.so");

    if (HI_SUCCESS != s32Ret)
    {
        WV_ERROR(" register file parser libffmpegformat.so fail, ret = 0x%x \e[0m \n", s32Ret);
    }

    return s32Ret;
}

/***************************************************************

WV_S32  HIS_PLAYER_DeInit();

***************************************************************/

WV_S32  HIS_PLAYER_DeInit()
{
    WV_S32  s32Ret;
    s32Ret = HI_SVR_PLAYER_UnRegisterDynamic(HI_SVR_PLAYER_DLL_PARSER, NULL);

    if (HI_SUCCESS != s32Ret)
    {
        WV_ERROR(" ### HI_SVR_PLAYER_UnRegisterDynamic err! \n");
    }

    s32Ret = HI_SVR_PLAYER_Deinit();

    if (HI_SUCCESS != s32Ret)
    {
        WV_ERROR(" ### HI_SVR_PLAYER_Deinit err! \n");
    }

    return WV_SOK;
}

static HI_CHAR *getVidFormatStr(HI_U32 u32Format)
{
    switch (u32Format)
    {
    case HI_FORMAT_VIDEO_MPEG2:
        return "MPEG2";
        break;

    case HI_FORMAT_VIDEO_MPEG4:
        return "MPEG4";
        break;

    case HI_FORMAT_VIDEO_AVS:
        return "AVS";
        break;

    case HI_FORMAT_VIDEO_H263:
        return "H263";
        break;

    case HI_FORMAT_VIDEO_H264:
        return "H264";
        break;

    case HI_FORMAT_VIDEO_REAL8:
        return "REAL8";
        break;

    case HI_FORMAT_VIDEO_REAL9:
        return "REAL9";
        break;

    case HI_FORMAT_VIDEO_VC1:
        return "VC1";
        break;

    case HI_FORMAT_VIDEO_VP6:
        return "VP6";
        break;

    case HI_FORMAT_VIDEO_DIVX3:
        return "DIVX3";
        break;

    case HI_FORMAT_VIDEO_RAW:
        return "RAW";
        break;

    case HI_FORMAT_VIDEO_JPEG:
        return "JPEG";
        break;

    case HI_FORMAT_VIDEO_MJPEG:
        return "MJPEG";
        break;
    case HI_FORMAT_VIDEO_MJPEGB:
        return "MJPEGB";
        break;
    case HI_FORMAT_VIDEO_SORENSON:
        return "SORENSON";
        break;

    case HI_FORMAT_VIDEO_VP6F:
        return "VP6F";
        break;

    case HI_FORMAT_VIDEO_VP6A:
        return "VP6A";
        break;

    case HI_FORMAT_VIDEO_VP8:
        return "VP8";
        break;
    case HI_FORMAT_VIDEO_MVC:
        return "MVC";
        break;
    case HI_FORMAT_VIDEO_SVQ1:
        return "SORENSON1";
        break;
    case HI_FORMAT_VIDEO_SVQ3:
        return "SORENSON3";
        break;
    case HI_FORMAT_VIDEO_DV:
        return "DV";
        break;
    case HI_FORMAT_VIDEO_WMV1:
        return "WMV1";
        break;
    case HI_FORMAT_VIDEO_WMV2:
        return "WMV2";
        break;
    case HI_FORMAT_VIDEO_MSMPEG4V1:
        return "MICROSOFT MPEG4V1";
        break;
    case HI_FORMAT_VIDEO_MSMPEG4V2:
        return "MICROSOFT MPEG4V2";
        break;
    case HI_FORMAT_VIDEO_CINEPAK:
        return "CINEPACK";
        break;
    case HI_FORMAT_VIDEO_RV10:
        return "RV10";
        break;
    case HI_FORMAT_VIDEO_RV20:
        return "RV20";
        break;
    case HI_FORMAT_VIDEO_INDEO4:
        return "INDEO4";
        break;
    case HI_FORMAT_VIDEO_INDEO5:
        return "INDEO5";
        break;
    case HI_FORMAT_VIDEO_HEVC:
        return "h265";
    default:
        return "UN-KNOWN";
        break;
    }

    return "UN-KNOWN";
}



/***************************************************************

static WV_S32 HIS_PLAYER_CallBack(HI_HANDLE hPlayer, HI_SVR_PLAYER_EVENT_S *pstruEvent);

***************************************************************/
static WV_S32 HIS_PLAYER_CallBack(HI_HANDLE hPlayer, HI_SVR_PLAYER_EVENT_S *pstruEvent)
{

    HI_SVR_PLAYER_STATE_E eEventBk = HI_SVR_PLAYER_STATE_BUTT;
    HI_SVR_PLAYER_STREAMID_S *pstSreamId = NULL;
    HI_HANDLE hWindow = (HI_HANDLE)NULL;
    HI_FORMAT_FILE_INFO_S *pstFileInfo;
    
    static HI_SVR_PLAYER_STATE_E ePlayerState = HI_SVR_PLAYER_STATE_BUTT;
    static HI_S32 s32PausedCtrl = 0;
    static HI_SVR_PLAYER_STREAMID_S s_stStreamID = {
        0,
        0,
        0,
        0
    };

    //printf("hi player[%d] event is [%d]\n ",hPlayer,(HI_SVR_PLAYER_EVENT_E)pstruEvent->eEvent);

    static HI_BOOL s_noPrintInHdmiATC = HI_FALSE;
    if (0 == hPlayer || NULL == pstruEvent)
    {
        //printf("0 == hPlayer || NULL == pstruEvent \n");
        return HI_SUCCESS;
    }

    if (HI_SVR_PLAYER_EVENT_STATE_CHANGED == (HI_SVR_PLAYER_EVENT_E)pstruEvent->eEvent)
    {
        eEventBk = (HI_SVR_PLAYER_STATE_E)*pstruEvent->pu8Data;
        //WV_printf("[0x%x]Status change to %d \n", hPlayer, eEventBk);
        ePlayerState = eEventBk;

        if (eEventBk == HI_SVR_PLAYER_STATE_STOP)
        {
            // WV_printf("[0x%x]Status change to %d \n", hPlayer,eEventBk);
            HI_SVR_PLAYER_Seek(hPlayer, 0);
            HI_SVR_PLAYER_GetParam(hPlayer, HI_SVR_PLAYER_ATTR_WINDOW_HDL, &hWindow);
            //HI_UNF_VO_ResetWindow(hWindow, HI_UNF_WINDOW_FREEZE_MODE_BLACK);
            HI_UNF_VO_ResetWindow(hWindow, HI_UNF_WINDOW_FREEZE_MODE_LAST );

        }

        if (eEventBk == HI_SVR_PLAYER_STATE_PAUSE && s32PausedCtrl == 1)
        {
            s32PausedCtrl = 2;
        }
        else
        {
            s32PausedCtrl = 0;
        }


    }
    else if (HI_SVR_PLAYER_EVENT_SOF == (HI_SVR_PLAYER_EVENT_E)pstruEvent->eEvent)
    {
        //WV_printf("[0x%x]:File position is start of file! \n",hPlayer);
        /* 快退到头重新启动播放 */
        eEventBk = (HI_SVR_PLAYER_STATE_E)*pstruEvent->pu8Data;

        if (HI_SVR_PLAYER_STATE_BACKWARD == eEventBk)
        {
            //WV_printf("[0x%x]backward to start of file, start play! \n",hPlayer);
            HI_SVR_PLAYER_Play(hPlayer);
        }
    }
    else if (HI_SVR_PLAYER_EVENT_EOF == (HI_SVR_PLAYER_EVENT_E)pstruEvent->eEvent)
    {
          HI_SVR_PLAYER_Play(hPlayer);
    }
    else if (HI_SVR_PLAYER_EVENT_STREAMID_CHANGED == (HI_SVR_PLAYER_EVENT_E)pstruEvent->eEvent)
    {
        //WV_printf("[0x%x]:HI_SVR_PLAYER_EVENT_STREAMID_CHANGED \n",hPlayer);
        pstSreamId = (HI_SVR_PLAYER_STREAMID_S*)pstruEvent->pu8Data;

        if (NULL != pstSreamId)
        {
            if (s_stStreamID.u16SubStreamId != pstSreamId->u16SubStreamId ||
                    s_stStreamID.u16ProgramId != pstSreamId->u16ProgramId)
            {
                // clearSubtitle();
                s_stStreamID = *pstSreamId;
            }

           /*  WV_printf("Stream id change to: ProgramId %d, vid %d, aid %d, sid %d \n",\
            pstSreamId->u16ProgramId,\
                    pstSreamId->u16VidStreamId,\
                    pstSreamId->u16AudStreamId,\
                    pstSreamId->u16SubStreamId); */
        }
    }
    else if (HI_SVR_PLAYER_EVENT_PROGRESS == (HI_SVR_PLAYER_EVENT_E)pstruEvent->eEvent)
    {
       // printf("HI_SVR_PLAYER_EVENT_PROGRESS == (HI_SVR_PLAYER_EVENT_E)pstruEvent->eEvent \n");
        HI_SVR_PLAYER_PROGRESS_S stProgress;

        memcpy(&stProgress, pstruEvent->pu8Data, pstruEvent->u32Len);
        if(s_noPrintInHdmiATC == HI_FALSE)
        {
            /*  WV_printf("Current progress is %d, Duration:%lld ms,Buffer size:%lld bytes\n",\
            stProgress.u32Progress, stProgress.s64Duration,stProgress.s64BufferSize); */
        }
    }
    else if (HI_SVR_PLAYER_EVENT_ERROR == (HI_SVR_PLAYER_EVENT_E)pstruEvent->eEvent)
    {
        //WV_printf("[0x%x]:HI_SVR_PLAYER_EVENT_STREAMID_ERROR \n",hPlayer);
        HI_S32 s32SysError = (HI_S32)*pstruEvent->pu8Data;

        if (HI_SVR_PLAYER_ERROR_VID_PLAY_FAIL == s32SysError)
        {
            WV_printf("\e[31m ERR: Vid start fail! \e[0m \n");
        }
        else if (HI_SVR_PLAYER_ERROR_AUD_PLAY_FAIL == s32SysError)
        {
            // WV_printf("\e[31m ERR: Aud start fail! \e[0m \n");
        }
        else if (HI_SVR_PLAYER_ERROR_PLAY_FAIL == s32SysError)
        {
            WV_printf("\e[31m ERR: Play fail! \e[0m \n");
        }
        else if (HI_SVR_PLAYER_ERROR_NOT_SUPPORT == s32SysError)
        {

            WV_printf("\e[31m ERR: Not support! \e[0m \n");
        }
        else if (HI_SVR_PLAYER_ERROR_TIMEOUT == s32SysError)
        {
  
        }
        else
        {
            WV_printf("unknow Error = 0x%x \n", s32SysError);
        }
    }
    else if (HI_SVR_PLAYER_EVENT_NETWORK_INFO == (HI_SVR_PLAYER_EVENT_E)pstruEvent->eEvent)
    {
       // WV_printf("[0x%x]:HI_SVR_PLAYER_EVENT_NETWORK_INFO \n",hPlayer);
        //HI_FORMAT_NET_STATUS_S *pstNetStat = (HI_FORMAT_NET_STATUS_S*)pstruEvent->pu8Data;
        /*  WV_printf("HI_SVR_PLAYER_EVNET_NETWORK_INFO: type:%d, code:%d, str:%s\n",\
        pstNetStat->eType, pstNetStat->s32ErrorCode, pstNetStat->szProtocals); */
        /*
        if (pstNetStat->eType == HI_FORMAT_MSG_NETWORK_ERROR_DISCONNECT)
        {
            s32Ret = pthread_create(&s_hThread, HI_NULL, reconnect, (HI_VOID*)hPlayer);
            if (s32Ret == HI_SUCCESS)
            {
                WV_printf("create thread:reconnect successfully\n");
            }
            else
            {
                 WV_printf("ERR:failed to create thread:reconnect\n");
            }
        }
       */
    }
    else if (HI_SVR_PLAYER_EVENT_FIRST_FRAME_TIME == (HI_SVR_PLAYER_EVENT_E)pstruEvent->eEvent)
    {
        //HI_U32 u32Time = *((HI_U32*)pstruEvent->pu8Data);
       // WV_printf("[0x%x]:the first frame time is %d ms\n",hPlayer,u32Time);
    }
    else if (HI_SVR_PLAYER_EVENT_DOWNLOAD_PROGRESS == (HI_SVR_PLAYER_EVENT_E)pstruEvent->eEvent)
    {

        //WV_printf("[0x%x]:HI_SVR_PLAYER_EVENT_DOWNLOAD_PROGRESS \n",hPlayer);
        HI_SVR_PLAYER_PROGRESS_S stDown;
        HI_S64 s64BandWidth = 0;

        HI_SVR_PLAYER_Invoke(hPlayer, HI_FORMAT_INVOKE_GET_BANDWIDTH, &s64BandWidth);

        memcpy(&stDown, pstruEvent->pu8Data, pstruEvent->u32Len);
        // WV_printf("download progress:%d, duration:%lld ms, buffer size:%lld bytes, bandwidth = %lld\n",stDown.u32Progress, stDown.s64Duration, stDown.s64BufferSize, s64BandWidth);
        if (stDown.u32Progress >= 100 && s32PausedCtrl == 2){
            HI_SVR_PLAYER_Resume(hPlayer);
        }
    }
    else if (HI_SVR_PLAYER_EVENT_BUFFER_STATE == (HI_SVR_PLAYER_EVENT_E)pstruEvent->eEvent)
    {
        WV_printf("[0x%x]:HI_SVR_PLAYER_EVENT_BUFFER_STATE \n",hPlayer);
        HI_SVR_PLAYER_BUFFER_S *pstBufStat = (HI_SVR_PLAYER_BUFFER_S*)pstruEvent->pu8Data;
        // WV_printf("HI_SVR_PLAYER_EVENT_BUFFER_STATE type:%d, duration:%lld ms, size:%lld bytes\n",pstBufStat->eType,pstBufStat->stBufStat.s64Duration,pstBufStat->stBufStat.s64BufferSize);

        HI_SVR_PLAYER_GetFileInfo(hPlayer, &pstFileInfo);

        if (pstBufStat->eType == HI_SVR_PLAYER_BUFFER_EMPTY)
        {
            WV_printf("### HI_SVR_PLAYER_EVENT_BUFFER_STATE type:HI_SVR_PLAYER_BUFFER_EMPTY, duration:%lld ms, size:%lld bytes\n",\
                      pstBufStat->stBufStat.s64Duration,\
                      pstBufStat->stBufStat.s64BufferSize);
            if ((HI_FORMAT_SOURCE_NET_VOD == pstFileInfo->eSourceType ||
                 HI_FORMAT_SOURCE_NET_LIVE == pstFileInfo->eSourceType ) &&
                    HI_SVR_PLAYER_STATE_PLAY == ePlayerState)
            {
                s32PausedCtrl = 1;
                HI_SVR_PLAYER_Pause(hPlayer);
            }
        }
        else if (pstBufStat->eType == HI_SVR_PLAYER_BUFFER_START)
        {
            // WV_printf("### HI_SVR_PLAYER_EVENT_BUFFER_STATE type:HI_SVR_PLAYER_BUFFER_START, duration:%lld ms, size:%lld bytes\n",pstBufStat->stBufStat.s64Duration, pstBufStat->stBufStat.s64BufferSize);
            if ((HI_FORMAT_SOURCE_NET_VOD == pstFileInfo->eSourceType ||
                 HI_FORMAT_SOURCE_NET_LIVE == pstFileInfo->eSourceType ) &&
                    HI_SVR_PLAYER_STATE_PLAY == ePlayerState)
            {
                s32PausedCtrl = 1;
                HI_SVR_PLAYER_Pause(hPlayer);
            }
        }
        else if (pstBufStat->eType == HI_SVR_PLAYER_BUFFER_ENOUGH)
        {
            // WV_printf("### HI_SVR_PLAYER_EVENT_BUFFER_STATE type:HI_SVR_PLAYER_BUFFER_ENOUGH, duration:%lld ms, size:%lld bytes\n",pstBufStat->stBufStat.s64Duration,pstBufStat->stBufStat.s64BufferSize);
            if ((HI_FORMAT_SOURCE_NET_VOD == pstFileInfo->eSourceType ||
                 HI_FORMAT_SOURCE_NET_LIVE == pstFileInfo->eSourceType ) &&
                    2 == s32PausedCtrl)
            {
                HI_SVR_PLAYER_Resume(hPlayer);
            }
        }
        else if (pstBufStat->eType == HI_SVR_PLAYER_BUFFER_FULL)
        {
            // WV_printf("### HI_SVR_PLAYER_EVENT_BUFFER_STATE type:HI_SVR_PLAYER_BUFFER_FULL, duration:%lld ms, size:%lld bytes\n",pstBufStat->stBufStat.s64Duration,pstBufStat->stBufStat.s64BufferSize);
            if ((HI_FORMAT_SOURCE_NET_VOD == pstFileInfo->eSourceType ||
                 HI_FORMAT_SOURCE_NET_LIVE == pstFileInfo->eSourceType ) &&
                    2 == s32PausedCtrl)
            {
                HI_SVR_PLAYER_Resume(hPlayer);
            }
        }
    }
    else if (HI_SVR_PLAYER_EVENT_SEEK_FINISHED == (HI_SVR_PLAYER_EVENT_E)pstruEvent->eEvent)
    {

        //WV_printf("[0x%x]:HI_SVR_PLAYER_EVENT_SEEK_FINISHED\n",hPlayer );
        // HIS_PLAYER_Seek(hPlayer,0);

        //  WV_printf("seek finish! \n");
    }
    else
    {
        WV_printf("[0x%x]:unknow event type is %d\n",hPlayer,pstruEvent->eEvent);
    }

    return HI_SUCCESS;
}

/***************************************************************

WV_S32  HIS_PLAYER_Create(HI_HANDLE  * pHndlPlayer ,HI_HANDLE * pHndlAVplay);

***************************************************************/

WV_S32  HIS_PLAYER_Create(HI_HANDLE  * pHndlPlayer ,HI_HANDLE * pHndlAVplay)
{
    HI_SVR_PLAYER_PARAM_S stParam;
    WV_S32 s32Ret;
    memset(&stParam,0,sizeof(stParam));

    stParam.hAVPlayer = *pHndlAVplay;

    s32Ret = HI_SVR_PLAYER_Create(&stParam, pHndlPlayer);

    if (HI_SUCCESS != s32Ret)
    {
        WV_ERROR(" player open fail, ret = 0x%x \r\n", s32Ret);

    }
    s32Ret = HI_SVR_PLAYER_RegCallback(*pHndlPlayer, HIS_PLAYER_CallBack);

    if (HI_SUCCESS != s32Ret)
    {
        WV_ERROR(" register event callback function fail, ret = 0x%x \r\n", s32Ret);
    }

    return  s32Ret;
}


HI_CHAR *getSubFormatStr(HI_U32 u32Format)
{
    switch (u32Format)
    {
    case HI_FORMAT_SUBTITLE_ASS:
        return "ASS";
        break;
    case HI_FORMAT_SUBTITLE_LRC:
        return "LRC";
        break;
    case HI_FORMAT_SUBTITLE_SRT:
        return "SRT";
        break;
    case HI_FORMAT_SUBTITLE_SMI:
        return "SMI";
        break;
    case HI_FORMAT_SUBTITLE_SUB:
        return "SUB";
        break;
    case HI_FORMAT_SUBTITLE_TXT:
        return "TEXT";
        break;
    case HI_FORMAT_SUBTITLE_HDMV_PGS:
        return "HDMV_PGS";
        break;
    case HI_FORMAT_SUBTITLE_DVB_SUB:
        return "DVB_SUB_BMP";
        break;
    case HI_FORMAT_SUBTITLE_DVD_SUB:
        return "DVD_SUB_BMP";
        break;
    default:
        return "UN-KNOWN";
        break;
    }

    return "UN-KNOWN";
}


static HI_CHAR *getAudFormatStr(HI_U32 u32Format)
{
    switch (u32Format)
    {
    case HI_FORMAT_AUDIO_MP2:
        return "MPEG2";
        break;
    case HI_FORMAT_AUDIO_MP3:
        return "MPEG3";
        break;
    case HI_FORMAT_AUDIO_AAC:
        return "AAC";
        break;
    case HI_FORMAT_AUDIO_AC3:
        return "AC3";
        break;
    case HI_FORMAT_AUDIO_DTS:
        return "DTS";
        break;
    case HI_FORMAT_AUDIO_VORBIS:
        return "VORBIS";
        break;
    case HI_FORMAT_AUDIO_DVAUDIO:
        return "DVAUDIO";
        break;
    case HI_FORMAT_AUDIO_WMAV1:
        return "WMAV1";
        break;
    case HI_FORMAT_AUDIO_WMAV2:
        return "WMAV2";
        break;
    case HI_FORMAT_AUDIO_MACE3:
        return "MACE3";
        break;
    case HI_FORMAT_AUDIO_MACE6:
        return "MACE6";
        break;
    case HI_FORMAT_AUDIO_VMDAUDIO:
        return "VMDAUDIO";
        break;
    case HI_FORMAT_AUDIO_SONIC:
        return "SONIC";
        break;
    case HI_FORMAT_AUDIO_SONIC_LS:
        return "SONIC_LS";
        break;
    case HI_FORMAT_AUDIO_FLAC:
        return "FLAC";
        break;
    case HI_FORMAT_AUDIO_MP3ADU:
        return "MP3ADU";
        break;
    case HI_FORMAT_AUDIO_MP3ON4:
        return "MP3ON4";
        break;
    case HI_FORMAT_AUDIO_SHORTEN:
        return "SHORTEN";
        break;
    case HI_FORMAT_AUDIO_ALAC:
        return "ALAC";
        break;
    case HI_FORMAT_AUDIO_WESTWOOD_SND1:
        return "WESTWOOD_SND1";
        break;
    case HI_FORMAT_AUDIO_GSM:
        return "GSM";
        break;
    case HI_FORMAT_AUDIO_QDM2:
        return "QDM2";
        break;
    case HI_FORMAT_AUDIO_COOK:
        return "COOK";
        break;
    case HI_FORMAT_AUDIO_TRUESPEECH:
        return "TRUESPEECH";
        break;
    case HI_FORMAT_AUDIO_TTA:
        return "TTA";
        break;
    case HI_FORMAT_AUDIO_SMACKAUDIO:
        return "SMACKAUDIO";
        break;
    case HI_FORMAT_AUDIO_QCELP:
        return "QCELP";
        break;
    case HI_FORMAT_AUDIO_WAVPACK:
        return "WAVPACK";
        break;
    case HI_FORMAT_AUDIO_DSICINAUDIO:
        return "DSICINAUDIO";
        break;
    case HI_FORMAT_AUDIO_IMC:
        return "IMC";
        break;
    case HI_FORMAT_AUDIO_MUSEPACK7:
        return "MUSEPACK7";
        break;
    case HI_FORMAT_AUDIO_MLP:
        return "MLP";
        break;
    case HI_FORMAT_AUDIO_GSM_MS:
        return "GSM_MS";
        break;
    case HI_FORMAT_AUDIO_ATRAC3:
        return "ATRAC3";
        break;
    case HI_FORMAT_AUDIO_VOXWARE:
        return "VOXWARE";
        break;
    case HI_FORMAT_AUDIO_APE:
        return "APE";
        break;
    case HI_FORMAT_AUDIO_NELLYMOSER:
        return "NELLYMOSER";
        break;
    case HI_FORMAT_AUDIO_MUSEPACK8:
        return "MUSEPACK8";
        break;
    case HI_FORMAT_AUDIO_SPEEX:
        return "SPEEX";
        break;
    case HI_FORMAT_AUDIO_WMAVOICE:
        return "WMAVOICE";
        break;
    case HI_FORMAT_AUDIO_WMAPRO:
        return "WMAPRO";
        break;
    case HI_FORMAT_AUDIO_WMALOSSLESS:
        return "WMALOSSLESS";
        break;
    case HI_FORMAT_AUDIO_ATRAC3P:
        return "ATRAC3P";
        break;
    case HI_FORMAT_AUDIO_EAC3:
        return "EAC3";
        break;
    case HI_FORMAT_AUDIO_SIPR:
        return "SIPR";
        break;
    case HI_FORMAT_AUDIO_MP1:
        return "MP1";
        break;
    case HI_FORMAT_AUDIO_TWINVQ:
        return "TWINVQ";
        break;
    case HI_FORMAT_AUDIO_TRUEHD:
        return "TRUEHD";
        break;
    case HI_FORMAT_AUDIO_MP4ALS:
        return "MP4ALS";
        break;
    case HI_FORMAT_AUDIO_ATRAC1:
        return "ATRAC1";
        break;
    case HI_FORMAT_AUDIO_BINKAUDIO_RDFT:
        return "BINKAUDIO_RDFT";
        break;
    case HI_FORMAT_AUDIO_BINKAUDIO_DCT:
        return "BINKAUDIO_DCT";
        break;
    case HI_FORMAT_AUDIO_DRA:
        return "DRA";
        break;

    case HI_FORMAT_AUDIO_PCM: /* various PCM "codecs" */
        return "PCM";
        break;

    case HI_FORMAT_AUDIO_ADPCM: /* various ADPCM codecs */
        return "ADPCM";
        break;

    case HI_FORMAT_AUDIO_AMR_NB: /* AMR */
        return "AMR_NB";
        break;
    case HI_FORMAT_AUDIO_AMR_WB:
        return "AMR_WB";
        break;
    case HI_FORMAT_AUDIO_AMR_AWB:
        return "AMR_AWB";
        break;

    case HI_FORMAT_AUDIO_RA_144: /* RealAudio codecs*/
        return "RA_144";
        break;
    case HI_FORMAT_AUDIO_RA_288:
        return "RA_288";
        break;

    case HI_FORMAT_AUDIO_DPCM: /* various DPCM codecs */
        return "DPCM";
        break;

    case HI_FORMAT_AUDIO_G711:  /* various G.7xx codecs */
        return "G711";
        break;
    case HI_FORMAT_AUDIO_G722:
        return "G722";
        break;
    case HI_FORMAT_AUDIO_G7231:
        return "G7231";
        break;
    case HI_FORMAT_AUDIO_G726:
        return "G726";
        break;
    case HI_FORMAT_AUDIO_G728:
        return "G728";
        break;
    case HI_FORMAT_AUDIO_G729AB:
        return "G729AB";
        break;
    case HI_FORMAT_AUDIO_PCM_BLURAY:
        return "PCM_BLURAY";
        break;
    default:
        break;
    }

    return "UN-KNOWN";
}

/*************************************************************************************

WV_S32 HIS_PLAYER_FileInfo(HI_FORMAT_FILE_INFO_S *pstFileInfo)

*************************************************************************************/

WV_S32 HIS_PLAYER_FileInfo(HI_FORMAT_FILE_INFO_S *pstFileInfo)

{
    HI_S32 i = 0, j = 0;

    WV_printf("\n\n*************************************************\n");

    if (HI_FORMAT_STREAM_TS == pstFileInfo->eStreamType)
        WV_printf("Stream type: %s \n", "TS");
    else
        WV_printf("Stream type: %s \n", "ES");

    if (HI_FORMAT_SOURCE_NET_VOD == pstFileInfo->eSourceType)
        WV_printf("Source type: %s \n", "NETWORK VOD");
    else if (HI_FORMAT_SOURCE_NET_LIVE == pstFileInfo->eSourceType)
        WV_printf("Source type: %s \n", "NETWORK LIVE");
    else if (HI_FORMAT_SOURCE_LOCAL == pstFileInfo->eSourceType)
        WV_printf("Source type: %s \n", "LOCAL");
    else
        WV_printf("Source type: %s \n", "UNKNOWN");

    WV_printf("File size:   %lld bytes \n", pstFileInfo->s64FileSize);
    WV_printf("Start time:  %lld:%lld:%lld \n",
              pstFileInfo->s64StartTime / (1000 * 3600),
              (pstFileInfo->s64StartTime % (1000 * 3600)) / (1000 * 60),
              ((pstFileInfo->s64StartTime % (1000 * 3600)) % (1000 * 60)) / 1000);
    WV_printf("Duration:    %lld:%lld:%lld \n",
              pstFileInfo->s64Duration / (1000 * 3600),
              (pstFileInfo->s64Duration % (1000 * 3600)) / (1000 * 60),
              ((pstFileInfo->s64Duration % (1000 * 3600)) % (1000 * 60)) / 1000);
    WV_printf("bps:         %d bits/s \n", pstFileInfo->u32Bitrate);

    for (i = 0; i < pstFileInfo->u32ProgramNum; i++)
    {
        WV_printf("\nProgram %d: \n", i);
        WV_printf("   video info: \n");

        for (j = 0; j < pstFileInfo->pastProgramInfo[i].u32VidStreamNum; j++)
        {
            if (HI_FORMAT_INVALID_STREAM_ID != pstFileInfo->pastProgramInfo[i].pastVidStream[j].s32StreamIndex)
            {
                WV_printf("     stream idx:   %d \n", pstFileInfo->pastProgramInfo[i].pastVidStream[j].s32StreamIndex);
                WV_printf("     format:       %s \n", getVidFormatStr(pstFileInfo->pastProgramInfo[i].pastVidStream[j].u32Format));
                WV_printf("     w * h:        %d * %d \n",\
                          pstFileInfo->pastProgramInfo[i].pastVidStream[j].u16Width,\
                          pstFileInfo->pastProgramInfo[i].pastVidStream[j].u16Height);
                WV_printf("     fps:          %d.%d \n",
                          pstFileInfo->pastProgramInfo[i].pastVidStream[j].u16FpsInteger,
                          pstFileInfo->pastProgramInfo[i].pastVidStream[j].u16FpsDecimal);
                WV_printf("     bps:          %d bits/s \n", pstFileInfo->pastProgramInfo[i].pastVidStream[j].u32Bitrate);
            }
        }

        if (pstFileInfo->pastProgramInfo[i].u32VidStreamNum <= 0)
        {
            WV_printf("     video stream is null. \n");
        }

        for (j = 0; j < pstFileInfo->pastProgramInfo[i].u32AudStreamNum; j++)
        {
            WV_printf("   audio %d info: \n", j);
            WV_printf("     stream idx:   %d \n", pstFileInfo->pastProgramInfo[i].pastAudStream[j].s32StreamIndex);
            WV_printf("     format:       %s \n", getAudFormatStr(pstFileInfo->pastProgramInfo[i].pastAudStream[j].u32Format));
            WV_printf("     profile:      0x%x \n", pstFileInfo->pastProgramInfo[i].pastAudStream[j].u32Profile);
            WV_printf("     samplerate:   %d Hz \n", pstFileInfo->pastProgramInfo[i].pastAudStream[j].u32SampleRate);
            WV_printf("     bitpersample: %d \n", pstFileInfo->pastProgramInfo[i].pastAudStream[j].u16BitPerSample);
            WV_printf("     channels:     %d \n", pstFileInfo->pastProgramInfo[i].pastAudStream[j].u16Channels);
            WV_printf("     bps:          %d bits/s \n", pstFileInfo->pastProgramInfo[i].pastAudStream[j].u32Bitrate);
            WV_printf("     lang:         %s \n", pstFileInfo->pastProgramInfo[i].pastAudStream[j].aszLanguage);
        }

        for (j = 0; j < pstFileInfo->pastProgramInfo[i].u32SubStreamNum; j++)
        {
            WV_printf("   subtitle %d info: \n", j);
            WV_printf("     stream idx:     %d \n", pstFileInfo->pastProgramInfo[i].pastSubStream[j].s32StreamIndex);
            WV_printf("     sub type:       %s \n", getSubFormatStr(pstFileInfo->pastProgramInfo[i].pastSubStream[j].u32Format));
            WV_printf("     be ext sub:     %d \n", pstFileInfo->pastProgramInfo[i].pastSubStream[j].bExtSub);
            WV_printf("     original width: %d \n", pstFileInfo->pastProgramInfo[i].pastSubStream[j].u16OriginalFrameWidth);
            WV_printf("     original height:%d \n", pstFileInfo->pastProgramInfo[i].pastSubStream[j].u16OriginalFrameHeight);
        }
    }

    WV_printf("\n*************************************************\n\n");

    return HI_SUCCESS;
}

/***************************************************************

WV_S32  HIS_PLAYER_Destory(HI_HANDLE  * pHndlPlayer);

***************************************************************/

WV_S32  HIS_PLAYER_Destory(HI_HANDLE  * pHndlPlayer )
{
    WV_S32 s32Ret;
    //WV_printf("HI_SVR_PLAYER_Destroy start\n");
//--------------------------------------------------------
    s32Ret = HI_SVR_PLAYER_Destroy (*pHndlPlayer);
    s32Ret = HI_SUCCESS;
//--------------------------------------------------------
    if (HI_SUCCESS != s32Ret)
    {
        WV_ERROR(" player Destroy fail, ret = 0x%x \e[0m \n", s32Ret);
    }
    //WV_printf("HI_SVR_PLAYER_Destroy ok\n");
    return  s32Ret;

}

/***************************************************************

WV_S32  HIS_PLAYER_GetTimePlayed(HI_HANDLE  * pHndlPlayer ,HI_U64  *u64TimePlayed)

***************************************************************/

WV_S32  HIS_PLAYER_GetTimePlayed(HI_HANDLE  * pHndlPlayer ,HI_U64  *u64TimePlayed)
{
    WV_S32  ret;
    HI_SVR_PLAYER_INFO_S  stPlayerInfo;
    ret = HI_SVR_PLAYER_GetPlayerInfo(*pHndlPlayer, &stPlayerInfo);

    if (HI_SUCCESS == ret )
    {
        *u64TimePlayed = stPlayerInfo.u64TimePlayed;

    }
    else
    {

        return WV_EFAIL;
    }
    /*
        if( stPlayerInfo.eStatus !=HIS_PLAYER_STATUS_PLAY )
        {
                return WV_EFAIL;
        }
*/
    return WV_SOK;

}


/***************************************************************

WV_S32  HIS_PLAYER_GetStatus(HI_HANDLE  * pHndlPlayer ,WV_U32 *pStatus);

***************************************************************/

WV_S32  HIS_PLAYER_GetStatus(HI_HANDLE  * pHndlPlayer ,WV_U32 *pStatus)
{
    WV_S32  ret;
    HI_SVR_PLAYER_INFO_S  stPlayerInfo;
    ret = HI_SVR_PLAYER_GetPlayerInfo(*pHndlPlayer, &stPlayerInfo);

    //printf("player status  [%d]\n",*pStatus);
    if (HI_SUCCESS == ret )
    {
        * pStatus  = (HI_U32)stPlayerInfo.eStatus;
        return WV_SOK;
    }
    else
    {
        * pStatus  = HI_SVR_PLAYER_STATE_BUTT;
        return WV_EFAIL;
    }


}

/***************************************************************

WV_S32  HIS_PLAYER_Seek(HI_HANDLE  * pHndlPlayer,HI_U64 u64TimeInMs);

***************************************************************/
WV_S32  HIS_PLAYER_Seek(HI_HANDLE  * pHndlPlayer,HI_U64 u64TimeInMs)
{
    WV_U32  status;
    WV_S32 ret;


    WV_CHECK_RET(HIS_PLAYER_GetStatus(pHndlPlayer,&status));

    if(status == HIS_PLAYER_STATUS_PLAY)
    {
        ret = HI_SVR_PLAYER_Seek(*pHndlPlayer, u64TimeInMs);

    }else{

        ret = WV_SOK;
    }

    return ret;

}

/***************************************************************

WV_S32  HIS_PLAYER_playRoll(HI_HANDLE  * pHndlPlayer);

***************************************************************/
WV_S32  HIS_PLAYER_playRoll(HI_HANDLE  * pHndlPlayer)
{
    return HI_SVR_PLAYER_Play(*pHndlPlayer);
    
}

/***************************************************************

WV_S32  HIS_PLAYER_Replay(HI_HANDLE  * pHndlPlayer);

***************************************************************/
WV_S32  HIS_PLAYER_Replay(HI_HANDLE  * pHndlPlayer)
{
    WV_S32 i;
    WV_U32  status;
    //	for(i=0;i<1000;i++)
    //	{
    WV_CHECK_RET(HIS_PLAYER_GetStatus(pHndlPlayer,&status));

    //printf("play[0] get starus [%d]\n",status);
    if(status == HIS_PLAYER_STATUS_STOP)
    {

        WV_CHECK_RET( HI_SVR_PLAYER_Play(*pHndlPlayer));
        return WV_SOK;

    }else if(status == HIS_PLAYER_STATUS_FORWAR  || \
             status == HIS_PLAYER_STATUS_BACKWARD  ||\
             status == HIS_PLAYER_STATUS_PAUSE  ||\
             status == HIS_PLAYER_STATUS_PLAY )
    {
        HI_SVR_PLAYER_Stop (*pHndlPlayer);

        for(i=0;i<200;i++)
        {
            WV_CHECK_RET(HIS_PLAYER_GetStatus( pHndlPlayer, &status));

            if(status == HIS_PLAYER_STATUS_STOP  )
            {
                HI_SVR_PLAYER_Play(*pHndlPlayer);
                return WV_SOK;
            }

            usleep(33000);
        }

    }
    //	usleep(33000);
    //}
    return WV_SOK;

}

/***************************************************************

WV_S32  HIS_PLAYER_Play(HI_HANDLE  * pHndlPlayer);

***************************************************************/

WV_S32  HIS_PLAYER_Play(HI_HANDLE  * pHndlPlayer )
{

    WV_U32  status;
    WV_S32  i;

    WV_CHECK_RET(HIS_PLAYER_GetStatus(pHndlPlayer,&status));

    if(status == HIS_PLAYER_STATUS_STOP || status == HIS_PLAYER_STATUS_INIT)
    {
        printf("IN HIS_PLAYER_Play +++++++++++++++\n");
        WV_CHECK_RET( HI_SVR_PLAYER_Play(*pHndlPlayer));

    }
    else if(status == HIS_PLAYER_STATUS_PLAY)
    {
        //WV_CHECK_RET( HIS_PLAYER_Seek(*pHndlPlayer,0));
        return WV_SOK;
    }
    else
    {
        return WV_EFAIL;
    }

    for(i=0;i<100;i++)
    {
        WV_CHECK_RET(HIS_PLAYER_GetStatus( pHndlPlayer, &status));

        if(status == HIS_PLAYER_STATUS_PLAY)
        {
            return WV_SOK;
        }

        usleep(33000);
    }
    //printf("player start error \n");
    return WV_EFAIL;
}

/***************************************************************

WV_S32  HIS_PLAYER_Pause(HI_HANDLE  * pHndlPlayer);

***************************************************************/

WV_S32  HIS_PLAYER_Pause(HI_HANDLE  * pHndlPlayer )
{

    WV_U32  status;
    WV_S32   i;

    WV_CHECK_RET(HIS_PLAYER_GetStatus( pHndlPlayer ,&status));

    if(status == HIS_PLAYER_STATUS_PLAY)
    {
        WV_CHECK_RET( HI_SVR_PLAYER_Pause (*pHndlPlayer));

    }
    else if(status == HIS_PLAYER_STATUS_PAUSE)
    {
        return WV_SOK;
    }
    else
    {
        return WV_EFAIL;
    }
    usleep(33000);
    for(i=0;i<100;i++)
    {
        WV_CHECK_RET(HIS_PLAYER_GetStatus( pHndlPlayer,&status));

        if(status == HIS_PLAYER_STATUS_PAUSE)
        {
            return WV_SOK;
        }

        usleep(33000);
    }
    return WV_EFAIL;
}





/***************************************************************

WV_S32  HIS_PLAYER_Resume(HI_HANDLE  * pHndlPlayer);

***************************************************************/

WV_S32  HIS_PLAYER_Resume(HI_HANDLE  * pHndlPlayer )
{

    WV_U32  status;
    WV_S32 i;
    WV_CHECK_RET(HIS_PLAYER_GetStatus( pHndlPlayer ,&status));

    if(status == HIS_PLAYER_STATUS_FORWAR  || \
            status == HIS_PLAYER_STATUS_BACKWARD  ||\
            status == HIS_PLAYER_STATUS_PAUSE   )
    {
        WV_CHECK_RET( HI_SVR_PLAYER_Resume (*pHndlPlayer));
    }
    else if(status == HIS_PLAYER_STATUS_PLAY)
    {
        return WV_SOK;
    }
    else
    {
        return WV_EFAIL;
    }
    /*
    switch  (status)
    {
                case HIS_PLAYER_STATUS_INIT :
                {

                        break;
                }
                case HIS_PLAYER_STATUS_DEINIT :
                {
                        break;
                }
                case HIS_PLAYER_STATUS_PLAY :
                {
                        break;
                }
                case HIS_PLAYER_STATUS_FORWAR :
                {
                        break;
                }
                case HIS_PLAYER_STATUS_BACKWARD :
                {
                        break;
                }
                case HIS_PLAYER_STATUS_PAUSE :
                {
                        break;
                }
                case HIS_PLAYER_STATUS_STOP :
                {
                        break;
                }
                default:
                {

                        break;
                }
    }
      */

    usleep(33000);
    for(i=0;i<100;i++)
    {
        WV_CHECK_RET(HIS_PLAYER_GetStatus( pHndlPlayer, &status));

        if(status == HIS_PLAYER_STATUS_PLAY )
        {
            return WV_SOK;
        }

        usleep(33000);
    }
    return WV_EFAIL;
}





/***************************************************************

WV_S32  HIS_PLAYER_Stop(HI_HANDLE  * pHndlPlayer);

***************************************************************/

WV_S32  HIS_PLAYER_Stop(HI_HANDLE  * pHndlPlayer )
{

    WV_U32  status;
    WV_S32  i;
    WV_CHECK_RET(HIS_PLAYER_GetStatus( pHndlPlayer, &status));

    if(status == HIS_PLAYER_STATUS_FORWAR  || \
            status == HIS_PLAYER_STATUS_BACKWARD  ||\
            status == HIS_PLAYER_STATUS_PAUSE  ||\
            status == HIS_PLAYER_STATUS_PLAY )
    {
        WV_CHECK_RET( HI_SVR_PLAYER_Stop (*pHndlPlayer));
    }
   
    else if(status == HIS_PLAYER_STATUS_STOP)
    {
        return WV_SOK;
    }
    else
    {
        return WV_EFAIL;
    }
    usleep(33000);
    for(i=0;i<200;i++)
    {
        WV_CHECK_RET(HIS_PLAYER_GetStatus( pHndlPlayer, &status));

        if(status == HIS_PLAYER_STATUS_STOP  )
        {
            return WV_SOK;
        }

        usleep(33000);
    }

    return WV_EFAIL;
}


/***************************************************************

WV_S32  HIS_PLAYER_Start(HI_HANDLE  * pHndlPlayer ,WV_S8 * pFileName);

***************************************************************/

WV_S32  HIS_PLAYER_Start(HI_HANDLE  * pHndlPlayer ,WV_S8 * pFileName)
{
    printf("goto HIS_PLAYER_Start\n");
    WV_S32 s32Ret;
    HI_SVR_PLAYER_MEDIA_S stMedia;
    //HI_FORMAT_FILE_INFO_S *pstFileInfo;

    memset(&stMedia, 0, sizeof(stMedia));
    sprintf(stMedia.aszUrl, "%s", pFileName);
    stMedia.u32UserData = 0;
    stMedia.s32PlayMode =0;

    WV_printf("handle = [0x%x]filedName = [%s]\n",*pHndlPlayer,stMedia.aszUrl);

    FILE *fp = NULL;
    fp = fopen(stMedia.aszUrl, "r");
    if ( fp == NULL )
    {
        WV_ERROR("fopen(stMedia.aszUrl) fopen error\n");
    }
    else
    {
        WV_printf("fopen(%s)ok\n",stMedia.aszUrl);
        fclose(fp);
        fp = NULL;
    }
    //printf("In HIS_PLAYER_Start used HI_SVR_PLAYER_Invoke+++++++++\n");
    //zdh//HI_SVR_PLAYER_Invoke(*pHndlPlayer, HI_FORMAT_INVOKE_SET_LOG_LEVEL, (HI_VOID *)HI_FORMAT_LOG_QUITE);
    //printf("In HIS_PLAYER_Start used HI_SVR_PLAYER_EnableDbg+++++++++++++++\n");
    HI_SVR_PLAYER_EnableDbg(HI_FALSE);

    s32Ret = HI_SVR_PLAYER_SetMedia(*pHndlPlayer, HI_SVR_PLAYER_MEDIA_STREAMFILE, &stMedia);
    if (HI_SUCCESS != s32Ret)
    {
        WV_ERROR(" open file fail, ret = %d! \r\n", s32Ret);
    }
    //printf("In HIS_PLAYER_Start used HI_SVR_PLAYER_SetMedia+++++++++++++\n");
    /***************************************************/
    //test set net filebuf size
#if 1
    HI_S64 s64BufMaxSize =(10*1024*1024);

    s32Ret = HI_SVR_PLAYER_Invoke(*pHndlPlayer,HI_FORMAT_INVOKE_SET_BUFFER_MAX_SIZE, &s64BufMaxSize);
    if(HI_SUCCESS != s32Ret){
        WV_ERROR("\n[HI_SVR_PLAYER_Invoke]:set buffer max size error\n");
    }
    //printf("In HIS_PLAYER_Start used HI_SVR_PLAYER_Invoke agen +++++++++\n");

    HI_FORMAT_BUFFER_CONFIG_S stBufConfig;
    memset(&stBufConfig, 0, sizeof(HI_FORMAT_BUFFER_CONFIG_S));
    stBufConfig.eType = HI_FORMAT_BUFFER_CONFIG_SIZE;
    s32Ret = HI_SVR_PLAYER_Invoke(*pHndlPlayer, HI_FORMAT_INVOKE_GET_BUFFER_CONFIG,&stBufConfig);


    if (HI_SUCCESS == s32Ret)
    {

        printf("\n**********************test set net fileBuf size \n");
        stBufConfig.eType = HI_FORMAT_BUFFER_CONFIG_SIZE;
        //stBufConfig.s64Total的值不能超过s64BufMaxSize的值
        stBufConfig.s64Total = s64BufMaxSize;
        stBufConfig.s64EventStart = (256 * 1024);
        stBufConfig.s64EventEnough = (3 * 1024 * 1024);
        stBufConfig.s64TimeOut = 1200;
        s32Ret = HI_SVR_PLAYER_Invoke(*pHndlPlayer,HI_FORMAT_INVOKE_SET_BUFFER_CONFIG, &stBufConfig);
        printf("set buf config ret=%d \n",s32Ret);


    }
    HI_BOOL bUnderRunPause = HI_TRUE;
    s32Ret = HI_SVR_PLAYER_Invoke(*pHndlPlayer,HI_FORMAT_INVOKE_SET_BUFFER_UNDERRUN, &bUnderRunPause);
    if (HI_SUCCESS != s32Ret)
    {
        WV_printf("set buffer underrun pause failed\n");
    }
    //printf("In HIS_PLAYER_Start used HI_SVR_PLAYER_Invoke agen and agen +++++++++\n");

#endif

     s32Ret=HI_SVR_PLAYER_Play(*pHndlPlayer);
    return  s32Ret;
}
