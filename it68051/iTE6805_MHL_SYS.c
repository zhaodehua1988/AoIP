///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_MHL_SYS.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
#include "iTE6805_Global.h"
#include "iTE6805_MHL_DRV.h"

extern iTE6805_INI_REG _CODE iTE6805_INIT_MHL_TABLE[];
extern _iTE6805_DATA iTE6805_DATA;
_iTE6805_MHL iTE6805_MHL;
MHL3D_STATE e3DReqState = MHL3D_REQ_DONE;

iTE_u8 Current_DisvFailCnt=0;
iTE_u8 Current_Wakeupfailcnt=0;

#define MAX_DISCOVERY_WAITNO 300
iTE_u8 Current_MHL_Wait=0;

void iTE6805_mhlrx_irq()
{
	iTE_u8 MHL04, MHL05, MHL06;
	iTE_u8 MHLA0, MHLA1, MHLA2, MHLA3;
	iTE_u8 rddata;

	MHL04 = mhlrxrd(0x04);	mhlrxwr(0x04, MHL04);
	MHL05 = mhlrxrd(0x05)&0xFD;	mhlrxwr(0x05, MHL05);
	MHL06 = mhlrxrd(0x06);	mhlrxwr(0x06, MHL06);

	MHLA0 = mhlrxrd(0xA0);	mhlrxwr(0xA0, MHLA0);
	MHLA1 = mhlrxrd(0xA1);	mhlrxwr(0xA1, MHLA1);
	MHLA2 = mhlrxrd(0xA2);	mhlrxwr(0xA2, MHLA2);
	MHLA3 = mhlrxrd(0xA3);	mhlrxwr(0xA3, MHLA3);

	if (!(MHL04 || MHL05 || MHL06 || MHLA0 || MHLA1 || MHLA2 || MHLA3))
		return;

	if (MHL04 & 0x01) {
		MHLRX_DEBUG_PRINTF(("CBUS Link Layer TX PKT Done INT!\n"));
	}

	if (MHL04 & 0x02) {
		MHLRX_DEBUG_PRINTF(("CBUS Link Layer TX PKT Fail INT ==> "));
		rddata = mhlrxrd(0x15);	mhlrxwr(0x15, rddata & 0xF0);
		if (rddata & 0x10){ MHLRX_DEBUG_PRINTF(("TX PKT Fail when Retry > 32 times!\n")); }
		if (rddata & 0x20){ MHLRX_DEBUG_PRINTF(("TX PKT TimeOut!\n")); }
		if (rddata & 0x40){ MHLRX_DEBUG_PRINTF(("TX Initiator Arbitration Error!\n")); }
		if (rddata & 0x80){ MHLRX_DEBUG_PRINTF(("CBUS Hang!\n")); }
	}

	if (MHL04 & 0x04) {
		MHLRX_DEBUG_PRINTF(("CBUS Link Layer RX PKT Done INT!\n"));
	}

	if (MHL04 & 0x08) {

		MHLRX_DEBUG_PRINTF(("CBUS Link Layer RX PKT Fail INT ==> "));
		rddata = mhlrxrd(0x15);	mhlrxwr(0x15, rddata & 0x0F);
		if (rddata & 0x01){ MHLRX_DEBUG_PRINTF(("RX Packet Fail !!!\n")); }
		if (rddata & 0x02){ MHLRX_DEBUG_PRINTF(("RX Packet TimeOut !!!\n")); }
		if (rddata & 0x04){ MHLRX_DEBUG_PRINTF(("RX Parity Check Error !!!\n")); }
		if (rddata & 0x08){ MHLRX_DEBUG_PRINTF(("Bi-Phase Error !!!\n")); }
	}

	if (MHL04 & 0x10) {
		MHLRX_DEBUG_PRINTF(("MSC RX MSC_MSG INT!\n"));
		iTE6805_MHL_MSC_Decoder();
	}

	if (MHL04 & 0x20) {
		MHLRX_DEBUG_PRINTF(("MSC RX WRITE_STAT INT!\n"));
	}

	if (MHL04 & 0x40) {
		MHLRX_DEBUG_PRINTF(("MSC RX WRITE_BURST INT!\n"));
	}

	if (MHL05 & 0x01) {
		MHLRX_DEBUG_PRINTF(("MSC Req Done INT!\n"));
	}

	if (MHL05 & 0x04) {
		MHLRX_DEBUG_PRINTF(("MSC Rpd Done INT!\n"));
	}

	if (MHL05 & 0x08) {
		MHLRX_DEBUG_PRINTF(("MSC Rpd Fail INT!\n"));

		rddata = mhlrxrd(0x1A);	mhlrxwr(0x1A, rddata);
		if (rddata & 0x01){ MHLRX_DEBUG_PRINTF(("Initial Bad Offset!\n")); }
		if (rddata & 0x02){ MHLRX_DEBUG_PRINTF(("Incremental Bad Offset!\n")); }
		if (rddata & 0x04){ MHLRX_DEBUG_PRINTF(("Invalid Command!\n")); }
		if (rddata & 0x08){ MHLRX_DEBUG_PRINTF(("Receive dPKT in Responder Idle State!\n")); }
		if (rddata & 0x10){ MHLRX_DEBUG_PRINTF(("Incomplete PKT!\n")); }
		if (rddata & 0x20){ MHLRX_DEBUG_PRINTF(("100ms TimeOut!\n")); }
		if (rddata & 0x40){ MHLRX_DEBUG_PRINTF(("MSC_MSG Responder Busy ==> Return NACK PKT!\n")); }
		if (rddata & 0x80){ MHLRX_DEBUG_PRINTF(("Protocol Error!\n")); }

		rddata = mhlrxrd(0x1B);	mhlrxwr(0x1B, rddata);
		if (rddata & 0x01){ MHLRX_DEBUG_PRINTF(("Retry > 32 times!\n")); }
		if (rddata & 0x02){
			delay1ms(WAITABORTNUM);
			MHLRX_DEBUG_PRINTF(("Receive ABORT PKT!\n"));
		}
	}

	if (MHL05 & 0x10) {
		if ((mhlrxrd(0xB1) & 0x07) == 0x02)
		{
			MHLRX_DEBUG_PRINTF(("MHL Clock Mode : PackPixel Mode!\n"));
		}
		else
		{
			MHLRX_DEBUG_PRINTF(("MHL Clock Mode : 24 bits Mode!\n"));
		}
	}

	if (MHL05 & 0x20) {
		MHLRX_DEBUG_PRINTF(("DDC Req Fail INT!\n"));
	}

	if (MHL05 & 0x40) {
		MHLRX_DEBUG_PRINTF(("DDC Rpd Done INT!\n"));
	}

	if (MHL05 & 0x80) {
		MHLRX_DEBUG_PRINTF(("DDC Rpd Fail INT! ==> "));

		rddata = mhlrxrd(0x16);	mhlrxwr(0x16, rddata);
		if (rddata & 0x01){ MHLRX_DEBUG_PRINTF(("RxState=IDLE, receive non-SOF PKT!\n")); }
		if (rddata & 0x02){ MHLRX_DEBUG_PRINTF(("RxState/=IDLE, receive unexpected PKT!\n")); }
		if (rddata & 0x04){ MHLRX_DEBUG_PRINTF(("100ms timeout!\n")); }
		if (rddata & 0x08){ MHLRX_DEBUG_PRINTF(("100ms timeout caused by link layer error!\n")); }
		if (rddata & 0x10){ MHLRX_DEBUG_PRINTF(("Receive unexpected STOP!\n")); }
		if (rddata & 0x20){ MHLRX_DEBUG_PRINTF(("Transmit PKT failed!\n")); }
		if (rddata & 0x40){ MHLRX_DEBUG_PRINTF(("DDC bus hang!\n")); }
		if (rddata & 0x80){ MHLRX_DEBUG_PRINTF(("TxState/=IDLE, receive new PKT!\n")); }

		rddata = mhlrxrd(0x17);	mhlrxwr(0x17, rddata);
		if (rddata & 0x01){ MHLRX_DEBUG_PRINTF(("Receive TxDDCArbLose!\n")); }
	}
	if( MHL06 & 0x01 ) {
		MHLRX_DEBUG_PRINTF(("Discovery WakeUp Interrupt\n"));
		Current_Wakeupfailcnt  = 0;
		delay1ms(100);
		mhlrxset(0x28, 0x08, 0x00);
	 }
	if (MHL06 & 0x02) {
		Current_Wakeupfailcnt++;
		MHLRX_DEBUG_PRINTF(("WakeUp Fail Interrupt\n"));
		if(Current_Wakeupfailcnt > 8)
		{
			if( mhlrxrd(0x28)&0x08)
			{
				mhlrxset(0x28, 0x08, 0x00);
			}
			else
			{
				mhlrxset(0x28, 0x08, 0x08);
			}
			Current_Wakeupfailcnt=0;
		}
	}
	if (MHL06 & 0x04) {
		MHLRX_DEBUG_PRINTF(("CBUS Discovery Done INT!\n"));
		Current_DisvFailCnt = 0;
		iTE6805_DATA.MHL_RAP_Content_State = RAP_CONTENT_ON;

		MHLRX_DEBUG_PRINTF(("Set DCAP_RDY to 1 ...\n"));
		delay1ms(5);
		iTE6805_MHL.MSC_MSG_TX_DATA[0] = 0x30;
		iTE6805_MHL.MSC_MSG_TX_DATA[1] = 0x01;
		iTE6805_MHL_MSC_Fire_With_Retry(MSC_PKT_WRITE_STAT);

		MHLRX_DEBUG_PRINTF(("Set PATH_EN to 1 ...\n"));
		delay1ms(5);
		iTE6805_MHL.MSC_MSG_TX_DATA[0] = 0x31;
		iTE6805_MHL.MSC_MSG_TX_DATA[1] = 0x08;
		iTE6805_MHL_MSC_Fire_With_Retry(MSC_PKT_WRITE_STAT);

		delay1ms(150);
		MHLRX_DEBUG_PRINTF(("Set HPD to 1 ...\n"));
		iTE6805_MHL_MSC_Fire_With_Retry(MSC_PKT_SET_HPD);
		iTE6805_DATA.MHL_DiscoveryDone = 1;

	}

	if (MHL06 & 0x08) {
		Current_DisvFailCnt++;
		MHLRX_DEBUG_PRINTF(("CBUS Discovery Fail INT! ==> %dth Fail\n", Current_DisvFailCnt));
	}

	if (MHL06 & 0x10) {
		MHLRX_DEBUG_PRINTF(("CBUS PATH_EN Change INT!\n"));
	}

	if (MHL06 & 0x20) {
		MHLRX_DEBUG_PRINTF(("CBUS MUTE Change INT!\n"));
		MHLRX_DEBUG_PRINTF(("Current CBUS MUTE Status = %d \n", (mhlrxrd(0xB1) & 0x10) >> 4));
	}

	if (MHL06 & 0x40) {
		MHLRX_DEBUG_PRINTF(("CBUS DCapRdy Change INT!\n"));
		if (mhlrxrd(0xB0) & 0x01)
		{
			iTE6805_MHL_MSC_Fire_With_Retry(MSC_PKT_READ_DEVCAP);
		}
		else
		{
			MHLRX_DEBUG_PRINTF(("DCapRdy Change from '1' to '0'\n"));
		}

	}

	if (MHL06 & 0x80) {
		MHLRX_DEBUG_PRINTF(("VBUS Status Change INT!\n"));
		MHLRX_DEBUG_PRINTF(("Current VBUS Status = 0x%02X \n", (int)(mhlrxrd(0x10))));
	}

	if(MHLA0)
	{
		if (MHLA0 & 0x01) MHLRX_DEBUG_PRINTF(("MHL Device Capability Change INT!\n"));
		if (MHLA0 & 0x02) MHLRX_DEBUG_PRINTF(("MHL DSCR_CHG INT!\n"));
		if (MHLA0 & 0x04) MHLRX_DEBUG_PRINTF(("MHL REQ_WRT INT!\n"));
		if (MHLA0 & 0x08) {
			MHLRX_DEBUG_PRINTF(("MHL GNT_WRT INT!\n"));
			if (e3DReqState == MHL3D_REQ_WRT)
			{
				e3DReqState = MHL3D_GNT_WRT;
			}
		}

		if (MHLA0 & 0x10) {
			MHLRX_DEBUG_PRINTF(("3D Request INT!\n"));
			if ((MHLVer == 0x20) || (MHLVer == 0x21))
			{
				if (e3DReqState == MHL3D_REQ_DONE)
				{
					e3DReqState = MHL3D_REQ_START;
				}
			}
		}

		if(e3DReqState != MHL3D_REQ_DONE)
		{
			iTE6805_MHL_3D_REQ_fsm(&e3DReqState);
		}
	}

	if (MHLA1 & 0x02){ MHLRX_DEBUG_PRINTF(("MHL EDID Change INT!\n")); }
}





