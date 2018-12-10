///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_CEC_SYS.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
#include "iTE6805_Global.h"
#include "iTE6805_CEC_SYS.h"
#include "iTE6805_CEC_DRV.h"
#include "iTE6805_CEC_FETURE.h"
//#include "iTE6805_CEC_CMDTable.c"

#define iTE6805_CEC_Logical_Address CEC_LOG_ADDR_TV

extern _iTE6805_DATA	iTE6805_DATA;
extern iTE_u8			CEC_timeunit;


#define Max_TX_Retry_Count 5
iTE_u8 Current_TX_Retry_Count = 0;

// In 6805 only need is iTE6805_CECRX
//_iTE6805_CEC iTE6805_CEC_RX[1], iTE6805_CEC_TX[1];
_iTE6805_CEC iTE6805_CEC_RX[1];
_iTE6805_CEC *iTE6805_CEC;

//cec table
#define DIRECTED	0x80
#define BCAST1_4	0x40
#define BCAST2_0	0x20	/* broadcast only allowed for >= 2.0 */
#define BCAST		(BCAST1_4 | BCAST2_0)
#define BOTH		(BCAST | DIRECTED)

/*
 * Specify minimum length and whether the message is directed, broadcast
 * or both. Messages that do not match the criteria are ignored as per
 * the CEC specification.
 */
iTE_u8 cec_msg_size[256] = {0};

void iTE6805_INIT_MSG_CHECK_TABLE()
{
	cec_msg_size[CEC_MSG_ACTIVE_SOURCE] = 4 | BCAST;
	cec_msg_size[CEC_MSG_IMAGE_VIEW_ON] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_TEXT_VIEW_ON] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_INACTIVE_SOURCE] = 4 | DIRECTED;
	cec_msg_size[CEC_MSG_REQUEST_ACTIVE_SOURCE] = 2 | BCAST;
	cec_msg_size[CEC_MSG_ROUTING_CHANGE] = 6 | BCAST;
	cec_msg_size[CEC_MSG_ROUTING_INFORMATION] = 4 | BCAST;
	cec_msg_size[CEC_MSG_SET_STREAM_PATH] = 4 | BCAST;
	cec_msg_size[CEC_MSG_STANDBY] = 2 | BOTH;
	cec_msg_size[CEC_MSG_RECORD_OFF] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_RECORD_ON] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_RECORD_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_RECORD_TV_SCREEN] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_CLEAR_ANALOGUE_TIMER] = 13 | DIRECTED;
	cec_msg_size[CEC_MSG_CLEAR_DIGITAL_TIMER] = 16 | DIRECTED;
	cec_msg_size[CEC_MSG_CLEAR_EXT_TIMER] = 13 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_ANALOGUE_TIMER] = 13 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_DIGITAL_TIMER] = 16 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_EXT_TIMER] = 13 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_TIMER_PROGRAM_TITLE] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_TIMER_CLEARED_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_TIMER_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_CEC_VERSION] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_GET_CEC_VERSION] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_PHYSICAL_ADDR] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_GET_MENU_LANGUAGE] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_PHYSICAL_ADDR] = 5 | BCAST;
	cec_msg_size[CEC_MSG_SET_MENU_LANGUAGE] = 5 | BCAST;
	cec_msg_size[CEC_MSG_REPORT_FEATURES] = 6 | BCAST;
	cec_msg_size[CEC_MSG_GIVE_FEATURES] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_DECK_CONTROL] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_DECK_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_DECK_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_PLAY] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_TUNER_DEVICE_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_SELECT_ANALOGUE_SERVICE] = 6 | DIRECTED;
	cec_msg_size[CEC_MSG_SELECT_DIGITAL_SERVICE] = 9 | DIRECTED;
	cec_msg_size[CEC_MSG_TUNER_DEVICE_STATUS] = 7 | DIRECTED;
	cec_msg_size[CEC_MSG_TUNER_STEP_DECREMENT] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_TUNER_STEP_INCREMENT] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_DEVICE_VENDOR_ID] = 5 | BCAST;
	cec_msg_size[CEC_MSG_GIVE_DEVICE_VENDOR_ID] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_VENDOR_COMMAND] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_VENDOR_COMMAND_WITH_ID] = 5 | BOTH;
	cec_msg_size[CEC_MSG_VENDOR_REMOTE_BUTTON_DOWN] = 2 | BOTH;
	cec_msg_size[CEC_MSG_VENDOR_REMOTE_BUTTON_UP] = 2 | BOTH;
	cec_msg_size[CEC_MSG_SET_OSD_STRING] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_OSD_NAME] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_OSD_NAME] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_MENU_REQUEST] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_MENU_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_USER_CONTROL_PRESSED] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_USER_CONTROL_RELEASED] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_DEVICE_POWER_STATUS] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_POWER_STATUS] = 3 | DIRECTED | BCAST2_0;
	cec_msg_size[CEC_MSG_FEATURE_ABORT] = 4 | DIRECTED;
	cec_msg_size[CEC_MSG_ABORT] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_AUDIO_STATUS] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_SYSTEM_AUDIO_MODE_STATUS] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_AUDIO_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_SHORT_AUDIO_DESCRIPTOR] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REQUEST_SHORT_AUDIO_DESCRIPTOR] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_SYSTEM_AUDIO_MODE] = 3 | BOTH;
	cec_msg_size[CEC_MSG_SYSTEM_AUDIO_MODE_REQUEST] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_SYSTEM_AUDIO_MODE_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_AUDIO_RATE] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_INITIATE_ARC] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_ARC_INITIATED] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_ARC_TERMINATED] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REQUEST_ARC_INITIATION] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REQUEST_ARC_TERMINATION] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_TERMINATE_ARC] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REQUEST_CURRENT_LATENCY] = 4 | BCAST;
	cec_msg_size[CEC_MSG_REPORT_CURRENT_LATENCY] = 7 | BCAST;
	cec_msg_size[CEC_MSG_CDC_MESSAGE] = 2 | BCAST;
}



void	iTE6805_hdmirx_CEC_irq()
{
	//0x4C
	//[5] TX Fail
	//[4] RX Done
	//[3] TX Done
	//[2] RX Fail
	//[1] RX Int : CEC follower received byte
	//[0] TX Int : CEC initiator output byte

	iTE_u8 Reg4Ch;
	Reg4Ch = cecrd(0x4C); cecwr(0x4C, Reg4Ch);

	if (Reg4Ch & BIT5) // TX INT
	{
		CEC_DEBUG_PRINTF(("TX Fail INT!\n"));
		iTE6805_CEC_chg(STATECEC_TXFail);
	}

	if (Reg4Ch & BIT3)
	{
		CEC_DEBUG_PRINTF(("TX Done INT!\n"));
		iTE6805_CEC_chg(STATECEC_TXDone);
	}

	if (Reg4Ch & (BIT4|BIT2)) // RX Done
	{
		CEC_DEBUG_PRINTF(("RX Done INT!\n"));
		iTE6805_CEC_chg(STATECEC_RXCheck);
	}

	if (Reg4Ch & 0xC0)
	{
		// DataOvFlw_Int or CmdOvFlw_Int
		CEC_DEBUG_PRINTF(("INT DataOvFlw_Int or CmdOvFlw_Int need to iTE6805_CEC_Reset_RX_FIFO \n"));
		iTE6805_CEC_Reset_RX_FIFO();
	}
}

void	iTE6805_CEC_fsm()
{
	switch(iTE6805_DATA.STATECEC)
	{
		case STATECEC_TXFail:
			break;
		case STATECEC_TXDone:
			break;
		case STATECEC_RXCheck:
			break;
		default:
			break;
	}
}

void	iTE6805_CEC_chg(STATECEC_Type NewState)
{
	//iTE_u8 i;
	iTE_u8 CMD_Vaild = TRUE;
	iTE6805_DATA.STATECEC = NewState;
	switch(NewState)
	{
		case STATECEC_TXFail:
			CEC_DEBUG_PRINTF(("CEC STATE change to STATECEC_TXFail \n"));
			if(!iTE6805_CEC_Check_IsPollingMsg())
			{
				// If is not polling, need to retry
				if(iTE6805_CEC_Get_TxFailStatus() == TX_FAIL_RECEIVE_NACK)
				{
					if( (Current_TX_Retry_Count++ < Max_TX_Retry_Count) && iTE6805_CEC_Check_Fire_Ready())
					{
						iTE6805_CEC_Fire_Tx();
					}
				}
			}
			break;
		case STATECEC_TXDone:
			CEC_DEBUG_PRINTF(("CEC STATE change to STATECEC_TXDone \n"));
			// TX Done only we need is check Polling msg
			if(iTE6805_CEC_Check_IsPollingMsg())
			{
				if(iTE6805_CEC_Get_TxFailStatus() == TX_FAIL_RECEIVE_ACK)
				{
					// Polling should not get ACK, need to do change MY_LA
					// but 6805 is sink (CEC_LOG_ADDR_TV), do not need to do anything
					// polling will trigger TX Fail, so in Reg4D, 4E, 4F will 100% get not-polling-message.
					// polling message size = 1 only header, no opCode, no Operand
				}
			}
			break;

		case STATECEC_RXCheck:
			CEC_DEBUG_PRINTF(("CEC STATE change to STATECEC_RXCheck \n"));

			// Push all Hardware get to SW Queue
			iTE6805_CEC_CMD_Push_To_Queue_Handler();

			while(iTE6805_CEC_RX_CMD_Pull()) // Get One Command
			{
				iTE6805_CEC_CMD_Print(&iTE6805_CEC->CEC_FRAME_RX);
				// Check Command Valid or not
				if(!iTE6805_CEC_CMD_Check(&iTE6805_CEC->CEC_FRAME_RX))
				{
					continue;
				}

				// Command valid, start feature decode
				iTE6805_CEC_CMD_Feature_decode(&iTE6805_CEC->CEC_FRAME_RX);
			}

			// decode done, Reset CMD Buffer
			iTE6805_CEC->Rx_Queue.Wptr = iTE6805_CEC->Rx_Queue.Rptr = 0;

			break;

		default:
			break;
	}
}

void iTE6805_CEC_CMD_Ready_To_Fire()
{
	CEC_DEBUG_PRINTF(("CEC STATE change to STATECEC_ReadyToFire \n"));
	iTE6805_CEC_CMD_Print(&iTE6805_CEC->CEC_FRAME_TX);
	cecwr(0x23, iTE6805_CEC->CEC_FRAME_TX.id.SIZE); // write size to register
	cecbwr(0x10, iTE6805_CEC->CEC_FRAME_TX.id.SIZE, &iTE6805_CEC->CEC_FRAME_TX.array[1]); // write CMD to register
	Current_TX_Retry_Count=0;
	iTE6805_CEC_Fire_Tx(); // fire
}

void iTE6805_CEC_CMD_Push_To_Queue_Handler()
{
	iTE_u8	pu8Reg51[3], RxCMDStage;
	piTE_u8	pu8RxHeader = iTE6805_CEC->Rx_TmpHeader;

	cecbrd(0x51, 0x03, pu8Reg51);
	CEC_DEBUG_PRINTF(("0x51,0x52,0x53 [%02X, %02X, %02X]\n",(int) pu8Reg51[0],(int) pu8Reg51[1],(int) pu8Reg51[2]));
	RxCMDStage = pu8Reg51[1] & 0x0F;

	do{
		if(pu8Reg51[0] & 0xCC)
		{
			// t FIFO overflow
			CEC_DEBUG_PRINTF(("**CecRx_FIFORESET\n"));
			iTE6805_CEC_Reset_RX_FIFO();
			return ;
		}
		else
		{
			// It may have Header and opcode in
			if(pu8RxHeader[0])
			{
				if(FALSE == iTE6805_CEC_RX_CMD_Push(pu8RxHeader))
				{
					return ;
				}
			}

			// Get All CMD in stage
			while(RxCMDStage--)
			{
				cecbrd(0x4D, 0x03, pu8RxHeader);	// Read 4D --> Header, OpCode, In_Cnd
				CEC_DEBUG_PRINTF(("0x%02X, %02X, %02X\n",(int) pu8RxHeader[0],(int) pu8RxHeader[1],(int) pu8RxHeader[2]));
				if(FALSE == iTE6805_CEC_RX_CMD_Push(pu8RxHeader))
				{
					CEC_DEBUG_PRINTF(("iTE6805_CEC_RX_CMD_Push Fail!\n"));
					return ;
				}
			}

			// After get all command in stage, it may get CMD again
			cecbrd(0x51, 0x02, pu8Reg51);
			RxCMDStage = pu8Reg51[1] & 0x0F;
		}
	}while(RxCMDStage);

}

void	iTE6805_CEC_CMD_Print(pCEC_FRAME CEC_FRAME)
{
	CEC_DEBUG_PRINTF(("\n"));
	CEC_DEBUG_PRINTF(("CEC CMD SIZE		=0x%02X \n"	,(int) CEC_FRAME->id.SIZE));
	CEC_DEBUG_PRINTF(("CEC CMD HEADER	=0x%02X \n"	,(int) CEC_FRAME->id.HEADER));
	CEC_DEBUG_PRINTF(("CEC CMD OPCODE	=0x%02X \n"	,(int) CEC_FRAME->id.OPCODE));
	CEC_DEBUG_PRINTF(("CEC CMD OPERAND1 =0x%02X \n"	,(int) CEC_FRAME->id.OPERAND1));
	CEC_DEBUG_PRINTF(("CEC CMD OPERAND2 =0x%02X \n"	,(int) CEC_FRAME->id.OPERAND2));
	CEC_DEBUG_PRINTF(("CEC CMD OPERAND3 =0x%02X \n"	,(int) CEC_FRAME->id.OPERAND3));
	CEC_DEBUG_PRINTF(("CEC CMD OPERAND4 =0x%02X \n"	,(int) CEC_FRAME->id.OPERAND4));
	CEC_DEBUG_PRINTF(("CEC CMD OPERAND5 =0x%02X \n"	,(int) CEC_FRAME->id.OPERAND5));
	CEC_DEBUG_PRINTF(("\n"));
}

#define CEC_MAX_MSG_SIZE        16
iTE_u8	iTE6805_CEC_CMD_Check(pCEC_FRAME CEC_FRAME)
{
	iTE_u8 Flag_Return = TRUE;
	iTE_u8 Flag_Direction = cec_msg_size[CEC_FRAME->id.OPCODE] & BOTH;
	iTE_u8 Length_Min = cec_msg_size[CEC_FRAME->id.OPCODE] & 0x1F;
	iTE_u8 IS_BOARD_CAST = FALSE;

	if(CEC_FRAME->id.Follower == CEC_LOG_ADDR_BROADCAST) IS_BOARD_CAST = TRUE;

	// Check Size, CEC_FRAME->id.SIZE <= 1 is for polling msg
	if(CEC_FRAME->id.SIZE <= 1 || CEC_FRAME->id.SIZE > CEC_MAX_MSG_SIZE || CEC_FRAME->id.SIZE < Length_Min)
	{
		CEC_DEBUG_PRINTF(("CEC CMD Check Size Fail!\n"));
		Flag_Return = FALSE;
	}

	// I'm the Initiator but I get the command
	if(CEC_FRAME->id.Initiator != CEC_LOG_ADDR_UNREGISTERED && iTE6805_CEC->MY_LA == CEC_FRAME->id.Initiator)
	{
		CEC_DEBUG_PRINTF(("CEC I'm the Initiator but I get the command!\n"));
		Flag_Return = FALSE;
	}

	/* Check if this message was for us (directed or broadcast). */
	if(IS_BOARD_CAST != TRUE && iTE6805_CEC->MY_LA != CEC_FRAME->id.Follower)
	{
		CEC_DEBUG_PRINTF(("CEC Check if this message was for us (directed or broadcast) Fail!\n"));
		Flag_Return = FALSE;
	}

	/*
	* Check if the length is not too short or if the message is a
	* broadcast message where a directed message was expected or
	* vice versa. If so, then the message has to be ignored (according
	* to section CEC 7.3 and CEC 12.2).
	*/
	if(cec_msg_size[CEC_FRAME->id.OPCODE])
	{
		if(!IS_BOARD_CAST && !(cec_msg_size[CEC_FRAME->id.OPCODE] & DIRECTED))
		{
			CEC_DEBUG_PRINTF(("CEC Check Direction Fail (Need to be Boardcast)!\n"));
			Flag_Return = FALSE;
		}
		else if(IS_BOARD_CAST && !(cec_msg_size[CEC_FRAME->id.OPCODE] & BCAST1_4))
		{
			CEC_DEBUG_PRINTF(("CEC Check Direction Fail! (Need to be Directed)!\n"));
			Flag_Return = FALSE;
		}
	}

	if(Flag_Return == FALSE)
	{
#ifndef DISABLE_CEC_REPLY
		CEC_DEBUG_PRINTF((" CEC iTE6805_CEC_CMD_Check Fail! \n"));
		CEC_DEBUG_PRINTF((" CEC Reply CEC_OP_ABORT_UNRECOGNIZED_OP\n"));
		iTE6805_CEC_MSG_Reply_Feture_Abort(CEC_FRAME, CEC_OP_ABORT_UNRECOGNIZED_OP);
#endif
		return FALSE;
	}

	return TRUE;
}

void iTE6805_CEC_INIT()
{
	printf("init cec start ..\r\n");
	iTE_u32 ulClkCnt = 0;

	iTE6805_DATA.STATECEC = STATECEC_None;
	iTE6805_CEC_RX->MY_LA = iTE6805_CEC_Logical_Address;
	iTE6805_CEC_RX->MY_PA_HIGH = 0x10;
	iTE6805_CEC_RX->MY_PA_LOW = 0x00; // ROOT TV PA FROM HIGH TO LOW = 1.0.0.0
	iTE6805_CEC_RX->Rx_Queue.Rptr = iTE6805_CEC->Rx_Queue.Wptr = 0;

	iTE6805_CEC_RX->Tx_Queue.Rptr = iTE6805_CEC->Tx_Queue.Wptr = 0;
	iTE6805_CEC_RX->Rx_TmpHeader[0] = 0;
	iTE6805_CEC_RX->Tx_QueueFull = 0;
	iTE6805_CEC_RX->Tx_CECDone = 1;
	iTE6805_CEC_RX->Tx_CECFire = 0x00;
	iTE6805_CEC_RX->Tx_CECInitDone = 0x00;
	iTE6805_CEC = &iTE6805_CEC_RX; // point to CEC_RX, if needed TX, need to implement change TX RX sel
	iTE6805_INIT_MSG_CHECK_TABLE();
	cecwr(0x08, 0x4C);
	cecwr(0x08, 0x48);

	cecwr(0x09 , 0x60);			// RD Setting
	cecwr(0x0B , 0x14);			// RD Setting

	cecwr(0x0C, CEC_timeunit);		//REG_Timer_unit[7:0]

	cecset(0x08, 0x04, 0x00);

	cecset(0x09, 0x02, 0x00);

	cecwr(0x06, 0x00);			// enable INT
	cecset(0x08, 0x01, 0x01);	// enable INT
	iTE6805_CEC_Reset_RX_FIFO();
	cecwr(0x22, iTE6805_CEC_Logical_Address);	// setting 6805 Logical Address

	cecwr(0x0A, 0x03);
	cecset(0x0A, 0x40, 0x40);			// Enable CEC

	printf("init cec end ..\r\n");
}
