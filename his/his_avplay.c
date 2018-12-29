#include "his_avplay.h"
#include "hi_unf_common.h"
#include "hi_unf_avplay.h"
#include "hi_unf_sound.h"
#include "hi_unf_disp.h"
#include "hi_unf_vo.h" 

#define INVALID_TRACK_HDL     (0xffffffff)

static HI_HANDLE s_hAudioTrack[3];// = (HI_HANDLE)INVALID_TRACK_HDL;

#define HI_SVR_PLAYER_INVALID_HDL        (0)


/********************************************************************************
HI_S32 HIADP_AVPlay_RegADecLib();
********************************************************************************/
HI_S32 HIADP_AVPlay_RegADecLib()
{
    HI_S32 Ret = HI_SUCCESS;

    //Ret = HI_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.BLURAYLPCM.decode.so");
    Ret = HI_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.FFMPEG_ADEC.decode.so");
    if (Ret != HI_SUCCESS)
    {
        WV_ERROR("\n\n!!! RegisterAcodecLib libHA.AUDIO.FFMPEG_ADEC.decode.so. error .\n\n");
    }	

    Ret = HI_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AMRWB.codec.so");
    if (Ret != HI_SUCCESS)
    {
         WV_ERROR("\n\n!!! RegisterAcodecLib  libHA.AUDIO.AMRWB.codec.so. error ..\n\n");
    }
    Ret = HI_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.MP3.decode.so");
    if (Ret != HI_SUCCESS)
    {
         WV_ERROR("\n\n!!! RegisterAcodecLib libHA.AUDIO.MP3.decode.so.error . .\n\n");
    }
    Ret = HI_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.MP2.decode.so");
    if (Ret != HI_SUCCESS)
    {
         WV_ERROR("\n\n!!! RegisterAcodecLib libHA.AUDIO.MP2.decode.so. error ..\n\n");
    }
    Ret = HI_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AAC.decode.so");
    if (Ret != HI_SUCCESS)
    {
         WV_ERROR("\n\n!!! RegisterAcodecLib libHA.AUDIO.AAC.decode.so. error ..\n\n");
    }

    Ret = HI_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DRA.decode.so");
    if (Ret != HI_SUCCESS)
    {
         WV_ERROR("\n\n!!!RegisterAcodecLib libHA.AUDIO.DRA.decode.so .error . .\n\n");
    }
	Ret = HI_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.TRUEHDPASSTHROUGH.decode.so");
    if (Ret != HI_SUCCESS)
    {
         WV_ERROR("\n\n!!!RegisterAcodecLib libHA.AUDIO.TRUEHDPASSTHROUGH.decode.so .error . .\n\n");
    }
	Ret = HI_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AMRNB.codec.so");
    if (Ret != HI_SUCCESS)
    {
         WV_ERROR("\n\n!!!RegisterAcodecLib libHA.AUDIO.AMRNB.codec.so .error . .\n\n");
    }
	Ret = HI_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.WMA.decode.so");
    if (Ret != HI_SUCCESS)
    {
         WV_ERROR("\n\n!!!RegisterAcodecLib libHA.AUDIO.WMA.decode.so .error . .\n\n");
    }
	Ret  = HI_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.COOK.decode.so");
    if (Ret != HI_SUCCESS)
    {
         WV_ERROR("\n\n!!!RegisterAcodecLib libHA.AUDIO.COOK.decode.so .error . .\n\n");
    }

	Ret= HI_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DTSPASSTHROUGH.decode.so");
    if (Ret != HI_SUCCESS)
    {
         WV_ERROR("\n\n!!!RegisterAcodecLib libHA.AUDIO.DTSPASSTHROUGH.decode.so .error . .\n\n");
    }
	Ret  = HI_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AC3PASSTHROUGH.decode.so");
    if (Ret != HI_SUCCESS)
    {
         WV_ERROR("\n\n!!!RegisterAcodecLib libHA.AUDIO.AC3PASSTHROUGH.decode.so .error . .\n\n");
    }
	Ret  = HI_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.PCM.decode.so");

    if (Ret != HI_SUCCESS)
    {
        WV_ERROR("\n\n!!!RegisterAcodecLib libHA.AUDIO.PCM.decode.so .error . ..\n\n");
    }

	
    return HI_SUCCESS;
}

/********************************************************************************
WV_S32  HIS_AVP_Init();
********************************************************************************/
WV_S32  HIS_AVP_Init()
{


	WV_S32 s32Ret=0,i;
	for(i=0;i<3;i++)
	{
		s_hAudioTrack[i] = (HI_HANDLE)INVALID_TRACK_HDL;
	}	
	s32Ret = HIADP_AVPlay_RegADecLib();
	if(s32Ret == HI_SUCCESS)
	{
		WV_printf("----------HI_UNF_AVPLAY_RegisterAcodecLib  success \n");
	}

    HI_S32  Ret;
    Ret = HI_UNF_AVPLAY_Init();
    if (Ret != HI_SUCCESS)
    {
        WV_printf("call HI_UNF_AVPLAY_Init failed.\n");
        return WV_EFAIL;
    }
    return  Ret;    
}

/********************************************************************************
WV_S32  HIS_AVP_DeInit();
********************************************************************************/
WV_S32  HIS_AVP_DeInit()
{
    HI_S32  Ret;
    Ret = HI_UNF_AVPLAY_DeInit();;
    if (Ret != HI_SUCCESS)
    {
        WV_printf("call HI_UNF_AVPLAY_DeInit failed.\n");
        return WV_EFAIL;
    }
    return  Ret;    
}
/********************************************************************************
WV_S32 HIS_AVP_openAudChannel(HI_HANDLE hAVPlay,WV_U32 playerID)
********************************************************************************/
WV_S32 HIS_AVP_openAudChannel(HI_HANDLE hAVPlay,WV_U32 playerID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_UNF_AUDIOTRACK_ATTR_S stTrackAttr;
    HI_UNF_SND_GAIN_ATTR_S stGain;
    HI_HANDLE hTrack = (HI_HANDLE)INVALID_TRACK_HDL;

    memset(&stTrackAttr, 0, sizeof(stTrackAttr));
    stTrackAttr.enTrackType = HI_UNF_SND_TRACK_TYPE_MASTER;
    s32Ret = HI_UNF_SND_GetDefaultTrackAttr(stTrackAttr.enTrackType, &stTrackAttr);
    if (HI_SUCCESS != s32Ret)
    {
        WV_printf("get default track attr fail ,return 0x%x\n", s32Ret);
        return s32Ret;
    }

    /*Warning:There must be only one master track on a sound device.*/
    stTrackAttr.enTrackType = HI_UNF_SND_TRACK_TYPE_SLAVE;
    s32Ret = HI_UNF_SND_CreateTrack(HI_UNF_SND_0, &stTrackAttr, &hTrack);
    if (HI_SUCCESS != s32Ret)
    {
        WV_printf("create track fail ,return 0x%x\n", s32Ret);
        return s32Ret;
    }

    stGain.bLinearMode = HI_TRUE;
    stGain.s32Gain = 0;
    s32Ret = HI_UNF_SND_SetTrackWeight(hTrack, &stGain);
    if (HI_SUCCESS != s32Ret)
    {
        WV_printf("set sound track mixer weight failed, return 0x%x\n", s32Ret);
    }

    s32Ret = HI_UNF_AVPLAY_ChnOpen(hAVPlay, HI_UNF_AVPLAY_MEDIA_CHAN_AUD, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        WV_printf("open audio channel fail, return %#x \n", s32Ret);
        HI_UNF_SND_DestroyTrack(hTrack);
        return s32Ret;
    }


    s32Ret = HI_UNF_SND_Attach(hTrack, hAVPlay);
    if (HI_SUCCESS != s32Ret)
    {
        HI_UNF_SND_DestroyTrack(hTrack);
        HI_UNF_AVPLAY_ChnClose(hAVPlay, HI_UNF_AVPLAY_MEDIA_CHAN_AUD);
        WV_printf("attach audio track fail, return %#x \n", s32Ret);
        return s32Ret;
    }
    s_hAudioTrack[playerID] = hTrack;
    WV_printf("open player%d audio channel success!\n",playerID);

    return HI_SUCCESS;
}
/********************************************************************************
HI_S32 HIS_AVP_closeAudChannel(HI_HANDLE hAVPlay,WV_U32 playerID)
********************************************************************************/
HI_S32 HIS_AVP_closeAudChannel(HI_HANDLE hAVPlay,WV_U32 playerID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32RetTmp = HI_SUCCESS;


	if (s_hAudioTrack[playerID] != (HI_HANDLE)INVALID_TRACK_HDL)
	{
	    s32RetTmp = HI_UNF_SND_Detach(s_hAudioTrack[playerID], hAVPlay);
	    if (HI_SUCCESS != s32RetTmp)
	    {
	        s32Ret = s32RetTmp;
	       WV_printf("detach audio track fail, return %#x \n", s32RetTmp);
	    }
	    s32RetTmp = HI_UNF_SND_DestroyTrack(s_hAudioTrack[playerID]);
	    if (HI_SUCCESS != s32RetTmp)
	    {
	        s32Ret = s32RetTmp;
	        WV_printf("destroy audio track fail, return %#x \n", s32RetTmp);
	    }

	    s_hAudioTrack[playerID] = (HI_HANDLE)INVALID_TRACK_HDL;
	}

    s32RetTmp = HI_UNF_AVPLAY_ChnClose(hAVPlay, HI_UNF_AVPLAY_MEDIA_CHAN_AUD);
    if (HI_SUCCESS != s32RetTmp)
    {
        s32Ret = s32RetTmp;
        WV_printf("close audio channel fail, return %#x \n", s32RetTmp);
    }

    return s32Ret;
}

/********************************************************************************

WV_S32  HIS_AVP_SetVolume(V_U32 playerID,WV_S32 volume);

********************************************************************************/
WV_S32  HIS_AVP_SetVolume(WV_U32 playerID,WV_S32 volume)
{
	
	WV_S32 ret;
	HI_UNF_SND_GAIN_ATTR_S stGain;
	
	if(s_hAudioTrack[playerID] == INVALID_TRACK_HDL)
	{
		//printf("HIS_AVP_SetVolume id[%d]=%d \n",playerID,s_hAudioTrack[playerID]);
		return WV_EFAIL;
	}

	stGain.bLinearMode =   HI_TRUE;  //HI_FALSE    = 0,    HI_TRUE     = 1,
								/**<Linear gain(bLinearMode is HI_TRUE) , ranging from 0 to 100*/ /**<CNcomment:��������: 0~100 */
                   				 /**<Decibel gain(bLinearMode is HI_FALSE) , ranging from -70dB to 0dB */ /**<CNcomment: dB����:-70~0*/
	stGain.s32Gain = volume	;
	ret=HI_UNF_SND_SetTrackWeight(s_hAudioTrack[playerID], &stGain);

	return WV_SOK;
}
/********************************************************************************

WV_S32  HIS_AVP_GetVolume(WV_U32 playerID,WV_S32 *pVolume);

********************************************************************************/
WV_S32  HIS_AVP_GetVolume(WV_U32 playerID,WV_S32 *pVolume)
{

	if(s_hAudioTrack[playerID] == INVALID_TRACK_HDL)
	{
		//printf("HIS_AVP_GetVolume id[%d]=%d \n",playerID,s_hAudioTrack[playerID]);
		*pVolume = 0;
		return WV_EFAIL;
	}
	WV_S32 ret;
	HI_UNF_SND_GAIN_ATTR_S stGain;
	stGain.bLinearMode =   HI_TRUE;  //HI_FALSE    = 0,    HI_TRUE     = 1,
								/**<Linear gain(bLinearMode is HI_TRUE) , ranging from 0 to 100*/ /**<CNcomment:��������: 0~100 */
                   				 /**<Decibel gain(bLinearMode is HI_FALSE) , ranging from -70dB to 0dB */ /**<CNcomment: dB����:-70~0*/
	//stGain.s32Gain = volume;
	ret =HI_UNF_SND_GetTrackWeight(s_hAudioTrack[playerID],&stGain);
	*pVolume = stGain.s32Gain;

	return WV_SOK;
}

/********************************************************************************
WV_S32  HIS_AVP_EsOpen(HI_HANDLE  avHandl,HI_UNF_VCODEC_TYPE_E vDecType);
********************************************************************************/
WV_S32  HIS_AVP_EsOpen(HI_HANDLE *avHandl,HI_UNF_VCODEC_TYPE_E vDecType,WV_U32 playerID)
{
    //HI_S32 Ret;
    HI_UNF_AVPLAY_ATTR_S avPlayAttr;
    //HI_UNF_SYNC_ATTR_S avSyncAttr;
    HI_UNF_AVPLAY_OPEN_OPT_S   *pMaxCapbility = HI_NULL; 
    HI_UNF_VCODEC_ATTR_S VcodecAttr;
    
 	//WV_CHECK_RET( HI_UNF_AVPLAY_GetDefaultConfig(&avPlayAttr,HI_UNF_AVPLAY_STREAM_TYPE_TS));
 	WV_CHECK_RET( HI_UNF_AVPLAY_GetDefaultConfig(&avPlayAttr,HI_UNF_AVPLAY_STREAM_TYPE_ES)); 
	//avPlayAttr.stStreamAttr.u32VidBufSize = 1024*1024*10;
    WV_CHECK_RET( HI_UNF_AVPLAY_Create(&avPlayAttr, avHandl));
    //video
    WV_CHECK_RET( HI_UNF_AVPLAY_ChnOpen(*avHandl, HI_UNF_AVPLAY_MEDIA_CHAN_VID, pMaxCapbility)); 
    //WV_CHECK_RET( HI_UNF_AVPLAY_GetAttr(*avHandl, HI_UNF_AVPLAY_ATTR_ID_SYNC, &avSyncAttr));
    //avSyncAttr.enSyncRef = HI_UNF_SYNC_REF_NONE;
    //WV_CHECK_RET(HI_UNF_AVPLAY_SetAttr(*avHandl, HI_UNF_AVPLAY_ATTR_ID_SYNC, &avSyncAttr));  
      
    WV_CHECK_RET( HI_UNF_AVPLAY_GetAttr(*avHandl, HI_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr));
	
    VcodecAttr.enType = vDecType;
    WV_CHECK_RET(HI_UNF_AVPLAY_SetAttr(*avHandl, HI_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr));  
    
    //audio
    
    HIS_AVP_openAudChannel(*avHandl,playerID);   
    //
    return WV_SOK;
    
}

/********************************************************************************
WV_S32  HIS_AVP_Close(HI_HANDLE  avHandl,WV_U32 playerID);
********************************************************************************/
WV_S32  HIS_AVP_Close(HI_HANDLE* avHandl,WV_U32 playerID)
{
	//
	HI_UNF_AVPLAY_STOP_OPT_S Stop;
	Stop.enMode = HI_UNF_AVPLAY_STOP_MODE_BLACK;
	Stop.u32TimeoutMs = 0;
	HI_UNF_AVPLAY_Stop(*avHandl, HI_UNF_AVPLAY_MEDIA_CHAN_VID, &Stop);
	HI_UNF_AVPLAY_Stop(*avHandl, HI_UNF_AVPLAY_MEDIA_CHAN_AUD, &Stop);
	//
	HIS_AVP_closeAudChannel(*avHandl,playerID);
	HI_UNF_AVPLAY_ChnClose(*avHandl, HI_UNF_AVPLAY_MEDIA_CHAN_VID);

	WV_CHECK_RET( HI_UNF_AVPLAY_Destroy(*avHandl));
  
  return   WV_SOK;
 
}

