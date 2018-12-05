///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_SYS_FLOW.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/
#ifndef _iTE6615_SYS_FLOW_H_
#define _iTE6615_SYS_FLOW_H_
#include "iTE6615_Config.h"
#include "iTE6615_Typedef.h"
#include "iTE6615_DEV_DEFINE.h"
// Customer Function
void iTE6615_Customer_Set_InputDualMode(u8 Input_Enable_DualMode, u8 Input_Enable_DualMode_Mode);
void iTE6615_Customer_Set_InputColorDepth(u8 Input_ColorDepth);
void iTE6615_Customer_Set_DRM_PRK(u8 Enable);

void iTE6615_fsm(void);

void iTE6615_INIT_Chip(void);
// IRQ
void iTE6615_TX_IRQ(void);
void iTE6615_HDCP_IRQ(void);
void iTE6615_SCDC_IRQ(void);
#if iTE6264
void iTE6615_LVDS_RX_IRQ( void );
#endif

// FSM
void iTE6615_sys_fsm(void);
void iTE6615_sys_chg(iTE6615_STATES_Type NewState );

void iTE6615_vid_fsm(void);
void iTE6615_vid_chg(iTE6615_STATEV_Type NewState);

void iTE6615_aud_fsm(void);
void iTE6615_aud_chg(iTE6615_STATEA_Type NewState);

//void iTE6615_hdcp_fsm(void);
void iTE6615_hdcp_chg(STATEH_Type NewState);

// IRQ
void iTE6615_INT_HPD_Chg(void);
void iTE6615_INT_RXSENSE_Chg(void);
void iTE6615_INT_SCDT_Chg(void);
void iTE6615_INT_SCDC_RRDetect( void );
void iTE6615_INT_SCDC_UpdateFlagChg( void );
void iTE6615_INT_SCDC_UpdateFlagChg_Handler( u8 rddata );
void iTE6615_INT_DDC_BusHang( void );

// EDID Parser
u8 iTE6615_EDID_Parser( void );

// output setting
void iTE6615_Set_Video_ColorSpace(void);
void iTE6615_Set_Video_Infoframe(void);
void iTE6615_Set_AVI_Infoframe(void);
void iTE6615_Set_DRM_Infoframe(void);
void iTE6615_Set_VendorSpecific_Infoframe(void);
void iTE6615_Set_HDMI2_SCDC();

// Delay TASK
void iTE6615_TASK_Handler( void );
void iTE6615_TASK_HDCP_PollingBCaps( void );
void iTE6615_TASK_HDCP_RetryAuth( void );
void iTE6615_TASK_HDCP_CheckKSVList( void );
void iTE6615_TASK_AVInfo_SysUnplug( void );
void iTE6615_TASK_AVInfo_ShowAudioInfo(void);
void iTE6615_TASK_AVInfo_ShowVideoInfo( void );

// Identify chip
void iTE6615_Identify_Chip(void);
#endif
