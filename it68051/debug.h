///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <debug.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/

#include <stdio.h>

#ifndef _DEBUG_H_
#define _DEBUG_H_


#define Debug_message 1

#ifndef Debug_message
#define Debug_message 1
#endif

#if Debug_message
    #define MHLRX_DEBUG_PRINTF(x)  //printf  x //kuro
    #define EQ_DEBUG_PRINTF(x) printf  x // kuro
    #define VIDEOTIMNG_DEBUG_PRINTF(x)  printf x
    #define iTE6805_DEBUG_INT_PRINTF(x)  printf x
    #define HDMIRX_VIDEO_PRINTF(x)  printf x //kuro
    #define HDMIRX_AUDIO_PRINTF(x)  printf x //kuro
    #define HDMIRX_DEBUG_PRINT(x) printf x //kuro
    #define CEC_DEBUG_PRINTF(x) printf x
    #define EDID_DEBUG_PRINTF(x) //printf  x // kuro
    #define RCP_DEBUG_PRINTF(x)  //printf  x
    #define MHL3D_DEBUG_PRINTF(x) //printf  x
    #define MHL_MSC_DEBUG_PRINTF(x) printf x
    #define HDCP_DEBUG_PRINTF(x) //printf x
    #define HDCP_DEBUG_PRINTF1(x) //printf x
    #define HDCP_DEBUG_PRINTF2(x) //printf x
	#define REG_PRINTF(x) //printf x
	#define REG_MHL_PRINTF(x) //printf x
#else
    #define MHLRX_DEBUG_PRINTF(x)
    #define EQ_DEBUG_PRINTF(x)
    #define VIDEOTIMNG_DEBUG_PRINTF(x)
    #define iTE6805_DEBUG_INT_PRINTF(x)
    #define HDMIRX_VIDEO_PRINTF(x)
    #define HDMIRX_AUDIO_PRINTF(x)
    #define HDMIRX_DEBUG_PRINT(x)
    #define CEC_DEBUG_PRINTF(x)
    #define EDID_DEBUG_PRINTF(x)
    #define IT680X_DEBUG_PRINTF(x)
    #define RCP_DEBUG_PRINTF(x)
    #define MHL3D_DEBUG_PRINTF(x)
	#define MHL_MSC_DEBUG_PRINTF(x)
	#define REG_PRINTF(x)
#endif




#endif
