///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_Global.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************

#ifdef WIN32
#include <windows.h>
#include "..\\src\\USBI2C.h"
#endif

#include <string.h>    // for memcmp..

#include "iTE6615_Typedef.h"
#include "iTE6615_Config.h"
#include "iTE6615_Debug.h"

#include "iTE6615_DEV_DEFINE.h"

#if (_ENABLE_CEC_==TRUE)
    #include "iTE6615_CEC_DEFINE.h"
#endif

#include "iTE6615_DRV_TX.h"

#include "iTE6615_I2C_RDWR.h"

#include "iTE6615_EDID_PARSER.h"


#ifdef _MCU_8051_
// this file don't need add to PC Code
    #if _MCU_8051_EVB_
        #include "iTE6615_EVB_Debug.h"
    #endif
#include "iTE6615_IO.h"
#include "iTE6615_mcu.h"

#endif

//#include "iTE6615_Utility.h"
#include "iTE6615_SYS_FLOW.h"

#if (_ENABLE_CEC_==TRUE)
    #include "iTE6615_CEC_SYS.h"
#endif





