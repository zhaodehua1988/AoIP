///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_DRV.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
#include "iTE6805_Global.h"
#include "iTE6805_VM_TABLE.h"
#include "iTE6805_INI_Table.h"
#include "iTE6805_MHL_INI_Table.h"
#include "iTE6805_DEV_DEFINE.h"
 #include <unistd.h>

//extern _iTE6805_DATA	iTE6805_DATA;
_iTE6805_DATA  iTE6805_DATA;
//extern _iTE6805_VTiming iTE6805_CurVTiming;
_iTE6805_VTiming iTE6805_CurVTiming;

iTE_u32 RCLKVALUE = 0;
iTE_u8	CEC_timeunit = 0;

void iTE6805_delay1ms(iTE_u32 time_ms)
{
	usleep(time_ms*1000);
}
// Block Set
void iTE6805_Set_AVMute(iTE_u8 AVMUTE_STATE)
{
	//	1C5
	//		Reg_TriSVDIO	4:0	1: Tri-state Video data IO when single pixel mode	R/W	11111
	//		Reg_TriSVDLLIO	5	1: Tri-state Video data IO when single pixel mode	R/W	1
	//		Reg_TriSVDLHIO	6	1: Tri-state Video data IO when single pixel mode	R/W	1
	//		Reg_VIOSel	7	1: VIO Tri-state controlled by VIO setting
	//						0: VIO Tri-state controlled by VDIO setting	R/W	1
	//	1C6	Reg_TriDVDIO	5:0	1: Tri-state Video data IO when dual pixel mode	R/W	111111
	//		Reg_TriDVDLLIO	6	1: Tri-state Video data IO when dual pixel mode	R/W	1
	//		Reg_TriVIO	7	1: Tri-state Video control IO	R/W	1

	//	4F	RegDisVAutoMute	7	1: disable video auto mute
	//		RegVDGatting	5	Enable output data gating to zero when no Video display

	// If AVMUTE ON, VIOSel should set 1
	// If VIOSel = 1, If no data, sync still on.
	// If VIOSel = 0, If no data, sync will stop.
	// Therefore AVMute represent sync still on but should disable video.
	// so set 3 set the output black clock (no data), and set VIOSel (sync still go on) or it will reSync(Check Stable) again.
	// If AV Mute On, hardware auto AV mute mute go before SW or it will be a short delay for shut the screen block
	// so Hardware auto AVMute on -> disable AVMute for triiger AVMute again.
	if (AVMUTE_STATE == AVMUTE_ON)
	{
		chgbank(0);
		hdmirxset(0x4F, BIT5 + BIT7, BIT5 + BIT7); // enable VD Gatting , RegDisVAutoMute : 1: Disable video auto mute
		chgbank(1);
		hdmirxset(0xC5, BIT7, BIT7); // Reg_VIOSel  :  1: VIO Tri-state controlled by VIO setting, 0: VIO Tri-state controlled by VDIO setting
		HDMIRX_VIDEO_PRINTF(("+++++++++++ iTE6805_Set_AVMute -> On\n"));
	}
	else
	{
		HDMIRX_VIDEO_PRINTF(("+++++++++++  iTE6805_Set_AVMute -> Off\n"));
		if (iTE6805_DATA.STATEV == STATEV_VidStable)
		{
			if (iTE6805_Check_AVMute())
			{
				chgbank(0);
				hdmirxset(0x4F, BIT7, BIT7); // RegDisVAutoMute : 1: Disable video auto mute
			}
			else
			{
				// clear AV Mute
				chgbank(1);
				hdmirxset(0xC5, BIT0, BIT0);
				hdmirxset(0xC5, BIT0, 0);
				if (iTE6805_Check_Single_Dual_Mode() == MODE_DUAL)
				{
					chgbank(1);
					hdmirxset(0xC5, BIT0, BIT0);
				}
				chgbank(0);

				hdmirxset(0x4F, BIT5|BIT7, BIT5|BIT7);	// enable VD Gatting
				hdmirxset(0x4F, BIT5|BIT7, BIT7);		// disable VD Gatting
			}
		}
	}
}

void iTE6805_Set_ColorDepth(){
	//iTE6805_CurVTiming.ColorDepth = (hdmirxrd(0x98) & 0xF0) >> 4;
	VIDEOTIMNG_DEBUG_PRINTF(("\n Force set Output ColorDepth = "));
	chgbank(0);
	switch (Output_Color_Depth)
	{
		case 10:
			VIDEOTIMNG_DEBUG_PRINTF(("10 b\n"));
			hdmirxset(0x6B, 0x03, 0x01);
			break;
		case 12:
			VIDEOTIMNG_DEBUG_PRINTF(("12 b\n"));
			hdmirxset(0x6B, 0x03, 0x02);
			break;
		case 8:
			VIDEOTIMNG_DEBUG_PRINTF(("8 b\n"));
			hdmirxset(0x6B, 0x03, 0x00);
			break;
		default:
			hdmirxset(0x6B, 0x03, 0x00);
			VIDEOTIMNG_DEBUG_PRINTF(("8 b\n"));
			break;
	}
}

void iTE6805_Set_LVDS_Video_Path(iTE_u8 LaneNum)
{
	#ifdef iTE68051
	chgbank(1);
	if(LaneNum == 2)
	{
		hdmirxset(0xC0, BIT0, BIT0); // setting to dual pixel mode
	}
	#endif
	chgbank(5);
	hdmirxset(0xD1, BIT0, LVDSMode);	// RegPixMap
	hdmirxset(0xD1, 0x0C, LVDSColDep);	// RegLVColDep
	hdmirxset(0xDA, BIT4, LVDSSSC);		// RegSPENSSC
	hdmirxwr(0xD0, 0xF3); // RegEnLVDS = 1
	chgbank(1);

	switch (LaneNum)
	{
		case 1:
			hdmirxset(0xBD, 0x30, 0x00); // [5:4] LaneNum
			break;
		case 2:
			hdmirxset(0xBD, 0x30, 0x10); // [5:4] LaneNum
			break;
		case 4:
			hdmirxset(0xBD, 0x30, 0x20); // [5:4] LaneNum
			break;
		default:
			break;
	}

	if (LVDSSwap)
	{
		hdmirxwr(0xBE, 0x02);
		hdmirxset(0xFE, 0x10, 0x10);// inverse VGA out clk
	}
	else
	{
		hdmirxwr(0xBE, 0x00);
		hdmirxset(0xFE, 0x10, 0x10);// inverse VGA out clk
	}
	chgbank(0);

}

void iTE6805_Set_TTL_Video_Path()
{
	chgbank(1);
	hdmirxset(0xC0, 0x06, 0x02);
	// Configure TTL Video Output mode
	// Reg1C1 [0] TTL half bus DDR mode , [1] TTL full bus DDR mode , [5] Enable sync embedded , [6] Enable BTA1004 mode
	// BTA1004 mode must SyncEmb
    switch(iTE6805_DATA.US_Video_Out_Data_Path)
	{
        case eTTL_SDR:
			hdmirxset(0xC1, BIT1, 0x00);
            break;
        case eTTL_HalfPCLKDDR: //use this
            hdmirxset(0xC1, BIT1, BIT1);
            break;
        case eTTL_HalfBusDDR:
            hdmirxset(0xC1, BIT0, BIT0);
            break;
        case eTTL_SDR_BTA1004:
            hdmirxset(0xC1, BIT5|BIT6, BIT5|BIT6);
            break;
        case eTTL_DDR_BTA1004:
            hdmirxset(0xC1, BIT1|BIT5|BIT6, BIT1|BIT5|BIT6);
			break;

    }

    switch(iTE6805_DATA.US_Video_Sync_Mode)
	{
        case eTTL_SepSync:  //use this
			hdmirxset(0xC1, BIT5, 0x00); 
            break;
        case eTTL_EmbSync:
            hdmirxset(0xC1, BIT5, BIT5);
            break;
    }

	chgbank(0);
	//Set Video Output Color Format
    switch(iTE6805_DATA.US_Output_ColorFormat)
	{
        case Color_Format_RGB :
            hdmirxset(0x6B,0x0C,0x00);
            break;
        case Color_Format_YUV422 :
            hdmirxset(0x6B,0x0C,0x04);
            break;
        case Color_Format_YUV444 :
            hdmirxset(0x6B,0x0C,0x08);
            break;
		case Color_Format_YUV420 :
            hdmirxset(0x6B,0x0C,0x0C);
            break;
    }
	// Color Depth Setting depend on input color depth , do not need setting here.
}

void iTE6805_Set_DNScale()
{
	iTE_u16 Max_YRGB;
	iTE_u16 Min_YRGB;
	iTE_u16 Max_CRCB;
	iTE_u16 Min_CRCB;

	iTE_u16 Src_Width;
	iTE_u16 Src_Height;
	iTE_u16 Ratio_Denominator;
	iTE_u16 Ratio_Numerator;
	iTE_u16 Ratio_Offset;

	iTE_u16 TG_HSPOL;
	iTE_u16 TG_VSPOL;
	iTE_u16	TG_HFP;
	iTE_u16	TG_HSW;
	iTE_u16	TG_HBP;
	iTE_u16	TG_HDEW;
	iTE_u16	TG_VFP;
	iTE_u16	TG_VSW;
	iTE_u16	TG_VBP;
	iTE_u16	TG_VDEW;
	iTE_u16 Double;
	iTE_u8	GCP_CD;

	//iTE6805_DATA.DumpREG = TRUE;
	// 20160926 reset downscale, and set register value,
	// release reset after register setting
	VIDEOTIMNG_DEBUG_PRINTF(("---- Video DownScale Start! ----\n"));
	chgbank(0);
	hdmirxset(0x33, 0x08, 0x08);

	if (iTE6805_DATA.AVIInfoFrame_Input_ColorFormat == Color_Format_YUV420) Double = 1;
	else Double = 0;
	Src_Width = (iTE6805_CurVTiming.HActive) << Double;
	Src_Height = iTE6805_CurVTiming.VActive;
	Ratio_Denominator = Src_Width - 2;
	Ratio_Numerator = (Src_Width / 2) - 1;
	Ratio_Offset = (Src_Width / 2) - 1;

	if(iTE6805_DATA.US_Output_ColorFormat == Color_Format_RGB)
	{
		Max_YRGB = 0xFFF;
		Min_YRGB = 0x000;
		Max_CRCB = 0xFFF;
		Min_CRCB = 0x000;
	}
	else
	{
#if (DownScale_YCbCr_Color_Range == 1)
		Max_YRGB = 0xFFF;
		Min_YRGB = 0x000;
		Max_CRCB = 0xFFF;
		Min_CRCB = 0x000;
#else
		Max_YRGB = 940;	// 235*4
		Min_YRGB = 64;	// 16*4
		Max_CRCB = 940;	// 235*4
		Min_CRCB = 64;	// 16*4
#endif
	}

	GCP_CD = ((hdmirxrd(0x98)&0xF0)>>4);

	TG_HSPOL = iTE6805_CurVTiming.HPolarity;
	TG_VSPOL = iTE6805_CurVTiming.HPolarity;
	TG_HFP = ((iTE6805_CurVTiming.HFrontPorch) / 2) << Double;
	TG_HSW = ((iTE6805_CurVTiming.HSyncWidth) / 2) << Double;
	TG_HBP = ((iTE6805_CurVTiming.HBackPorch) / 2) << Double;
	TG_HDEW = ((iTE6805_CurVTiming.HActive) / 2) << Double;

	// for VIC 100 : HDE 4096 to 3840 only
	//	TG_HDEW = 1920;
	//  TG_HBP  = 2200 -  TG_HFP - TG_HSW -1920;
	// end  for HDE 4096 to 3840 only

	TG_VFP = (iTE6805_CurVTiming.VFrontPorch) / 2;
	TG_VSW = (iTE6805_CurVTiming.VSyncWidth) / 2;
	TG_VBP = (iTE6805_CurVTiming.VBackPorch) / 2;
	TG_VDEW = (iTE6805_CurVTiming.VActive) / 2;

	// for 4096 to 1080p only
#ifdef EVB_4096_DOWNSCALE_TO_2048_OR_1920_CONTROL_BY_PIN
	if(EVB_CONTROL_4K_DOWNSCALE==0)
	{
#endif

#ifdef ENABLE_4K_MODE_4096x2048_DownScale_To_1920x1080p
		if(TG_HDEW == 2048)
		{
			TG_HSPOL = 1;
			TG_VSPOL = 1;
			TG_HDEW = 1920 ;
			TG_HSW = 44 ;
			TG_HBP = 148 ;

			TG_HFP = ((iTE6805_CurVTiming.HTotal/2) << Double) - (TG_HBP) - (TG_HSW) - 1920 ;
			Ratio_Numerator = TG_HDEW -1;
			Ratio_Offset = TG_HDEW - 1;
			VIDEOTIMNG_DEBUG_PRINTF(("!!!! New Double = %d !!!!\n", Double));
			VIDEOTIMNG_DEBUG_PRINTF(("!!!! TG_HFP = %d !!!!\n", TG_HFP));
			VIDEOTIMNG_DEBUG_PRINTF(("!!!! DownScale 2048x1080 to 1920x1080 !!!!\n"));
		}
#endif

#ifdef EVB_4096_DOWNSCALE_TO_2048_OR_1920_CONTROL_BY_PIN
	}
#endif


	VIDEOTIMNG_DEBUG_PRINTF(("Target HActive= %d , HFrontPorch=%d, HSync Width=%d, HBackPorch=%d \n", TG_HDEW, TG_HFP, TG_HSW, TG_HBP));
	VIDEOTIMNG_DEBUG_PRINTF(("Target VActive= %d , VFrontPorch=%d, VSync Width=%d, VBackPorch=%d \n", TG_VDEW, TG_VFP, TG_VSW, TG_VBP));
	chgbank(5);
	hdmirxwr(0x21, Src_Width & 0xFF);
	hdmirxwr(0x22, (Src_Width & 0xFF00) >> 8);
	hdmirxwr(0x23, Src_Height & 0xFF);
	hdmirxwr(0x24, (Src_Height & 0xFF00) >> 8);
	hdmirxwr(0x25, Ratio_Denominator & 0xFF);
	hdmirxwr(0x26, (Ratio_Denominator & 0xFF00) >> 8);
	hdmirxwr(0x27, Ratio_Numerator & 0xFF);
	hdmirxwr(0x28, (Ratio_Numerator & 0xFF00) >> 8);
	hdmirxwr(0x29, Ratio_Offset & 0xFF);
	hdmirxwr(0x2A, (Ratio_Offset & 0xFF00) >> 8);
	hdmirxwr(0x2B, Max_YRGB & 0xFF);
	hdmirxwr(0x2C, Min_YRGB & 0xFF);
	hdmirxwr(0x2D, ((Min_YRGB & 0xF00) >> 4) + (Max_YRGB & 0xF00) >> 8 );

	hdmirxwr(0x2E, Max_CRCB & 0xFF);
	hdmirxwr(0x2F, Min_CRCB & 0xFF);
	hdmirxwr(0x30, ((Min_CRCB & 0xF00) >> 4) + (Max_CRCB & 0xF00) >> 8 );

	hdmirxwr(0x31, 0x10 + (0x02 << 2) + (TG_HSPOL << 1) + TG_VSPOL); // this color depth force set to 12bits Reg31 2:3
	hdmirxwr(0x32, TG_HFP & 0xFF);
	hdmirxwr(0x33, (TG_HFP & 0x3F00) >> 8);
	hdmirxwr(0x34, TG_HSW & 0xFF);
	hdmirxwr(0x35, (TG_HSW & 0x3F00) >> 8);
	hdmirxwr(0x36, TG_HBP & 0xFF);
	hdmirxwr(0x37, (TG_HBP & 0x3F00) >> 8);
	hdmirxwr(0x38, TG_HDEW & 0xFF);
	hdmirxwr(0x39, (TG_HDEW & 0x3F00) >> 8);
	hdmirxwr(0x3A, TG_VFP & 0xFF);
	hdmirxwr(0x3B, (TG_VFP & 0x3F00) >> 8);
	hdmirxwr(0x3C, TG_VSW & 0xFF);
	hdmirxwr(0x3D, (TG_VSW & 0x3F00) >> 8);
	hdmirxwr(0x3E, TG_VBP & 0xFF);
	hdmirxwr(0x3F, (TG_VBP & 0x3F00) >> 8);
	hdmirxwr(0x40, TG_VDEW & 0xFF);
	hdmirxwr(0x41, (TG_VDEW & 0x3F00) >> 8);
	hdmirxset(0x20, 0x40, 0x00);
	VIDEOTIMNG_DEBUG_PRINTF(("---- Video DownScale Enable! Video DownScale Enable! ----\n"));
	VIDEOTIMNG_DEBUG_PRINTF(("---- Video DownScale End! ----\n"));
	chgbank(0);
	hdmirxset(0x33, 0x08, 0x00);
	//iTE6805_DATA.DumpREG = FALSE;
}
// Block Set End


// Blcok Get
void iTE6805_Get_AVIInfoFrame_Info()
{
    chgbank(2);
    iTE6805_DATA.AVIInfoFrame_Input_ColorFormat = ((hdmirxrd(REG_RX_AVI_DB1)&0x60)>>5);   			// CEA page 43 RGB/YCbCr
    iTE6805_DATA.AVIInfoFrame_Colorimetry = ((hdmirxrd(REG_RX_AVI_DB2)&0xC0)>>6); 			// 10=IT709 11=ExtendedColorimetry
    //iTE6805_DATA.AVIInfoFrame_ExtendedColorimetry = ((hdmirxrd(REG_RX_AVI_DB3)&0x70)>>4);	// 000=xvYUV601, 001=xvYUV709, 010=sYUV601 ....
    iTE6805_DATA.AVIInfoFrame_RGBQuantizationRange = ((hdmirxrd(REG_RX_AVI_DB3)&0x0C)>>2);	// 00 = Depend on Video Format, 01 = Limit, 10 = Full
    iTE6805_DATA.AVIInfoFrame_VIC = ((hdmirxrd(REG_RX_AVI_DB4)&0x7F));
    iTE6805_DATA.AVIInfoFrame_YUVQuantizationRange = ((hdmirxrd(REG_RX_AVI_DB5)&0xC0)>>6);	// 00 = Limit Range, 01 = Full Range
    chgbank(0);

    if(iTE6805_DATA.AVIInfoFrame_RGBQuantizationRange == RGB_RANGE_DEPENDED_ON_VIC )
    {
		if( iTE6805_DATA.AVIInfoFrame_VIC >=2 )	iTE6805_DATA.AVIInfoFrame_RGBQuantizationRange = RGB_RANGE_LIMIT; // CE Mode
		else iTE6805_DATA.AVIInfoFrame_RGBQuantizationRange = RGB_RANGE_FULL; // IT mode
    }

}
// Block Get End


// Block Reset
//hdmirx_ECCTimingOut
void iTE6805_Reset_ALL_Logic(iTE_u8 PORT_NUM)
{
	// REG23	Reg_P0_SWRst		0	1: Port 0 all logic reset	R/W	0
	//			Reg_P0_HDCPRst		1	1: Port 0 HDCP logic reset	R/W	0
	//			Reg_P0_CDRDataRst	2	1: Port 0 CDR Data logic reset	R/W	0
	//			Reg_P0_DCLKRst		3	1: Port 0 CLKD10 domain logic reset	R/W	0


	HDMIRX_DEBUG_PRINT(("******* iTE6805_Reset_ALL_Logic *******\n"));
    if(PORT_NUM == 0)
	{
		iTE6805_Set_HPD_Ctrl(PORT0,HPD_LOW);

		chgbank(0);
		hdmirxwr(0x08, 0x04); // port0
		hdmirxwr(0x22, 0x12);
		hdmirxwr(0x22, 0x10);
		hdmirxwr(0x23, 0xAC);
		hdmirxwr(0x23, 0xA0);

		// Auto Reset when B0
		if(iTE6805_DATA.ChipID == 0xA0 || iTE6805_DATA.ChipID == 0xA1)
		{
			hdmirxwr(0xC5, 0x12); // Unplug need to reset EDID, because EDID reset is also reset SCDC in hardware, or SCDC may hang in sometimes
			iTE6805_delay1ms(1);
			hdmirxwr(0xC5, 0x02); // Unplug need to reset EDID, because EDID reset is also reset SCDC in hardware, or SCDC may hang in sometimes
		}

		iTE6805_INT_5VPWR_Chg(PORT0);
    }else{
        iTE6805_Set_HPD_Ctrl(PORT1,HPD_LOW);

		chgbank(0);
		hdmirxwr(0x0D, 0x04); // port1
		hdmirxwr(0x22, 0x12);
		hdmirxwr(0x22, 0x10);
		hdmirxwr(0x2B, 0xAC);
		hdmirxwr(0x2B, 0xA0);

		// Auto Reset when B0
		if(iTE6805_DATA.ChipID == 0xA0 || iTE6805_DATA.ChipID == 0xA1)
		{
			chgbank(4);
			hdmirxwr(0xC5, 0x12); // Unplug need to reset EDID, because EDID reset is also reset SCDC in hardware, or SCDC may hang in sometimes
			iTE6805_delay1ms(1);
			hdmirxwr(0xC5, 0x02); // Unplug need to reset EDID, because EDID reset is also reset SCDC in hardware, or SCDC may hang in sometimes
			chgbank(0);
		}
		iTE6805_INT_5VPWR_Chg(PORT1);
    }
	chgbank(0);
}

void iTE6805_Reset_Video_Logic()
{
	chgbank(0);
	hdmirxset(0x22, BIT0, BIT0);
	iTE6805_delay1ms(1);
	hdmirxset(0x22, BIT0, 0x00);
	hdmirxset(0x10, BIT1, BIT1); // clear vidstable change INT
	hdmirxset(0x12, BIT7, BIT7); // clear vidstable change INT
}
void iTE6805_Reset_Audio_Logic()
{
	iTE_u8 temp;
	chgbank(0);
	// ALL MARK FROM 6802
	// hdmirxset(0x81,0x0c,0x0c);	// enable  Mute i2s and ws and s/pdif
	// iTE6805_delay1ms(100);
	// hdmirxset(0x81,0x0c,0x00);	// disable Mute i2s and ws and s/pdif

	hdmirxset(0x22, BIT1, BIT1); // audio reset
	//iTE6805_delay1ms(1000); // HW ADD , SW MARK
	hdmirxset(0x22, BIT1, 0x00);
	// RegFS_Set[5:0] : Software set sampling frequency R/W
	temp = hdmirxrd(0x8A);
	hdmirxwr(0x8A, temp);
	hdmirxwr(0x8A, temp);
	hdmirxwr(0x8A, temp);
	hdmirxwr(0x8A, temp);
}
void iTE6805_Reset_Video_FIFO()
{
	// from 6802 can't find in 6805
	chgbank(1);
	hdmirxset(0x64, 0x80, 0x80); 		// reset Video FIFO's pointer
	hdmirxset(0x64, 0x80, 0x00);
	chgbank(0);
}


// Block Reset End

// Block Check
iTE_u8 iTE6805_Check_PORT0_IS_MHL_Mode(iTE_u8 PORT_NUM)
{
	chgbank(0);
	//HDMIRX_DEBUG_PRINT(("\n\niTE6805_Check_PORT0_IS_MHL_Mode = MODE_HDMI hdmirxrd(0x13)=0x%02X !!!\n",(int) hdmirxrd(0x13)));
	if(PORT_NUM == PORT0)
	{
		return hdmirxrd(0x13)&BIT6;
	}
	else
	{
		return MODE_HDMI;
	}

}

iTE_u8 iTE6805_Check_HDMI_OR_DVI_Mode(iTE_u8 PORT_NUM)
{
	chgbank(0);
	if((PORT_NUM == PORT0 && (hdmirxrd(0x13)&BIT1))||
	   (PORT_NUM == PORT1 && (hdmirxrd(0x16)&BIT1)))
	{
		return MODE_HDMI;
	}
	else
	{
		return MODE_DVI;
	}
}

iTE_u8 iTE6805_Check_CLK_Vaild()
{
	chgbank(0);
	if (iTE6805_DATA.CurrentPort == PORT0)
	{
		return hdmirxrd(0x13)&BIT3;
	}
	else
	{
		return hdmirxrd(0x16)&BIT3;
	}
}

iTE_u8 iTE6805_Check_SCDT()
{
	chgbank(0);
	return hdmirxrd(0x19)&BIT7;
}

iTE_u8 iTE6805_Check_AUDT()
{
	// REG10[7] : Audio FIFO Error Interrupt
	// REGB1[7] : Audio is On Flag
	chgbank(0);
	return !((hdmirxrd(0x10)&0x80)>>7) & ((hdmirxrd(0xB1)&0x80)>>7);
}
iTE_u8 iTE6805_Check_AVMute()
{
	iTE_u8 temp;
	// REG 0AA[3] Port 0 is in AVMute state
	// REG 4AA[3] Port 1 is in AVMute state
	if(iTE6805_DATA.CurrentPort == PORT0)
	{
		chgbank(0);
		return hdmirxrd(0xAA)&BIT3;
	}
	else
	{
		chgbank(4);
		temp = hdmirxrd(0xAA)&BIT3;
		chgbank(0);
		return temp;
	}
}

iTE_u8 iTE6805_Check_5V_State(iTE_u8 PORT_NUM)
{
    if(PORT_NUM==0)
	{
		return hdmirxrd(0x13)&BIT0;
    }
	else
	{
		// can't detect port1 HPD , need using GPID rework for detect HPD signal
		if(iTE6805_DATA.ChipID == 0xA0)
			//return gpPORT1_5V_STATE;

		return hdmirxrd(0x16)&BIT0;
	}
}

iTE_u8 iTE6805_Check_Single_Dual_Mode(){
	chgbank(1);
	if(hdmirxrd(0xC0)&BIT0)
	{
		chgbank(0);
		return MODE_DUAL;
	}
	else
	{
		chgbank(0);
		return MODE_SINGLE;
	}
}
iTE_u8 iTE6805_Check_4K_Resolution()
{

	iTE_u32 rddata,i;
	iTE_u32 PCLK, sump;
	sump = 0;
	chgbank(0);
	for (i = 0; i<5; i++)
	{
		iTE6805_delay1ms(3);
		rddata = ((iTE_u32)(hdmirxrd(0x9A) & 0x03) << 8) + hdmirxrd(0x99);
		sump += rddata;
	}
	PCLK = RCLKVALUE * 512 * 5 / sump; // 512=2*256 because of 1T 2 pixel
	iTE6805_CurVTiming.PCLK = PCLK;

	HDMIRX_DEBUG_PRINT(("PCLK = %ld.%03ldMHz\n", iTE6805_CurVTiming.PCLK/1000, iTE6805_CurVTiming.PCLK%1000));
	if(iTE6805_CurVTiming.PCLK > 320000 ||
	  (iTE6805_CurVTiming.PCLK > 160000 && iTE6805_DATA.AVIInfoFrame_Input_ColorFormat == Color_Format_YUV420)
	  )
	{
		HDMIRX_DEBUG_PRINT(("iTE6805_Check_4K_Resolution = TRUE!!!\n"));
		return TRUE;
	}
	HDMIRX_DEBUG_PRINT(("iTE6805_Check_4K_Resolution = FALSE!!!\n"));
	return FALSE;
}

iTE_u8 iTE6805_Check_HDMI2()
{
	// using clock ratio 1/10 or 1/40 judge is HDMI2.0 or not
	// ADD BIT7 for really HDMI2.0
	iTE_u8 temp ;
	chgbank(0);

	if(iTE6805_DATA.CurrentPort == PORT0)
	{
		// Edit judge the hdmi2.x from 0xC0 to BIT6 when present 6805 EQ flow
		temp = hdmirxrd(0x14)&BIT6;
		if(temp == BIT6)
		{
			return TRUE;
		}else{
			return FALSE;
		}

	}
	else if(iTE6805_DATA.CurrentPort == PORT1)
	{
		temp = hdmirxrd(0x17)&BIT6;
		if(temp == BIT6)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}

iTE_u8 iTE6805_Check_EQ_Busy()
{
	// using Dbg_AutoStat judge EQ is on trigger now or not (0 mean not busy)
	iTE_u8 EQ_Busy = 0;
	if(iTE6805_DATA.CurrentPort == PORT0) chgbank(3);
	if(iTE6805_DATA.CurrentPort == PORT1) chgbank(7);
	EQ_Busy = hdmirxrd(0xD4);
	chgbank(0);
	return EQ_Busy;
}

iTE_u8 iTE6805_Check_TMDS_Bigger_Than_1G()
{
	chgbank(0);
	if(iTE6805_DATA.CurrentPort == PORT0)
	{
		return hdmirxrd(0x14)&BIT0;
	}
	else if(iTE6805_DATA.CurrentPort == PORT1)
	{
		return hdmirxrd(0x17)&BIT0;
	}

}

iTE_u8 iTE6805_Check_Scramble_State()
{
	chgbank(0);
	if(iTE6805_DATA.CurrentPort == PORT0)
	{
		return hdmirxrd(0x14)&BIT7;
	}
	else if(iTE6805_DATA.CurrentPort == PORT1)
	{
		return hdmirxrd(0x17)&BIT7;
	}
}

// Block Check End


//Hot Plug Detection
void iTE6805_Set_HPD_Ctrl(iTE_u16 PORT_NUM, iTE_u16 HPD_State){

	// 5V DETECT OFF
	if (PORT_NUM == PORT0 && iTE6805_Check_5V_State(PORT0) == MODE_5V_OFF)
	{
		chgbank(3);
		hdmirxset(0xAB, 0xC0, 0x00); //SET PORT0 Tri-State
		chgbank(0);
		return;
	}

	// 5V DETECT ON + HDMI MODE
	if (PORT_NUM == PORT0 && iTE6805_Check_PORT0_IS_MHL_Mode(PORT0) == MODE_HDMI)
	{
		chgbank(3);
		if (HPD_State == HPD_LOW)
			
		{
			hdmirxset(0xAB, 0xC0, 0xC0); // SET PORT0 HPD HIGH
			//hdmirxset(0xAB, 0xC0, 0x40); // SET PORT0 HPD LOW

		}
		else
		{
			hdmirxset(0xAB, 0xC0, 0x40); // SET PORT0 HPD LOW
			//hdmirxset(0xAB, 0xC0, 0xC0); // SET PORT0 HPD HIGH
		}
		hdmirxset(0xAC, 0x7C, 0x40); // Reg_P0_ForceCBUS=1
		chgbank(0);
	}

	// 5V DETECT ON + MHL MODE
	#ifdef _ENABLE_IT6805_MHL_FUNCTION_
	if (PORT_NUM == PORT0 && iTE6805_Check_PORT0_IS_MHL_Mode(PORT0) == MODE_MHL)
	{

		if(iTE6805_DATA.MHL_DiscoveryDone == 0)
		{
			chgbank(3);
			hdmirxset(0xAB, 0xC0, 0x00);
			hdmirxset(0xAC, 0x7C, 0x00); // Reg_P0_ForceCBUS=0
			chgbank(0);
			return;
		}

		HDMIRX_DEBUG_PRINT(("MHL HPD Setting\n"));
		if (HPD_State == HPD_LOW)
		{
			while(iTE6805_MHL_MSC_Fire(MSC_PKT_CLR_HPD)!=MSC_SUCCESS);
			HDMIRX_DEBUG_PRINT(("MHL_LOW\n"));
		}
		else
		{
			while(iTE6805_MHL_MSC_Fire(MSC_PKT_SET_HPD)!=MSC_SUCCESS);
			HDMIRX_DEBUG_PRINT(("MHL_HIGH\n"));
		}
	}
	#endif
	chgbank(0);

}

//IT6802VideoOutputEnable
void iTE6805_Set_Video_Tristate(iTE_u8 TRISTATE_STATE) // TRISTATE_ON, TRISTATE_OFF
{

	//  6805 no RegTriVDIO 3:1 1: enable tri-state Video IO
    if(TRISTATE_STATE == TRISTATE_OFF)
	{
		chgbank(1);
		hdmirxset(0xC5, 0x80, 0x00);
		hdmirxset(0xC6, 0x80, 0x00);
		if(iTE6805_Check_Single_Dual_Mode() == MODE_SINGLE)
		{
			chgbank(1);
			switch(hdmirxrd(0xC0)&0xC0)
			{
				case 0x00:
				case 0x40:
// setting Tristate IO by IO/Mode & Color Depth
#if (Output_Color_Depth == 8)
					hdmirxwr(0xC5, 0x78); // turn off tristate If IO mode do not need it, only for single mode
#elif (Output_Color_Depth == 10)
					hdmirxwr(0xC5, 0x38); // turn off tristate If IO mode do not need it, only for single mode
#elif (Output_Color_Depth == 12)
					hdmirxwr(0xC5, 0x18); // turn off tristate If IO mode do not need it, only for single mode
#endif
					break;
				case 0x80:

				// setting Tristate IO by IO/Mode & Color Depth
#if (Output_Color_Depth == 8)
					hdmirxwr(0xC5, 0x63); // turn off tristate If IO mode do not need it, only for single mode
#elif (Output_Color_Depth == 10)
					hdmirxwr(0xC5, 0x23); // turn off tristate If IO mode do not need it, only for single mode
#elif (Output_Color_Depth == 12)
					hdmirxwr(0xC5, 0x03); // turn off tristate If IO mode do not need it, only for single mode
#endif
					break;
			}
			//hdmirxwr(0xC5, 0x00);
		}
		else
		{
			chgbank(1);

#if (Output_Color_Depth == 8)
			hdmirxwr(0xC6, 0x40); // turn off tristate If IO mode do not need it, only for single mode
#elif (Output_Color_Depth == 10 || Output_Color_Depth == 12)
			hdmirxwr(0xC6, 0x00); // turn off tristate If IO mode do not need it, only for single mode
#endif
		}
        HDMIRX_VIDEO_PRINTF(("---------------- iTE6805_Set_Video_Tristate -> VIDEO_ON = Tristate off\n"));
    }
    else{
		chgbank(1);
		hdmirxwr(0xC5, 0xFF);
		hdmirxwr(0xC6, 0xFF);
        HDMIRX_VIDEO_PRINTF(("---------------- iTE6805_Set_Video_Tristate -> VIDEO_OFF = Tristate on\n"));
	}
	chgbank(0);
}

void iTE6805_Set_Audio_Tristate(iTE_u8 TRISTATE_STATE)
{
	chgbank(1);
	if(TRISTATE_STATE == TRISTATE_OFF)
	{
		hdmirxwr(0xC7, 0x00); // SPDIF/I2S tri-state off
	}
	else
	{
        hdmirxwr(0xC7, 0x7F); // SPDIF/I2S tri-state on
	}
	chgbank(0);
}



// ***************************************************************************
// VIC Related Function
// ***************************************************************************
/*
iTE_u16 iTE6805_Get_VIC_Number()
{
	iTE_u16 i;
	for (i = 0; i<(SizeofVMTable - 1); i++) {
		if ((abs(iTE6805_CurVTiming.PCLK - s_VMTable[i].PCLK)>(s_VMTable[i].PCLK * 5 / 100)) ||
			(iTE6805_CurVTiming.HActive != s_VMTable[i].HActive) ||
			//         (iTE6805_CurVTiming.HBackPorch != s_VMTable[i].HBackPorch ) ||
			//         (iTE6805_CurVTiming.HFrontPorch != s_VMTable[i].HFrontPorch ) ||
			//         (iTE6805_CurVTiming.HSyncWidth != s_VMTable[i].HSyncWidth ) ||
			(iTE6805_CurVTiming.HTotal != s_VMTable[i].HTotal) ||
			//         (iTE6805_CurVTiming.HPolarity != s_VMTable[i].HPolarity ) ||
			(iTE6805_CurVTiming.VActive != s_VMTable[i].VActive) ||
			//         (iTE6805_CurVTiming.VBackPorch != s_VMTable[i].VBackPorch ) ||
			//         (iTE6805_CurVTiming.VFrontPorch != s_VMTable[i].VFrontPorch ) ||
			//         (iTE6805_CurVTiming.VSyncWidth != s_VMTable[i].VSyncWidth ) ||
			(iTE6805_CurVTiming.VTotal != s_VMTable[i].VTotal))
			//         (iTE6805_CurVTiming.VPolarity != s_VMTable[i].VPolarity ) ||
			//         (iTE6805_CurVTiming.ScanMode != s_VMTable[i].ScanMode ) )
			continue;
		else
			break;
	}
	return i;
}
*/

void iTE6805_Get_VID_Info(void)
{
	iTE_u32 rddata;

	iTE_u16 HSyncPol, VSyncPol, InterLaced;
	iTE_u16 HTotal, HActive, HFP, HSYNCW;
	iTE_u16 VTotal, VActive, VFP, VSYNCW;
	iTE_u16 i;

	iTE_u32 PCLK, sump;
	iTE_u32 TMDSCLK, sumt;
	iTE_u32 FrameRate;

	// 0x43 bit5 : P0_WDog RxCLK PreDiv2
	// 0x43 bit6 : P0_WDog RxCLK PreDiv4
	// 0x43 bit7 : P0_WDog RxCLK PreDiv8
	// 0x443 bit5 : P1_WDog RxCLK PreDiv2
	// 0x443 bit6 : P1_WDog RxCLK PreDiv4
	// 0x443 bit7 : P1_WDog RxCLK PreDiv8
	// 0x48	 P0_TMDSCLKSpeed	7:0	TMDSCLK = RCLKVALUE*256/ TMDSCLKSpeed
	// 0x448 P1_TMDSCLKSpeed	7:0	TMDSCLK = RCLKVALUE*256/ TMDSCLKSpeed

	if (iTE6805_DATA.CurrentPort == PORT0)
	{
		chgbank(0);
	}
	else
	{
		chgbank(4);
	}
	sumt = 0;
	for (i = 0; i<10; i++){
		iTE6805_delay1ms(10);
		rddata = hdmirxrd(0x48) + 1;
		VIDEOTIMNG_DEBUG_PRINTF(("0x48 TMDSCLKSpeed = 0x%02x \n",(int) rddata));
		sumt += rddata;
	}
	VIDEOTIMNG_DEBUG_PRINTF(("sumt = %ld \n", sumt));
	rddata = hdmirxrd(0x43) & 0xE0;
	chgbank(0);

	HDMIRX_DEBUG_PRINT(("RCLKVALUE=%ld.%02ldMHz\n", RCLKVALUE / 1000, (RCLKVALUE % 1000) / 10));
	HDMIRX_DEBUG_PRINT(("rddata=%d \n", (int) rddata));

	if (rddata&BIT7)
		TMDSCLK = (RCLKVALUE * (iTE_u32)10240) / sumt;
	else if (rddata&BIT6)
		TMDSCLK = (RCLKVALUE * (iTE_u32)5120) / sumt;
	else if (rddata&BIT5)
		TMDSCLK = (RCLKVALUE * (iTE_u32)2560) / sumt;
	if (rddata == 0x00)
		TMDSCLK = (RCLKVALUE * (iTE_u32)1280) / (sumt);


	if(RCLKFreqSel==1 ){TMDSCLK = TMDSCLK*2;}
	else if(RCLKFreqSel==2 ){TMDSCLK = TMDSCLK*4;}
	else if(RCLKFreqSel==3 ){TMDSCLK = TMDSCLK*8;}


	sump = 0;
	for (i = 0; i<10; i++) {
		iTE6805_delay1ms(1);
		rddata = ((iTE_u32)(hdmirxrd(0x9A) & 0x03) << 8) + hdmirxrd(0x99);
		sump += rddata;
	}

	PCLK = RCLKVALUE * 512 * 10 / sump; // 512=2*256 because of 1T 2 pixel

	InterLaced = (hdmirxrd(0x98) & 0x02) >> 1;
	HTotal = ((hdmirxrd(0x9C) & 0x3F) << 8) + hdmirxrd(0x9B);
	HActive = ((hdmirxrd(0x9E) & 0x3F) << 8) + hdmirxrd(0x9D);
	HFP = ((hdmirxrd(0xA1) & 0xF0) << 4) + hdmirxrd(0xA0);
	HSYNCW = ((hdmirxrd(0xA1) & 0x01) << 8) + hdmirxrd(0x9F);


	VTotal = ((hdmirxrd(0xA3) & 0x3F) << 8) + hdmirxrd(0xA2);
	VActive = ((hdmirxrd(0xA5) & 0x3F) << 8) + hdmirxrd(0xA4);
	VFP = ((hdmirxrd(0xA8) & 0xF0) << 4) + hdmirxrd(0xA7);
	VSYNCW = ((hdmirxrd(0xA8) & 0x01) << 8) + hdmirxrd(0xA6);

	if (iTE6805_DATA.CurrentPort == PORT1)
	{
		chgbank(4);
		HDMIRX_VIDEO_PRINTF(("hdmirxrd(0xAA) = 0x%02X \n",(int) hdmirxrd(0xAA)));

	}

	HSyncPol = (hdmirxrd(0xAA)&BIT5) >> 5;
	VSyncPol = (hdmirxrd(0xAA)&BIT6) >> 6;
	chgbank(0);

	iTE6805_CurVTiming.TMDSCLK = TMDSCLK;
	iTE6805_CurVTiming.PCLK = PCLK;
	iTE6805_CurVTiming.HActive = HActive;
	iTE6805_CurVTiming.HTotal = HTotal;
	iTE6805_CurVTiming.HFrontPorch = HFP;
	iTE6805_CurVTiming.HSyncWidth = HSYNCW;
	iTE6805_CurVTiming.HBackPorch = (HTotal - HActive - HFP - HSYNCW);
	iTE6805_CurVTiming.VActive = VActive;
	iTE6805_CurVTiming.VTotal = VTotal;
	iTE6805_CurVTiming.VFrontPorch = VFP;
	iTE6805_CurVTiming.VSyncWidth = VSYNCW;
	iTE6805_CurVTiming.VBackPorch = VTotal - VActive - VFP - VSYNCW;
	iTE6805_CurVTiming.ScanMode = (InterLaced)& 0x01;
	iTE6805_CurVTiming.VPolarity = (VSyncPol)& 0x01;
	iTE6805_CurVTiming.HPolarity = (HSyncPol)& 0x01;

	FrameRate = (iTE_u32)(iTE6805_CurVTiming.PCLK) * 1000;
	FrameRate /= iTE6805_CurVTiming.HTotal;
	FrameRate /= iTE6805_CurVTiming.VTotal;
	iTE6805_CurVTiming.FrameRate = FrameRate;

}

void iTE6805_Show_VID_Info()
{
    int voutstb, lvpclk_hs;

	chgbank(0);

	if(hdmirxrd(0x15)&0x02)
		VIDEOTIMNG_DEBUG_PRINTF(("HDMI2 Scramble Enable !! \n"));
	else VIDEOTIMNG_DEBUG_PRINTF(("HDMI2 Scramble Disable !! \n"));
	if(hdmirxrd(0x14)&0x40)
		VIDEOTIMNG_DEBUG_PRINTF(("HDMI2 CLK Ratio 1/40 !! \n"));
	else VIDEOTIMNG_DEBUG_PRINTF(("HDMI CLK Ratio 1/10  !! \n"));

    voutstb   = ((hdmirxrd(0x19)&0x02)>>1);
	lvpclk_hs = ((hdmirxrd(0x36)&0x0C)>>2);

    //VIDEOTIMNG_DEBUG_PRINTF("Video Input Timing: %s\n", s_VMTable[iTE6805_CurVTiming.VIC].format); // mark because all @#&@)!!$@! can't read ..
    VIDEOTIMNG_DEBUG_PRINTF(("TMDSCLK = %lu.%03luMHz\n", iTE6805_CurVTiming.TMDSCLK/1000, iTE6805_CurVTiming.TMDSCLK%1000));
    VIDEOTIMNG_DEBUG_PRINTF(("PCLK = %ld.%03ldMHz\n", iTE6805_CurVTiming.PCLK/1000, iTE6805_CurVTiming.PCLK%1000));
	VIDEOTIMNG_DEBUG_PRINTF(("HActive = %d\n", iTE6805_CurVTiming.HActive));
    VIDEOTIMNG_DEBUG_PRINTF(("VActive = %d\n", iTE6805_CurVTiming.VActive));
    VIDEOTIMNG_DEBUG_PRINTF(("HTotal = %d\n", iTE6805_CurVTiming.HTotal));
	VIDEOTIMNG_DEBUG_PRINTF(("VTotal = %d\n", iTE6805_CurVTiming.VTotal));
	VIDEOTIMNG_DEBUG_PRINTF(("HFrontPorch = %d\n", iTE6805_CurVTiming.HFrontPorch));
	VIDEOTIMNG_DEBUG_PRINTF(("HSyncWidth = %d\n", iTE6805_CurVTiming.HSyncWidth));
    VIDEOTIMNG_DEBUG_PRINTF(("HBackPorch = %d\n", iTE6805_CurVTiming.HBackPorch));
    VIDEOTIMNG_DEBUG_PRINTF(("VFrontPorch = %d\n", iTE6805_CurVTiming.VFrontPorch));
    VIDEOTIMNG_DEBUG_PRINTF(("VSyncWidth = %d\n", iTE6805_CurVTiming.VSyncWidth));
    VIDEOTIMNG_DEBUG_PRINTF(("VBackPorch = %d\n", iTE6805_CurVTiming.VBackPorch));
	VIDEOTIMNG_DEBUG_PRINTF(("FrameRate = %ld\n", iTE6805_CurVTiming.FrameRate ));

	if( iTE6805_CurVTiming.ScanMode==0 )
	{
		VIDEOTIMNG_DEBUG_PRINTF(("ScanMode = Progressive\n"));
	}
	else
	{
		VIDEOTIMNG_DEBUG_PRINTF(("ScanMode = InterLaced\n"));
	}

	if( iTE6805_CurVTiming.VPolarity==1 )
	{
		VIDEOTIMNG_DEBUG_PRINTF(("VSyncPol = Positive\n"));
	}
	else
	{
		VIDEOTIMNG_DEBUG_PRINTF(("VSyncPol = Negative\n"));
	}

	if( iTE6805_CurVTiming.HPolarity==1 )
	{
		VIDEOTIMNG_DEBUG_PRINTF(("HSyncPol = Positive\n"));
	}
	else
	{
		VIDEOTIMNG_DEBUG_PRINTF(("HSyncPol = Negative\n"));
	}

    if( voutstb==1 )
	{
		VIDEOTIMNG_DEBUG_PRINTF(("Video Output Detect Stable   LVPCLK_HS=%d\n", lvpclk_hs));
	}
	else
	{
		VIDEOTIMNG_DEBUG_PRINTF(("Video Output Detect Non-Stable   LVPCLK_HS=%d\n", lvpclk_hs));
	}
}

void iTE6805_Show_AUD_Info(void)
{
	iTE_u8 Aud3DOn, MultAudOn, DSDAud, DSDFs, HBRAud, AudFmt, AudCh;
	//iTE_u32 N, CTS;

	Aud3DOn = (hdmirxrd(0xB0) & 0x10) >> 4;
	MultAudOn = (hdmirxrd(0xB0) & 0x20) >> 5;
	HBRAud = (hdmirxrd(0xB0) & 0x40) >> 6;
	DSDAud = (hdmirxrd(0xB0) & 0x80) >> 7;
	AudCh = (hdmirxrd(0xB1) & 0x3F);
	AudFmt = ((hdmirxrd(0xB5) & 0xC0) >> 2) + (hdmirxrd(0xB5) & 0x0F);

	chgbank(2);
	//N = ((iTE_u32)hdmirxrd(0xBE)) << 12;
	//N += ((iTE_u32)hdmirxrd(0xBF)) << 4;
	//N += ((iTE_u32)hdmirxrd(0xC0)) & 0x0F;

	//CTS = ((hdmirxrd(0xC0) & 0xF0)) >> 4;
	//CTS += ((iTE_u32)hdmirxrd(0xC2)) << 4;
	//CTS += ((iTE_u32)hdmirxrd(0xC1)) << 12;
	DSDFs = (hdmirxrd(0x46) & 0x1C) >> 2;
	chgbank(0);

	HDMIRX_AUDIO_PRINTF(("\nAudio Format: "));

	if (Aud3DOn == 1) {
		if (DSDAud == 1) {
			HDMIRX_AUDIO_PRINTF(("\n One Bit 3D Audio \n"));
		}
		else {
			HDMIRX_AUDIO_PRINTF(("\n 3D Audio \n"));
		}
	}
	else if (MultAudOn == 1) {
		if (DSDAud == 1) {
			HDMIRX_AUDIO_PRINTF(("\n One Bit Multi-Stream Audio \n"));
		}
		else {
			HDMIRX_AUDIO_PRINTF(("\n Multi-Stream Audio \n"));
		}
	}
	else {
		if (DSDAud == 1) {
			HDMIRX_AUDIO_PRINTF(("\n One Bit Audio \n"));
		}
		else {
			HDMIRX_AUDIO_PRINTF(("\n Audio Sample \n"));
		}
	}

	if (HBRAud == 1)
	{
		HDMIRX_AUDIO_PRINTF(("\n High BitRate Audio \n"));
	}

	if (HBRAud == 1)
	{
		HDMIRX_AUDIO_PRINTF(("\n AudCh=2Ch "));
	}
	else
	{
		switch (AudCh) {
		case 1:
			HDMIRX_AUDIO_PRINTF(("\nAudCh=2Ch "));
			break;
		case 3:
			HDMIRX_AUDIO_PRINTF(("\nAudCh=4Ch "));
			break;
		case 7:
			HDMIRX_AUDIO_PRINTF(("\nAudCh=6Ch "));
			break;
		case 15:
			HDMIRX_AUDIO_PRINTF(("\nAudCh=8Ch "));
			break;
		case 31:
			HDMIRX_AUDIO_PRINTF(("\nAudCh=10Ch "));
			break;
		case 63:
			HDMIRX_AUDIO_PRINTF(("\nAudCh=12Ch "));
			break;
		default:
			HDMIRX_AUDIO_PRINTF(("\nAudCh Error !!!\n"));
			break;
		}
	}

	if (DSDAud == 1)
	{
		HDMIRX_AUDIO_PRINTF(("AudFs="));
		switch (DSDFs) {
		case 0:
			HDMIRX_AUDIO_PRINTF(("Refer to Stream Header\n"));
			break;
		case 1:
			HDMIRX_AUDIO_PRINTF(("32K\n"));
			break;
		case 2:
			HDMIRX_AUDIO_PRINTF(("44.1K\n"));
			break;
		case 3:
			HDMIRX_AUDIO_PRINTF(("48K\n"));
			break;
		case 4:
			HDMIRX_AUDIO_PRINTF(("88.2K\n"));
			break;
		case 5:
			HDMIRX_AUDIO_PRINTF(("96K\n"));
			break;
		case 6:
			HDMIRX_AUDIO_PRINTF(("176.4K\n"));
			break;
		case 7:
			HDMIRX_AUDIO_PRINTF(("192K\n"));
			break;
		default:
			HDMIRX_AUDIO_PRINTF(("Error !!!\n"));
		}
	}
	else {
		HDMIRX_AUDIO_PRINTF(("AudFs="));
		switch (AudFmt) {
		case 3:
			HDMIRX_AUDIO_PRINTF(("=32K\n"));
			break;
		case 0:
			HDMIRX_AUDIO_PRINTF(("=44.1K\n"));
			break;
		case 2:
			HDMIRX_AUDIO_PRINTF(("=48K\n"));
			break;
		case 11:
			HDMIRX_AUDIO_PRINTF(("=64K\n"));
			break;
		case 8:
			HDMIRX_AUDIO_PRINTF(("=88.2K\n"));
			break;
		case 10:
			HDMIRX_AUDIO_PRINTF(("=96K\n"));
			break;
		case 43:
			HDMIRX_AUDIO_PRINTF(("=128K\n"));
			break;
		case 12:
			HDMIRX_AUDIO_PRINTF(("=176.4K\n"));
			break;
		case 14:
			HDMIRX_AUDIO_PRINTF(("=192K\n"));
			break;
		case 27:
			HDMIRX_AUDIO_PRINTF(("=256K\n"));
			break;
		case 13:
			HDMIRX_AUDIO_PRINTF(("=352.8K\n"));
			break;
		case 5:
			HDMIRX_AUDIO_PRINTF(("=384K\n"));
			break;
		case 59:
			HDMIRX_AUDIO_PRINTF(("=512K\n"));
			break;
		case 45:
			HDMIRX_AUDIO_PRINTF(("=705.6K\n"));
			break;
		case 9:
			HDMIRX_AUDIO_PRINTF(("=768K\n"));
			break;
		case 53:
			HDMIRX_AUDIO_PRINTF(("=1024K\n"));
			break;
		case 29:
			HDMIRX_AUDIO_PRINTF(("=1411.2K\n"));
			break;
		case 21:
			HDMIRX_AUDIO_PRINTF(("=1536K\n"));
			break;
		default:
			HDMIRX_AUDIO_PRINTF((" Error !!!\n"));
		}
	}

	//HDMIRX_AUDIO_PRINTF(("N value = %ld , ", N));
	//HDMIRX_AUDIO_PRINTF(("CTS value = %ld\n\n", CTS));
}

void iTE6805_Show_AVIInfoFrame_Info(void)
{
	VIDEOTIMNG_DEBUG_PRINTF(("AVI Info Frame VIC = %d\n",(int) iTE6805_DATA.AVIInfoFrame_VIC));
	VIDEOTIMNG_DEBUG_PRINTF(("AVI Info Frame Input ColorFormat = %X ",(int) iTE6805_DATA.AVIInfoFrame_Input_ColorFormat));
	switch(iTE6805_DATA.AVIInfoFrame_Input_ColorFormat){
		case Color_Format_RGB:
			VIDEOTIMNG_DEBUG_PRINTF(("- RGB\n"));
			break;
		case Color_Format_YUV422:
			VIDEOTIMNG_DEBUG_PRINTF(("- YUV422\n"));
			break;
		case Color_Format_YUV444:
			VIDEOTIMNG_DEBUG_PRINTF(("- YUV444\n"));
			break;
		case Color_Format_YUV420:
			VIDEOTIMNG_DEBUG_PRINTF(("- YUV420\n"));
			break;
		default :
			break;
	}

	VIDEOTIMNG_DEBUG_PRINTF(("AVI Info Frame Colorimetry = %X",(int) iTE6805_DATA.AVIInfoFrame_Colorimetry));
	switch(iTE6805_DATA.AVIInfoFrame_Colorimetry){
		case Colormetry_ITU601:
			VIDEOTIMNG_DEBUG_PRINTF((" - ITU601\n"));
			break;
		case Colormetry_ITU709:
			VIDEOTIMNG_DEBUG_PRINTF((" - ITU709\n"));
			break;
		default :
			VIDEOTIMNG_DEBUG_PRINTF((" - UNKNOW\n"));
			break;
	}

	VIDEOTIMNG_DEBUG_PRINTF(("AVI Info Frame RGBQuantizationRange = %X",(int) iTE6805_DATA.AVIInfoFrame_RGBQuantizationRange));
	switch(iTE6805_DATA.AVIInfoFrame_RGBQuantizationRange){
		case RGB_RANGE_DEPENDED_ON_VIC:
			VIDEOTIMNG_DEBUG_PRINTF((" - DEPENDED_ON_VIC\n"));
			break;
		case RGB_RANGE_LIMIT:
			VIDEOTIMNG_DEBUG_PRINTF((" - RGB_RANGE_LIMIT\n"));
			break;
		case RGB_RANGE_FULL:
			VIDEOTIMNG_DEBUG_PRINTF((" - RGB_RANGE_FULL\n"));
			break;
		default :
			VIDEOTIMNG_DEBUG_PRINTF((" - UNKNOW\n"));
			break;
	}

	VIDEOTIMNG_DEBUG_PRINTF(("AVI Info Frame YUVQuantizationRange = %X",(int) iTE6805_DATA.AVIInfoFrame_YUVQuantizationRange));
	switch(iTE6805_DATA.AVIInfoFrame_YUVQuantizationRange){
		case YUV_RANGE_LIMIT:
			VIDEOTIMNG_DEBUG_PRINTF((" - YUV_RANGE_LIMIT\n"));
			break;
		case YUV_RANGE_FULL:
			VIDEOTIMNG_DEBUG_PRINTF((" - YUV_RANGE_FULL\n"));
			break;
		default :
			VIDEOTIMNG_DEBUG_PRINTF((" - UNKNOW\n"));
			break;
	}

    //VIDEOTIMNG_DEBUG_PRINTF(("AVI Info Frame ExtendedColorimetry = %X\n",(int) iTE6805_DATA.AVIInfoFrame_ExtendedColorimetry));
}

void chgbank(iTE_u16 bank)
{
	hdmirxset(0x0F, 0x07, bank & 7);
}



void iTE6805_OCLK_Cal(void)
{
	iTE_u32 oscdiv, t1usint, t1usflt;
	iTE_u32 OSCCLK, OCLK, CPOCLK ;
	iTE_u32 rddata;
	iTE_u32 rddata2;
	iTE_u8	AA;

	mhlrxwr(0x01, 0x41); //Hold_Pin=0;
	iTE6805_delay1ms(99);
	mhlrxwr(0x01, 0x40); //Hold_Pin=1;
	rddata = mhlrxrd(0x12);
	rddata += ((iTE_u32)mhlrxrd(0x13)) << 8;
	rddata += ((iTE_u32)mhlrxrd(0x14)) << 16;

	OSCCLK = rddata / 100;
	HDMIRX_DEBUG_PRINT(("rddata = %ld, OSCCLK = %ld\n", rddata, OSCCLK));
	HDMIRX_DEBUG_PRINT(("OSCCLK=%ld.%03ldMHz\n", OSCCLK / 1000, OSCCLK % 1000));
	printf("----------------------------------------------\n");
/*

    CPOCLK = iTE6805_OCLK_Load() ;
    HDMIRX_DEBUG_PRINT(("CPOCLK=%ld.%03ldMHz\n", CPOCLK / 1000, CPOCLK % 1000));
	rddata2 = (48000 * 1000/CPOCLK) * 64;

	HDMIRX_DEBUG_PRINT(("rddata2 = %ld\n", rddata2));
	chgbank(3);
	#ifdef EVB_BOARD_FOR_RING_FREQ
	CPOCLK = OSCCLK;
	#endif
	AA = 0x0C;
	if(CPOCLK <  31984){
		AA = 0x1F;
	}else if(CPOCLK >= 31984 && CPOCLK <  32586){
		AA = 0x1E;
	}else if(CPOCLK >= 32586 && CPOCLK <  33155){
		AA = 0x1D;
	}else if(CPOCLK >= 33155 && CPOCLK <  33293){
		AA = 0x1C;
	}else if(CPOCLK >= 33293 && CPOCLK <  34274){
		AA = 0x1B;
	}else if(CPOCLK >= 34274 && CPOCLK <  35006){
		AA = 0x1A;
	}else if(CPOCLK >= 35006 && CPOCLK <  35709){
		AA = 0x19;
	}else if(CPOCLK >= 35709 && CPOCLK <  36119){
		AA = 0x18;
	}else if(CPOCLK >= 36119 && CPOCLK <  36890){
		AA = 0x17;
	}else if(CPOCLK >= 36890 && CPOCLK <  37798){
		AA = 0x16;
	}else if(CPOCLK >= 37798 && CPOCLK <  38682){
		AA = 0x15;
	}else if(CPOCLK >= 38682 && CPOCLK <  39437){
		AA = 0x14;
	}else if(CPOCLK >= 39437 && CPOCLK <  40434){
		AA = 0x13;
	}else if(CPOCLK >= 40434 && CPOCLK <  41618){
		AA = 0x12;
	}else if(CPOCLK >= 41618 && CPOCLK <  42778){
		AA = 0x11;
	}else if(CPOCLK >= 42778 && CPOCLK <  43545){
		AA = 0x10;
	}else if(CPOCLK >= 43545 && CPOCLK <  44858){
		AA = 0x0F;
	}else if(CPOCLK >= 44858 && CPOCLK <  46443){
		AA = 0x0E;
	}else if(CPOCLK >= 46443 && CPOCLK <  48024){
		AA = 0x0D;
	}else if(CPOCLK >= 48024 && CPOCLK <  49408){
		AA = 0x0C;
	}else if(CPOCLK >= 49408 && CPOCLK <  51270){
		AA = 0x0B;
	}else if(CPOCLK >= 51270 && CPOCLK <  53574){
		AA = 0x0A;
	}else if(CPOCLK >= 53574 && CPOCLK <  55916){
		AA = 0x09;
	}else if(CPOCLK >= 55916 && CPOCLK <  57284){
		AA = 0x08;
	}else if(CPOCLK >= 57284 && CPOCLK <  60101){
		AA = 0x07;
	}else if(CPOCLK >= 60101 && CPOCLK <  63649){
		AA = 0x06;
	}else if(CPOCLK >= 63649 && CPOCLK <  67405){
		AA = 0x05;
	}else if(CPOCLK >= 67405 && CPOCLK <  70839){
		AA = 0x04;
	}else if(CPOCLK >= 70839 && CPOCLK <  72851){
		AA = 0x03;
	}else if(CPOCLK >= 72851 && CPOCLK <  82503){
		AA = 0x02;
	}else if(CPOCLK >= 82503 && CPOCLK <  90053){
		AA = 0x01;
	}else if(CPOCLK >90053){
		AA = 0x00;
	}
	AA++;

	HDMIRX_DEBUG_PRINT(("AA = %02X\n",(int) AA));
	hdmirxset(0xAA, 0x1f ,AA);
	chgbank(0);

//new start
	mhlrxwr(0x01, 0x41); //Hold_Pin=0;
	iTE6805_delay1ms(99);
	mhlrxwr(0x01, 0x40); //Hold_Pin=1;
	rddata = mhlrxrd(0x12);
	rddata += ((iTE_u32)mhlrxrd(0x13)) << 8;
	rddata += ((iTE_u32)mhlrxrd(0x14)) << 16;

	OSCCLK = rddata / 100;
	HDMIRX_DEBUG_PRINT(("rddata = %ld, OSCCLK = %ld\n", rddata, OSCCLK));
	HDMIRX_DEBUG_PRINT(("OSCCLK=%ld.%03ldMHz\n", OSCCLK / 1000, OSCCLK % 1000));
//new end


	//oscdiv = OSCCLK / 1000 / 10;
	//if (((OSCCLK / 1000 / oscdiv) - 10)>(10 - (OSCCLK / 1000 / (oscdiv + 1))))
	//	oscdiv++;


	if(OSCCLK > 49000){
		oscdiv = 5;
	}else{
		oscdiv = 4;
	}
	HDMIRX_DEBUG_PRINT(("oscdif = %d\n", (int)oscdiv));

	//#define FixOCLKD4 TRUE emily suggest mark it
	//if (FixOCLKD4)
	//	oscdiv = 4;

	OCLK = OSCCLK / oscdiv;
	HDMIRX_DEBUG_PRINT(("OCLK=%ld.%03ldMHz\n", OCLK / 1000, OCLK % 1000));
	mhlrxset(0x01, 0x70, oscdiv << 4);

	if (RCLKFreqSel == 0)
		RCLKVALUE = OSCCLK / 2;   // 20 MHz
	else if (RCLKFreqSel == 1)
		RCLKVALUE = OSCCLK / 4;   // 10 MHz
	else if (RCLKFreqSel == 2)
		RCLKVALUE = OSCCLK / 8;
	else
		RCLKVALUE = OSCCLK / 16;

#ifdef _ENABLE_IT6805_CEC_
    CEC_timeunit = (iTE_u8)(RCLKVALUE/160);	// use RCLK/160 instead  of OCLK/320 , because timeunit last step came to RCLK
    HDMIRX_DEBUG_PRINT(("CEC_timeunit (T100us) = %X\n",(int)CEC_timeunit));
#endif

	RCLKVALUE*=1.1;    // 20170327 MHL CTS
	HDMIRX_DEBUG_PRINT(("RCLKVALUE=%ld.%02ldMHz\n", RCLKVALUE / 1000, (RCLKVALUE % 1000) / 10));
	t1usint = RCLKVALUE / 1000;
	t1usflt = (RCLKVALUE % 1000) * 256 / 1000;
	hdmirxset(0x91, 0x3F, t1usint & 0x3F);
	hdmirxwr(0x92, t1usflt);
	HDMIRX_DEBUG_PRINT(("1us Timer reg91=%02bx, reg92=%02bx \n", hdmirxrd(0x91), hdmirxrd(0x92)));
	RCLKVALUE/=1.1;    // 20170327 MHL CTS add for CTS (RCLK*=1.1)
	*/
}

#define OSC_TYPICAL_VALUE 43000

iTE_u16 iTE6805_OCLK_Load()
{
	iTE_u8 Read_Blcok;
	iTE_u32 lTemp ;
	iTE_u8 temp[4];
	chgbank(0) ;
	hdmirxwr(0xF8, 0xC3) ;
	hdmirxwr(0xF8, 0xA5) ;
	hdmirxwr(0x34, 0x00) ; // reg34[0] to prevent HW gatting something.
	printf("11111111111111111111111111111111111111\n");
	chgbank(1) ;
	hdmirxwr(0x5F, 0x04) ;
	hdmirxwr(0x5F, 0x05) ;
	hdmirxwr(0x58, 0x12) ;
	hdmirxwr(0x58, 0x02) ;
	hdmirxwr(0x57, 0x01) ;


	hdmirxwr(0x50, 0x00) ;
	hdmirxwr(0x51, 0x00) ;
	hdmirxwr(0x54, 0x04) ;
	temp[0]=hdmirxrd(0x61) ;
	temp[1]=hdmirxrd(0x62) ;

	hdmirxwr(0x50, 0x00) ;
	hdmirxwr(0x51, 0x01) ;
	hdmirxwr(0x54, 0x04) ;
	temp[2]=hdmirxrd(0x61) ;
	temp[3]=hdmirxrd(0x62) ;

	if(temp[0]==0xFF&&temp[1]==0xFF&&temp[2]==0x00&&temp[3]==0x00)
	{
		Read_Blcok = 0x04;
	}
	else
	{
		Read_Blcok = 0x00;
	}

	hdmirxwr(0x50, Read_Blcok) ;
	hdmirxwr(0x51, 0xb0) ;
	hdmirxwr(0x54, 0x04) ;
	// hdmirxbrd(0x61, 4, temp);
	temp[0]=hdmirxrd(0x61) ;
	temp[1]=hdmirxrd(0x62) ;
	hdmirxwr(0x50, Read_Blcok) ;
	hdmirxwr(0x51, 0xb1) ;
	hdmirxwr(0x54, 0x04) ; // trigger read.
	// hdmirxbrd(0x61, 4, temp);
	temp[2]=hdmirxrd(0x61) ;
	temp[3]=hdmirxrd(0x62) ;

	HDMIRX_DEBUG_PRINT(("read 0x61=0x%02X, \n",(int)temp[0]));
	HDMIRX_DEBUG_PRINT(("read 0x61=0x%02X, \n",(int)temp[1]));
	HDMIRX_DEBUG_PRINT(("read 0x61=0x%02X, \n",(int)temp[2]));
	HDMIRX_DEBUG_PRINT(("read 0x61=0x%02X, \n",(int)temp[3]));
	lTemp = (iTE_u32)temp[0] ;
	lTemp |= ((iTE_u32)temp[1]&0xFF)<<8 ;
	lTemp |= ((iTE_u32)temp[2]&0xFF)<<16 ;

	if(temp[3]&0xC0==0xC0)
	{
		lTemp /= 100;
	}

	if(( lTemp > 70000 )||(lTemp < 25000))
	{
		HDMIRX_DEBUG_PRINT((" Readback value invalid: 0x%08lX, use typical value instead.\n",lTemp));
		lTemp = OSC_TYPICAL_VALUE ;
	}
	else
	{
		HDMIRX_DEBUG_PRINT((" Readback value valid: 0x%08lX(%ldKHz)\n",lTemp,lTemp));
	}

	hdmirxwr(0x5F, 0x00) ;
	chgbank(0) ;
	hdmirxwr(0xF8, 0x00) ;

	return (iTE_u32)lTemp ;
}




void iTE6805_Init_fsm()
{
	#ifdef DYNAMIC_HDCP_ENABLE_DISABLE
	//iTE6805_DATA.STATE_HDCP = HDCP_ENABLE;
	//iTE6805_DATA.STATE_HDCP_FINAL = HDCP_ENABLE;
	#endif
	iTE6805_DATA.STATEV = STATEV_Unplug;
	iTE6805_DATA.STATEA = STATEA_AudioOff;
	iTE6805_DATA.STATEEQ = STATEEQ_Off;
	iTE6805_DATA.DumpREG = FALSE;
	iTE6805_DATA.CurrentPort = PORT0;
	// init table
	iTE6805_Identify_Chip();
	hdimrx_write_init(iTE6805_INIT_HDMI_TABLE);;

	//iTE6805_Init_CAOF();  //ok

	chgbank(0);
	//hdmirxwr(0x28, 0x88); // 0714 MHL CTS need set 0x28 to 0x88
	//iTE6805_Set_Video_Tristate(TRISTATE_ON);//ok
	//iTE6805_OCLK_Cal();  //set mhl

	// init EDID
	iTE6805_EDID_Init();

	//iTE6805_Init_TTL_VideoOutputConfigure(); //ok
	//iTE6805_Set_TTL_Video_Path(); //ok

	//#ifdef _ENABLE_IT6805_MHL_FUNCTION_
	iTE6805_DATA.DumpREG = FALSE;
	iTE6805_DATA.MHL_DiscoveryDone = 0;
	iTE6805_DATA.MHL_RAP_Content_State = RAP_CONTENT_ON;
	
	mhlrx_write_init(iTE6805_INIT_MHL_TABLE);
	iTE6805_DATA.DumpREG = FALSE;
	//#endif

	#ifdef _ENABLE_IT6805_CEC_   //HDMI数字控制
	//iTE6805_CEC_INIT();
	#endif

	// set MCU pin to low or can't detect iTE6805A0 port1 5V
	//gpPORT1_5V_STATE = 0 ;
	#ifndef EVB_AUTO_DETECT_PORT_BY_PIN
	// main port select
	// check if port 5v on, then that port will be main port
	// if no port have 5v, then main port = port0
	if(iTE6805_Check_5V_State(PORT0) || !iTE6805_Check_5V_State(PORT1))
	{
		iTE6805_Port_Select(PORT0);

	}
	else
	{
		iTE6805_Port_Select(PORT1);

	}
	#else
	iTE6805_Port_Select(PORT0);
	// port select by pin
	//iTE6805_Port_Detect(); //reset port0 
	#endif

	#ifdef DYNAMIC_HDCP_ENABLE_DISABLE
	iTE6805_HDCP_Detect();
	#endif

	// add for checking power saving mode
	//iTE6805_INT_5VPWR_Chg(iTE6805_DATA.CurrentPort);

		iTE6805_DATA.STATEV = STATEV_VidStable;
	iTE6805_DATA.STATEA = STATEA_AudioOn;

	iTE6805_Set_LVDS_Video_Path(2);
	printf("iTE6805_Init_fsm ok..\n");
}

void iTE6805_Init_TTL_VideoOutputConfigure()
{
		//eVidOutConfig =eTTL_SepSync_FullBusHalfPCLKDDR_RGB444
	switch (eVidOutConfig)
	{
		case eTTL_SepSync_FullBusSDR_RGB444:
			iTE6805_DATA.US_Output_ColorFormat = Color_Format_RGB;
			iTE6805_DATA.US_Video_Out_Data_Path = eTTL_SDR;
			iTE6805_DATA.US_Video_Sync_Mode = eTTL_SepSync;
			break;

		case eTTL_SepSync_FullBusSDR_YUV444:
			iTE6805_DATA.US_Output_ColorFormat = Color_Format_YUV444;
			iTE6805_DATA.US_Video_Out_Data_Path = eTTL_SDR;
			iTE6805_DATA.US_Video_Sync_Mode = eTTL_SepSync;
			break;

		case eTTL_SepSync_FullBusSDR_YUV422:
			iTE6805_DATA.US_Output_ColorFormat = Color_Format_YUV422;
			iTE6805_DATA.US_Video_Out_Data_Path = eTTL_SDR;
			iTE6805_DATA.US_Video_Sync_Mode = eTTL_SepSync;
			break;

		case eTTL_SepSync_FullBusHalfPCLKDDR_RGB444:
			iTE6805_DATA.US_Output_ColorFormat = Color_Format_RGB;
			iTE6805_DATA.US_Video_Out_Data_Path = eTTL_HalfPCLKDDR;
			iTE6805_DATA.US_Video_Sync_Mode = eTTL_SepSync;
			break;

		case eTTL_SepSync_FullBusHalfPCLKDDR_YUV444:
			iTE6805_DATA.US_Output_ColorFormat = Color_Format_YUV444;
			iTE6805_DATA.US_Video_Out_Data_Path = eTTL_HalfPCLKDDR;
			iTE6805_DATA.US_Video_Sync_Mode = eTTL_SepSync;
			break;

		case eTTL_SepSync_FullBusHalfPCLKDDR_YUV422:
			iTE6805_DATA.US_Output_ColorFormat = Color_Format_YUV422;
			iTE6805_DATA.US_Video_Out_Data_Path = eTTL_HalfPCLKDDR;
			iTE6805_DATA.US_Video_Sync_Mode = eTTL_SepSync;
			break;

		case eTTL_SepSync_HalfBusDDR_RGB444:
			iTE6805_DATA.US_Output_ColorFormat = Color_Format_RGB;
			iTE6805_DATA.US_Video_Out_Data_Path = eTTL_HalfBusDDR;
			iTE6805_DATA.US_Video_Sync_Mode = eTTL_SepSync;
			break;

		case eTTL_SepSync_HalfBusDDR_YUV444:
			iTE6805_DATA.US_Output_ColorFormat = Color_Format_YUV444;
			iTE6805_DATA.US_Video_Out_Data_Path = eTTL_HalfBusDDR;
			iTE6805_DATA.US_Video_Sync_Mode = eTTL_SepSync;
			break;

		case eTTL_EmbSync_FullBusSDR_YUV422:
			iTE6805_DATA.US_Output_ColorFormat = Color_Format_YUV422;
			iTE6805_DATA.US_Video_Out_Data_Path = eTTL_SDR;
			iTE6805_DATA.US_Video_Sync_Mode = eTTL_EmbSync;
			break;

		case eTTL_EmbSync_FullBusHalfPCLKDDR_YUV422:
			iTE6805_DATA.US_Output_ColorFormat = Color_Format_YUV422;
			iTE6805_DATA.US_Video_Out_Data_Path = eTTL_HalfPCLKDDR;
			iTE6805_DATA.US_Video_Sync_Mode = eTTL_EmbSync;
			break;

		case eTTL_BTA1004_SDR:	//BTA1004_SDR_Emb_Sync
			iTE6805_DATA.US_Output_ColorFormat = Color_Format_YUV422;
			iTE6805_DATA.US_Video_Out_Data_Path = eTTL_SDR_BTA1004;
			iTE6805_DATA.US_Video_Sync_Mode = eTTL_EmbSync;
			break;

		case eTTL_BTA1004_DDR:  //BTA1004_DDR_Emb_Sync
			iTE6805_DATA.US_Output_ColorFormat = Color_Format_YUV422;
			iTE6805_DATA.US_Video_Out_Data_Path = eTTL_DDR_BTA1004;
			iTE6805_DATA.US_Video_Sync_Mode = eTTL_EmbSync;
			break;
	}
}

void iTE6805_Init_CAOF()
{
	iTE_u8 Reg08h, Reg0Dh;
	iTE_u8 Port0_Status;
	iTE_u8 Port1_Status;
	iTE_u8 Port0_Int;
	iTE_u8 Port1_Int;
	iTE_u8 waitcnt;
	chgbank(3);
	hdmirxset(0x3A, 0x80, 0x00); // Reg_CAOFTrg low
	hdmirxset(0xA0, 0x80, 0x80);
	hdmirxset(0xA1, 0x80, 0x80);
	hdmirxset(0xA2, 0x80, 0x80);

	hdmirxset(0x3B, 0xC0, 0x00); // Reg_ENSOF, Reg_ENCAOF
	hdmirxset(0xA7, 0x10, 0x10); // set Reg_PHSELEN high
	hdmirxset(0x48, 0x80, 0x80); // for read back sof value registers
	chgbank(0);
	// Disable MHL AutoPWD
	hdmirxset(0x29, 0x01, 0x01);
	// Inverse COF CLK of Port0, CAOFRST
	hdmirxset(0x2A, 0x41, 0x41);
	iTE6805_delay1ms(10);
	hdmirxset(0x2A, 0x40, 0x00);
	hdmirxset(0x24, 0x04, 0x04); // IPLL RST
	hdmirxwr(0x25, 0x00);        // Disable AFE PWD
	hdmirxwr(0x26, 0x00);
	hdmirxwr(0x27, 0x00);
	hdmirxwr(0x28, 0x00);
	hdmirxset(0x3C, 0x10, 0x00); //disable PLLBufRst
	//----------------------------------------
	// Port 1
	
	chgbank(7);
	hdmirxset(0x3A, 0x80, 0x00); // Reg_CAOFTrg low
	hdmirxset(0xA0, 0x80, 0x80);
	hdmirxset(0xA1, 0x80, 0x80);
	hdmirxset(0xA2, 0x80, 0x80);

	hdmirxset(0x3B, 0xC0, 0x00); // Reg_ENSOF, Reg_ENCAOF
	hdmirxset(0xA7, 0x10, 0x10); // set Reg_PHSELEN high
	hdmirxset(0x48, 0x80, 0x80); // for read back sof value registers

	chgbank(0);
	hdmirxset(0x32, 0x41, 0x41); // CAOF RST, inverse CAOF CLK
	iTE6805_delay1ms(10);
	hdmirxset(0x32, 0x40, 0x00);
	hdmirxset(0x2C, 0x04, 0x04); // IPLL RST
	hdmirxwr(0x2D, 0x00);        // Disable AFE PWD
	hdmirxwr(0x2E, 0x00);
	hdmirxwr(0x2F, 0x00);
	hdmirxwr(0x30, 0x00);
	chgbank(4);
	hdmirxset(0x3C, 0x10, 0x00); //disable PLLBufRst
	
	//----------------------------------------
	// Port 0
	chgbank(3);
	hdmirxset(0x3A, 0x80, 0x80); // Reg_CAOFTrg high
	
	// Port 1
	chgbank(7);
	hdmirxset(0x3A, 0x80, 0x80); // Reg_CAOFTrg high
	

	// wait for INT Done
	chgbank(0);
	Reg08h = hdmirxrd(0x08) & 0x30;
	Reg0Dh = hdmirxrd(0x0D) & 0x30; //0x0d 是port1
	waitcnt = 0;
	while (Reg08h==0x00 ){//|| Reg0Dh==0x00){
        Reg08h= hdmirxrd(0x08)&0x30;
	    Reg0Dh= hdmirxrd(0x0D)&0x30;
		HDMIRX_DEBUG_PRINT(("Wait for CAOF Done!!!!!!\n"));
		HDMIRX_DEBUG_PRINT((" Reg08h= %x .......................\n",(int) hdmirxrd(0x08)));//(int) hdmirxrd(0x0D)));
		if(waitcnt>4) {
			HDMIRX_DEBUG_PRINT(("\n"));
			HDMIRX_DEBUG_PRINT(("CAOF Fail to Finish!! \n"));
			if(Reg08h==0x00) { // 20170322
				hdmirxset(0x2A, 0x40, 0x40);
				iTE6805_delay1ms(10);
				hdmirxset(0x2A, 0x40, 0x00);
			}
			waitcnt=0;
		}
		iTE6805_delay1ms(10);
		waitcnt++;
	}
	chgbank(3);
	Port0_Status = (hdmirxrd(0x5A) << 4) + (hdmirxrd(0x59) & 0x0F);
	Port0_Int = hdmirxrd(0x59) & 0xC0;
	
	chgbank(7);
	Port1_Status = (hdmirxrd(0x5A) << 4) + (hdmirxrd(0x59) & 0x0F);
	Port1_Int = hdmirxrd(0x59) & 0xC0;
	
	printf("CAOF     CAOF    CAOF     CAOF    CAOF     CAOF\n");
	printf("Port 0 CAOF Int =%x , CAOF Status=%3x\n", Port0_Int, Port0_Status);
	printf("Port 1 CAOF Int =%x , CAOF Status=%3x\n", Port1_Int, Port1_Status);
	// De-assert Port 0
	chgbank(0);
	hdmirxset(0x08, 0x30, 0x30);
	hdmirxset(0x0D, 0x30, 0x30);
	hdmirxset(0x29, 0x01, 0x00); // Enable MHL AutoPWD
	hdmirxset(0x24, 0x04, 0x00); // IPLL RST low
	hdmirxset(0x3C, 0x10, 0x10); //Enable PLLBufRst
	
	// De-assert Port 1
	hdmirxset(0x2C, 0x04, 0x00); // Port 1 IPLL RST low
	chgbank(4);
	hdmirxset(0x3C, 0x10, 0x10); // Port 1 Enable PLLBufRst
	
	chgbank(3);
	hdmirxset(0x3A, 0x80, 0x00); // Reg_CAOFTrg low
	hdmirxset(0xA0, 0x80, 0x00);
	hdmirxset(0xA1, 0x80, 0x00);
	hdmirxset(0xA2, 0x80, 0x00);
	
	chgbank(7);
	hdmirxset(0x3A, 0x80, 0x00); // Reg_CAOFTrg low
	hdmirxset(0xA0, 0x80, 0x00);
	hdmirxset(0xA1, 0x80, 0x00);
	hdmirxset(0xA2, 0x80, 0x00);
	
	chgbank(0);

}



void iTE6805_Init_6028LVDS(iTE_u8 chip)
{

//pccmd w 07 1f b4
//pccmd w 07 1f b4
//pccmd w 07 1e b4
//pccmd w 37 50 b4
//pccmd s 2e 80 80 b4 ::FF auto reset
//pccmd s 0a df 14 b4 ::bit2:DO, bit3:DI, bit4:ddr , bit6:pclkdiv2, bit7:bypass
	//if(chip == 0) gp6028 = 1;
	//else gp6028 = 0;
//pause
	lvdsrxwr(0x07, 0x1f);
	lvdsrxwr(0x07, 0x1f);
	lvdsrxwr(0x07, 0x1e);
	lvdsrxwr(0x37, 0x50);
	lvdsrxwr(0x2e, 0x99);
	//lvdsrxset(0x0a, 0xdf, 0x14);

	switch(eVidOutConfig){
		case eTTL_SepSync_FullBusSDR_RGB444:
		case eTTL_SepSync_FullBusSDR_YUV444:
		case eTTL_SepSync_FullBusSDR_YUV422:
		case eTTL_EmbSync_FullBusSDR_YUV422:
		case eTTL_BTA1004_SDR:
			lvdsrxwr(0x0a, 0x64);
			break;
		case eTTL_SepSync_FullBusHalfPCLKDDR_RGB444:
		case eTTL_SepSync_FullBusHalfPCLKDDR_YUV444:
		case eTTL_SepSync_FullBusHalfPCLKDDR_YUV422:
		case eTTL_EmbSync_FullBusHalfPCLKDDR_YUV422:
		case eTTL_BTA1004_DDR:
			lvdsrxwr(0x0a, 0x34);
			break;
		case eTTL_SepSync_HalfBusDDR_RGB444:
		case eTTL_SepSync_HalfBusDDR_YUV444:
			HDMIRX_DEBUG_PRINT(("iTE68051 do not SUP HalfBusDDR mode"));
			break;
		default:
			break;
	}
	//pccmd w 0b ff b4
	//pccmd s 0a 20 20 b4 ::bit5:RFCLK
	//pccmd s 08 10 10 b4 ::forcestable
	//pccmd s 09 20 20 b4 ::RBSwap
	//pccmd s 2f 70 00 b4 ::for bypass only
	//pccmd w 07 00 b4
	lvdsrxwr(0x0b, 0xff);

	lvdsrxwr(0x08, iTE6028_DE_Delay);
	//if(chip == 0) lvdsrxwr(0x08, 0xD1);
	//else lvdsrxwr(0x08, 0xD1);

	lvdsrxwr(0x09, 0x21);
	lvdsrxwr(0x2f, 0x02);
	//iTE6805_delay1ms(1000);
	lvdsrxwr(0x07, 0x00);
	//gp6028 = 1;
}

// kuro
// w 07 1f b4
// w 07 1f b4
// w 07 1e b4
// w 37 50 b4
// w 2e 99 b4
// w 0a 34 b4
// w 0b ff b4
// w 08 D0 b4
// w 09 21 b4
// w 2f 02 b4
// w 07 00 b4

void iTE6805_Set_1B0_By_PixelClock()
{
	iTE_u8 temp , pixel_repeat;
	chgbank(0);
	temp = (hdmirxrd(0x1B)&0x30)>>4;
	switch(temp){
		case 0:
			pixel_repeat = 0 ;
			break;
		case 1:
			pixel_repeat = 2 ;
			break;
		case 3:
			pixel_repeat = 4 ;
			break;
		default :
			pixel_repeat = 0 ;
			break;
	}
	chgbank(1);
	if((iTE6805_CurVTiming.PCLK/pixel_repeat) < 25000)
	{
		hdmirxset(0xB0, BIT0, 0);
	}
	else
	{
		hdmirxset(0xB0, BIT0, BIT0);
	}
	chgbank(0);
}


#ifdef ENABLE_68051_POWER_SAVING_MODE
void iTE6805_Set_Power_Mode(iTE_u8 Mode)
{
	iTE6805_DATA.CurrentPowerMode = Mode;
	chgbank(0);
	if(Mode == MODE_POWER_SAVING)
	{
		HDMIRX_DEBUG_PRINT(("----------------------------------------MODE_POWER_SAVING\n"));
		hdmirxset(0x23, BIT1, BIT1);
		hdmirxset(0x23, BIT1, 0);

		hdmirxwr(0x25, 0xDF);
		hdmirxwr(0x26, 0xFF);
		hdmirxwr(0x27, 0xFF);
		hdmirxwr(0x2A, 0x3A);
		hdmirxwr(0x2D, 0xFF);
		hdmirxwr(0x2E, 0xFF);
		hdmirxwr(0x2F, 0xFF);
		hdmirxwr(0x32, 0x3E);
		hdmirxset(0xEE, 0x80, 0x00);
		hdmirxwr(0xF8, 0xC3);
		hdmirxwr(0xF8, 0xA5);
		hdmirxwr(0x0F, 0x01);	// chbank
		hdmirxwr(0x5F, 0x04);
		hdmirxwr(0x58, 0x03);
		hdmirxwr(0x0F, 0x03);	// chbank
		hdmirxset(0xA8, 0x08, 0x00);
		hdmirxset(0xAA, 0x60, 0x20);
		hdmirxset(0xAC, 0x03, 0x03);
		hdmirxwr(0x0F, 0x05);	// chbank
		hdmirxset(0xC1, 0x03, 0x00);
		hdmirxset(0xC2, 0x03, 0x00);
		hdmirxset(0xC6, 0x20, 0x00);
		hdmirxwr(0xC7, 0x80);
		hdmirxwr(0xC8, 0x00);
		hdmirxwr(0xC9, 0x00);
		hdmirxwr(0xCA, 0x00);
		hdmirxset(0xCB, 0x0F, 0x00);
		hdmirxwr(0x0F, 0x07);	// chbank
		hdmirxset(0xA8, 0x08, 0x00);
		hdmirxwr(0x0F, 0x00);
		hdmirxwr(0x21, 0xFB);	//hdmirxwr(0x21, 0xFB);	for Enable CEC  when powersaving , //hdmirxwr(0x21, 0xFF);	for Disable CEC when powersaving
		hdmirxwr(0x20, 0xEF);
	}

	if(Mode == MODE_POWER_NORMAL)
	{
		HDMIRX_DEBUG_PRINT(("----------------------------------------MODE_POWER_NORMAL\n"));
		hdmirxwr(0x25, 0x00);
		hdmirxwr(0x26, 0x00);
		hdmirxwr(0x27, 0x00);
		hdmirxwr(0x2A, 0x01);
		hdmirxwr(0x2D, 0x00);
		hdmirxwr(0x2E, 0x00);
		hdmirxwr(0x2F, 0x00);
		hdmirxwr(0x32, 0x01);
		hdmirxset(0xEE, 0x80, 0x80);
		hdmirxwr(0xF8, 0xC3);
		hdmirxwr(0xF8, 0xA5);
		hdmirxwr(0x0F, 0x01);	// chbank
		hdmirxwr(0x5F, 0x04);
		hdmirxwr(0x58, 0x02);
		hdmirxwr(0x0F, 0x03);	// chbank
		hdmirxset(0xA8, 0x08, 0x08);
		hdmirxset(0xAA, 0x60, 0x60);
		hdmirxset(0xAC, 0x03, 0x00);
		hdmirxwr(0x0F, 0x05);	// chbank
		hdmirxset(0xC1, 0x03, 0x00); // same
		hdmirxset(0xC2, 0x03, 0x00); // same
		hdmirxset(0xC6, 0x20, 0x20);
		hdmirxwr(0xC7, 0x00);
		hdmirxwr(0xC8, 0x00);	//same
		hdmirxwr(0xC9, 0x00);	//same
		hdmirxwr(0xCA, 0x00);	//same
		hdmirxset(0xCB, 0x0F, 0x00);	//same
		hdmirxwr(0x0F, 0x07);	// chbank
		hdmirxset(0xA8, 0x08, 0x08);	//same
		hdmirxwr(0x0F, 0x00);	// chbank
		hdmirxwr(0x21, 0x00);
		hdmirxwr(0x20, 0x00);

	}
}
#endif


#ifdef DYNAMIC_HDCP_ENABLE_DISABLE
void iTE6805_Set_HDCP(iTE_u8 HDCP_STATE)
{
	if(HDCP_STATE == HDCP_ENABLE)
	{
		// enable HDCP2
		chgbank(0);
		hdmirxset(0xE2, BIT0, BIT0);

		// enable HDCP1
		hdmirxset(0xCB, BIT2, BIT2);
		chgbank(4);
		hdmirxset(0xCB, BIT2, BIT2);
		chgbank(0);

	}
	else
	{
		// disable HDCP2
		chgbank(0);
		hdmirxset(0xE2, BIT0, 0x00);

		// disable HDCP1
		hdmirxset(0xCB, BIT2, 0x00);
		chgbank(4);
		hdmirxset(0xCB, BIT2, 0x00);
		chgbank(0);

	}
}
#endif
