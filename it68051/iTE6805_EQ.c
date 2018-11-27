///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_EQ.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
#include "iTE6805_Global.h"
#define HW_EQ_SKEWOPT 0

#define EQ_BitErr_Threshold 0xf0

// for 68051 only
#define EQ_BitErr_G_Threshold_H 0xF0

#define Table_CED_Max_Index 5 // iTE6805_EQ_Table size + 1(auto EQ result)
#define EQ_AutoEQ_Timeout 30
#define EQ_Report_BirErr_Count 0x10

#define Channel_B 0
#define Channel_G 1
#define Channel_R 2

extern _iTE6805_DATA	iTE6805_DATA;

_CODE iTE_u8 RS_Value[] =  { 0x7F, 0x7E, 0x3F, 0x3E, 0x1F, 0x1E, 0x0F, 0x0E, 0x07, 0x06, 0x03, 0x02, 0x01, 0x00 };
iTE_u8 DFE_VALUE[14][3][3] = { 0 };  // equal as EQ RS_Value
// Value , Channel, DEF_A DEF_B DEF_C

piTE_u8 iTE6805_EQ_Table;
_CODE iTE_u8 iTE6805_EQ_Table_Hdmi10[] = { 0x01, 0x3F, 0x07, 0x0F }; // hdmi1.x from week to strong
_CODE iTE_u8 iTE6805_EQ_Table_Hdmi20[] = { 0x01, 0x07, 0x0F, 0x3F }; // hdmi2.x from strong to week

iTE_u8 CED_Err[Table_CED_Max_Index][6]; // index 0 for autoEQ, 1:4 are Manual EQ
iTE_u32 CED_Err_Total[3] = {0};

iTE_u8 gEQ_B_TargetRS = 0x00;
iTE_u8 gEQ_G_TargetRS = 0x00;
iTE_u8 gEQ_R_TargetRS = 0x00;

iTE_u8 gEQ_B_BitErr_Valid = FALSE;
iTE_u8 gEQ_G_BitErr_Valid = FALSE;
iTE_u8 gEQ_R_BitErr_Valid = FALSE;

iTE_u8 gEQ_B_BitErr_Valid_Changed = FALSE;
iTE_u8 gEQ_G_BitErr_Valid_Changed = FALSE;
iTE_u8 gEQ_R_BitErr_Valid_Changed = FALSE;

iTE_u8 gEQ_B_BitErr_Valid_index = 0;
iTE_u8 gEQ_G_BitErr_Valid_index = 0;
iTE_u8 gEQ_R_BitErr_Valid_index = 0;

iTE_u8 Flag_Done_EQ = FALSE;
iTE_u8 Flag_Done_SAREQ = FALSE;
iTE_u8 Flag_Fail_EQ = FALSE;
iTE_u8 Flag_Fail_SAREQ = FALSE;
iTE_u8 Flag_Need_Trigger_RSSKEW = FALSE;
iTE_u8 Flag_Need_Check_EQResult = FALSE;
iTE_u8 Flag_AMP_ALL_Timeout = FALSE;
iTE_u8 Flag_Need_Adjust_RSValue = FALSE;

iTE_u8 Flag_Trigger_EQ;
iTE_u8 Flag_Trigger_SAREQ;

iTE_u8 Current_EQ_Report_BirErr_Count_ADD = 1;
iTE_u8 Current_EQ_Report_BirErr_Count = 0;
iTE_s8 Current_EQ_TableIndex = 0;
iTE_u8 Current_AutoEQ_Timeout_Count = 0;

iTE_u16 Valid_CED[3];
iTE_u16 AMP_TimeOut[3];


void iTE6805_hdmirx_port0_EQ_irq()
{
	iTE_u8 Reg07h;
	chgbank(0);
	Reg07h = hdmirxrd(0x07);	hdmirxwr(0x07, Reg07h);
	if (Reg07h != 0x00)
	{
		if (Reg07h & 0x01)
		{
			EQ_DEBUG_PRINTF(("# Port 0 CH0 Lag Err#\n"));
		}
		if (Reg07h & 0x02)
		{
			EQ_DEBUG_PRINTF(("# Port 0 CH1 Lag Err#\n"));
		}
		if (Reg07h & 0x04)
		{
			EQ_DEBUG_PRINTF(("# Port 0 CH2 Lag Err#\n"));
		}
		if (Reg07h & 0x08)
		{
			EQ_DEBUG_PRINTF(("# Port 0 HDMI Bus Mode Change #\n"));
			chgbank(0);
			if (hdmirxrd(0x13)&0x40)
			{
				EQ_DEBUG_PRINTF(("# Port 0 Bus Mode : MHL #\n"));
			}
			else
			{
				EQ_DEBUG_PRINTF(("# Port 0 Bus Mode : HDMI #\n"));
			}
		}
		if (Reg07h & 0x10)
		{
			EQ_DEBUG_PRINTF(("# Port 0 SAREQ Done!!#\n"));
			Flag_Done_SAREQ = TRUE;
			// 07/10 emily command add register setting trigger-eq to 0 prevent some unstable condition
			if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
			else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
			hdmirxset(0x22, 0x04, 0x00);
			chgbank(0);
		}
		if (Reg07h & 0x20)
		{
			EQ_DEBUG_PRINTF(("# Port 0 SAREQ Fail!!#\n"));
			Flag_Fail_SAREQ = TRUE;
			if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
			else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
			hdmirxset(0x22, 0x04, 0x00);
			chgbank(0);

			iTE6805_Set_EQResult_Flag();
			if(Flag_Need_Adjust_RSValue==TRUE)
			{
				// don't truely fail.
				Flag_Fail_SAREQ	= FALSE;
				Flag_Done_SAREQ = TRUE;
				iTE6805_EQ_chg(STATEEQ_EQCheckAutoEQDone);
			}
		}
		if (Reg07h & 0x40)
		{
			EQ_DEBUG_PRINTF(("# Port 0 HDMI1 EQ Done!!#\n"));
			Flag_Done_EQ = TRUE;
			if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
			else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
			hdmirxset(0x22, 0x04, 0x00);
			chgbank(0);
		}
		if (Reg07h & 0x80)
		{
			EQ_DEBUG_PRINTF(("# Port 0 HDMI1 EQ Fail!!#\n"));
			Flag_Fail_EQ = TRUE;
			if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
			else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
			hdmirxset(0x22, 0x04, 0x00);
			chgbank(0);
		}
	}
}

void iTE6805_hdmirx_port1_EQ_irq()
{
	iTE_u8 Reg0Ch;
	chgbank(0);
	Reg0Ch = hdmirxrd(0x0C);	hdmirxwr(0x0C, Reg0Ch); // port1
	if (Reg0Ch != 0x00)
	{
		if (Reg0Ch & 0x01)
		{
			EQ_DEBUG_PRINTF(("# Port 1 CH0 Lag Err#\n"));
		}
		if (Reg0Ch & 0x02)
		{
			EQ_DEBUG_PRINTF(("# Port 1 CH1 Lag Err#\n"));
		}
		if (Reg0Ch & 0x04)
		{
			EQ_DEBUG_PRINTF(("# Port 1 CH2 Lag Err#\n"));
		}
		if (Reg0Ch & 0x10)
		{
			EQ_DEBUG_PRINTF(("# Port 1 SAR EQ Done!!#\n"));
			Flag_Done_SAREQ = TRUE;
		}
		if (Reg0Ch & 0x20)
		{
			EQ_DEBUG_PRINTF(("# Port 1 SAR EQ Fail!!#\n"));
			Flag_Fail_SAREQ = TRUE;
		}
		if (Reg0Ch & 0x40)
		{
			EQ_DEBUG_PRINTF(("# Port 1 HDMI1 EQ Done!!#\n"));
			Flag_Done_EQ = TRUE;
		}
		if (Reg0Ch & 0x80)
		{
			EQ_DEBUG_PRINTF(("# Port 1 HDMI1 EQ Fail!!#\n"));
			Flag_Fail_EQ = TRUE;
		}
	}

}


void iTE6805_EQ_fsm()
{
	#ifdef _ENABLE_IT6805_MHL_FUNCTION_
	if(iTE6805_DATA.CurrentPort == PORT0 && iTE6805_Check_PORT0_IS_MHL_Mode(PORT0)) // if MHL Mode
	{
		if(iTE6805_DATA.STATEEQ != STATEEQ_EQDone)
		{
			return;
		}
	}
	#endif

	switch (iTE6805_DATA.STATEEQ)
	{
		case STATEEQ_Off:
		case STATEEQ_KeepEQStateUntil5VOff:
			break;
		case STATEEQ_ClockONDet_And_TriggerEQ:
			if (iTE6805_Check_CLK_Vaild())
			{
				EQ_DEBUG_PRINTF(("******EQ in STATEEQ_ClockONDet_And_TriggerEQ state and iTE6805_Check_CLK_Vaild == TRUE ******\n"));
				Flag_Trigger_EQ = FALSE;
				Flag_Trigger_SAREQ = FALSE;

				if (iTE6805_Check_HDMI2() == TRUE)
				{
					iTE6805_EQ_Table = iTE6805_EQ_Table_Hdmi20;
					Flag_Trigger_SAREQ = TRUE;
					Flag_Need_Check_EQResult = TRUE;

					// for EQ effectiency, clk valid then check BitErr judge if AutoEQ or not
					EQ_DEBUG_PRINTF(("******EQ STEP1. PRE Check ******\n"));
					if (iTE6805_BitErr_Check())
					{
						#ifdef EQ_KURO_TEST
						break;
						#endif
						iTE6805_EQ_chg(STATEEQ_EQDone);
						break;
					}
					EQ_DEBUG_PRINTF(("******EQ STEP2. Trigger SAREQ******\n"));
					iTE6805_Trigger_SAREQ();
				}
				else
				{
					iTE6805_EQ_Table = iTE6805_EQ_Table_Hdmi10;
					Flag_Trigger_EQ = TRUE;
					EQ_DEBUG_PRINTF(("******EQ STEP2. Trigger EQ******\n"));
					iTE6805_Trigger_EQ(); // Trigger then wait for EQ Done interrupt
				}
				iTE6805_EQ_chg(STATEEQ_EQWaitAutoEQUntilDone);
			}
			break;
		case STATEEQ_EQWaitAutoEQUntilDone:
			EQ_DEBUG_PRINTF(("******EQ iTE6805_EQ_fsm STATEEQ_EQWaitAutoEQUntilDone state ******\n"));
			if (Flag_Done_EQ == TRUE || Flag_Done_SAREQ == TRUE)
			{
				iTE6805_EQ_chg(STATEEQ_EQCheckAutoEQDone);
				break;
			}

			if (Current_AutoEQ_Timeout_Count > EQ_AutoEQ_Timeout ||
			   (Flag_Fail_EQ == TRUE && Flag_Trigger_EQ==TRUE)    ||
			   (Flag_Fail_SAREQ == TRUE && Flag_Trigger_SAREQ==TRUE))
			{
				Current_AutoEQ_Timeout_Count = 0;
				Flag_Fail_EQ = FALSE;
				Flag_Fail_SAREQ = FALSE;
				EQ_DEBUG_PRINTF(("******EQ Can't wait EQ Done INT or EQ Fail Int, change to Manual EQ ******\n"));
				Flag_AMP_ALL_Timeout = TRUE;
				iTE6805_EQ_chg(STATEEQ_EQManual);
				break;
			}
			Current_AutoEQ_Timeout_Count++;
			break;
		case STATEEQ_EQCheckAutoEQDone:
			EQ_DEBUG_PRINTF(("******EQ iTE6805_EQ_fsm STATEEQ_EQCheckAutoEQDone state ******\n"));
			break;
		case STATEEQ_EQCheckBitErr:
			break;
		case STATEEQ_EQManual:
			break;
		case STATEEQ_WaitForManualEQ:
			iTE6805_EQ_chg(STATEEQ_EQManual);
			break;
		case STATEEQ_EQDone:
			// rpt BitErr
			if(Current_EQ_Report_BirErr_Count == 0x00)
			{
				#ifdef _ENABLE_IT6805_MHL_FUNCTION_
				// MHL do not check
				if(iTE6805_DATA.CurrentPort == PORT0 && iTE6805_Check_PORT0_IS_MHL_Mode(PORT0)) // if MHL Mode
				{
					if(iTE6805_DATA.Flag_VidStable_Done == TRUE)
					{
						iTE6805_BitErr_Get();
					}
					return;
				}
				#endif

				// HDMI Part need to check
				// For Chroma 10cm more check
				if (iTE6805_Check_HDMI2()==FALSE)
				{
					// when hdmi1.x 30m/10cm (10cm with EQ 0x01 will check ok but EQ still fail, so need auto EQ again when EQ1.x and only trigger once)
					if(iTE6805_BitErr_Check_Again() == FALSE)
					{
						EQ_DEBUG_PRINTF(("******EQ Done But Find CED Error in EQDone Stage, Restart Manual EQ ******\n"));
						iTE6805_EQ_chg(STATEEQ_ClockONDet_And_TriggerEQ);
					}
				}
				else
				{
					if(iTE6805_DATA.Flag_VidStable_Done == TRUE)
					{
						iTE6805_BitErr_Get();
					}
				}

			}

			if(Current_EQ_Report_BirErr_Count == Current_EQ_Report_BirErr_Count_ADD)
			{
				if(Current_EQ_Report_BirErr_Count_ADD < 0x20) Current_EQ_Report_BirErr_Count_ADD++;
				Current_EQ_Report_BirErr_Count = 0x00;
			}else{
				Current_EQ_Report_BirErr_Count++;
			}

			break;
	}
	chgbank(0);
}

void iTE6805_EQ_chg(STATEEQ_Type NewState)
{
	iTE_u8	i;
	iTE_u8	EQ_ManualBEST_Value_B = 0;
	iTE_u8	EQ_ManualBEST_Value_G = 0;
	iTE_u8	EQ_ManualBEST_Value_R = 0;
	iTE_u16	CED_B, CED_B_TEMP;
	iTE_u16	CED_G, CED_G_TEMP;
	iTE_u16	CED_R, CED_R_TEMP;
	iTE_u8	EQ_TableIndex;
	iTE_u8	Flag_Need_Set_CKITPENI = FALSE;
	iTE_u8	Flag_Need_to_Force_SCDC_Clock_Ratio_To_1_40 = TRUE;

	// If ECC Error need to reset HPD, this code for ignore EQ_chg when ECC Reset HPD until 5V off
	if(iTE6805_DATA.STATEEQ == STATEEQ_KeepEQStateUntil5VOff)
	{
		return;
	}

	iTE6805_DATA.STATEEQ = NewState;

	#ifdef _ENABLE_IT6805_MHL_FUNCTION_
	if(iTE6805_DATA.CurrentPort == PORT0 && iTE6805_Check_PORT0_IS_MHL_Mode(PORT0)) // If MHL Mode
	{
		if(iTE6805_DATA.STATEEQ != STATEEQ_EQDone || iTE6805_DATA.STATEEQ != STATEEQ_Off )
		{
			if(iTE6805_DATA.STATEEQ != STATEEQ_EQDone)
			{
				// MHL do not need to do EQ, only using fix EQ 9F, recommand by JJ
				EQ_DEBUG_PRINTF(("******MHL Mode Change To EQDone State, do not need to do EQ, change to EQDone Stage ******\n"));
				iTE6805_DATA.STATEEQ = STATEEQ_EQDone;
				return;
			}
		}
	}
	#endif

	switch (iTE6805_DATA.STATEEQ)
	{
		case STATEEQ_Off:
			EQ_DEBUG_PRINTF(("******EQ change to STATEEQ_Off state ******\n"));
			iTE6805_Reset_EQ();
			break;
		case STATEEQ_ClockONDet_And_TriggerEQ:
			//20170411 Flag_Need_Set_CKITPENI add from emily mail 'it66341 ff corner lot issue', when input > 1G, Reg3A7_bit6[Reg_P0_CKITPENI] = 1
			// only A0/A1 need to set this bitf
			if(iTE6805_DATA.ChipID == 0xA0 || iTE6805_DATA.ChipID == 0xA1 )
			{
				break;
			}

			if(iTE6805_Check_HDMI2() || iTE6805_Check_TMDS_Bigger_Than_1G())
			{
				Flag_Need_Set_CKITPENI = TRUE;
			}
			else
			{
				Flag_Need_Set_CKITPENI = FALSE;
			}

			if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
			else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);

			if(Flag_Need_Set_CKITPENI)
			{
				hdmirxset(0xA7, BIT6, BIT6);
			}
			else
			{
				hdmirxset(0xA7, BIT6, 0);
			}
			chgbank(0);

			break;
		case STATEEQ_EQWaitAutoEQUntilDone:
			Current_AutoEQ_Timeout_Count = 0;
			break;

		case STATEEQ_EQCheckAutoEQDone:
			EQ_DEBUG_PRINTF(("******EQ change to STATEEQ_EQCheckAutoEQDone state ******\n"));
			if (Flag_Done_EQ)
			{
				iTE6805_Report_EQ();
				// AutoEQ Done, check BitErr
				iTE6805_EQ_chg(STATEEQ_EQCheckBitErr);
			}

			if (Flag_Done_SAREQ)
			{
				// for final manual stage that needed trigger RSSKEW and check here
				if(Flag_Need_Trigger_RSSKEW == TRUE && (iTE6805_DATA.ChipID != 0xA0 && iTE6805_DATA.ChipID != 0xA1))
				{
					Flag_Need_Trigger_RSSKEW = FALSE;
					iTE6805_Report_Skew();
					iTE6805_EQ_chg(STATEEQ_EQCheckBitErr);
					break;
				}

				iTE6805_Report_EQ();
				// B0 version chip do EQ and skew together
				if(iTE6805_DATA.ChipID != 0xA0 && iTE6805_DATA.ChipID != 0xA1)
				{
					EQ_DEBUG_PRINTF(("****** ChipID=0xB0 iTE6805_Report_Skew ******\n"));
					iTE6805_Report_Skew();
				}

				if(Flag_Need_Check_EQResult == TRUE)
				{
					iTE6805_Set_EQResult_Flag();

					#ifdef EQ_KURO_TEST
					Flag_AMP_ALL_Timeout = FALSE;
					Flag_Need_Adjust_RSValue = TRUE;
					#endif

					if(Flag_AMP_ALL_Timeout == FALSE)
					{
						// need to get DFE value or adjust RS value
						iTE6805_Get_DFE(Channel_B);
						iTE6805_Get_DFE(Channel_G);
						iTE6805_Get_DFE(Channel_R);

						if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
						else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
						hdmirxset(0x22, 0x40, 0x40); // Enable ENDFE, DEF value is cauclate by AutoEQ, Disable when AutoEQ, Enable after AutoEQ
					}
					else
					{
						if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
						else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
						hdmirxset(0x22, 0x40, 0x00); // Disable DFE
						iTE6805_EQ_chg(STATEEQ_EQManual);
					}
					chgbank(0);
					Flag_Need_Check_EQResult = FALSE;
				}


				if(iTE6805_DATA.ChipID == 0xA0 || iTE6805_DATA.ChipID == 0xA1)	// add for B0, only B0 need to Trigger RSSKEWEQ
				{
					if (Flag_Need_Trigger_RSSKEW == FALSE)  // Flag_Need_Trigger_RSSKEW only change status here
					{
						Flag_Need_Trigger_RSSKEW = TRUE;
					}
					else
					{
						iTE6805_Report_Skew();
						Flag_Need_Trigger_RSSKEW = FALSE;
					}
				}

				iTE6805_EQ_chg(STATEEQ_EQCheckBitErr);
			}

			break;
		case STATEEQ_EQCheckBitErr:
			if (iTE6805_BitErr_Check())
			{
				if(Flag_Need_Trigger_RSSKEW == TRUE)
				{
					EQ_DEBUG_PRINTF(("******EQ Check OK, do not need to trigger RSSKEWEQ******\n"));
					Flag_Need_Trigger_RSSKEW = FALSE;
				}
				#ifdef EQ_KURO_TEST
				goto Force_EQ_Fail;
				#endif
				iTE6805_EQ_chg(STATEEQ_EQDone);
			}
			else
			{
#ifdef EQ_KURO_TEST
Force_EQ_Fail:
#endif
				if(CED_Err[Current_EQ_TableIndex][1] == 0x00 && CED_Err[Current_EQ_TableIndex][3] == 0x00 && CED_Err[Current_EQ_TableIndex][5] == 0x00)
				{
					EQ_DEBUG_PRINTF(("               Flag_Need_to_Force_SCDC_Clock_Ratio \n"));

					if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
					else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
					if(hdmirxrd(0xE5)&0x10)
					{
						// if already set this, clear and return
						hdmirxset(0xE5, 0x1C, 0x00);
						chgbank(0);
						iTE6805_EQ_chg(STATEEQ_Off);
						break;
					}

					if(iTE6805_Check_HDMI2())
					{
						// SCDC Clock Ratio = 1/40  ->  Force Change to 1/10
						// sometimes TMDS from 4K60 to 480p TX do not clear the SCDC Clock Ratio .....
						if(iTE6805_Check_Scramble_State())
						{
							if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
							else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
							hdmirxset(0xE5, 0x1C, 0x14); // set clock = 1/10 and enable scramble
							EQ_DEBUG_PRINTF(("               set clock = 1/10 and enable scramble \n"));
						}
						else
						{
							if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
							else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
							hdmirxset(0xE5, 0x1C, 0x10); // set clock = 1/10 and disable scramble
							EQ_DEBUG_PRINTF(("               set clock = 1/10 and disable scramble \n"));
						}
					}else{
						// SCDC Clock Ratio = 1/10  ->  Force Change to 1/40
						// for some device send 1/40 but not write SCDC register to tell RX
						if(iTE6805_Check_Scramble_State())
						{
							if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
							else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
							hdmirxset(0xE5, 0x1C, 0x1C); // set clock = 1/40 and enable scramble
							EQ_DEBUG_PRINTF(("               set clock = 1/40 and enable scramble \n"));
						}
						else
						{
							if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
							else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
							hdmirxset(0xE5, 0x1C, 0x18); // set clock = 1/40 and disable scramble
							EQ_DEBUG_PRINTF(("               set clock = 1/40 and disable scramble \n"));
						}
					}
					chgbank(0);
					iTE6805_EQ_chg(STATEEQ_Off);
					break;
				}

				if (Flag_Need_Trigger_RSSKEW == TRUE)
				{
					EQ_DEBUG_PRINTF(("****** ChipID=0xA0/A1 Flag_Need_Trigger_RSSKEW ******\n"));
					iTE6805_Trigger_RSSKEW_EQ();
					iTE6805_EQ_chg(STATEEQ_EQWaitAutoEQUntilDone);
					break;
				}
				iTE6805_EQ_chg(STATEEQ_EQManual);
			}
			break;
		case STATEEQ_EQManual:

			EQ_DEBUG_PRINTF(("******EQ STEP4. Manual EQ******\n"));
			EQ_DEBUG_PRINTF(("******EQ change to STATEEQ_EQManual state ******\n"));
			// Auto EQ Fail need to set EQ RS Value then CheckBitErr
			Current_EQ_TableIndex++;

			// Current_EQ_TableIndex will from 1 to Table_CED_Max_Index - 1
			if (Current_EQ_TableIndex < Table_CED_Max_Index)
			{
				EQ_TableIndex = Current_EQ_TableIndex - 1;

				// EQ Manual setting
				if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
				else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);

				// Force SKEW value to 0x00
				hdmirxwr(0x2C, 0x00);
				hdmirxwr(0x2D, 0x07);

				if(gEQ_B_BitErr_Valid == FALSE)
				{
					EQ_DEBUG_PRINTF(("**Manual EQ Force B RS to 0x%x **\n", (int)iTE6805_EQ_Table[EQ_TableIndex]));
					hdmirxwr(0x27, iTE6805_EQ_Table[EQ_TableIndex] | 0x80);
					if(Flag_Trigger_SAREQ == TRUE && Flag_AMP_ALL_Timeout==FALSE) iTE6805_Set_DFE(iTE6805_EQ_Table[EQ_TableIndex], Channel_B);
				}

				if(gEQ_G_BitErr_Valid == FALSE)
				{
					EQ_DEBUG_PRINTF(("**Manual EQ Force G RS to 0x%x **\n", (int)iTE6805_EQ_Table[EQ_TableIndex]));
					hdmirxwr(0x28, iTE6805_EQ_Table[EQ_TableIndex] | 0x80);
					if(Flag_Trigger_SAREQ == TRUE && Flag_AMP_ALL_Timeout==FALSE) iTE6805_Set_DFE(iTE6805_EQ_Table[EQ_TableIndex], Channel_G);
				}

				if(gEQ_R_BitErr_Valid == FALSE)
				{
					EQ_DEBUG_PRINTF(("**Manual EQ Force R RS to 0x%x **\n", (int)iTE6805_EQ_Table[EQ_TableIndex]));
					hdmirxwr(0x29, iTE6805_EQ_Table[EQ_TableIndex] | 0x80);
					if(Flag_Trigger_SAREQ == TRUE && Flag_AMP_ALL_Timeout==FALSE) iTE6805_Set_DFE(iTE6805_EQ_Table[EQ_TableIndex], Channel_R);
				}


				chgbank(0);
				iTE6805_EQ_chg(STATEEQ_EQCheckBitErr);
			}
			else
			{
				// exceed Max Index of EQ Table, none of all EQ < Threshold, can't be EQ Done
				// then need Manual EQ + RSSKEW
				Current_EQ_TableIndex = 0; // using Current_EQ_TableIndex = 0 for index or exceed
				EQ_DEBUG_PRINTF(("******EQ STEP 5(Final) . Judge Best EQ and Check BigErr******\n"));

				CED_B = 0x7FFF;
				CED_G = 0x7FFF;
				CED_R = 0x7FFF;

				// printf all the result of EQ
				for (i = 0; i < Table_CED_Max_Index; i++)
				{
					if (i == 0)
					{
						EQ_DEBUG_PRINTF(("##  i = %d  AutoEQ ##\n", (int)i));
					}
					else
					{
						EQ_DEBUG_PRINTF(("##  i = %d  EQ value = 0x%02x ##\n", (int)i, (int)iTE6805_EQ_Table[i - 1]));
					}

					if (gEQ_B_BitErr_Valid == FALSE) EQ_DEBUG_PRINTF(("Channel B CED Err=%x, %x\n", (int)CED_Err[i][1], (int)CED_Err[i][0]));
					if (gEQ_G_BitErr_Valid == FALSE) EQ_DEBUG_PRINTF(("Channel G CED Err=%x, %x\n", (int)CED_Err[i][3], (int)CED_Err[i][2]));
					if (gEQ_R_BitErr_Valid == FALSE) EQ_DEBUG_PRINTF(("Channel R CED Err=%x, %x\n", (int)CED_Err[i][5], (int)CED_Err[i][4]));
					EQ_DEBUG_PRINTF(("\n"));
				}

				// start judge best RS EQ Value
				// 0 = auto EQ result, manual here, so start from 1
				#ifdef EQ_KURO_TEST
				gEQ_B_BitErr_Valid = TRUE;
				gEQ_G_BitErr_Valid = TRUE;
				#endif
				for (i = 0; i < Table_CED_Max_Index - 1; i++)
				{

					if (gEQ_B_BitErr_Valid == FALSE && (CED_Err[i+1][1]&0x80))
					{
						CED_B_TEMP = CED_Err[i+1][0];
						CED_B_TEMP += (CED_Err[i+1][1]&0x7F) << 8;
						if(CED_B_TEMP < CED_B)
						{
							EQ_ManualBEST_Value_B = iTE6805_EQ_Table[i];
							CED_B = CED_B_TEMP;
							EQ_DEBUG_PRINTF(("**EQ Find Best B RS (BestEQValue) = 0x%02X, CED Err = 0x%04X **\n", (int)iTE6805_EQ_Table[i], CED_B));
						}
					}
					if (gEQ_G_BitErr_Valid == FALSE && (CED_Err[i+1][3]&0x80))
					{
						CED_G_TEMP = CED_Err[i+1][2];
						CED_G_TEMP += (CED_Err[i+1][3]&0x7F) << 8;
						if(CED_G_TEMP < CED_G)
						{
							EQ_ManualBEST_Value_G = iTE6805_EQ_Table[i];
							CED_G = CED_G_TEMP;
							EQ_DEBUG_PRINTF(("**EQ Find Best G RS (BestEQValue) = 0x%02X, CED Err = 0x%04X **\n", (int)iTE6805_EQ_Table[i], CED_G));
						}
					}
					if (gEQ_R_BitErr_Valid == FALSE && (CED_Err[i+1][5]&0x80))
					{
						CED_R_TEMP = CED_Err[i+1][4];
						CED_R_TEMP += (CED_Err[i+1][5]&0x7F) << 8;
						if(CED_R_TEMP < CED_R)
						{
							EQ_ManualBEST_Value_R = iTE6805_EQ_Table[i];
							CED_R = CED_R_TEMP;
							EQ_DEBUG_PRINTF(("**EQ Find Best R RS (BestEQValue) = 0x%02X, CED Err = 0x%04X **\n", (int)iTE6805_EQ_Table[i], CED_R));
						}
					}

				}

				if (gEQ_B_BitErr_Valid == FALSE) gEQ_B_TargetRS = EQ_ManualBEST_Value_B;
				if (gEQ_G_BitErr_Valid == FALSE) gEQ_G_TargetRS = EQ_ManualBEST_Value_G;
				if (gEQ_R_BitErr_Valid == FALSE) gEQ_R_TargetRS = EQ_ManualBEST_Value_R;
				EQ_DEBUG_PRINTF(("****EQ Final Judge state ****\n"));
				if (gEQ_B_BitErr_Valid == FALSE) EQ_DEBUG_PRINTF(("****EQ Force B RS value (BestEQValue) to 0x%02x ****\n", (int)gEQ_B_TargetRS));
				if (gEQ_G_BitErr_Valid == FALSE) EQ_DEBUG_PRINTF(("****EQ Force G RS value (BestEQValue) to 0x%02x ****\n", (int)gEQ_G_TargetRS));
				if (gEQ_R_BitErr_Valid == FALSE) EQ_DEBUG_PRINTF(("****EQ Force R RS value (BestEQValue) to 0x%02x ****\n", (int)gEQ_R_TargetRS));


				if(iTE6805_Check_HDMI2())
				{
					EQ_DEBUG_PRINTF(("***EQ Trigger iTE6805_Trigger_RSSKEW_EQ() in Final Step ***\n"));
					Flag_Need_Trigger_RSSKEW = TRUE;
					iTE6805_Trigger_RSSKEW_EQ();
					iTE6805_EQ_chg(STATEEQ_EQWaitAutoEQUntilDone);
				}
				else
				{
					if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
					else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
					hdmirxwr(0x27, 0x80 + gEQ_B_TargetRS);
					hdmirxwr(0x28, 0x80 + gEQ_G_TargetRS);
					hdmirxwr(0x29, 0x80 + gEQ_R_TargetRS);
					chgbank(0);
#ifdef EQ_KURO_TEST
					goto Force_EQ_Fail2;
#endif
					if(iTE6805_BitErr_Check_Again() == TRUE)
					{
						iTE6805_EQ_chg(STATEEQ_EQDone);
					}else{
#ifdef EQ_KURO_TEST
Force_EQ_Fail2:
#endif
						EQ_DEBUG_PRINTF(("***EQ iTE6805_EQ_chg(STATEEQ_WaitForManualEQ) ***\n"));
						iTE6805_EQ_chg(STATEEQ_WaitForManualEQ);
					}
				}
			}

			break;
		case STATEEQ_EQDone:
			CED_Err_Total[Channel_B] = 0 ;
			CED_Err_Total[Channel_G] = 0 ;
			CED_Err_Total[Channel_R] = 0 ;
			EQ_DEBUG_PRINTF(("\n\n\n-*-*-*EQ change to STATEEQ_EQDone state-*-*-*-\n"));
			EQ_DEBUG_PRINTF(("-*-*-*-*-*-*Report Final EQ Result-*-*-*-*-*-*"));
			iTE6805_Report_EQ();

			// only HDMI2 need Skew
			if(iTE6805_Check_HDMI2())
			{
				iTE6805_Report_Skew();
			}

			break;
	}
	chgbank(0);
}


iTE_u8 iTE6805_BitErr_Check()
{
	EQ_DEBUG_PRINTF(("******EQ Start iTE6805_BitErr_Check ******\n"));
	iTE6805_BitErr_Get(); // Get the result of EQ

	#ifndef EQ_KURO_TEST
	if (gEQ_B_BitErr_Valid == FALSE && CED_Err[Current_EQ_TableIndex][1] != 0xFF && (CED_Err[Current_EQ_TableIndex][1] == 0x80 && CED_Err[Current_EQ_TableIndex][0] < (iTE_u8)EQ_BitErr_Threshold)) gEQ_B_BitErr_Valid = TRUE;

	if (gEQ_R_BitErr_Valid == FALSE && CED_Err[Current_EQ_TableIndex][5] != 0xFF && (CED_Err[Current_EQ_TableIndex][5] == 0x80 && CED_Err[Current_EQ_TableIndex][4] < (iTE_u8)EQ_BitErr_Threshold)) gEQ_R_BitErr_Valid = TRUE;

	if(iTE6805_DATA.ChipID != 0xA0)
	{
		// same as other
		if (gEQ_G_BitErr_Valid == FALSE && CED_Err[Current_EQ_TableIndex][3] != 0xFF && (CED_Err[Current_EQ_TableIndex][3] == 0x80 && CED_Err[Current_EQ_TableIndex][2] < (iTE_u8)EQ_BitErr_Threshold)) gEQ_G_BitErr_Valid = TRUE;
	}
	else
	{
		if (gEQ_G_BitErr_Valid == FALSE &&
			CED_Err[Current_EQ_TableIndex][3] != 0xFF &&
			CED_Err[Current_EQ_TableIndex][3]&0x80 &&
			CED_Err[Current_EQ_TableIndex][3] < (iTE_u8)EQ_BitErr_G_Threshold_H)
			gEQ_G_BitErr_Valid = TRUE;
		// for special condition, judge by other channel
		if(gEQ_B_BitErr_Valid == FALSE && gEQ_R_BitErr_Valid==FALSE) gEQ_G_BitErr_Valid = FALSE;
	}

	#endif

	if (gEQ_B_BitErr_Valid == TRUE && gEQ_G_BitErr_Valid == TRUE && gEQ_R_BitErr_Valid == TRUE)
	{
		if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
		else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);

		EQ_DEBUG_PRINTF(("****** EQ Check Valid ******\n"));
		//hdmirxwr(0x22, BIT6);
		hdmirxset(0x22, BIT3, 0); // power down AutoEQ
		chgbank(0);
		hdmirxwr(0x07, 0xF0);// Clear PORT0 EQ Done , EQ Fail Interrupt
		hdmirxwr(0x0C, 0xF0);// Clear PORT1 EQ Done , EQ Fail Interrupt
		return TRUE;
	}
	else
	{
		EQ_DEBUG_PRINTF(("******EQ Check Invalid ******\n"));
		if (gEQ_B_BitErr_Valid_Changed == FALSE && gEQ_B_BitErr_Valid == TRUE)
		{
			gEQ_B_BitErr_Valid_Changed = TRUE;
			gEQ_B_BitErr_Valid_index = Current_EQ_TableIndex;
		}

		if (gEQ_G_BitErr_Valid_Changed == FALSE && gEQ_G_BitErr_Valid == TRUE)
		{
			gEQ_G_BitErr_Valid_Changed = TRUE;
			gEQ_G_BitErr_Valid_index = Current_EQ_TableIndex;
		}

		if (gEQ_R_BitErr_Valid_Changed == FALSE && gEQ_R_BitErr_Valid == TRUE)
		{
			gEQ_R_BitErr_Valid_Changed = TRUE;
			gEQ_R_BitErr_Valid_index = Current_EQ_TableIndex;
		}

		EQ_DEBUG_PRINTF(("***EQ B Valid = %x, using index = %d ***\n", (int)gEQ_B_BitErr_Valid,(int) gEQ_B_BitErr_Valid_index));
		EQ_DEBUG_PRINTF(("***EQ G Valid = %x, using index = %d ***\n", (int)gEQ_G_BitErr_Valid,(int) gEQ_G_BitErr_Valid_index));
		EQ_DEBUG_PRINTF(("***EQ R Valid = %x, using index = %d ***\n", (int)gEQ_R_BitErr_Valid,(int) gEQ_R_BitErr_Valid_index));

		return FALSE;
	}
}


iTE_u8 iTE6805_BitErr_Check_Again()
{
	iTE_u8 Result = TRUE;

	iTE6805_BitErr_Get();

	if (CED_Err[Current_EQ_TableIndex][1] != 0x80 || CED_Err[Current_EQ_TableIndex][0] > EQ_BitErr_Threshold)
	{
		gEQ_B_BitErr_Valid = FALSE;
		Result=FALSE;
	}

	if (iTE6805_DATA.ChipID != 0xA0 && (CED_Err[Current_EQ_TableIndex][3] != 0x80 || CED_Err[Current_EQ_TableIndex][2] > EQ_BitErr_Threshold))
	{
		gEQ_G_BitErr_Valid = FALSE;
		Result=FALSE;
	}

	if (CED_Err[Current_EQ_TableIndex][5] != 0x80 || CED_Err[Current_EQ_TableIndex][4] > EQ_BitErr_Threshold)
	{
		gEQ_R_BitErr_Valid = FALSE;
		Result=FALSE;
	}

	// for 68051 DFE bug
	if(iTE6805_DATA.ChipID == 0xA0 && (gEQ_B_BitErr_Valid == FALSE || gEQ_R_BitErr_Valid == FALSE))
	{
		gEQ_G_BitErr_Valid = FALSE;
		Result=FALSE;
	}
	EQ_DEBUG_PRINTF(("**iTE6805_BitErr_Check_Again** Result = %x\n", (int)Result));
	return Result;
}

void iTE6805_Trigger_EQ()
{
	iTE_u8 speed_1G_3G = FALSE;

	Flag_Done_SAREQ = FALSE;
	Flag_Done_EQ = FALSE;
	Flag_Fail_EQ = FALSE;
	Flag_Fail_SAREQ = FALSE;
	chgbank(0);
	if (iTE6805_DATA.CurrentPort == PORT0)
	{
		hdmirxwr(0x07, 0xff);
		hdmirxwr(0x23, 0xB0);
		delay1ms(10);
		hdmirxwr(0x23, 0xA0);

		EQ_DEBUG_PRINTF(("******hdmirxrd(0x14) = 0x%X ******\n", (int) hdmirxrd(0x14)));
		if(hdmirxrd(0x14)&0x01)
		{
			speed_1G_3G = TRUE;
		}
	}
	else if (iTE6805_DATA.CurrentPort == PORT1)
	{
		hdmirxwr(0x0C, 0xff);
		hdmirxwr(0x2B, 0xB0); // EQRst
		delay1ms(10);
		hdmirxwr(0x2B, 0xA0);

		EQ_DEBUG_PRINTF(("******hdmirxrd(0x17) = 0x%X ******\n", (int) hdmirxrd(0x17)));
		if(hdmirxrd(0x17)&0x01)
		{
			speed_1G_3G = TRUE;
		}
	}

	// port 0 EQ setup EQ1
	if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
	else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);

	// if 1G~3G, need to force set CSEL to 110
	if(speed_1G_3G == TRUE)
	{
		EQ_DEBUG_PRINTF(("******EQ speed 1G ~ 3G ******\n"));
		hdmirxwr(0x20, 0x36);
		hdmirxwr(0x21, 0x0E);
	}
	else
	{
		EQ_DEBUG_PRINTF(("******EQ speed under 1G ******\n"));
		hdmirxwr(0x20, 0x1B);
		hdmirxwr(0x21, 0x03);
	}

	hdmirxwr(0x26, 0x00);
	hdmirxwr(0x27, 0x00);
	hdmirxwr(0x28, 0x00);
	hdmirxwr(0x29, 0x00);
	hdmirxwr(0x22, 0x38); // [5:3] AUTOAMP , AUTOEQ , EQPWDB
	hdmirxset(0x22, 0x04, 0x04); // Trigger EQ
	delay1ms(1);
	hdmirxset(0x22, 0x04, 0x00); // Trigger EQ
	chgbank(0);
}


void iTE6805_Trigger_SAREQ()
{
	Flag_Done_SAREQ = FALSE;
	Flag_Done_EQ = FALSE;
	Flag_Fail_EQ = FALSE;
	Flag_Fail_SAREQ = FALSE;

	// MUST! disable EQTrg before EQRst
	if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
	else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);

	//  init CSC value
	hdmirxwr(0x20, 0x1B);
	hdmirxwr(0x21, 0x03);

	hdmirxset(0x20, 0x80, 0x00);	// disable CLKStb AutoEQTrg
	hdmirxwr(0x22, 0x00);	// disable [4] ENDFE, set [2] EQTrg low

	chgbank(0);
	if (iTE6805_DATA.CurrentPort == PORT0)
	{
		hdmirxwr(0x07, 0xff);
		hdmirxwr(0x23, 0xB0);
		delay1ms(10);
		hdmirxwr(0x23, 0xA0);
	}
	else if (iTE6805_DATA.CurrentPort == PORT1)
	{
		hdmirxwr(0x0C, 0xff);
		hdmirxwr(0x2B, 0xB0); // EQRst
		delay1ms(10);
		hdmirxwr(0x2B, 0xA0);
	}

	if (iTE6805_DATA.CurrentPort == PORT0) chgbank(0);
	else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(4);
	hdmirxwr(0x3B, 0x03);	// Reg_CEDOPT[5:0]

	if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
	else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);

	// port 0 EQ setup SAREQ //
	hdmirxwr(0x26, 0x00);
	hdmirxwr(0x27, 0x1F);
	hdmirxwr(0x28, 0x1F);
	hdmirxwr(0x29, 0x1F);

	// first time using this function within eq, the SKEWOPT is 0 , then 1
	if(iTE6805_DATA.ChipID == 0xA0 || iTE6805_DATA.ChipID == 0xA1)
	{
		EQ_DEBUG_PRINTF(("****** SKEWOPT = 0 ******\n"));
		hdmirxwr(0x30, 0x80);			// [2] SKEWOPT
	}
	else
	{
		EQ_DEBUG_PRINTF(("****** SKEWOPT = 1 ******\n"));
		// add for B0 Chip

		hdmirxset(0x2C, 0xC0, 0xC0);
		hdmirxset(0x2D, 0xF0, 0x20);
		hdmirxset(0x2D, 0x07, 0x00);
		hdmirxwr(0x30, 0x8C);
	}

	hdmirxwr(0x31, 0xB0);							// AMPTime[7:0]

	hdmirxwr(0x32, 0x43);
	hdmirxwr(0x33, 0x47);
	hdmirxwr(0x34, 0x4B);
	hdmirxwr(0x35, 0x53);

	hdmirxset(0x36, 0xc0, 0x00); // [7:6] AMPTime[9:8]

	hdmirxwr(0x37, 0x0B);        // [7:6] RecChannel, [4]: RSOnestage,
	                             // [3] IgnoreOPT, [1:0] MacthNoSel
	hdmirxwr(0x38, 0xF2);        // [7:4] MonTime
	hdmirxwr(0x39, 0x0D);       // [5] POLBOPT, [4] ADDPClrOPT, [3:0] CED Valid Threshold 0x0D
	hdmirxset(0x4A, 0x80, 0x00);
	hdmirxset(0x4B, 0x80, 0x00);
	hdmirxset(0x54, 0x80, 0x80); // Reg_EN_PREEQ
	hdmirxset(0x54, 0x38, 0x38);

	hdmirxwr(0x55, 0x40);  // RSM Threshold
	hdmirxset(0x22, 0x04, 0x04);  // Trigger EQ
	chgbank(0);
}


void iTE6805_Trigger_RSSKEW_EQ(void)
{
	Flag_Done_SAREQ = FALSE;
	Flag_Done_EQ = FALSE;
	Flag_Fail_EQ = FALSE;
	Flag_Fail_SAREQ = FALSE;

	//iTE_u8 SAREQDone;
	if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
	else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
	hdmirxset(0x22, 0x04, 0x00);

	// force RS to Target RS
	EQ_DEBUG_PRINTF(("Force Set Rec_B_RS =%x \n", (int)(gEQ_B_TargetRS)));
	EQ_DEBUG_PRINTF(("Force Set Rec_G_RS =%x \n", (int)(gEQ_G_TargetRS)));
	EQ_DEBUG_PRINTF(("Force Set Rec_R_RS =%x \n", (int)(gEQ_R_TargetRS)));

	// force RS to Target RS
	hdmirxwr(0x27, 0x80 + gEQ_B_TargetRS);
	hdmirxwr(0x28, 0x80 + gEQ_G_TargetRS);
	hdmirxwr(0x29, 0x80 + gEQ_R_TargetRS);

	// setting DFE Value
	if(Flag_AMP_ALL_Timeout == FALSE)
	{
		// DFE Enable is all on or all of, can't be setting only for one on
		iTE6805_Set_DFE(gEQ_B_TargetRS, Channel_B);
		iTE6805_Set_DFE(gEQ_G_TargetRS, Channel_G);
		iTE6805_Set_DFE(gEQ_R_TargetRS, Channel_R);

		if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
		else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
		hdmirxset(0x4B, BIT7, BIT7);
	}

	if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
	else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
	hdmirxwr(0x2D, 0x00); // No Force SKEW
	hdmirxwr(0x30, 0x94); // [4] Manual RS, [2] SKEWOPT
	hdmirxwr(0x31, 0xB0); // AMPTime[7:0]
	hdmirxset(0x37, 0x10, 0x10); //[4]: RSOnestage
	hdmirxset(0x54, 0x80, 0x00); // Reg_EN_PREEQ
	hdmirxset(0x22, 0x04, 0x04); // Trigger EQ
	chgbank(0);
}

void iTE6805_Reset_EQ()
{
	gEQ_B_BitErr_Valid = FALSE;
	gEQ_G_BitErr_Valid = FALSE;
	gEQ_R_BitErr_Valid = FALSE;

	gEQ_B_BitErr_Valid_Changed = FALSE;
	gEQ_G_BitErr_Valid_Changed = FALSE;
	gEQ_R_BitErr_Valid_Changed = FALSE;

	gEQ_B_BitErr_Valid_index = 0;
	gEQ_G_BitErr_Valid_index = 0;
	gEQ_R_BitErr_Valid_index = 0;

	Flag_Done_EQ = FALSE;
	Flag_Done_SAREQ = FALSE;
	Flag_Fail_EQ = FALSE;
	Flag_Fail_SAREQ = FALSE;
	Flag_Need_Trigger_RSSKEW = FALSE;

	Current_EQ_Report_BirErr_Count = 0;
	Current_EQ_Report_BirErr_Count_ADD = 0x20;
	Current_EQ_TableIndex = 0;
	Current_AutoEQ_Timeout_Count = 0;


	if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
	else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
	// Force SKEW value to 0x00
	hdmirxwr(0x2C, 0x00);
	hdmirxwr(0x2D, 0x07);

	chgbank(0);
	if (iTE6805_DATA.CurrentPort == PORT0)
	{
		hdmirxwr(0x07, 0xff); // Clear Interrupt
		hdmirxwr(0x23, 0xB0); // EQRst
		delay1ms(1);
		hdmirxwr(0x23, 0xA0);
		chgbank(3);
		hdmirxwr(0x27, EQ_INIT_VALUE);
		hdmirxwr(0x28, EQ_INIT_VALUE);
		hdmirxwr(0x29, EQ_INIT_VALUE);
		hdmirxwr(0x22, 0x00);
		hdmirxset(0x4B, BIT7, 0); // using AutoEQ DFE Value
	}
	if (iTE6805_DATA.CurrentPort == PORT1)
	{
		hdmirxwr(0x0C, 0xff);
		hdmirxwr(0x2B, 0xB0); // EQRst
		delay1ms(1);
		hdmirxwr(0x2B, 0xA0);
		chgbank(7);
		hdmirxwr(0x27, EQ_INIT_VALUE);
		hdmirxwr(0x28, EQ_INIT_VALUE);
		hdmirxwr(0x29, EQ_INIT_VALUE);
		hdmirxwr(0x22, 0x00);
		hdmirxset(0x4B, BIT7, 0); // using AutoEQ DFE Value
	}
	chgbank(0);
}

void iTE6805_Set_EQResult_Flag()
{

	iTE_u8 All_Ignore = 0, i;
	iTE_u8 Reg63h, Reg6Dh;
	iTE_u16 Reg64h = 0, Reg6Eh = 0;

	Flag_AMP_ALL_Timeout = FALSE;
	Flag_Need_Adjust_RSValue = FALSE;
	if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
	else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);

	for(i=0;i<3;i++)
	{
		hdmirxset(0x37, 0xC0, i<<6);
		Reg63h=hdmirxrd(0x63);
		Reg64h=hdmirxrd(0x64);
		Reg6Dh=hdmirxrd(0x6D);
		Reg6Eh=hdmirxrd(0x6E);
		Valid_CED[i] =(Reg6Eh<<8) + Reg6Dh;
		AMP_TimeOut[i]= (Reg64h<<8) + Reg63h;
		EQ_DEBUG_PRINTF(("CH %d, AMP_TimeOut = 0x%04X \n", (int) i, (int)AMP_TimeOut[i] ));
		EQ_DEBUG_PRINTF(("CH %d, Valid_CED = 0x%04X \n", (int) i, (int)Valid_CED[i] ));
		if(AMP_TimeOut[i] == 0x3FFF)
		{
			Flag_AMP_ALL_Timeout=TRUE; // Channel Timeout, ignore ALL DFE value setting
			EQ_DEBUG_PRINTF(("CH %d, ALL_TimeOut, set Flag_AMP_ALL_Timeout = TRUE \n", (int) i));
		}
		if(Valid_CED[i] == 0x0000)
		{
			All_Ignore=1;
			EQ_DEBUG_PRINTF(("CH %d, All Ingore !!!!!!!!\n", (int) i));
		}
	}

	if(Flag_AMP_ALL_Timeout == FALSE && All_Ignore == 1)
	{
		Flag_Need_Adjust_RSValue = TRUE;
		EQ_DEBUG_PRINTF(("\n                EQ need to adjust RS value \n\n"));
	}
}

void iTE6805_Report_EQ()
{
	if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
	else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);

	gEQ_B_TargetRS = hdmirxrd(0xD5) & 0x7F;
	gEQ_G_TargetRS = hdmirxrd(0xD6) & 0x7F;
	gEQ_R_TargetRS = hdmirxrd(0xD7) & 0x7F;
	EQ_DEBUG_PRINTF(("\n\n\n"));
	EQ_DEBUG_PRINTF(("iTE6805_Report_EQ gEQ_B_TargetRS =%x \n", (int)gEQ_B_TargetRS));
	EQ_DEBUG_PRINTF(("iTE6805_Report_EQ gEQ_G_TargetRS =%x \n", (int)gEQ_G_TargetRS));
	EQ_DEBUG_PRINTF(("iTE6805_Report_EQ gEQ_R_TargetRS =%x \n", (int)gEQ_R_TargetRS));
	EQ_DEBUG_PRINTF(("\n\n\n"));
	chgbank(0);
}

void iTE6805_Report_Skew()
{
	iTE_u8 Reg74h, Reg75h, Reg76h, Reg77h;
	iTE_u8 Reg78h, Reg79h, Reg7Ah;
	iTE_u8 CHB_SKEW, CHG_SKEW, CHR_SKEW;
	iTE_u16 SKEW00_BErr, SKEWP0_BErr, SKEWP1_BErr;
	iTE_u8 Rec_Channel;
	if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
	else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
	// SKEW Read Back
	Reg74h = hdmirxrd(0x74);
	CHB_SKEW = (Reg74h & 0x03);
	CHG_SKEW = ((Reg74h & 0x0C) >> 2);
	CHR_SKEW = ((Reg74h & 0x30) >> 4);

	if (CHB_SKEW == 0x02) EQ_DEBUG_PRINTF(("CHB SKEW : ENSKEW =1, PSKEW= 0 \n"));
	else if (CHB_SKEW == 0x03) EQ_DEBUG_PRINTF(("CHB SKEW : ENSKEW =1, PSKEW= 1 \n"));
	else EQ_DEBUG_PRINTF(("CHB SKEW : ENSKEW =0, PSKEW= 0 \n"));

	if (CHG_SKEW == 0x02) EQ_DEBUG_PRINTF(("CHG SKEW : ENSKEW =1, PSKEW= 0 \n"));
	else if (CHG_SKEW == 0x03) EQ_DEBUG_PRINTF(("CHG SKEW : ENSKEW =1, PSKEW= 1 \n"));
	else EQ_DEBUG_PRINTF(("CHG SKEW : ENSKEW =0, PSKEW= 0 \n"));

	if (CHR_SKEW == 0x02) EQ_DEBUG_PRINTF(("CHR SKEW : ENSKEW =1, PSKEW= 0 \n"));
	else if (CHR_SKEW == 0x03) EQ_DEBUG_PRINTF(("CHR SKEW : ENSKEW =1, PSKEW= 1 \n"));
	else EQ_DEBUG_PRINTF(("CHR SKEW : ENSKEW =0, PSKEW= 0 \n"));

	for (Rec_Channel = 0; Rec_Channel<3; Rec_Channel++)
	{
		hdmirxset(0x37, 0xC0, Rec_Channel << 6);
		Reg75h = hdmirxrd(0x75);
		Reg76h = hdmirxrd(0x76);
		Reg77h = hdmirxrd(0x77);
		Reg78h = hdmirxrd(0x78);
		Reg79h = hdmirxrd(0x79);
		Reg7Ah = hdmirxrd(0x7A);
		SKEW00_BErr = (Reg78h << 8) + Reg75h;
		SKEWP0_BErr = (Reg79h << 8) + Reg76h;
		SKEWP1_BErr = (Reg7Ah << 8) + Reg77h;
		EQ_DEBUG_PRINTF(("\n SKEW info of Selected channel: %d : \n",(int) Rec_Channel));
		EQ_DEBUG_PRINTF((" SKEW00 : BErr =%d, SKEWP0 : BErr =%d, SKEWP1 : BErr =%d \n\n", (int)SKEW00_BErr, (int)SKEWP0_BErr, (int)SKEWP1_BErr));
	}
	chgbank(0);
}


void iTE6805_BitErr_Get()
{
	iTE_u8 i;

	if (iTE6805_DATA.CurrentPort == PORT0) chgbank(0);
	else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(4);
	hdmirxset(0x3B, 0x08, 0x08);  // read CED Error from SAREQ CEDError Counter

	if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
	else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
	hdmirxset(0x55, 0x80, 0x00);

	for (i = 0; i < 0x01; i++)
	{
		hdmirxwr(0xe9, 0x80);
		delay1ms(100);
		hdmirxwr(0xe9, 0x00);
		CED_Err[Current_EQ_TableIndex][0] = hdmirxrd(0xEA);
		CED_Err[Current_EQ_TableIndex][1] = hdmirxrd(0xEB);
		hdmirxwr(0xe9, 0x20);
		CED_Err[Current_EQ_TableIndex][2] = hdmirxrd(0xEA);
		CED_Err[Current_EQ_TableIndex][3] = hdmirxrd(0xEB);
		hdmirxwr(0xe9, 0x40);
		CED_Err[Current_EQ_TableIndex][4] = hdmirxrd(0xEA);
		CED_Err[Current_EQ_TableIndex][5] = hdmirxrd(0xEB);

		EQ_DEBUG_PRINTF(("......................................................\n"));
		EQ_DEBUG_PRINTF(("\n"));

		if(iTE6805_DATA.STATEEQ ==  STATEEQ_EQDone)
		{
			CED_Err_Total[Channel_B] += CED_Err[Current_EQ_TableIndex][0];
			CED_Err_Total[Channel_B] += (CED_Err[Current_EQ_TableIndex][1]&0x7F) << 8;

			CED_Err_Total[Channel_G] += CED_Err[Current_EQ_TableIndex][2];
			CED_Err_Total[Channel_G] += (CED_Err[Current_EQ_TableIndex][3]&0x7F) << 8;

			CED_Err_Total[Channel_R] += CED_Err[Current_EQ_TableIndex][4];
			CED_Err_Total[Channel_R] += (CED_Err[Current_EQ_TableIndex][5]&0x7F) << 8;

			EQ_DEBUG_PRINTF(("Channel B Current CED Err=%x, %x  Total Err=0x%x\n", (int)CED_Err[Current_EQ_TableIndex][1], (int)CED_Err[Current_EQ_TableIndex][0] , (int)CED_Err_Total[Channel_B] ));

			#ifdef _ENABLE_IT6805_MHL_FUNCTION_
			if(iTE6805_DATA.CurrentPort != PORT0 || iTE6805_Check_PORT0_IS_MHL_Mode(PORT0) == MODE_HDMI) // if MHL Mode
			{
			#endif

				EQ_DEBUG_PRINTF(("Channel G Current CED Err=%x, %x  Total Err=0x%x\n", (int)CED_Err[Current_EQ_TableIndex][3], (int)CED_Err[Current_EQ_TableIndex][2] , (int)CED_Err_Total[Channel_G] ));
				EQ_DEBUG_PRINTF(("Channel R Current CED Err=%x, %x  Total Err=0x%x\n", (int)CED_Err[Current_EQ_TableIndex][5], (int)CED_Err[Current_EQ_TableIndex][4] , (int)CED_Err_Total[Channel_R] ));

			#ifdef _ENABLE_IT6805_MHL_FUNCTION_
			}
			else
			{
				EQ_DEBUG_PRINTF(("MHL CED Error only need to check B Channel\n"));
			}
			#endif

		}
		else
		{
			EQ_DEBUG_PRINTF(("Channel B CED Err=%x, %x\n", (int)CED_Err[Current_EQ_TableIndex][1], (int)CED_Err[Current_EQ_TableIndex][0]));
			EQ_DEBUG_PRINTF(("Channel G CED Err=%x, %x\n", (int)CED_Err[Current_EQ_TableIndex][3], (int)CED_Err[Current_EQ_TableIndex][2]));
			EQ_DEBUG_PRINTF(("Channel R CED Err=%x, %x\n", (int)CED_Err[Current_EQ_TableIndex][5], (int)CED_Err[Current_EQ_TableIndex][4]));
		}

		EQ_DEBUG_PRINTF(("\n"));
	}
	EQ_DEBUG_PRINTF(("......................................................\n"));
	EQ_DEBUG_PRINTF(("\n"));

	chgbank(0);

}

void iTE6805_Get_DFE(iTE_u8 Channel_Color)
{
	iTE_u8 i;
	iTE_u8 AMP_A, AMP_B, AMP_C, AMP_D;
	iTE_u8 Tap1_Sign, Tap1_Value;
	iTE_u8 Tap2_Sign, Tap2_Value;
	iTE_u8 Tap3_Sign, Tap3_Value;
	iTE_u8 DFE_A, DFE_B, DFE_C;

	// 0320 add
	iTE_u8 Tap12_Sign, Tap12_Value;
	iTE_u8 AMP_AA, AMP_BC;
	iTE_u8 AMP_AAA, AMP_BCD;
	iTE_u8 Tap123_Value;
	iTE_u8 TimeOut_Now = 0x00, TargetRS;
	iTE_u8 MinTap123_Value=0xFF;
	iTE_u8 MinTap12_Value=0xFF;
	iTE_u8 MinTap12_RS=0xFF;
	//iTE_u8 MinTap1, MinTap2, MinTap3;
	//iTE_u8 MinTap1_Sign, MinTap2_Sign, MinTap3_Sign;
	//iTE_u8 Reg61h, Reg62h ;
	//iTE_u8 Reg6Bh, Reg6Ch ;
	iTE_u16 Rec_TimeOut;

	if(Flag_AMP_ALL_Timeout == TRUE)
	{
		EQ_DEBUG_PRINTF(("Flag_AMP_ALL_Timeout == TRUE , dont need to get DFE value \n"));
		return;
	}


	if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
	else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);

	EQ_DEBUG_PRINTF(("!!!!!!!!!!!!\n"));
	switch(Channel_Color)
	{
		case Channel_B:
			hdmirxset(0x37 ,0xC0 ,0x00);
			TargetRS = hdmirxrd(0xD5);
			EQ_DEBUG_PRINTF(("!! Report Channel B DFE Value , EQ TargetRS = 0x%x!!\n", (int)TargetRS));
			break;
		case Channel_G:
			hdmirxset(0x37 ,0xC0 ,0x40);
			TargetRS = hdmirxrd(0xD6);
			EQ_DEBUG_PRINTF(("!! Report Channel G DFE Value , EQ TargetRS = 0x%x!!\n", (int)TargetRS));
			break;
		case Channel_R:
			hdmirxset(0x37 ,0xC0 ,0x80);
			TargetRS = hdmirxrd(0xD7);
			EQ_DEBUG_PRINTF(("!! Report Channel R DFE Value , EQ TargetRS = 0x%x!!\n", (int)TargetRS));
			break;
		default:
			break;
	}

	MinTap12_RS = TargetRS;
	Rec_TimeOut = AMP_TimeOut[Channel_Color];
	EQ_DEBUG_PRINTF(("!! Rec_TimeOut = 0x%04x!!\n", (int)Rec_TimeOut));
	EQ_DEBUG_PRINTF(("!!!!!!!!!!!!\n"));

	for (i = 0; i < sizeof(RS_Value); i++)
	{
		hdmirxset(0x36, 0x0F, i);
		AMP_A = hdmirxrd(0x5d);
		AMP_B = hdmirxrd(0x5e);
		AMP_C = hdmirxrd(0x5f);
		AMP_D = hdmirxrd(0x60);

		EQ_DEBUG_PRINTF(("RS =%02x ", (int)RS_Value[i]));
		if (AMP_A > AMP_B)
		{
			Tap1_Sign = 1;
			Tap1_Value = (AMP_A - AMP_B) >> 1;
		}
		else
		{
			Tap1_Sign = 0;
			Tap1_Value = (AMP_B - AMP_A) >> 1;
		}

		if (AMP_A > AMP_C)
		{
			Tap2_Sign = 1;
			Tap2_Value = (AMP_A - AMP_C) >> 1;
		}
		else
		{
			Tap2_Sign = 0;
			Tap2_Value = (AMP_C - AMP_A) >> 1;
		}

		if (AMP_A > AMP_D)
		{
			Tap3_Sign = 1;
			Tap3_Value = (AMP_A - AMP_D) >> 1;
		}
		else
		{
			Tap3_Sign = 0;
			Tap3_Value = (AMP_D - AMP_A) >> 1;
		}

		//EQ_DEBUG_PRINTF(("Tap1_Sign=%d, Tap1=%x,  Tap2_Sign=%d  Tap2=%x, Tap3_Sign=%d, Tap3=%x\n", (int)Tap1_Sign, (int)Tap1_Value, (int)Tap2_Sign, (int)Tap2_Value, (int)Tap3_Sign, (int)Tap3_Value));

		if(Tap1_Value > 0x1F) Tap1_Value = 0x1F;
		if(Tap2_Value > 0x0F) Tap2_Value = 0x0F;
		if(Tap3_Value > 0x07) Tap3_Value = 0x07;

		DFE_A = BIT6 + (Tap1_Sign << 5) + Tap1_Value;
		DFE_B = BIT5 + (Tap2_Sign << 4) + Tap2_Value;
		DFE_C = BIT4 + (Tap3_Sign << 3) + Tap3_Value;

		DFE_VALUE[i][Channel_Color][0] = DFE_A;
		DFE_VALUE[i][Channel_Color][1] = DFE_B;
		DFE_VALUE[i][Channel_Color][2] = DFE_C;

		EQ_DEBUG_PRINTF(("DFE_A = 0x%02x, ",(int) DFE_A));
		EQ_DEBUG_PRINTF(("DFE_B = 0x%02x, ",(int) DFE_B));
		EQ_DEBUG_PRINTF(("DFE_C = 0x%02x  ",(int) DFE_C));

		if(Flag_Need_Adjust_RSValue == FALSE) {EQ_DEBUG_PRINTF(("\n"));}

		if(Flag_Need_Adjust_RSValue == TRUE)
		{
			if(iTE6805_DATA.ChipID == 0xA0 || iTE6805_DATA.ChipID == 0xA1)
			{
				AMP_AA= AMP_A << 1;
				AMP_BC= AMP_B + AMP_C;
				AMP_AAA= AMP_AA + AMP_A;
				AMP_BCD= AMP_B + AMP_C + AMP_D;
				if(AMP_AA > AMP_BC)
				{
					Tap12_Sign=1;
					Tap12_Value= (AMP_AA- AMP_BC)>>1;
				}
				else
				{
					Tap12_Sign=0;
					Tap12_Value=(AMP_BC- AMP_AA)>>1;
				}
				if(AMP_AAA > AMP_BCD)
				{
					Tap123_Value= (AMP_AAA- AMP_BCD)>>1;
				}
				else
				{
					Tap123_Value=(AMP_BCD- AMP_AAA)>>1;
				}

				TimeOut_Now = Rec_TimeOut&0x01;
				Rec_TimeOut = Rec_TimeOut>>1;

				EQ_DEBUG_PRINTF(("Tap12 = 0x%02x ",(int) Tap12_Value));
				EQ_DEBUG_PRINTF(("Tap123 = 0x%02x ",(int) Tap123_Value));
				EQ_DEBUG_PRINTF(("TimeOut = 0x%x \n",(int) TimeOut_Now));

				if(TimeOut_Now==0 && ((Tap12_Value < MinTap12_Value) ||
				  (Tap12_Value == MinTap12_Value && Tap123_Value < MinTap123_Value) ))
				{
					MinTap123_Value = Tap123_Value;
					MinTap12_Value = Tap12_Value;
					MinTap12_RS    = RS_Value[i];
				}
			}
			else
			{
				// add for ChipID = B0
				MinTap12_RS = TargetRS;
				EQ_DEBUG_PRINTF(("\n"));
			}
		}
	}

	// after B0 because do not have RSSKEW EQ, so need to start manual DFE value or Manual EQ DFE setting will fake.
	if(Flag_Need_Adjust_RSValue == TRUE || (iTE6805_DATA.ChipID != 0xA0 && iTE6805_DATA.ChipID != 0xA1 ))
	{
		EQ_DEBUG_PRINTF(("#   Adjust RS to %02x     #\n", (int) MinTap12_RS));
		switch (Channel_Color)
		{
			case Channel_B:
				hdmirxwr(0x27, 0x80 + MinTap12_RS);
				gEQ_B_TargetRS = MinTap12_RS;
				iTE6805_Set_DFE(MinTap12_RS, Channel_B);
				break;
			case Channel_G:
				hdmirxwr(0x28, 0x80 + MinTap12_RS);
				gEQ_G_TargetRS = MinTap12_RS;
				iTE6805_Set_DFE(MinTap12_RS, Channel_G);
				break;
			case Channel_R:
				hdmirxwr(0x29, 0x80 + MinTap12_RS);
				gEQ_R_TargetRS = MinTap12_RS;
				iTE6805_Set_DFE(MinTap12_RS, Channel_R);
				if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
				else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
				hdmirxset(0x4B, BIT7, BIT7);
				break;
		}
	}
	chgbank(0);

}

iTE_u8 iTE6805_Find_indexof_DEF(iTE_u8 EQ_Value)
{
	iTE_u8 i = 0;
	for(i = 0; i < sizeof(RS_Value); i++)
	{
		if(RS_Value[i] == EQ_Value) return i;
	}
}

void iTE6805_Set_DFE(iTE_u8 EQ_Value, iTE_u8 Type_Channel)
{
	iTE_u8 i;

	i = iTE6805_Find_indexof_DEF(EQ_Value);

	if (iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
	else if (iTE6805_DATA.CurrentPort == PORT1) chgbank(7);

	switch(Type_Channel)
	{
		case Channel_B:
			hdmirxwr(0x4B, DFE_VALUE[i][Channel_B][0]);
			hdmirxwr(0x4C, DFE_VALUE[i][Channel_B][1]);
			hdmirxwr(0x4D, DFE_VALUE[i][Channel_B][2]);
			EQ_DEBUG_PRINTF(("\n--------------setting Channel B DFE--------------\n"));
			EQ_DEBUG_PRINTF(("-setting 4B to 0x%x -\n",(int) hdmirxrd(0x4B)));
			EQ_DEBUG_PRINTF(("-setting 4C to 0x%x -\n",(int) hdmirxrd(0x4C)));
			EQ_DEBUG_PRINTF(("-setting 4D to 0x%x -\n",(int) hdmirxrd(0x4D)));
			break;
		case Channel_G:
			hdmirxwr(0x4E, DFE_VALUE[i][Channel_G][0]);

			// iTE6805 A0 if write value to REG 0x34F/0x74F, the Reg 4F will be also over write.
			if(iTE6805_DATA.ChipID != 0xA0) hdmirxwr(0x4F, DFE_VALUE[i][Channel_G][1]); // need ECO

			hdmirxwr(0x50, DFE_VALUE[i][Channel_G][2]);
			EQ_DEBUG_PRINTF(("\n--------------setting Channel G DFE--------------\n"));
			EQ_DEBUG_PRINTF(("-setting 4E to 0x%x -\n",(int) hdmirxrd(0x4E)));
			EQ_DEBUG_PRINTF(("-setting 4F to 0x%x -\n",(int) hdmirxrd(0x4F)));
			EQ_DEBUG_PRINTF(("-setting 50 to 0x%x -\n",(int) hdmirxrd(0x50)));
			EQ_DEBUG_PRINTF(("-------------------------------------------------\n"));
			break;
		case Channel_R:
			hdmirxwr(0x51, DFE_VALUE[i][Channel_R][0]);
			hdmirxwr(0x52, DFE_VALUE[i][Channel_R][1]);
			hdmirxwr(0x53, DFE_VALUE[i][Channel_R][2]);
			EQ_DEBUG_PRINTF(("\n--------------setting Channel R DFE--------------\n"));
			EQ_DEBUG_PRINTF(("-setting 51 to 0x%x -\n",(int) hdmirxrd(0x51)));
			EQ_DEBUG_PRINTF(("-setting 52 to 0x%x -\n",(int) hdmirxrd(0x52)));
			EQ_DEBUG_PRINTF(("-setting 53 to 0x%x -\n",(int) hdmirxrd(0x53)));
			break;
		default :
			break;
	}
	chgbank(0);
}

