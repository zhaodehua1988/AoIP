/******************************************************************************
 Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.
******************************************************************************
 File Name     : HA.AUDIO.DTSHD.decode.h
 Version       : Initial Draft
 Author        : Hisilicon multimedia software group
 Created       :
 Last Modified :
 Description   :
 Function List :
 History       :
******************************************************************************/
#ifndef __HISI_AUDIO_DECODER_DTSM6_H__
#define __HISI_AUDIO_DECODER_DTSM6_H__

#include "hi_type.h"
#include "hi_audio_codec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


typedef  struct
{
    HI_U32  spkrOut;              /* set as a result of spkrout ,default is 2(Lo/Ro) */
    HI_BOOL coreOnly;             /**< true: core decode only, false: dts hd decode,default value is false */
    HI_U32  outputBitWidth;       /**< 16: 16bit, 24: 24bit, 0,:native, defalut is 16 */
    HI_U32  DRCPercent;           /* 0~100 - default is 0 ,no DRC */
    HI_BOOL enableHDPassThrough;  /* true: enable , default value is true */
    HI_BOOL enableSPDIFOutput;    /* true: enable , default value is true */
    HI_BOOL enableTransEncode;    /* true: enable , default value is false */
    HI_U32  transEncodeMode;      /* 0,DTSTRANSCODEMODE_5_1_MIXOUT;1,DTSTRANSCODEMODE_FULL_MIXOUT;default 1 */
    HI_BOOL allowUalignWord;      /* true: allow , default value is true */
} DTSM6_DECODE_OPENCONFIG_S;

#define HA_DTSM6_DecGetDefalutOpenConfig(pConfigParam) \
    do {  \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->spkrOut = 2; \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->coreOnly = HI_FALSE; \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->outputBitWidth  = 24; \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->DRCPercent = 0; \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->enableHDPassThrough = HI_TRUE; \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->enableSPDIFOutput = HI_TRUE; \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->enableTransEncode = HI_FALSE; \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->transEncodeMode = 1; \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->allowUalignWord = HI_TRUE; \
    } while (0)

#define HA_DTSM6_DecGetDefalutOpenParam(pOpenParam, pstPrvateConfig) \
    do { HI_S32 i; \
        ((HI_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = HD_DEC_MODE_SIMUL; \
        ((HI_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredOutChannels = 2; \
        ((HI_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.bInterleaved = HI_TRUE; \
        ((HI_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32BitPerSample = 16; \
        ((HI_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredSampleRate = 48000; \
        for (i = 0; i < HA_AUDIO_MAXCHANNELS; i++) \
        { \
            ((HI_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.enChannelMapping[i] = HA_AUDIO_ChannelNone; \
        } \
        ((HI_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = (HI_VOID*)pstPrvateConfig; \
        ((HI_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(DTSM6_DECODE_OPENCONFIG_S); \
    } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HISI_AUDIO_DECODER_DTSM6_H__ */
