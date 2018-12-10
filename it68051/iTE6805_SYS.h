///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_SYS.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
// IRQ
#ifndef _ITE6805_SYS_H_H_
#define _ITE6805_SYS_H_H_
void iTE6805_hdmirx_port0_SYS_irq();
void iTE6805_hdmirx_port1_SYS_irq();
void iTE6805_hdmirx_common_irq();

// finite state machine
void iTE6805_vid_fsm();
void iTE6805_aud_fsm();
void iTE6805_vid_chg(STATEV_Type NewState);
void iTE6805_aud_chg(STATEA_Type NewState);

// output setting
void iTE6805_Port_Select(iTE_u8 ucPortSel);
void iTE68051_Video_Output_Setting();
void iTE68052_Video_Output_Setting();
void iTE6805_Enable_Video_Output();
void iTE6805_Enable_Audio_Output();

// INT function
void iTE6805_INT_5VPWR_Chg(iTE_u8 ucport);
void iTE6805_INT_HDMI_DVI_Mode_Chg(iTE_u8 ucport);
void iTE6805_INT_ECC_ERROR();
void iTE6805_INT_SCDT_Chg();

// Identify chip
void iTE6805_Identify_Chip();

#ifdef EVB_AUTO_DETECT_PORT_BY_PIN
void iTE6805_Port_Detect();
#endif

#ifdef DYNAMIC_HDCP_ENABLE_DISABLE
void iTE6805_HDCP_Detect();
#endif

#endif