///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_CEC_DRV.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/


void	iTE6805_CEC_Fire_Tx();

void	iTE6805_CEC_INIT_CMD(iTE_u8 Initiator,iTE_u8 Follower);

iTE_u8	iTE6805_CEC_Check_IsPollingMsg();
iTE_u8	iTE6805_CEC_Check_Fire_Ready();

iTE_u8	iTE6805_CEC_Get_TxFailStatus();

void	iTE6805_CEC_Reset_RX_FIFO();

void	iTE6805_CEC_Clear_INT();

// Queue
iTE_u8 iTE6805_CEC_RX_CMD_Push(piTE_u8 pHeader);
iTE_u8 iTE6805_CEC_RX_CMD_Pull();
