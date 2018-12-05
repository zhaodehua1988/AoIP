///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_mcu.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/


#ifndef _MCU_H_
#define _MCU_H_

#include "iTE6615_Typedef.h"
#include "iTE6615_Regc51.h"

// iTE6805 MCU config
sbit Hold_Pin        = P2^0;    // kuro fix from P1^5 to P2^0


sbit  DEV0_SCL_PORT = P1^0;
sbit  DEV0_SDA_PORT = P1^1;

sbit  DEV1_SCL_PORT = P1^2;
sbit  DEV1_SDA_PORT = P1^3;

sbit  DEV2_SCL_PORT = P1^5;
sbit  DEV2_SDA_PORT = P1^6;

sbit  DEV3_SCL_PORT = P1^0;
sbit  DEV3_SDA_PORT = P1^1;

sbit  DEV4_SCL_PORT = P1^2;
sbit  DEV4_SDA_PORT = P1^3;

#endif

