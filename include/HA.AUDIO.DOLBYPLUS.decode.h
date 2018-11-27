/******************************************************************************
 Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.
******************************************************************************
 File Name     : HA.AUDIO.DOLBYPLUS.decode.h
 Version       : Initial Draft
 Author        : Hisilicon multimedia software group
 Created       :
 Last Modified :
 Description   :
 Function List :
 History       :
******************************************************************************/
#ifndef __HISI_AUDIO_DECODER_DOLBYPLUS_H__
#define __HISI_AUDIO_DECODER_DOLBYPLUS_H__

#include "hi_type.h"
#include "hi_audio_codec.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */
   

#define ENA_DUMP_FILE_TEST

/*!< Decoder dynamic range compression mode
- 0 = Custom mode (no digital dialogue normalization)
- 1 = 2/0 Dolby Surround mode
- 2 = Line out mode
- 3 = RF mode */
typedef enum
{
    DOLBYPLUS_DRC_CUSTOM_0 = 0,
    DOLBYPLUS_DRC_CUSTOM_1,
    DOLBYPLUS_DRC_LINE,            /* default */
    DOLBYPLUS_DRC_RF,
}  DOLBYPLUS_DRC_MODE_E;

/* define LFE on/off */
typedef enum
{
    DOLBYPLUS_LFEOUTOFF = 0,
    DOLBYPLUS_LFEOUTON           /* default */
} DOLBYPLUS_LFEOUT_MODE;

/*!< Decoder output mode. If the output mode does not equal
                                         the bitstream audio coding mode (acmod), then the decoder
                                         will perform downmixing or fill channels with zero values
                                         to meet the desired output configuration.

                                         Note: In Dolby SIP, zero equals 2/0 Dolby Surround
                                               compatible (Lt/Rt))
                                         - 1 = 1/0 (C)
                                         - 2 = 2/0 (L, R)
                                         - 3 = 3/0 (L, C, R)
                                         - 4 = 2/1 (L, R, S)
                                         - 5 = 3/1 (L, C, R, S)
                                         - 6 = 2/2 (L, R, Ls, Rs)
                                         - 7 = 3/2 (L, C, R, Ls, Rs) */
typedef enum
{
    DOLBYPLUS_OUT_MODE11 = 0,
    DOLBYPLUS_OUT_MODE10,
    DOLBYPLUS_OUT_MODE20,      /* default */
    DOLBYPLUS_OUT_MODE30,
    DOLBYPLUS_OUT_MODE21,
    DOLBYPLUS_OUT_MODE31,
    DOLBYPLUS_OUT_MODE22,
    DOLBYPLUS_OUT_MODE32,
} DOLBYPLUS_OUTPUT_MODE_E;

/*!< Decoder dual mono mode (Only when <i>acmod</i> is 1/1)
                                     - 0 = Stereo
                                     - 1 = Left mono
                                     - 2 = Right mono
                                     - 3 = Mixed mono */
typedef enum
{
    DOLBYPLUS_DUAL_STEREO = 0,   /* default */
    DOLBYPLUS_DUAL_LEFTMONO,
    DOLBYPLUS_DUAL_RGHTMONO,
    DOLBYPLUS_DUAL_MIXMONO,
} DOLBYPLUS_DUAL_MODE_E;

/*!< Decoder stereo output mode.  This parameter specifies the
                                     reproduction mode associated with stereo output
                                     (output mode = 2/0).
                                     - 0 = Automatically detects stereo mode based on setting in the bit stream.
                                     - 1 = 2/0 Dolby Surround compatible (Lt/Rt)
                                     - 2 = 2/0 Stereo (Lo/Ro)
                                     - 3 = Reserved */
typedef enum
{
    DOLBYPLUS_DMX_AUTO = 0,   /* default */
    DOLBYPLUS_DMX_SRND,
    DOLBYPLUS_DMX_STEREO,
} DOLBYPLUS_STEREODMX_MODE_E;

/* define DDP convert  on/off */
typedef enum
{
    DOLBYPLUS_CONVERTER_OFF = 0,
    DOLBYPLUS_CONVERTER_ON    /* default */
} DOLBYPLUS_CONVERTER_MODE;

/** DDP  HA_DOLBYPLUS_EVENT_SOURCE_CHANGE event, stream information user data defination */
typedef struct  hiDOLBYPLUS_STREAM_INFO_S
{
    HI_S16 s16StreamType;   /* 0: dolby digital, 1: dolby digital plus */
    HI_S16 s16Acmod;
    HI_S32 s32BitRate;
    HI_S32 s32SampleRateRate;
} DOLBYPLUS_STREAM_INFO_S;

/** DDP callback event defination */
typedef enum
{
    HA_DOLBYPLUS_EVENT_SOURCE_CHANGE = 0,                   /**< stream information change event, DOLBYPLUS_STREAM_INFO_S     */
    HA_DOLBYPLUS_EVENT_BUTT
} DOLBYPLUS_EVENT_E;

/** DDP callback method defination */
typedef HI_VOID (*DOLBYPLUS_EVENT_CB_FN)(DOLBYPLUS_EVENT_E enEvent, HI_VOID* pAppData);

typedef struct  hiDOLBYPLUS_DECODE_OPENCONFIG_S
{
    DOLBYPLUS_DRC_MODE_E       enDrcMode;      /* dynamic range compression mode */
    DOLBYPLUS_LFEOUT_MODE      enOutLfe;       /* output LFE channel present     */
    DOLBYPLUS_OUTPUT_MODE_E    enOutputMode;   /* output channel configuration   */
    DOLBYPLUS_STEREODMX_MODE_E enDmxMode;      /* stereo output mode    */
    DOLBYPLUS_DUAL_MODE_E      enDualMode;     /* dual mono reproduction mode   */
    HI_S16                     s16PcmScale;    /* PCM scale factor, 0~100,default:100,  100 means 1.0, 33 means 0.33,etc */
    HI_S16                     s16DynScaleHigh;/* dynamic range compression cut scale factor, 0~100,default:100,  100 means 1.0, 33 means 0.33,etc*/
    HI_S16                     s16DynScaleLow; /* dynamic range compression boost scale factor, 0~100,default:100, 100 means 1.0, 33 means 0.33,etc   */
    DOLBYPLUS_CONVERTER_MODE   enConvertMode;  /* ddp decoder-converter on/off     */
    HI_S16                     PirvateControl; /* private test control(0~255), default:0. */
    HI_CHAR                    szInputFileName[256]; /*  private test params */
    DOLBYPLUS_EVENT_CB_FN      pfnEvtCbFunc[HA_DOLBYPLUS_EVENT_BUTT];      /* ddp call back method */
    HI_VOID*                   pAppData[HA_DOLBYPLUS_EVENT_BUTT];       /* application defined value for ddp call back method */
} DOLBYPLUS_DECODE_OPENCONFIG_S;

#define HA_DOLBYPLUS_DecGetDefalutOpenConfig(pConfigParam) \
    do { HI_S32 n; \
         ((DOLBYPLUS_DECODE_OPENCONFIG_S *)(pConfigParam))->enDrcMode = DOLBYPLUS_DRC_RF; \
         ((DOLBYPLUS_DECODE_OPENCONFIG_S *)(pConfigParam))->enOutLfe = DOLBYPLUS_LFEOUTON; \
         ((DOLBYPLUS_DECODE_OPENCONFIG_S *)(pConfigParam))->enOutputMode = DOLBYPLUS_OUT_MODE20; \
         ((DOLBYPLUS_DECODE_OPENCONFIG_S *)(pConfigParam))->enDmxMode   = DOLBYPLUS_DMX_SRND; \
         ((DOLBYPLUS_DECODE_OPENCONFIG_S *)(pConfigParam))->enDualMode  = DOLBYPLUS_DUAL_STEREO; \
         ((DOLBYPLUS_DECODE_OPENCONFIG_S *)(pConfigParam))->s16PcmScale = 100; \
         ((DOLBYPLUS_DECODE_OPENCONFIG_S *)(pConfigParam))->s16DynScaleHigh = 100; \
         ((DOLBYPLUS_DECODE_OPENCONFIG_S *)(pConfigParam))->s16DynScaleLow = 100; \
         ((DOLBYPLUS_DECODE_OPENCONFIG_S *)(pConfigParam))->enConvertMode  = DOLBYPLUS_CONVERTER_OFF; \
         ((DOLBYPLUS_DECODE_OPENCONFIG_S *)(pConfigParam))->PirvateControl = 0; \
         for (n = 0; n < HA_DOLBYPLUS_EVENT_BUTT; n++) \
         { \
             ((DOLBYPLUS_DECODE_OPENCONFIG_S *)(pConfigParam))->pfnEvtCbFunc[n] = HI_NULL; \
             ((DOLBYPLUS_DECODE_OPENCONFIG_S *)(pConfigParam))->pAppData[n] = HI_NULL; \
         } \
    } while (0)

#define HA_DOLBYPLUS_DecGetDefalutOpenParam(pOpenParam, pstPrivateParams) \
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
         ((HI_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = (HI_VOID*)pstPrivateParams; \
         ((HI_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(DOLBYPLUS_DECODE_OPENCONFIG_S); \
    } while (0)

#define HA_DOLBYPLUS_DecSetMode(pOpenParam, eDecMode) \
    do { ((HI_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = eDecMode; } while (0)

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __HISI_AUDIO_DECODER_DOLBYPLUS_H__ */
