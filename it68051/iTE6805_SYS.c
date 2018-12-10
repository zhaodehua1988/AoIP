///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_SYS.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
#include "iTE6805_Global.h"
#include "iTE6805_CSC_Table.h"

extern _iTE6805_DATA	iTE6805_DATA;
extern _iTE6805_VTiming	iTE6805_CurVTiming;

#define TIMEOUT_ECC_ERROR			0x0A
#define TIMEOUT_SCDT_CHECK_COUNT	MS_TimeOut(30)
#define TIMEOUT_AUDIO_CHECK_COUNT	MS_TimeOut(10)
#define TIMEOUT_AUDIO_MONITOR		MS_TimeOut(20)

iTE_u8 Current_ECCAbnormal_Count = 0;
iTE_u8 Current_ECCError_Count = 0;
iTE_u8 Current_SCDTCheck_Count = 0;
iTE_u8 Current_AudioCheck_Count = 0;
iTE_u8 Current_AudioMonitor_Count = 0;

#define Max_SymbolLockRst_Need_Rst_SCDC 20
iTE_u8 Current_Ch0_SymbolLockRst_Count = 0;
iTE_u8 Current_Ch1_SymbolLockRst_Count = 0;
iTE_u8 Current_Ch2_SymbolLockRst_Count = 0;

#define Max_AudioSamplingFreq_ErrorCount	30
iTE_u8 Current_AudioSamplingFreq_ErrorCount;

// need to restart eq
#define MAX_deskewerrcnt 10
iTE_u8 deskewerrcnt		= 0;
iTE_u8 abnormalcnt		= 0;

// pre Frame data container
iTE_u8 prevAVIDB[2] = {0};
iTE_u8 prevAudioB0_Status;		// compare mask 0xF0
iTE_u8 prevAudioB1_Status;		// compare mask 0xFF
iTE_u8 prevAudioB2_CHStatus;	// compare mask BIT1

// Flag
iTE_u8 Flag_NewAVIInfoFrame = FALSE;
iTE_u8 Flag_FirstTimeAudioMonitor = TRUE;
iTE_u8 Flag_HDCP_Trigger_AutoEQ_Again = TRUE;
iTE_u8 Flag_FirstTimeParameterChange = FALSE;
iTE_u8 Flag_Disable_NewAVIInfoFrame_When_Parameter_Change = FALSE;
iTE_u8 Flag_First_Time_VidStable_Done = FALSE;

void iTE6805_hdmirx_port0_SYS_irq(void)
{
	iTE_u8 Reg05h, Reg06h, Reg08h, Reg09h;
	iTE_u8 Reg13h, Reg14h, Reg15h;

	chgbank(0);
	Reg05h = hdmirxrd(0x05);	hdmirxwr(0x05 ,Reg05h);// port0
	Reg06h = hdmirxrd(0x06);	hdmirxwr(0x06 ,Reg06h);// port0
	Reg08h = hdmirxrd(0x08);	hdmirxwr(0x08 ,Reg08h);// port0
	Reg09h = hdmirxrd(0x09);	hdmirxwr(0x09 ,Reg09h);// port0

	Reg13h = hdmirxrd(0x13);
	Reg14h = hdmirxrd(0x14);
	Reg15h = hdmirxrd(0x15);

	if (Reg05h != 0x00)
	{
		if (Reg05h & 0x10)
		{
			if(iTE6805_Check_CLK_Vaild())
			{
				iTE6805_INT_HDMI_DVI_Mode_Chg(PORT0);
			}
		}
		if (Reg05h & 0x20)
		{
			HDMIRX_DEBUG_PRINT(("# Port 0 ECC Error # \n"));
			iTE6805_INT_ECC_ERROR();
		}
		if (Reg05h & 0x40)
		{
			deskewerrcnt += 1;
			HDMIRX_DEBUG_PRINT(("# Port 0 Deskew Error #  deskewerrcnt=%d\n",(int) deskewerrcnt));
#ifdef _ENABLE_AUTO_EQ_
			if(deskewerrcnt > MAX_deskewerrcnt && iTE6805_Check_HDMI2()==FALSE)
			{
				deskewerrcnt = 0;
				HDMIRX_DEBUG_PRINT(("# Deskew Error > %d #  EQ Restart \n",(int) deskewerrcnt ));
				iTE6805_EQ_chg(STATEEQ_ClockONDet_And_TriggerEQ);
				abnormalcnt++;
				if(abnormalcnt==3)
				{
					abnormalcnt = 0;
					iTE6805_Reset_ALL_Logic(PORT0);
				}
			}
#endif
		}

		if (Reg05h & 0x80)
		{
			HDMIRX_DEBUG_PRINT(("# Port 0 H2VSkew Fail #\n"));
		}

		if (Reg05h & 0x04)
		{
			HDMIRX_DEBUG_PRINT(("# Port 0 Input Clock Change Detect #\n"));
			Current_Ch0_SymbolLockRst_Count = 0;
			Current_Ch1_SymbolLockRst_Count = 0;
			Current_Ch2_SymbolLockRst_Count = 0;
			if (hdmirxrd(0x13) & 0x10)
			{
				HDMIRX_DEBUG_PRINT(("# Clock Stable		#\n"));
			}
			else
			{

				HDMIRX_DEBUG_PRINT(("# Clock NOT Stable	#\n"));
				#ifdef _ENABLE_AUTO_EQ_
				iTE6805_EQ_chg(STATEEQ_Off);
				iTE6805_EQ_chg(STATEEQ_ClockONDet_And_TriggerEQ);
				#endif
			}
		}

		if (Reg05h & 0x02)
		{
			Current_ECCError_Count = 0;
			HDMIRX_DEBUG_PRINT(("# Port 0 Rx CKOn Detect #\n"));
		}

		if (Reg05h & 0x01)
		{
			HDMIRX_DEBUG_PRINT(("# Port 0 5V state change INT #\n"));
			iTE6805_INT_5VPWR_Chg(PORT0);
		}
	}

	if (Reg06h != 0x00)
	{
		if (Reg06h & 0x80)
		{
			HDMIRX_DEBUG_PRINT(("# FSM Error  #\n"));
		}

		if ( (Reg06h & 0x40) && iTE6805_Check_PORT0_IS_MHL_Mode(PORT0) == MODE_HDMI)
		{
			HDMIRX_DEBUG_PRINT(("# CH2 Symbol lock Rst #\n"));
			Current_Ch2_SymbolLockRst_Count++;
		}

		if ( (Reg06h & 0x20) && iTE6805_Check_PORT0_IS_MHL_Mode(PORT0) == MODE_HDMI)
		{
			HDMIRX_DEBUG_PRINT(("# CH1 Symbol lock Rst #\n"));
			Current_Ch1_SymbolLockRst_Count++;
		}

		if (Reg06h & 0x10)
		{
			HDMIRX_DEBUG_PRINT(("# CH0 Symbol lock Rst Count=%d #\n",(int) Current_Ch0_SymbolLockRst_Count));
			Current_Ch0_SymbolLockRst_Count++;

			// If 3Ch all symbolLockRst and clk not stable cus EQ Can't start
			// force SCDC Rst and force setting Clock Ration from 1/10 to 1/40
			// try to clock valid , this case is 3.4Gup and not write SCDC to RX
			if((Current_Ch0_SymbolLockRst_Count > Max_SymbolLockRst_Need_Rst_SCDC) &&
			   (Current_Ch1_SymbolLockRst_Count > Max_SymbolLockRst_Need_Rst_SCDC) &&
			   (Current_Ch2_SymbolLockRst_Count > Max_SymbolLockRst_Need_Rst_SCDC) &&
			   (iTE6805_DATA.STATEEQ == STATEEQ_Off) && iTE6805_Check_HDMI2()==FALSE)
			{
				HDMIRX_DEBUG_PRINT(("# Force set SCDC to 1/40#\n"));

				// force set SCDC Clock Ratio 1/40 and Scramble
				chgbank(3);
				hdmirxset(0xE5, 0x1C, 0x1C);
				chgbank(0);
			}
		}

		if (Reg06h & 0x08)
		{
			HDMIRX_DEBUG_PRINT(("# CH2 CDR FIFO Aut0-Rst #\n"));
		}

		if (Reg06h & 0x04)
		{
			HDMIRX_DEBUG_PRINT(("# CH1 CDR FIFO Aut0-Rst #\n"));
		}

		if (Reg06h & 0x02)
		{
			HDMIRX_DEBUG_PRINT(("# CH0 CDR FIFO Aut0-Rst #\n"));
		}

		if (Reg06h & 0x01)
		{
			HDMIRX_DEBUG_PRINT(("# Symbol Lock State Change # "));
			HDMIRX_DEBUG_PRINT((" 0x14 =  %x \n", (int) hdmirxrd(0x14)&0x38));
			#ifdef _ENABLE_AUTO_EQ_
			chgbank(0);
			if(hdmirxrd(0x14)&0x38 && iTE6805_DATA.STATEEQ == STATEEQ_Off)
			{
				iTE6805_EQ_chg(STATEEQ_Off);
				iTE6805_EQ_chg(STATEEQ_ClockONDet_And_TriggerEQ);
			}
			#endif

			if (Reg13h & 0x80)
			{
				HDMIRX_DEBUG_PRINT(("# Symbol Lock #\n"));
			}
			else
			{
				HDMIRX_DEBUG_PRINT(("# Symbol NOT Lock #\n"));
				if (Reg14h & 0x20) HDMIRX_DEBUG_PRINT(("# CH2 Symbol NOT Lock #\n"));
				if (Reg14h & 0x10) HDMIRX_DEBUG_PRINT(("# CH1 Symbol NOT Lock #\n"));
				if (Reg14h & 0x08) HDMIRX_DEBUG_PRINT(("# CH0 Symbol NOT Lock #\n"));
			}
		}
	}

	if (Reg08h != 0x00)
	{
		if (Reg08h & 0x01)
		{

			HDMIRX_DEBUG_PRINT(("# SCDC CLK Ratio Change #\n"));
			iTE6805_vid_chg(STATEV_WaitSync);
			#ifdef _ENABLE_AUTO_EQ_
			iTE6805_EQ_chg(STATEEQ_Off);
			iTE6805_EQ_chg(STATEEQ_ClockONDet_And_TriggerEQ);
			#endif
			if (Reg14h & 0x40)
			{
				HDMIRX_DEBUG_PRINT(("SCDC CLK Ratio 1/40 \n"));
			}
			else
			{
				HDMIRX_DEBUG_PRINT(("SCDC CLK Ratio 1/10 \n"));
			}
		}
		if (Reg08h & 0x02)
		{
			HDMIRX_DEBUG_PRINT(("# SCDC Scrambe Enable Change #\n"));
			if (Reg14h & 0x80)
			{
				HDMIRX_DEBUG_PRINT(("SCDC SCR Enable Bit= 1 \n"));
			}
			else
			{
				HDMIRX_DEBUG_PRINT(("SCDC SCR Enable Bit= 0\n"));
			}
		}
		if (Reg08h & 0x04)
		{
			HDMIRX_DEBUG_PRINT(("# SCDC Scramble Status Change #\n"));
			if ((Reg15h & 0x02) >> 1)
			{
				HDMIRX_DEBUG_PRINT(("#  SCDC Scarmable Status:  ON  #\n"));
			}else
			{
				HDMIRX_DEBUG_PRINT(("#  SCDC Scarmable Status: OFF  #\n"));
			}
		}
		if (Reg08h & 0x08)
		{
			HDMIRX_DEBUG_PRINT(("# EDID Bus Hange #\n"));
		}
		if (Reg08h & 0x80)  // HDMI2 Auto Det
		{
			HDMIRX_DEBUG_PRINT(("HDMI2 Det State=%x \n", (hdmirxrd(0x15) & 0x3C) >> 2));
			hdmirxset(0x4C, 0x80, 0x80);
		}
	}

	if (Reg09h != 0x00)
	{
		if (Reg09h & 0x01)
		{
			Current_ECCError_Count = 0;
			HDMIRX_DEBUG_PRINT(("# Port 0 HDCP Authentication Start #\n"));
			//iTE6805_sys_chg(STATES_AuthStr);
		}
		if (Reg09h & 0x02)
		{
			HDMIRX_DEBUG_PRINT(("# Port 0 HDCP Authentication Done #\n"));
			//iTE6805_sys_chg(STATES_AuthDn);
		}
		if (Reg09h & 0x08)
		{
			Current_ECCError_Count = 0;
			HDMIRX_DEBUG_PRINT(("# Port 0 HDCP Encryption Off #\n"));
		}
	}

}

void iTE6805_hdmirx_port1_SYS_irq(void)
{
	iTE_u8 Reg0Ah, Reg0Bh, Reg0Dh, Reg0Eh;
	iTE_u8 Reg16h, Reg17h, Reg18h;

	//if (iTE6805_DATA.STATEV == STATEV_Unplug || iTE6805_DATA.STATEV == STATEV_VideoOff ||
	//	iTE6805_DATA.CurrentPort != PORT1)  return;

	chgbank(0);
	Reg0Ah = hdmirxrd(0x0A);	hdmirxwr(0x0A, Reg0Ah); // port1
	Reg0Bh = hdmirxrd(0x0B);	hdmirxwr(0x0B, Reg0Bh); // port1
	Reg0Dh = hdmirxrd(0x0D);	hdmirxwr(0x0D, Reg0Dh); // port1
	Reg0Eh = hdmirxrd(0x0E);	hdmirxwr(0x0E, Reg0Eh); // port1

	Reg16h = hdmirxrd(0x16);
	Reg17h = hdmirxrd(0x17);
	Reg18h = hdmirxrd(0x18);

	if (Reg0Ah != 0x00)
	{
		if (Reg0Ah & 0x10)
		{
			if(iTE6805_Check_CLK_Vaild())
			{
				iTE6805_INT_HDMI_DVI_Mode_Chg(PORT1);
			}
		}
		if (Reg0Ah & 0x20)
		{
			HDMIRX_DEBUG_PRINT(("# Port 1 ECC Error # \n"));
			iTE6805_INT_ECC_ERROR();
		}
		if (Reg0Ah & 0x40)
		{
			deskewerrcnt += 1;
			HDMIRX_DEBUG_PRINT(("# Port 1 Deskew Error #  deskewerrcnt=%d\n",(int) deskewerrcnt));
#ifdef _ENABLE_AUTO_EQ_
			if(deskewerrcnt > MAX_deskewerrcnt && iTE6805_Check_HDMI2()==FALSE)
			{
				deskewerrcnt = 0;
				HDMIRX_DEBUG_PRINT(("# Deskew Error > %d #  EQ Restart \n",(int) deskewerrcnt ));
				iTE6805_EQ_chg(STATEEQ_ClockONDet_And_TriggerEQ);
				abnormalcnt++;
				if(abnormalcnt==3)
				{
					abnormalcnt = 0;
					iTE6805_Reset_ALL_Logic(PORT1);
				}
			}
#endif
		}

		if (Reg0Ah & 0x80)
		{
			HDMIRX_DEBUG_PRINT(("# Port 1 H2VSkew Fail #\n"));
		}

		if (Reg0Ah & 0x04)
		{
			HDMIRX_DEBUG_PRINT(("# Port 1 Input Clock Change Detect #\n"));
			Current_Ch0_SymbolLockRst_Count = 0;
			Current_Ch1_SymbolLockRst_Count = 0;
			Current_Ch2_SymbolLockRst_Count = 0;
			if (hdmirxrd(0x16) & 0x10)
			{
				HDMIRX_DEBUG_PRINT(("# Clock Stable		#\n"));
			}
			else
			{
				HDMIRX_DEBUG_PRINT(("# Clock NOT Stable	#\n"));
				#ifdef _ENABLE_AUTO_EQ_
				iTE6805_EQ_chg(STATEEQ_Off);
				iTE6805_EQ_chg(STATEEQ_ClockONDet_And_TriggerEQ);
				#endif
			}
		}

		if (Reg0Ah & 0x02)
		{
			Current_ECCError_Count = 0;
			HDMIRX_DEBUG_PRINT(("# Port 1 Rx CKOn Detect #\n"));
		}

		if (Reg0Ah & 0x01)
		{
			iTE6805_INT_5VPWR_Chg(PORT1);
		}
	}


	if (Reg0Bh != 0x00)
	{
		if (Reg0Bh & 0x80)
		{
			HDMIRX_DEBUG_PRINT(("# FSM Error  #\n"));
		}

		if ( (Reg0Bh & 0x40) && iTE6805_Check_PORT0_IS_MHL_Mode(PORT1) == MODE_HDMI)
		{
			HDMIRX_DEBUG_PRINT(("# CH2 Symbol lock Rst #\n"));
			Current_Ch2_SymbolLockRst_Count++;
		}

		if ( (Reg0Bh & 0x20) && iTE6805_Check_PORT0_IS_MHL_Mode(PORT1) == MODE_HDMI)
		{
			HDMIRX_DEBUG_PRINT(("# CH1 Symbol lock Rst #\n"));
			Current_Ch1_SymbolLockRst_Count++;
		}

		if (Reg0Bh & 0x10)
		{
			HDMIRX_DEBUG_PRINT(("# CH0 Symbol lock Rst #\n"));
			Current_Ch0_SymbolLockRst_Count++;
			if((Current_Ch0_SymbolLockRst_Count > Max_SymbolLockRst_Need_Rst_SCDC) &&
			   (Current_Ch1_SymbolLockRst_Count > Max_SymbolLockRst_Need_Rst_SCDC) &&
			   (Current_Ch2_SymbolLockRst_Count > Max_SymbolLockRst_Need_Rst_SCDC) &&
			   (iTE6805_DATA.STATEEQ == STATEEQ_Off) && iTE6805_Check_HDMI2()==FALSE)
			{
				HDMIRX_DEBUG_PRINT(("# Force set SCDC to 1/40#\n"));

				// force set SCDC Clock Ratio 1/40 and Scramble
				chgbank(7);
				hdmirxset(0xE5, 0x1C, 0x1C);
				chgbank(0);
			}

		}

		if (Reg0Bh & 0x08)
		{
			HDMIRX_DEBUG_PRINT(("# CH2 CDR FIFO Aut0-Rst #\n"));
		}

		if (Reg0Bh & 0x04)
		{
			HDMIRX_DEBUG_PRINT(("# CH1 CDR FIFO Aut0-Rst #\n"));
		}

		if (Reg0Bh & 0x02)
		{
			HDMIRX_DEBUG_PRINT(("# CH0 CDR FIFO Aut0-Rst #\n"));
		}

		if (Reg0Bh & 0x01)
		{
			HDMIRX_DEBUG_PRINT(("# Symbol Lock State Change #\n"));

			#ifdef _ENABLE_AUTO_EQ_
			chgbank(0);
			HDMIRX_DEBUG_PRINT((" 0x17 =  %x \n", (int) hdmirxrd(0x17)&0x38));
			HDMIRX_DEBUG_PRINT((" STATEEQ = %d \n" , (int) iTE6805_DATA.STATEEQ));
			if(hdmirxrd(0x17)&0x38 && iTE6805_DATA.STATEEQ == STATEEQ_Off)
			{
				iTE6805_EQ_chg(STATEEQ_Off);
				iTE6805_EQ_chg(STATEEQ_ClockONDet_And_TriggerEQ); // Check inside
			}
			#endif

			if (Reg16h & 0x80)
			{
				HDMIRX_DEBUG_PRINT(("# Symbol Lock #\n"));
			}
			else
			{
				HDMIRX_DEBUG_PRINT(("# Symbol NOT Lock #\n"));
				if (Reg17h & 0x20) HDMIRX_DEBUG_PRINT(("# CH2 Symbol NOT Lock #\n"));
				if (Reg17h & 0x10) HDMIRX_DEBUG_PRINT(("# CH1 Symbol NOT Lock #\n"));
				if (Reg17h & 0x08) HDMIRX_DEBUG_PRINT(("# CH0 Symbol NOT Lock #\n"));
			}
		}
	}

	if (Reg0Dh != 0x00)
	{
		if (Reg0Dh & 0x01)
		{
			HDMIRX_DEBUG_PRINT(("# SCDC CLK Ratio Change #\n"));
			iTE6805_vid_chg(STATEV_WaitSync);
			#ifdef _ENABLE_AUTO_EQ_
			iTE6805_EQ_chg(STATEEQ_Off);
			iTE6805_EQ_chg(STATEEQ_ClockONDet_And_TriggerEQ);
			#endif
			if (Reg17h & 0x40)
			{
				HDMIRX_DEBUG_PRINT(("SCDC CLK Ratio 1/40 \n"));
			}
			else
			{
				HDMIRX_DEBUG_PRINT(("SCDC CLK Ratio 1/10 \n"));
			}
		}
		if (Reg0Dh & 0x02)
		{
			HDMIRX_DEBUG_PRINT(("# SCDC Scrambe Enable Change #\n"));
			if (Reg17h & 0x80)
			{
				HDMIRX_DEBUG_PRINT(("SCDC SCR Enable Bit= 1 \n"));
			}
			else
			{
				HDMIRX_DEBUG_PRINT(("SCDC SCR Enable Bit= 0\n"));
			}
		}
		if (Reg0Dh & 0x04)
		{
			HDMIRX_DEBUG_PRINT(("# SCDC Scramble Status Change #\n"));
			if ((Reg18h & 0x02) >> 1)
			{
				HDMIRX_DEBUG_PRINT(("#  SCDC Scarmable Status:  ON  #\n"));
			}else
			{
				HDMIRX_DEBUG_PRINT(("#  SCDC Scarmable Status: OFF  #\n"));
			}
		}
		if (Reg0Dh & 0x08)
		{
			HDMIRX_DEBUG_PRINT(("# EDID Bus Hange #\n"));
		}
		if (Reg0Dh & 0x80)  // HDMI2 Auto Det
		{
			HDMIRX_DEBUG_PRINT(("HDMI2 Det State=%x \n", (hdmirxrd(0x15) & 0x3C) >> 2));
			hdmirxset(0x4C, 0x80, 0x80);
		}
	}


	if (Reg0Eh != 0x00)
	{
		if (Reg0Eh & 0x01)
		{
			Current_ECCError_Count = 0;
			HDMIRX_DEBUG_PRINT(("# Port 1 HDCP Authentication Start #\n"));
		}
		if (Reg0Eh & 0x02)
		{
			HDMIRX_DEBUG_PRINT(("# Port 1 HDCP Authentication Done #\n"));
		}
		if (Reg0Eh & 0x08)
		{
			Current_ECCError_Count = 0;
			HDMIRX_DEBUG_PRINT(("# Port 1 HDCP Encryption Off #\n"));
		}
	}


}

void iTE6805_hdmirx_common_irq(void)
{
	iTE_u8 Reg10h, Reg11h, Reg12h;
	iTE_u8 Reg1Ah, Reg1Bh, Reg1Dh, RegD4h, RegD5h;

	iTE_u8 HDMI_VSIF_OUI;
	iTE_u8 HDMI_VSIF_3DValid;

	iTE_u16 HActive, Flag_Trigger_DownScale_Reset = FALSE;
	// move to main
	//if (iTE6805_DATA.STATEV == STATEV_Unplug || iTE6805_DATA.STATEV == STATEV_VideoOff )  return;

	Reg10h = hdmirxrd(0x10);	hdmirxwr(0x10 ,Reg10h);// common
	Reg11h = hdmirxrd(0x11);	hdmirxwr(0x11 ,Reg11h);// common
	Reg12h = hdmirxrd(0x12);	hdmirxwr(0x12 ,Reg12h&0x7F);// common
	//Reg19h = hdmirxrd(0x19);	hdmirxwr(0x19 ,Reg19h);// common //useless code
	Reg1Dh = hdmirxrd(0x1D);	hdmirxwr(0x1D ,Reg1Dh);// common
	RegD4h = hdmirxrd(0xD4);	hdmirxwr(0xD4 ,RegD4h);// common
	RegD5h = hdmirxrd(0xD5);	hdmirxwr(0xD5 ,RegD5h);// common

	if (Reg10h != 0x00)
	{
		if (Reg10h & 0x80)
		{
			if(iTE6805_DATA.STATEV == STATEV_VidStable)
			{
				HDMIRX_DEBUG_PRINT(("# Audio FIFO Error #\n"));
				iTE6805_Reset_Audio_Logic();
				iTE6805_Enable_Audio_Output();
			}
		}

		if (Reg10h & 0x40)
		{
			HDMIRX_DEBUG_PRINT(("# Audio Auto Mute #\n"));
		}

		if ((Reg10h & 0x20) && iTE6805_DATA.Flag_VidStable_Done)
		{
			HDMIRX_DEBUG_PRINT(("# PKT Left Mute #\n"));
			iTE6805_Set_AVMute(AVMUTE_OFF);
		}

		if ((Reg10h & 0x10) && iTE6805_DATA.Flag_VidStable_Done)
		{
			HDMIRX_DEBUG_PRINT(("# Set Mute PKT Received #\n"));
			iTE6805_Set_AVMute(AVMUTE_ON);
		}

		if (Reg10h & 0x08)
		{
			HDMIRX_DEBUG_PRINT(("# Timer Counter Tntterrupt #\n"));
		}

		if (Reg10h & 0x04)
		{
			HDMIRX_DEBUG_PRINT(("# Video Mode Changed #\n"));
		}

		if (Reg10h & 0x02)
		{
			HDMIRX_DEBUG_PRINT(("# SCDT Change #\n"));
			iTE6805_INT_SCDT_Chg();
		}

		if (Reg10h & 0x01)
		{
			HDMIRX_DEBUG_PRINT(("# Video Abnormal Interrup #\n"));
		}

	}

	if (Reg11h != 0x00)
	{
		if (Reg11h & BIT7)
		{
			//HDMIRX_DEBUG_PRINT(("# No General PKT 2 Received #\n"));
		}

		if (Reg11h & BIT6)
		{
			//HDMIRX_DEBUG_PRINT(("# No General PKT 1 Received #\n"));
		}

		if (Reg11h & BIT5)
		{
			HDMIRX_DEBUG_PRINT(("# No Audio InfoFrame Received #\n"));
		}

		if (Reg11h & BIT4)
		{
			HDMIRX_DEBUG_PRINT(("# No AVI InfoFrame Received #\n"));
		}

		if (Reg11h & BIT3)
		{
			HDMIRX_DEBUG_PRINT(("# CD Detect #\n"));
			//iTE6805_Set_ColorDepth();
		}

		if (Reg11h & BIT2)
		{
			HDMIRX_DEBUG_PRINT(("# Vender Specific InfoFrame Detect #\n"));
			chgbank(2);
			HDMI_VSIF_OUI = (hdmirxrd(0x2A) << 16) + (hdmirxrd(0x29) << 8) + (hdmirxrd(0x28));
			HDMI_VSIF_3DValid = (hdmirxrd(0x2C) & 0x01);
			if (HDMI_VSIF_OUI == 0xC45DD8) HDMIRX_DEBUG_PRINT(("Valid HDMI_VSIF ! \n"));
			else if (HDMI_VSIF_OUI == 0x0C03) HDMIRX_DEBUG_PRINT(("Valid HDMI1.4 VSIF !\n"));
			else HDMIRX_DEBUG_PRINT(("NOT HDMI_VSIF !  XXXXXXX \n"));
			if (HDMI_VSIF_OUI == 0xC45DD8 && HDMI_VSIF_3DValid == 1)
				HDMIRX_DEBUG_PRINT(("HDMI_VSIF with 3D_Valid ! \n"));
			else HDMIRX_DEBUG_PRINT(("NOT HDMI_VSIF with 3D  Valid ! XXXXX  \n"));
			chgbank(0);
		}

		if (Reg11h & BIT1)
		{
			hdmirxwr(0x11, BIT1);
			//HDMIRX_DEBUG_PRINT(("# ISRC2 Detect #\n"));
		}

		if (Reg11h & BIT0)
		{
			hdmirxwr(0x11, BIT0);
			//HDMIRX_DEBUG_PRINT(("# ISRC1 Detect #\n"));
		}
	}

	if (Reg12h != 0x00)
	{
		if (Reg12h & 0x80)
		{
			chgbank(0);
			Reg1Ah = hdmirxrd(0x1A);
			Reg1Bh = hdmirxrd(0x1B) & 0x07;
			HDMIRX_DEBUG_PRINT(("# Video Parameters Change #\n"));
			HDMIRX_DEBUG_PRINT(("# VidParaChange_Sts=Reg1Bh=0x%02X Reg1Ah=0x%02X\n",(int) Reg1Bh,(int) Reg1Ah));

			hdmirxwr(0x12 ,0x80);// only parameter change need to clear INT here , or register 1A/1B can't be read after clear.

			// only downscale need this section
			// because parameter change need reset downscale setting
			#if defined(DEMO) || iTE68051_4K60_Mode==MODE_DownScale || iTE68051_4K60_Mode==MODE_EvenOdd_Plus_DownScale
			HActive = ((hdmirxrd(0x9E) & 0x3F) << 8) + hdmirxrd(0x9D);
			Reg1Ah = Reg1Ah & (BIT1|BIT6);

			Flag_Disable_NewAVIInfoFrame_When_Parameter_Change = FALSE;

			if( iTE6805_DATA.Flag_VidStable_Done == TRUE )
			{
				if(Flag_First_Time_VidStable_Done==TRUE)
				{
					Flag_First_Time_VidStable_Done=FALSE;
				}else{
					Flag_Trigger_DownScale_Reset = FALSE;
					if(Flag_FirstTimeParameterChange == TRUE)
					{
						Flag_FirstTimeParameterChange = FALSE;
						if(iTE6805_CurVTiming.HActive!=0 && (iTE6805_CurVTiming.HActive!=HActive))
						{
							HDMIRX_DEBUG_PRINT(("HActive = %d, before HActive = %d \n", HActive, iTE6805_CurVTiming.HActive));
							Flag_Trigger_DownScale_Reset = TRUE;
						}
					}else{
						if(iTE6805_Check_4K_Resolution())
						{
							Flag_Trigger_DownScale_Reset = TRUE;
						}
					}

					if(Flag_Trigger_DownScale_Reset==TRUE)
					{
						HDMIRX_DEBUG_PRINT((" Set_DNScale in Parameter Change \n"));

						iTE6805_Get_VID_Info(); // this need time to calculate, put this here for save time and get item for setting downscale
						iTE6805_Get_AVIInfoFrame_Info(); // add here for setting DownScale variable
						#if Debug_message
						iTE6805_Show_VID_Info();
						#endif

						iTE68051_Video_Output_Setting();
						iTE6805_Reset_Video_Logic();

						Flag_Trigger_DownScale_Reset = FALSE;
						Flag_FirstTimeParameterChange = TRUE;
						Flag_Disable_NewAVIInfoFrame_When_Parameter_Change = TRUE; // for parameter change mean AVI infoframe already setting here
					}

				}
			}
			#endif
		}
		if (Reg12h & 0x40)
		{
			HDMIRX_DEBUG_PRINT(("# 3D audio Valie Change #\n"));
		}

		if( Reg12h&0x20 )
		{
			HDMIRX_DEBUG_PRINT(("# New DMR PKT Received #\n"));
		}

		if (Reg12h & 0x10)
		{
			HDMIRX_DEBUG_PRINT(("# New Audio PKT Received #\n"));
		}

		if (Reg12h & 0x08)
		{
			HDMIRX_DEBUG_PRINT(("# New ACP PKT Received #\n"));
		}

		if (Reg12h & 0x04)
		{
			HDMIRX_DEBUG_PRINT(("# New SPD PKT Received #\n"));
		}

		if (Reg12h & 0x02)
		{
			HDMIRX_DEBUG_PRINT(("# New MPEG InfoFrame Received #\n"));
		}

		if (Reg12h & 0x01)
		{
			iTE_u8 AVIDB[2] ;

            chgbank(2) ;
            AVIDB[0] = hdmirxrd(REG_RX_AVI_DB0) ;
            AVIDB[1] = hdmirxrd(REG_RX_AVI_DB1) ;
            chgbank(0) ;

			// some device may send different resolution  but video still stable, but it will send new AVI InfoFrame to tell you
			// so need judge New AVI InfoFrame
			if(iTE6805_DATA.Flag_VidStable_Done != TRUE)
			{
				HDMIRX_DEBUG_PRINT(("Flag_VidStable_Done!=TRUE, Save AVI Infoframe to variable. \n" ));
				HDMIRX_DEBUG_PRINT(("AVIDB[0] = %x \n", AVIDB[0] ));
				HDMIRX_DEBUG_PRINT(("AVIDB[0] = %x \n", AVIDB[1] ));
				HDMIRX_DEBUG_PRINT(("prevAVIDB[0] = %x \n", prevAVIDB[0] ));
				HDMIRX_DEBUG_PRINT(("prevAVIDB[0] = %x \n", prevAVIDB[1] ));
				prevAVIDB[0] = AVIDB[0] ;
				prevAVIDB[1] = AVIDB[1] ;
				Flag_Disable_NewAVIInfoFrame_When_Parameter_Change = FALSE;
			}else{
				if( (AVIDB[0] != prevAVIDB[0]) ||
					(AVIDB[1] != prevAVIDB[1]))
				{
					if(Flag_Disable_NewAVIInfoFrame_When_Parameter_Change == TRUE)
					{
						// diable Flag_NewAVIInfoFrame because video parameter change already setting on top, do not trigger this Flag for STATEV_VidStable in vid_fsm
						Flag_Disable_NewAVIInfoFrame_When_Parameter_Change = FALSE;
						Flag_NewAVIInfoFrame = FALSE;
					}else{
						HDMIRX_DEBUG_PRINT(("AVIDB[0] = %x \n", AVIDB[0] ));
						HDMIRX_DEBUG_PRINT(("AVIDB[0] = %x \n", AVIDB[1] ));
						HDMIRX_DEBUG_PRINT(("prevAVIDB[0] = %x \n", prevAVIDB[0] ));
						HDMIRX_DEBUG_PRINT(("prevAVIDB[0] = %x \n", prevAVIDB[1] ));

						prevAVIDB[0] = AVIDB[0] ;
						prevAVIDB[1] = AVIDB[1] ;
						HDMIRX_DEBUG_PRINT(("# New AVI InfoFrame Received #\n"));
						Flag_NewAVIInfoFrame = TRUE;
					}

				}
			}

		}

	}

	if (Reg1Dh != 0x00)
	{
		if (Reg1Dh & 0x01)
		{
			HDMIRX_DEBUG_PRINT(("HDCP2 Authen Start \n"));
		}
		if (Reg1Dh & 0x02)
		{
			HDMIRX_DEBUG_PRINT(("HDCP2 Authen Done \n"));
		}
		if (Reg1Dh & 0x04)
		{
			HDMIRX_DEBUG_PRINT(("HDCP2 Off Detect \n"));
		}
		if (Reg1Dh & 0x08)
		{
			HDMIRX_DEBUG_PRINT(("HDCP Encryption Change \n"));
#ifdef _ENABLE_AUTO_EQ_
			if ( (iTE6805_Check_HDMI2()==FALSE) && Flag_HDCP_Trigger_AutoEQ_Again == TRUE)
			{
				Flag_HDCP_Trigger_AutoEQ_Again = FALSE;
				// when hdmi1.x 30m/10cm (10cm with EQ 0x01 will check ok but EQ still fail, so need auto EQ again when EQ1.x and only trigger once)
				if(iTE6805_BitErr_Check_Again() == FALSE)
				{
					EQ_DEBUG_PRINTF(("******EQ Done But Find CED Error when Encryption change, Restart Manual EQ ******\n"));
					iTE6805_EQ_chg(STATEEQ_ClockONDet_And_TriggerEQ);
				}
			}
#endif
			if(hdmirxrd(0xD6) & 0x08){HDMIRX_DEBUG_PRINT(("HDCP Encryption ON! \n"));}
			else HDMIRX_DEBUG_PRINT(("HDCP Encryption OFF !\n"));
		}
		if (Reg1Dh & 0x10)
		{
			if (RegD4h & 0x01)
			{
				HDMIRX_DEBUG_PRINT((" AKE_Init_Rcv \n"));
			}
			if (RegD4h & 0x02)
			{
				HDMIRX_DEBUG_PRINT((" AKE_NoStr_Km_Rcv \n"));
			}
			if (RegD4h & 0x04)
			{
				HDMIRX_DEBUG_PRINT((" AKE_Str_Km_Rcv \n"));
			}
			if (RegD4h & 0x08)
			{
				HDMIRX_DEBUG_PRINT((" LC_Init_Rcv \n"));
			}
			if (RegD4h & 0x10)
			{
				HDMIRX_DEBUG_PRINT((" SKE_Send_Eks_Rcv \n"));
			}
			if (RegD4h & 0x20)
			{
				HDMIRX_DEBUG_PRINT((" Rpt_Send_Ack_Rcv \n"));
			}
			if (RegD4h & 0x40)
			{
				HDMIRX_DEBUG_PRINT((" Rpt_Str_Manage_Rcv \n"));
			}
			if (RegD4h & 0x80)
			{
				HDMIRX_DEBUG_PRINT((" RSA_Fail_pulse \n"));
			}
			if (RegD5h & 0x01)
			{
				HDMIRX_DEBUG_PRINT((" HDCP2 Read CERT Done \n"));
			}
			if (RegD5h & 0x02)
			{
				HDMIRX_DEBUG_PRINT((" HDCP2 Read H Done \n"));
			}
			if (RegD5h & 0x04)
			{
				HDMIRX_DEBUG_PRINT((" HDCP2 Read Pair Done \n"));
			}

			if (RegD5h & 0x08)
			{
				HDMIRX_DEBUG_PRINT((" HDCP2 Read L' Done \n"));
			}
			if (RegD5h & 0x10)
			{
				HDMIRX_DEBUG_PRINT((" HDCP2 Read KSV and V' \n"));
			}
			if( RegD5h & 0x20)
			{
				HDMIRX_DEBUG_PRINT((" Rpt_Send_Ack_Rcv \n"));
			}
			if (RegD5h & 0x40)
			{
				HDMIRX_DEBUG_PRINT((" HDCP2 Message Read Error !! \n"));
			}
			if (RegD5h & 0x80)
			{
				HDMIRX_DEBUG_PRINT((" ECC Re-Authen !! \n"));
			}
		}
	}
}
// ***************************************************************************
// FSM
// ***************************************************************************
void iTE6805_vid_fsm(void)
{
	//int vidstable;

	switch (iTE6805_DATA.STATEV)
	{
		case STATEV_VideoOff:
			break;
		case STATEV_Unplug:
			if(iTE6805_Check_5V_State(iTE6805_DATA.CurrentPort)==MODE_5V_ON)
			{
				iTE6805_INT_5VPWR_Chg(iTE6805_DATA.CurrentPort);
			}
			break;
		case STATEV_WaitSync:
			if(iTE6805_Check_SCDT())
			{
				iTE6805_vid_chg(STATEV_CheckSync);
			}
			break;
		case STATEV_CheckSync:
			if(Current_SCDTCheck_Count == 0)
			{
				iTE6805_vid_chg(STATEV_VidStable);
			}
			else
			{
				Current_SCDTCheck_Count--;
			}
			break;
		case STATEV_VidStable:

			// New AVI Info Frame
			if( Flag_NewAVIInfoFrame == TRUE )
			{
				HDMIRX_DEBUG_PRINT(("!!! New AVI InfoFrmae in STATEV_VidStable !!!\n"));
				iTE6805_Get_AVIInfoFrame_Info(); // add here for setting DownScale variable
				iTE6805_Get_VID_Info(); // need this for downscale setting parameter
				iTE6805_Show_VID_Info();
				iTE6805_Set_1B0_By_PixelClock(); // 0713 Andrew suggest if PCLK < 25 need to set 1B0[0] to 0, else 1

				#ifdef iTE68051
				iTE68051_Video_Output_Setting(); // DaulPixel setting/ DownScaling setting/ Demo Setting/ PIN Config Setting only for 6805A0
				#endif

				#ifdef iTE68052
				iTE68052_Video_Output_Setting();
				#endif

				iTE6805_Enable_Video_Output();
				iTE6805_Reset_Video_Logic(); // can't put reset video logic here or vidstable will be keep reset reset reset reset .... 0330
				Flag_NewAVIInfoFrame = FALSE;
				Flag_FirstTimeParameterChange = TRUE;
				#ifdef iTE68051
				// If new avi info frame arrive , need init LVDS again, 15m line might had new AVI infoframe condition
				//iTE6805_Init_6028LVDS(0); // init first 6028 chip
				//iTE6805_Init_6028LVDS(1); // init first 6028 chip
				#endif
			}

			// If setting AV by last stable video, and do not receive av mute clear pkt
			// because HW auto reset and clock and vid stable again
			// VDGatting will be set by last AV Mute, should check and clear again
			chgbank(0);
			if(hdmirxrd(0x4F)&BIT5 && iTE6805_Check_AVMute()==AVMUTE_OFF)
			{
				#ifdef _ENABLE_IT6805_MHL_FUNCTION_
				if(iTE6805_DATA.MHL_RAP_Content_State == RAP_CONTENT_ON) // MHL RAP MSG Content setting
				{
				#endif

					iTE6805_Set_AVMute(AVMUTE_OFF);

				#ifdef _ENABLE_IT6805_MHL_FUNCTION_
				}
				#endif
			}
			break;

		default:
			HDMIRX_DEBUG_PRINT(("Video State Error !!!\n"));
			break;
	}
}

void iTE6805_aud_fsm( void )
{
	iTE_u8 AudioB0_Status, AudioB1_Status, AudioB2_CHStatus;
	iTE_u8 audstable = 0, eccerr = 0;
	switch( iTE6805_DATA.STATEA ) {
		case STATEA_AudioOff :
			break;
		case STATEA_RequestAudio :
			Flag_FirstTimeAudioMonitor = TRUE;
			AudioB0_Status = 0x00;
			AudioB1_Status = 0x00;
			AudioB2_CHStatus = 0x00;
			prevAudioB0_Status = 0x00;		// compare mask 0xF0
			prevAudioB1_Status = 0x00;		// compare mask 0xFF
			prevAudioB2_CHStatus = 0x00;	// compare mask BIT1
			iTE6805_aud_chg(STATEA_WaitForReady);
			break;
		case STATEA_WaitForReady :
			if(Current_AudioCheck_Count == 0)
			{

				if(iTE6805_DATA.CurrentPort == PORT0) chgbank(0);
				else chgbank(4);
				// REG0B9 : P0 Received error
				// REG4B9 : P1 Received error
				eccerr = hdmirxrd(0xB9)&0xC0;
				chgbank(0);

				if(eccerr) iTE6805_Reset_Audio_Logic();

				if(iTE6805_Check_SCDT() && iTE6805_Check_AUDT())
				{
					iTE6805_aud_chg(STATEA_AudioOn);
					break;
				}
				Current_AudioCheck_Count = TIMEOUT_AUDIO_CHECK_COUNT;
			}
			else
			{
				Current_AudioCheck_Count--;
			}
			break;
		case STATEA_AudioOn :
			if(Current_AudioMonitor_Count-- == 0)
			{
				Current_AudioMonitor_Count = TIMEOUT_AUDIO_MONITOR;
				chgbank(0);

				if(Flag_FirstTimeAudioMonitor == TRUE)
				{
					prevAudioB0_Status = hdmirxrd(0xB0)&0xF0;	// compare mask 0xF0
					prevAudioB1_Status = hdmirxrd(0xB1);		// compare mask 0xFF
					prevAudioB2_CHStatus = hdmirxrd(0xB2)&BIT1;	// compare mask BIT1
					Flag_FirstTimeAudioMonitor = FALSE;
					if(prevAudioB2_CHStatus == 0x00)
					{
						// If it is not LPCM i.e. compress audio , disable auto mute.
						hdmirxset(0x8C, BIT3, 0);
					}else{
						// LPCM , enablue HW auto mute.
						hdmirxset(0x8C, BIT3, BIT3);
					}
				}
				else
				{
					AudioB0_Status = hdmirxrd(0xB0)&0xF0;
					AudioB1_Status = hdmirxrd(0xB1);
					AudioB2_CHStatus = hdmirxrd(0xB2)&BIT1;

					if(prevAudioB0_Status != AudioB0_Status ||
					   prevAudioB1_Status != AudioB1_Status ||
					   prevAudioB2_CHStatus != AudioB2_CHStatus)
					{
						iTE6805_aud_chg(STATEA_RequestAudio);
					}
				}
			}
			break;

	default :
		 HDMIRX_DEBUG_PRINT(("Audio State Error !!!\n"));
		 break;
	}
}

// ***************************************************************************
// Video State function
// ***************************************************************************

void iTE6805_vid_chg(STATEV_Type NewState)
{
	iTE6805_DATA.STATEV = NewState;

	switch (iTE6805_DATA.STATEV) {
		case STATEV_VideoOff:
			iTE6805_Set_HPD_Ctrl(PORT0, HPD_LOW) ;
			iTE6805_Set_HPD_Ctrl(PORT1, HPD_LOW) ; // if downstream switch off the state, it should be hold and HPD goes to off.
			break;
		case STATEV_Unplug:

			// Auto Reset when B0
			if(iTE6805_DATA.ChipID == 0xA0 || iTE6805_DATA.ChipID == 0xA1)
			{
				// RD suggest START
				chgbank(0);
				hdmirxwr(0xC5, 0x12); // Unplug need to reset EDID, because EDID reset is also reset SCDC in hardware, or SCDC may hang in sometimes
				//delay1ms(1);
				hdmirxwr(0xC5, 0x02); // Unplug need to reset EDID, because EDID reset is also reset SCDC in hardware, or SCDC may hang in sometimes

				chgbank(4);
				hdmirxwr(0xC5, 0x12); // Unplug need to reset EDID, because EDID reset is also reset SCDC in hardware, or SCDC may hang in sometimes
				//delay1ms(1);
				hdmirxwr(0xC5, 0x02); // Unplug need to reset EDID, because EDID reset is also reset SCDC in hardware, or SCDC may hang in sometimes
				chgbank(0);
			}

			// force set SCDC Clock Ratio 1/40 and Scramble
			chgbank(3);
			hdmirxset(0xE5, 0x1C, 0x00);
			chgbank(0);

			Flag_First_Time_VidStable_Done = FALSE;
			Flag_FirstTimeParameterChange = FALSE;
			Flag_Disable_NewAVIInfoFrame_When_Parameter_Change = FALSE;

			iTE6805_DATA.Flag_VidStable_Done = FALSE;
			prevAVIDB[0] = 0;
			prevAVIDB[1] = 0;
			prevAudioB0_Status = 0;		// compare mask 0xF0
			prevAudioB1_Status = 0;		// compare mask 0xFF
			prevAudioB2_CHStatus = 0 ;	// compare mask BIT1
			deskewerrcnt = 0;
			Flag_HDCP_Trigger_AutoEQ_Again = TRUE;
			Current_Ch0_SymbolLockRst_Count = 0;
			Current_Ch1_SymbolLockRst_Count = 0;
			Current_Ch2_SymbolLockRst_Count = 0;

			#ifdef _ENABLE_IT6805_MHL_FUNCTION_
			iTE6805_DATA.MHL_RAP_Content_State = RAP_CONTENT_ON;
			iTE6805_DATA.MHL_DiscoveryDone = 0;
			#endif

			// RD suggest START
			hdmirxwr(0x08, 0x04); // port0
			hdmirxwr(0x0D, 0x04); // port1
			hdmirxwr(0x22, 0x12);
			hdmirxwr(0x22, 0x10);
			hdmirxwr(0x23, 0xAC);
			hdmirxwr(0x23, 0xA0);

			hdmirxwr(0x2B, 0xAC);
			hdmirxwr(0x2B, 0xA0);

			// RD suggest END
			HDMIRX_DEBUG_PRINT(("VidState change to STATEV_Unplug state\n"));
			iTE6805_Set_Video_Tristate(TRISTATE_ON);
			break;
		case STATEV_WaitSync:

			Flag_First_Time_VidStable_Done = FALSE;
			Flag_FirstTimeParameterChange = FALSE;
			Flag_Disable_NewAVIInfoFrame_When_Parameter_Change = FALSE;
			iTE6805_DATA.Flag_VidStable_Done = FALSE;
			deskewerrcnt = 0;
			HDMIRX_DEBUG_PRINT(("VidState change to STATEV_WaitSync state\n"));
			iTE6805_Set_Video_Tristate(TRISTATE_ON);
			break;
		case STATEV_CheckSync:
			HDMIRX_DEBUG_PRINT(("VidState change to STATEV_CheckSync state\n"));
			Current_SCDTCheck_Count = TIMEOUT_SCDT_CHECK_COUNT;
			break;
		case STATEV_VidStable:
			HDMIRX_DEBUG_PRINT(("VidState change to STATEV_VidStable state\n"));
			chgbank(0);
			hdmirxwr(0x90, 0x8F);	// for enable Rx Chip count

			iTE6805_Get_VID_Info();
			#if Debug_message
			iTE6805_Show_VID_Info();
			#endif
			iTE6805_Set_1B0_By_PixelClock(); // 0713 Andrew suggest if PCLK < 25 need to set 1B0[0] to 0, else 1
			iTE6805_Get_AVIInfoFrame_Info(); // add here for setting DownScale variable
			#ifdef iTE68051
			iTE68051_Video_Output_Setting(); // DaulPixel setting/ DownScaling setting/ Demo Setting/ PIN Config Setting only for 6805A0
			#endif
			#ifdef iTE68052
			iTE68052_Video_Output_Setting(); // DaulPixel setting/ DownScaling setting/ Demo Setting/ PIN Config Setting only for 6805A0
			#endif

			iTE6805_Enable_Video_Output();	// AVI Info Frame Handler , CSC Table Handler

			// need to set to AVMUTE_ON for might lost the PKT in the moment between Source-Video-In and Sink-Video-Stable/
			// and detect AVMUTE off status in the vid_fsm selection
			iTE6805_Set_AVMute(AVMUTE_ON);

			iTE6805_aud_chg(STATEA_RequestAudio);

			// 0330 fix it : no warning, because video logic reset need to reset parameter change INT and SCDT INT or it will be SCDT on->off
			// this cammand is wrong-> !!!!!! WARNING !!!!!!can't put logic reset between iTE6805_Set_AVMute and iTE6805_Set_AVMute or video tristate off then there will a SCDT off->SCDT on->SCDT off->......
			iTE6805_Reset_Video_Logic();

			iTE6805_Set_Video_Tristate(TRISTATE_OFF); // tristate must put after setting single/dual pixel mode, or can't judge it

			#ifdef iTE68051
			delay1ms(100);
			// 6028 init move from beginning to here because if init in the beginning, the DE between 6028 and 6805 can't sync
			// and the rightest V-line in the screen will be all block or white, need move 6028 init from beginning to here.
			//iTE6805_Init_6028LVDS(0); // init first 6028 chip
			//iTE6805_Init_6028LVDS(1); // init first 6028 chip

			//iTE6805_Init_6028LVDS(0); // init first 6028 chip
			//iTE6805_Init_6028LVDS(1); // init first 6028 chip
			// reset twice for ensurance ...
			#endif

			iTE6805_DATA.Flag_VidStable_Done = TRUE;
			Flag_First_Time_VidStable_Done = TRUE;

			// if this state here mean ECC error before, so if vid stable again, release EQ to EQDone state.
			// release state STATEEQ_KeepEQStateUntil5VOff to normal state
			#ifdef _ENABLE_AUTO_EQ_
			if( iTE6805_DATA.STATEEQ == STATEEQ_KeepEQStateUntil5VOff )
			{
				iTE6805_DATA.STATEEQ = STATEEQ_EQDone;
				iTE6805_EQ_chg(STATEEQ_EQDone);
			}
			#endif
			break;
		default:
			HDMIRX_DEBUG_PRINT(("ERROR: VidState change to Unknown state !!!\n"));
			break;
	}
}


// ***************************************************************************
// Audio State function
// ***************************************************************************
void iTE6805_aud_chg( STATEA_Type NewState )
{
	if(iTE6805_DATA.STATEA == NewState) return;
	iTE6805_DATA.STATEA = NewState;

	switch( iTE6805_DATA.STATEA ) {
		case STATEA_AudioOff :
		case STATEA_RequestAudio :
			HDMIRX_DEBUG_PRINT(("AudState change to STATEA_RequestAudio state !!!\n"));
			Current_AudioSamplingFreq_ErrorCount = 0;
			chgbank(0);
			hdmirxset(0x81,BIT6,0x00);	// RegForce_FS : 0: Disable Force Audio FS mode
			iTE6805_Set_Audio_Tristate(TRISTATE_ON);
			break;
		case STATEA_WaitForReady :
			HDMIRX_DEBUG_PRINT(("AudState change to STATEA_WaitForReady state !!!\n"));
			chgbank(0);
			hdmirxset(0x8C, BIT4, BIT4); // set RegHWMuteClr
			hdmirxset(0x8C, BIT4, 0x00); // clear RegHWMuteClr for clear H/W Mute
			Current_AudioCheck_Count = TIMEOUT_AUDIO_CHECK_COUNT;
			break;
		case STATEA_AudioOn :
			iTE6805_Show_AUD_Info();
			HDMIRX_DEBUG_PRINT(("AudState change to STATEA_AudioOn state !!!\n"));
			iTE6805_Enable_Audio_Output();
			iTE6805_Set_Audio_Tristate(TRISTATE_OFF);
			break;
		default :
			 HDMIRX_DEBUG_PRINT(("ERROR: AudState change to Unknown state !!!\n"));
			 break;
	}
}

// it6802PortSelect
void iTE6805_Port_Select(iTE_u8 ucPortSel)
{
	chgbank(0);
	HDMIRX_DEBUG_PRINT(("iTE6805_Port_Select= %d ",(int) ucPortSel));
    if( ucPortSel == PORT0 )
	{
	    hdmirxset(0x35, BIT0, 0); //select port 0
		iTE6805_Set_HPD_Ctrl(PORT1 , HPD_LOW);
	}
	else
	{
		hdmirxset(0x35, BIT0, 1); //select port 1
		if(iTE6805_DATA.ChipID == 0xA0)
		{
			hdmirxset(0xE2, BIT0, 0x00);
		}
		iTE6805_Set_HPD_Ctrl(PORT0 , HPD_LOW);
	}
	chgbank(0);
	iTE6805_DATA.CurrentPort = ucPortSel;
    //if(iTE6805_DATA.CurrentPort != ucPortSel)
    //{
		//iTE6805_DATA.CurrentPort = ucPortSel;
        //iTE6805_vid_chg(STATEV_WaitSync);
		//iTE6805_vid_chg(STATEV_Unplug);
    //}
}

#ifdef EVB_AUTO_DETECT_PORT_BY_PIN
iTE_u8 LAST_PORT = 3;
void iTE6805_Port_Detect()
{
	/*if(PORT_SWITCH == LAST_PORT)
	{
		return;
	}
	*/
	//if(PORT_SWITCH!=0)
	{
		iTE6805_Port_Select(PORT0);
		iTE6805_Reset_ALL_Logic(PORT0);
	}
	/*
	else
	{
		iTE6805_Port_Select(PORT1);
		iTE6805_Reset_ALL_Logic(PORT1);
	}

	LAST_PORT = PORT_SWITCH;*/
}
#endif

void iTE68051_Video_Output_Setting()
{
	iTE_u8 _iTE68051_4K60_Mode_;
	chgbank(0);
	// set Dual Pixel output mode to Odd/Even Mode
	hdmirxset(0x64, BIT2, 0);
	// setting LREnable need to reset REG64[1]
	hdmirxset(0x64, BIT1, BIT1);
	hdmirxset(0x64, BIT1, 0);

	chgbank(5);
	hdmirxset(0x20, 0x40, 0x40); // reset Downscale

	chgbank(1);
	hdmirxset(0xC0, BIT0, 0x00); // setting to single pixel mode
	chgbank(0);
	_iTE68051_4K60_Mode_ = iTE68051_4K60_Mode;

	#ifdef DEMO
	if (EDID_WP0 == 1)
	#else
	if(_iTE68051_4K60_Mode_ != MODE_DownScale || _iTE68051_4K60_Mode_ == MODE_EvenOdd_Plus_DownScale)
	#endif
	{
		#ifdef DEMO
		if (EDID_WP1 == 0)
			_iTE68051_4K60_Mode_ = MODE_EvenOdd;
		else
			_iTE68051_4K60_Mode_ = MODE_LeftRight;
		#endif

		if (iTE6805_Check_4K_Resolution())
		{
			HDMIRX_DEBUG_PRINT(("!!!!!! Set Dual Pixel Mode For 4K60 !!!!!!\n"));
			iTE6805_Set_LVDS_Video_Path(2);	// 2 lane for dual pixel mode
		}
		else
		{
			HDMIRX_DEBUG_PRINT(("!!!!!! Set Single Pixel Mode !!!!!!\n"));
			iTE6805_Set_LVDS_Video_Path(1);	// 1 lane for signle pixel mode
			_iTE68051_4K60_Mode_ = MODE_EvenOdd; // not 4k need to set to even odd mode
		}

		chgbank(0);
		switch (_iTE68051_4K60_Mode_)
		{
		case MODE_EvenOdd:
			hdmirxset(0x64, BIT2, 0);
			break;
		case MODE_LeftRight:
#if (Enable_LR_Overlap == 1)
			hdmirxset(0x64, BIT2|BIT3, BIT2|BIT3);
#else
			hdmirxset(0x64, BIT2, BIT2);
#endif
			break;
		}

		// setting _iTE68051_4K60_Mode_ hardware suggest reset REG64[1]
		hdmirxset(0x64, BIT1, BIT1);
		hdmirxset(0x64, BIT1, 0);

	}


	#ifdef DEMO
	if (EDID_WP0 == 0)
	#else
	if (_iTE68051_4K60_Mode_ == MODE_DownScale || _iTE68051_4K60_Mode_ == MODE_EvenOdd_Plus_DownScale)
	#endif
	{


#ifdef ENABLE_4K_MODE_ALL_DownScaling_1080p
		// more check HActive/VActive
		chgbank(0);
		iTE6805_CurVTiming.HActive = ((hdmirxrd(0x9E) & 0x3F) << 8) + hdmirxrd(0x9D);
		iTE6805_CurVTiming.VActive = ((hdmirxrd(0xA5) & 0x3F) << 8) + hdmirxrd(0xA4);
		if(iTE6805_Check_4K_Resolution() || (iTE6805_CurVTiming.HActive >=3000 && iTE6805_CurVTiming.VActive >= 2000))
		{
			HDMIRX_DEBUG_PRINT(("!!!!!Set Video DownSacle !!!!!\n"));
			iTE6805_Set_DNScale();
		}
#else
		if(iTE6805_Check_4K_Resolution())
		{
			HDMIRX_DEBUG_PRINT(("!!!!!Set Video DownSacle !!!!!\n"));
			iTE6805_Set_DNScale();
		}
#endif

	}
	chgbank(0);
}

void iTE68052_Video_Output_Setting()
{
	iTE_u8 _Mode_68052_4K_ = Mode_68052_4K;
	// reset to orign mode start
	chgbank(0);
	hdmirxwr(0x64, 0x83); // odd even mode
	hdmirxwr(0x64, 0x81);
	chgbank(1);
	hdmirxwr(0xBD, 0x0F); // lvdsmap set BD[6:7] to 00 (seq)
	hdmirxwr(0xBD, 0x0D); // and reset BD[1]
	chgbank(0);
	// reset to orign mode end

	chgbank(5);
	hdmirxwr(0xC1, 0x23);
	hdmirxwr(0xC2, 0x0B);
	hdmirxwr(0xCB, 0x6F);
	chgbank(1);
	hdmirxset(0xC0, 0x04, 0x04);
	if(iTE6805_Check_4K_Resolution()){
		iTE6805_Set_LVDS_Video_Path(4);	// 4 lane for dual pixel mode

		if(_Mode_68052_4K_ == MODE_LeftRight)
		{
			chgbank(0);
			hdmirxwr(0x64, 0x87);
			hdmirxwr(0x64, 0x85);
			chgbank(1);
			hdmirxwr(0xBD, 0x6F);
			hdmirxwr(0xBD, 0x6D);
			chgbank(0);
		}
	}else{
		iTE6805_Set_LVDS_Video_Path(2);	// 2 lane for single
	}




#ifdef Mode_68052_4K_DownScale
	#ifdef ENABLE_4K_MODE_ALL_DownScaling_1080p
	// more check HActive/VActive
	chgbank(0);
	iTE6805_CurVTiming.HActive = ((hdmirxrd(0x9E) & 0x3F) << 8) + hdmirxrd(0x9D);
	iTE6805_CurVTiming.VActive = ((hdmirxrd(0xA5) & 0x3F) << 8) + hdmirxrd(0xA4);
	#endif
	if (iTE6805_Check_4K_Resolution()
	#ifdef ENABLE_4K_MODE_ALL_DownScaling_1080p
	||	(iTE6805_CurVTiming.HActive >=3000 && iTE6805_CurVTiming.VActive >= 2000)
	#endif
	)
	{
		HDMIRX_DEBUG_PRINT(("!!!!!Set Video DownSacle !!!!!\n"));
		iTE6805_Set_DNScale();
		chgbank(1);
		hdmirxwr(0xBD, 0x1F); // lvdsmap set BD[6:7] to 00 (seq)
		hdmirxwr(0xBD, 0x1D); // and reset BD[1]
		chgbank(0);
		hdmirxwr(0x64, 0x83);
		hdmirxwr(0x64, 0x81);

	}
#endif

	// need to reset video logic or Left Screen Lan0, Lan1 might swap sometimes
	iTE6805_Reset_Video_Logic();
	chgbank(0);
}

// Block Enable
void iTE6805_Enable_Video_Output()
{
	iTE_s8 REG_RX_PIXCLK_SPEED;
	iTE_u8 USING_CSC_TABLE = FALSE;
	iTE_u8 cscpwd = 0;
	_CSCMtx_Type CSCMtx_Type = CSCMtx_Unknown;

	//  return;
	//iTE6805_DATA.DumpREG = TRUE;
	// REG6B[5:4]: Reg_ColMod_Set Input color mode set 00: RGB mode - 01: YUV422 mode, 10: YUV444 mode, 11: YUV420 mode
	chgbank(0);
	if (iTE6805_Check_HDMI_OR_DVI_Mode(iTE6805_DATA.CurrentPort) == MODE_HDMI)
	{
		HDMIRX_DEBUG_PRINT(("---- CSC HDMI mode ----\n"));
		iTE6805_Get_AVIInfoFrame_Info();
		iTE6805_Show_AVIInfoFrame_Info();
		hdmirxset(0x6B, 0x30, iTE6805_DATA.AVIInfoFrame_Input_ColorFormat << 4);// seting input format by info frame ??? do not need ???
		hdmirxset(0x6B, 0x01, 0x00);											// 0: Input color mode auto detect , 1: Force input color mode as bit[3:2] setting
	}
	else
	{
		HDMIRX_DEBUG_PRINT(("---- CSC DVI mode ----\n"));
		hdmirxset(0x6B, 0x30, 0x10); 											// seting input format to RGB
		hdmirxset(0x6B, 0x01, 0x01);											// 0: Input color mode auto detect , 1: Force input color mode as bit[3:2] setting

		if (iTE6805_DATA.CurrentPort == 1)chgbank(4);
		REG_RX_PIXCLK_SPEED = hdmirxrd(0x48);
		chgbank(0);

		// Set Colormetry for CSC , come from 6802 code
		if (REG_RX_PIXCLK_SPEED < 0x34)	iTE6805_DATA.AVIInfoFrame_Colorimetry = Colormetry_ITU709;
		else							iTE6805_DATA.AVIInfoFrame_Colorimetry = Colormetry_ITU601;
	}

	//return;
	// output = YUV444 , input = YUV422, 444, 420 no need csc , RGB need B_CSC_RGB2YUV, Udfilter!BDither �Dy?�G3B2z
	// output = YUV422 , input = YUV422, 444, 420 no need csc , RGB need B_CSC_RGB2YUV, Udfilter!BDither �Dy?�G3B2z
	// output = RGB888 , input = YUV444!BYUV422!B420 cs = B_CSC_YUV2RGB, Udfilter!BDither �Dy?�G3B2z
	// if no need csc, need pasing LIMIT RANGE to FULL RANGE / FULL RANGE TO LIMIT RANGE

	// 6805 new REG, Reg_CRCBLimit, Reg_ColorClip 6E[4:5], default 1 ,
	// if YCbCr Limit to RGB Full , should set to 0 , else 1
	hdmirxset(0x6E, 0x30, 0x30);
	HDMIRX_DEBUG_PRINT(("US_Output_ColorFormat = %d \n",(int) iTE6805_DATA.US_Output_ColorFormat));
	HDMIRX_DEBUG_PRINT(("AVIInfoFrame_Input_ColorFormat = %d \n",(int) iTE6805_DATA.AVIInfoFrame_Input_ColorFormat));
	if ((iTE6805_DATA.US_Output_ColorFormat == Color_Format_RGB) &&
		(iTE6805_DATA.AVIInfoFrame_Input_ColorFormat == Color_Format_YUV422 ||
		 iTE6805_DATA.AVIInfoFrame_Input_ColorFormat == Color_Format_YUV444 ||
		 iTE6805_DATA.AVIInfoFrame_Input_ColorFormat == Color_Format_YUV420))
	{
		HDMIRX_DEBUG_PRINT(("---- input YUV to output RGB ----\n"));
		// input YUV to output RGB
		hdmirxset(0x6C, 0x03, 0x03);	// CSC Sel 00: bypass CSC 10: RGB to YUV 11: YUV to RGB
		hdmirxset(0x6E, 0x78, 0x00);
		if (iTE6805_DATA.AVIInfoFrame_Colorimetry == Colormetry_ITU709)
		{
			if (iTE6805_DATA.AVIInfoFrame_YUVQuantizationRange == YUV_RANGE_FULL)
			{
				CSCMtx_Type = CSCMtx_YUV2RGB_ITU709_00_255;
			}
			else
			{
				CSCMtx_Type = CSCMtx_YUV2RGB_ITU709_16_235;
			}
		}
		else // Colormetry_ITU601
		{

			if (iTE6805_DATA.AVIInfoFrame_YUVQuantizationRange == YUV_RANGE_FULL)
			{
				CSCMtx_Type = CSCMtx_YUV2RGB_ITU601_00_255;
			}
			else
			{
				CSCMtx_Type = CSCMtx_YUV2RGB_ITU601_16_235;
			}
		}
	}
	else if (iTE6805_DATA.AVIInfoFrame_Input_ColorFormat == Color_Format_RGB &&
		    (iTE6805_DATA.US_Output_ColorFormat == Color_Format_YUV422 ||
		     iTE6805_DATA.US_Output_ColorFormat == Color_Format_YUV444 ||
		     iTE6805_DATA.US_Output_ColorFormat == Color_Format_YUV420))
	{
		HDMIRX_DEBUG_PRINT(("---- input RGB to output YUV ----\n"));
		// input RGB to output YUV
		hdmirxset(0x6C, 0x03, 0x02);	// CSC Sel 00: bypass CSC 10: RGB to YUV 11: YUV to RGB
		hdmirxset(0x6E, 0x78, 0x00);
		if (iTE6805_DATA.AVIInfoFrame_Colorimetry == Colormetry_ITU709)
		{
			if (iTE6805_DATA.AVIInfoFrame_RGBQuantizationRange == RGB_RANGE_LIMIT)
			{
				CSCMtx_Type = CSCMtx_RGB2YUV_ITU709_16_235;
			}
			else
			{
				CSCMtx_Type = CSCMtx_RGB2YUV_ITU709_00_255;
			}
		}
		else 	// Colormetry_ITU601
		{
			if (iTE6805_DATA.AVIInfoFrame_RGBQuantizationRange == RGB_RANGE_LIMIT)
			{
				CSCMtx_Type = CSCMtx_RGB2YUV_ITU601_16_235;
			}
			else
			{
				CSCMtx_Type = CSCMtx_RGB2YUV_ITU601_00_255;
			}
		}
	}
	else // YUV to YUV    or   RGB to RGB
	{
		HDMIRX_DEBUG_PRINT(("---- YUV to YUV or RGB to RGB ----\n"));
		hdmirxset(0x6C, 0x03, 0x00);	// CSC Sel 00: bypass CSC 10: RGB to YUV 11: YUV to RGB
		if (iTE6805_DATA.AVIInfoFrame_Input_ColorFormat == Color_Format_RGB)
		{
			iTE6805_DATA.US_RGBQuantizationRange = RGB_RANGE_FULL;
			if (iTE6805_DATA.AVIInfoFrame_RGBQuantizationRange == RGB_RANGE_LIMIT && iTE6805_DATA.US_RGBQuantizationRange != RGB_RANGE_LIMIT)
			{
				// RGB Limit Range to Full Range
				HDMIRX_DEBUG_PRINT(("using CSCMtx_RGB_16_235_RGB_00_255\n\n\n"));
				CSCMtx_Type = CSCMtx_RGB_16_235_RGB_00_255;
				hdmirxset(0x6E, 0x78, 0x78);	// Reg_EnUdFilt 0 1: Enable Color Up/Dn Filter R/W 0  come from 6802
												// Reg_EnDither 1 1: Enable Dither function R/W 0
												// Reg_DNFreeGo 2 1: Enable Dither!|s Fcnt function
												// RegAutoCSCSel 7 1: Enable HW CSCSel
				hdmirxset(0x6C, 0x03, 0x02);	// CSC Sel 00: bypass CSC 10: RGB to YUV 11: YUV to RGB ??? come from 6802
			}
			else if (iTE6805_DATA.AVIInfoFrame_RGBQuantizationRange == RGB_RANGE_FULL && iTE6805_DATA.US_RGBQuantizationRange == RGB_RANGE_LIMIT)
			{
				HDMIRX_DEBUG_PRINT(("using CSCMtx_RGB_00_255_RGB_16_235\n\n\n"));
				// RGB Full Range to Limit Range
				CSCMtx_Type = CSCMtx_RGB_00_255_RGB_16_235;
				hdmirxset(0x6E, 0x78, 0x40);	// come from 6802
				hdmirxset(0x6C, 0x03, 0x02);	// CSC Sel 00: bypass CSC 10: RGB to YUV 11: YUV to RGB ??? come from 6802
			}
		}
		// Do not need YUV Limit/Full Change
	}

	if(CSCMtx_Type != CSCMtx_Unknown)
	{
		HDMIRX_DEBUG_PRINT(("--- CSC_TABLE != NULL --- \n"));
		HDMIRX_DEBUG_PRINT(("--- Using CSC_TABLE = %d --- \n", (int) CSCMtx_Type));
		chgbank(1);
		hdmirxbwr(0x70, sizeof(CSC_Matrix[0]), &CSC_Matrix[CSCMtx_Type]);
		chgbank(0);
	}else{
	    //default to turn off CSC offset
		chgbank(0);
        hdmirxset(0x6E,0x78,0x00);
		// mark it lool the command blow //hdmirxset(0x4F,BIT6,BIT6); //1: power down color space conversion logic
		chgbank(1);
		hdmirxwr(0x85,0x00); // Clear Reg_CBOffset
		chgbank(0);

	}

	//Set Video Output CD by input CD
	iTE6805_Set_ColorDepth();

	// follow 6802 (Input mode is YUV444/Input mode is YUV420 and Output mode is YUV444) or Input/Output = RGB
	// 20170119 can't power down CSC,
	// if power down CSC, Dualpixel mode LR mode/OddEven Mode will be shut down because CSC shut down
	// so mark this section.
	/*
	if(((iTE6805_DATA.AVIInfoFrame_Input_ColorFormat == Color_Format_YUV444 || iTE6805_DATA.AVIInfoFrame_Input_ColorFormat == Color_Format_YUV420 ) && iTE6805_DATA.US_Output_ColorFormat == Color_Format_YUV444)
	 || (iTE6805_DATA.AVIInfoFrame_Input_ColorFormat == Color_Format_RGB && iTE6805_DATA.US_Output_ColorFormat == Color_Format_RGB) )
	{
		hdmirxset(0x4F, BIT6, BIT6); // power down CSC
	}
	else
	{
		hdmirxset(0x4F, BIT6, 0);	// power on CSC
	}*/

	//iTE6805_Reset_Video_FIFO();
	//iTE6805_DATA.DumpREG = FALSE;
}

// Block Enable End


// AudioFsCal
void iTE6805_Enable_Audio_Output(void)
{
    iTE_u8 SW_Sampling_Frequency, Force_Sampling_Frequency;
    iTE_u8 Audio_CH_Status, resetAudio = 0;
	iTE_u32 sum=0;
	chgbank(0);

	// RegForce_CTSMode : need to set to 1 for get the CTS in the PKT, 0 repersent nothing (HW using)
	// so set to 1 to get CTS in the REG2C1/2C2/2C0
    hdmirxset(0x86,BIT0,BIT0);
	chgbank(2);
	iTE6805_CurVTiming.N	= ((iTE_u32)hdmirxrd(0xBE)<<12) + ((iTE_u32)hdmirxrd(0xBF)<<4) + ((iTE_u32)hdmirxrd(0xC0)&0x0F);
	iTE6805_CurVTiming.CTS = hdmirxrd(0xC0) >> 4;
	iTE6805_CurVTiming.CTS += ((iTE_u32)hdmirxrd(0xC1)) << 12;
	iTE6805_CurVTiming.CTS += ((iTE_u32)hdmirxrd(0xC2)) << 4;

	HDMIRX_AUDIO_PRINTF(("N = %ld \n", iTE6805_CurVTiming.N));
	HDMIRX_AUDIO_PRINTF(("CTS = %ld \n", iTE6805_CurVTiming.CTS));
	HDMIRX_AUDIO_PRINTF(("TMDSCLK = %ld \n", iTE6805_CurVTiming.TMDSCLK));
	chgbank(0);

    sum = (iTE6805_CurVTiming.N * (iTE6805_CurVTiming.TMDSCLK) );
	HDMIRX_AUDIO_PRINTF(("sum = %lu \n", sum));
	// TMDS <= 340 M ,fs = N * TMDSCLK / 128 * CTS , in hdmi2.0 page 85
	// TMDS > 340  M ,fs = 4* N * TMDSCLK / 128 * CTS
	//if(iTE6805_CurVTiming.TMDSCLK > 340000)

	//if(iTE6805_Check_HDMI2())
	//{
	//	SW_Sampling_Frequency = (iTE_u8) ((sum/(iTE_u32)32)/iTE6805_CurVTiming.CTS);
	//}																					 y
	//else
	//{
	//	SW_Sampling_Frequency = (iTE_u8) ((sum/(iTE_u32)128)/iTE6805_CurVTiming.CTS);
	//}
	if(iTE6805_CurVTiming.CTS == 0){
		SW_Sampling_Frequency = 0;
	}else{
	    SW_Sampling_Frequency = (iTE_u8) (sum/(128*iTE6805_CurVTiming.CTS));
	//HDMIRX_AUDIO_PRINTF(("SW caulate SW_Sampling_Frequency = %d \n",(int) SW_Sampling_Frequency));	
	}
    //SW_Sampling_Frequency = (iTE_u8) (sum/(128*iTE6805_CurVTiming.CTS));
	HDMIRX_AUDIO_PRINTF(("SW caulate SW_Sampling_Frequency = %d \n",(int) SW_Sampling_Frequency));

    if(SW_Sampling_Frequency>25 && SW_Sampling_Frequency<=38){
        Force_Sampling_Frequency = Audio_Sampling_32K;
    }
    else if(SW_Sampling_Frequency>38 && SW_Sampling_Frequency<=45){
        Force_Sampling_Frequency = Audio_Sampling_44P1K;
    }
    else if(SW_Sampling_Frequency>45 && SW_Sampling_Frequency<=58){
        Force_Sampling_Frequency = Audio_Sampling_48K;
    }
	else if(SW_Sampling_Frequency>58 && SW_Sampling_Frequency<=78){
        Force_Sampling_Frequency = Audio_Sampling_64K;
    }
    else if(SW_Sampling_Frequency>78 && SW_Sampling_Frequency<=92){
        Force_Sampling_Frequency = Audio_Sampling_88P2K;
    }
    else if(SW_Sampling_Frequency>92 && SW_Sampling_Frequency<=106){
        Force_Sampling_Frequency = Audio_Sampling_96K;
    }
	else if(SW_Sampling_Frequency>106 && SW_Sampling_Frequency<=166){
		Force_Sampling_Frequency = Audio_Sampling_128K;
	}
    else if(SW_Sampling_Frequency>166 && SW_Sampling_Frequency<=182){
        Force_Sampling_Frequency = Audio_Sampling_176P4K;
    }
    else if(SW_Sampling_Frequency>182 && SW_Sampling_Frequency<=202){
        Force_Sampling_Frequency = Audio_Sampling_192K;
    }

	chgbank(0);
	// in the hdmi2.0 page 84, need bit 24, 25, 26, 27, 30, 31
    Audio_CH_Status = hdmirxrd(0xB5) & 0x0F;	// Audio_CH_Status : Audio Channel status decoder value[31:24] and bit[24:27] = Audio Sampling Rate
	Audio_CH_Status = (Audio_CH_Status | ((hdmirxrd(0xB6)&0xC0) >> 2)) ;

    HDMIRX_AUDIO_PRINTF(("Audio_CH_Status[24:27 - 30:31][bit0~bit5] = %x ,",(int) Audio_CH_Status ));
    HDMIRX_AUDIO_PRINTF(("Force_Sampling_Frequency %x\n",(int) ( Force_Sampling_Frequency) ));

	// SW caulate Sampling Frequency equal to 60958-3 SPEC channel status, no need to enable Force FS mode
	if( Audio_CH_Status == Force_Sampling_Frequency){
		HDMIRX_AUDIO_PRINTF(("Audio_CH_Status == Force_Sampling_Frequency reset Audio \n"));
		if( hdmirxrd(0x81) & BIT6) resetAudio = 1; // If Already Force FS Mode, Need to reset Audio;

        hdmirxset(0x81,BIT6,0x00);	// RegForce_FS : 0: Disable Force Audio FS mode

		if( resetAudio ) iTE6805_Reset_Audio_Logic();
		Current_AudioSamplingFreq_ErrorCount=0;
        return;
    }

	Current_AudioSamplingFreq_ErrorCount++;
    HDMIRX_AUDIO_PRINTF(("Current_AudioSamplingFreq_ErrorCount=%d \n",(int) Current_AudioSamplingFreq_ErrorCount));

	// exceed max error count , enable Force Sampling Mode
    if(Current_AudioSamplingFreq_ErrorCount > Max_AudioSamplingFreq_ErrorCount)
    {
		hdmirxset(0x81, BIT6, BIT6);	// RegForce_FS : Force Audio FS mode
        hdmirxset(0x8A, 0x3F, Force_Sampling_Frequency); // RegFS_Set[5:0] : Software set sampling frequency

		Current_AudioSamplingFreq_ErrorCount=0;
        iTE6805_Reset_Audio_Logic();
        HDMIRX_AUDIO_PRINTF(("ForceAudio Mode\n"));
    }
}

//iTE_u8 LAST_5V_STATE = MODE_5V_OFF;

void iTE6805_INT_5VPWR_Chg(iTE_u8 ucport){

	//if( LAST_5V_STATE == iTE6805_Check_5V_State(ucport)) return;
	if(iTE6805_Check_5V_State(ucport))
	{
		#ifdef ENABLE_68051_POWER_SAVING_MODE
		iTE6805_Set_Power_Mode(MODE_POWER_NORMAL);
		#endif

		iTE6805_DEBUG_INT_PRINTF(("# Power 5V ON #\n"));
		//iTE6805_DATA.DumpREG = TRUE;
		iTE6805_vid_chg(STATEV_WaitSync);
		iTE6805_Set_HPD_Ctrl(ucport,HPD_HIGH);
	}
	else
	{
		iTE6805_DEBUG_INT_PRINTF(("# Power 5V OFF #\n"));
		iTE6805_Set_HPD_Ctrl(ucport,HPD_LOW);
		iTE6805_vid_chg(STATEV_Unplug);

		#ifdef _ENABLE_AUTO_EQ_
		// Because add STATEEQ_KeepEQStateUntil5VOff STATE for ECC Error ,
		// need using iTE6805_DATA.STATEEQ = STATEEQ_Off; for leave that state.
		iTE6805_DATA.STATEEQ = STATEEQ_Off;
		iTE6805_EQ_chg(STATEEQ_Off);
		#endif

		#ifdef ENABLE_68051_POWER_SAVING_MODE
		iTE6805_Set_Power_Mode(MODE_POWER_SAVING);
		#endif
	}
	//LAST_5V_STATE = !LAST_5V_STATE;
}

// hdmirx_INT_HDMIMode_Chg
void iTE6805_INT_HDMI_DVI_Mode_Chg(iTE_u8 ucport)
{

	if (iTE6805_Check_HDMI_OR_DVI_Mode(ucport) == MODE_HDMI)
	{
		if (iTE6805_DATA.STATEV == STATEV_VidStable)
		{
			iTE6805_aud_chg(STATEA_RequestAudio);
		}

		iTE6805_DEBUG_INT_PRINTF(("# HDMI/DVI Mode : HDMI #\n"));
	}
	else
	{
		iTE6805_aud_chg(STATEA_AudioOff);
		if (iTE6805_DATA.STATEV == STATEV_VidStable)
		{
			iTE6805_Enable_Video_Output();
		}
		iTE6805_DEBUG_INT_PRINTF(("# HDMI/DVI Mode : DVI #\n"));
	}
}

// hdmirx_INT_P0_ECC
void iTE6805_INT_ECC_ERROR()
{
	if( (Current_ECCError_Count++) > TIMEOUT_ECC_ERROR )
    {

		Current_ECCError_Count = 0;
		Current_ECCAbnormal_Count++;
		if(Current_ECCAbnormal_Count == 3)
		{
			Current_ECCAbnormal_Count = 0;
			iTE6805_Reset_ALL_Logic(iTE6805_DATA.CurrentPort);
			return;
		}

		#ifdef _ENABLE_AUTO_EQ_
		iTE6805_DEBUG_INT_PRINTF(("++++++++++++++ECC ERROR TIMEOUT ++++++++++++++ Force EQ STATEEQ_KeepEQStateUntil5VOff \n" ));
		// Force EQ do not change anymore, only using iTE6805_DATA.STATEEQ = STATEEQ_KeepEQStateUntil5VOff here
		// other code need using iTE6805_EQ_chg(); to change state
		iTE6805_DATA.STATEEQ = STATEEQ_KeepEQStateUntil5VOff;

		// Force EQ in this state and set HPD
		iTE6805_Set_HPD_Ctrl(iTE6805_DATA.CurrentPort, HPD_LOW);
		delay1ms(100);
		iTE6805_Set_HPD_Ctrl(iTE6805_DATA.CurrentPort, HPD_HIGH);
		#endif
    }
}

iTE_u8 LAST_SCDT_STATE = SCDT_OFF;
// hdmirx_INT_SCDT_Chg
void iTE6805_INT_SCDT_Chg()
{
	// kuro add may add a time to check stable really change ?
	if(LAST_SCDT_STATE == iTE6805_Check_SCDT()) return;
	LAST_SCDT_STATE = iTE6805_Check_SCDT();
    if(LAST_SCDT_STATE)
	{
		// emily 20170718 add for HDCP CTS
		chgbank(0);
		hdmirxset(0x40, 0x03, 0x00);
        iTE6805_DEBUG_INT_PRINTF(("# SCDT ON #\n"));
        iTE6805_vid_chg(STATEV_CheckSync);
    }
    else
	{
		// emily 20170718 add for HDCP CTS
		chgbank(0);
		hdmirxset(0x40, 0x03, 0x02);

        iTE6805_DEBUG_INT_PRINTF(("# SCDT OFF #\n"));
		iTE6805_vid_chg(STATEV_WaitSync);
        iTE6805_aud_chg(STATEA_AudioOff);
    }
}

// only using for MHL
void iTE6805_INT_HDMI_Mode_Chg()
{

	if(iTE6805_DATA.CurrentPort != PORT0)
	{
		return;
	}

	if(iTE6805_Check_PORT0_IS_MHL_Mode(PORT0) == MODE_MHL)
	{
		chgbank(3);
        hdmirxset(0x3A, 0x06, 0x04); // set Reg_ENMEQ
        chgbank(0);
		iTE6805_aud_chg(STATEA_AudioOff);
	}
	else
	{
		chgbank(3);
        hdmirxset(0x3A, 0x06, 0x02); // set Reg_ENHEQ
        chgbank(0);
	}

}

void iTE6805_Identify_Chip()
{
	iTE_u8 Result = 0;
	iTE_u8 REG00;
	iTE_u8 REG01;
	iTE_u8 REG02;
	iTE_u8 REG03;

	do
	{
		REG00 = hdmirxrd(0x00);
		REG01 = hdmirxrd(0x01);
		REG02 = hdmirxrd(0x02);
		REG03 = hdmirxrd(0x03);
		if( REG00 != 0x54 ||
			REG01 != 0x49 ||
			REG02 != 0x05 ||
			REG03 != 0x68 )
		{
			HDMIRX_DEBUG_PRINT(("This is not iTE6805 chip !!!\n"));
			HDMIRX_DEBUG_PRINT(("REG00 = %X !!!\n",(int)REG00));
			HDMIRX_DEBUG_PRINT(("REG01 = %X !!!\n",(int)REG01));
			HDMIRX_DEBUG_PRINT(("REG02 = %X !!!\n",(int)REG02));
			HDMIRX_DEBUG_PRINT(("REG03 = %X !!!\n",(int)REG03));
			Result = 0;
		}
		else
		{
			Result = 1;
		}

	}while(Result==0);

	//iTE6805_DATA.ChipID = 0xA0; // kuro test
	iTE6805_DATA.ChipID = hdmirxrd(0x04);
	HDMIRX_DEBUG_PRINT(("iTE6805_DATA.ChipID = %X !!!\n",(int) iTE6805_DATA.ChipID ));
}

#ifdef DYNAMIC_HDCP_ENABLE_DISABLE
void iTE6805_HDCP_Detect()
{
	if(iTE6805_DATA.STATE_HDCP == iTE6805_DATA.STATE_HDCP_FINAL)
	{
		return;
	}
	iTE6805_Set_HPD_Ctrl(iTE6805_DATA.CurrentPort, HPD_LOW);
	iTE6805_Set_HDCP(iTE6805_DATA.STATE_HDCP);
	delay1ms(100); //HPD OFF spec need at least 100ms
	iTE6805_Set_HPD_Ctrl(iTE6805_DATA.CurrentPort, HPD_HIGH);
	iTE6805_DATA.STATE_HDCP_FINAL = iTE6805_DATA.STATE_HDCP;
}
#endif
