///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_sha1.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/


#ifndef _SHA_1_H_
#define _SHA_1_H_

#ifdef _MCU_8051_
    #include "Mcu.h"
#endif

#include <string.h>
//#include "debug.h"
#include "iTE6615_Debug.h"
#include "iTE6615_Config.h"
#include "iTE6615_Typedef.h"
#if Debug_message
    #include <stdio.h>
#endif

#ifndef HDCP_DEBUG_PRINTF
    #define HDCP_DEBUG_PRINTF(x)
#endif //HDCP_DEBUG_PRINTF

#ifndef HDCP_DEBUG_PRINTF1
    #define HDCP_DEBUG_PRINTF1(x)
#endif //HDCP_DEBUG_PRINTF1

#ifndef HDCP_DEBUG_PRINTF2
    #define HDCP_DEBUG_PRINTF2(x)
#endif //HDCP_DEBUG_PRINTF2


#ifndef DISABLE_HDCP
void SHA_Simple(void *p,u32 len,u8 *output);
void SHATransform(u32 * h);
#endif

#endif // _SHA_1_H_
