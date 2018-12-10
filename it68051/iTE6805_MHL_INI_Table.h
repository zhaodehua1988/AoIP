///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_MHL_INI_Table.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
#ifndef _ITE6805_MHL_INI_TABLE_H_H_
#define _ITE6805_MHL_INI_TABLE_H_H_
iTE6805_INI_REG _CODE iTE6805_INIT_MHL_TABLE[] = {

	{0x0A , 0xFF , 0x00 } ,
	{0x08 , 0xFF , 0x00 } ,
	{0x09 , 0xFF , 0x00 } ,

	{0x28 , 0xDF , 0x40 } , // force to hdmi//come from 6802 setting, and 6805 setting is so.
	{0x29 , 0x83 , (T10usSrcSel<<7)+CDDGSel } ,
	{0x39 , 0x80 , 0x80 } , // I2C Deglitch for MHL
	{0x2A , 0xFE , (Reg5VStableTSel<<1)+(RegWKpulseLAdj<<3)+(RegWKpulseSAdj<<4)+(RegWKFailThr<<5)+(RegDisvOpt<<7) } ,
	{0x2B , 0x3E , (RegAutoWakeUpFailTrg<<1)+(RegEnWakeUpFailTog<<2)+(RegEnWakeUpFailFlt<<3)+(RegWakeUpFailTogSel<<4) } ,

	{0x52 , 0xFF , 0x00 } , // 0: handle by HW
	{0x53 , 0xFF , 0x80 } , // [7]=1 disable FW mode
	{0x01 , 0x8F , 0x88 } , //Cbus configure , [6:4] OSCCLK Divide count select = 5 for bit time =1us
	//{0x01 , 0xFF , (EnCBusDeGlitch<<7) | (Reg100msTOAdj<<2) } , //Cbus configure , [6:4] OSCCLK Divide count select = 5 for bit time =1us
	{0x0C , 0x89 , (PPHDCPOpt<<7)+EnHWPathEn+(PPHDCPOpt2<<3) } , //PATH_EN configure
	{0x36 , 0xFC , (AckHigh<<4)+(AckLow<<2) } ,

	{0x0F , 0x20 , EnPPGBSwap<<5},
	{0x38 , 0x20 , EnDDCSendAbort<<5 } ,

	{0x5C , 0xFF , 0xD4 } ,
	//{0x5C , 0xFC , (EnPktFIFOBurst<<7)+(MSCBurstWrID<<6)+(MSCBurstWrOpt<<5)+(EnMSCBurstWr<<4)+(EnMSCHwRty<<3)+(MSCRxUCP2Nack<<2) } ,
	{0x66 , 0xFF , 0x02 } ,
	//{0x66 , 0x03 , RegBurstWrTOSel } ,

#if(DisCBusBurst == TRUE)
	{0x32, 0xFF, 0x00},
#else
	{0x32, 0xFF, 0x0C},
#endif

	{0x81 , 0xFF , MHLVersion	} ,
	{0x82 , 0x70 , DEV_CAT		} ,
	{0x83 , 0xFF , ADOPTER_ID_H } ,
	{0x84 , 0xFF , ADOPTER_ID_L } ,
	{0x85 , 0xff , SUPP_VID_LINK},		//VID_LINK_MODE come from 6662
	{0x86 , 0xff , AUD_CH},				//AUD_LINK_MODE come from 6662
	{0x87 , 0xff , VIDEO_TYPE},			//VIDEO_TYPE	come from 6662
	{0x88 , 0xff , LOG_DEV_MAP},		//LOG_DEV_MAP	come from 6662
	{0x89 , 0xff , BANDWIDTH},			//BANDWIDTH		come from 6662
	{0x8a , 0xff , FEATURE_FLAG},		//FEATURE_FLAG	come from 6662
	{0x8B , 0xFF , DEVICE_ID_H	} ,
	{0x8C , 0xFF , DEVICE_ID_L	} ,

	{0x2A , 0x01 , 0x01 } ,
	{0x0F , 0x10 , 0x00 } ,
	{0xFF , 0xFF , 0xFF } ,
};

#endif