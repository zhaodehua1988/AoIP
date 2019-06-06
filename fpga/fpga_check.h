#ifndef _FPGA_CHECK_H_H_
#define _FPGA_CHECK_H_H_
#include "fpga_common.h"


enum _E_VOLUME_STATUS
{
    VOLUME_OK = 0, //音量正常
    VOLUME_MUTE,   //静音
    VOLUME_LOW,    //音量过低
    VOLUME_HIGHT   //音量过高
};

////////////////////////////////////////////////////////////////////////////////
//以下为设置
///////////////////////////////////////////////////////////////////////////////
/***************************************************
 * void FPGA_CHECK_SetAudioMuteCheck(WV_U16 ethID, WV_U16 ipID,float threshold,WV_U16 time_s)
 * 函数说明：设置静音的门限值和检测音频丢失的时间
 * 参数说明：threshold 为音量的分贝值 （0dB代表最大，-80dB代表静音）
 * ************************************************/
void FPGA_CHECK_SetAudioMuteCheck(WV_U16 ethID, WV_U16 ipID,float threshold,WV_U16 time_s);

/***************************************************
 * void FPGA_CHECK_SetAudioMuteheck(WV_U16 ethID, WV_U16 ipID,float threshold,WV_U16 time_s)
 * 设置静音的门限值和检测音频过高的时间
 * ************************************************/
void FPGA_CHECK_SetAudioHightCheck(WV_U16 ethID, WV_U16 ipID,float threshold,WV_U16 time_s);

/***************************************************
 * void FPGA_CHECK_SetAudioLowCheck(WV_U16 ethID, WV_U16 ipID,float threshold,WV_U16 time_s)
 * 设置静音的门限值和检测音频过低的时间
 *
 * ************************************************/
void FPGA_CHECK_SetAudioLowCheck(WV_U16 ethID, WV_U16 ipID,float threshold,WV_U16 time_s);

/***************************************************
 * void FPGA_CHECK_SetCheckFreezeTimeValue(WV_U16 ethID, WV_U16 ipID, WV_U16 thousandth,WV_U32 time_s,WV_U16 mutltiple)
 * 静帧检测时间和结合音频的倍数
 * ************************************************/
void FPGA_CHECK_SetCheckFreezeTimeValue(WV_U16 ethID, WV_U16 ipID, WV_U16 thousandth,WV_U32 time_s,WV_U16 mutltiple);

///////////////////////////////////////////////////////////////////////////////////
//以下为查询
//////////////////////////////////////////////////////////////////////////////////
/****************************************************
 * WV_U16 FPGA_CHECK_AudioStatus(WV_U16 ethID, WV_U16 ipID)
 * 返回值为当前音频的状态
 * *************************************************/
WV_U16 FPGA_CHECK_AudioStatus(WV_U16 ethID, WV_U16 ipID);

/****************************************************
 * WV_S32 FPGA_CHECK_GetWinFreeze(WV_U16 ethID,WV_U16 ipID) 
 * 函数说明：检查是否静帧
 * 参数说明：
 *    ethID:第几路网卡
 *    ipID :第几个ip地址
 * 返回值说明：
 *    返回值为1：说明窗口静帧
 *           0：则窗口没有静帧
 *          -1：查询错误
 * *************************************************/
WV_S32 FPGA_CHECK_GetWinFreeze(WV_U16 ethID, WV_U16 ipID);

/****************************************************
 * WV_S32 FPGA_Check_NoSignal(WV_U16 ethID, WV_U16 ipID)
 * 函数说明：查询窗口是否没有信号
 * 返回值 0:有信号
 *       1:没有信号
 *      -1:查询错误
 * *************************************************/
WV_S32 FPGA_Check_NoSignal(WV_U16 ethID, WV_U16 ipID);

/******************************************************
 * void FGPA_CHECK_Open()
 * ****************************************************/
void FGPA_CHECK_Open();

/******************************************************
 * void FPGA_CHECK_Close()
 * ****************************************************/
void FPGA_CHECK_Close();

#endif

