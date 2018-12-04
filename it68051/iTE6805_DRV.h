///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_DRV.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
#ifndef _ITE6805_DRV_H_
#define _ITE6805_DRV_H_
#include "iTE6805_typedef.h"
void iTE6805_Set_LVDS_Video_Path(iTE_u8 LaneNum);
void iTE6805_Set_TTL_Video_Path();
void iTE6805_Set_AVMute(iTE_u8 AVMUTE_STATE);
void iTE6805_Set_ColorDepth();
void iTE6805_Set_DNScale();
void iTE6805_Set_HPD_Ctrl(iTE_u16 PORT_NUM, iTE_u16 HPD_State);
void iTE6805_Set_Video_Tristate(iTE_u8 VIDEO_STATE);
void iTE6805_Set_Audio_Tristate(iTE_u8 AUDIO_STATE);
void iTE6805_Set_1B0_By_PixelClock();

#ifdef ENABLE_68051_POWER_SAVING_MODE
void iTE6805_Set_Power_Mode(iTE_u8 Mode);
#endif

#ifdef DYNAMIC_HDCP_ENABLE_DISABLE
void iTE6805_Set_HDCP(iTE_u8 HDCP_STATE);
#endif

void iTE6805_Reset_ALL_Logic(iTE_u8 PORT_NUM);
void iTE6805_Reset_Video_Logic();
void iTE6805_Reset_Audio_Logic();
void iTE6805_Reset_Video_FIFO();

iTE_u8 iTE6805_Check_PORT0_IS_MHL_Mode(iTE_u8 PORT_NUM);
iTE_u8 iTE6805_Check_HDMI_OR_DVI_Mode(iTE_u8 PORT_NUM);
iTE_u8 iTE6805_Check_CLK_Vaild();
iTE_u8 iTE6805_Check_SCDT();
iTE_u8 iTE6805_Check_AUDT();
iTE_u8 iTE6805_Check_AVMute();
iTE_u8 iTE6805_Check_5V_State(iTE_u8 PORT_NUM);
iTE_u8 iTE6805_Check_Single_Dual_Mode();
iTE_u8 iTE6805_Check_4K_Resolution();
iTE_u8 iTE6805_Check_HDMI2();
iTE_u8 iTE6805_Check_EQ_Busy();
iTE_u8 iTE6805_Check_TMDS_Bigger_Than_1G();
iTE_u8 iTE6805_Check_Scramble_State();


void	iTE6805_Get_AVIInfoFrame_Info();
void	iTE6805_Get_VID_Info();
//iTE_u16	iTE6805_Get_VIC_Number();

void	iTE6805_Show_VID_Info();
void	iTE6805_Show_AUD_Info();
void	iTE6805_Show_AVIInfoFrame_Info();
void	chgbank( iTE_u16 bank );

void 	iTE6805_OCLK_Cal();
iTE_u16 iTE6805_OCLK_Load();

void iTE6805_Init_fsm();
void iTE6805_Init_TTL_VideoOutputConfigure();
void iTE6805_Init_CAOF();
void iTE6805_Init_6028LVDS(iTE_u8 chip);



#endif