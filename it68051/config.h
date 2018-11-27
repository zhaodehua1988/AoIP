///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <config.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef _iTE6805_
#pragma message("defined _iTE6805_ config")
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// 68051 68052 EVB Option
//////////////////////////////////////////////////////////////////////////////////////////
#define DEMO		// for demo code MCU pin
// DEMO code
// if EDID_WP0 = 0, when 4k then output dowscale to 1080p
// if EDID_WP0 = 1, if EDID_WP1 = 0 ,when 4k the output is Odd Even Mode
// if EDID_WP0 = 1, if EDID_WP1 = 1 ,when 4k the output is LeftRight Mode


#define EVB_BOARD_FOR_RING_FREQ // for EVB clock ring error need using MCU 100ms K
#define EVB_AUTO_DETECT_PORT_BY_PIN // for P3^0 pin auto detect port
//#define EVB_4096_DOWNSCALE_TO_2048_OR_1920_CONTROL_BY_PIN // for EVB DownScale By PIN P0^0

//////////////////////////////////////////////////////////////////////////////////////////
// 68051 68052 Common Config
//////////////////////////////////////////////////////////////////////////////////////////


#define iTE68051 // iTE68051 iTE68052

#define MAIN_PORT PORT0

//////////////////////////////////////////////////////////////////////////////////////////
// 68051 68052 Common Config
//////////////////////////////////////////////////////////////////////////////////////////


// this define for dynamic HDCP enable/disable by using variable iTE6805_DATA.STATE_HDCP = HDCP_ENABLE/HDCP_DISABLE
// for customer can change HDCP state in upper code level, default HDCP is enable
// default disable this define

#define DYNAMIC_HDCP_ENABLE_DISABLE


// DownScale Filter : DownScale Quality setting
#define DownScale_Filter 0x01 // Default :0x01 ---- 0x01 : 1:6:1 , 0x10 : 1:2:1 , 0x00 : bypass

// DDR setting
// 68051A1 change Clock_Delay from 3 to 2
#define Clock_Delay  2			// arg from 0 to 7
#define Clock_Invert 1			// arg 0/1

// SDR setting
//#define Clock_Delay  3	// arg from 0 to 7
//#define Clock_Invert 0	// arg 0/1

#define Output_Color_Depth 8	// only set to 8 : 8bit , 10 : 10bit, 12 : 12bit

#define Reg_IOMode 0x40	// default 0x40, 0x00 / 0x40 / 0x80

#define DownScale_YCbCr_Color_Range 1	// 1 : Full Range  0 : Limit Range

//#define ENABLE_4K_MODE_ALL_DownScaling_1080p				// this config is define for TI, for all 4k downscaling
//#define ENABLE_4K_MODE_4096x2048_DownScale_To_1920x1080p

#ifndef Enable_LR_Overlap
#define Enable_LR_Overlap 0	// 1 for enable overlap when LR mode only
#endif


//////////////////////////////////////////////////////////////////////////////////////////
// EQ Config
//////////////////////////////////////////////////////////////////////////////////////////
#define _ENABLE_AUTO_EQ_
#define EQ_INIT_VALUE 0x9F
//#define EQ_INIT_VALUE 0x80
//#define EQ_KURO_TEST


//////////////////////////////////////////////////////////////////////////////////////////
// iTE68051 Config
////////////////////////////////////////////////////S//////////////////////////////////////

// still testing, default disable
//#define ENABLE_68051_POWER_SAVING_MODE

// If define DEMO, this define will be ignored, iTE68051_4K60_Mode setting will by WP1 pin setting
#ifndef iTE68051_4K60_Mode
#define iTE68051_4K60_Mode MODE_LeftRight // MODE_EvenOdd or MODE_LeftRight or MODE_DownScale or MODE_EvenOdd_Plus_DownScale
#endif

#define iTE6028_DE_Delay 0xD1 // can be D0~D3, De Delay Level D3>D2>D1>D0

//////////////////////////////////////////////////////////////////////////////////////////
// iTE68051 TTL Config
//////////////////////////////////////////////////////////////////////////////////////////
// iTE68051 RD Suggest all using FullBus DDR Mode
// (6028 no half bus mode and SDR mode can't output 4k60 because LVDS HW limit)
#ifndef eVidOutConfig
#define eVidOutConfig eTTL_SepSync_FullBusHalfPCLKDDR_RGB444
#endif
//eTTL_SepSync_FullBusSDR_RGB444,	// SDR to 66121 can't output 1080p mode
//eTTL_SepSync_FullBusSDR_YUV444,
//eTTL_SepSync_FullBusSDR_YUV422,
//eTTL_SepSync_FullBusHalfPCLKDDR_RGB444,
//eTTL_SepSync_FullBusHalfPCLKDDR_YUV444,
//eTTL_SepSync_FullBusHalfPCLKDDR_YUV422,
//eTTL_SepSync_HalfBusDDR_RGB444,	// 6805A0 6028 do not sup this bus mode, dont using it when 6805A0
//eTTL_SepSync_HalfBusDDR_YUV444,	// 6805A0 6028 do not sup this bus mode, dont using it when 6805A0
//eTTL_EmbSync_FullBusSDR_YUV422,
//eTTL_EmbSync_FullBusHalfPCLKDDR_YUV422,
//eTTL_BTA1004_SDR,
//eTTL_BTA1004_DDR

//////////////////////////////////////////////////////////////////////////////////////////
// 68052 LVDS 4K output mode config
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef Mode_68052_4K
#define Mode_68052_4K MODE_LeftRight	// MODE_EvenOdd or MODE_LeftRight or
#endif

#ifndef Mode_68052_4K_DownScale
//#define Mode_68052_4K_DownScale			// for testing now
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// 68052 LVDS Config
//////////////////////////////////////////////////////////////////////////////////////////
#define LVDSSwap	FALSE

#ifndef LVDSMode
#define LVDSMode	JEIDA		// JEIDA or VESA
#endif

#ifndef LVDSColDep
#define LVDSColDep	LVDS8BIT	// LVDS6BIT LVDS8BIT LVDS10BIT
#endif

#ifndef LVDSSSC
#define LVDSSSC		DISABLE		// ENABLE or DISABLE
#endif


//////////////////////////////////////////////////////////////////////////////////////////
// EDID Setting
//////////////////////////////////////////////////////////////////////////////////////////
#define _ENABLE_EDID_RAM_
//#define _HDMI_SWITCH_	// for EDID ABCD Value Setting from TX or default


//////////////////////////////////////////////////////////////////////////////////////////
// 6805 RD define
//////////////////////////////////////////////////////////////////////////////////////////
#define RCLKFreqSel 0x00


//////////////////////////////////////////////////////////////////////////////////////////
// CEC Config
//////////////////////////////////////////////////////////////////////////////////////////
#define _ENABLE_IT6805_CEC_
#define _ENABLE_IT6805_CEC_I2C_
#define CEC_VENDOR_ID 0x0000
#define iTE6805CECGPIOid		0	// for identife iTE6805_CEC use Pin 1.0 & 1.1
#define DISABLE_CEC_REPLY


//////////////////////////////////////////////////////////////////////////////////////////
// Other Config
//////////////////////////////////////////////////////////////////////////////////////////
#define ADDR_HDMI   0x90
#define	ADDR_MHL	0xE0	//Software programmable I2C address of iTE6805 MHL
#define	ADDR_EDID	0xA8	//Software programmable I2C address of iTE6805 EDID RAM
#define ADDR_CEC	0xC8	//Software programmable I2C address of iTE6805 CEC
#define ADDR_LVDS	0xB4	//Software programmable I2C address of iTE6805 LVDS



//////////////////////////////////////////////////////////////////////////////////////////
// MHL Config
//////////////////////////////////////////////////////////////////////////////////////////
//#define _ENABLE_IT6805_MHL_FUNCTION_
//#define _ENABLE_MHL_SUPPORT_3D_
#define _ENABLE_IT6805_MHL_I2C_




#endif // _CONFIG_H_
