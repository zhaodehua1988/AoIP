///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_CEC_FETURE.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
#include "iTE6805_Global.h"
#include "iTE6805_CEC_FETURE.h"
#include "iTE6805_CEC_DRV.h"
#include "iTE6805_CEC_SYS.h"

extern _iTE6805_CEC *iTE6805_CEC;

// for more CEC Feture setting, plz reference http://lxr.free-electrons.com/source/include/uapi/linux/cec-funcs.h
// In CEC , Give mean Request

void iTE6805_CEC_CMD_Feature_decode(pCEC_FRAME CEC_FRAME)
{
	CEC_DEBUG_PRINTF(("iTE6805_CEC_CMD_Feature_decode!!"));
	// only response
	switch(CEC_FRAME->id.OPCODE)
	{

		///////////////////////////////
		// General Protocol Messages //
		///////////////////////////////
		case CEC_MSG_FEATURE_ABORT:
			/* Don't reply with CEC_MSG_FEATURE_ABORT to a CEC_MSG_FEATURE_ABORT message! */
			break;
		case CEC_MSG_ABORT:
			iTE6805_CEC_MSG_Reply_Feture_Abort(CEC_FRAME, CEC_OP_ABORT_REFUSED);
			break;

		////////////////////////////////
		// System Information Feature //
		////////////////////////////////
		case CEC_MSG_GET_CEC_VERSION:
			iTE6805_CEC_MSG_Reply_CEC_Version(CEC_FRAME->id.Initiator, CEC_OP_CEC_VERSION_1_3A);
			break;
		case CEC_MSG_CEC_VERSION:
			CEC_DEBUG_PRINTF((" CEC_MSG_CEC_VERSION = "));
			switch(CEC_FRAME->id.OPERAND1)
			{
				case CEC_OP_CEC_VERSION_1_3A:
					CEC_DEBUG_PRINTF(("CEC_OP_CEC_VERSION_1_3A"));
					break;
				case CEC_OP_CEC_VERSION_1_4:
					CEC_DEBUG_PRINTF(("CEC_OP_CEC_VERSION_1_4"));
					break;
				case CEC_OP_CEC_VERSION_2_0:
					CEC_DEBUG_PRINTF(("CEC_OP_CEC_VERSION_2_0"));
					break;
			}
			break;

		case CEC_MSG_REPORT_PHYSICAL_ADDR:
			CEC_DEBUG_PRINTF((" CEC_MSG_REPORT_PHYSICAL_ADDR = %02X%02X, Primary Device Type = %02X \n",(int) iTE6805_CEC->CEC_FRAME_RX.id.OPERAND1,(int) iTE6805_CEC->CEC_FRAME_RX.id.OPERAND2,(int) iTE6805_CEC->CEC_FRAME_RX.id.OPERAND3));
			break;
		case CEC_MSG_GIVE_PHYSICAL_ADDR:
			iTE6805_CEC_MSG_Future_Report_PA(iTE6805_CEC->MY_PA_LOW, iTE6805_CEC->MY_PA_HIGH, CEC_OP_PRIM_DEVTYPE_TV);
			break;


		//////////////////////////////////////
		// Vendor Specific Commands Feature //
		//////////////////////////////////////
		case CEC_MSG_DEVICE_VENDOR_ID:
			CEC_DEBUG_PRINTF((" CEC_MSG_DEVICE_VENDOR_ID: %02X %02X %02X\n",(int) iTE6805_CEC->CEC_FRAME_RX.id.OPERAND1,(int) iTE6805_CEC->CEC_FRAME_RX.id.OPERAND2,(int) iTE6805_CEC->CEC_FRAME_RX.id.OPERAND3));
			break;

		case CEC_MSG_GIVE_DEVICE_VENDOR_ID:
			iTE6805_CEC_MSG_Device_VenderID(CEC_VENDOR_ID);
			break;

		/////////////////////////////////
		// Feature Don't implement yet //
		/////////////////////////////////
		/* One Touch Play Feature */
		case CEC_MSG_ACTIVE_SOURCE:
		case CEC_MSG_IMAGE_VIEW_ON:
		case CEC_MSG_TEXT_VIEW_ON:

		/* Routing Control Feature */
		case CEC_MSG_INACTIVE_SOURCE:
		case CEC_MSG_REQUEST_ACTIVE_SOURCE:
		case CEC_MSG_ROUTING_CHANGE:
		case CEC_MSG_ROUTING_INFORMATION:
		case CEC_MSG_SET_STREAM_PATH:

		/* Standby Feature */
		case CEC_MSG_STANDBY:

		/* System Information Feature */
		case CEC_MSG_GET_MENU_LANGUAGE:
		case CEC_MSG_SET_MENU_LANGUAGE:
		case CEC_MSG_REPORT_FEATURES:

		/* Deck Control Feature */
		case CEC_MSG_DECK_CONTROL:
		case CEC_MSG_DECK_STATUS:
		case CEC_MSG_GIVE_DECK_STATUS:
		case CEC_MSG_PLAY:

		/* Tuner Control Feature */
		case CEC_MSG_GIVE_TUNER_DEVICE_STATUS:
		case CEC_MSG_SELECT_ANALOGUE_SERVICE:
		case CEC_MSG_SELECT_DIGITAL_SERVICE:
		case CEC_MSG_TUNER_DEVICE_STATUS:

		/* Vendor Specific Commands Feature */
		case CEC_MSG_VENDOR_COMMAND:
		case CEC_MSG_VENDOR_COMMAND_WITH_ID:
		case CEC_MSG_VENDOR_REMOTE_BUTTON_UP:
		case CEC_MSG_VENDOR_REMOTE_BUTTON_DOWN:
#ifndef DISABLE_CEC_REPLY
			CEC_DEBUG_PRINTF((" CEC Reply CEC_OP_ABORT_REFUSED\n"));
			iTE6805_CEC_MSG_Reply_Feture_Abort(CEC_FRAME, CEC_OP_ABORT_REFUSED);
#endif
			break;
		default:
#ifndef DISABLE_CEC_REPLY
			CEC_DEBUG_PRINTF((" CEC Reply CEC_OP_ABORT_UNRECOGNIZED_OP\n"));
			iTE6805_CEC_MSG_Reply_Feture_Abort(CEC_FRAME, CEC_OP_ABORT_UNRECOGNIZED_OP);
#endif
			break;
	}
}




//////////////////////////////////////////////////////////////////////////////////////////
// System Information Feature
//////////////////////////////////////////////////////////////////////////////////////////
#define CEC_OP_CEC_VERSION_1_3A                         4
#define CEC_OP_CEC_VERSION_1_4                          5
#define CEC_OP_CEC_VERSION_2_0                          6
void iTE6805_CEC_MSG_Reply_CEC_Version(iTE_u8 TARGET_LA, iTE_u8 CEC_Version)
{
	CEC_DEBUG_PRINTF((" CEC iTE6805_CEC_MSG_Reply_CEC_Version\n"));
	iTE6805_CEC_INIT_CMD(iTE6805_CEC->MY_LA,TARGET_LA);
	iTE6805_CEC->CEC_FRAME_TX.id.SIZE = 3;
	iTE6805_CEC->CEC_FRAME_TX.id.OPCODE = CEC_MSG_CEC_VERSION;
	iTE6805_CEC->CEC_FRAME_TX.id.OPERAND1 =  CEC_Version;
	iTE6805_CEC_CMD_Ready_To_Fire();
}
void iTE6805_CEC_MSG_Feture_CEC_Version(iTE_u8 TARGET_LA)
{
	CEC_DEBUG_PRINTF((" CEC iTE6805_CEC_MSG_Feture_CEC_Version\n"));
	iTE6805_CEC_INIT_CMD(iTE6805_CEC->MY_LA,TARGET_LA);
	iTE6805_CEC->CEC_FRAME_TX.id.SIZE = 2;
	iTE6805_CEC->CEC_FRAME_TX.id.OPCODE = CEC_MSG_GET_CEC_VERSION;
	iTE6805_CEC_CMD_Ready_To_Fire();
}

/* Primary Device Type Operand (prim_devtype) */
#define CEC_OP_PRIM_DEVTYPE_TV				0
#define CEC_OP_PRIM_DEVTYPE_RECORD			1
#define CEC_OP_PRIM_DEVTYPE_TUNER			3
#define CEC_OP_PRIM_DEVTYPE_PLAYBACK		4
#define CEC_OP_PRIM_DEVTYPE_AUDIOSYSTEM		5
#define CEC_OP_PRIM_DEVTYPE_SWITCH			6
#define CEC_OP_PRIM_DEVTYPE_PROCESSOR		7
void iTE6805_CEC_MSG_Future_Report_PA(iTE_u8 PA_LOW,iTE_u8 PA_HIGH, iTE_u8 prim_devtype)
{
	CEC_DEBUG_PRINTF((" CEC iTE6805_CEC_MSG_Future_Report_PA\n"));
	iTE6805_CEC_INIT_CMD(iTE6805_CEC->MY_LA,CEC_LOG_ADDR_BROADCAST);
	iTE6805_CEC->CEC_FRAME_TX.id.SIZE = 5;
	iTE6805_CEC->CEC_FRAME_TX.id.OPCODE = CEC_MSG_REPORT_PHYSICAL_ADDR;
	iTE6805_CEC->CEC_FRAME_TX.id.OPERAND1 =  PA_HIGH;
	iTE6805_CEC->CEC_FRAME_TX.id.OPERAND2 =  PA_LOW;
	iTE6805_CEC->CEC_FRAME_TX.id.OPERAND2 =  prim_devtype;
	iTE6805_CEC_CMD_Ready_To_Fire();
}

void iTE6805_CEC_MSG_Future_Give_PA()
{
	CEC_DEBUG_PRINTF((" CEC iTE6805_CEC_MSG_Future_Give_PA\n"));
	iTE6805_CEC_INIT_CMD(iTE6805_CEC->MY_LA,CEC_LOG_ADDR_BROADCAST);
	iTE6805_CEC->CEC_FRAME_TX.id.SIZE = 2;
	iTE6805_CEC->CEC_FRAME_TX.id.OPCODE = CEC_MSG_GIVE_PHYSICAL_ADDR;
	iTE6805_CEC_CMD_Ready_To_Fire();
}


//////////////////////////////////////////////////////////////////////////////////////////
// General Protocol Messages
//////////////////////////////////////////////////////////////////////////////////////////
//	The whole structure is zeroed, the len field is set to 1 (i.e. a poll
//	message) and the initiator and destination are filled in.
void iTE6805_CEC_MSG_Future_Polling(iTE_u8 TARGET_LA)
{
	CEC_DEBUG_PRINTF((" CEC iTE6805_CEC_MSG_Future_Polling\n"));
	iTE6805_CEC_INIT_CMD(iTE6805_CEC->MY_LA,TARGET_LA);
	iTE6805_CEC_CMD_Ready_To_Fire();
}

/* Abort Reason Operand (reason) */
#define CEC_OP_ABORT_UNRECOGNIZED_OP                    0
#define CEC_OP_ABORT_INCORRECT_MODE                     1
#define CEC_OP_ABORT_NO_SOURCE                          2
#define CEC_OP_ABORT_INVALID_OP                         3
#define CEC_OP_ABORT_REFUSED                            4
#define CEC_OP_ABORT_UNDETERMINED                       5
void iTE6805_CEC_MSG_Feture_Abort(iTE_u8 TARGET_LA, iTE_u8 CEC_RXCMD, iTE_u8 Abort_Reason)
{
	CEC_DEBUG_PRINTF((" CEC iTE6805_CEC_MSG_Feture_Abort\n"));
	iTE6805_CEC_INIT_CMD(iTE6805_CEC->MY_LA,TARGET_LA);
	iTE6805_CEC->CEC_FRAME_TX.id.SIZE = 4;
	iTE6805_CEC->CEC_FRAME_TX.id.OPCODE = CEC_MSG_FEATURE_ABORT;
	iTE6805_CEC->CEC_FRAME_TX.id.OPERAND1 =  CEC_RXCMD;
	iTE6805_CEC->CEC_FRAME_TX.id.OPERAND2 =  Abort_Reason;
	iTE6805_CEC_CMD_Ready_To_Fire();
}

/* This changes the current message into a feature abort message */
// some as  iTE6805_CEC_MSG_Feture_Abort function, wrap it
void iTE6805_CEC_MSG_Reply_Feture_Abort(pCEC_FRAME RX_CEC_FRAME, iTE_u8 Abort_Reason)
{
	CEC_DEBUG_PRINTF((" CEC iTE6805_CEC_MSG_Reply_Feture_Abort\n"));
	iTE6805_CEC_INIT_CMD(iTE6805_CEC->MY_LA,RX_CEC_FRAME->id.Initiator); // reply to Initiator
	iTE6805_CEC->CEC_FRAME_TX.id.SIZE = 4;
	iTE6805_CEC->CEC_FRAME_TX.id.OPCODE = CEC_MSG_FEATURE_ABORT;
	iTE6805_CEC->CEC_FRAME_TX.id.OPERAND1 =  RX_CEC_FRAME->id.OPERAND1;
	iTE6805_CEC->CEC_FRAME_TX.id.OPERAND2 =  Abort_Reason;
	iTE6805_CEC_CMD_Ready_To_Fire();
}

void iTE6805_CEC_MSG_Abort(iTE_u8 TARGET_LA)
{
	CEC_DEBUG_PRINTF((" CEC iTE6805_CEC_MSG_Abort\n"));
	iTE6805_CEC_INIT_CMD(iTE6805_CEC->MY_LA,TARGET_LA);
	iTE6805_CEC->CEC_FRAME_TX.id.SIZE = 2;
	iTE6805_CEC->CEC_FRAME_TX.id.OPCODE = CEC_MSG_ABORT;
	iTE6805_CEC_CMD_Ready_To_Fire();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Vendor Specific Commands Feature
//////////////////////////////////////////////////////////////////////////////////////////
 void iTE6805_CEC_MSG_Device_VenderID(iTE_u32 vendor_id)
{
	CEC_DEBUG_PRINTF((" CEC iTE6805_CEC_MSG_Device_VenderID\n"));
	iTE6805_CEC_INIT_CMD(iTE6805_CEC->MY_LA,CEC_LOG_ADDR_BROADCAST);
	iTE6805_CEC->CEC_FRAME_TX.id.SIZE = 5;
	iTE6805_CEC->CEC_FRAME_TX.id.OPCODE = CEC_MSG_DEVICE_VENDOR_ID;
	iTE6805_CEC->CEC_FRAME_TX.id.OPERAND1 =  vendor_id >> 16;
	iTE6805_CEC->CEC_FRAME_TX.id.OPERAND2 =  (vendor_id >> 8) & 0xff;
	iTE6805_CEC->CEC_FRAME_TX.id.OPERAND3  = vendor_id & 0xff;
	iTE6805_CEC_CMD_Ready_To_Fire();
}

void iTE6805_CEC_MSG_Give_Deive_VendorID()
{
	CEC_DEBUG_PRINTF((" CEC iTE6805_CEC_MSG_Give_Deive_VendorID\n"));
	iTE6805_CEC_INIT_CMD(iTE6805_CEC->MY_LA,CEC_LOG_ADDR_BROADCAST);
	iTE6805_CEC->CEC_FRAME_TX.id.SIZE = 2;
	iTE6805_CEC->CEC_FRAME_TX.id.OPCODE = CEC_MSG_GIVE_DEVICE_VENDOR_ID;
	iTE6805_CEC_CMD_Ready_To_Fire();
}

