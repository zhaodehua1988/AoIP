/******************************************************************************

  Copyright (C), 2001-2014, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hi_unf_pes.h
  Version       : Initial Draft
  Author        : l00168554
  Created       : 2011/07/05
  Description   :

*******************************************************************************/
/**
 * \file
 * \brief  Describes the information about the Packetized elementary stream parse information. CNcomment: 提供PES stream 解码的相关信息 CNend
 */
#ifndef __HI_UNF_PES_H__
#define __HI_UNF_PES_H__

#include "hi_unf_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*************************** Structure Definition ****************************/
/** \addtogroup      PES */
/** @{ */  /** <!-- 【PES】 */

#ifdef HI_ADVCA_FUNCTION_RELEASE
#define HI_FATAL_PES
#define HI_ERR_PES
#define HI_WARN_PES
#define HI_INFO_PES
#else
/**control print, if undefined HI_ADVCA_FUNCTION_RELEASE, pes component  will print some information*/
/**CNcomment:打印控制开关，如果没定义HI_ADVCA_FUNCTION_RELEASE，开启打印*/
#define HI_FATAL_PES printf
/**control print, if undefined HI_ADVCA_FUNCTION_RELEASE, pes component  will print some information*/
/**CNcomment:打印控制开关，如果没定义HI_ADVCA_FUNCTION_RELEASE，开启打印*/
#define HI_ERR_PES   printf
/**control print, if undefined HI_ADVCA_FUNCTION_RELEASE, pes component  will print some information*/
/**CNcomment:打印控制开关，如果没定义HI_ADVCA_FUNCTION_RELEASE，开启打印*/
#define HI_WARN_PES  printf
/**control print, if undefined HI_ADVCA_FUNCTION_RELEASE, pes component  will print some information*/
/**CNcomment:打印控制开关，如果没定义HI_ADVCA_FUNCTION_RELEASE，开启打印*/
#define HI_INFO_PES  printf
#endif

#if 0
#define HI_FATAL_PES(fmt...) \
            HI_TRACE(HI_LOG_LEVEL_FATAL, HI_DEBUG_ID_AVPLAY, fmt)

#define HI_ERR_PES(fmt...) \
            HI_TRACE(HI_LOG_LEVEL_ERROR, HI_DEBUG_ID_AVPLAY, fmt)

#define HI_WARN_PES(fmt...) \
            HI_TRACE(HI_LOG_LEVEL_WARNING, HI_DEBUG_ID_AVPLAY, fmt)

#define HI_INFO_PES(fmt...) \
            HI_TRACE(HI_LOG_LEVEL_INFO, HI_DEBUG_ID_AVPLAY, fmt)
#endif

/**the largest node number for dvb format to store information*/
/**CNcomment:DVD格式存储信息的最大节点数目*/
#define PES_MAX_NODE 10

/**Defines the PES stream information attributes.*/
/**CNcomment:定义PES码流信息的结构体 */
typedef struct hiUNF_PES_INFO_S
{
    HI_U32 streamNum;      /**<the index of pes stream information*//**<CNcomment: PES 码流信息索引 */
    struct
    {
        HI_U32 StreamType;      /**<specifying the type of the elementary stream See 13818-1*//**<CNcomment: PES 码流元素类型，具体参考13818-1*/
        HI_U32 StrmID;          /**<indicating the value of the stream_id field in the PES packet headers of PES packets in which this elementary stream is stored.*//**<CNcomment: PES 包头域中的stream_id*/
    }node[PES_MAX_NODE];

}HI_UNF_PES_INFO_S;

/**Defines PES Stream Type*/
/**CNcomment:定义PES码流类型枚举类型 */
enum
{
    HI_UNF_PES_STREAM_TYPE_NULL,                /**<NULL*/
    HI_UNF_PES_STREAM_TYPE_VIDEO_MPEG2,         /**<MPEG2*/
    HI_UNF_PES_STREAM_TYPE_VIDEO_MPEG4,         /**<MPEG4 DIVX4 DIVX5*/
    HI_UNF_PES_STREAM_TYPE_VIDEO_H264,          /**<H264*/
    HI_UNF_PES_STREAM_TYPE_VIDEO_H263,          /**<H263*/
    HI_UNF_PES_STREAM_TYPE_VIDEO_UNKNOW,        /**<VIDEO UNKNOW*/

    HI_UNF_PES_STREAM_TYPE_AUDIO_AAC,           /**<AAC*/
    HI_UNF_PES_STREAM_TYPE_AUDIO_MP3,           /**<MP2 MP3*/
    HI_UNF_PES_STREAM_TYPE_AUDIO_AC3,           /**<AC3*/
    HI_UNF_PES_STREAM_TYPE_AUDIO_EAC3,          /**<EAC3*/
    HI_UNF_PES_STREAM_TYPE_AUDIO_DTS,           /**<DTS*/
    HI_UNF_PES_STREAM_TYPE_AUDIO_PCM,           /**<PCM*/
    HI_UNF_PES_STREAM_TYPE_AUDIO_DDPLUS,        /**<DDPLUS*/
    HI_UNF_PES_STREAM_TYPE_AUDIO_UNKNOW,        /**<AUDIO UNKNOW*/

    HI_UNF_PES_STREAM_TYPE_BUTT
};

/**Defines PES Stream Parse Method */
/**CNcomment:定义PES码流解析方法枚举类型 */
typedef enum
{
    HI_UNF_PES_PARSE_TYPE_REGULAR = 0,          /**<believe peslen, protocol parse *//**<CNcomment: 认为pes长度域准确，按照协议流程解析 */
    HI_UNF_PES_PARSE_TYPE_ADJUST,               /**<when peslen is not correct, search pes header to parse , only for video element*//**<CNcomment: 不相信pes长度域，采用搜索pes头去解析，仅适用于视频流*/

    HI_UNF_PES_PARSE_TYPE_BUTT
}HI_UNF_PES_PARSE_TYPE_E;

/**Defines PES  Open parameter, this is used as parma[in] for api HI_UNF_PES_Open.*/
/**CNcomment: 定义PES打开参数，作为HI_UNF_PES_Open接口的输入参数 */
typedef struct hiUNF_PES_OPEN_S
{
    HI_HANDLE                      hAVPlay;             /**<AVPlay Handle *//**<CNcomment: AVPlay句柄 */
    HI_U32                         u32VdecType;         /**<PES Video type, if do not open video decoder, please select HI_UNF_PES_STREAM_TYPE_VIDEO_UNKNOW */
                                                        /**<CNcomment: PES视频类型，如果视频解码器没有打开，请选择HI_UNF_PES_STREAM_TYPE_VIDEO_UNKNOW */
    HI_U32                         u32AudioType;        /**<PES Audio type, if do not open video decoder, please select HI_UNF_PES_STREAM_TYPE_AUDIO_UNKNOW */
                                                        /**<CNcomment: PES音频类型，如果音频解码器没有打开，请选择HI_UNF_PES_STREAM_TYPE_AUDIO_UNKNOW */
    HI_U32                         u32PESBufferSize;    /**<PES packet Max size is 64KB, so it should be >= 100KB.IF PES length may equals 0,you need buffer bigger than 1MB.*/
                                                        /**<CNcomment: PES包最大为64KB，所以该值至少为100KB 。如果pes长度域为0，该值至少为1MB*/
    HI_UNF_PES_PARSE_TYPE_E        enPESParseType;      /**<*PES STREAM Parse Type *//**<CNcomment:PES码流解析方法*/
    HI_BOOL                        bIsDVD;              /**<*whether PES STREAM is DVD format, HI_TRUE means DVD format*//**<CNcomment:PES码流是否为DVD格式，HI_TRUE是DVD格式*/
}HI_UNF_PES_OPEN_S;

/** @} */  /** <!-- ==== Structure Definition End ==== */

/******************************* API Declaration *****************************/
/** \addtogroup      PES */
/** @{ */  /** <!-- 【PES】 */



/**
\brief Open  PES module  CNcomment:打开PES模块 CNend
\attention \n
N/A
\param[in] stOpenParam Open parameter.CNcomment:PES  模块打开输入参数 CNend
\param[out] handle    Pointer to the handle of a pes instance .CNcomment:指针类型，打开PES模块所生成的PES实例句柄 CNend
\retval ::HI_SUCCESS Success CNcomment: 成功 CNend
\retval ::HI_FAILURE  This API fails to be called due to system errors. CNcomment:出现系统错误，API调用失败 CNend
\see \n
N/A
*/
HI_S32 HI_UNF_PES_Open(HI_UNF_PES_OPEN_S *pstOpenParam, HI_HANDLE *handle);

/**
\brief  Close  PES module  CNcomment:关闭PES模块 CNend
\attention \n
N/A
\param[in] handle  the handle of a pes instance CNcomment:PES实例句柄 CNend
\retval ::HI_SUCCESS Success  CNcomment:成功 CNend
\retval ::HI_FAILURE  This API fails to be called due to system errors. CNcomment:出现系统错误，API调用失败 CNend
\see \n
N/A
*/
HI_S32 HI_UNF_PES_Close(HI_HANDLE handle);

/**
\brief PES  Obtain PES buffer.CNcomment:获取PES buffer CNend
\attention \n
N/A
\param[in] handle  the handle of a pes instance CNcomment:PES实例句柄 CNend
\param[in] u32ReqSize: Required buffer size,it must smaller than whole PS Buffer.Recommend it's a multiple of  whole PS Buffer's 1/N CNcomment:需要的缓存大小，必须小于整个PS Buffer的大小。建议是整个大小的1/N CNend
\param[out] pstData    Pointer to the returned buffer CNcomment:返回缓存指针 CNend
\retval ::HI_SUCCESS Success  CNcomment:成功 CNend
\retval ::HI_FAILURE  This API fails to be called due to system errors. CNcomment:出现系统错误，API调用失败 CNend
\see \n
N/A
*/
HI_S32 HI_UNF_PES_GetBuffer(HI_HANDLE handle, HI_U32 u32ReqLen, HI_UNF_STREAM_BUF_S *pstData);

/**
\brief  After filling data,send it to PES process module.CNcomment: PES数据填完后，推送数据给PES模块处理 CNend
\attention \n
It must be called after HI_UNF_PES_GetBuffer  CNcomment:  必须与HI_UNF_PES_GetPSBuffer配对使用。 CNend
\param[in] handle  the handle of a pes instance CNcomment:PES实例句柄 CNend
\param[in] u32ValidDatalen : Valid data length  CNcomment:送入的数据长度 CNend
\retval ::HI_SUCCESS Success  CNcomment:成功 CNend
\retval ::HI_FAILURE  This API fails to be called due to system errors. CNcomment:出现系统错误，API调用失败 CNend
\see \n
N/A
*/
HI_S32 HI_UNF_PES_PutBuffer(HI_HANDLE handle, HI_U32 u32ValidDatalen);

/**
\brief Obtain PES stream information CNcomment: 获取PES流信息. CNend
\attention \n
N/A
\param[in] handle  the handle of a pes instance CNcomment:PES实例句柄 CNend
\param[out] pstruPSInfo  Pointer to the returned PES stream information CNcomment: 返回PES码流信息指针 CNend
\retval ::HI_SUCCESS Success  CNcomment:成功 CNend
\retval ::HI_FAILURE  This API fails to be called due to system errors. CNcomment:出现系统错误，API调用失败 CNend
\see \n
N/A
*/
HI_S32 HI_UNF_PES_GetInfo(HI_HANDLE handle, HI_UNF_PES_INFO_S *pstruPSInfo);

/**
\brief Set Video PES stream id CNcomment: 设置视频PES流ID. CNend
\attention \n
N/A
\param[in] handle  the handle of a pes instance CNcomment:PES实例句柄 CNend
\param[in] videostreamid  Video PES stream id CNcomment: 视频PES流ID CNend
\retval ::HI_SUCCESS Success  CNcomment:成功 CNend
\see \n
N/A
*/
HI_S32 HI_UNF_PES_SetVideoStreamID(HI_HANDLE handle, HI_U32 videostreamid);

/**
\brief Set Audio PES stream id CNcomment: 设置音频PES流ID. CNend
\attention \n
N/A
\param[in] handle  the handle of a pes instance CNcomment:PES实例句柄 CNend
\param[in] audiostreamid  Audio PES stream id CNcomment: 音频PES流ID CNend
\retval ::HI_SUCCESS Success  CNcomment:成功 CNend
\see \n
N/A
*/
HI_S32 HI_UNF_PES_SetAudioStreamID(HI_HANDLE handle, HI_U32 audiostreamid);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

/** @} */  /** <!-- ==== API declaration end ==== */

#endif /*__HI_UNF_PES_H__*/
