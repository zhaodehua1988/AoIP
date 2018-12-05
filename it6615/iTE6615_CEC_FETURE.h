///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_CEC_FETURE.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/
#ifndef _iTE6615_CEC_FETURE_H_
#define _iTE6615_CEC_FETURE_H_

#if (_ENABLE_CEC_==TRUE)
void iTE6615_CEC_CMD_Feature_decode(pCEC6615_FRAME CEC6615_FRAME);

void iTE6615_CEC_MSG_Reply_CEC_Version(u8 TARGET_LA, u8 CEC_Version);
void iTE6615_CEC_MSG_Feture_CEC_Version(u8 TARGET_LA);
void iTE6615_CEC_MSG_Future_Report_PA(u8 PA_LOW,u8 PA_HIGH, u8 prim_devtype);
void iTE6615_CEC_MSG_Future_Give_PA(void);

void iTE6615_CEC_MSG_Future_Polling(u8 TARGET_LA);
void iTE6615_CEC_MSG_Feture_Abort(u8 TARGET_LA, u8 CEC_RXCMD, u8 Abort_Reason);
void iTE6615_CEC_MSG_Reply_Feture_Abort(pCEC6615_FRAME RX_CEC6615_FRAME, u8 Abort_Reason);
void iTE6615_CEC_MSG_Abort(u8 TARGET_LA);

void iTE6615_CEC_MSG_Device_VenderID(u32 vendor_id);
void iTE6615_CEC_MSG_Give_Deive_VendorID(void);

void iTE6615_CEC_MSG_STANDBY(void);
void iTE6615_CEC_MSG_REPORT_POWER_STATUS(u8 CASTING_MODE, u8 TARGET_LA);
void iTE6615_CEC_MSG_SET_OSD_NAME(u8 TARGET_LA);
void iTE6615_CEC_MSG_ACTIVE_SOURCE(u8 PA_LOW,u8 PA_HIGH);
#endif
#endif
