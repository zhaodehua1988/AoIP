///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_DRV_TX.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/
#include "iTE6615_Global.h"
#include "iTE6615_Table_VM.h"
#include "iTE6615_common.h"
extern iTE6615_GlobalDATA   iTE6615_DATA;
extern iTE6615_VTiming      iTE6615_CurVTiming;



u8 iTE6615_HDCP_Check_Revocation_List[][5]=
{
    {0xa6, 0x50, 0x3D, 0x09, 0xFD},    // HDCP Check Revocation List
};


// Check Block
u8 iTE6615_CheckTX_RXSENSE(void)
{
    return hdmitxrd(0x07)&BIT2;
}

u8 iTE6615_CheckRX_SCDT(void)
{
    return ((hdmitxrd(0x07)&0x18)==0x18);
}

u8 iTE6615_CheckTX_HPD(void)
{
    //delay1ms(1000);
    return hdmitxrd(0x07)&BIT1;
}

u8 iTE6615_Check_RCLK_Valid( u32 rclk )
{
    #define RCLK_DIFF  30UL
    #define RCLK_MAX  (RCLK_TYPICAL*(100UL+RCLK_DIFF)/100UL)
    #define RCLK_MIN  (RCLK_TYPICAL*(100UL-RCLK_DIFF)/100UL)

    if ( (rclk <= RCLK_MAX) && (rclk >=RCLK_MIN) ) {
        return TRUE;
    }
    HDMITX_AUDIO_PRINTF(("RCLK_MAX=%d,RCLK_MIN=%d,RCLK=%ld, max=%ld, min=%ld, avg=%ld, range=%ld\r\n",RCLK_MAX,RCLK_MIN, rclk, (u32)RCLK_MAX / 1000, (u32)RCLK_MIN / 1000, (u32)RCLK_TYPICAL / 1000, (u32)RCLK_DIFF));
    return FALSE;
}

u8 iTE6615_CheckConfig_Audio(void)
{
    u8 result = FALSE;

    if (iTE6615_DATA.config.Audio_Type == LPCM) {
        switch (iTE6615_DATA.config.Audio_Frequency) {
        case AUD32K:
        case AUD64K:
        case AUD128K:
        case AUD44K:
        case AUD88K:
        case AUD176K:
        case AUD48K:
        case AUD96K:
        case AUD192K:
            result = TRUE;
            break;
        default:
            HDMITX_AUDIO_PRINTF(("ERROR: Incorrect LPCM Audio Sampling Frequency !!!\r\n"));
            break;
        }
    } else if (iTE6615_DATA.config.Audio_Type == NLPCM) {
        switch (iTE6615_DATA.config.Audio_Frequency) {
        case AUD32K:
        case AUD44K:
        case AUD48K:
        case AUD88K:
        case AUD96K:
        case AUD176K:
        case AUD192K:
            result = TRUE;
            break;
        default:
            HDMITX_AUDIO_PRINTF(("ERROR: Incorrect NLPCM Audio Sampling Frequency !!!\r\n"));
            break;
        }
    } else if (iTE6615_DATA.config.Audio_Type == DSD) {
        switch (iTE6615_DATA.config.Audio_Frequency) {
        case AUD32K:
        case AUD44K:
        case AUD48K:
        case AUD88K:
        case AUD96K:
        case AUD176K:
        case AUD192K:
            result = TRUE;
            break;
        default:
            HDMITX_AUDIO_PRINTF(("ERROR: Incorrect DSD Audio Sampling Frequency !!!\r\n"));
            break;
        }
    } else if (iTE6615_DATA.config.Audio_Type == HBR) {
        switch (iTE6615_DATA.config.Audio_Frequency) {
        case AUD256K:
        case AUD352K:
        case AUD384K:
        case AUD512K:
        case AUD705K:
        case AUD768K:
        case AUD1024K:
        case AUD1411K:
        case AUD1536K:
            result = TRUE;
            break;
        default:
            HDMITX_AUDIO_PRINTF(("ERROR: Incorrect HBR Audio Sampling Frequency !!!\r\n"));
            break;
        }
    } else {
        HDMITX_AUDIO_PRINTF(("ERROR: Unknown Audio Type !!!\r\n"));
        result = FALSE;
    }

    if (iTE6615_DATA.config.Audio_Enable_MSAudio == TRUE && (iTE6615_DATA.config.Audio_ChannelCount == 1 || iTE6615_DATA.config.Audio_ChannelCount == 2)) {
        HDMITX_AUDIO_PRINTF(("ERROR: Incorrect Multi-Stream Audio Channel Setting !!!\r\n"));
        result = FALSE;
    }

    // auto config part
    if (iTE6615_DATA.config.Audio_ChannelCount > 8)
    {
        iTE6615_DATA.config.Audio_Enable_3DAudio = TRUE;
        iTE6615_DATA.config.Audio_Enable_MSAudio = FALSE;
    }
    else
    {
        iTE6615_DATA.config.Audio_Enable_3DAudio = FALSE;
    }

    if (iTE6615_DATA.config.Audio_Enable_TDMAudio)
    {
        iTE6615_DATA.config.Audio_Output_Sel = I2S;
    }


    return result;
}

u8 iTE6615_CheckConfig_Video(void)
{
    u8 flag = TRUE;


#if iTE6264
    iTE6615_DATA.config.TTL_Enable_DDR = 0;    // If 6264 DDR must setting to SDR or nonstable
    if(iTE6615_DATA.config.Input_Enable_DualMode == 1){iTE6615_DATA.config.LVDS_RxChNum = LVDS4CH;}
    else                                              {iTE6615_DATA.config.LVDS_RxChNum = LVDS2CH;}
#endif

    if (iTE6615_DATA.config.Input_ColorSpace != YUV420 &&
        iTE6615_DATA.config.Output_ColorSpace == YUV420)
    {
        HDMITX_DEBUG_PRINTF(("ERROR: Incorrect Video Format Setting !!!\r\n"));
        flag = FALSE;
    }

    if (iTE6615_DATA.config.Output_ExtendedColorimetry_Enable)
    {
        if (iTE6615_DATA.config.Output_ColorSpace == RGB444 &&
            iTE6615_DATA.config.Output_ExtendedColorimetry_Format != AdobeRGB &&
            iTE6615_DATA.config.Output_ExtendedColorimetry_Format != BT2020YCC) {
            HDMITX_DEBUG_PRINTF(("ERROR: RGB444 must set Output_ExtendedColorimetry_Format to AdobeRGB or BT2020YCC !!!\r\n"));
            flag = FALSE;
        }

        if (iTE6615_DATA.config.Output_ColorSpace != RGB444 &&
            iTE6615_DATA.config.Output_ExtendedColorimetry_Format == AdobeRGB) {
            HDMITX_DEBUG_PRINTF(("ERROR: YCbCr can not set Output_ExtendedColorimetry_Format to AdobeRGB !!!\r\n"));
            flag = FALSE;
        }
    }

    if(iTE6615_DATA.config.TTL_Enable_SyncEmbedd)
    {
        iTE6615_DATA.config.Input_ColorSpace = YUV422;
    }

    if(iTE6615_DATA.config.TTL_Enable_SyncEmbedd)
    {
        iTE6615_DATA.config.EnH2InLBBypass = TRUE;
    }
    else if(iTE6615_DATA.config.Input_Enable_DualMode_Mode ||
    (iTE6615_DATA.config.Input_ColorSpace == YUV420 &&iTE6615_DATA.config.Output_ColorSpace != YUV420))
    {
        iTE6615_DATA.config.EnH2InLBBypass = FALSE;
    }


    return flag;
}

// EDID read
u8 iTE6615_Get_EDID_Block(u8 block, u8 *edid_buffer)
{
    u8 offset;
    u8 i;
    u8 read_len = 16;
    u8 retry = 3;

__RETRY:
    offset = 0;
    for (i = 0; i < 128 / read_len; i++)
    {
        if (iTE6615_DDC_Read(block, offset, read_len, edid_buffer))
        {

            edid_buffer += read_len;
            offset += read_len;
            continue;
        }
        else
        {
            if ( retry > 0 )
            {
                retry--;
                goto __RETRY;
            }
            else
            {
                HDMITX_DEBUG_PRINTF(("ERROR: read edid block 0, offset %d, length %d fail.\r\n", (int)offset, (int)read_len));
                return FALSE;
            }
        }
    }

    return TRUE;
}



// SCDC
u8 iTE6615_SCDC_Read(u8 offset, u8 bytenum)
{
    u8 result;

    if (!iTE6615_CheckTX_HPD())
    {
        return FALSE;
    }

    hdmitxset(0x28, 0x01, 0x00);
    hdmitxwr(0x2E, 0x09);           // DDC FIFO Clear
    hdmitxwr(0x29, 0xA8);           // Header
    hdmitxwr(0x2A, offset);         // Offset
    hdmitxwr(0x2B, bytenum);        // ReqByte[7:0]
    hdmitxset(0x2C, 0x03, 0x00);    // ReqByte[9:8]
    hdmitxwr(0x2E, 0x00);           // Sequential Burst Read
    hdmitxset(0x28, 0x01, 0x00);

    result = iTE6615_DDC_Wait();
    hdmitxset(0x28, 0x01, 0x00);

    return result;
}

u8 iTE6615_SCDC_Write(u8 offset, u8 scdc_data)
{
    u8 result;

    if (!iTE6615_CheckTX_HPD())
    {
        return FALSE;
    }

    hdmitxset(0x28, 0x01, 0);       // DDC Master set to FW
    hdmitxwr(0x2E, 0x09);           // DDC FIFO Clear
    hdmitxwr(0x29, 0xA8);           // I2C address
    hdmitxwr(0x2A, offset);         // Offset
    hdmitxwr(0x2B, 0x01);           // ReqByte[7:0]
    hdmitxset(0x2C, 0x03, 0x00);    // ReqByte[9:8]
    hdmitxwr(0x30, scdc_data);      // WrData
    hdmitxwr(0x2E, 0x01);           // Sequential Burst Write

    result = iTE6615_DDC_Wait();
    hdmitxset(0x28, 0x01, 0x00);
    return result;
}

u8 iTE6615_SCDC_Set_RR(u8 rr_enable)
{
    u8 rddata;
    u8 retry;

    if ( iTE6615_DATA.sink.HDMIVer < 2 ) {
        return FALSE;
    }

    if ( (iTE6615_DATA.sink.rr_capable == 0) && rr_enable)  {
        return FALSE;
    }

    for( retry = 0 ; retry < 3 ; retry++ )
    {
        if ( iTE6615_SCDC_Write(0x30, rr_enable) )
        {
            iTE6615_SCDC_Read(0x30, 1);
            rddata = hdmitxrd(0x30);
            if ( rr_enable == (rddata&0x01) )
            {
                HDMITX_DEBUG_PRINTF(("RR_Enable=%d\r\n", rr_enable));
                return TRUE;
            }
            else{
                HDMITX_DEBUG_PRINTF(("iTE6615_SCDC_Set_RR fail %d != %d\r\n", (int)rr_enable, (int)rddata));
            }
        }
        delay1ms(5);
    }

    return FALSE;
}

void iTE6615_SCDC_RR_Cancel(void)
{
    u8 rddata;
    u8 retry;

    for( retry = 0 ; retry < 2 ; retry++ )
    {
        if ( iTE6615_SCDC_Write(0x30, 0x00) )
        {
            iTE6615_SCDC_Read(0x30, 1);
            rddata = hdmitxrd(0x30);
            if ( 0 == (rddata&0x01) ) {
                break;
            }
        }
        delay1ms(5);
    }
}



// DDC
#define DDCWAITNUM 200
u8 iTE6615_DDC_Wait(void)
{
    u16 ddc_wait_count;
    u8  ddc_status;

    ddc_wait_count = 0;
    do {
        ddc_wait_count++;
        delay1ms(1);
    } while ((hdmitxrd(0x2F) & 0x80) == 0x00 && ddc_wait_count < DDCWAITNUM);

    if (ddc_wait_count == DDCWAITNUM)
    {
        ddc_status = hdmitxrd(0x2F) & 0xFE;

        HDMITX_DEBUG_PRINTF(("ERROR: DDC Bus Wait TimeOut, Status = %02X,",(int) ddc_status));
        if (hdmitxrd(0x16) & BIT3)
        {
            HDMITX_DEBUG_PRINTF(("DDC Bus Hang !!!\r\n"));
            hdmitxset(0x35, 0x10, 0x10);    // RegDDCMasterRst=1
            hdmitxset(0x35, 0x10, 0x00);    // RegDDCMasterRst=0
            hdmitxwr(0x2E, 0x0F);  // Abort DDC Command
        }
        else if(hdmitxrd(0x12)&0x04)
        {
            HDMITX_DEBUG_PRINTF(("DDC Command Fail !!!\r\n"));
            hdmitxset(0x35, 0x10, 0x10);    // RegDDCMasterRst=1
            hdmitxset(0x35, 0x10, 0x00);    // RegDDCMasterRst=0
            hdmitxwr(0x2E, 0x0F);  // Abort DDC Command
        }
        else if (ddc_status & 0x20){HDMITX_DEBUG_PRINTF(("DDC NoACK !!!\r\n"));}
        else if (ddc_status & 0x10){HDMITX_DEBUG_PRINTF(("DDC WaitBus !!!\r\n"));}
        else if (ddc_status & 0x08){HDMITX_DEBUG_PRINTF(("DDC ArbiLose !!!\r\n"));}
        else {HDMITX_DEBUG_PRINTF(("UnKnown Issue !!!\r\n"));}

        hdmitxset(0x28, 0x01, 0x01);
        hdmitxset(0x35, 0x10, 0x10);
        hdmitxset(0x35, 0x10, 0x00);
        hdmitxset(0x28, 0x01, 0x00);
        return FALSE;  // will try again
    }
    else
    {
        return TRUE;
    }

}

u8 iTE6615_DDC_Read(u8 block, u8 offset, u16 length, u8 *buffer)
{
    u8 result = FALSE;
    u32 off = block*128 + offset;
    u8 segment = off / 256;
    u8 retry = 0;

    offset = off % 256;

__RETRY:

    hdmitxset(0x28, 0x01, 0x01);    // Enable PC DDC Mode
    hdmitxset(0x19, 0x04, 0x04);    // Enable DDC Command Fail Interrupt
    hdmitxset(0x1D, 0x08, 0x08);    // Enable DDC Bus Hang Interrupt

    hdmitxwr(0x2E, 0x09);                  // DDC FIFO Clear
    hdmitxwr(0x29, 0xA0);                  // EDID Address
    hdmitxwr(0x2A, offset);                // EDID Offset
    hdmitxwr(0x2B, length);                // Read ByteNum[7:0]
    hdmitxwr(0x2C, (length & 0x300) >> 8); // Read ByteNum[9:8]
    hdmitxwr(0x2D, segment / 2);           // EDID Segment

    if (iTE6615_CheckTX_HPD())
    {
        hdmitxwr(0x2E, 0x03);  // EDID Read Fire

        if (iTE6615_DDC_Wait())
        {
            hdmitxbrd(0x30, length, buffer);
            result = TRUE;
        }
        else
        {
            HDMITX_DEBUG_PRINTF(("ERROR: DDC EDID Read Fail !!!\r\n"));
            if ( retry > 0 )
            {
                retry--;
                delay1ms(100);
                goto __RETRY;
            }
        }
    }

    hdmitxset(0x28, 0x01, 0x00);   // Disable PC DDC Mode
    return result;
}


void iTE6615_DDC_FIFOClear(void)
{
    u8 reg = hdmitxrd(0x28);
    hdmitxwr(0x28, reg|0x01);
    hdmitxwr(0x2E, 0x09);
    hdmitxwr(0x28, reg);
}


// Set
void iTE6615_SetTX_AVMute(u8 mute)
{
    u8 reg;
    u8 retry = 10;

__rewrite:
    retry--;
    if ( retry == 0 ) {
        return;
    }
    hdmitxset(0xC1, 0x01, mute);
    reg = hdmitxrd(0xc1);
    if ( mute == 0 )
    {
        if ( reg & 0x01 )
        {
            HDMITX_DEBUG_PRINTF(("** SetTX_AVMute error mute=%d\r\n", (int)mute));
            goto __rewrite;
        }
    }
    else
    {
        if ( (reg & 0x01)==0 )
        {
            HDMITX_DEBUG_PRINTF(("** SetTX_AVMute error mute=%d\r\n", (int)mute));
            goto __rewrite;
        }
    }
}

void iTE6615_SetTX_Power(u8 POWER_STATE)
{
    if(POWER_STATE == POWER_ON)
    {
        HDMITX_DEBUG_PRINTF(("Power On HDMITX \r\n"));

        hdmitxset(0x0C, 0x01, 0x00);   // enable IDDQ
        hdmitxwr(0xFB, (ADDR_LVDSRX|0x00));   // disable LVDS

        #if iTE6264
        hdmitxwr(0xFB, (ADDR_LVDSRX|0x01));    // Enable LVDSRX slave address
        lvrxset(0x21, 0x02, 0x00);    // RegLvRxPwdAll

        if( iTE6615_DATA.config.LVDS_RxEnDeSSC )
        {
            lvrxset(0x05, 0x04, 0x04);    // RegSoftSSCRst
            lvrxset(0x97, 0x80, 0x80);

            // setting to LC
            lvrxset(0x97, 0x4f, 0x03);
            lvrxwr(0x4E, 0x40);  //LC

            lvrxset(0x08, 0x04, 0x04);
            lvrxset(0x71, 0x07, 0x04);   // RegL01_CKSel
            lvrxset(0x73, 0x07, 0x04);   // RegL23_CKSel
            lvrxset(0x74, 0x03, 0x02);    //if DeSSC , SP_RSTB=0, reset
            lvrxset(0x74, 0x03, 0x03);    //if DeSSC , SP_RSTB=1, normal
            lvrxset(0x74, 0x80, (config_LvRxDeSSCCLKSel<<7));   //1:DeSSCCLK from PLLDESSC, 0: DeSSCCLK from RxPLL
            lvrxset(0x05, 0x04, 0x00);    // RegSoftSSCRst
        }

        if( iTE6615_DATA.config.Input_Enable_DualMode == 1 )
        {
            lvrxset(0x70, 0x03, 0x03);    // L01_XP_RSTB
            lvrxset(0x72, 0x03, 0x03);    // L23_XP_RSTB
        }
        else {
            lvrxset(0x70, 0x03, 0x03);    // L01_XP_RSTB
            lvrxset(0x72, 0x03, 0x00);    // L23_XP_RSTB
        }
        #endif

        // HDMITX PwrOn
        hdmitxset(0x25, 0x01, 0x00);   // PwrOn GRCLK
        hdmitxset(0x25, 0x0E, 0x00);   // PwrOn ICLK, IACLK, TXCLK

        // PLL PwrOn
        hdmitxset(0x88, 0x02, 0x00);   // PwrOn DRV
        hdmitxset(0x84, 0x40, 0x00);   // PwrOn XPLL
        hdmitxset(0x81, 0x80, 0x00);   // PwrOn IPLL

        // PLL Reset OFF
        hdmitxset(0x80, 0x80, 0x80);   // IP_RESETB
        hdmitxset(0x84, 0x80, 0x80);   // XP_RESETB

        // Misc PasswWord OFF
        hdmitxwr(0xFF, 0xFF);
    }
    else
    {
        HDMITX_DEBUG_PRINTF(("Power Off HDMITX \r\n"));

        hdmitxset(0x25, 0x01, 0x00);

        #if iTE6264
        hdmitxwr(0xfb, (ADDR_LVDSRX|0x01));   // enable LVDS
        lvrxset(0x70, 0x03, 0x00);    // L01_XP_RSTB
        lvrxset(0x72, 0x03, 0x00);    // L23_XP_RSTB
        lvrxset(0x74, 0x03, 0x00);    // SP_RSTB
        lvrxset(0x97, 0x4f, 0x08); // PWDB_XTAL, LC_REG_V12PWD, LC_ICTP_PWDB, LC_PWDB, LC_RSTB,
        lvrxset(0x21, 0x02, 0x02);    // RegLvRxPwdAll
        #endif
        hdmitxwr(0xfb, (ADDR_LVDSRX|0x00));   // disable LVDS

        // PLL Reset
        hdmitxset(0x88, 0x01, 0x01);   // DRV_RST
        hdmitxset(0x84, 0x80, 0x00);   // XP_RESETB
        hdmitxset(0x80, 0x80, 0x00);   // IP_RESETB
        delay1ms(1);

        // PLL PwrDn
        hdmitxset(0x88, 0x02, 0x02);   // PwrDn DRV
        hdmitxset(0x84, 0x40, 0x40);   // PwrDn XPLL
        hdmitxset(0x81, 0x80, 0x80);   // PwrDn IPLL

        // HDMITX PwrDn
        hdmitxset(0x25, 0x0E, 0x0E);   // PwrDn ICLK, IACLK, TXCLK
        hdmitxset(0x26, 0x01, 0x01);   // PwrDn FPCLK, FTXCLK ??
        //hdmitxset(0xFC, 0x01, 0x00);   // PwrDn CRCLK (Kuro Modify to keep CEC run)
        hdmitxset(0x0C, 0x01, 0x01);   // enable IDDQ
        //hdmitxset(0x25, 0x01, 0x01);   // PwrDn GRCLK // kuro mark it for 6264 can't video stable due to it

    }
}

void iTE6615_SetTX_AudioOption( void )
{
    u8 tmp;

    hdmitxset(0xC4, 0x3A, (BIT1|BIT3));
    hdmitxset(0xE3, 0x08, 0x00);
    hdmitxset(0xE4, 0x02, 0x02);
    hdmitxset(0xC5, 0x06, 0x04);

    if (iTE6615_DATA.PCLK > 150000)
        tmp = 0;
    else if (iTE6615_DATA.PCLK > 75000)
        tmp = 1;
    else if (iTE6615_DATA.PCLK > 37500)
        tmp = 2;
    else
        tmp = 3;

    tmp <<= 6;
    hdmitxset(0x21, 0xC0, tmp);
    hdmitxset(0xF3, 0x10, SPDIF_SCHMITT_TRIGGER<<4);
}


void iTE6615_SetTX_AudioFormat( void )
{
    u8 i, audsrc, infoca;
    u8 chksum;

    if(iTE6615_DATA.config.Audio_Output_Sel == SPDIF && iTE6615_DATA.config.Audio_Type == HBR)
    {
        iTE6615_DATA.config.Audio_Enable_MCLKSamplingMode = 1;
    }

    chgtxbank(0);
    hdmitxset(0xDA, 0x7F, (iTE6615_DATA.config.Audio_I2S_WordLength << 5) + iTE6615_DATA.config.Audio_I2C_HardwareFormat);

    if (iTE6615_DATA.config.Audio_Output_Sel == SPDIF)
    {
        hdmitxwr(0xDC, 0x00);
        hdmitxwr(0xDD, 0x00);
        hdmitxwr(0xDE, 0x00);
        hdmitxset(0x20, 0x80, (iTE6615_DATA.config.Audio_Enable_MCLKSamplingMode << 7));
    }
    else
    {
        hdmitxwr(0xDC, 0x10);
        hdmitxwr(0xDD, 0x32);
        hdmitxwr(0xDE, 0x54);
    }

    hdmitxset(0xE3, 0x01, config_RecChStSel);

    if (iTE6615_DATA.config.Audio_Type == HBR)
    {
        hdmitxwr(0xD8, (iTE6615_DATA.config.Audio_Enable_MSAudio << 5) + (iTE6615_DATA.config.Audio_Enable_3DAudio << 4) + 0x08);
    }
    else if (iTE6615_DATA.config.Audio_Type == DSD)
    {
        hdmitxwr(0xD8, (iTE6615_DATA.config.Audio_Enable_MSAudio << 5) + (iTE6615_DATA.config.Audio_Enable_3DAudio << 4) + 0x04);
    }
    else if (iTE6615_DATA.config.Audio_Enable_TDMAudio)
    {
        hdmitxwr(0xD8, (iTE6615_DATA.config.Audio_Enable_MSAudio << 5) + (iTE6615_DATA.config.Audio_Enable_3DAudio << 4) + 0x02);
        hdmitxset(0xE2, 0x07, iTE6615_DATA.config.Audio_Enable_TDMAudio_ChannelCount);
    }
    else
    {
        hdmitxwr(0xD8, (iTE6615_DATA.config.Audio_Enable_MSAudio << 5) + (iTE6615_DATA.config.Audio_Enable_3DAudio << 4));
    }


    chgtxbank(1);

    // Set Audio N Value
    switch (iTE6615_DATA.config.Audio_Frequency)
    {
        case AUD32K :   // 4096 = 0x1000
            hdmitxwr(0x33, 0x00);
            hdmitxwr(0x34, 0x10);
            hdmitxwr(0x35, 0x00);
            break;
        case AUD44K :   // 6272 = 0x1880
            hdmitxwr(0x33, 0x80);
            hdmitxwr(0x34, 0x18);
            hdmitxwr(0x35, 0x00);
            break;
        case AUD48K :   // 6144 = 0x1800
            hdmitxwr(0x33, 0x00);
            hdmitxwr(0x34, 0x18);
            hdmitxwr(0x35, 0x00);
            break;
        case AUD64K :   // 8192 = 0x2000
        case AUD256K :
            hdmitxwr(0x33, 0x00);
            hdmitxwr(0x34, 0x20);
            hdmitxwr(0x35, 0x00);
            break;
        case AUD88K :   // 12544 = 0x3100
        case AUD352K :
            hdmitxwr(0x33, 0x00);
            hdmitxwr(0x34, 0x31);
            hdmitxwr(0x35, 0x00);
            break;
        case AUD96K :   // 12288 = 0x3000
        case AUD384K :
            hdmitxwr(0x33, 0x00);
            hdmitxwr(0x34, 0x30);
            hdmitxwr(0x35, 0x00);
            break;
        case AUD128K :   // 16384 = 0x4000
        case AUD512K :
            hdmitxwr(0x33, 0x00);
            hdmitxwr(0x34, 0x40);
            hdmitxwr(0x35, 0x00);
            break;
        case AUD176K :   // 25088 = 0x6200
        case AUD705K :
            hdmitxwr(0x33, 0x00);
            hdmitxwr(0x34, 0x62);
            hdmitxwr(0x35, 0x00);
            break;
        case AUD192K :   // 24576 = 0x6000
        case AUD768K :
            hdmitxwr(0x33, 0x00);
            hdmitxwr(0x34, 0x60);
            hdmitxwr(0x35, 0x00);
            break;
        case AUD1024K :  // 32768 = 0x8000
            hdmitxwr(0x33, 0x00);
            hdmitxwr(0x34, 0x80);
            hdmitxwr(0x35, 0x00);
            break;
        case AUD1411K :  // 50176 = 0xC400
            hdmitxwr(0x33, 0x00);
            hdmitxwr(0x34, 0xC4);
            hdmitxwr(0x35, 0x00);
            break;
        case AUD1536K :  // 49152 = 0xC000
            hdmitxwr(0x33, 0x00);
            hdmitxwr(0x34, 0xC0);
            hdmitxwr(0x35, 0x00);
            break;
        default :
            HDMITX_DEBUG_PRINTF(("Error: Audio_Frequency Error !!!\r\n"));
    }

    // Channel Status
    if (iTE6615_DATA.config.Audio_Type == LPCM)
        hdmitxwr(0xF0, 0x00);
    else
        hdmitxwr(0xF0, 0x02);

    hdmitxwr(0xF1, 0x00);
    hdmitxwr(0xF2, 0x00);
    hdmitxwr(0xF3, ((iTE6615_DATA.config.Audio_Frequency & 0x30) << 2) + (iTE6615_DATA.config.Audio_Frequency & 0x0F));
    hdmitxwr(0xF4, ((~(iTE6615_DATA.config.Audio_Frequency << 4)) & 0xF0) + 0x0B);

    // Audio InfoFrame
    if (iTE6615_DATA.config.Audio_Enable_3DAudio)
    {
        infoca = 0x00;
    }
    else
    {
        switch (iTE6615_DATA.config.Audio_ChannelCount)
        {
            case 0 :infoca = 0xFF; break;  // no audio
            case 2 :infoca = 0x00; break;
            case 3 :infoca = 0x01; break;  // 0x01,0x02,0x04
            case 4 :infoca = 0x03; break;  // 0x03,0x05,0x06,0x08,0x14
            case 5 :infoca = 0x07; break;  // 0x07,0x09,0x0A,0x0C,0x15,0x16,0x18
            case 6 :infoca = 0x0B; break;  // 0x0B,0x0D,0x0E,0x10,0x17,0x19,0x1A,0x1C
            case 7 :infoca = 0x0F; break;  // 0x0F,0x11,0x12,0x1B,0x1D,0x1E
            case 8 :infoca = 0x1F; break;  // 0x13,0x1F
            default :HDMITX_AUDIO_PRINTF(("Error: Audio Channel Number Error !!!\r\n"));
        }
    }


    if (iTE6615_DATA.config.Audio_Enable_3DAudio)
        hdmitxwr(0x68, 0x00);
    else
        hdmitxwr(0x68, iTE6615_DATA.config.Audio_ChannelCount - 1);

    if (iTE6615_DATA.config.Audio_Type == DSD)
    {
        switch (iTE6615_DATA.config.Audio_Frequency)
        {
            case AUD32K  :hdmitxwr(0x69, 0x04); break;
            case AUD44K  :hdmitxwr(0x69, 0x08); break;
            case AUD48K  :hdmitxwr(0x69, 0x0C); break;
            case AUD88K  :hdmitxwr(0x69, 0x10); break;
            case AUD96K  :hdmitxwr(0x69, 0x14); break;
            case AUD176K :hdmitxwr(0x69, 0x18); break;
            case AUD192K :hdmitxwr(0x69, 0x1C); break;
            default :HDMITX_AUDIO_PRINTF(("Error: Audio_Frequency Error for DSD !!!\r\n"));
        }
    }
    else
    {
        hdmitxwr(0x69, 0x00);
    }

    hdmitxwr(0x6A, 0x00);
    hdmitxwr(0x6B, infoca);
    hdmitxwr(0x6C, 0x00);

    chksum = 0x84;
    chksum += 0x01;
    chksum += 0x0A;
    for (i = 0x68; i <= 0x6C; i++)
        chksum += hdmitxrd(i);

    hdmitxwr(0x6D, 0x100 - chksum);

    if (iTE6615_DATA.config.Audio_Enable_3DAudio || iTE6615_DATA.config.Audio_Enable_MSAudio)
    {
        if (iTE6615_DATA.config.Audio_Enable_3DAudio)
        {
            hdmitxwr(0xC8, 0x01);
            hdmitxwr(0xC9, 0x00);
            hdmitxwr(0xCA, iTE6615_DATA.config.Audio_ChannelCount - 1);
            hdmitxwr(0xCB, 0x01);
            hdmitxwr(0xCC, 0x01);
            for (i = 0xCD; i <= 0xDD; i++)
                hdmitxwr(i, 0x00);
        } else {
            hdmitxwr(0xC8, 0x00);
            hdmitxwr(0xC9, (((iTE6615_DATA.config.Audio_ChannelCount >> 1) - 1) << 2) + iTE6615_DATA.config.Infoframe_Enable3D_DualView);
            for (i = 0; i <= (iTE6615_DATA.config.Audio_ChannelCount >> 1); i++) {
                if (iTE6615_DATA.config.Infoframe_Enable3D_DualView)
                    hdmitxwr(0xCA + i * 5, 0x03);
                else
                    hdmitxwr(0xCA + i * 5, 0x00);
                hdmitxwr(0xCB + i * 5, 0x00);
                hdmitxwr(0xCC + i * 5, 0x00);
                hdmitxwr(0xCD + i * 5, 0x00);
                hdmitxwr(0xCE + i * 5, 0x00);
            }
            for (i = 0xCA + i * 5; i <= 0xDD; i++)
                hdmitxwr(i, 0x00);
        }
    }

    chgtxbank(0);

    switch( infoca )
    {
        case 0x00 : audsrc = 0x01; break;
        case 0x01 : audsrc = 0x03; break;
        case 0x02 : audsrc = 0x03; break;
        case 0x03 : audsrc = 0x03; break;
        case 0x04 : audsrc = 0x05; break;
        case 0x05 : audsrc = 0x07; break;
        case 0x06 : audsrc = 0x07; break;
        case 0x07 : audsrc = 0x07; break;
        case 0x08 : audsrc = 0x05; break;
        case 0x09 : audsrc = 0x07; break;
        case 0x0a : audsrc = 0x07; break;
        case 0x0b : audsrc = 0x07; break;
        case 0x0c : audsrc = 0x0d; break;
        case 0x0d : audsrc = 0x0f; break;
        case 0x0e : audsrc = 0x0f; break;
        case 0x0f : audsrc = 0x0f; break;
        case 0x10 : audsrc = 0x0d; break;
        case 0x11 : audsrc = 0x0f; break;
        case 0x12 : audsrc = 0x0f; break;
        case 0x13 : audsrc = 0x0f; break;
        case 0x14 : audsrc = 0x09; break;
        case 0x15 : audsrc = 0x0b; break;
        case 0x16 : audsrc = 0x0b; break;
        case 0x17 : audsrc = 0x0b; break;
        case 0x18 : audsrc = 0x0d; break;
        case 0x19 : audsrc = 0x0f; break;
        case 0x1a : audsrc = 0x0f; break;
        case 0x1b : audsrc = 0x0f; break;
        case 0x1c : audsrc = 0x0d; break;
        case 0x1d : audsrc = 0x0f; break;
        case 0x1e : audsrc = 0x0f; break;
        case 0x1f : audsrc = 0x0f; break;
        default   : audsrc = 0x00; break;
    }

    if (iTE6615_DATA.config.Audio_Type == HBR && iTE6615_DATA.config.Audio_Output_Sel == I2S)
    {
        hdmitxwr(0xDB, 0x0F);
    }
    else if (iTE6615_DATA.config.Audio_Enable_3DAudio)
    {
        if (iTE6615_DATA.config.Audio_ChannelCount <= 10)
            hdmitxwr(0xDB, 0x1F);   // 9/10-channel audio
        else
            hdmitxwr(0xDB, 0x3F);   // 11/12-channel audio
    }
    else
    {
        hdmitxwr(0xDB, audsrc);
    }

    hdmitxset(0xD8, 0x01, iTE6615_DATA.config.Audio_Output_Sel);
}

void iTE6615_SetTX_CSCMetrix(const u8 *csc_matrix )
{
    u8 offset;
    chgtxbank(0);
    for( offset=0xA4 ; offset<=0xB9 ; offset++ )
    {
        if ( offset != 0xA6 ) {
            hdmitxwr( offset, *csc_matrix );
            csc_matrix++;
        }
    }
    hdmitxwr( 0xA1, *csc_matrix );
}

void iTE6615_SetTX_Timestamp( u8 active )
{
    if (active) {
        iTE6615_DATA.Flag_SYS_Tick_Enable = 1;
        hdmitxwr(0x09, 0x00);
        hdmitxwr(0x09, 0x80);
    } else {
        iTE6615_DATA.Flag_SYS_Tick_Enable = 0;
        hdmitxwr(0x09, 0x00);
    }
}

void iTE6615_SetTX_AFE( void )
{
    u8 DRV_TERMON, DRV_RTERM, DRV_ISW, DRV_ISWC, DRV_TPRE, DRV_NOPE, DRV_PISW, DRV_PISWC,DRV_ISWP;
    u32 RealPR, H2ON_PLL;

    if( iTE6615_DATA.config.DisLockPR )
        RealPR = iTE6615_DATA.config.ManuallPR;
    else
        RealPR = iTE6615_DATA.config.PixRpt;

    iTE6615_DATA.VCLK_expected = iTE6615_DATA.PCLK * 2;

    if( iTE6615_DATA.config.Input_ColorSpace==YUV420 && iTE6615_DATA.config.Output_ColorSpace!=YUV420 )
        iTE6615_DATA.VCLK_expected *= 2;

    if( iTE6615_DATA.config.Output_ColorDepth==VID10BIT )
        iTE6615_DATA.VCLK_expected += (iTE6615_DATA.VCLK_expected/4);
    else if( iTE6615_DATA.config.Output_ColorDepth==VID12BIT )
        iTE6615_DATA.VCLK_expected += (iTE6615_DATA.VCLK_expected/2);

    iTE6615_DATA.VCLK_expected *= RealPR;

    HDMITX_DEBUG_PRINTF(("\r\n\r\n########## ICLK=%d MHz, Expected VCLK = %d MHz ##########\r\n\r\n", iTE6615_DATA.ICLK/1000, iTE6615_DATA.VCLK_expected/1000));
    if ( iTE6615_DATA.VCLK_expected > ((u32)100000) ) // ICLKIN > 80MHz
    {
        hdmitxset(0x81, 0x07, 0x04);
        if (iTE6615_DATA.vars.AFESPEED == LOW)
        {
            HDMITX_DEBUG_PRINTF(("Change TX AFE setting to High Speed mode ...\r\n"));

            iTE6615_DATA.vars.AFESPEED = HIGH;
        }
    }
    else
    {
        hdmitxset(0x81, 0x07, 0x03);
        if (iTE6615_DATA.vars.AFESPEED == HIGH) {
            HDMITX_DEBUG_PRINTF(("Change TX AFE setting to Low Speed mode ...\r\n"));

            iTE6615_DATA.vars.AFESPEED = LOW;
        }
    }

    if ( iTE6615_DATA.VCLK_expected > ((u32)100000) ) // IP_VCLK05 > 50MHz
        hdmitxset(0x84, 0x07, 0x04);
    else
        hdmitxset(0x84, 0x07, 0x03);

    if ( iTE6615_DATA.VCLK_expected > ((u32)162000) ) // IP_VCLK05 > 81MHz
        hdmitxset(0x88, 0x04, 0x04);
    else
        hdmitxset(0x88, 0x04, 0x00);


    if( iTE6615_DATA.VCLK_expected > ((u32)340000) )
    {
        DRV_TERMON = 1;
        DRV_RTERM = 0x5;
        DRV_ISW = 0xC;
        DRV_ISWC = 0xB;
        DRV_PISW = 1;
        DRV_PISWC = 0;
        DRV_ISWP = 0x3 ;
        DRV_TPRE = 0x0;
        DRV_NOPE = 0;
        H2ON_PLL = 1;
    }
    else if( iTE6615_DATA.VCLK_expected > ((u32)150000) ) // single-end swing = 450mV
    {
        DRV_TERMON = 1;
        DRV_RTERM = 0x1;
        DRV_ISW = 0x7;
        DRV_ISWC = 0x7;
        DRV_PISW = 1;
        DRV_PISWC = 0;
        DRV_ISWP = 0 ;
        DRV_TPRE = 0;
        DRV_NOPE = 1;
        H2ON_PLL = 0;
    }
    else // single-end swing = 500mV
    {
        DRV_TERMON = 0;
        DRV_RTERM = 0x0;
        DRV_ISW = 0x3;
        DRV_ISWC = 0x3;
        DRV_PISW = 1;
        DRV_PISWC = 0;
        DRV_ISWP = 0 ;
        DRV_TPRE = 0;
        DRV_NOPE = 1;
        H2ON_PLL = 0;
    }

    hdmitxset(0x80, 0x08, (H2ON_PLL<<3));
    hdmitxset(0x83, 0x40, (H2ON_PLL<<6));
    hdmitxset(0x87, 0x1F, DRV_ISW);     // DRV_ISW[4:0]
    hdmitxset(0x88, 0xF0, (DRV_PISWC<<6)+(DRV_PISW<<4));
    hdmitxset(0x89, 0xBF, (DRV_NOPE<<7)+(DRV_TERMON<<5)+DRV_RTERM);
    hdmitxset(0x8A, 0x0F, DRV_TPRE);   // DRV_TPRE[3:0]
    hdmitxset(0x8B, 0x1F, DRV_ISWC);
    hdmitxset(0x8D, 0x1F, DRV_ISWP);

    if( H2ON_PLL )
    {
        chgtxbank(2);
        hdmitxset(0x40, 0x8E, 0x80+(0x01<<1));
        hdmitxset(0x44, 0xF0, (0x02<<4));
        hdmitxwr(0x46, 0x2F);
        hdmitxset(0x47, 0x06, 0x06);
        chgtxbank(0);
        hdmitxset(0x83, 0x10, 0x10);
    }
    else
    {
        chgtxbank(2);
        hdmitxset(0x47, 0x06, 0x00);
        chgtxbank(0);
        hdmitxset(0x83, 0x10, 0x00);
    }

}

void iTE6615_SetTX_Reset(void)
{

    // Enable GRCLK
    hdmitxset(0x25, 0x01, 0x00);

    #if iTE6264
    hdmitxwr(0xFB, (ADDR_LVDSRX|0x01));    // Enable LVDSRX slave address
    // LVRX PLL Reset
    lvrxset(0x70, 0x01, 0x00);    // L01_XP_RSTB
    lvrxset(0x72, 0x01, 0x00);    // L23_XP_RSTB
    lvrxset(0x74, 0x01, 0x00);    // SP_RSTB
    lvrxset(0x97, 0x4f, 0x08);    // PWDB_XTAL, LC_REG_V12PWD, LC_ICTP_PWDB, LC_PWDB, LC_RSTB,
    lvrxset(0x97, 0x80, 0x80);    // set init LC for DESSC
    #endif

    // H2TX PLL Reset
    hdmitxset(0x80, 0x80, 0x00);   // IP_RESETB
    hdmitxset(0x84, 0x80, 0x00);   // XP_RESETB
    delay1ms(2);

    hdmitxset(0x35, 0x10, 0x10);    // ddcmastrst
    hdmitxset(0x35, 0x10, 0x00);    // ddcmastrst

    #if iTE6264
    lvrxset(0x05, 0x06, 0x06);
    // LVRX Reset
    lvrxset(0x05, 0x01, 0x01);      // RegSoftRRst
    #endif

    hdmitxset(0x05, 0xFE, 0xEE);    // 0518 modify by 0507 RD code
    hdmitxset(0x06, 0x03, 0x03);
    // H2TX Reset
    hdmitxset(0x05, 0x01, 0x01);    // RegSoftRRst    Kuro :reset LVDS RX Address too ... and init setting ?

    delay1ms(1);
    hdmitxset(0x05, 0xFE, 0xEE);    // Reset All but RegStorekmRst 0518 modify by 0507 RD code
    hdmitxset(0x06, 0x02, 0x02);

    #if iTE6264
    hdmitxwr(0xFB, (ADDR_LVDSRX|0x01));    // Enable LVDSRX slave address
    lvrxset(0x05, 0x06, 0x06);    // RegSoftSSCRst

    //lvrx pll on
    if( iTE6615_DATA.config.Input_Enable_DualMode == 1 )
    {
        lvrxset(0x70, 0x03, 0x03);    // L01_XP_RSTB
        lvrxset(0x72, 0x03, 0x03);    // L23_XP_RSTB
    }
    else
    {
        lvrxset(0x70, 0x03, 0x03);    // L01_XP_RSTB
        lvrxset(0x72, 0x03, 0x00);    // L23_XP_RSTB
    }
    lvrxset(0x08, 0x7B, (iTE6615_DATA.config.LVDS_RxOutMap<<6)+(iTE6615_DATA.config.LVDS_RxColDep<<4 )+(iTE6615_DATA.config.LVDS_RxSkewDis<<3)+iTE6615_DATA.config.LVDS_RxChNum);
    lvrxset(0x09, 0x0C, (iTE6615_DATA.config.LVDS_RxInSwap<<3));

    // Clock Delay & Invert
    lvrxset(0x20, 0x3F, (EnSSCHPCLKInv<<5)+(EnSSCLPCLKInv<<4)+(SSCHPCLKDly<<2)+SSCLPCLKDly);

    lvrxset(0x4B, 0x1F, iTE6615_DATA.config.LVDS_RxStbDENum);

    if( iTE6615_DATA.config.LVDS_RxEnDeSSC )
    {
        lvrxset(0x05, 0x04, 0x04);    // RegSoftSSCRst
        lvrxset(0x97, 0x80, 0x80);

        // setting to LC
        lvrxset(0x97, 0x4f, 0x03);
        lvrxwr(0x4E, 0x40);  //LC

        lvrxset(0x08, 0x04, 0x04);
        lvrxset(0x71, 0x07, 0x04);   // RegL01_CKSel
        lvrxset(0x73, 0x07, 0x04);   // RegL23_CKSel
        lvrxset(0x74, 0x01, 0x00);    //if DeSSC , SP_RSTB=0, reset
        lvrxset(0x74, 0x01, 0x01);    //if DeSSC , SP_RSTB=1, normal
        lvrxset(0x74, 0x80, (config_LvRxDeSSCCLKSel<<7));   //1:DeSSCCLK from PLLDESSC, 0: DeSSCCLK from RxPLL

        lvrxset(0x48, 0x20, 0x20);    // RegLockSoft=1 for FPGA only
        lvrxset(0x3E, 0x20, 0x20);    // RegLockSel=1 for FPGA only
        lvrxset(0x3E, 0x20, 0x00);    // RegLockSel=1 for FPGA only
        lvrxset(0x05, 0x04, 0x00);    // RegSoftSSCRst
    }
    #endif

    hdmitxset(0x35, 0x10, 0x10);
    hdmitxset(0x35, 0x10, 0x00);

    iTE6615_DATA.vars.AFESPEED = HIGH;   // set default AFE speed as High after reset

    hdmitxwr(0xFC, (ADDR6615_CEC|0x01));

    hdmitxset(0x21, 0x03, 0x01);

    // H2TX Initial Setting
    hdmitxwr(0xFF, 0xC3);
    hdmitxwr(0xFF, 0xA5);
    hdmitxset(0x0C, 0x1C, (config_ForceRxOn << 2));
    hdmitxwr(0xF8, 0xFF);

    hdmitxset(0x20, 0x0E, ((iTE6615_DATA.config.ManuallPR-1)<<2)+(iTE6615_DATA.config.DisLockPR<<1));
    hdmitxset(0x22, 0x06, 0x00);
    hdmitxset(0x24, 0x3F, (TTL_EnICLKHInv<<5)+(TTL_EnICLKLInv<<4)+(TTL_ICLKHDly<<2)+TTL_ICLKLDly );
    hdmitxset(0x34, 0xC0, 0x80);
    hdmitxset(0x35, 0x03, iTE6615_DATA.config.DDCSpeed);
    hdmitxset(0x3A, 0xFC, 0x90);

    // Input TTL setting
    hdmitxset(0x90, 0x1F, (1<<4)+(config_InSyncAutoSwapOff<<3)+(iTE6615_DATA.config.Input_Enable_DualMode_Mode<<2)+(iTE6615_DATA.config.Input_Enable_HL_Swap<<1)+iTE6615_DATA.config.Input_Enable_DualMode);
    hdmitxset(0x91, 0xF8, (iTE6615_DATA.config.TTL_Enable_BTAFormat<<7)+(0<<6)+(iTE6615_DATA.config.TTL_Enable_SyncEmbedd<<5)+(iTE6615_DATA.config.TTL_Enable_DDR<<4)+(iTE6615_DATA.config.TTL_Enable_HalfBus<<3));
    hdmitxset(0x92, 0xEF, (_TTL_INPUT_LMSwap_<<7)+(_TTL_INPUT_YCSwap_<<6)+(_TTL_INPUT_RBSwap_<<5)+(_TTL_INPUT_LMSwap_<<3)+(_TTL_INPUT_YCSwap_<<2)+(_TTL_INPUT_RBSwap_<<1)+_TTL_INPUT_PackSwap_);
    hdmitxset(0x93, 0x1F, 0x00);
    hdmitxset(0x94, 0x3E, 0x26);
    hdmitxset(0xBF, 0xC8, 0x80);
    hdmitxset(0xC0, 0x10, 0x00);
    hdmitxset(0xC3, 0x0F, 0x01);

    hdmitxset(0x95, 0x80, (iTE6615_DATA.config.EnH2InLBBypass << 7));

    hdmitxset(0x96, 0x77, 0x00);

    // Debug Port Selection
    hdmitxset(0xF0, 0x3f, 0x00);

    // Setup INT Pin: Active Low & Open-Drain
    hdmitxset(0x0D, 0x07, 0x03);

    hdmitxset(0x19, 0x03, 0x03);   // Enable HPD and RxSen Interrupt
    hdmitxset(0xc0, 0x28, 0x20);
    hdmitxset(0xcf, 0x3F, 0x80);
    hdmitxset(0xfe, 0x10, 0x10);

    hdmitxset(0xf1, 0x0f, 0x06);

    hdmitxset(0xc5, 0x08, 0x00);

    iTE6615_DATA.ICLK = 0;
    iTE6615_DATA.PCLK = 0;
    iTE6615_DATA.TXCLK = 0;
    iTE6615_DATA.VCLK = 0;
    iTE6615_DATA.VCLK_expected = 0;

    #if iTE6264
    iTE6615_DATA.SSCPCLKL = 0;
    iTE6615_DATA.SSCPCLKH = 0;
    #endif
}

void iTE6615_SetTX_RCLK(void)
{
    iTE6615_DATA.RCLK = 0;
    iTE6615_RCLK_Load();
    if ( iTE6615_DATA.RCLK == 0 )
    {
        //iTE6615_EVB_Ca100ms_RCLK();
        if ( iTE6615_DATA.RCLK == 0 )
        {
            iTE6615_DATA.RCLK = RCLK_TYPICAL;
        }
    }
    iTE6615_RCLK_Set();
}
// GET
void iTE6615_Get_TXCLK()
{
    u32 rddata, predivsel, txclk_sum = 0, vclk_sum = 0;
    u8  i;
    iTE6615_DATA.DumpREG = 0;
    // PCLK Count Pre-Test
    hdmitxset(0x0B, 0x80, 0x80);
    delay1ms(2);
    hdmitxset(0x0B, 0x80, 0x00);

    rddata = hdmitxrd(0x0A);
    rddata += ((hdmitxrd(0x0B)&0x0F)<<8);

    if( rddata<16 )
        predivsel = 7;
    else if( rddata<32 )
        predivsel = 6;
    else if( rddata<64 )
        predivsel = 5;
    else if( rddata<128 )
        predivsel = 4;
    else if( rddata<256 )
        predivsel = 3;
    else if( rddata<512 )
        predivsel = 2;
    else if( rddata<1024 )
        predivsel = 1;
    else
        predivsel = 0;

    for(i=0; i<5; i++)
    {
        hdmitxset(0x0B, 0xF0, (0x80+(predivsel<<4)));
        delay1ms(1);
        hdmitxset(0x0B, 0xF0, (predivsel<<4));

        rddata = hdmitxrd(0x0A);
        rddata += ((hdmitxrd(0x0B)&0x0F)<<8);

        txclk_sum += rddata;

        rddata = hdmitxrd(0xD0);
        rddata += ((hdmitxrd(0xD1)&0x0F)<<8);
        vclk_sum += rddata;
    }

    txclk_sum /= (5* (1<<predivsel));  // txclk_sum /= (5*pow(2,predivsel));
    vclk_sum /= (5* (1<<predivsel));   // vclk_sum /= (5*pow(2,predivsel));

    if(txclk_sum)iTE6615_DATA.TXCLK = iTE6615_DATA.RCLK*2048/txclk_sum;
    if(vclk_sum)iTE6615_DATA.VCLK = iTE6615_DATA.RCLK*2048/vclk_sum*2;    // BitRate/10

    if( iTE6615_DATA.config.Input_ColorSpace==YUV420
     && iTE6615_DATA.config.Output_ColorSpace!=YUV420 )
        iTE6615_DATA.PCLK = iTE6615_DATA.TXCLK/2;
    else
        iTE6615_DATA.PCLK = iTE6615_DATA.TXCLK;

    HDMITX_DEBUG_PRINTF(("iTE6615_DATA.PCLK = %d \r\n", iTE6615_DATA.PCLK / 1000));
    HDMITX_DEBUG_PRINTF(("iTE6615_DATA.TXCLK = %d \r\n", iTE6615_DATA.TXCLK /1000));

    if( iTE6615_DATA.config.Input_Enable_DualMode &&
       (iTE6615_DATA.config.TTL_Enable_DDR && !iTE6615_DATA.config.TTL_Enable_HalfBus))
        iTE6615_DATA.ICLK = iTE6615_DATA.PCLK/2;
    else if( iTE6615_DATA.config.Input_Enable_DualMode ||
            (iTE6615_DATA.config.TTL_Enable_DDR && !iTE6615_DATA.config.TTL_Enable_HalfBus) )
        iTE6615_DATA.ICLK = iTE6615_DATA.PCLK;
    else
        iTE6615_DATA.ICLK = iTE6615_DATA.PCLK*2;
}

void    iTE6615_Get_VideoInfo(void)
{
    u8 need_double;
    u32 Reg210, Reg211, Reg212, Reg213, Reg214, Reg215, Reg216, Reg217;
    u32 Reg218, Reg219, Reg21A, Reg21B, Reg21C, Reg21D, Reg21E, Reg21F;
    u32 Reg220, Reg221, Reg222, Reg223, Reg22E;

    u8 InterLaced;
    u32 HVR2nd,VFP2nd;
    u16 HBlank;

    if(iTE6615_DATA.config.Input_ColorSpace == YUV420) need_double = 1;
    else                                               need_double = 0;

    chgtxbank(2);
    hdmitxset(0x2F, 0x80, 0x80);    // Enable Video Timing Read Back

    Reg210 = hdmitxrd(0x10);
    Reg211 = hdmitxrd(0x11);
    Reg212 = hdmitxrd(0x12);
    Reg213 = hdmitxrd(0x13);
    Reg214 = hdmitxrd(0x14);
    Reg215 = hdmitxrd(0x15);
    Reg216 = hdmitxrd(0x16);
    Reg217 = hdmitxrd(0x17);
    Reg218 = hdmitxrd(0x18);
    Reg219 = hdmitxrd(0x19);
    Reg21A = hdmitxrd(0x1A);
    Reg21B = hdmitxrd(0x1B);
    Reg21C = hdmitxrd(0x1C);
    Reg21D = hdmitxrd(0x1D);
    Reg21E = hdmitxrd(0x1E);
    Reg21F = hdmitxrd(0x1F);
    Reg220 = hdmitxrd(0x20);
    Reg221 = hdmitxrd(0x21);
    Reg222 = hdmitxrd(0x22);
    Reg223 = hdmitxrd(0x23);
    Reg22E = hdmitxrd(0x2E);

    iTE6615_CurVTiming.HPolarity = (Reg22E&0x01)>>0;
    iTE6615_CurVTiming.VPolarity = (Reg22E&0x02)>>1;

    iTE6615_CurVTiming.HFrontPorch  = ((Reg211<<8)+Reg210) << need_double;
    iTE6615_CurVTiming.HSyncWidth   = ((Reg213<<8)+Reg212) << need_double;
    iTE6615_CurVTiming.HBackPorch   = ((Reg215<<8)+Reg214) << need_double;
    iTE6615_CurVTiming.HActive      = ((Reg217<<8)+Reg216) << need_double;
    iTE6615_CurVTiming.HTotal       = iTE6615_CurVTiming.HFrontPorch+iTE6615_CurVTiming.HSyncWidth+iTE6615_CurVTiming.HBackPorch +iTE6615_CurVTiming.HActive;

    iTE6615_CurVTiming.VFrontPorch  = (Reg21B<<8)+Reg21A;
    iTE6615_CurVTiming.VSyncWidth   = (Reg21D<<8)+Reg21C;
    iTE6615_CurVTiming.VBackPorch   = (Reg21F<<8)+Reg21E;
    iTE6615_CurVTiming.VActive      = (Reg221<<8)+Reg220;
    iTE6615_CurVTiming.VTotal       = iTE6615_CurVTiming.VFrontPorch+iTE6615_CurVTiming.VSyncWidth+iTE6615_CurVTiming.VBackPorch+iTE6615_CurVTiming.VActive;

    HVR2nd = (Reg219<<8)+Reg218;
    VFP2nd = (Reg223<<8)+Reg222;
    InterLaced = (Reg22E&0x04)>>2;

    hdmitxset(0x2F, 0x80, 0x00);    // Disable Video Timing Read Back
    chgtxbank(0);

    HBlank = iTE6615_CurVTiming.HBackPorch + iTE6615_CurVTiming.HSyncWidth + iTE6615_CurVTiming.HFrontPorch;

    iTE6615_CurVTiming.ScanMode = (!InterLaced);
    iTE6615_CurVTiming.PCLK = ((int)iTE6615_DATA.TXCLK)*2;
    iTE6615_CurVTiming.PixRpt = iTE6615_DATA.config.PixRpt;  // 1 = repeat 0 (1-1) times  , 2 = repeat one times

    // Auto Set VIC
    //iTE6615_DATA.VIndex = iTE6615_Get_VIndex_ByTiming(&iTE6615_CurVTiming);
    //if(iTE6615_DATA.VIndex > 0)
    //{
    //    iTE6615_CurVTiming.VIC = iTE6615_VTable[iTE6615_DATA.VIndex].VIC; // set index
    //}

    // need to set VSIF when VIC = 93, 94, 95, 98
    if( iTE6615_CurVTiming.VIC > 0 &&
        iTE6615_DATA.config.Infoframe_Enable3D == 0 &&
        iTE6615_DATA.Flag_Force_Set_VIC_0 == TRUE )
    {
        switch ( iTE6615_CurVTiming.VIC )
        {
            case 95: iTE6615_DATA.vars.HDMI_VIC = 0x01; iTE6615_DATA.Flag_Force_Set_VIC_0 = TRUE;break;
            case 94: iTE6615_DATA.vars.HDMI_VIC = 0x02; iTE6615_DATA.Flag_Force_Set_VIC_0 = TRUE;break;
            case 93: iTE6615_DATA.vars.HDMI_VIC = 0x03; iTE6615_DATA.Flag_Force_Set_VIC_0 = TRUE;break;
            case 98: iTE6615_DATA.vars.HDMI_VIC = 0x04; iTE6615_DATA.Flag_Force_Set_VIC_0 = TRUE;break;
            default: iTE6615_DATA.vars.HDMI_VIC = 0x00; iTE6615_DATA.Flag_Force_Set_VIC_0 = FALSE;break;
        }
    }
}

// Show
void iTE6615_Show_VideoInfo(void)
{
    if (iTE6615_DATA.config.EnHDMI){HDMITX_DEBUG_PRINTF(("\r\nHDMI Mode\r\n"));}
    else{HDMITX_DEBUG_PRINTF(("\r\nDVI Mode\r\n"));}

    HDMITX_DEBUG_PRINTF(("Input : \r\n"));
    switch (iTE6615_DATA.config.Input_ColorSpace)
    {
        case RGB444 :
            HDMITX_DEBUG_PRINTF(("\tCSC = RGB444\r\n")); break;
        case YUV422 :
            HDMITX_DEBUG_PRINTF(("\tCSC = YUV422\r\n")); break;
        case YUV444 :
            HDMITX_DEBUG_PRINTF(("\tCSC = YUV444\r\n")); break;
        default :
            HDMITX_DEBUG_PRINTF(("\tCSC = YUV420\r\n")); break;
    }

    if (iTE6615_DATA.config.Input_ColorSpace)
    {
        if (iTE6615_DATA.config.Input_YCC_Colorimetry==ITU709)
            HDMITX_DEBUG_PRINTF(("\tYUVColorimetry = ITU709\r\n"));
        else
            HDMITX_DEBUG_PRINTF(("\tYUVColorimetry = ITU601\r\n"));
    }

    HDMITX_DEBUG_PRINTF(("Output : \r\n"));

    switch (iTE6615_DATA.config.Output_ColorSpace) {
        case RGB444 :
            HDMITX_DEBUG_PRINTF(("\tCSC = RGB444\r\n")); break;
        case YUV422 :
            HDMITX_DEBUG_PRINTF(("\tCSC = YUV422\r\n")); break;
        case YUV444 :
            HDMITX_DEBUG_PRINTF(("\tCSC = YUV444\r\n")); break;
        default :
            HDMITX_DEBUG_PRINTF(("\tCSC = YUV420\r\n")); break;
    }

    switch (iTE6615_DATA.config.Output_ColorDepth)
    {
        case VID10BIT :
            HDMITX_DEBUG_PRINTF(("\tColorDepth = 10bpc"));
            #if iTE6264
            HDMITX_DEBUG_PRINTF((" (6264 LVDS ColorDepth Max to 10bpc (LVDS Limitation))\r\n"));
            #endif
            HDMITX_DEBUG_PRINTF(("\r\n"));
            break;
        case VID12BIT :
            HDMITX_DEBUG_PRINTF(("\tColorDepth = 12bpc\r\n")); break;
        default :
            HDMITX_DEBUG_PRINTF(("\tColorDepth = 8bpc\r\n")); break;
    }

    if( iTE6615_DATA.config.Output_ExtendedColorimetry_Enable )
    {
        HDMITX_DEBUG_PRINTF(("\tExtended Colorimetry Enabled-"));
        switch( iTE6615_DATA.config.Output_ExtendedColorimetry_Format )
        {
            case xvYCC601    : HDMITX_DEBUG_PRINTF(("xvYCC601")); break;
            case xvYCC709    : HDMITX_DEBUG_PRINTF(("xvYCC709")); break;
            case sYCC601     : HDMITX_DEBUG_PRINTF(("sYCC601")); break;
            case AdobeYCC601 : HDMITX_DEBUG_PRINTF(("AdobeYCC601")); break;
            case AdobeRGB    : HDMITX_DEBUG_PRINTF(("AdobeRGB")); break;
            case BT2020cYCC  : HDMITX_DEBUG_PRINTF(("BT2020cYCC")); break;
            case BT2020YCC   : HDMITX_DEBUG_PRINTF(("BT2020YCC")); break;
            default          : break;
        }
        HDMITX_DEBUG_PRINTF(("\r\n"));
    }



    HDMITX_DEBUG_PRINTF(("\tPCLK = %u MHz\r\n\r\n", iTE6615_DATA.VCLK/1000));

    HDMITX_DEBUG_PRINTF(("\tHActive  = %d\r\n", iTE6615_CurVTiming.HActive));
    HDMITX_DEBUG_PRINTF(("\tVActive  = %d\r\n\r\n", iTE6615_CurVTiming.VActive));

    HDMITX_DEBUG_PRINTF(("\tHTotal  = %d\r\n", iTE6615_CurVTiming.HTotal));
    HDMITX_DEBUG_PRINTF(("\tHFPH    = %d\r\n", iTE6615_CurVTiming.HFrontPorch));
    HDMITX_DEBUG_PRINTF(("\tHSyncW  = %d\r\n", iTE6615_CurVTiming.HSyncWidth));
    HDMITX_DEBUG_PRINTF(("\tHBPH    = %d\r\n", iTE6615_CurVTiming.HBackPorch));
    HDMITX_DEBUG_PRINTF(("\tHSyncPol= %d\r\n", iTE6615_CurVTiming.HPolarity));
    HDMITX_DEBUG_PRINTF(("\tVTotal  = %d\r\n", iTE6615_CurVTiming.VTotal));
    HDMITX_DEBUG_PRINTF(("\tVFPH    = %d\r\n", iTE6615_CurVTiming.VFrontPorch));
    HDMITX_DEBUG_PRINTF(("\tVSyncW  = %d\r\n", iTE6615_CurVTiming.VSyncWidth));
    HDMITX_DEBUG_PRINTF(("\tVBPH    = %d\r\n", iTE6615_CurVTiming.VBackPorch));
    HDMITX_DEBUG_PRINTF(("\tVSyncPol= %d\r\n", iTE6615_CurVTiming.VPolarity));
    HDMITX_DEBUG_PRINTF(("\tInterLaced= %d\r\n\r\n",(int) !iTE6615_CurVTiming.ScanMode));

    if( iTE6615_DATA.config.Infoframe_Enable3D )
    {
        HDMITX_DEBUG_PRINTF(("\r\n\t Enable 3D output by Infoframe :\r\n"));
        switch( iTE6615_DATA.config.Infoframe_3D_Structure )
        {
            case _3D_FramePacking   : HDMITX_DEBUG_PRINTF(("FramePacking\r\n")); break;
            case _3D_SideBySide_Full: HDMITX_DEBUG_PRINTF(("SideBySide_Full\r\n")); break;
            case _3D_TopBottom      : HDMITX_DEBUG_PRINTF(("TopBottom\r\n")); break;
            case _3D_SideBySide_Half: HDMITX_DEBUG_PRINTF(("SideBySide_Half\r\n")); break;
            default : HDMITX_DEBUG_PRINTF(("Unsupport 3D Video Input\r\n")); break;
        }
    }
    else
    {
        HDMITX_DEBUG_PRINTF(("2D Video Input\r\n"));
    }

    //HDMITX_DEBUG_PRINTF(("Video Input Timing: %s, PixRpt=%d\r\n", iTE6615_VTable[iTE6615_DATA.VIndex].format, iTE6615_CurVTiming.PixRpt));
    HDMITX_DEBUG_PRINTF(("EnDualIn=%d ,EnDEOnly=%d\r\n", (int) iTE6615_DATA.config.Input_Enable_DualMode,(int) iTE6615_DATA.config.Enable_DEOnly_Mode));

    #ifdef iTE6615
    HDMITX_DEBUG_PRINTF(("TTL : EnInDDR=%d, EnHalfBus=%d\r\n",
                        (int)iTE6615_DATA.config.TTL_Enable_DDR,
                        (int)iTE6615_DATA.config.TTL_Enable_HalfBus));
    #endif

    #if iTE6264
    HDMITX_DEBUG_PRINTF(("LVDS : \r\n"));
    HDMITX_DEBUG_PRINTF(("LVDS ChannelNum : "));
    switch(iTE6615_DATA.config.LVDS_RxChNum)
    {
        case LVDS1CH: HDMITX_DEBUG_PRINTF(("1"));break;
        case LVDS2CH: HDMITX_DEBUG_PRINTF(("2"));break;
        case LVDS4CH: HDMITX_DEBUG_PRINTF(("4"));break;
        default: break;
    }
    HDMITX_DEBUG_PRINTF(("\r\n"));
    HDMITX_DEBUG_PRINTF(("\r\nSSCPCLKL=%ld MHz, SSCPCLKH=%ld MHz\r\n", iTE6615_DATA.SSCPCLKL/1000, iTE6615_DATA.SSCPCLKH/1000));

    #endif

    HDMITX_DEBUG_PRINTF(("\r\n"));
    HDMITX_DEBUG_PRINTF(("RCLK=%ld MHz\r\n", iTE6615_DATA.RCLK / 1000));
    HDMITX_DEBUG_PRINTF(("ICLK=%ld MHz\r\n", iTE6615_DATA.ICLK / 1000));
    HDMITX_DEBUG_PRINTF(("PCLK=%ld MHz\r\n", iTE6615_DATA.PCLK / 1000));
    HDMITX_DEBUG_PRINTF(("TXCLK=%ld MHz\r\n",iTE6615_DATA.TXCLK / 1000));
    HDMITX_DEBUG_PRINTF(("VCLK=%ld MHz\r\n", iTE6615_DATA.VCLK / 1000));
    HDMITX_DEBUG_PRINTF(("VCLK_expected=%ld MHz\r\n", iTE6615_DATA.VCLK_expected / 1000));
    HDMITX_DEBUG_PRINTF(("\r\n"));



}

void iTE6615_Show_AudioCTS(void)
{
#if 1
    u32 ACLK, audN, aud_cts_cal, aud_freq_cnt, aud_cts_cnt, tmp;

    chgtxbank( 1 );
    audN = hdmitxrd(0x33);
    tmp = hdmitxrd(0x34);
    audN |= (tmp<<8);
    tmp =hdmitxrd(0x35)&0x0F;
    audN |= (tmp<<16);

    aud_cts_cnt = ((hdmitxrd(0x35)&0xF0)>>4);
    tmp = hdmitxrd(0x36);
    aud_cts_cnt |= (tmp<<4);
    tmp = hdmitxrd(0x37);
    aud_cts_cnt |= (tmp<<12);

    chgtxbank(0);
    switch ( iTE6615_DATA.config.Audio_Frequency )
    {
        case AUD32K  :
            ACLK = 320; break;
        case AUD44K  :
            ACLK = 441; break;
        case AUD48K  :
            ACLK = 480; break;
        case AUD64K  :
        case AUD256K :
            ACLK = 640; break;
        case AUD88K  :
        case AUD352K :
            ACLK = 882; break;
        case AUD96K  :
        case AUD384K :
            ACLK = 960; break;
        case AUD128K :
        case AUD512K :
            ACLK = 1280; break;
        case AUD176K :
        case AUD705K :
            ACLK = 1764; break;
        case AUD192K :
        case AUD768K :
            ACLK = 1920; break;
        case AUD1024K:
            ACLK = 2560; break;
        case AUD1411K:
            ACLK = 3528; break;
        case AUD1536K:
            ACLK = 3840; break;
        default :
            HDMITX_AUDIO_PRINTF(("ACLK Error!!!\r\n"));
    }

    aud_cts_cal = (u32)(iTE6615_DATA.VCLK*audN/12.8/ACLK);

    HDMITX_AUDIO_PRINTF(("Audio Time Stamp N = 0x%X\r\n", (int)audN));
    HDMITX_AUDIO_PRINTF(("Audio Time Stamp CTSCal = 0x%X\r\n", (int)aud_cts_cal));
    HDMITX_AUDIO_PRINTF(("Audio Time Stamp CTSCnt = 0x%X\r\n", (int)aud_cts_cnt));
    HDMITX_AUDIO_PRINTF(("\r\n"));

    aud_freq_cnt = hdmitxrd(0xEE);

    HDMITX_AUDIO_PRINTF(("Audio Frequecncy Count = 0x%X\r\n", aud_freq_cnt));
    if ( iTE6615_DATA.config.Audio_Type == DSD )
    {
        tmp = 28*1000*iTE6615_DATA.RCLK / 2 / aud_freq_cnt / 64;
        HDMITX_AUDIO_PRINTF(("Input Audio Frequency = %ld.%ld KHz\r\n\r\n", tmp/1000, tmp%1000));
    }
    else
    {
        tmp = 1000*iTE6615_DATA.RCLK / 2 / aud_freq_cnt;
        HDMITX_AUDIO_PRINTF(("Input Audio Frequency = %ld.%ld KHz\r\n\r\n", tmp/1000, tmp%1000));
    }

    if ( iTE6615_DATA.config.Audio_Output_Sel == SPDIF && iTE6615_DATA.config.Audio_Enable_MCLKSamplingMode )
    {
         HDMITX_AUDIO_PRINTF(("Enable SPDIF MCLK-Sampling Mode \r\n"));
    }
    else if ( iTE6615_DATA.config.Audio_Output_Sel == SPDIF &&
              iTE6615_DATA.config.Audio_Enable_SPDIF_OSMode &&
              iTE6615_DATA.config.Audio_Type != DSD )
    {
        u16 RefOSFreqCnt;
        RefOSFreqCnt = hdmitxrd( 0xE5 );
        RefOSFreqCnt += ((hdmitxrd( 0xE6 ) & 0x3F) << 8);
        HDMITX_AUDIO_PRINTF(("Enable Enhanced SPDIF Over-Sampling Mode \r\n"));
        HDMITX_AUDIO_PRINTF(("Current RefAutoOSCLKSel=%d, RefASCLKDiv2=%d, RefASCLKDiv4=%d, RefOSFreqCnt=0x%04X\r\n",
                (hdmitxrd( 0x21 ) & 0x30) >> 4,
                (hdmitxrd( 0x23 ) & 0x02) >> 1,
                (hdmitxrd( 0x23 ) & 0x04) >> 2, RefOSFreqCnt));

        if ( (RefOSFreqCnt >> 6) > 8 && (hdmitxrd( 0x23 ) & 0x06) == 0x00 )
        {
            HDMITX_AUDIO_PRINTF(("Over-sampling frequency is too high, should use ASCLK Division !!!\r\n"));
            hdmitxset( 0x05, 0x80, 0x80 );
            hdmitxset( 0x05, 0x80, 0x00 );
        }
        HDMITX_AUDIO_PRINTF(("\r\n"));
    }
#endif
}

// RCLK
void iTE6615_RCLK_Load( void )
{
    u32 sum;
    u8 sip_data[4];

    if ( iTE6615_DATA.RCLK == 0 )
    {
        chgtxbank(0);
        hdmitxwr(0xFF, 0xC3);
        hdmitxwr(0xFF, 0xA5);
        chgtxbank(2);
        hdmitxwr(0x5f, 0x04);
        hdmitxwr(0x5f, 0x05);
        hdmitxwr(0x58, 0x12);
        hdmitxwr(0x58, 0x02);
        hdmitxwr(0x57, 0x01);
        hdmitxwr(0x50, 0x00);
        hdmitxwr(0x50, 0x00);
        hdmitxwr(0x51, 0xb0);
        hdmitxwr(0x54, 0x04);

        hdmitxbrd(0x61,4,sip_data);

        hdmitxset(0x5f, 0x05, 0x00);

        chgtxbank(0);
        hdmitxwr(0xFF, 0x00);
        if ( (sip_data[0] == 0x00) && (sip_data[1] == 0x00) && (sip_data[2] == 0xff) )
        {
            HDMITX_DEBUG_PRINTF(("data = %02x %02x %02x %02x\r\n", (int)sip_data[0], (int)sip_data[1], (int)sip_data[2], (int)sip_data[3]));
        }
        else
        {
            sum = sip_data[2];
            sum <<=8;
            sum |= sip_data[1];
            sum <<=8;
            sum |= sip_data[0];

            iTE6615_DATA.RCLK = (sum ) / 100;

            if ( FALSE == iTE6615_Check_RCLK_Valid(iTE6615_DATA.RCLK) )
            {
                iTE6615_DATA.RCLK = 0;
            }
        }
    }

    HDMITX_DEBUG_PRINTF(("load RCLK=%ld MHz\r\n", iTE6615_DATA.RCLK/1000));
}

void iTE6615_RCLK_Set()
{
    u32 TimerInt, TimerFlt;

    HDMITX_DEBUG_PRINTF(("RCLKVALUE=%ld.%02ldMHz\r\n", iTE6615_DATA.RCLK / 1000, (iTE6615_DATA.RCLK % 1000) / 10));

    #if _ENABLE_CEC_
    iTE6615_DATA.vars.cec_time_unit = (iTE6615_DATA.RCLK * 100 / 16)/1000;
    HDMITX_DEBUG_PRINTF(("Set CEC time unit = 0x%02X\r\n", (int)iTE6615_DATA.vars.cec_time_unit));
    #endif

    TimerInt = iTE6615_DATA.RCLK / 1000;
    TimerFlt = (iTE6615_DATA.RCLK % 1000) * 256 / 1000;
    chgtxbank(1);
    hdmitxwr(0x14, TimerInt);
    hdmitxwr(0x15, TimerFlt);
    chgtxbank(0);

}

// HDCP

u8 iTE6615_HDCP_Read(u8 offset, u16 bytenum)
{
    u8 result;
    u8 retry = 1;
    u8 tmp;

__RETRY:

    if (!iTE6615_CheckTX_HPD()) {
        HDMITX_DEBUG_PRINTF(("Abort hdcp read becasue of detecting unplug !!!\r\n"));
        return FALSE;
    }

    iTE6615_SCDC_IRQ();

    hdmitxset(0x28, 0x01, 0);
    //hdmitxwr(0x2E, 0x0F);         // Abort DDC Command
    hdmitxwr(0x2E, 0x09);           // DDC FIFO Clear
    hdmitxwr(0x29, 0x74);           // HDCP Address
    hdmitxwr(0x2A, offset);         // HDCP Offset

    tmp = bytenum & 0xFF;
    hdmitxwr(0x2B, tmp);            // Read ByteNum[7:0]

    bytenum = bytenum>>8;
    tmp = bytenum & 0x03;
    hdmitxwr(0x2C, tmp);            // ByteNum[9:8]
                                    //
    hdmitxwr(0x2E, 0x00);           // HDCP Read Fire

    result = iTE6615_DDC_Wait();

    hdmitxset(0x28, 0x01, 0x00);    // Disable PC DDC Mode

    if ( result == FALSE ) {
        if ( retry > 0 ) {
            retry--;
            delay1ms( 10 );
            goto __RETRY;
        }
    }

    return result;
}


// HDCP

void iTE6615_HDCP1_Enable( void )
{
    u32 wait_count;
    u8 BKSV[5];

    hdmitxset(0x42, 0x10, 0x00);    // RegHDCPSel=0
    hdmitxset(0x19, 0x04, 0x04);    // Enable DDC Command Fail Interrupt
    hdmitxset(0x1D, 0x08, 0x08);    // Enable DDC Bus Hang Interrupt
    hdmitxset(0x41, 0x01, 0x00);    // Disable HDCP_Desired

    // Reset HDCP Module
    hdmitxset(0x05, 0x40, 0x40);
    delay1ms(2);
    hdmitxset(0x05, 0x40, 0x00);

    // set HDCP Option
    hdmitxwr(0xFF, 0xC3);
    hdmitxwr(0xFF, 0xA5);
    hdmitxset(0x41, 0x80, 0x80);
    hdmitxset(0x57, 0x01, 0x00);
    hdmitxset(0x41, 0x80, 0x00);
    hdmitxset(0xC2, 0x40, 0x00);
    hdmitxset(0xC2, 0x1F, 0x0A);    // For UITA-2000 ATC
    hdmitxset(0x6B, 0x03, (config_DisRiShortRead << 1) + config_DisR0ShortRead);
    hdmitxwr(0xFF, 0xFF);

    hdmitxset(0x41, 0x0E, (config_KeepOutOpt<<3) + (config_EnSyncDetChk << 2) + (config_HDCP_Enable1p1 << 1));
    hdmitxset(0x6F, 0x0F, (config_HDCP_EnableAutoMute << 3) + (config_EnSyncDet2FailInt << 2) + (config_EnRiChk2DoneInt << 1) + config_EnAutoReAuth);

    // Enable Authentication Fail/Done/KSVListChk Interrupt
    hdmitxset(0x1A, 0x07, 0x07);

    // Clear Ri/Pj Done Interrupt
    hdmitxwr(0x16, 0x30);

    // Enable Ri Done Interrupt
    //hdmitxset(0x1D, 0x30, 0x10);

    // Disable Ri Done Interrupt kuro set to disable
    hdmitxset(0x1D, 0x30, 0x00);

    hdmitxset(0x40, 0x01, 0x01);  // Enable An Generator
    delay1ms(1);
    hdmitxset(0x40, 0x01, 0x00);  // Stop An Generator

    hdmitxwr(0x48, hdmitxrd(0x50));
    hdmitxwr(0x49, hdmitxrd(0x51));
    hdmitxwr(0x4A, hdmitxrd(0x52));
    hdmitxwr(0x4B, hdmitxrd(0x53));
    hdmitxwr(0x4C, hdmitxrd(0x54));
    hdmitxwr(0x4D, hdmitxrd(0x55));
    hdmitxwr(0x4E, hdmitxrd(0x56));
    hdmitxwr(0x4F, hdmitxrd(0x57));

    // Enable HDCP_Desired
    hdmitxset(0x41, 0x01, 0x01);
    iTE6615_DATA.vars.RxHDMIMode = 0;
    wait_count = 0;

    while (iTE6615_DATA.config.EnHDMI && !iTE6615_DATA.vars.RxHDMIMode)
    {
        if (iTE6615_HDCP_Read(0x41, 2)) //Read BStatus;
        {
            iTE6615_DATA.vars.RxHDMIMode = (hdmitxrd(0x65) & 0x10) >> 4;
            HDMITX_DEBUG_PRINTF(("Enable HDCP Fire %d => Current RX HDMI MODE=%d \r\n",(int) iTE6615_DATA.vars.HDCPFireCnt++,(int) iTE6615_DATA.vars.RxHDMIMode));
        }
        else
        {
            HDMITX_DEBUG_PRINTF(("DDC ERROR: When waiting for RX HDMI_MODE change to 1 ...\r\n"));
        }

        if (wait_count++ == 10) {
            HDMITX_DEBUG_PRINTF(("\r\nERROR: RX HDMI_MODE keeps in 0 Time-Out !!!\r\n\r\n"));
            break;
        }

        if (!(iTE6615_CheckTX_HPD() && iTE6615_CheckTX_RXSENSE()) ) {
            HDMITX_DEBUG_PRINTF(("Abort hdcp read becasue of detecting unplug !!!\r\n"));
            break;
        }
    }

    if (iTE6615_HDCP_Read(0x00, 5))
    {
        hdmitxbrd(0x5B, 5, &BKSV[0]);
        if (iTE6615_HDCP_Check_Revocation(BKSV))
        {
            iTE6615_DATA.STATEH = STATEH_Fail;
        }
        else
        {
            // HDCP fire
            hdmitxset(0x42, 0x01, 0x01);
            iTE6615_DATA.STATEH = STATEH_CPGoing;
        }
    }
    else
    {
        HDMITX_DEBUG_PRINTF(("Failed to read BKSV !!!\r\n"));
        iTE6615_DATA.STATEH = STATEH_Fail;
    }

    if (iTE6615_DATA.STATEH == STATEH_Fail)
    {
        iTE6615_TASK_HDCP_RetryAuth();
    }
}



void iTE6615_HDCP2_Enable( void )
{
    u8 RxHDCP2, EnRepWaitTxMgm, EnRepWaitTxEks, HDCP_Enable2ListSRMChk;

    if( (hdmitxrd(0x07)&0x06)!=0x06 ) {
        iTE6615_sys_chg(iTE6615_STATES_Unplug);
        return;
    }

    // kuro this section can be set to init from here
    hdmitxset(0x42, 0x10, 0x10);    // RegHDCPSel=1
    hdmitxset(0x19, 0x04, 0x04);    // Enable DDC Command Fail Interrupt
    hdmitxset(0x1D, 0x08, 0x08);    // Enable DDC Bus Hang Interrupt
    hdmitxset(0x41, 0x01, 0x00);    // Disable HDCP_Desired
    hdmitxset(0x13, 0x01, 0x01);    // kuro add

    // Reset HDCP Module
    hdmitxset(0x05, 0x48, 0x48);
    delay1ms(2);
    hdmitxset(0x05, 0x48, 0x00);
    hdmitxset(0x05, 0x10, 0x00);

    if ( iTE6615_DATA.config.HDCP_Enable_RepeaterMode ) {
        EnRepWaitTxMgm = 1;
        EnRepWaitTxEks = 1;
        HDCP_Enable2ListSRMChk = 1;
    }
    else{
        EnRepWaitTxMgm = 0;
        EnRepWaitTxEks = 0;
        HDCP_Enable2ListSRMChk = 0;
    }

    // set HDCP Option
    hdmitxwr(0xFF, 0xC3);
    hdmitxwr(0xFF, 0xA5);
    hdmitxset(0x6B, 0x01, config_DisR0ShortRead);
    hdmitxset(0x6B, 0x30, (EnRepWaitTxMgm<<5)+(EnRepWaitTxEks<<4));
    hdmitxwr(0xFF, 0xFF);

    hdmitxset(0x48, 0x0F, config_LClimit);
    hdmitxset(0x6F, 0x09, (config_HDCP_EnableAutoMute<<3)+config_EnAutoReAuth);

    // Enable Authentication Fail/Done/KSVListChk Interrupt
    hdmitxset(0x1A, 0x07, 0x07);

    hdmitxset(0x50, 0xFF, 0x02);

    // Enable HDCP_Desired
    hdmitxset(0x41, 0x01, 0x01);

    if (iTE6615_HDCP_Read(0x50, 1))
    {
        RxHDCP2 = hdmitxrd(0x4B) & 0x01;    // HDCP2Version
        HDMITX_DEBUG_PRINTF(("HDCP2Version = %d \r\n",(int) RxHDCP2));
    }
    else
    {
        HDMITX_DEBUG_PRINTF(("HDCP2Version read fail, set to HDCP1\r\n"));
        RxHDCP2 = 0;
    }


    if ( RxHDCP2 )
    {
        if ((iTE6615_CurVTiming.HActive < 3800) || (iTE6615_CurVTiming.VActive < 2000))
        {
            HDMITX_DEBUG_PRINTF(("Still Enable HDCP2.2 for non-4k mode %ux%u\r\n", iTE6615_CurVTiming.HActive, iTE6615_CurVTiming.VActive));
            //RxHDCP2 = 0;
        }
    }

    // force to fire HDCP2.2 , RxHDCP2 = 1
    //RxHDCP2 = 1; // kuro test
    if (RxHDCP2)
    {
        HDMITX_DEBUG_PRINTF(("Support HDCP2.2 = TRUE\r\n"));

        hdmitxset(0x70, 0x10, 0x10);

        // [6]:RegHDCP_Enable2ListSRMChk, [5]:RegHDCP_Enable2SRMChk
        hdmitxset(0x42, 0x60, (HDCP_Enable2ListSRMChk << 6) );

        // [7]=RHDCP2RepWaitTxMgm_Int
        hdmitxset(0x1D, 0x80, 0x80);

        // [7]=RHDCP2RepWaitTxEks_Int
        // [6]=RHDCP2ListSRMChk_Int
        // [3]=RHDCP2StrmMgmFail_Int
        // [2]=RHDCP2RxReauthReq_Int
        // [1]=RHDCP2 SRMChk_Int
        hdmitxset(0x1E, 0xCE, 0xCE);

        // [7]=RegFixRandErr,
        // [6]=RegEnMrolloverChk, [5]=RegEnMFailRetry
        // [4]=RegDisMRetryLimit, [3]=RegEnSeq_num_VChk
        // [2]=RegEnPollRdyOnly,  [1:0]=RegAuthEncTol
        // RegAuthEncTol="11" will cause encryption enable before 200ms
        hdmitxset(0x4A, 0xFF, 0x20+(config_EnHDCP2TxPollRdyOnly<<2));    // kuro modify

        // [7:6]=RegAuthPollTol
        // [5]=RegForceNo_Stored_Km, [4]=RegEnHwVerChk
        // [3]=RegMsgPollStart,      [2]=RegMsgPollGap
        // [1:0]=RegMsgPollTOTol
        hdmitxset(0x49, 0xFF, 0x00);

        hdmitxset(0x51, 0x01, config_EnHDCP2MsgPoll10ms);    // [0]=RegMsgPoll10ms
        hdmitxset(0x42, 0x08, 0x08);

        //delay1ms(200);  // kuro mark

        //iTE6615_DDC_FIFOClear();

        hdmitxset(0x42, 0x01, 0x01);    // HDCP2 authentication fire

        iTE6615_DATA.Flag_HDCP2x_Enable = TRUE;

    }
    else
    {
        HDMITX_DEBUG_PRINTF(("Support HDCP2.2 = FALSE\r\n"));
        HDMITX_DEBUG_PRINTF(("Try HDCP1 Authentication ... \r\n"));

        iTE6615_DATA.Flag_HDCP2x_Enable = FALSE;
        iTE6615_HDCP1_Enable();
    }
}


void iTE6615_HDCP1_Show_Status( void )
{
    u8 tmp[8];
    u8 BCaps, BStatus1, BStatus2, status;

    status = hdmitxrd(0x66);

    BCaps = hdmitxrd(0x63);
    BStatus1 = hdmitxrd(0x64);
    BStatus2 = hdmitxrd(0x65);

    // Read An
    hdmitxbrd(0x48, 8, tmp);
    HDCP_DEBUG_PRINTF(("An = %02X %02X %02X %02X - %02X %02X %02X %02X\r\n",
        (int)tmp[0],(int)tmp[1],(int)tmp[2],(int)tmp[3],(int)tmp[4],(int)tmp[5],(int)tmp[6],(int)tmp[7]));

    // Read AKSV
    hdmitxbrd(0x43, 5, tmp);
    HDCP_DEBUG_PRINTF(("AKSV = %02X %02X %02X %02X %02X\r\n",
        (int)tmp[0],(int)tmp[1],(int)tmp[2],(int)tmp[3],(int)tmp[4]));

    // Read BKSV
    hdmitxbrd(0x5B, 5, tmp);
    HDCP_DEBUG_PRINTF(("BKSV = %02X %02X %02X %02X %02X\r\n",
        (int)tmp[0],(int)tmp[1],(int)tmp[2],(int)tmp[3],(int)tmp[4]));

    iTE6615_HDCP_Show_RI();

    HDCP_DEBUG_PRINTF(("Downstream count = %d\r\n", (int)(BStatus1 & 0x7F)));
    HDCP_DEBUG_PRINTF(("MAX_DEVS_EXCEEDED = %d\r\n", (int)((BStatus1 >> 7)&0x01)));
    HDCP_DEBUG_PRINTF(("BStatus[15:8] = 0x%02X\r\n", (int)(BStatus2)));


    HDCP_DEBUG_PRINTF(("Rx HDCP Fast Reauthentication = %d \r\n", BCaps & 0x01));
    HDCP_DEBUG_PRINTF(("Rx HDCP 1.1 Features = %d ", (int)((BCaps & 0x02) >> 1)));

    if( (BCaps&0x02) && config_HDCP_Enable1p1) {
        HDCP_DEBUG_PRINTF(("Enabled\r\n"));
    }
    else{
        HDCP_DEBUG_PRINTF(("Disabled\r\n"));
    }

    if (BCaps & 0x10) {
        HDCP_DEBUG_PRINTF(("Rx HDCP Maximum DDC Speed = 400KHz\r\n"));
    }
    else {
        HDCP_DEBUG_PRINTF(("Rx HDCP Maximum DDC Speed = 100KHz\r\n"));
    }

    HDMITX_DEBUG_PRINTF(("Rx HDCP Repeater = %d \r\n", (int)((BCaps & 0x40) >> 6)));

    HDMITX_DEBUG_PRINTF(("Tx Auth Status = 0x%02X \r\n", (int)status));

    if ((status & 0x80) != 0x80) {
        if (status & 0x01) {
            HDMITX_DEBUG_PRINTF((": DDC NAck too may times !!!\r\n"));
        }

        if (status & 0x02) {
            HDCP_DEBUG_PRINTF((": BKSV Invalid !!!\r\n"));
        }

        if (status & 0x04) {
            HDCP_DEBUG_PRINTF((": Link Check Fail (AR0/=BR0) !!!\r\n"));
        }

        if (status & 0x08) {
            HDCP_DEBUG_PRINTF((": Link Integrity Ri Check Fail !!!\r\n"));
            iTE6615_HDCP_Show_RI();
        }

        if (status & 0x10) {
            HDCP_DEBUG_PRINTF((": Link Integrity Pj Check Fail !!!\r\n"));
        }

        if (status & 0x20) {
            u8 ARi1, ARi2, BRi1, BRi2;
            u8 preARi1, preARi2, preBRi1, preBRi2;

            HDCP_DEBUG_PRINTF((": Link Integrity HDCP 1.2 Sync Detect Fail !!!\r\n"));

            ARi1 = hdmitxrd(0x58);
            ARi2 = hdmitxrd(0x59);
            BRi1 = hdmitxrd(0x60);
            BRi2 = hdmitxrd(0x61);
            HDCP_DEBUG_PRINTF(("Current ARi = 0x%02X%02X \r\n", (int)ARi2 , (int)ARi1));
            HDCP_DEBUG_PRINTF(("Current BRi = 0x%02X%02X \r\n", (int)BRi2 , (int)BRi1));

            hdmitxset(0x70, 0x08, 0x00);
            preARi1 = hdmitxrd(0x76);
            preARi2 = hdmitxrd(0x77);
            hdmitxset(0x70, 0x08, 0x08);
            preBRi1 = hdmitxrd(0x76);
            preBRi2 = hdmitxrd(0x77);
            hdmitxset(0x70, 0x08, 0x00);
            HDCP_DEBUG_PRINTF(("Previous ARi = 0x%02X%02X \r\n", (int)preARi2 , (int)preARi1));
            HDCP_DEBUG_PRINTF(("Previous BRi = 0x%02X%02X \r\n", (int)preBRi2 , (int)preBRi1));

            if (BRi1 == preARi1 && BRi2 == preARi2) {
                HDCP_DEBUG_PRINTF(("CurBRi==PreARi ==> Sync Detection Error because of Miss CTLx signal\r\n"));
            }
            if (ARi1 == preBRi1 && ARi2 == preBRi2) {
                HDCP_DEBUG_PRINTF(("CurARi==PreBRi ==> Sync Detection Error because of Extra CTLx signal\r\n"));
            }

        }

        if (status & 0x40) {
            HDCP_DEBUG_PRINTF((": DDC Bus Hang TimeOut !!!\r\n"));
        }
    }
}

void iTE6615_HDCP2_Show_Status( void )
{
    u16 HDCP2FailStatus;
    u8 tmp[5];

    // Read RxID
    chgtxbank(0);
    hdmitxbrd(0x5B, 5, tmp);
    HDCP_DEBUG_PRINTF(("RxID = %02X %02X %02X %02X %02X\r\n",
        (int)tmp[4],(int)tmp[3],(int)tmp[2],(int)tmp[1],(int)tmp[0]));

    HDCP2FailStatus = (hdmitxrd(0x4F) << 8) + hdmitxrd(0x4E);

    HDCP_DEBUG_PRINTF(("HDCP2Tx Auth Fail Status %04X \r\n", (int)HDCP2FailStatus));
    if (HDCP2FailStatus != 0x0000) {
        if (HDCP2FailStatus & 0x0001) {
            HDCP_DEBUG_PRINTF(("Auth Fail: RxHDCP2 Capability=0 !\r\n"));
        }
        if (HDCP2FailStatus & 0x0002) {
            HDCP_DEBUG_PRINTF(("Auth Fail: Locality Check fail !\r\n"));
        }
        if (HDCP2FailStatus & 0x0004) {
            HDCP_DEBUG_PRINTF(("Auth Fail: Read AKE Send Cert 100ms timeout !\r\n"));
        }
        if (HDCP2FailStatus & 0x0008) {
            HDCP_DEBUG_PRINTF(("Auth Fail: Read AKE Send H prime (km_stored_rdy=1) 200ms timeout !\r\n"));
        }
        if (HDCP2FailStatus & 0x0010) {
            HDCP_DEBUG_PRINTF(("Auth Fail: Read AKE Send H prime (km_stored_rdy=0) 1sec timeout !\r\n"));
        }
        if (HDCP2FailStatus & 0x0020) {
            HDCP_DEBUG_PRINTF(("Auth Fail: Read AKE Send Pairing Info 200ms timeout !\r\n"));
        }
        if (HDCP2FailStatus & 0x0040) {
            HDCP_DEBUG_PRINTF(("Auth Fail: Read Repeater Receiver ID List 3sec timeout !\r\n"));
        }
        if (HDCP2FailStatus & 0x0080) {
            HDCP_DEBUG_PRINTF(("Auth Fail: Signature Verified fail !\r\n"));
        }
        if (HDCP2FailStatus & 0x0100) {
            HDCP_DEBUG_PRINTF(("Auth Fail: H Verified fail !\r\n"));
        }
        if (HDCP2FailStatus & 0x0200) {
            HDCP_DEBUG_PRINTF(("Auth Fail: A1/A5 polling Rxstatus ReAuth_Req = 1 !\r\n"));
        }
        if (HDCP2FailStatus & 0x0400) {
            HDCP_DEBUG_PRINTF(("Auth Fail: Repeater device count/cascade exceed or seq_num_V Error !\r\n"));
        }
        if (HDCP2FailStatus & 0x0800) {
            HDCP_DEBUG_PRINTF(("Auth Fail: A6 polling Rxstatus ReAuth_Req = 1 !\r\n"));
        }
        if (HDCP2FailStatus & 0x1000) {
            HDCP_DEBUG_PRINTF(("Auth Fail: V Verified fail !\r\n"));
        }
        if (HDCP2FailStatus & 0x2000) {
            HDCP_DEBUG_PRINTF(("Auth Fail: seq_num_M rolls over error !\r\n"));
        }
        if (HDCP2FailStatus & 0x4000) {
            HDCP_DEBUG_PRINTF(("Auth Fail: DDC Bus command error !\r\n"));
        }
    }
}

void iTE6615_HDCP_Show_RI( void )
{
    u8 tmp[4];
    // Read ARi/BRi
    hdmitxbrd(0x58, 2, tmp);
    hdmitxbrd(0x60, 2, &tmp[2]);
    HDCP_DEBUG_PRINTF(("ARi = 0x%02X%02X\r\n", (int)tmp[0],(int)tmp[1]));
    HDCP_DEBUG_PRINTF(("BRi = 0x%02X%02X\r\n\r\n", (int)tmp[2],(int)tmp[3]));
}

void iTE6615_HDCP_Show_PJ( void )
{
    u8 APj, BPj;

    APj = hdmitxrd(0x5A);
    BPj = hdmitxrd(0x62);

    HDCP_DEBUG_PRINTF(("APj = 0x%02X\r\n", (int)APj));
    HDCP_DEBUG_PRINTF(("BPj = 0x%02X\r\n\r\n", (int)BPj));
}

u8 iTE6615_HDCP_Check_Revocation(u8 *RxID)
{
    u16 i;

    for (i = 0; i < sizeof(iTE6615_HDCP_Check_Revocation_List) / sizeof(iTE6615_HDCP_Check_Revocation_List[0]); i++)
    {
        if( RxID[0] == iTE6615_HDCP_Check_Revocation_List[i][0] &&
            RxID[1] == iTE6615_HDCP_Check_Revocation_List[i][1] &&
            RxID[2] == iTE6615_HDCP_Check_Revocation_List[i][2] &&
            RxID[3] == iTE6615_HDCP_Check_Revocation_List[i][3] &&
            RxID[4] == iTE6615_HDCP_Check_Revocation_List[i][4]
        )
        {
            return TRUE;
        }
        // prevent call lib
        //if (0 == memcmp(RxID, &iTE6615_HDCP_Check_Revocation_List[i][0], 5))
        //{
        //    return TRUE;
        //}
    }

    return FALSE;
}

void iTE6615_HDCP2Repeater_StreamManage(void)
{
    hdmitxwr(0xFF, 0xC3);
    hdmitxwr(0xFF, 0xA5);
    hdmitxset(0x6B, 0x80, 0x80);// RRepWaitTxMgmDone
    hdmitxwr(0xFF, 0xFF);
}

void iTE6615_HDCP2Repeater_ContinueSKESendEKS(void)
{
    hdmitxwr(0xFF, 0xC3);
    hdmitxwr(0xFF, 0xA5);
    hdmitxset(0x6B, 0x40, 0x40);// RRepWaitTxEksDone
    hdmitxwr(0xFF, 0xFF);
}


#if _HDCP_Enable_RepeaterMode_

#define rol(x,y) (((x) << (y)) | (((unsigned long)x) >> (32-y)))
void iTE6615_HDCPRepeater_SHATransfrom( u32 *h, u32 *w )
{
    u32 t;
    u32 tmp;

    for (t = 16; t < 80; t++) {
         tmp = w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16];
         w[t] = rol(tmp,1);
    }

    h[0] = 0x67452301 ;
    h[1] = 0xefcdab89;
    h[2] = 0x98badcfe;
    h[3] = 0x10325476;
    h[4] = 0xc3d2e1f0;

    for (t = 0; t < 20; t++) {
         tmp = rol(h[0],5) + ((h[1] & h[2]) | (h[3] & ~h[1])) + h[4] + w[t] + 0x5a827999;

         h[4] = h[3];
         h[3] = h[2];
         h[2] = rol(h[1],30);
         h[1] = h[0];
         h[0] = tmp;

    }
    for (t = 20; t < 40; t++) {
         tmp = rol(h[0],5) + (h[1] ^ h[2] ^ h[3]) + h[4] + w[t] + 0x6ed9eba1;
         h[4] = h[3];
         h[3] = h[2];
         h[2] = rol(h[1],30);
         h[1] = h[0];
         h[0] = tmp;
    }
    for (t = 40; t < 60; t++) {
         tmp = rol(h[0],5) + ((h[1] & h[2]) | (h[1] & h[3]) | (h[2] & h[3])) + h[4] + w[t] + 0x8f1bbcdc;
         h[4] = h[3];
         h[3] = h[2];
         h[2] = rol(h[1],30);
         h[1] = h[0];
         h[0] = tmp;
    }
    for (t = 60; t < 80; t++) {
         tmp = rol(h[0],5) + (h[1] ^ h[2] ^ h[3]) + h[4] + w[t] + 0xca62c1d6;
         h[4] = h[3];
         h[3] = h[2];
         h[2] = rol(h[1],30);
         h[1] = h[0];
         h[0] = tmp;
    }

    h[0] += 0x67452301 ;
    h[1] += 0xefcdab89;
    h[2] += 0x98badcfe;
    h[3] += 0x10325476;
    h[4] += 0xc3d2e1f0;
}

void iTE6615_HDCPRepeater_SHASimple( u8 *p, u32 msgcnt , u8 *output)
{
    u32 i, t ;
    u32 c ;
    unsigned char *pBuff = p ;
    //u32 w[20];
    // 64 + 20*4 = 144;
    u32 *w = (u32*) ( iTE6615_DATA.data_buffer +64 );  // +64 because m[64]
    u32 sha[5];

    for( i = 0 ; i < msgcnt ; i++ ){
        t = i/4 ;
        if( i%4 == 0 ){
            w[t] = 0 ;
        }
        c = pBuff[i] ;
        c &= 0xFF ;
        c <<= (3-(i%4))*8 ;
        w[t] |= c ;
    }
    t = i/4 ;
    if( i%4 == 0 ){
        w[t] = 0 ;
    }
    c = 0x80 << ((3-i%4)*8) ;
    w[t]|= c ; t++ ;
    for( ; t < 15 ; t++ ){
        w[t] = 0 ;
    }
    w[15] = msgcnt*8  ;

    iTE6615_HDCPRepeater_SHATransfrom(sha,w);

    for(i=0; i<5; i++)
    {
        output[i*4]   = (unsigned char) (sha[i]&0xFF) ;
        output[i*4+1] = (unsigned char)((sha[i]>>8)&0xFF) ;
        output[i*4+2] = (unsigned char)((sha[i]>>16)&0xFF) ;
        output[i*4+3] = (unsigned char)((sha[i]>>24)&0xFF) ;
    }
}

u8 iTE6615_HDCPRepeater_CheckSupport( void )
{
    u8 i;
    u32 ksvcnt = 0, msgcnt;

    u8 bstatus[2];
    u8 av[5][4],bv[5][4],ksvlist[32];
    //u8 m[64];
    u8 *m = iTE6615_DATA.data_buffer;
    u8 m0[8];

    hdmitxset(0x70, 0x07, 0x05);
    for (i = 0; i < 4; i++)
        m0[i] = hdmitxrd(0x71 + i);

    for (i = 0; i < 4; i++) {
        hdmitxset(0x70, 0x07, i);
        m0[4 + i] = hdmitxrd(0x75);
    }

    HDCP_DEBUG_PRINTF(("iTE: M0: 0x "));

    for(i=0; i<8; i++)
        HDCP_DEBUG_PRINTF(("%02X ", m0[7-i]));


    iTE6615_HDCP_Read(0x41, 2);     // BStatus
    bstatus[0] = hdmitxrd(0x64);
    bstatus[1] = hdmitxrd(0x65);

    HDCP_DEBUG_PRINTF(("iTE: Device Count = %X\r\n", bstatus[0]&0x7F));
    HDCP_DEBUG_PRINTF(("iTE: Max. Device Exceeded = %02X\r\n", (bstatus[0]&0x80)>>7));


    HDCP_DEBUG_PRINTF(("iTE: Depth = %X\r\n", bstatus[1]&0x07));
    HDCP_DEBUG_PRINTF(("iTE: Max. Cascade Exceeded = %02X\r\n", (bstatus[1]&0x08)>>3));
    HDCP_DEBUG_PRINTF(("iTE: HDMI_MODE = %d\r\n", (bstatus[1]&0x10)>>4));

    if( (bstatus[0]&0x80) || (bstatus[1]&0x08) ) {
        HDCP_DEBUG_PRINTF(("iTE: ERROR: Max. Device or Cascade Exceeded !!!\r\n"));
        return FALSE;
    }
    else {

        // read ksv list from ddc fifo
        ksvcnt = 5*(bstatus[0]&0x7F);

        //read back HDCP offset KSV LIST (0x43) back to internal register
        //

        if( ksvcnt ){
            iTE6615_HDCP_Read(0x43, ksvcnt);     // KSV LIST FIFO
            hdmitxbrd(0x30, ksvcnt, &ksvlist[0]);
        }
        else{
            HDCP_DEBUG_PRINTF(("iTE: WARNING: Device Count = 0 !!!\r\n"));
        }

        msgcnt = 0;

        for(i=0; i<(bstatus[0]&0x7F); i++) {
            m[msgcnt++] = (unsigned char) ksvlist[i*5+0];
            m[msgcnt++] = (unsigned char) ksvlist[i*5+1];
            m[msgcnt++] = (unsigned char) ksvlist[i*5+2];
            m[msgcnt++] = (unsigned char) ksvlist[i*5+3];
            m[msgcnt++] = (unsigned char) ksvlist[i*5+4];

             HDCP_DEBUG_PRINTF(("iTE: KSV List %d = 0x %02X %02X %02X %02X %02X\r\n", i, (int)m[i*5+4], (int)m[i*5+3], (int)m[i*5+2], (int)m[i*5+1], (int)m[i*5+0]));
        }

        HDCP_DEBUG_PRINTF(("\r\n"));

        m[msgcnt++] = bstatus[0];
        m[msgcnt++] = bstatus[1];

        m[msgcnt++] = m0[0];
        m[msgcnt++] = m0[1];
        m[msgcnt++] = m0[2];
        m[msgcnt++] = m0[3];
        m[msgcnt++] = m0[4];
        m[msgcnt++] = m0[5];
        m[msgcnt++] = m0[6];
        m[msgcnt++] = m0[7];

        HDCP_DEBUG_PRINTF(("SHA Message Count = %d\r\n\r\n", msgcnt));

        m[msgcnt] = 0x80;

        for(i=msgcnt+1; i<62 ;i++)
            m[i] = 0x00;

        m[62] = ((8*msgcnt) >> 8)&0xFF ;
        m[63] = (8*msgcnt) &0xFF ;

        iTE6615_HDCPRepeater_SHASimple(&m[0], msgcnt, &av[0][0]) ;

        for(i=0; i<5; i++){HDCP_DEBUG_PRINTF(("iTE: AV.H%d = 0x %02X %02X %02X %02X\r\n", i, (int)av[i][3], (int)av[i][2], (int)av[i][1], (int)av[i][0]));}

        HDCP_DEBUG_PRINTF(("\r\n"));


        for(i=0; i<5; i++) {

            iTE6615_HDCP_Read(0x20 + i * 4, 4);   // V'Hi
            hdmitxset(0x70, 0x07, i);
            bv[i][0] = (u8)hdmitxrd(0x71);
            bv[i][1] = (u8)hdmitxrd(0x72);
            bv[i][2] = (u8)hdmitxrd(0x73);
            bv[i][3] = (u8)hdmitxrd(0x74);

            HDCP_DEBUG_PRINTF(("BV.H%d = 0x %02X %02X %02X %02X\r\n", i, (int)bv[i][3], (int)bv[i][2], (int)bv[i][1], (int)bv[i][0]));

            if( av[i][0]!=bv[i][0] || av[i][1]!=bv[i][1] || av[i][2]!=bv[i][2] || av[i][3]!=bv[i][3])
            return FALSE;
        }
    }

    return TRUE;
}
#else

u8 iTE6615_HDCPRepeater_CheckSupport( void )
{
    HDMITX_DEBUG_PRINTF(("### HDCP Repeater supporting is disabled\r\n"));
    return FALSE;
}

#endif


// other
void iTE6615_print_buffer( void *buffer, int length )
{
    u8 *buf = (u8 *)buffer;
    int pr_len = 8;

    while (length) {

        if (length < pr_len) {
            pr_len = length;
        }
        else {
            pr_len = 8;
        }

        length -= pr_len;

        while( pr_len ) {
            PRINTF_BUFFER(( "%02X ", (int)(*buf) ));
            pr_len--;
            buf++;
        }
        PRINTF_BUFFER(( "\r\n" ));
    }
}


void    chgtxbank( u8 bank )
{
    hdmitxset(0x0F, 0x03, bank & 3);
}



#if iTE6264
void iTE6264_Get_SSCPCLK( void )
{
    u32 rddata, sscpclkl_sum, sscpclkh_sum ;
    u8  i, predivsel;

    // PCLK Count Pre-Test
    lvrxset(0x0E, 0x80, 0x80);
    delay1ms(1);
    lvrxset(0x0E, 0x80, 0x00);

    rddata = lvrxrd(0x0B);
    rddata += ((lvrxrd(0x0C)&0x0F)<<8);



    if( rddata<16 )
        predivsel = 7;
    else if( rddata<32 )
        predivsel = 6;
    else if( rddata<64 )
        predivsel = 5;
    else if( rddata<128 )
        predivsel = 4;
    else if( rddata<256 )
        predivsel = 3;
    else if( rddata<512 )
        predivsel = 2;
    else if( rddata<1024 )
        predivsel = 1;
    else
        predivsel = 0;

    printf("predivsel=%d\r\n", predivsel);

    sscpclkl_sum = 0;
    sscpclkh_sum = 0;
    for(i=0; i<10; i++)
    {
        lvrxset(0x0E, 0xF0, (0x80+(predivsel<<4)));
        delay1ms(1);
        lvrxset(0x0E, 0xF0, (predivsel<<4));

        rddata = lvrxrd(0x0B);
        rddata += ((lvrxrd(0x0C)&0x0F)<<8);

        sscpclkl_sum += rddata;

        rddata = lvrxrd(0x0D);
        rddata += ((lvrxrd(0x0E)&0x0F)<<8);

        sscpclkh_sum += rddata;
    }

    sscpclkl_sum /= (5* (1 << predivsel));
    sscpclkh_sum /= (5* (1 << predivsel));

    if(sscpclkl_sum)iTE6615_DATA.SSCPCLKL = iTE6615_DATA.RCLK*2048/sscpclkl_sum;
    if(sscpclkh_sum)iTE6615_DATA.SSCPCLKH = iTE6615_DATA.RCLK*2048/sscpclkh_sum;

    HDMITX_DEBUG_PRINTF(("\nCount SSCPCLKL=%d MHz, SSCPCLKH=%3.3fMHz\n", (int) (iTE6615_DATA.SSCPCLKL/1000),(int)  (iTE6615_DATA.SSCPCLKH/1000)));
}
#endif
