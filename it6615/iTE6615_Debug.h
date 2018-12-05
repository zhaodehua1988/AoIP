///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_Debug.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/

#include <stdio.h>
#include <stdarg.h>

#ifndef _DEBUG_H_
#define _DEBUG_H_


#ifndef Debug_message
#define Debug_message 1
#endif

#if Debug_message
    #define HDMITX_INT_PRINTF(x)  printf x
    #define HDMITX_AUDIO_PRINTF(x)  printf x //kuro
    #define HDMITX_DEBUG_PRINTF(x) printf x //kuro
    #define HDMITX_SCDC_PRINTF(x) printf x //kuro
    #define CEC_DEBUG_PRINTF(x) //printf x
    #define HDCP_DEBUG_PRINTF(x) printf x
    #define REG_PRINTF(x) //printf x
    #define PRINTF_EDID(x) //printf x
    #define PRINTF_BUFFER(x) //printf x
    #define HWRD_MSG(x)

    //#define HDMITX_INT_PRINTF(x)  //printf x
    //#define HDMITX_AUDIO_PRINTF(x)  //printf x //kuro
    //#define HDMITX_DEBUG_PRINTF(x) //printf x //kuro
    //#define HDMITX_SCDC_PRINTF(x) //printf x //kuro
    //#define CEC_DEBUG_PRINTF(x) //printf x
    //#define HDCP_DEBUG_PRINTF(x) //printf x
    //#define REG_PRINTF(x) printf x
    //#define PRINTF_EDID(x) //printf x
    //#define PRINTF_BUFFER(x) //printf x
#else
    #define HDMITX_INT_PRINTF(x)
    #define HDMITX_AUDIO_PRINTF(x)
    #define HDMITX_DEBUG_PRINTF(x)
    #define HDMITX_SCDC_PRINTF(x)
    #define CEC_DEBUG_PRINTF(x)
    #define HDCP_DEBUG_PRINTF(x)
    #define REG_PRINTF(x)
    #define PRINTF_EDID(x)
    #define PRINTF_BUFFER(x)
    #define HWRD_MSG(x)
#endif




#endif
