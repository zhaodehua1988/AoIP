#ifndef _FPGA_CHECK_H_H_
#define _FPGA_CHECK_H_H_
#include "fpga_common.h"


/***************************************************
 * WV_S32 FPGA_CHECK_SetCheckTimeValue(WV_U32 time_s)
 * 设置检测时间,单位时秒
 * ************************************************/
void FPGA_CHECK_SetCheckTimeValue(WV_U32 time_s);

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

