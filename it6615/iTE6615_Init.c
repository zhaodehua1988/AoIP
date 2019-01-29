///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_Main.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/
//#include "iTE6615_Global.h"
#include "PCA9548A.h"
#include "PCA9555.h"
#include "iTE6615_Global.h"
#include "iTE6615_Init.h"
//#include "config.h"
#include "wv_common.h"
#include "wv_thr.h"
extern iTE6615_GlobalDATA iTE6615_DATA;

typedef struct ITE6615_DEV
{
    WV_THR_HNDL_T thrHndl;
    WV_U32 open;
    WV_U32 close;

} ITE6615_DEV;

ITE6615_DEV gITE6615Dev;

void *iTE6615_Proc(void *prm)
{

    ITE6615_DEV *pDev = (ITE6615_DEV *)prm;
    pDev->open = 1;
    pDev->close = 0;
    //iTE6615_fsm();

    while (pDev->open == 1)
    {

        usleep(500000);
        // never changed config, auto run.
        iTE6615_fsm();
    // Below Code is example/reference for Customer when you need to change config at run-time
        /*************************************************************************************************

    // This example code of combine 6805(HDMI2.x/HDMI1.x input, output TTL)+6615(TTL input, output HDMI2.x/HDMI1.x)

    // HDMI2.0->6805->TTL->6615->HDMI TX

    // If you want, for efficient , you could run 6615 fsm only if you know the before chip of 6615 TTL is already output.
    // For two case, 1: 6805 video unstable->stable let 6615 knows need to start run fsm, or not. (using iTE6805_DATA.Flag_VidStable_Done)
    //               2: 6805 may video stable but AVI Infoframe change, need to reset 6615 for new condition (may be color space changed but video still stable) (using iTE6805_DATA.Flag_Reset6615)
    if(iTE6805_DATA.Flag_VidStable_Done == TRUE || iTE6805_DATA.Flag_Reset6615)
    {
        iTE6805_DATA.Flag_Reset6615 = FALSE;

        if(Flag_Vid_Change == 0)
        {
            Flag_Vid_Change = 1;
            iTE6805_CurVTiming.ColorDepth = (hdmirxrd(0x98) & 0xF0);

            // setting 6615 color depth by 6805 color depth
            switch (iTE6805_CurVTiming.ColorDepth)
            {
                case 0x00:
                case 0x40:
                    printf("\n set to 8 b\n");
                    iTE6615_Customer_Set_InputColorDepth(8);
                    break;
                case 0x50:
                    printf("\n set to 10 b\n");
                    iTE6615_Customer_Set_InputColorDepth(10);
                    break;
                case 0x60:
                    printf("\n set to 12 b\n");
                    iTE6615_Customer_Set_InputColorDepth(12);
                    break;
                default:
                    printf("8 b\n");
                    iTE6615_Customer_Set_InputColorDepth(8);
                    break;
            }

            // config Pixel Repeating, you need config pixel repeating for VIC below if your device support
            // 10, 11 ,12, 13, 14, 15, 25, 26 ,27, 28, 29, 30, 35, 36 ,37, 38,
            // setting by chip before 6805 because of those VIC pixel repeating sent 1~10 times(knows from chip before 6615)
            iTE6615_DATA.config.PixRpt = iTE6805_DATA.pixel_repeat;

#if iTE6264
            iTE6615_DATA.config.TTL_Enable_DDR = 0;    // If chip 6264, need to setting to SDR, or video will not stable.
#endif

            // config audio
            iTE6615_DATA.config.Audio_Frequency = iTE6805_DATA.Force_Sampling_Frequency;

            // config TTL single/dual pixel mode
            if(iTE6805_DATA.Flag_Pixel_Mode){iTE6615_Customer_Set_InputDualMode(iTE6805_DATA.Flag_Pixel_Mode, 1);}
            else{iTE6615_Customer_Set_InputDualMode(iTE6805_DATA.Flag_Pixel_Mode, 0);}

            // config AVI Infoframe VIC, auto detect pixel repeating when pixel repeating fixed
            iTE6615_CurVTiming.VIC = iTE6805_DATA.AVIInfoFrame_VIC;

            // Audio Channel
            iTE6615_DATA.config.Audio_ChannelCount = iTE6805_DATA.Audio_Channel_Count;

            // config when 6805 output YUV420
            if(iTE6805_DATA.Flag_IS_YUV420)
            {
                // set 68051 to dual output start
                chgbank(0);
                hdmirxset(0x64, BIT2, 0x00);
                chgbank(1);
                hdmirxset(0xC0, BIT0, BIT0);
                #if iTE6615
                hdmirxset(0xBD, BIT4|BIT5, 0x10);    // setting 68051 dual link
                #else
                hdmirxset(0xBD, BIT4|BIT5, 0x30);    // setting 68052 dual link
                #endif
                chgbank(0);
                // set 68051 to dual output end

                iTE6615_Customer_Set_InputDualMode(1, 0);

                // setting 6805 tristate for dual data output
                iTE6805_Set_Video_Tristate(TRISTATE_OFF);

                // 6615 output YUV420 only if input is YUV420
                iTE6615_DATA.config.Input_ColorSpace = YUV420;
                iTE6615_DATA.config.Output_ColorSpace = YUV420;
                iTE6615_DATA.config.Input_YCC_Colorimetry = ITU709;
                iTE6615_DATA.config.Output_YCC_Colorimetry = ITU709;
            }
            else
            {
                iTE6615_DATA.config.Input_ColorSpace = RGB444;
                iTE6615_DATA.config.Output_ColorSpace = YUV444;
                iTE6615_DATA.config.Output_YCC_Colorimetry = ITU709;
            }

            // If you need change config run-time, you must call this function for reconfig everything!
            iTE6615_sys_chg(iTE6615_STATES_Unplug);
        }

        // polling 6805 depthcolor if ColorDepth changed
        chgbank(0);
        polling_6805_depth_color = (hdmirxrd(0x98) & 0xF0);
        if(iTE6805_CurVTiming.ColorDepth != polling_6805_depth_color)
        {
            iTE6805_CurVTiming.ColorDepth = polling_6805_depth_color;
            Flag_Vid_Change = 0;
        }

        // polling 6805 Audio Channel count if changed but 6615 couldn't get 6805 audio status on time.
        if(iTE6805_DATA.Audio_Channel_Count != iTE6615_DATA.config.Audio_ChannelCount )
        {
            iTE6615_DATA.config.Audio_ChannelCount = iTE6805_DATA.Audio_Channel_Count;
            iTE6615_aud_chg(iTE6615_STATEA_Reset);
            iTE6615_aud_chg(iTE6615_STATEA_WaitForAudIn);
        }

        // maybe VIC change but 6615 TTL-in still vidstable, add for detect
        if(iTE6615_CurVTiming.VIC != iTE6805_DATA.AVIInfoFrame_VIC)
        {
            Flag_Vid_Change = 0;
        }

        iTE6615_fsm();
    }
    else
    {
        // If 6615 RX not ready (knows from chip before 6615. i.e. 6805 output not ready), only need to polling iTE6615_sys_fsm();
        // polling iTE6615_sys_fsm(); is for HPD on/RX Sense detect, read EDID and power on.
        if(Flag_Vid_Change == 1 && iTE6805_DATA.Flag_VidStable_Done == 0)    // when 6805 stable -> 6805 unstable
        {
            iTE6615_sys_chg(iTE6615_STATES_Unplug);
            Flag_Vid_Change = 0;
        }
        iTE6615_sys_fsm();
    }
*************************************************************************************************/
    }
    return NULL;
}


/****************************************************************************

WV_S32 iTE6805_GetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 iTE6615_GetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
	WV_U32 regAddr, dataNum;
	WV_S32 ret=0, i;
	WV_U8 data;
	
	if (argc < 1)
	{

		prfBuff += sprintf(prfBuff, "get 6615 <cmd> ;cmd like:reg/info\r\n");
		return 0;
	}

	if (strcmp(argv[0], "reg") == 0)
	{

		if (argc < 3)
		{
			prfBuff += sprintf(prfBuff, "get 6615 reg <regAddr> <dataNum>\r\n");
		}
		ret = WV_STR_S2v(argv[1], &regAddr);
		if (ret != WV_SOK)
		{
			prfBuff += sprintf(prfBuff, "input erro!\r\n");
			return WV_SOK;
		}
		ret = WV_STR_S2v(argv[2], &dataNum);
		if (ret != WV_SOK)
		{
			prfBuff += sprintf(prfBuff, "input erro!\r\n");
			return WV_SOK;
		}
		//get bank num
		WV_U8 bank = (regAddr>>8) && 0x3;
		printf("bank = %d \n",bank);
		PCA9548_IIC_Read(PCA9548A_IIC_SWID_6615_HDMI_OUT, ADDR_HDMITX, 0xf, &data);
		data = data | bank;
		ret = PCA9548_IIC_Write(PCA9548A_IIC_SWID_6615_HDMI_OUT, ADDR_HDMITX, 0xf, data);
		if (ret != 0)
			return WV_SOK;
		for (i = 0; i < dataNum; i++)
		{
			PCA9548_IIC_Read(PCA9548A_IIC_SWID_6615_HDMI_OUT, ADDR_HDMITX, regAddr + i, &data);
			prfBuff += sprintf(prfBuff, "get 0x%X = 0x%X\r\n", regAddr + i, data);
		}
	}
	
	

	return WV_SOK;
}

/****************************************************************************

WV_S32 iTE6805_SetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 iTE6615_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
	WV_U32 cmd, regAddr, data;
	WV_S32 ret;
	if (argc < 1)
	{

		prfBuff += sprintf(prfBuff, "set 6805 <cmd>;//cmd like: reg/mode/\r\n");
		return 0;
	}
	//设置寄存器
	if (strcmp(argv[0], "reg") == 0)
	{
		if (argc < 3)
		{
			prfBuff += sprintf(prfBuff, "set 6805 reg <regAddr> <data>\r\n");
			return 0;
		}

		ret = WV_STR_S2v(argv[1], &regAddr);
		if (ret != WV_SOK)
		{
			prfBuff += sprintf(prfBuff, "input erro!\r\n");
			return WV_SOK;
		}
		ret = WV_STR_S2v(argv[2], &data);
		if (ret != WV_SOK)
		{
			prfBuff += sprintf(prfBuff, "input erro!\r\n");
			return WV_SOK;
		}
		PCA9548_IIC_Write(PCA9548A_IIC_SWID_6615_HDMI_OUT, ADDR_HDMITX, (WV_U8)regAddr, (WV_U8)data);
		prfBuff += sprintf(prfBuff, "set 0x%X = 0x%X\r\n", regAddr, data);
	}else
	{
		prfBuff += sprintf(prfBuff, "input erro!\r\n");
	}

	return WV_SOK;
}

/************************************************************
 * void ITE6615_Close()
 * *********************************************************/
void ITE6615_Open()
{

    printf("6615 init start 1.....\n");
    PCA9555_Clr(PCA9555_OUT_PORT0_REG, PCA9555_PIN_P02);
    sleep(1);
    PCA9555_Set(PCA9555_OUT_PORT0_REG, PCA9555_PIN_P02);
    sleep(1);
    iTE6615_INIT_Chip();

    WV_CMD_Register("set", "6615", "pca9555 set reg", iTE6615_SetCmd);
    WV_CMD_Register("get", "6615", "pca9555 get reg", iTE6615_GetCmd); 

    WV_THR_Create(&gITE6615Dev.thrHndl, iTE6615_Proc, WV_THR_PRI_DEFAULT, WV_THR_STACK_SIZE_DEFAULT, &gITE6615Dev);
    printf("ite6615 init end \n");
}
/************************************************************
 * void ITE6615_Close()
 * *********************************************************/
void ITE6615_Close()
{

    if (gITE6615Dev.open == 1)
    {
        gITE6615Dev.open = 0;
        while (gITE6615Dev.close == 1)
            ;
        WV_THR_Destroy(&gITE6615Dev.thrHndl);
    }
    printf("ite68051 deinit ok..");
}