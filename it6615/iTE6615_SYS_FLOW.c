///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_SYS_FLOW.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/
#include "iTE6615_Global.h"
#include "iTE6615_Table_CSC.h"
#include "iTE6615_common.h"
iTE6615_GlobalDATA       iTE6615_DATA;
iTE6615_VTiming    iTE6615_CurVTiming;

#define Max_WaitForVidIn_Count 10
u8 Current_WaitForVidIn_Count = Max_WaitForVidIn_Count;

#define MAX_HDCP2_FAIL_COUNT 10
static u8 Current_HDCP2_Fail_Count = 0;

u8 InfoFrame_DRM_DB[26] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};

void iTE6615_Customer_Set_InputColorDepth(u8 Input_ColorDepth)
{
    #if iTE6264
    switch(Input_ColorDepth)
    {
        case 6:
            iTE6615_DATA.config.LVDS_RxColDep = 0;
            iTE6615_DATA.config.Output_ColorDepth =VID8BIT;
            break;
        case 8:
            iTE6615_DATA.config.LVDS_RxColDep = 1;
            iTE6615_DATA.config.Output_ColorDepth =VID8BIT;
            break;
        case 10:
        case 12:    // There is not exist LVDS 12 bit output
            iTE6615_DATA.config.LVDS_RxColDep = 2;
            iTE6615_DATA.config.Output_ColorDepth =VID10BIT;
            break;
        default :
            iTE6615_DATA.config.LVDS_RxColDep = 1;
            iTE6615_DATA.config.Output_ColorDepth =VID8BIT;
            break;
    }
    lvrxset(0x08, 0x30, (iTE6615_DATA.config.LVDS_RxColDep<<4));
    #endif

    #if iTE6615
    switch(Input_ColorDepth)
    {
        case 8:
            iTE6615_DATA.config.Output_ColorDepth = VID8BIT;
            break;
        case 10:
            iTE6615_DATA.config.Output_ColorDepth = VID10BIT;
            break;
        case 12:
            iTE6615_DATA.config.Output_ColorDepth = VID12BIT;
            break;
        default :
            iTE6615_DATA.config.Output_ColorDepth = VID8BIT;
            break;
    }
    #endif
}

void iTE6615_Customer_Set_InputDualMode(u8 Input_Enable_DualMode, u8 Input_Enable_DualMode_Mode)
{
    iTE6615_DATA.config.Input_Enable_DualMode = Input_Enable_DualMode;
    if(Input_Enable_DualMode == 0)
    {
        // single in
        iTE6615_DATA.config.Input_Enable_DualMode_Mode = MODE_ODDEVEN;    // single in must
    }
    else
    {
        iTE6615_DATA.config.Input_Enable_DualMode_Mode = Input_Enable_DualMode_Mode;
    }
    hdmitxset(0x90, 0x05, (iTE6615_DATA.config.Input_Enable_DualMode_Mode<<2)+iTE6615_DATA.config.Input_Enable_DualMode);

    #if iTE6264
    if(iTE6615_DATA.config.Input_Enable_DualMode == 1){iTE6615_DATA.config.LVDS_RxChNum = LVDS4CH;}
    else                                              {iTE6615_DATA.config.LVDS_RxChNum = LVDS2CH;}

    lvrxset(0x08, 0x03, iTE6615_DATA.config.LVDS_RxChNum);
    #endif

    hdmitxset(0x95, 0x02, 0x02);
    delay1ms(2);
    hdmitxset(0x95, 0x02, 0x00);
}

void iTE6615_Customer_Set_DRM_PRK(u8 Enable)
{
    if(Enable)
    {
        iTE6615_DATA.config.Infoframe_Enable_DRM = 1;
        hdmitxset(0xC8, 0x03, 0x03);
    }
    else
    {
        iTE6615_DATA.config.Infoframe_Enable_DRM = 0;
        hdmitxset(0xC8, 0x03, 0x00);
    }
}

void iTE6615_fsm(void)
{
    iTE6615_TX_IRQ();
    iTE6615_SCDC_IRQ();
    //iTE6615_DATA.STATES = iTE6615_STATES_RxSenON;
    //printf("iTE6615_DATA.STATEA = %d \n",iTE6615_DATA.STATES);
    if(iTE6615_DATA.STATES == iTE6615_STATES_RxSenON)
    {
        iTE6615_vid_fsm();
    }
   // iTE6615_DATA.STATEV = iTE6615_STATEV_VidOutEnable;
   // printf("iTE6615_DATA.STATEV = %d \n",iTE6615_DATA.STATEV);
    if(iTE6615_DATA.STATEV >= iTE6615_STATEV_VidOutEnable)
    {
        iTE6615_HDCP_IRQ();
        iTE6615_aud_fsm();
    }

    #if _ENABLE_CEC_
    iTE6615_CEC_irq();
    #endif

    #if iTE6264
    iTE6615_LVDS_RX_IRQ();
    #endif
    iTE6615_TASK_Handler();
}


void iTE6615_INIT_Chip(void)
{
    iTE6615_DATA.DumpREG = 1;
    iTE6615_Identify_Chip();

    iTE6615_SetTX_RCLK();

    #if _ENABLE_CEC_
    iTE6615_DATA.vars.cec_time_unit = (iTE6615_DATA.RCLK * 100 / 16)/1000;
    HDMITX_DEBUG_PRINTF(("Set CEC time unit = 0x%02X\r\n", (int)iTE6615_DATA.vars.cec_time_unit));

    hdmitxwr(0xFC, ADDR6615_CEC|0x01);
    iTE6615_CEC_INIT();
    #endif

    // HDMI/DVI and decided by EDID
    iTE6615_DATA.config.EnHDMI = 1;// 1;

    //Input Setting
    iTE6615_DATA.config.Input_ColorSpace = _Input_ColorSpace_;
    iTE6615_DATA.config.Input_YCC_Colorimetry = _Input_YCC_Colorimetry_;
    iTE6615_DATA.config.Input_RGB_QuantizationRange = _Input_RGB_QuantizationRange_;
    iTE6615_DATA.config.Input_Video_AspectRatio = _Input_Video_AspectRatio_;

    //config.Output Setting
    iTE6615_DATA.config.Output_ColorSpace = _Output_ColorSpace_;
    iTE6615_DATA.config.Output_ColorDepth = _Output_ColorDepth_;
    iTE6615_DATA.config.Output_YCC_Colorimetry = _Output_YCC_Colorimetry_;
    iTE6615_DATA.config.Output_RGB_QuantizationRange = _Output_RGB_QuantizationRange_;    // CSC YUV to RGB convert to Full range, please do not modify it

    //Extend Colorimetry Setting;
    iTE6615_DATA.config.Output_ExtendedColorimetry_Enable = _Output_ExtendedColorimetry_Enable_;
    iTE6615_DATA.config.Output_ExtendedColorimetry_Format = _Output_ExtendedColorimetry_Format_;

    //Input setting : TTL/LVDS Single/Dual  (Common config)
    iTE6615_DATA.config.Input_Enable_DualMode = _Input_Enable_DualMode_;    // LVDS setting to 4 Channel will auto config to Dual
    iTE6615_DATA.config.Input_Enable_DualMode_Mode = _Input_Enable_DualMode_Mode_;          // define by Customer
    iTE6615_DATA.config.Input_Enable_HL_Swap = _Input_Enable_HL_Swap_;

    //Input setting : TTL config   ----- 6615 config -----
    iTE6615_DATA.config.TTL_Enable_DDR = _TTL_Enable_DDR_;
    iTE6615_DATA.config.TTL_Enable_HalfBus = _TTL_Enable_HalfBus_;
    iTE6615_DATA.config.TTL_Enable_BTAFormat = _TTL_Enable_BTAFormat_;      // Input Enable YCbCr422 BTA-T1004 format
    iTE6615_DATA.config.TTL_Enable_SyncEmbedd = _TTL_Enable_SyncEmbedd_;    // Input Enable Sync Embedded

    //Input setting : LVDS config  ----- 6264 config -----
    // iTE6615_DATA.config.LVDS_RxChNum will be auto config by iTE6615_Customer_Set_InputDualMode
    iTE6615_DATA.config.LVDS_RxChNum = LVDS4CH;    // LVDS1CH: 1-channel, LVDS2CH: 2-channel, LVDS4CH: 4-channel
    iTE6615_DATA.config.LVDS_RxEnDeSSC = _LVDS_RxEnDeSSC_;
    iTE6615_DATA.config.LVDS_RxSkewDis = _LVDS_RxSkewDis_;
    iTE6615_DATA.config.LVDS_RxColDep = _LVDS_RxColDep_;   // 0: 6-bit, 1: 8-bit, 2: 10-bit
    iTE6615_DATA.config.LVDS_RxOutMap = _LVDS_RxOutMap_;   // 0: JEIDA, 1: VESA
    iTE6615_DATA.config.LVDS_RxInSwap = _LVDS_RxInSwap_;
    iTE6615_DATA.config.LVDS_RxStbDENum = _LVDS_RxStbDENum_;

    //3D Infoframe Option
    iTE6615_DATA.config.Infoframe_Enable3D = _Infoframe_Enable3D_;     // for VSIF 1.4 let sink know its 3D mode for display
    iTE6615_DATA.config.Infoframe_Enable3D_DualView = _Infoframe_Enable3D_DualView_;               // for VSIF 2.x let sink know its 3D mode for display
    iTE6615_DATA.config.Infoframe_Enable3D_DisparityData_present = _Infoframe_Enable3D_DisparityData_present_;  // for VSIF 2.x let sink know its 3D mode for display
    iTE6615_DATA.config.Infoframe_Enable3D_ViewDependency = _Infoframe_Enable3D_ViewDependency_;         // for VSIF 2.x let sink know its 3D mode for display
    iTE6615_DATA.config.Infoframe_3D_Structure = _Infoframe_3D_Structure_  ;  // 3D Structure

    //Dynamic Range and Mastering InfoFrame Option
    iTE6615_DATA.config.Infoframe_Enable_DRM = _Infoframe_Enable_DRM_;
    iTE6615_DATA.config.Infoframe_DRM_ZERO = _Infoframe_DRM_ZERO_; // set all DRM to 0
    // Infoframe content please find global variable InfoFrame_DRM_DB in iTE6615_SYS.c for fill it

    //other mode
    iTE6615_DATA.config.Enable_DEOnly_Mode = 0;    // H2TX DEOnly Mode

    //Software Audio Option
    iTE6615_DATA.config.Audio_Enable = AUDIO_ENABLE;
    iTE6615_DATA.config.Audio_Type = AUDIO_TYPE;
    iTE6615_DATA.config.Audio_Frequency = AUDIO_FREQ;
    iTE6615_DATA.config.Audio_ChannelCount = AUDIO_CHANNELS;
    //Hardward output setting
    iTE6615_DATA.config.Audio_Output_Sel = AUDIO_SELECT;   // SPDIF/I2S
    iTE6615_DATA.config.Audio_I2C_HardwareFormat = (I2S_FMT_4<<4)|(I2S_FMT_3<<3)|(I2S_FMT_2<<2)|(I2S_FMT_1<<1)|(I2S_FMT_0);
    //I2S option
    iTE6615_DATA.config.Audio_I2S_WordLength = I2S_WORD_LENGTH;

    //Audio Special Mode Option
    iTE6615_DATA.config.Audio_Enable_SPDIF_OSMode = _Audio_Enable_SPDIF_OSMode_;                    // SPDIF Over-Sampling Mode
    iTE6615_DATA.config.Audio_Enable_TDMAudio = _Audio_Enable_TDMAudio_;                            // If TRUE, AudSel must be I2S and AudType cannot be DSD and EnAudGen must be TRUE for Internal AudGen
    iTE6615_DATA.config.Audio_Enable_TDMAudio_ChannelCount = _Audio_Enable_TDMAudio_ChannelCount_;  // 0, TDM2CH, TDM4CH, TDM6CH, TDM8CH
    iTE6615_DATA.config.Audio_Enable_3DAudio = _Audio_Enable_3DAudio_;                              // only for LPCM and DSD when AudCh is more than 8
    iTE6615_DATA.config.Audio_Enable_MSAudio = _Audio_Enable_MSAudio_;                              // only for LPCM/NLPCM and DSD when AudCh is less than or equal to 8
    iTE6615_DATA.config.Audio_Enable_MCLKSamplingMode = _Audio_Enable_MCLKSamplingMode_;            // SPDIF MCLK-Sampling Mode

    // HDCP Option
    iTE6615_DATA.config.HDCP_Enable = _HDCP_Enable_;
    iTE6615_DATA.config.HDCP_Enable_RepeaterMode = _HDCP_Enable_RepeaterMode_;

    // Other for CTS
    iTE6615_DATA.config.DDCSpeed = DDC75K;

    iTE6615_DATA.config.DisLockPR = 0;   // TRUE: for ATC format 6,7,21,22, 0530 do not need it because of add code for detect PixelRpt by VIC
    iTE6615_DATA.config.ManuallPR = 1;
    iTE6615_DATA.config.AutoVIC = 1;
    iTE6615_DATA.config.PixRpt = 1;
    iTE6615_DATA.config.EnH2InLBBypass = 1;

    iTE6615_DATA.Flag_EDID_Parser_Ready = 0;
    iTE6615_sys_chg(iTE6615_STATES_Unplug);
    printf("iTE6615_SetTX_RCLK end..");
}

// IRQ
void iTE6615_TX_IRQ(void)
{
    u8 TxReg0F, TxReg12, TxReg13, TxReg16;
    u8 rddata, rddata2;
    TxReg0F = hdmitxrd(0x0F)&0x04;
    TxReg12 = hdmitxrd(0x12);  hdmitxwr(0x12, TxReg12);
    TxReg13 = hdmitxrd(0x13);  hdmitxwr(0x13, TxReg13&0xF8);
    TxReg16 = hdmitxrd(0x16);  hdmitxwr(0x16, TxReg16&0x0F);

    if(!TxReg0F){
        //HDMITX_INT_PRINTF(("getTxReg0f err ...\r\n"));
        return;
    } 

    if (TxReg12 & BIT0)
    {
        iTE6615_INT_HPD_Chg();
    }

    if (TxReg12 & BIT1)
    {
        iTE6615_INT_RXSENSE_Chg();
    }

    if (TxReg12 & BIT2) {
        HDMITX_INT_PRINTF(("DDC Command Fail Interrupt ...\r\n"));
        hdmitxset(0x35, 0x10, 0x10);
        hdmitxset(0x35, 0x10, 0x00);
        hdmitxwr(0x2E, 0x0F);           // Abort DDC Command
    }

    if (TxReg12 & BIT3) {
        HDMITX_INT_PRINTF(("DDC FIFO Error Interrupt ...\r\n"));
        hdmitxset(0x28, 0x01, 0x01);
        hdmitxwr(0x2E, 0x09);           // DDC FIFO Clear
        hdmitxset(0x28, 0x01, 0x00);
    }

    if (TxReg12 & BIT4) {
        HDMITX_INT_PRINTF(("DDC FIFO Valid Interrupt ...\r\n"));
    }

    if (TxReg12 & BIT5) {
        HDMITX_INT_PRINTF(("Audio FIFO Error Interrupt ...\r\n"));
        hdmitxset(0x05, 0x20, 0x20);
        delay1ms(1);
        hdmitxset(0x05, 0x20, 0x00);
        iTE6615_aud_chg(iTE6615_STATEA_Reset);
        iTE6615_aud_chg(iTE6615_STATEA_WaitForAudIn);
    }

    if (TxReg12 & BIT6) {
        HDMITX_INT_PRINTF(("VSync Event Interrupt ...\r\n"));
    }

    if (TxReg12 & BIT7) {
        iTE6615_INT_SCDT_Chg();
    }

    if (TxReg13 & 0x08) {
        HDMITX_INT_PRINTF(("Input Line Buffer Over-Flow Interrupt ...\r\n"));
        hdmitxset(0x95, 0x0a, 0x0a);
        hdmitxset(0x95, 0x0a, 0x00);
    }

    if (TxReg13 & 0x10) {
        HDMITX_INT_PRINTF(("Input Sync FIFO Over-Flow Interrupt ...\r\n"));
        #if iTE6264
        lvrxset(0x05, 0x02, 0x02);
        lvrxset(0x05, 0x02, 0x00);
        #endif
        hdmitxset(0x06, 0x02, 0x02);
        hdmitxset(0x06, 0x02, 0x00);
    }

    if (TxReg13 & 0x20) {
        HDMITX_INT_PRINTF(("Input DE Sync Lose Interrupt ...\r\n"));
    }

    if (TxReg13 & 0x40) {
        HDMITX_INT_PRINTF(("Packet Event Done Interrupt ...\r\n"));
        HDMITX_INT_PRINTF(("Current Event \r\n"));

        switch ( (hdmitxrd(0x10) & 0x0F) ) {
            case 0x0:HDMITX_INT_PRINTF(("GenCtrl Packet\r\n")); break;
            case 0x1:HDMITX_INT_PRINTF(("Null Packet\r\n")); break;
            case 0x2:HDMITX_INT_PRINTF(("ACP Packet\r\n")); break;
            case 0x3:HDMITX_INT_PRINTF(("AVI Packet\r\n")); break;
            case 0x4:HDMITX_INT_PRINTF(("Audio Sample Packet\r\n")); break;
            case 0x5:HDMITX_INT_PRINTF(("MPEG Packet\r\n")); break;
            case 0x6:HDMITX_INT_PRINTF(("GBD Packet\r\n")); break;
            case 0x7:HDMITX_INT_PRINTF(("Vendor Specific Packet\r\n")); break;
            case 0x8:HDMITX_INT_PRINTF(("Audio CTS Change, Step = %d\r\n", (hdmitxrd(0x10) & 0x30) >> 4));
                {
                    u32 AudCTSCnt;
                    chgtxbank(1);
                    AudCTSCnt = ((hdmitxrd(0x35) & 0xF0) >> 4);
                    AudCTSCnt += (hdmitxrd(0x36) << 4);
                    AudCTSCnt += (hdmitxrd(0x37) << 12);
                    chgtxbank(0);
                    HDMITX_INT_PRINTF(("Audio Time Stamp CTSCnt = 0x%04X\r\n", AudCTSCnt));
                }
                break;
            case 0x9:HDMITX_INT_PRINTF(("Audio Meta Packet\r\n")); break;
            default :HDMITX_INT_PRINTF(("Reserved\r\n")); break;
        }
    }


    if (TxReg16 & 0x01) {
        HDMITX_INT_PRINTF(("Audio Decode Error Interrupt ...\r\n"));
    }

    if (TxReg16 & 0x02) {
        HDMITX_INT_PRINTF(("No Audio Interrupt ...\r\n"));
        iTE6615_aud_chg(iTE6615_STATEA_Reset);
        iTE6615_aud_chg(iTE6615_STATEA_WaitForAudIn);
    }

    if (TxReg16 & 0x04) {
        HDMITX_INT_PRINTF(("Video FIFO Error Interrupt (AutoReset) ...\r\n"));
    }

    if (TxReg16 & 0x08) {
        HDMITX_INT_PRINTF(("DDC Bus Hang Interrupt ...\r\n"));
        iTE6615_INT_DDC_BusHang();
    }

    if (TxReg16 & 0x40) {

        //HDMITX_INT_PRINTF(("H2TX Video Parameter Change Interrupt ...\r\n"));
        rddata = hdmitxrd(0x99) ;
        rddata2 = hdmitxrd(0x9A) & 0x03 ;
        HDMITX_INT_PRINTF(("INT H2TX Video Parameter Change Status = 0x%02X => ", (int)rddata));
        hdmitxwr(0x16, 0x40);   // write after rddata read

        if(!rddata)HDMITX_DEBUG_PRINTF(("No Video Parameter Change !!!\r\n"));
        if(rddata&BIT0)HDMITX_DEBUG_PRINTF(("HFP Change !!!\r\n"));
        if(rddata&BIT1)HDMITX_DEBUG_PRINTF(("HSW Change !!!\r\n"));
        if(rddata&BIT2)HDMITX_DEBUG_PRINTF(("HBP Change !!!\r\n"));
        if(rddata&BIT3)HDMITX_DEBUG_PRINTF(("HDEW Change!!!\r\n"));
        if(rddata&BIT4)HDMITX_DEBUG_PRINTF(("VFP Change !!!\r\n"));
        if(rddata&BIT5)HDMITX_DEBUG_PRINTF(("VSW Change !!!\r\n"));
        if(rddata&BIT6)HDMITX_DEBUG_PRINTF(("VBP Change !!!\r\n"));
        if(rddata&BIT7)HDMITX_DEBUG_PRINTF(("VDEW Change!!!\r\n"));

        if(rddata2&BIT0)HDMITX_DEBUG_PRINTF(("VFP 2nd Change !!!\r\n"));
        if(rddata2&BIT1)HDMITX_DEBUG_PRINTF(("HVR 2nd Change !!!\r\n"));
        if(rddata2&BIT2)HDMITX_DEBUG_PRINTF(("Interlanced Change !!!\r\n"));
        HDMITX_INT_PRINTF(("\r\n"));
    }

}

void iTE6615_HDCP_IRQ(void)
{
    u8 rxid[5], rddata;
    u8 TxReg13, TxReg16, TxReg17;

    TxReg13 = hdmitxrd(0x13);  hdmitxwr(0x13, TxReg13&0x07);
    TxReg16 = hdmitxrd(0x16);  hdmitxwr(0x16, TxReg16&0xB0);
    TxReg17 = hdmitxrd(0x17);  hdmitxwr(0x17, TxReg17&0xCF);

    if (TxReg13 & BIT0) {

        iTE6615_hdcp_chg(STATEH_Fail);
        if (iTE6615_DATA.Flag_HDCP2x_Enable)
        {
            HDCP_DEBUG_PRINTF(("HDCP2 Auth Fail Interrupt ...\r\n"));
            iTE6615_HDCP2_Show_Status();
            if ( Current_HDCP2_Fail_Count <= MAX_HDCP2_FAIL_COUNT )
            {
                Current_HDCP2_Fail_Count++;
            }
        }
        else
        {
            HDCP_DEBUG_PRINTF(("HDCP1 Auth Fail Interrupt ...\r\n"));
            iTE6615_HDCP1_Show_Status();
        }
        iTE6615_TASK_HDCP_RetryAuth();
    }

    if ((TxReg13 & BIT1) && (iTE6615_DATA.STATEH == STATEH_CPGoing)) {

        if (iTE6615_DATA.Flag_HDCP2x_Enable == FALSE)
        {
            HDCP_DEBUG_PRINTF(("HDCP1 Auth Done Interrupt ...\r\n"));
            iTE6615_HDCP1_Show_Status();
        }
        else
        {
            HDCP_DEBUG_PRINTF(("HDCP2 Auth Done Interrupt ...\r\n"));
            iTE6615_HDCP2_Show_Status();
            Current_HDCP2_Fail_Count = 0;
        }
        iTE6615_hdcp_chg(STATEH_Done);
    }

    if (TxReg13 & BIT2)
    {
        HDCP_DEBUG_PRINTF(("HDCP KSV List Check Interrupt ... \r\n"));
        iTE6615_TASK_HDCP_PollingBCaps();
    }

    if (TxReg16 & BIT4) {
        iTE6615_HDCP_Show_RI();
    }

    if (TxReg16 & BIT5) {
        iTE6615_HDCP_Show_PJ();
    }

    if (TxReg16 & BIT7) {
        rddata = (hdmitxrd(0x40) & 0x7f) >> 1 ;
        HDCP_DEBUG_PRINTF(("HDCP2.2 RHDCP2RepWaitTxMgm!!!\r\n"));
        HDCP_DEBUG_PRINTF(("HDCP2Tx auth current state = %x\r\n", (int)rddata));

        if ( iTE6615_DATA.config.HDCP_Enable_RepeaterMode == TRUE) {
            iTE6615_HDCP2Repeater_StreamManage();
        }
    }

    if (TxReg17 & 0x01) {
        HDCP_DEBUG_PRINTF(("HDCP 1.2 Sync. Detection Fail Interrupt !!!\r\n"));
    }

    if (TxReg17 & 0x02) {
        HDCP_DEBUG_PRINTF(("HDCP2 SRM check Interrupt !!!\r\n"));
        hdmitxbrd(0x5B, 5, &rxid[0]);
        HDCP_DEBUG_PRINTF(("HDCP2 RxID= %02X%02X%02X%02X%02X \r\n", (int)rxid[4],(int)rxid[3],(int)rxid[2],(int)rxid[1],(int)rxid[0]));

        if (iTE6615_HDCP_Check_Revocation(rxid))
        {
            HDCP_DEBUG_PRINTF(("This is revoked RxID in HDCP2 ATC test! \r\n"));
            HDCP_DEBUG_PRINTF(("Authentication Abort !\r\n"));
            iTE6615_hdcp_chg(STATEH_Fail);
        }
    }
    if (TxReg17 & 0x04) {
        HDCP_DEBUG_PRINTF(("HDCP2RxReauthReq Interrupt !!!\r\n"));
    }

    if (TxReg17 & 0x08) {
        HDCP_DEBUG_PRINTF(("RHDCP2 StrmMgmFail Interrupt !!!\r\n"));
    }

    // Read RxID List
    if (TxReg17 & 0x40) {
        u32 RxMsgSize, RxListTotalBytes, MsgfifoWrptr, MsgfifoRdptr, MsgfifoRdStartAdr;
        u8 *RxListID = iTE6615_DATA.data_buffer;
        u16 i;

        HDCP_DEBUG_PRINTF(("HDCP2ListSRMChk Interrupt !!!\r\n"));

        RxMsgSize = ((hdmitxrd(0x4d) & 0x03) << 8) + hdmitxrd(0x4c);
        RxListTotalBytes = RxMsgSize - 0x16;
        hdmitxset(0x70, 0x10, 0x10); // set hdcp2 debug enable reg70h[4]=1

        MsgfifoWrptr = ((hdmitxrd(0x74) & 0x30) << 4) + hdmitxrd(0x76);
        MsgfifoRdptr = ((hdmitxrd(0x74) & 0xc0) << 2) + hdmitxrd(0x75);
        HDCP_DEBUG_PRINTF(("RxMsgSize=%03x, RxListTotalBytes=%03X, MsgfifoWrptr=%03x, MsgfifoRdptr=%03x \r\n", RxMsgSize, RxListTotalBytes, MsgfifoWrptr, MsgfifoRdptr));
        hdmitxset(0x72, 0x80, 0x80); // set hdcp2 FSM debug enable to control message fifo read start address

        MsgfifoRdStartAdr = MsgfifoRdptr - RxListTotalBytes;
        HDCP_DEBUG_PRINTF(("MsgfifoRdStartAdr=0x%03x \r\n", MsgfifoRdStartAdr));
        hdmitxwr(0x73, (MsgfifoRdStartAdr & 0x0ff));        // set Message fifo read start address
        hdmitxwr(0x74, (MsgfifoRdStartAdr & 0xf00) >> 8);    // set Message fifo read start address

        for (i = 0; i < (RxListTotalBytes / 5); i++)
        {
            hdmitxbrd(0x77, 5, RxListID+i*5);
            HDCP_DEBUG_PRINTF(("RxListID %d =%02x %02x %02x %02x %02x \r\n", i, RxListID[0], RxListID[1], RxListID[2], RxListID[3], RxListID[4]));
        }

        hdmitxset(0x72, 0x80, 0x00); // clear hdcp2 FSM debug enable
        hdmitxset(0x70, 0x10, 0x00); // clear hdcp2 debug enable
        hdmitxset(0x42, 0x80, 0x80); // set RHDCP2ListRdDone=1 to let auth process continue
    }

    if (TxReg17 & 0x80) {
        HDCP_DEBUG_PRINTF(("HDCP2RepWaitTxEks Interrupt !!!\r\n"));
        HDCP_DEBUG_PRINTF(("HDCP2Tx auth current state = %x\r\n", (hdmitxrd(0x40) & 0x7f) >> 1));

        if ( iTE6615_DATA.config.HDCP_Enable_RepeaterMode == TRUE)
        {
            iTE6615_HDCP2Repeater_ContinueSKESendEKS();
        }
    }
}


void iTE6615_SCDC_IRQ( void )
{
    u8 TxReg17;

    TxReg17 = hdmitxrd(0x17);    hdmitxwr(0x17, 0x30);

    if ( iTE6615_DATA.sink.HDMIVer < 2 ) {
        return;
    }

    if (TxReg17 & 0x10) {
        HDMITX_SCDC_PRINTF(("HDMI2 SCDC update flag change Interrupt !!!\r\n"));
        iTE6615_INT_SCDC_UpdateFlagChg();
    }

    if (TxReg17 & 0x20) {
        HDMITX_SCDC_PRINTF(("HDMI2 SCDC detect read request Interrupt !!!\r\n"));
        iTE6615_INT_SCDC_RRDetect();
    }
}


#if iTE6264
void iTE6615_LVDS_RX_IRQ( void )
{
    u8 RxReg10, RxReg11, rddata;

    RxReg10 = lvrxrd(0x10); lvrxwr(0x10, RxReg10);
    RxReg11 = lvrxrd(0x11); lvrxwr(0x11, RxReg11);

    if( RxReg10&0x01 ) {
        HDMITX_INT_PRINTF(("LVRX Ch0/1 De-SSC FIFO over-flow interrupt !!!\r\n"));
        lvrxset(0x05, 0x06, 0x06);    // RegSoftSSCRst
        lvrxset(0x05, 0x06, 0x00);    // RegSoftSSCRst
    }

    if( RxReg10&0x02 ) {
        HDMITX_INT_PRINTF(("LVRX Ch0/1 De-SSC FIFO under-flow interrupt !!!\r\n"));
        lvrxset(0x05, 0x06, 0x06);    // RegSoftSSCRst
        lvrxset(0x05, 0x06, 0x00);    // RegSoftSSCRst
    }

    if( RxReg10&0x04 ) {
        HDMITX_INT_PRINTF(("LVRX Ch2/3 De-SSC FIFO over-flow interrupt !!!\r\n"));
        lvrxset(0x05, 0x06, 0x06);    // RegSoftSSCRst
        lvrxset(0x05, 0x06, 0x00);    // RegSoftSSCRst
    }

    if( RxReg10&0x08 ) {
        HDMITX_INT_PRINTF(("LVRX Ch2/3 De-SSC FIFO under-flow interrupt !!!\r\n"));
        lvrxset(0x05, 0x06, 0x06);    // RegSoftSSCRst
        lvrxset(0x05, 0x06, 0x00);    // RegSoftSSCRst
    }

    if( RxReg10&0x10 ) {
        HDMITX_INT_PRINTF(("LVRX Ch0/1 CLK7X_LOCK change interrupt ... L01_CLK7X_LOCK=%d \r\n", lvrxrd(0x06)&0x01));
    }

    if( RxReg10&0x20 ) {
        HDMITX_INT_PRINTF(("LVRX Ch2/3 CLK7X_LOCK change interrupt ... L23_CLK7X_LOCK=%d \r\n", (lvrxrd(0x06)&0x02)>>1));
    }

    if( RxReg10&0x40 ) {
        HDMITX_INT_PRINTF(("LVRX De-SSC PCLKI_LOCK change interrupt ... PCLKI_LOCK=%d \r\n", (lvrxrd(0x06)&0x04)>>2));
    }

    if( RxReg11&0x01 ) {
        HDMITX_INT_PRINTF(("LVRX Ch0/1 LVRx video stable change interrupt, "));
        rddata = lvrxrd(0x38);
        if( rddata&0x10 )
        {
            HDMITX_INT_PRINTF(("LVRX 0/1 Video is Stable!!! \r\n"));
        }
        else
        {
            HDMITX_INT_PRINTF(("LVRX 0/1 Video is Unstable!!! \r\n"));
            lvrxset(0x05, 0x02, 0x02);   //wmh
            lvrxset(0x05, 0x02, 0x00);   //wmh
        }
    }

    if( RxReg11&0x02 ) {
        HDMITX_INT_PRINTF(("LVRX Ch2/3 LVRx video stable change interrupt, "));
        rddata = lvrxrd(0x39);
        if( rddata&0x10 )
        {
            HDMITX_INT_PRINTF(("LVRX 2/3 Video is Stable!!! \r\n"));
        }
        else
        {
            HDMITX_INT_PRINTF(("LVRX 2/3 Video is Unstable!!! \r\n"));
            lvrxset(0x05, 0x02, 0x02);   //wmh
            lvrxset(0x05, 0x02, 0x00);   //wmh
        }
    }
}
#endif

// FSM
void  iTE6615_sys_fsm(void)
{
    //iTE6615_sys_chg(iTE6615_STATES_HPDOn);
    
    switch(iTE6615_DATA.STATES)
    {
        case iTE6615_STATES_Unplug:
            if(iTE6615_CheckTX_HPD())
                iTE6615_sys_chg(iTE6615_STATES_HPDOn);
            break;
        case iTE6615_STATES_HPDOn:
            break;
        case iTE6615_STATES_WaitRxSen:
            if(iTE6615_CheckTX_RXSENSE())
                iTE6615_sys_chg(iTE6615_STATES_RxSenON);
            break;
        case iTE6615_STATES_RxSenON:
            break;
    }
}

void iTE6615_sys_chg(iTE6615_STATES_Type NewState)
{
    if (iTE6615_DATA.STATES == NewState && NewState != iTE6615_STATES_Unplug) {return;}
    iTE6615_DATA.STATES = NewState;

    switch(iTE6615_DATA.STATES)
    {
        case iTE6615_STATES_Unplug:
            HDMITX_DEBUG_PRINTF(("STATES to iTE6615_STATES_Unplug\r\n"));

            iTE6615_aud_chg(iTE6615_STATEA_Reset);

            // check config
            iTE6615_CheckConfig_Audio();
            iTE6615_CheckConfig_Video();

#if iTE6264
            iTE6615_DATA.config.TTL_Enable_DDR = 0;    // If 6264 DDR must setting to SDR or nonstable
            if(iTE6615_DATA.config.Input_Enable_DualMode == 1){iTE6615_DATA.config.LVDS_RxChNum = LVDS4CH;}
            else                                              {iTE6615_DATA.config.LVDS_RxChNum = LVDS2CH;}
#endif

            iTE6615_SetTX_Timestamp(DISABLE);

            iTE6615_SetTX_Reset();

            iTE6615_vid_chg(iTE6615_STATEV_Reset);
            iTE6615_aud_chg(iTE6615_STATEA_Reset);

            iTE6615_SetTX_Power(POWER_OFF);


            iTE6615_DATA.Flag_EDID_Parser_Ready = FALSE;

            break;
        case iTE6615_STATES_HPDOn:

            HDMITX_DEBUG_PRINTF(("STATES to iTE6615_STATES_HPDOn\r\n"));
            iTE6615_SetTX_Power(POWER_ON);


            hdmitxset(0x25, 0x01, 0x00);

            iTE6615_SCDC_RR_Cancel();
            if(iTE6615_DATA.Flag_EDID_Parser_Ready == 0)
            {
                iTE6615_EDID_Parser();
                iTE6615_DATA.Flag_EDID_Parser_Ready = 1;
                HDMITX_DEBUG_PRINTF(("Need to Parser EDID\r\n"));
            }
            else
            {
                HDMITX_DEBUG_PRINTF(("Don't Need to Parser EDID\r\n"));
            }

            #if iTE6264
            lvrxset(0x05, 0x02, 0x02);
            if(iTE6615_DATA.config.LVDS_RxEnDeSSC)
            {
                lvrxset(0x05, 0x04, 0x04);    // RegSoftSSCRst
                lvrxset(0x74, 0x01, 0x00);    //if DeSSC , SP_RSTB=0, reset
                lvrxset(0x74, 0x01, 0x01);    //if DeSSC , SP_RSTB=1, normal
                lvrxset(0x05, 0x04, 0x00);    // RegSoftSSCRst
            }
            lvrxset(0x05, 0x02, 0x00);
            #endif

            hdmitxset(0x06, 0x02, 0x00);
            iTE6615_DATA.STATES = iTE6615_STATES_WaitRxSen;
            delay1ms(10);
        case iTE6615_STATES_WaitRxSen:
            HDMITX_DEBUG_PRINTF(("STATES to iTE6615_STATES_WaitRxSen\r\n"));
            if(iTE6615_CheckTX_RXSENSE())
            {
                iTE6615_DATA.STATES = iTE6615_STATES_RxSenON;   // no break contines go on
            }
            else
            {
                break;
            }

        case iTE6615_STATES_RxSenON:
            HDMITX_DEBUG_PRINTF(("STATES to iTE6615_STATES_RxSenON\r\n"));

            if ( iTE6615_DATA.config.Enable_DEOnly_Mode )
            {
                // not ready
            }
            iTE6615_vid_chg(iTE6615_STATEV_WaitForVidIn);
            break;
        default:
            HDMITX_DEBUG_PRINTF(("ERROR: STATES to Unknown state !!!\r\n"));
            break;
    }
}

void iTE6615_vid_fsm(void)
{
    u8 rechk =6;
    switch(iTE6615_DATA.STATEV)
    {
        case iTE6615_STATEV_Reset:
            if(iTE6615_DATA.STATES==iTE6615_STATES_RxSenON)
            {
                iTE6615_vid_chg(iTE6615_STATEV_WaitForVidIn);
            }
            break;
        case iTE6615_STATEV_WaitForVidIn:
            HDMITX_DEBUG_PRINTF(("iTE6615_STATEV_WaitForVidIn \r\n"));
            if(Current_WaitForVidIn_Count==0)
            {
                Current_WaitForVidIn_Count = Max_WaitForVidIn_Count;
                HDMITX_DEBUG_PRINTF(("RESET Input FIFO\r\n"));

                iTE6615_Get_TXCLK();    // for get the setting of AFE

                #if iTE6264
                iTE6264_Get_SSCPCLK();
                #endif

                #if iTE6615
                hdmitxset(0x94, 0xC0, 0xC0);
                hdmitxset(0x94, 0xC0, 0x00);
                #else
                lvrxset(0x49, 0x40, 0x40);
                hdmitxset(0x95, 0x02, 0x02);
                hdmitxset(0x94, 0x01, 0x01);
                delay1ms(10);
                lvrxset(0x49, 0x40, 0x00);
                hdmitxset(0x95, 0x02, 0x00);
                hdmitxset(0x94, 0x01, 0x00);
                delay1ms(100);    // delay for video stable
                #endif

                if( iTE6615_DATA.TEMP_TXCLK > (iTE6615_DATA.TXCLK+(iTE6615_DATA.TEMP_TXCLK/10))
                ||  iTE6615_DATA.TEMP_TXCLK < (iTE6615_DATA.TXCLK-(iTE6615_DATA.TEMP_TXCLK/10)))
                {
                    iTE6615_sys_chg(iTE6615_STATES_Unplug);
                }
            }
            else
            {
                Current_WaitForVidIn_Count--;
            }

        case iTE6615_STATEV_VidInStable:
__recheck2:
            if (iTE6615_CheckRX_SCDT())
            {
                iTE6615_vid_chg(iTE6615_STATEV_VidOutEnable); // no break, continue to iTE6615_STATEV_VidOutEnable
            }
            else
            {
                if ( rechk > 0 )
                {
                    delay1ms(2);
                    rechk--;
                    goto __recheck2;
                }
                break;
            }
            break;
        case iTE6615_STATEV_VidOutEnable:
            break;
        default:
            break;
    }
}

void iTE6615_vid_chg(iTE6615_STATEV_Type NewState)
{
    u8 rechk = 5;
    if (iTE6615_DATA.STATEV == NewState && NewState != iTE6615_STATEV_Reset) {return;}

    iTE6615_DATA.STATEV = NewState;

    switch(iTE6615_DATA.STATEV)
    {
        case iTE6615_STATEV_Reset:

            HDMITX_DEBUG_PRINTF(("STATEV to iTE6615_STATEV_Reset\r\n"));

            iTE6615_hdcp_chg(STATEH_Reset);
            iTE6615_aud_chg(iTE6615_STATEA_Reset);
            hdmitxset(0x05, 0x04, 0x04);
            break;
        case iTE6615_STATEV_WaitForVidIn:
            Current_WaitForVidIn_Count = Max_WaitForVidIn_Count;

            iTE6615_Set_Video_ColorSpace();        //config CSC
            iTE6615_Set_Video_Infoframe();        //config Infoframe
            #if iTE6264
            lvrxwr(0x14, 0x7F); // Enable LVRX interrupt
            lvrxwr(0x15, 0x03); // Enable LVRX interrupt
            #endif

            // INT Enable
            hdmitxset(0x19, 0x80, 0x80);    // Enable Video Stable Change Interrupt
            hdmitxset(0x1B, 0x1F, 0x1F);    // Enable IPLock/XPLock and TxCLKStb/TxVidStb/TMDSStb change interrupt
            hdmitxset(0x1A, 0x38, 0x38);    // Enable InLBOvFlw, InFFOvFlw, InDESyncLose Interrupt

            hdmitxset(0x05, 0x04, 0x00);    // Disable Video Clock Domain Reset

            #if iTE6264
            iTE6264_Get_SSCPCLK();
            #endif

            iTE6615_Get_TXCLK();    // for get the setting of AFE
            iTE6615_DATA.TEMP_TXCLK = iTE6615_DATA.TXCLK;    // for recover RX input
            iTE6615_SetTX_AFE();
            break;
        case iTE6615_STATEV_VidInStable:
            HDMITX_DEBUG_PRINTF(("STATEV to iTE6615_STATEV_VidInStable\n"));
        __recheck:
            if (iTE6615_CheckRX_SCDT())
            {
                iTE6615_DATA.STATEV = iTE6615_STATEV_VidOutEnable; // no break, continue to iTE6615_STATEV_VidOutEnable
            }
            else
            {
                if ( rechk > 0 )
                {
                    delay1ms(2);
                    rechk--;
                    goto __recheck;
                }
                break;
            }

        case iTE6615_STATEV_VidOutEnable:
            HDMITX_DEBUG_PRINTF(("STATEV to iTE6615_STATEV_VidOutEnable\n"));

            hdmitxwr(0x16, 0x04);           // Clear Video Input FIFO Interrupt     // kuro can be setting to init
            hdmitxset(0x1D, 0x04, 0x04);    // Enable Video Input FIFO Interrupt    // kuro can be setting to init

            iTE6615_Get_VideoInfo();
            iTE6615_Show_VideoInfo();

            iTE6615_Set_HDMI2_SCDC();


            // Enable AFE output
            hdmitxset(0x88, 0x03, 0x00);    // Set DRV_RST='0'

            iTE6615_Set_HDMI2_SCDC();

            hdmitxwr(0x16, 0x40);
            hdmitxset(0x1D, 0x40, 0x40);    // Enable Video Parameter Change Interrupt  // kuro can be setting to init
            hdmitxset(0x6F, 0x08, 0x08);    // Enable auto mute
            iTE6615_SetTX_AVMute(AVMUTE_OFF);

            //iTE6615_TASK_AVInfo_ShowVideoInfo();
            iTE6615_aud_chg(iTE6615_STATEA_WaitForAudIn);

            if ( iTE6615_DATA.config.HDCP_Enable )
            {
                delay1ms(100);
                iTE6615_hdcp_chg(STATEH_CPGoing);
            }
            break;
        default:
            HDMITX_DEBUG_PRINTF(("ERROR: STATEV to Unknown state !!!\r\n"));
            break;
    }
}

void iTE6615_aud_fsm(void)
{
    u8 rddata, i;
    switch(iTE6615_DATA.STATEA)
    {
        case iTE6615_STATEA_Reset:
            //HDMITX_DEBUG_PRINTF(("STATEA to iTE6615_STATEA_Reset\r\n"));
            break;
        case iTE6615_STATEA_WaitForAudIn:
            //HDMITX_DEBUG_PRINTF(("STATEA to iTE6615_STATEA_WaitForAudIn\n"));
            for (i = 0; i < 5; i++)
            {
                if (!(hdmitxrd(0xEF) & 0x40))
                {
                    rddata = hdmitxrd(0xEE);
                    if (hdmitxrd(0xEE) == rddata && rddata != 0xFF)
                    {
                        HDMITX_DEBUG_PRINTF(("STATEA to iTE6615_STATEA_AudInStable\r\n"));
                        iTE6615_aud_chg(iTE6615_STATEA_AudInStable);
                        break;
                    }
                    else
                    {
                        HDMITX_DEBUG_PRINTF(("Wait For Audio Input Stable !!!\r\n"));
                    }
                }
                delay1ms(1);
            }
            break;
        case iTE6615_STATEA_AudInStable:
            //HDMITX_DEBUG_PRINTF(("STATEA to iTE6615_STATEA_AudInStable\r\n"));
            break;
        default:
            //HDMITX_DEBUG_PRINTF(("ERROR: STATEA to Unknown state !!!\r\n"));
            break;
    }
}

void iTE6615_aud_chg(iTE6615_STATEA_Type NewState)
{
    u8 i, rddata;
    if ( iTE6615_DATA.config.EnHDMI == 0 || iTE6615_DATA.config.Audio_Enable == 0 ){return;}
    if (iTE6615_DATA.STATEA == NewState && NewState != iTE6615_STATEA_Reset) {return;}

    iTE6615_DATA.STATEA = NewState;

    switch(iTE6615_DATA.STATEA)
    {
        case iTE6615_STATEA_Reset:
            HDMITX_DEBUG_PRINTF(("STATEA to iTE6615_STATEA_Reset\r\n"));
            hdmitxset(0x05, 0xA2, 0xA2);    // Audio Clock Domain Reset
            hdmitxset(0xC7, 0x3C, 0x00);    // Disable AudInfoFrame and AudMetaInfo
            hdmitxset(0x19, 0x20, 0x00);    // Disable Audio FIFO OverFlow Interrupt
            hdmitxset(0x1A, 0x40, 0x00);    // Disable Audio CTS Error Interrupt
            hdmitxset(0x1D, 0x03, 0x00);    // Disable Audio Decode Error and No Audio Interrupt
            break;
        case iTE6615_STATEA_WaitForAudIn:
            HDMITX_DEBUG_PRINTF(("STATEA to iTE6615_STATEA_WaitForAudIn\r\n"));
            iTE6615_SetTX_AudioFormat();
            iTE6615_SetTX_AudioOption();
            hdmitxset(0x25, 0x02, 0x00);
            hdmitxset(0x05, 0x82, 0x00);    // Release Audio Clock Domain Reset and SPDIF OverSampling Reset
                                            // Enable Synchronous Audio Reset
            hdmitxset(0xC7, 0x3C, 0x00);    // Disable AudInfoFrame and AudMetaInfo
            for (i = 0; i < 5; i++)
            {
                if (!(hdmitxrd(0xEF) & 0x40))
                {
                    rddata = hdmitxrd(0xEE);
                    if (hdmitxrd(0xEE) == rddata && rddata != 0xFF)
                    {
                        iTE6615_aud_chg(iTE6615_STATEA_AudInStable);
                    }
                    else
                    {
                        HDMITX_DEBUG_PRINTF(("Wait For Audio Input Stable !!!\r\n"));
                    }
                }
                delay1ms(1);
            }
            break;
        case iTE6615_STATEA_AudInStable:
            HDMITX_DEBUG_PRINTF(("STATEA to iTE6615_STATEA_AudInStable\r\n"));
            hdmitxset(0x05, 0x20, 0x00);    // Release Synchronous Audio Reset
            hdmitxset(0xC7, 0x0C, 0x0C);    // Enable Audio InfoFrame
            if (iTE6615_DATA.config.Audio_Enable_3DAudio || iTE6615_DATA.config.Audio_Enable_MSAudio)
            {
                hdmitxset(0xC7, 0x30, 0x30); // Enable AudMetaInfo
            }
            hdmitxset(0x19, 0x20, 0x20);    // Enable Audio FIFO OverFlow Interrupt
            hdmitxset(0x10, 0x0F, 0x08);    // Select Audio CTS Packet Interrupt
            hdmitxset(0x1A, 0x40, 0x40);    // Enable Audio CTS Error Interrupt
            hdmitxwr(0x16, 0x03);           // Clear Audio Decode Error and No Audio Interrupt
            hdmitxset(0x1D, 0x03, 0x03);    // Enable Audio Decode Error and No Audio Interrupt
            iTE6615_TASK_AVInfo_ShowAudioInfo();
            break;
        default:
            HDMITX_DEBUG_PRINTF(("ERROR: STATEA to Unknown state !!!\r\n"));
            break;
    }
}

/*
void iTE6615_hdcp_fsm(void)
{
    switch(iTE6615_DATA.STATEH)
    {
        case STATEH_Reset:
            printf("STATEH to STATEH_Reset\r\n");
            break;
        case STATEH_CPGoing:
            printf("STATEH to STATEH_CPGoing\r\n");
            break;
        case STATEH_Done:
            printf("STATEH to STATEH_Done\r\n");
            break;
        case STATEH_Fail:
            printf("STATEH to STATEH_Fail\r\n");
            break;
        case STATEH_Unknown:
            printf("ERROR: STATEH to Unknown state !!!\r\n");
            break;
    }
}
*/

void iTE6615_hdcp_chg(STATEH_Type NewState)
{
    if (iTE6615_DATA.STATEH == NewState && NewState != STATEH_Reset) {return;}

    iTE6615_DATA.STATEH = NewState;

    if ( iTE6615_DATA.STATEH > STATEH_Reset )
    {
        if (!iTE6615_CheckTX_HPD())
        {
            HDMITX_DEBUG_PRINTF(("HDCP=> STATEH_Reset because HPD Low\r\n"));
            iTE6615_DATA.STATEH = STATEH_Reset;
            iTE6615_TASK_AVInfo_SysUnplug();
        }
    }

    switch(iTE6615_DATA.STATEH)
    {
        case STATEH_Reset:
            HDMITX_DEBUG_PRINTF(("STATEH to STATEH_Reset\r\n"));
            break;
        case STATEH_CPGoing:
            HDMITX_DEBUG_PRINTF(("HDCP=> STATEH_CPGoing\r\n"));
            hdmitxset(0x6F, 0x08, 0x08);    // Enable auto mute
            iTE6615_SetTX_AVMute(AVMUTE_OFF);
            iTE6615_SCDC_Set_RR(DISABLE);
            iTE6615_HDCP2_Enable();
            break;
        case STATEH_Done:
            HDMITX_DEBUG_PRINTF(("HDCP=> STATEH_Done\r\n"));
            iTE6615_SCDC_Set_RR(ENABLE);
            iTE6615_DATA.task.hdcp_cmd = CMD_HDCP_NONE;
            break;
        case STATEH_Fail:
            HDMITX_DEBUG_PRINTF(("HDCP=> STATEH_Fail\r\n"));
            iTE6615_SetTX_AVMute(AVMUTE_ON);
            iTE6615_SCDC_Set_RR(ENABLE);
            if (iTE6615_DATA.STATEV == iTE6615_STATEV_VidOutEnable)
            {
                iTE6615_DATA.STATEH = STATEH_Reset;
            }
            break;
        default:
            HDMITX_DEBUG_PRINTF(("ERROR: STATEH to Unknown state !!!\r\n"));
            break;
    }
}

// INT
void iTE6615_INT_HPD_Chg(void)
{
    HDMITX_INT_PRINTF(("HPD INT,"));

    if (iTE6615_CheckTX_HPD())
    {
        HDMITX_INT_PRINTF(("HPD ON\r\n"));
        iTE6615_sys_chg(iTE6615_STATES_HPDOn);
    }
    else
    {
        HDMITX_INT_PRINTF(("HPD OFF\r\n"));
        iTE6615_DATA.Flag_EDID_Parser_Ready = 0;
        iTE6615_sys_chg(iTE6615_STATES_Unplug);
    }
}


void iTE6615_INT_RXSENSE_Chg(void)
{
    HDMITX_INT_PRINTF(("RxSen Change Interrupt => "));

    if (!iTE6615_CheckTX_HPD())
    {
        iTE6615_sys_chg(iTE6615_STATES_Unplug);
    }
    else
    {
        if ( iTE6615_DATA.STATES == iTE6615_STATES_Unplug )
        {
            // for trigger EDID read and power on
            iTE6615_sys_chg(iTE6615_STATES_HPDOn);
        }

        if ( iTE6615_CheckTX_RXSENSE())
        {
            HDMITX_INT_PRINTF(("RxSen ON\r\n"));
            if ( iTE6615_DATA.STATES == iTE6615_STATES_WaitRxSen )
            {
                iTE6615_sys_chg( iTE6615_STATES_RxSenON );
            }
        }
        else
        {
            HDMITX_INT_PRINTF(("RxSen OFF\r\n"));
            iTE6615_sys_chg(iTE6615_STATES_WaitRxSen);
        }
    }
}

void iTE6615_INT_SCDT_Chg(void)
{
    u8 rddata;
    HDMITX_INT_PRINTF(("TX SCDT change Interrupt ...\r\n"));

    if (iTE6615_CheckRX_SCDT())
    {
        HDMITX_INT_PRINTF(("SCDT On.STATEV=%d\r\n",(int) iTE6615_DATA.STATEV));
        if (iTE6615_DATA.STATEV==iTE6615_STATEV_WaitForVidIn)
        {
            iTE6615_vid_chg(iTE6615_STATEV_VidInStable);
        }
    }
    else
    {
        //TX FIFO manual reset
        chgtxbank(0);
        rddata = hdmitxrd(0x07);
        HDMITX_INT_PRINTF(("TxReg07=0x%02X\r\n",(int) rddata));
        hdmitxset(0x94, 0x01, 0x01);
        hdmitxset(0x94, 0x01, 0x00);
        if ( iTE6615_DATA.STATEV >= iTE6615_STATEV_VidInStable )
        {
            // For Video Input Change Without Information
            iTE6615_SetTX_AVMute(AVMUTE_ON);
            iTE6615_vid_chg(iTE6615_STATEV_Reset);
            iTE6615_aud_chg(iTE6615_STATEA_Reset);
            iTE6615_vid_chg(iTE6615_STATEV_WaitForVidIn);
            iTE6615_aud_chg(iTE6615_STATEA_WaitForAudIn);
        }
        HDMITX_INT_PRINTF(("H2TX Video Stable Off Interrupt ...STATEV=%d\r\n",(int) iTE6615_DATA.STATEV));
    }
}


void iTE6615_INT_SCDC_UpdateFlagChg( void )
{
    u8 rddata;
    rddata = hdmitxrd(0x3E);
    HDMITX_SCDC_PRINTF(("SCDC Update Flags = 0x%02X\r\n", rddata));
    iTE6615_INT_SCDC_UpdateFlagChg_Handler(rddata);
}


void iTE6615_INT_SCDC_UpdateFlagChg_Handler( u8 rddata )
{
    // for clear corresponding register
    u8 sts_flag;
    u8 CED[7], sum, clip_sum;
    u8 i;

    if (rddata & 0x01) {
        iTE6615_SCDC_Write(0x10, 0x01); // W1C
        if (iTE6615_SCDC_Read(0x21, 1)){
            HDMITX_SCDC_PRINTF(("SCDC Scrambling_Status = %d\r\n", hdmitxrd(0x30) & 0x01));
        }

        if (iTE6615_SCDC_Read(0x40, 1)) {
            sts_flag = hdmitxrd(0x30);
            HDMITX_SCDC_PRINTF(("SCDC clk_Detected = %d\r\n", sts_flag & 0x01));
            HDMITX_SCDC_PRINTF(("SCDC Ch0_Locked = %d\r\n",  sts_flag & 0x01));
            HDMITX_SCDC_PRINTF(("SCDC Ch1_Locked = %d\r\n", (sts_flag & 0x02) >> 1));
            HDMITX_SCDC_PRINTF(("SCDC Ch2_Locked = %d\r\n", (sts_flag & 0x04) >> 2));
        }
    }

    if (rddata & 0x02) {
        iTE6615_SCDC_Write(0x10, 0x02); // W1C
        if (iTE6615_SCDC_Read(0x50, 7)) {

            hdmitxbrd(0x30, 7, &CED[0]);
            HDMITX_SCDC_PRINTF(("SCDC Ch0 Valid=%d, Channel 0 Error Count = 0x%04X\r\n", (CED[1] & 0x80) >> 7, ((CED[1] & 0xEF) << 8) + CED[0]));
            HDMITX_SCDC_PRINTF(("SCDC Ch1 Valid=%d, Channel 1 Error Count = 0x%04X\r\n", (CED[3] & 0x80) >> 7, ((CED[3] & 0xEF) << 8) + CED[2]));
            HDMITX_SCDC_PRINTF(("SCDC Ch2 Valid=%d, Channel 2 Error Count = 0x%04X\r\n", (CED[5] & 0x80) >> 7, ((CED[5] & 0xEF) << 8) + CED[4]));
            HDMITX_SCDC_PRINTF(("SCDC Check Sum  = 0x%04X\r\n", CED[6]));

            sum = CED[0];

            for (i = 1; i < 7; i++){
                sum += CED[i];
            }

            clip_sum = (sum & 0xFF);

            if (clip_sum == 0){
                HDMITX_SCDC_PRINTF(("SCDC Checksum of Character Error Detection is Correct ...\r\n"));
            }
            else{
                HDMITX_SCDC_PRINTF(("SCDC Checksum of Character Error Detection is Error !!!\r\n"));
            }
        }
    }

    if (rddata & 0x04) {
        iTE6615_SCDC_Write(0x10, 0x04); // W1C
        HDMITX_SCDC_PRINTF(("SCDC RR_Test ...\r\n"));
    }
}

void iTE6615_INT_SCDC_RRDetect( void )
{
    u8 rddata;
    if (iTE6615_SCDC_Read(0x10, 1))
    {
        rddata = hdmitxrd(0x30);
        HDMITX_SCDC_PRINTF(("SCDC Update Flag = 0x%02X\r\n", rddata));
        iTE6615_INT_SCDC_UpdateFlagChg_Handler(rddata);
    }
    else
    {
        HDMITX_SCDC_PRINTF(("ERROR: Read SCDC Update Flag Error !!!\r\n"));
    }
}

void iTE6615_INT_DDC_BusHang( void )
{
    u8 rddata;
    chgtxbank(0);

    hdmitxset(0x35, 0x10, 0x10);    // RegDDCMasterRst=1
    hdmitxset(0x35, 0x10, 0x00);    // RegDDCMasterRst=0
    hdmitxwr(0x2E, 0x0F);  // Abort DDC Command
    rddata = hdmitxrd(0x41) & 0x01; // RegCPDesired

    if (rddata == 0x01)
    {
        hdmitxset(0x41, 0x01, 0x00);   // Disable HDCP_Desired
        hdmitxset(0x05, BIT6, BIT6);   // Enable HDCP Reset
    }

    hdmitxset(0x28, 0x01, 0x01);
    hdmitxwr(0x2E, 0x0A);              // Generate SCL Clock
    hdmitxset(0x28, 0x01, 0x00);

    if (rddata == 0x01)
    {
        hdmitxset(0x05, BIT6, 0x00);   // Disable HDCP Reset
        hdmitxset(0x41, 0x01, 0x01);   // Enable HDCP_Desired
    }
}

// EDID
u8 iTE6615_EDID_Parser( void )
{
    u8 *edid_tmp = iTE6615_DATA.data_buffer;
    u8 extblock;
    u8 i;

    memset(&iTE6615_DATA.data_buffer, 0, sizeof(iTE6615_DATA.data_buffer));

    if (!iTE6615_Get_EDID_Block(0, edid_tmp))
    {
        HDMITX_DEBUG_PRINTF(("ERROR: read edid block 0\r\n"));
        goto __err_exit;
    }

    memset(&iTE6615_DATA.sink, 0, sizeof(iTE6615_DATA.sink));

    iTE6615_Parser_EDID_block0( edid_tmp );

    // read Ext block no
    extblock = edid_tmp[0x7E];

    if (extblock > 3) {
        HDMITX_DEBUG_PRINTF(("Warning: Extblock = %d \r\n", (int)extblock));
        extblock = 3;
    }

    for (i = 1; i <= extblock; i++) {

        if (FALSE == iTE6615_Get_EDID_Block(i, edid_tmp)) {
            HDMITX_DEBUG_PRINTF(("ERROR: read edid block %d\r\n", (int)i));
            goto __err_exit;
        }
        iTE6615_Parser_EDID_block1(edid_tmp);
    }

    // judge output ColorDepth by EDID
    //iTE6615_DATA.config.Output_ColorDepth = _Output_ColorDepth_;    // need to reset it
    if ( iTE6615_DATA.config.Output_ColorDepth != VID8BIT )
    {
        if ( iTE6615_DATA.config.Output_ColorDepth == VID16BIT && iTE6615_DATA.sink.dc_48bit == 0)
        {
            iTE6615_DATA.config.Output_ColorDepth = VID12BIT;
        }
        if ( iTE6615_DATA.config.Output_ColorDepth == VID12BIT && iTE6615_DATA.sink.dc_36bit == 0)
        {
            HDMITX_DEBUG_PRINTF(("Sink EDID set to 10B %d\r\n", (int)i));
            iTE6615_DATA.config.Output_ColorDepth = VID10BIT;
        }
        if ( iTE6615_DATA.config.Output_ColorDepth == VID10BIT && iTE6615_DATA.sink.dc_30bit == 0)
        {
            HDMITX_DEBUG_PRINTF(("Sink EDID set to 8B %d\r\n", (int)i));
            iTE6615_DATA.config.Output_ColorDepth = VID8BIT;
        }
    }

    if ( (iTE6615_DATA.sink.HDMIVer > 1) && (iTE6615_DATA.sink.VSIF2_MAX_TMDS != 0) )
    {
        iTE6615_DATA.sink.max_TMDS_clk = iTE6615_DATA.sink.VSIF2_MAX_TMDS;
    }
    else
    {
        iTE6615_DATA.sink.max_TMDS_clk = iTE6615_DATA.sink.VSIF1_MAX_TMDS;
    }

    HDMITX_DEBUG_PRINTF(( "### Sink is HDMI %d.x device, EnHDMI=%d\r\n", (int)iTE6615_DATA.sink.HDMIVer, (int)iTE6615_DATA.config.EnHDMI ));

    return TRUE;

__err_exit:
    return FALSE;
}




// output setting
void iTE6615_Set_Video_Infoframe(void)
{
    if(iTE6615_DATA.config.EnHDMI)
    {
        iTE6615_Set_AVI_Infoframe();
        iTE6615_Set_VendorSpecific_Infoframe();
        iTE6615_Set_DRM_Infoframe();
        hdmitxset(0xC8, 0x30, 0x30);    // Enable General Control Packet    // kuro can be setting to init
    }
}

void iTE6615_Set_AVI_Infoframe(void)
{
    u8 chksum, i;
    chgtxbank(1);

    // AVI InfoFrame
    hdmitxwr(0x58, iTE6615_DATA.config.Output_ColorSpace << 5);
    hdmitxset(0x59, 0x3F, iTE6615_CurVTiming.PicAR );
    //
    if(iTE6615_DATA.config.Output_ColorSpace == RGB444)
    {
        hdmitxset(0x59, 0xC0, 0x00);    // Colorimetry = No Data
        if(iTE6615_DATA.config.Output_RGB_QuantizationRange == RGB_RANGE_FULL)
        {
            hdmitxset(0x5A, 0x0C, 0x08);           // RGB Quantization Range = Full Range
        }
        else
        {
            hdmitxset(0x5A, 0x0C, 0x04);           // RGB Quantization Range = Limit Range
        }
    }
    else
    {
        if(iTE6615_DATA.config.Output_YCC_Colorimetry == ITU709)
        {
            hdmitxset(0x59, 0xC0, BIT7); // Colorimetry = ITU709
        }
        else if(iTE6615_DATA.config.Output_YCC_Colorimetry == ITU601)
        {
            hdmitxset(0x59, 0xC0, BIT6); // Colorimetry = ITU601
        }
    }

    hdmitxset(0x59, 0x3F, iTE6615_DATA.config.Input_Video_AspectRatio);

    if(iTE6615_DATA.config.Output_ExtendedColorimetry_Enable)
    {
        hdmitxset(0x59, 0xC0, 0xC0);  // Colorimetry = 11: Extended colorimetry information valid
        hdmitxwr(0x5A, iTE6615_DATA.config.Output_ExtendedColorimetry_Format); // ColorimetryExtended
    }

    // Setting VIC
    if ( iTE6615_DATA.Flag_Force_Set_VIC_0 )
    {
        hdmitxwr(0x5B, 0x00); // If VIC93~VIC95, VIC98, need force setting to 0 , cause VIC indicate by VSIF, If not 0, CTS fail.
    }
    else
    {
        hdmitxwr(0x5B, iTE6615_CurVTiming.VIC);
    }

    // pixel repeat times
    switch(iTE6615_CurVTiming.VIC)
    {
        case 6:case 7:case 8:case 9:
        case 21:case 22:case 23:case 24:
        case 44:case 45:case 50:case 51:
        case 54:case 55:case 58:case 59:
            iTE6615_DATA.config.PixRpt = 2;
            break;
        case 10: case 11: case 12: case 13: case 14: case 15:
        case 25: case 26: case 27: case 28: case 29: case 30:
        case 35: case 36: case 37: case 38:
            // setting by chip before 6805!
            break;
        default:
            iTE6615_DATA.config.PixRpt = 1;
    }
    hdmitxwr(0x5C, iTE6615_DATA.config.PixRpt - 1);

    hdmitxwr(0x5D, 0x00);
    hdmitxwr(0x5E, 0x00);
    hdmitxwr(0x5F, 0x00);
    hdmitxwr(0x60, 0x00);
    hdmitxwr(0x61, 0x00);
    hdmitxwr(0x62, 0x00);
    hdmitxwr(0x63, 0x00);
    hdmitxwr(0x64, 0x00);
    hdmitxwr(0x65, 0x00);

    chksum = 0x82;
    chksum += 0x02;
    chksum += 0x0D;
    for (i = 0x58; i <= 0x65; i++)
        chksum += hdmitxrd(i);

    hdmitxwr(0x5D, 0x100 - chksum);
    chgtxbank(0);
    hdmitxset(0xC7, 0x03, 0x03);     // Enable AVI InfoFrame
}

void iTE6615_Set_DRM_Infoframe()
{
    u8 chksum, i;

    if(!iTE6615_DATA.config.Infoframe_Enable_DRM)
    {
        chgtxbank(0);
        hdmitxset(0xC8, 0x03, 0x00);    // Disable Null PKT (DRM PKT)
        return;
    }

    // use NULL packet
    chgtxbank(1);
    hdmitxwr(0x38, 0x87);   // (HB0)
    hdmitxwr(0x39, 0x01);   // (HB1)
    hdmitxwr(0x3A, 0x1A);   // (HB2) (Length=26)

    for (i = 0x3C; i <= 0x55; i++)
    {
        if (iTE6615_DATA.config.Infoframe_DRM_ZERO)
            hdmitxwr(i, 0); // zeroed content for ATC
        else
            hdmitxwr(i, InfoFrame_DRM_DB[i-0x3C]); // dummy content for test
    }

    hdmitxwr(i, 0x00);      // PB27

    chksum = 0x87;  // (HB0)
    chksum += 0x01; // (HB1)
    chksum += 0x1A; // (HB2)
    for (i = 0x3C; i <= 0x55; i++)
        chksum += hdmitxrd(i);

    hdmitxwr(0x3B, 0x100 - chksum);
    chgtxbank(0);


    if(iTE6615_DATA.config.Infoframe_Enable_DRM == 1)
        hdmitxset(0xC8, 0x03, 0x03);    // Enable Null PKT (DRM PKT)
    else
        hdmitxset(0xC8, 0x03, 0x00);    // Enable Null PKT (DRM PKT)

    // xvYCC need colorimetry requires transmission of the gamut metadata, HDMI SPEC 6.7.3 Gamut-Related Metadata
    if (iTE6615_DATA.config.Output_ExtendedColorimetry_Format == xvYCC601
    || iTE6615_DATA.config.Output_ExtendedColorimetry_Format == xvYCC709)
    {
        chgtxbank(1);
        hdmitxwr(0xB0, 0x00);
        hdmitxwr(0xB1, 0xB0);
        hdmitxwr(0xB2, (iTE6615_DATA.config.Output_ColorDepth << 3) + (iTE6615_DATA.config.Output_ExtendedColorimetry_Format + 1));
        hdmitxwr(0xB3, 0x00);
        hdmitxwr(0xB4, 0x04);

        // The following GBD is used for 8-bit only
        if (iTE6615_DATA.config.Output_ExtendedColorimetry_Format == xvYCC601) {
            hdmitxwr(0xB5, 16);     // Black(Y)
            hdmitxwr(0xB6, 128);    // Black(Cb)
            hdmitxwr(0xB7, 128);    // Black(Cr)

            hdmitxwr(0xB8, 65);     // Red(Y)
            hdmitxwr(0xB9, 100);    // Red(Cb)
            hdmitxwr(0xBA, 212);    // Red(Cr)

            hdmitxwr(0xBB, 112);    // Green(Y)
            hdmitxwr(0xBC, 72);     // Green(Cb)
            hdmitxwr(0xBD, 58);     // Green(Cr)

            hdmitxwr(0xBE, 35);     // Blue(Y)
            hdmitxwr(0xBF, 212);    // Blue(Cb)
            hdmitxwr(0xC0, 114);    // Blue(Cr)
        } else {    // xvYCC709
            hdmitxwr(0xB5, 16);     // Black(Y)
            hdmitxwr(0xB6, 128);    // Black(Cb)
            hdmitxwr(0xB7, 128);    // Black(Cr)

            hdmitxwr(0xB8, 51);     // Red(Y)
            hdmitxwr(0xB9, 109);    // Red(Cb)
            hdmitxwr(0xBA, 212);    // Red(Cr)

            hdmitxwr(0xBB, 133);    // Green(Y)
            hdmitxwr(0xBC, 63);     // Green(Cb)
            hdmitxwr(0xBD, 52);     // Green(Cr)

            hdmitxwr(0xBE, 28);     // Blue(Y)
            hdmitxwr(0xBF, 212);    // Blue(Cb)
            hdmitxwr(0xC0, 120);    // Blue(Cr)
        }

        for (i = 0xC0; i <= 0xC6; i++)
            hdmitxwr(i, 0x00);

        chgtxbank(0);
        hdmitxwr(0xC6, 0x02);   // RegPktGBDUpd=1
        hdmitxwr(0xC6, 0x01);   // RegPktGBDEn=1
    }
}

void iTE6615_Set_VendorSpecific_Infoframe()
{
    u8 chksum, i;
    chgtxbank(0);

    // HDMI Vendor Specific InfoFrame
    if (iTE6615_DATA.config.Infoframe_Enable3D_DualView)
    {
        // DualView = TRUE = send as VSIF1.4,
        // Infoframe_Enable3D_DisparityData_present = VSIF 2.x present, so disable
        // Infoframe_Enable3D_ViewDependency = VSIF 2.x present, so disable
        iTE6615_DATA.config.Infoframe_Enable3D_DisparityData_present = FALSE;
        iTE6615_DATA.config.Infoframe_Enable3D_ViewDependency = FALSE;
    }

    if (iTE6615_DATA.config.Infoframe_Enable3D &&
       (iTE6615_DATA.config.Infoframe_Enable3D_DisparityData_present ||
        iTE6615_DATA.config.Infoframe_Enable3D_DualView ||
        iTE6615_DATA.config.Infoframe_Enable3D_ViewDependency))
    {
        // in SPEC HDMI2.x define
        chgtxbank(1);
        hdmitxset(0x90, 0x80, 0x80);    // Enable 2.x HD VSIF (1.4/2.x select reg)

        i = 0x92;
        hdmitxwr(i++, 0x01);                                    // (PB4) Version=1
        hdmitxwr(i++, 0x01);                                    // (PB5) 3D_Valid=1
        hdmitxwr(i++, (iTE6615_DATA.config.Infoframe_3D_Structure << 4) +    // (PB6) 3D_Structure
            ((iTE6615_DATA.config.Infoframe_Enable3D_DualView || iTE6615_DATA.config.Infoframe_Enable3D_ViewDependency) << 3) +// 3D_Additional_Info_present
            ((iTE6615_DATA.config.Infoframe_Enable3D_DisparityData_present) << 2)); // 3D_DisparityData_present, 3D_Meta_present=0

        if (iTE6615_DATA.config.Infoframe_3D_Structure == _3D_SideBySide_Half)
        {
            hdmitxwr(i++, 0x00);                                // 3D_Ext_Data
        }

        if (iTE6615_DATA.config.Infoframe_Enable3D_DualView)
        {
            hdmitxwr(i++, (iTE6615_DATA.config.Infoframe_Enable3D_DualView << 4));   // 3D_DualView=1, 3DViewDependency=0b00, 3D_Preferred2DView=0b00
        }
        else if (iTE6615_DATA.config.Infoframe_Enable3D_ViewDependency)
        {
            hdmitxwr(i++, (3 << 2) + 1);                    // 3D_DualView=0, 3DVideDependency=0b11, 3D_Preferred2DVies=0b01
        }

        if (iTE6615_DATA.config.Infoframe_Enable3D_DisparityData_present)
            hdmitxwr(i++, 0x00);                            // 3D_DisparityData_version=0b000, 3D_DisparitData_lenght=0x00

        hdmitxwr(0x90, i - 0x92 + 3);   // (HB2) Length

        for (; i <= 0xA9; i++)
            hdmitxwr(i, 0x00);  // Other Set to 0

        chksum = 0x81;  // (HB0)
        chksum += 0x01; // (HB1)
        chksum += hdmitxrd(0x90);
        chksum += 0xD8; // VSIF2.x IEEE OUI
        chksum += 0x5D; // VSIF2.x IEEE OUI
        chksum += 0xC4; // VSIF2.x IEEE OUI
        for (i = 0x92; i <= 0xA9; i++)
            chksum += hdmitxrd(i);

        hdmitxwr(0x91, 0x100 - chksum); // Chksum setting

        chgtxbank(0);
        hdmitxset(0xC7, 0xC0, 0xC0);    // Enable HDMI Vendor Specific InfoFrame

    }
    else if (iTE6615_DATA.config.Infoframe_Enable3D)
    {
        // HDMI1.4 VSIF
        chgtxbank(1);
        hdmitxset(0x90, 0x80, 0x00);    // Enable 1.4 HD VSIF (1.4/2.x select reg)

        if (iTE6615_DATA.config.Infoframe_3D_Structure == _3D_SideBySide_Half)
            hdmitxwr(0x90, 0x06);   // If _3D_SideBySide_Half, 3D_Ext_Data present
        else
            hdmitxwr(0x90, 0x05);   // If no _3D_SideBySide_Half, without 3D_Ext_Data

        hdmitxwr(0x92, 0x40);       // HDMI Video Format = 010, 3D format indication present.
        hdmitxwr(0x93, iTE6615_DATA.config.Infoframe_3D_Structure << 4);    // 3D_Structure
        hdmitxwr(0x94, 0x00);       // 3D_Ext_Data for _3D_SideBySide_Half, see Appendix H

        for (i = 0x95; i <= 0xA9; i++)
            hdmitxwr(i, 0x00);

        chksum = 0x81;  // (HB0)
        chksum += 0x01; // (HB1)
        chksum += hdmitxrd(0x90);
        chksum += 0x03; // VSIF1.4 IEEE OUI
        chksum += 0x0C; // VSIF1.4 IEEE OUI
        chksum += 0x00; // VSIF1.4 IEEE OUI
        for (i = 0x92; i <= 0xA9; i++)
            chksum += hdmitxrd(i);

        hdmitxwr(0x91, 0x100 - chksum);
        chgtxbank(0);
        hdmitxset(0xC7, 0xC0, 0xC0);    // Enable HDMI Vendor Specific InfoFrame
    }
    else if (iTE6615_DATA.Flag_Force_Set_VIC_0)
    {
        // If VIC93~VIC95, VIC98, need force setting to 0 , cause VIC indicate by VSIF, If not 0, CTS fail.
        chgtxbank(1);
        hdmitxset(0x90, 0x80, 0x00);    // Enable 1.4 HD VSIF (1.4/2.x select reg)
        hdmitxwr(0x90, 0x05);
        hdmitxwr(0x92, 0x20);           // HDMI Video Format = 001, 4Kx2K video present
        hdmitxwr(0x93, iTE6615_DATA.vars.HDMI_VIC);   // HDMI_VIC
        for (i = 0x94; i <= 0xA9; i++)
            hdmitxwr(i, 0x00);

        chksum = 0x81;
        chksum += 0x01;
        chksum += 0x05;
        chksum += 0x03;
        chksum += 0x0C;
        chksum += 0x00;
        for (i = 0x92; i <= 0xA9; i++)
            chksum += hdmitxrd(i);

        hdmitxwr(0x91, 0x100 - chksum);
        chgtxbank(0);
        hdmitxset(0xC7, 0xC0, 0xC0);    // Enable HDMI Vendor Specific InfoFrame
    }
    else
    {
        hdmitxset(0xC7, 0xC0, 0x00);    // Disable HDMI Vendor Specific InfoFrame
    }
}

void iTE6615_Set_Video_ColorSpace(void)
{
    _CSCMtx_Type CSCMtx_Type = CSCMtx_Unknown;

    hdmitxset(0x91, 0x03, iTE6615_DATA.config.Input_ColorSpace);
    hdmitxset(0xC0, 0x01, iTE6615_DATA.config.EnHDMI);

    if (iTE6615_DATA.config.Output_ColorDepth == VID8BIT || iTE6615_DATA.config.Output_ColorSpace == YUV422)
        hdmitxset(0xC1, 0x70, 0);
    else
        hdmitxset(0xC1, 0x70, (0x04 + iTE6615_DATA.config.Output_ColorDepth) << 4);

    // CSC
    if (iTE6615_DATA.config.Input_ColorSpace  == RGB444 &&
       (iTE6615_DATA.config.Output_ColorSpace == YUV444 ||
        iTE6615_DATA.config.Output_ColorSpace == YUV422))
    {
        // Input RGB to YUV
        printf("................Input RGB to YUV ..................\n");
        hdmitxset(0xA0, 0x03, 0x02);    // Reg_CSCSel = RGB to YUV
        switch(iTE6615_DATA.config.Input_RGB_QuantizationRange)
        {
            case RGB_RANGE_FULL:
                if(iTE6615_DATA.config.Output_YCC_Colorimetry == ITU601)
                    CSCMtx_Type = CSCMtx_RGB2YUV_ITU601_00_255; // RGB Full Range to ITU601
                else
                    CSCMtx_Type = CSCMtx_RGB2YUV_ITU709_00_255; // RGB Full Range to ITU709
                break;
            case RGB_RANGE_LIMIT:
                if(iTE6615_DATA.config.Output_YCC_Colorimetry == ITU601)
                    CSCMtx_Type = CSCMtx_RGB2YUV_ITU601_16_235;
                else
                    CSCMtx_Type = CSCMtx_RGB2YUV_ITU709_16_235;
                break;
        }

    }
    else if ((iTE6615_DATA.config.Input_ColorSpace == YUV444 ||
              iTE6615_DATA.config.Input_ColorSpace == YUV422 ||
              iTE6615_DATA.config.Input_ColorSpace == YUV420) &&
              iTE6615_DATA.config.Output_ColorSpace == RGB444)
    {
        // Input YUV to RGB, to RGB always to Full Range
        printf("................Input YUV to RGB ..................\n");
        hdmitxset(0xA0, 0x03, 0x03);    // Reg_CSCSel = YUV to RGB
        switch(iTE6615_DATA.config.Output_RGB_QuantizationRange)
        {
            case RGB_RANGE_FULL:
                if(iTE6615_DATA.config.Input_YCC_Colorimetry == ITU601)
                {
                    CSCMtx_Type = CSCMtx_YUV2RGB_ITU601_00_255;
                }
                else if(iTE6615_DATA.config.Input_YCC_Colorimetry == ITU709)
                {
                    CSCMtx_Type = CSCMtx_YUV2RGB_ITU709_00_255;
                }
                break;
            case RGB_RANGE_LIMIT:
                if(iTE6615_DATA.config.Input_YCC_Colorimetry == ITU601)
                {
                    CSCMtx_Type = CSCMtx_YUV2RGB_ITU601_16_235;
                }
                else if(iTE6615_DATA.config.Input_YCC_Colorimetry == ITU709)
                {
                    CSCMtx_Type = CSCMtx_YUV2RGB_ITU709_16_235;
                }
        }

    }
    else
    {
        printf("................Input RGB to RGB  ..................\n");
        // RGB to RGB / YUV to YUV
        hdmitxset(0xA0, 0x03, 0x00);    // Reg_CSCSel = No Color Space conversion
    }

    if(CSCMtx_Type != CSCMtx_Unknown)
    {
        iTE6615_SetTX_CSCMetrix((u8 *)&CSC_Matrix[CSCMtx_Type][0]);
        hdmitxset(0x25, 0x04, 0x00);
    }else{
        hdmitxset(0x25, 0x04, 0x04);
    }
}

void iTE6615_Set_HDMI2_SCDC( void )
{
    u8 EnFlagPolling, rddata;
    u8 Enable_HDMI2_Encode, Enable_HDMI2_Scramble, Enable_HDMI2_ClkRatio401;

    // if not HDMI 2.0
    // 20160510 add (iTE6615_DATA.VCLK <= 320000)
    // : the FW will force H2.0 encode if not a H2.0 EDID
    if ( (iTE6615_DATA.sink.HDMIVer < 2) && ( iTE6615_DATA.VCLK <= ((u32)320000)) )
    {
        Enable_HDMI2_Encode = FALSE;
        Enable_HDMI2_Scramble = FALSE;
        Enable_HDMI2_ClkRatio401 = FALSE;
    }
    else
    {
        if ( iTE6615_DATA.VCLK <= ((u32)340000) )
        {
            // check if sink support  LTE_340Mcsc ( accept scrambling less than 3.4G )
            Enable_HDMI2_Encode &= iTE6615_DATA.sink.lte_340M_csc_scramble; // EDID Support scrambling for TMDS character Rate or below 340Mcsc.
            Enable_HDMI2_Scramble = Enable_HDMI2_Encode;
            Enable_HDMI2_ClkRatio401 = FALSE;
        } else {
            Enable_HDMI2_Encode = TRUE;
            Enable_HDMI2_Scramble = TRUE;
            Enable_HDMI2_ClkRatio401 = TRUE;
        }

    }

    EnFlagPolling =  iTE6615_DATA.sink.scdc_present;

    // When EnH2DetRR is enabled, EnFlagPolling is disabled.
    if (iTE6615_DATA.sink.rr_capable)
    {
        EnFlagPolling = FALSE;
    }

    //hdmitxset(0x83, 0x04, (Enable_HDMI2_ClkRatio401 << 2)); // HDMI2ONPLL
    hdmitxset(0xC0, 0x46, (Enable_HDMI2_ClkRatio401 << 6) + (Enable_HDMI2_Scramble << 2) + (Enable_HDMI2_Encode << 1));

    delay1ms(2);

    hdmitxset(0x3A, 0x03, (iTE6615_DATA.sink.rr_capable << 1) + EnFlagPolling);

    if (EnFlagPolling)
        hdmitxset(0x1E, 0x10, 0x10);   // Enable SCDC update flag change interrupt

    if (iTE6615_DATA.sink.rr_capable)
        hdmitxset(0x1E, 0x20, 0x20);   // Enable SCDC detect read request interrupt

    if (Enable_HDMI2_Encode)  /* &&  iTE6615_DATA.sink.scdc_present*/
    {
        iTE6615_SCDC_Write(0x02, 0x01);
        if (iTE6615_SCDC_Read(0x02, 1) == TRUE)
        {
            rddata = hdmitxrd(0x30);
            HDMITX_DEBUG_PRINTF(("RX SCDC Sink Version = 0x%02X\r\n",(int) rddata));
        }

        iTE6615_SCDC_Write(0x20, (Enable_HDMI2_ClkRatio401 << 1) + (Enable_HDMI2_Scramble << 0));
        if (iTE6615_SCDC_Read(0x20, 1) == TRUE)
        {
            rddata = hdmitxrd(0x30);
            HDMITX_DEBUG_PRINTF(("RX SCDC H2ClkRatio=%d, EnH2Scr=%d\r\n",(int) (rddata & 0x02) >> 1,(int) rddata & 0x01));
        }

    }
    else if ((iTE6615_DATA.sink.HDMIVer >= 2) &&  iTE6615_DATA.sink.scdc_present)
    {
        iTE6615_SCDC_Write(0x20, 0x00);
        if (iTE6615_SCDC_Read(0x20, 1) == TRUE)
        {
            rddata = hdmitxrd(0x30);
            HDMITX_DEBUG_PRINTF(("RX SCDC H2ClkRatio=%d, EnH2Scr=%d\r\n",(int) (rddata & 0x02) >> 1,(int) rddata & 0x01));
        }
    }
    else
    {
        iTE6615_SCDC_Write(0x20, 0x00);    // kuro test
        HDMITX_DEBUG_PRINTF(("RX SCDC k H2ClkRatio=%d, EnH2Scr=%d\r\n", (rddata & 0x02) >> 1, rddata & 0x01));
    }

    if (iTE6615_DATA.sink.rr_capable)
    {
        if ( iTE6615_SCDC_Set_RR(ENABLE) )
        {
            delay1ms(5);
            iTE6615_SCDC_IRQ();
        }
    }
    else if ((iTE6615_DATA.sink.HDMIVer >= 2) &&  iTE6615_DATA.sink.scdc_present)
    {
        iTE6615_SCDC_Set_RR(DISABLE);
    }
}

// TASK
void iTE6615_TASK_Handler( void )
{
    u8 bcaps, ksvready;

    if(!iTE6615_DATA.Flag_SYS_Tick_Enable) return;

    iTE6615_DATA.task.sys_tick_inc = hdmitxrd(0x09)&0x7f;

    if ( iTE6615_DATA.task.avi_cmd_timeout )
    {
        iTE6615_DATA.task.avi_cmd_tick = iTE6615_DATA.task.sys_tick_inc;
        if ( iTE6615_DATA.task.avi_cmd_tick >= iTE6615_DATA.task.avi_cmd_timeout )
        {
            HDMITX_DEBUG_PRINTF(("iTE6615_TASK_Handler avi (%d, %d) ...\r\n", (int)iTE6615_DATA.task.avi_cmd, (int)iTE6615_DATA.task.avi_cmd_tick));
            iTE6615_DATA.task.avi_cmd_timeout = 0;
            switch (iTE6615_DATA.task.avi_cmd)
            {
                case AVICMD_SYS_UNPLUG:
                    iTE6615_sys_chg(iTE6615_STATES_Unplug);
                    break;
                case AVICMD_SHOW_AUD_INFO:
                    iTE6615_Show_AudioCTS();
                    break;
                case AVICMD_SHOW_VID_INFO:
                    iTE6615_Show_VideoInfo();

                    break;
                default:HDMITX_DEBUG_PRINTF(("iTE: avi_cmd default cmd\n"));break;
            }

        }
    }

    if ( iTE6615_DATA.task.hdcp_cmd_timeout )
    {

        iTE6615_DATA.task.hdcp_cmd_tick = iTE6615_DATA.task.sys_tick_inc;

        if ( iTE6615_DATA.task.hdcp_cmd_tick >= iTE6615_DATA.task.hdcp_cmd_timeout )
        {
            iTE6615_DATA.task.hdcp_cmd_timeout = 0;
            hdmitxwr(0x09, 0x00); // todo: check this
            HDMITX_DEBUG_PRINTF(("iTE: iTE6615_TASK_Handler hdcp (%d, %d) ... \r\n", (int)iTE6615_DATA.task.hdcp_cmd, (int)iTE6615_DATA.task.hdcp_cmd_tick));

            switch (iTE6615_DATA.task.hdcp_cmd)
            {
                case CMD_HDCP_AUTH_RETRY:
                    HDMITX_DEBUG_PRINTF(("iTE: CMD_HDCP_AUTH_RETRY, STATEV=%d \r\n", (int)iTE6615_DATA.STATEV));
                    if (iTE6615_DATA.STATEV == iTE6615_STATEV_VidOutEnable)
                    {
                        delay1ms(200);
                        iTE6615_hdcp_chg( STATEH_Reset );
                        iTE6615_hdcp_chg( STATEH_CPGoing );
                    }
                    else
                    {
                        iTE6615_DATA.Flag_SYS_Tick_Enable = 0;
                        iTE6615_sys_chg(iTE6615_STATES_Unplug);
                    }
                    break;

                case CMD_HDCP_POLLING_BCAPS:
                    HDMITX_DEBUG_PRINTF(("iTE: CMD_HDCP_POLLING_BCAPS\r\n"));
                    ksvready = FALSE;

                    if ( iTE6615_DATA.task.hdcp_cmd_count > 50 ) { // polling over 5 seconds
                        HDCP_DEBUG_PRINTF(("iTE:  ksv list not read, hdcp fail...\r\n"));
                        iTE6615_TASK_HDCP_RetryAuth();
                        break;
                    }

                    if ( (iTE6615_DATA.task.hdcp_cmd_count & 0x1) == 0 ) { // polling every 200ms

                        iTE6615_HDCP_Read(0x40, 1); // BCaps
                        bcaps = hdmitxrd(0x63);
                        if((bcaps &0x20) ==0x00)
                        {
                            HDCP_DEBUG_PRINTF(("iTE:  HDCP KSV list not ready ... \r\n"));
                        }
                        else
                        {
                            ksvready = TRUE;
                            HDCP_DEBUG_PRINTF(("iTE:  HDCP KSV list ready ... \r\n"));
                        }
                    }

                    iTE6615_DATA.task.hdcp_cmd_count++;

                    if(ksvready == TRUE){
                        // do not break and check ksv list immediatelly
                        // iTE6615_DATA.vars.hdcp_cmd = CMD_HDCP_KSV_CHK;
                        iTE6615_TASK_HDCP_CheckKSVList();
                        break;
                    }
                    else {
                        // enable next polling
                        iTE6615_DATA.task.hdcp_cmd_timeout = 1;
                        iTE6615_DATA.task.hdcp_cmd_tick = 0;
                        hdmitxwr(0x09, 0x80);
                        break;
                    }

                case CMD_HDCP_KSV_CHK:
                    HDCP_DEBUG_PRINTF(("iTE: CMD_HDCP_KSV_CHK\n"));
                    if(iTE6615_HDCPRepeater_CheckSupport() == TRUE)
                    {
                        hdmitxset(0x42, 0x02, 0x02);
                        HDCP_DEBUG_PRINTF(("iTE: SHA Check Result = PASS\r\n"));
                    }
                    else{

                        hdmitxset(0x42, 0x06, 0x06);
                        HDCP_DEBUG_PRINTF(("iTE :SHA Check Result = FAIL\r\n"));
                    }
                    break;
                default:
                    HDMITX_DEBUG_PRINTF(("iTE: default cmd\n"));
                    break;
            }
        }
    }

    if ( iTE6615_DATA.task.avi_cmd_timeout == 0 && iTE6615_DATA.task.hdcp_cmd_timeout == 0 )
    {
        hdmitxwr(0x09, 0x00);
    }


}

void iTE6615_TASK_HDCP_RetryAuth( void )
{
    HDMITX_DEBUG_PRINTF(("---> iTE6615_TASK_HDCP_RetryAuth ::"));
    iTE6615_DATA.task.hdcp_cmd = CMD_HDCP_AUTH_RETRY;
    iTE6615_DATA.task.hdcp_cmd_tick = 0;
    iTE6615_DATA.task.hdcp_cmd_timeout = 3;    // do command after 3*100 ms
    iTE6615_SetTX_Timestamp(ENABLE);
}

void iTE6615_TASK_HDCP_CheckKSVList( void )
{
    HDMITX_DEBUG_PRINTF(("---> iTE6615_TASK_HDCP_CheckKSVList ::"));
    iTE6615_DATA.task.hdcp_cmd = CMD_HDCP_KSV_CHK;
    iTE6615_DATA.task.hdcp_cmd_tick = 0;
    iTE6615_DATA.task.hdcp_cmd_timeout = 1;    // do command after 1*100 ms
    iTE6615_SetTX_Timestamp(ENABLE);
}

void iTE6615_TASK_HDCP_PollingBCaps( void )
{
    HDMITX_DEBUG_PRINTF(("---> iTE6615_TASK_HDCP_PollingBCaps ::"));
    iTE6615_DATA.task.hdcp_cmd = CMD_HDCP_POLLING_BCAPS;
    iTE6615_DATA.task.hdcp_cmd_tick = 0;
    iTE6615_DATA.task.hdcp_cmd_count = 0;
    iTE6615_DATA.task.hdcp_cmd_timeout = 1;    // do command after 1*100 ms
    iTE6615_SetTX_Timestamp(ENABLE);
}

void iTE6615_TASK_AVInfo_SysUnplug( void )
{
    iTE6615_DATA.task.avi_cmd = AVICMD_SYS_UNPLUG;
    iTE6615_DATA.task.avi_cmd_tick = 0;
    iTE6615_DATA.task.avi_cmd_timeout = 1;    // do command after 100 ms
    iTE6615_SetTX_Timestamp(ENABLE);
}

void iTE6615_TASK_AVInfo_ShowAudioInfo( void )
{
    iTE6615_DATA.task.avi_cmd = AVICMD_SHOW_AUD_INFO;
    iTE6615_DATA.task.avi_cmd_tick = 0;
    iTE6615_DATA.task.avi_cmd_timeout = 20;    // do command after 2000 ms
    iTE6615_SetTX_Timestamp(ENABLE);
}

void iTE6615_TASK_AVInfo_ShowVideoInfo( void )
{
    iTE6615_DATA.task.avi_cmd = AVICMD_SHOW_VID_INFO;
    iTE6615_DATA.task.avi_cmd_tick = 0;
    iTE6615_DATA.task.avi_cmd_timeout = 10;    // do command after 1000 ms
    iTE6615_SetTX_Timestamp(ENABLE);
}


void iTE6615_Identify_Chip(void)
{
    u8 Result = 0;
    u8 REG00;
    u8 REG01;
    u8 REG02;
    u8 REG03;

    do
    {
        REG00 = hdmitxrd(0x00);
        REG01 = hdmitxrd(0x01);
        REG02 = hdmitxrd(0x02);
        REG03 = hdmitxrd(0x03);
        if( REG00 == 0x54 && REG01 == 0x49 && REG02 == 0x64 && REG03 == 0x62)
        {
            Result = 1;
        }
        else
        {
            HDMITX_DEBUG_PRINTF(("This is not iTE6264,6615 chip !!!\r\n"));
            HDMITX_DEBUG_PRINTF(("REG00 = %X\n",(int)REG00));
            HDMITX_DEBUG_PRINTF(("REG01 = %X\n",(int)REG01));
            HDMITX_DEBUG_PRINTF(("REG02 = %X\n",(int)REG02));
            HDMITX_DEBUG_PRINTF(("REG03 = %X\n",(int)REG03));
            Result = 0;
        }

    }while(Result==0);

    iTE6615_DATA.ChipID = hdmitxrd(0x04);
    HDMITX_DEBUG_PRINTF(("ChipID = %X !!!\r\n",(int) iTE6615_DATA.ChipID ));
}


