///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_Config.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/
#ifndef _iTE6615_CONFIG_H_
#define _iTE6615_CONFIG_H_

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// EVB Option //
#ifndef _MCU_8051_EVB_
#define _MCU_8051_EVB_ FALSE //TRUE // if not ITE EVB, do not set this.
#endif

// COMMON iTE6264/iTE6615 ENABLE //
#ifndef iTE6615
    #define iTE6615 TRUE   // iTE6615
#endif
#ifndef iTE6264
    #define iTE6264 FALSE  // iTE6264
#endif

// COMMON INPUT CONFIG //
#define _Input_ColorSpace_              RGB444              // RGB444, YUV422, YUV444, YUV420
#define _Input_YCC_Colorimetry_         ITU709              // ITU601, ITU709 , dont care If input is RGB, RGB auto set this to 0 (No Data)
#define _Input_RGB_QuantizationRange_   RGB_RANGE_LIMIT     // RGB_RANGE_LIMIT, RGB_RANGE_FULL
#define _Input_Video_AspectRatio_       AR_4_3              // AR_4_3, AR_16_9, AR_14_9

// COMMON OUTPUT CONFIG //
#define _Output_ColorSpace_             RGB444// YUV444          // RGB444, YUV422, YUV444, YUV420 ( only if the input color mode is also YUV420 )
#define _Output_ColorDepth_             VID8BIT         // VID8BIT, VID10BIT, VID12BIT
#define _Output_YCC_Colorimetry_        ITU709          // ITU601, ITU709 , dont care If output is RGB, or can be set to 0 (No Data)
#define _Output_RGB_QuantizationRange_  RGB_RANGE_LIMIT // RGB_RANGE_FULL, RGB_RANGE_LIMIT

// HDMI AVI Infoframe - Extended Colorimetry CONFIG
#define _Output_ExtendedColorimetry_Enable_ 0           // 1 = Enable, 0 = Disable
#define _Output_ExtendedColorimetry_Format_  BT2020cYCC //xvYCC709 //AdobeRGB //BT2020cYCC  // AdobeRGB BT2020cYCC BT2020YCC
                                                        // xvYCC601 xvYCC709 sYCC601 AdobeYCC601,

// COMMON INPUT DATA PROCESSING CONFIG
#define _Input_Enable_DualMode_         1 //0                // TTL/LVDS Dual Input = 1,  TTL/LVDS Single Input = 0
#define _Input_Enable_DualMode_Mode_    MODE_ODDEVEN    // MODE_ODDEVEN/MODE_LEFTRIGHT
#define _Input_Enable_HL_Swap_          0               // Input L/H swap function, !!!single-in only for "Low"!!! can't swap by register.

// 6615 TTL INPUT CONFIG
#define _TTL_Enable_DDR_        1  //1   // 1 : Input Enable DDR
#define _TTL_Enable_HalfBus_    0   // 1 : Input Enable Half-Bus
#define _TTL_Enable_BTAFormat_  0   // 1 : Input Enable YCbCr422 BTA-T1004 format
#define _TTL_Enable_SyncEmbedd_ 0   // 1 : Input Enable Sync Embedded
#define _TTL_INPUT_LMSwap_      0
#define _TTL_INPUT_YCSwap_      0
#define _TTL_INPUT_RBSwap_      0
#define _TTL_INPUT_PackSwap_    0

// 6615 TTL Clock Latch, Delay/Invert CONFIG
#define TTL_EnICLKHInv  0   // 0,1 for input clk invert
#define TTL_EnICLKLInv  0   // 0,1 for input clk invert
#define TTL_ICLKHDly    0   // 0...3 for input clk delay
#define TTL_ICLKLDly    0   // 0...3 for input clk delay

// 6264 LVDS INPUT CONFIG
#define _LVDS_RxEnDeSSC_    0           // De-SSC function
#define _LVDS_RxSkewDis_    0           // De-Skew function in DCABL mode
#define _LVDS_RxColDep_     1           // 0: 6-bit, 1: 8-bit, 2: 10-bit
#define _LVDS_RxOutMap_     0           // 0: JEIDA, 1: VESA
#define _LVDS_RxInSwap_     0           // Enable LvRx Ch0/1 swap, Ch2/3 swap mode
#define _LVDS_RxStbDENum_   8           // Stable DE numbr

// LVDS Clock Latch, Delay & Invert
#define EnSSCLPCLKInv   1   // 0/1
#define EnSSCHPCLKInv   1   // 0/1
#define SSCLPCLKDly     2   // 0..3
#define SSCHPCLKDly     2   // 0..3

// Audio CONFIG
#define AUDIO_ENABLE    1       // 1 : Enable Audio
#define AUDIO_CHANNELS  2       // channel count
#define AUDIO_FREQ      AUD48K  // Output Audio frequency
#define AUDIO_TYPE      LPCM    // LPCM, NLPCM, DSD, HBR
#define AUDIO_SELECT    I2S     // I2S or SPDIF
    // I2S Option
    #define I2S_WORD_LENGTH         0       //  0: 16 bits, 1: 18 bits, 2: 20 bits, 3: 24 bits
    #define I2S_FMT_0               1       //  0: standard I2s, 1 : 32-bit I2s
    #define I2S_FMT_1               0       //  0: Left-justified, 1: Right-justified
    #define I2S_FMT_2               0       //  0: Data delay 1T correspond to WS, // 1: No data delay correspond to WS
    #define I2S_FMT_3               0       //  0: WS=0 is left channel // 1: WS=0 is right channel
    #define I2S_FMT_4               0       //  0: MSB shift first, 1: LSB shift first
    // SPDIF Option
    #define SPDIF_SCHMITT_TRIGGER   0       //  SPDIF Schmitt trigger 0:disable, 1:enable
    // Audio Special Mode Config, 1 : Enable
    #define _Audio_Enable_SPDIF_OSMode_             0
    #define _Audio_Enable_TDMAudio_                 0
    #define _Audio_Enable_TDMAudio_ChannelCount_    0
    #define _Audio_Enable_3DAudio_                  0
    #define _Audio_Enable_MSAudio_                  0
    #define _Audio_Enable_MCLKSamplingMode_         0


// InfoFrame : Output Dynamic Range and Mastering (HDR) Option
// Infoframe content please find global variable InfoFrame_DRM_DB in iTE_6615SYS.c for fill it
// And use iTE_6615Customer_Set_DRM_PRK(u8 Enable) for enable/disable in runtime
#define _Infoframe_Enable_DRM_  0   // Enable Ouput DRM Infoframe
#define _Infoframe_DRM_ZERO_    0   // 1 : Setting all DRM PB to 0

// InfoFrame : 3D Infoframe Setting
#define _Infoframe_Enable3D_                        0   // 1 : Enable 3D Infoframe for 3D format output (Default VSIF 1.4)
#define _Infoframe_Enable3D_DualView_               0   // 1 : Enable 3D Dual-View Signaling (not real 3D) (VSIF 2.x)
#define _Infoframe_Enable3D_DisparityData_present_  0   // 1 : Enable 3D OSD Disparity Indication (the depth informataion for real 3D) (VSIF 2.x)
#define _Infoframe_Enable3D_ViewDependency_         0   // 1 : Enable 3D Independent View Signaling (the dependency information for real 3D) (VSIF 2.x)
#define _Infoframe_3D_Structure_    _3D_FramePacking    // _3D_FramePacking, _3D_SideBySide_Full, _3D_TopBottom, _3D_SideBySide_Half

// HDCP
#define _HDCP_Enable_               0   // 1 : HDCP Enable
#define _HDCP_Enable_RepeaterMode_  0   // 1 : HDCP REPEATER ENABLE

// other
#define _ENABLE_EDID_PARSING_DETAIL_    FALSE  // for parsing EDID detail and show EDID log, default FALSE

// CEC Config //
#define _ENABLE_CEC_        FALSE   // code not ready yet
#define _ENABLE_CEC_I2C_    TRUE
#define CEC_VENDOR_ID       0x0000
#define iTE6615CECGPIOid    1    // for identife iTE6805_CEC use Pin 1.0 & 1.1

//////////////////////////////////////////////////////////////////////////////////////////
// Other Config
//////////////////////////////////////////////////////////////////////////////////////////
#define ADDR_HDMITX 0x98
#define ADDR_LVDSRX 0x2C
#define ADDR_EDID   0xA8    //Software programmable I2C address of iTE6805 EDID RAM
#define ADDR6615_CEC    0x9C    //Software programmable I2C address of iTE6805 CEC



//////////////////////////////////////////////////////////////////////////////////////////
// Other
//////////////////////////////////////////////////////////////////////////////////////////

// hardware RD option
#define config_DisR0ShortRead   1
#define config_DisRiShortRead   1
#define config_LClimit          0
#define config_HDCP_EnableAutoMute 1    // 6265 set to 1, RD set to 0
#define config_EnAutoReAuth     0
#define config_KeepOutOpt       1
#define config_EnSyncDetChk     0
#define config_HDCP_Enable1p1   1         // RD different to code
#define config_EnSyncDet2FailInt    0
#define config_EnRiChk2DoneInt  0
#define config_RecChStSel       0
#define config_SCDCFlagByte     0
#define config_ForceRxOn        0
#define config_VidLatEdge       0
#define config_AudLatEdge       0


#define config_EnDDRInAutoSwap  1
#define config_InSyncAutoSwapOff 0
#define config_EnHDCP2MsgPoll10ms 0
#define config_LvRxDeSSCCLKSel 1
#define config_EnHDCP2TxPollRdyOnly 0   //TRUE: wait RxKSVListRdy when (MSGSize not 0 && Ready=1) for HDCP2Tx CTS 1B-02
// message when complier
#if (_MCU_8051_EVB_==TRUE)
#if ((!iTE6264)&&(!iTE6615))
#error message("no define iTE6264 and iTE6615")
#endif
#if (iTE6264)
#pragma message("Defined for iTE6264")
#endif
#if (iTE6615)
//#pragma message("Defined for iTE6615")
#endif
#endif


#endif // _CONFIG_H_
