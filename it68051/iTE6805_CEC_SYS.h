///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_CEC_SYS.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
#ifndef _ITE6805_CEC_SYS_H_H_
#define _ITE6805_CEC_SYS_H_H_
void	iTE6805_hdmirx_CEC_irq();

void	iTE6805_CEC_fsm();
void	iTE6805_CEC_chg(STATECEC_Type NewState);

iTE_u8	iTE6805_CEC_CMD_Check(pCEC_FRAME CEC_FRAME);
void	iTE6805_CEC_CMD_Print(pCEC_FRAME CEC_FRAME);
void	iTE6805_CEC_CMD_Push_To_Queue_Handler();
void	iTE6805_CEC_CMD_Ready_To_Fire();

void	iTE6805_CEC_INIT();

#endif
