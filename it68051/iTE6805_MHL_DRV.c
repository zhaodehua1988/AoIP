///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_MHL_DRV.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
#include "iTE6805_Global.h"
#include "iTE6805_MHL_DRV.h"

extern _iTE6805_MHL			iTE6805_MHL;
extern _iTE6805_PARSE3D_STR	iTE6805_EDID_Parse3D;
extern _iTE6805_DATA		iTE6805_DATA;

#define MSC_3D_VIC	(0x0010)
#define MSC_3D_DTD	(0x0011)
// If needed support MHL 3D, need update this array by SVD
iTE_u8 _CODE uc3DDtd[]={0x00};		//unsupport 3D DTD information


/* supported RCP key code 00~7F, BIT7 = KeyPress */
iTE_u8 _CODE RCP_KeyCode_Supported[128]= {	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, // 0
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 2
							1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 3
							0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 4
							1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 5
							1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, // 6
							0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0};// 7

piTE_s8 _CODE RCP_KeyCodeString[0x80]={NULL};
#if 0
piTE_s8 _CODE RCP_KeyCodeString[0x80]={
	"Select", 				"Up",	 				"Down", 			"Left", 				"Right", 			"Right-Up", 		"Right-Down",		"Left-Up",
	"Left-Down", 			"Root Menu",	 		"Setup Menu", 		"Contents Menu", 		"Favorite Menu", 	"Exit",				NULL,				NULL,
	NULL,					NULL,					NULL,				NULL,					NULL,				NULL,				NULL,				NULL,
	NULL,					NULL,					NULL,				NULL,					NULL,				NULL,				NULL,				NULL,
	"Numeric 0",			"Numeric 1",			"Numeric 2",		"Numeric 3",			"Numeric 4",		"Numeric 5",		"Numeric 6",		"Numeric 7",
	"Numeric 8",			"Numeric 9",			"Dot",				"Enter",				"Clear",			NULL,				NULL,				NULL,
	"Channel Up",			"Channel Down",			"Previous Channel",	"Sound Select",			"Input Select",		"Show Information",	"Help",				"Page Up",
	"Page Down",			NULL,					NULL,				NULL,					NULL,				NULL,				NULL,				NULL,
	NULL,					"Volume Up",			"Volume Down",		"Mute",					"Play",				"Stop",				"Pause",			"Record",
	"Rewind",				"Fast Forward",			"Eject",				"Forward",			"Backward",			NULL,				NULL,				NULL,
	"Angle",				"Subpicture",			NULL,				NULL,					NULL,				NULL,				NULL,				NULL,
	NULL,					NULL,					NULL,				NULL,					NULL,				NULL,				NULL,				NULL,
	"Play Function",		"Pause Play Function",	"Record Function",	"Pause Record Function","Stop",	"Mute",		"Restore Volume Funation",				"Tune Function",
	"Select Media Function", NULL,					NULL,				NULL,					NULL,				NULL,				NULL,				NULL,
	NULL,					"F1 (Blue)",			"F2 (Red)",			"F3 (Green)",			"F4 (Yellow)",		"F5",				NULL,				NULL,
	NULL,					NULL,					NULL,				NULL,					NULL,				NULL,				"Vendor Specific",	NULL
};
#endif

#define MAX_Entry_Count 0x12
iTE_u8 Entry_Count = 0;
iTE_u8 iTE6805_MHL_MSC_Fire_With_Retry(MSC_PKT_ID MSC_PKT)
{
	#define Max_Retry_Count (5)
	iTE_u8 MSC_Result, Current_Retry_Count = 0;

	if(Entry_Count++ > MAX_Entry_Count)
	{
		Entry_Count = 0 ;
		MHLRX_DEBUG_PRINTF(("iTE6805_MHL_MSC_Fire_With_Retry recursive too many times\n"));
		MSC_Result = MSC_FAIL;
		return MSC_Result;
	}

	do{
		MSC_Result = iTE6805_MHL_MSC_Fire(MSC_PKT);
	}while( (MSC_Result != MSC_SUCCESS) &&  (Current_Retry_Count++ < Max_Retry_Count) );

	Entry_Count = 0;
	return MSC_Result;
}

iTE_u8 iTE6805_MHL_MSC_Fire(MSC_PKT_ID MSC_PKT)
{
    iTE_u8 MSC_MSG_VALUE[2];
	iTE_u8 cbuswaitcnt = 0;
	iTE_u8 MSC_Result, MSC_Result2;
	iTE_u8 i, temp;
	switch(MSC_PKT)
	{
		case MSC_PKT_MSC_MSG:
			MHLRX_DEBUG_PRINTF(("MSC_PKT_MSC_MSG \n"));
			mhlrxbwr(0x54, 0x02, &iTE6805_MHL.MSC_MSG_TX_DATA);
			mhlrxwr(0x51, BIT1);
			break;
		case MSC_PKT_WRITE_BURST:
			MHLRX_DEBUG_PRINTF(("MSC_PKT_WRITE_BURST \n"));
			mhlrxbwr(0x54, 0x02, &iTE6805_MHL.MSC_MSG_TX_DATA);
			mhlrxwr(0x51, BIT0);
			break;
		case MSC_PKT_SET_INT:
		case MSC_PKT_WRITE_STAT:
			MHLRX_DEBUG_PRINTF(("MSC_PKT_SET_INT/MSC_PKT_WRITE_STAT \n"));
			mhlrxbwr(0x54, 0x02, &iTE6805_MHL.MSC_MSG_TX_DATA);
			mhlrxwr(0x50, BIT7);
			break;
		case MSC_PKT_GET_STATE:
			MHLRX_DEBUG_PRINTF(("MSC_PKT_GET_STATE \n"));
			mhlrxwr(0x50, BIT0);
			break;
		case MSC_PKT_GET_VENDOR_ID:
			MHLRX_DEBUG_PRINTF(("MSC_PKT_GET_VENDOR_ID \n"));
			mhlrxwr(0x50, BIT1);
			break;
		case MSC_PKT_SET_HPD:
			MHLRX_DEBUG_PRINTF(("MSC_PKT_SET_HPD \n"));
			mhlrxwr(0x50, BIT2);
			break;
		case MSC_PKT_CLR_HPD:
			MHLRX_DEBUG_PRINTF(("MSC_PKT_CLR_HPD \n"));
			mhlrxwr(0x50, BIT3);
			break;
		case MSC_PKT_GET_DDC_ERRORCODE:
			MHLRX_DEBUG_PRINTF(("MSC_PKT_GET_DDC_ERRORCODE \n"));
			mhlrxwr(0x50, BIT4);
			break;
		case MSC_PKT_GET_MSC_ERRORCODE:
			MHLRX_DEBUG_PRINTF(("MSC_PKT_GET_MSC_ERRORCODE \n"));
			mhlrxwr(0x50, BIT5);
			break;
		case MSC_PKT_READ_DEVCAP:
			MHLRX_DEBUG_PRINTF(("MSC_PKT_READ_DEVCAP \n"));
			for (i = 0; i <0x10; i++)
			{
				mhlrxwr(0x54, i);	// set offset
				mhlrxwr(0x50, BIT6);// fire

				MSC_Result2 = iTE6805_MHL_MSC_Get_Response_State();

				if(MSC_Result2 == MSC_SUCCESS)
				{
					temp = mhlrxrd(0x56);
					if ((3 == i) || (4 == i))
					{
						mhlrxwr(0x5B + i, temp);	// write ADOPTER_ID_H/ADOPTER_ID_L to Register
					}

					iTE6805_MHL_MSC_DevCap_Parse(i, temp);
					return MSC_SUCCESS;
				}
				else
				{
					return MSC_FAIL;
				}
			}
			break;
		default:
			MHLRX_DEBUG_PRINTF(("UNKNOW MSC_PKT_ID !!\n"));
			break;
	}

	MSC_Result = iTE6805_MHL_MSC_Get_Response_State();

	// Check Result Below
	//if(MSC_Result == MSC_RX_RCVABORT)
	//{
	//	mhlrxwr(0x50, BIT5); // trigger MSC_PKT_GET_MSC_ERRORCODE
	//	MSC_Result2 = iTE6805_MHL_MSC_Get_Response_State();
	//	if(MSC_Result2 == MSC_SUCCESS)
	//	{
	//		MHLRX_DEBUG_PRINTF(("GET_MSC_ERRORCODE = 0x%02X\n\n", mhlrxrd(0x56)));
	//	}else{
	//		MHLRX_DEBUG_PRINTF(("Can't GET GET_MSC_ERRORCODE \n"));
	//	}
	//}

	if(MSC_Result != MSC_SUCCESS)
	{
		MHLRX_DEBUG_PRINTF(("MSC Fail\n"));
		if(MSC_PKT == MSC_PKT_WRITE_BURST)
		{
			MHLRX_DEBUG_PRINTF(("MSC Write Burst Fail need Retry \n"));
			mhlrxset(0x5C, 0x01, 0x01); // TxPktFIFO clear for Retry
			iTE6805_MHL_MSC_WriteBurstDataFill(iTE6805_MHL.MSC_TX_WB.Offset,iTE6805_MHL.MSC_TX_WB.Count, iTE6805_MHL.MSC_TX_WB.TX_Scrpad);
		}

		// receive MSC_MSG INT , there will may sub-command in a sequence
		if(mhlrxrd(0x04)&0x10)
		{
			mhlrxwr(0x04, 0x10);
			MHLRX_DEBUG_PRINTF(("MSC RX MSC_MSG INT\n"));
			mhlrxbrd(0x54, 2, &MSC_MSG_VALUE[0]);
			iTE6805_MHL_MSC_Decoder();
			mhlrxbwr(0x54, 2, &MSC_MSG_VALUE[0]);
		}
	}

	if( MSC_Result != MSC_SUCCESS )
	{
		return MSC_FAIL;
	}
	else
	{
		return MSC_SUCCESS;
	}

}

iTE_u8 iTE6805_MHL_MSC_WriteBurstDataFill(iTE_u8 Offset, iTE_u8 ucByteNo, iTE_u8 *pucData)
{
	#define MSCBurstWrID TRUE
	iTE_u8	ucTemp;

	iTE6805_MHL.MSC_TX_WB.TX_Scrpad = pucData;
	iTE6805_MHL.MSC_TX_WB.Count = ucByteNo;
	iTE6805_MHL.MSC_TX_WB.Offset = Offset;

	if(ucByteNo<=2 || (Offset+ucByteNo)>16)
	{
		MHLRX_DEBUG_PRINTF(("ERR:Set Burst Write Data Fail\n"));
		return FALSE;
	}
	else
	{
		mhlrxbwr(0x5E, 2, pucData);	// write BurstID to register
		mhlrxwr(0x54, 0x40+Offset);	// write MSCTxValue 0x40 = write to Scratchpad register
		ucTemp = 2;

		MHLRX_DEBUG_PRINTF(("iTE6805_MHL_MSC_WriteBurstDataFill ="));
		for(; ucTemp < ucByteNo;)
		{
			MHLRX_DEBUG_PRINTF((" 0x%x\n ", (int)pucData[ucTemp]));
			mhlrxwr(0x59, pucData[ucTemp++]);	// write command to MSCTxFIFO
		}
		MHLRX_DEBUG_PRINTF(("\n"));
		return TRUE;
	}
}

#define CBUSWAITNUM 100
iTE_u8 iTE6805_MHL_MSC_Get_Response_State()
{
	iTE_u8 cbuswaitcnt, MHL05, MHL18, MHL19;
	cbuswaitcnt = 0;

	// wait for CBusMSCBusy = 0
	while ((mhlrxrd(0x1C) & 0x02) && cbuswaitcnt<CBUSWAITNUM)
	{
		cbuswaitcnt++;
		delay1ms(1);
	}

	if (cbuswaitcnt == CBUSWAITNUM)
	{
		MHLRX_DEBUG_PRINTF(("ERR:MSC Wait TimeOut!\n"));
		return MSC_FAIL;
	}

	MHL05 = mhlrxrd(0x05);	mhlrxwr(0x05, 0x02);
	if (MHL05 & 0x02)
	{
		MHL18 = mhlrxrd(0x18);	mhlrxwr(0x18, MHL18);
		MHL19 = mhlrxrd(0x19);	mhlrxwr(0x19, MHL19);

		if (MHL18 & BIT0) { MHLRX_DEBUG_PRINTF(("Incomplete PKT!\n"));}
		if (MHL18 & BIT1) { MHLRX_DEBUG_PRINTF(("MSC 100ms TimeOut!\n"));}
		if (MHL18 & BIT2) { MHLRX_DEBUG_PRINTF(("Protocol Error!\n"));	}
		if (MHL18 & BIT3) { MHLRX_DEBUG_PRINTF(("Retry > 32 times!\n"));	}
		if (MHL18 & BIT4) {
			MHLRX_DEBUG_PRINTF(("Receive ABORT PKT!\n"));
			delay1ms(WAITABORTNUM); // follow 6662
			return MSC_RX_RCVABORT;
		}
		if (MHL18 & BIT5) {
			MHLRX_DEBUG_PRINTF(("MSC_MSG Requester Receive NACK PKT \n" ));
			return MSC_RX_RCVNACK;
		}
		if (MHL18 & BIT6) {
			MHLRX_DEBUG_PRINTF(("Disable HW Retry and MSC Requester Arbitration Lose at 1st PKT!" ));
			return MSC_RX_ARBLOSE;
		}
		if (MHL18 & BIT7) {
			MHLRX_DEBUG_PRINTF(("Disable HW Retry and MSC Requester Arbitration Lose before 1st PKT! \n" ));
			return MSC_RX_ARBLOSE;
		}

		if (MHL19 & BIT0) {
			MHLRX_DEBUG_PRINTF(("TX FW Fail in the middle of the command sequence!\n"));
			return MSC_FAIL;
		}
		if (MHL19 & BIT1) {
			MHLRX_DEBUG_PRINTF(("TX Fail because FW mode RxPktFIFO not empty!\n"));
			return MSC_FAIL;
		}

		MHLRX_DEBUG_PRINTF(("Unknown Issue!\n"));
		return MSC_FAIL;
	}
	else
	{
		return MSC_SUCCESS;
	}

}


void iTE6805_MHL_MSC_Decoder()
{
	#define RAPBUSYNUM      0   // 0: for ATC, 50: for Test
	iTE_u8 MHL60h[2];
	iTE_u8 KeyCode, KeyPress, Flag_Supported_RAP;

	// get RX reponse value byte0, byte1
	mhlrxbrd(0x60, 2, &MHL60h[0]);

	switch (MHL60h[0])
	{
		// MSC_MSG Error sub-command
		case MSG_PKT_MSGE:
			MHLRX_DEBUG_PRINTF(("RX MSGE => "));
			switch (MHL60h[1])
			{
				case 0x00:	MHLRX_DEBUG_PRINTF(("No Error\n")); break;
				case 0x01:	MHLRX_DEBUG_PRINTF(("ERR:Invalid sub-command code!\n")); break;
				default:	MHLRX_DEBUG_PRINTF(("ERR:Unknown MSC_MSG status code 0x%02X!\n", MHL60h[1]));
			}
			break;

		// Remote Control Protocol sub-command Handle START
		case MSG_PKT_RCP:
			KeyCode	= MHL60h[1]&0x7F;
			KeyPress = MHL60h[1]&0x80;
			if (RCP_KeyCode_Supported[KeyCode])
			{
				// Support Key code

				if(KeyPress){MHLRX_DEBUG_PRINTF((" Key Release\n"));}
				else		{MHLRX_DEBUG_PRINTF((" Key Press\n"));}
				MHLRX_DEBUG_PRINTF(("MSG_PKT_RCP : KeyCode=0x%02X", (int)KeyCode));
				if(RCP_KeyCodeString[KeyCode] != NULL)
				{
					MHLRX_DEBUG_PRINTF(("==> [%s]  ", RCP_KeyCodeString[KeyCode]));
				}
			}
			else
			{
				// NOT Support Key code
				iTE6805_MHL.MSC_MSG_TX_DATA[0] = MSG_PKT_RCPE;
				iTE6805_MHL.MSC_MSG_TX_DATA[1] = MSG_PKT_RCPE_INEFFECTIVE_KEY;
				iTE6805_MHL_MSC_Fire_With_Retry(MSC_PKT_MSC_MSG);
			}

			// MHL Spec 108, MHL RCP shall ack the sub-command by sending an RCPK sub-command or RCPE + RCPK
			iTE6805_MHL.MSC_MSG_TX_DATA[0] = MSG_PKT_RCPK;
			iTE6805_MHL.MSC_MSG_TX_DATA[1] = MHL60h[1];
			iTE6805_MHL_MSC_Fire_With_Retry(MSC_PKT_MSC_MSG);
			MHLRX_DEBUG_PRINTF(("Send a RCPK with action code = 0x%02X \n", iTE6805_MHL.MSC_MSG_TX_DATA[1]));
			break;
		case MSG_PKT_RCPK:
			KeyCode = MHL60h[1]&0x7F;
			if (RCP_KeyCodeString[KeyCode] != NULL)
			{
				MHLRX_DEBUG_PRINTF(("KeyCode=0x%02X ==> [%s]\n ", (int)KeyCode, RCP_KeyCodeString[KeyCode]));
			}
			else
			{
				MHLRX_DEBUG_PRINTF(("ERR:Reserved RCP sub-command code [0x%x]!!!\n", (int)MHL60h[1]));
			}
			break;
		case MSG_PKT_RCPE:
			switch (MHL60h[1])
			{
				case MSG_PKT_RCPE_NO_ERROR:			MHLRX_DEBUG_PRINTF(("No Error\n")); break;
				case MSG_PKT_RCPE_INEFFECTIVE_KEY:	MHLRX_DEBUG_PRINTF(("ERR:Ineffective RCP Key Code!\n")); break;
				case MSG_PKT_RCPE_BUSY:				MHLRX_DEBUG_PRINTF(("Responder Busy\n")); break;
				default:
					MHLRX_DEBUG_PRINTF(("ERR:Unknown RCP status code!\n"));
			}
			break;
		// Remote Control Protocol sub-command Handle END

		// Request Action Protocol sub-command Handle START
		case MSG_PKT_RAP: // MHL Spec page.107
			MHLRX_DEBUG_PRINTF(("RX RAP => "));
			Flag_Supported_RAP = TRUE;
			switch (MHL60h[1])
			{
				case MSG_PKT_RAP_POLL:			MHLRX_DEBUG_PRINTF(("Poll\n")); break;
				case MSG_PKT_RAP_CONTENT_ON:
					iTE6805_MHL_Set_RAP_Content(RAP_CONTENT_ON);
					MHLRX_DEBUG_PRINTF(("Change to CONTENT_ON state\n"));
					break;
				case MSG_PKT_RAP_CONTENT_OFF:
					iTE6805_MHL_Set_RAP_Content(RAP_CONTENT_OFF);
					MHLRX_DEBUG_PRINTF(("Change to CONTENT_OFF state\n"));
					break;
				default:
					MHLRX_DEBUG_PRINTF(("ERR:Unknown RAP action code 0x%02X!\n", MHL60h[1]));
					Flag_Supported_RAP = FALSE;
			}

			// MHL Spec page.106 RAP sub-commands are confirmed by the RAPK sub-command
			if (Flag_Supported_RAP)
			{
				iTE6805_MHL.MSC_MSG_TX_DATA[0] = MSG_PKT_RAPK;
				iTE6805_MHL.MSC_MSG_TX_DATA[1] = MSG_PKT_RAPK_NO_ERROR;
			}
			else {
				iTE6805_MHL.MSC_MSG_TX_DATA[0] = MSG_PKT_RAPK;
				iTE6805_MHL.MSC_MSG_TX_DATA[1] = MSG_PKT_RAPK_UNRECOGN_ACTION_CODE;
			}
			iTE6805_MHL_MSC_Fire_With_Retry(MSC_PKT_MSC_MSG);
			break;
		case MSG_PKT_RAPK:
			MHLRX_DEBUG_PRINTF(("RX RAPK => "));
			switch (MHL60h[1])
			{
				case 0x00:	MHLRX_DEBUG_PRINTF(("No Error\n")); break;
				case 0x01:	MHLRX_DEBUG_PRINTF(("ERR:Unrecognized Action Code!\n")); break;
				case 0x02:	MHLRX_DEBUG_PRINTF(("ERR:Unsupported Action Code!\n")); break;
				case 0x03:	MHLRX_DEBUG_PRINTF(("Responder Busy\n")); break;
				default:	MHLRX_DEBUG_PRINTF(("ERR:Unknown RAP status code 0x%02X!\n", MHL60h[1]));
			}
			break;
		// Request Action Protocol sub-command Handle END


		//  UTF-8 Character Protocol sub-command Handle START
		case MSG_PKT_UCP:
			MHLRX_DEBUG_PRINTF(("RX UCP [0x%x]=%c\n", (int)MHL60h[1], MHL60h[1]));

			if ((MHL60h[1] & 0x80) == 0x00)
			{
				// character 0x00~0x7f (0~127), response RCPK
				iTE6805_MHL.MSC_MSG_TX_DATA[0] = MSG_PKT_UCPK;
				iTE6805_MHL.MSC_MSG_TX_DATA[1] = MHL60h[1];
				MHLRX_DEBUG_PRINTF(("Send a UCPK with action code = 0x%02X \n", iTE6805_MHL.MSC_MSG_TX_DATA[1]));
			}
			else
			{
				iTE6805_MHL.MSC_MSG_TX_DATA[0] = MSG_PKT_UCPE;
				iTE6805_MHL.MSC_MSG_TX_DATA[1] = MSG_PKT_UCPE_INEFFECTIVE_KEY;
				MHLRX_DEBUG_PRINTF(("Send a UCPE with status code = 0x%02X \n", iTE6805_MHL.MSC_MSG_TX_DATA[1]));
			}
			iTE6805_MHL_MSC_Fire_With_Retry(MSC_PKT_MSC_MSG);
			break;
		case MSG_PKT_UCPK:
			MHLRX_DEBUG_PRINTF(("RX UCPK [0x%x] =%c\n", (int)MHL60h[1], MHL60h[1]));
			break;
		case MSG_PKT_UCPE:
			MHLRX_DEBUG_PRINTF(("RX UCPE => "));
			switch (MHL60h[1])
			{
				case MSG_PKT_UCPE_NO_ERROR:			MHLRX_DEBUG_PRINTF(("No Error\n")); break;
				case MSG_PKT_UCPE_INEFFECTIVE_KEY:	MHLRX_DEBUG_PRINTF(("ERR:Ineffective UCP Key Code!\n")); break;
				default: MHLRX_DEBUG_PRINTF(("ERR:Unknown UCP status code!\n"));
			}
			break;
		//  UTF-8 Character Protocol sub-command Handle END

		default:
			MHLRX_DEBUG_PRINTF(("ERR:Unknown MSC_MSG sub-command code 0x%02X!\n", MHL60h[0]));
			iTE6805_MHL.MSC_MSG_TX_DATA[0] = MSG_PKT_MSGE; // send MSC_MSG Error sub-command
			iTE6805_MHL.MSC_MSG_TX_DATA[1] = MSC_INVALID_CODE;
			iTE6805_MHL_MSC_Fire_With_Retry(MSC_PKT_MSC_MSG);
		}
}

void iTE6805_MHL_Set_RAP_Content(iTE_u8 RAP_CONTENT)
{
	if(iTE6805_DATA.CurrentPort != PORT0)	return;


	if(RAP_CONTENT == RAP_CONTENT_OFF)
	{
		iTE6805_Set_AVMute(AVMUTE_ON);
		iTE6805_Set_Audio_Tristate(TRISTATE_ON);
	}
	else
	{
		iTE6805_Set_AVMute(AVMUTE_OFF);
		iTE6805_Set_Audio_Tristate(TRISTATE_OFF);
	}
	iTE6805_DATA.MHL_RAP_Content_State = RAP_CONTENT;
}

void iTE6805_MHL_MSC_DevCap_Parse(iTE_u8 ucOffset, iTE_u8 ucData)
{
	switch (ucOffset)
	{
		case	0:
			MHLRX_DEBUG_PRINTF(("DEV_STATE=0x%x\n", (int)ucData));
			break;
		case	1:
			MHLRX_DEBUG_PRINTF(("MHL_Version=0x%x\n", (int)ucData));
			break;
		case	2:
			switch (ucData & 0xF){
			case	0:	MHLRX_DEBUG_PRINTF(("ERR:DEV_TYPE at least one bit must be set!\n"));	break;
			case	1:	MHLRX_DEBUG_PRINTF(("DEV_TYPE = Sink, "));								break;
			case	2:	MHLRX_DEBUG_PRINTF(("DEV_TYPE = Source, "));								break;
			case	3:	MHLRX_DEBUG_PRINTF(("DEV_TYPE = Dongle, "));								break;
			default:	MHLRX_DEBUG_PRINTF(("ERR:Reserved for future use! "));				break;
			}
			MHLRX_DEBUG_PRINTF(("POW=%d, PLIM=%d \n", (int)((ucData & 0x10) >> 4), (int)((ucData & 0x60) >> 5)));
			break;
		case	3:
			MHLRX_DEBUG_PRINTF(("ADOPTER_ID_H=0x%X, ", (int)ucData));
			break;
		case	4:
			MHLRX_DEBUG_PRINTF(("ADOPTER_ID_L=0x%X \n", (int)ucData));
			break;
		case	5:
			MHLRX_DEBUG_PRINTF(("\nVID_LINK_MODE:\n"));
			if (ucData & 0x01)	MHLRX_DEBUG_PRINTF(("	SUPP_RGB444\n"));
			if (ucData & 0x02)	MHLRX_DEBUG_PRINTF(("	SUPP_YCBCR444\n"));
			if (ucData & 0x04)	MHLRX_DEBUG_PRINTF(("	SUPP_YCBCR422\n"));
			if (ucData & 0x08)	MHLRX_DEBUG_PRINTF(("	SUPP_PPIXEL\n"));
			if (ucData & 0x10)	MHLRX_DEBUG_PRINTF(("	SUPP_ISLANDS\n"));
			if (ucData & 0x20)	MHLRX_DEBUG_PRINTF(("	SUPP_VGA\n"));
			break;
		case	6:
			MHLRX_DEBUG_PRINTF(("\nAUD_LINK_MODE:\n"));
			if (ucData & 0x01)	MHLRX_DEBUG_PRINTF(("	SUPP_2CH\n"));
			if (ucData & 0x02)	MHLRX_DEBUG_PRINTF(("	SUPP_8CH\n"));
			break;
		case	7:
			if (ucData & 0x80){
				MHLRX_DEBUG_PRINTF(("\nVIDEO_TYPE:\n"));
				if (ucData & 0x01)	MHLRX_DEBUG_PRINTF(("	VT_GRAPHICS\n"));
				if (ucData & 0x02)	MHLRX_DEBUG_PRINTF(("	VT_PHOTO\n"));
				if (ucData & 0x04)	MHLRX_DEBUG_PRINTF(("	VT_CINEMA\n"));
				if (ucData & 0x08)	MHLRX_DEBUG_PRINTF(("	VT_GAME\n"));
			}
			else{
				MHLRX_DEBUG_PRINTF(("Not Support VIDEO_TYPE\n"));
			}
			break;
		case	8:
			MHLRX_DEBUG_PRINTF(("\nLOG_DEV_MAP:\n"));
			if (ucData & 0x01)	MHLRX_DEBUG_PRINTF(("	LD_DISPLAY\n"));
			if (ucData & 0x02)	MHLRX_DEBUG_PRINTF(("	LD_VIDEO\n"));
			if (ucData & 0x04)	MHLRX_DEBUG_PRINTF(("	LD_AUDIO\n"));
			if (ucData & 0x08)	MHLRX_DEBUG_PRINTF(("	LD_MEDIA\n"));
			if (ucData & 0x10)	MHLRX_DEBUG_PRINTF(("	LD_TUNER\n"));
			if (ucData & 0x20)	MHLRX_DEBUG_PRINTF(("	LD_RECORD\n"));
			if (ucData & 0x40)	MHLRX_DEBUG_PRINTF(("	LD_SPEAKER\n"));
			if (ucData & 0x80)	MHLRX_DEBUG_PRINTF(("	LD_GUI\n"));
			break;
		case	9:
			MHLRX_DEBUG_PRINTF(("\nBANDWIDTH= %dMHz\n", (int)ucData * 5));
			break;
		case	10:
			MHLRX_DEBUG_PRINTF(("FEATURE_FLAG:\n"));
			if (ucData & 0x01)	MHLRX_DEBUG_PRINTF(("	RCP_SUPPORT\n"));
			if (ucData & 0x02)	MHLRX_DEBUG_PRINTF(("	RAP_SUPPORT\n"));
			if (ucData & 0x04)	MHLRX_DEBUG_PRINTF(("	SP_SUPPORT\n"));
			if (ucData & 0x08)	MHLRX_DEBUG_PRINTF(("	UCP_SEND_SUPPORT\n"));
			if (ucData & 0x10)	MHLRX_DEBUG_PRINTF(("	UCP_RECV_SUPPORT\n"));
			break;
		case	11:
			MHLRX_DEBUG_PRINTF(("DEVICE_ID_H=0x%x, ", (int)ucData));
			break;
		case	12:
			MHLRX_DEBUG_PRINTF(("DEVICE_ID_L=0x%x\n", (int)ucData));
			break;
		case	13:
			MHLRX_DEBUG_PRINTF(("SCRATCHPAD_SIZE=%d Bytes\n", (int)ucData));
			break;
		case	14:
			MHLRX_DEBUG_PRINTF(("INT_SIZE=%d\n", (int)(ucData & 0xf) + 1));
			MHLRX_DEBUG_PRINTF(("STAT_SIZE=%d\n", (int)(ucData >> 4) + 1));
			break;
	}
}


MHL3D_STATE MSC_3DInforSend(iTE_u8 b3dDtd)
{
	iTE_u8	ucTemp, uc3DInforLen;
	iTE_u8	ucWBData[16];
	iTE_u8	uc3DTempCnt;
	MHL3D_STATE eRet3dState;

	uc3DTempCnt = iTE6805_EDID_Parse3D.uc3DTempCnt & 0x7F;

	if (b3dDtd)
	{
		ucWBData[0] = MSC_3D_DTD >> 8;		//	3D_DTD_H
		ucWBData[1] = MSC_3D_DTD & 0xff;	//	3D_DTD_L
		ucWBData[3] = iTE6805_EDID_Parse3D.ucDtdCnt;
	}
	else
	{
		ucWBData[0] = MSC_3D_VIC >> 8;		//	3D_VIC_H
		ucWBData[1] = MSC_3D_VIC & 0xff;	//	3D_VIC_L
		ucWBData[3] = iTE6805_EDID_Parse3D.ucVicCnt;
	}

	ucWBData[2] = 0;						// check sum
	ucWBData[4] = (uc3DTempCnt / 5) + 1;	// set the seq to the brust command

	ucWBData[5] = ucWBData[3] - uc3DTempCnt;

	if (ucWBData[5] > 5)
	{
		ucWBData[5] = 5;
		eRet3dState = MHL3D_REQ_WRT;
		iTE6805_EDID_Parse3D.uc3DTempCnt += 5;
		MHLRX_DEBUG_PRINTF(("*** MSC_3DInforSend MHL3D_REQ_WRT ***\n"));
	}
	else
	{
		if (b3dDtd)
		{
			iTE6805_EDID_Parse3D.uc3DTempCnt = 0;
			eRet3dState = MHL3D_REQ_WRT;
			MHLRX_DEBUG_PRINTF(("*** MSC_3DInforSend DTD Done ***\n"));
		}
		else
		{
			iTE6805_EDID_Parse3D.uc3DTempCnt = 0x80;
			eRet3dState = MHL3D_REQ_DONE;
			MHLRX_DEBUG_PRINTF(("*** MSC_3DInforSend VIC Done ***\n"));
		}
	}
	uc3DInforLen = 6 + (ucWBData[5] * 2);

	for (ucTemp = 6; ucTemp < uc3DInforLen;)
	{
		ucWBData[ucTemp++] = 0x00;
		if (b3dDtd)
		{
			ucWBData[ucTemp++] = uc3DDtd[uc3DTempCnt++];
		}
		else
		{
			ucWBData[ucTemp++] = iTE6805_EDID_Parse3D.uc3DInfor[uc3DTempCnt++];
		}
	}

	do
	{
		if (--ucTemp != 2)
		{
			ucWBData[2] -= ucWBData[ucTemp];
		}
	} while (ucTemp);


	if (TRUE == iTE6805_MHL_MSC_WriteBurstDataFill(0x00, uc3DInforLen, ucWBData))
	{
		iTE6805_MHL.MSC_MSG_TX_DATA[0] = 0x40; // offset = Scratchpad Register
		if (iTE6805_MHL_MSC_Fire_With_Retry(MSC_PKT_WRITE_BURST) == MSC_SUCCESS)
		{
			MHLRX_DEBUG_PRINTF(("*** MSC_3DInforSend iTE6805_MHL_MSC_Fire_With_Retry SUCCESS ***\n"));
			// Set Register Change INT Register Detail
			iTE6805_MHL.MSC_MSG_TX_DATA[0] = 0x20;
			iTE6805_MHL.MSC_MSG_TX_DATA[1] = MHL_INT_DSCR_CHG;
			iTE6805_MHL_MSC_Fire_With_Retry(MSC_PKT_SET_INT);
		}
	}
	MHLRX_DEBUG_PRINTF(("*** MSC_3DInforSend eRet3dState = 0x%x ***\n", (int)eRet3dState));
	return eRet3dState;
}

void iTE6805_MHL_3D_REQ_fsm(MHL3D_STATE *e3DReqState)
{
    iTE_u8	ucTemp;
    iTE_u8	uc;

    MHLRX_DEBUG_PRINTF(("***iTE6805_MHL_3D_REQ_fsm***\n"));


	switch(*e3DReqState){

		case MHL3D_REQ_START:
			MHLRX_DEBUG_PRINTF(("***iTE6805_MHL_3D_REQ_fsm*** MHL3D_REQ_START\n"));
			for(ucTemp=0;ucTemp < 16;ucTemp++)
			{
				uc =iTE6805_EDID_Parse3D.uc3DInfor[ucTemp]; // keep the info of MHL_3D_Support FramePacking/TopBottom/SideBySide
				MHLRX_DEBUG_PRINTF(("%x ",(int)uc));
			}
			ucTemp = sizeof(uc3DDtd);
            MHLRX_DEBUG_PRINTF(("\n uc3DDtd = %X\n",(int) uc3DDtd[0]));
            if((ucTemp == 1) && (uc3DDtd[0]==0))
				ucTemp = 0;

			iTE6805_EDID_Parse3D.ucDtdCnt = ucTemp;
			iTE6805_EDID_Parse3D.uc3DTempCnt = 0x80;

			iTE6805_MHL.MSC_MSG_TX_DATA[0] = 0x20;	// RECANGE_INT offset = 0x20
			iTE6805_MHL.MSC_MSG_TX_DATA[1] = BIT2;	// REQ_WRT = BIT2
			iTE6805_MHL_MSC_Fire_With_Retry(MSC_PKT_SET_INT);

			*e3DReqState = MHL3D_REQ_WRT;
			break;

		case MHL3D_REQ_WRT:
			MHLRX_DEBUG_PRINTF(("***iTE6805_MHL_3D_REQ_fsm*** MHL3D_REQ_WRT\n"));
			*e3DReqState = MHL3D_GNT_WRT;
			break;

		// Two connected MHL device coordinate the transfer of data via their Scratchpad Registers
		// using the REQ_WRT and GRT_WRT bits, An initiator shall request clearance to write data
		// WRITE_BURST command using the REQ_WRT
		case MHL3D_GNT_WRT:
			MHLRX_DEBUG_PRINTF(("iTE6805_MHL_3D_REQ_fsm*** MHL3D_GNT_WRT\n"));
			if(iTE6805_EDID_Parse3D.uc3DTempCnt & 0x80)
			{
				*e3DReqState = MSC_3DInforSend(TRUE);
				MHLRX_DEBUG_PRINTF(("3D_REQ_fsm*** MSC_3DInforSend(TRUE)\n"));
			}
			else
			{
				*e3DReqState = MSC_3DInforSend(FALSE);
				MHLRX_DEBUG_PRINTF(("3D_REQ_fsm*** MSC_3DInforSend(FALSE)\n"));
			}
			if(*e3DReqState == MHL3D_REQ_DONE)
			{
				iTE6805_EDID_Parse3D.uc3DTempCnt = 0x80;
				MHLRX_DEBUG_PRINTF(("3D_REQ_fsm*** MHL3D_REQ_DONE\n"));
			}
			else
			{
				iTE6805_MHL.MSC_MSG_TX_DATA[0] = 0x20;	// RECANGE_INT offset = 0x20
				iTE6805_MHL.MSC_MSG_TX_DATA[1] = BIT2;	// REQ_WRT = BIT2
				iTE6805_MHL_MSC_Fire_With_Retry(MSC_PKT_SET_INT);
				*e3DReqState = MHL3D_REQ_WRT;
				MHLRX_DEBUG_PRINTF(("3D_REQ_fsm*** MHL3D_REQ_WRT\n"));
			}
			break;

		case MHL3D_REQ_DONE:
			MHLRX_DEBUG_PRINTF(("3D_REQ_fsm*** MHL3D_REQ_DONE\n"));
			break;

		default:
			break;
	}


}
