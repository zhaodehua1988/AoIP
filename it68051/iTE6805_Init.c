///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <Main.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
#include "iTE6805_Global.h"
//#include "Main.h"
//#include "version.h"

#include "PCA9555.h"
#include "PCA9548A.h"
#include "wv_thr.h"
#include "wv_common.h"
#include "iTE6805_DRV.h"

_iTE6805_DATA			iTE6805_DATA;
_iTE6805_VTiming		iTE6805_CurVTiming;
_iTE6805_PARSE3D_STR	iTE6805_EDID_Parse3D;

//////////////////////////////////////////////////
typedef struct ITE68051_HDMI_INFO
{
	WV_U8 inputType; //0=MHL ;1=HDMI
	WV_U16 totalPixel;
	WV_U16 activePixel;
	WV_U16 totalLine;
	WV_U16 activeLine;
	WV_U16 pixMode; //0:sigle pix mode ;1:dual pix mode (单双像素)

} ITE68051_HDMI_INFO;

typedef struct ITE68051_DEV{
	WV_THR_HNDL_T thrHndl;
	WV_U32      open;
	WV_U32      close;

}ITE68051_DEV;

ITE68051_DEV gIte68051Dev;


void *ITE6805_Proc(void *prm){ 


	//iTE6805_Init_fsm();
	ITE68051_DEV *pDev = (ITE68051_DEV *)prm;
	pDev->open = 1;
	pDev->close = 0;	
	//EVB_CONTROL_4K_DOWNSCALE = 1; // mid
     while( pDev->open = 1 ) {

	//	if( INT_TimeOutCnt > 50 ) {
			#ifdef DYNAMIC_HDCP_ENABLE_DISABLE
			iTE6805_HDCP_Detect();
			#endif

			#ifdef EVB_AUTO_DETECT_PORT_BY_PIN
			iTE6805_Port_Detect();
			#endif

			if( iTE6805_DATA.STATEV != STATEV_Unplug && iTE6805_DATA.STATEV != STATEV_VideoOff )
			{
				if(iTE6805_DATA.CurrentPort == PORT0)
				{
					iTE6805_hdmirx_port0_SYS_irq();
					#ifdef _ENABLE_AUTO_EQ_
					iTE6805_hdmirx_port0_EQ_irq();
					#endif

					#ifdef _ENABLE_IT6805_MHL_FUNCTION_
					iTE6805_mhlrx_irq();
					#endif
				}
				else
				{
					iTE6805_hdmirx_port1_SYS_irq();
					#ifdef _ENABLE_AUTO_EQ_
					iTE6805_hdmirx_port1_EQ_irq();
					#endif
				}
				iTE6805_hdmirx_common_irq();
			}

			#ifdef _ENABLE_IT6805_CEC_
			iTE6805_hdmirx_CEC_irq();
			#endif

			#ifdef _ENABLE_AUTO_EQ_
			iTE6805_EQ_fsm();
			#endif

		    iTE6805_vid_fsm();
		    iTE6805_aud_fsm();
           // INT_TimeOutCnt = 0;
		}

	

	gIte68051Dev.close = 1;
}


/****************************************************************************

WV_S32 iTE6805_GetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 iTE6805_GetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
	WV_U32 regAddr, dataNum;
	WV_S32 ret=0, i;
	WV_U8 data;
	ITE68051_HDMI_INFO info;
	if (argc < 1)
	{

		prfBuff += sprintf(prfBuff, "get 6805 <cmd> ;cmd like:reg/info\r\n");
		return 0;
	}

	if (strcmp(argv[0], "reg") == 0)
	{

		if (argc < 3)
		{
			prfBuff += sprintf(prfBuff, "get 6805 reg <regAddr> <dataNum>\r\n");
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
		PCA9548_IIC_Read(PCA9548A_IIC_SWID_68051_HDMI_IN, ADDR_HDMI, 0xf, &data);
		data = data | bank;
		ret = PCA9548_IIC_Write(PCA9548A_IIC_SWID_68051_HDMI_IN, ADDR_HDMI, 0xf, data);
		if (ret != 0)
			return WV_SOK;
		for (i = 0; i < dataNum; i++)
		{
			PCA9548_IIC_Read(PCA9548A_IIC_SWID_68051_HDMI_IN, ADDR_HDMI, regAddr + i, &data);
			prfBuff += sprintf(prfBuff, "get 0x%X = 0x%X\r\n", regAddr + i, data);
		}
	}
	else if (strcmp(argv[0], "info") == 0)
	{
		//change to bank 0
		ret += PCA9548_IIC_Write(PCA9548A_IIC_SWID_68051_HDMI_IN, ADDR_HDMI, 0xf, 0);
		WV_U16 temp1, temp;
		ret += PCA9548_IIC_Read(PCA9548A_IIC_SWID_68051_HDMI_IN, ADDR_HDMI, 0x13, &data);
		if (data & 0x40)
		{
			info.inputType = 1;
		}
		else
		{
			info.inputType = 0;
		}
		//get totalPix
		ret += PCA9548_IIC_Read(PCA9548A_IIC_SWID_68051_HDMI_IN, ADDR_HDMI, 0x9B, &data);
		info.totalPixel = data;
		ret += PCA9548_IIC_Read(PCA9548A_IIC_SWID_68051_HDMI_IN, ADDR_HDMI, 0x9C, &data);
		info.totalPixel = info.totalPixel | (data & 0x3f) << 8;
		//get activePix
		ret += PCA9548_IIC_Read(PCA9548A_IIC_SWID_68051_HDMI_IN, ADDR_HDMI, 0x9D, &data);
		info.activePixel = data;
		ret += PCA9548_IIC_Read(PCA9548A_IIC_SWID_68051_HDMI_IN, ADDR_HDMI, 0x9E, &data);
		info.activePixel = info.activePixel | (data & 0x3f) << 8;
		//get totalLine
		ret += PCA9548_IIC_Read(PCA9548A_IIC_SWID_68051_HDMI_IN, ADDR_HDMI, 0xA2, &data);
		info.totalLine = data;
		ret += PCA9548_IIC_Read(PCA9548A_IIC_SWID_68051_HDMI_IN, ADDR_HDMI, 0xA3, &data);
		info.totalLine = info.totalLine | (data & 0x3f) << 8;
		//get activeLine
		ret += PCA9548_IIC_Read(PCA9548A_IIC_SWID_68051_HDMI_IN, ADDR_HDMI, 0xA4, &data);
		info.activeLine = data;
		ret += PCA9548_IIC_Read(PCA9548A_IIC_SWID_68051_HDMI_IN, ADDR_HDMI, 0xA5, &data);
		info.activeLine = info.activeLine | (data & 0x3f) << 8;

		//get pix mode 
		ret += PCA9548_IIC_Write(PCA9548A_IIC_SWID_68051_HDMI_IN, ADDR_HDMI, 0xf, 1);
		ret += PCA9548_IIC_Read(PCA9548A_IIC_SWID_68051_HDMI_IN, ADDR_HDMI, 0xC0, &data);
		info.pixMode = data & 0x1;
		prfBuff += sprintf(prfBuff, "------------------input  info---------------------------\r\n");
		if (info.inputType = 0)
		{
			prfBuff += sprintf(prfBuff, "input mode = MHL\r\n");
		}
		else
		{
			prfBuff += sprintf(prfBuff, "input mode = HDMI\r\n");
		}
		prfBuff += sprintf(prfBuff, "input totalPix = %d\r\n",info.totalPixel);
		prfBuff += sprintf(prfBuff, "input activePix = %d\r\n",info.activePixel);
		prfBuff += sprintf(prfBuff, "input totalLine = %d\r\n",info.totalLine);
		prfBuff += sprintf(prfBuff, "input totalLine = %d\r\n",info.activeLine);
		//show input info
		
		prfBuff += sprintf(prfBuff, "-------------------------------------------------------\r\n");
		prfBuff += sprintf(prfBuff, "------------------output  info---------------------------\r\n");

		if (info.pixMode = 0)
		{
			prfBuff += sprintf(prfBuff, "output pix mode = sigle pix mode\r\n");
		}
		else
		{
			prfBuff += sprintf(prfBuff, "output pix mode = dual pix mode\r\n");
		}	

		sleep(1);
		iTE6805_Show_AVIInfoFrame_Info();
		
		iTE6805_Show_VID_Info();
		printf("11111111111111111111111111111");
	}

	return WV_SOK;
}

/****************************************************************************

WV_S32 iTE6805_SetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 iTE6805_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
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
		PCA9548_IIC_Write(PCA9548A_IIC_SWID_68051_HDMI_IN, ADDR_HDMI, (WV_U8)regAddr, (WV_U8)data);
		prfBuff += sprintf(prfBuff, "set 0x%X = 0x%X\r\n", regAddr, data);
	}
	else if (strcmp(argv[0], "mode") == 0)
	{ //设置输出模式单双沿

		if (argc < 2)
		{

			prfBuff += sprintf(prfBuff, "set 6805 mode <1 0r 2>//1:sigle pixel mode ;2 dual pixel mode \r\n");
			return 0;
		}

		ret = WV_STR_S2v(argv[1], &data);
		if (ret != WV_SOK)
		{
			prfBuff += sprintf(prfBuff, "input erro!\r\n");
			return WV_SOK;
		}

		if (data != 1 && data != 2)
		{
			prfBuff += sprintf(prfBuff, "input mode error\r\n");
			return WV_SOK;
		}
		iTE6805_Set_LVDS_Video_Path((WV_U8)data);
		prfBuff += sprintf(prfBuff, "set 6805 hdmi mode = %d \r\n", data);
	}
	else
	{
		prfBuff += sprintf(prfBuff, "input erro!\r\n");
	}

	return WV_SOK;
}

/******************************************************
 * void ITE6805_Open(void)
 ******************************************************/
void ITE6805_Open(void)
{	WV_printf("it68051 init start \n");

	PCA9555_Clr(PCA9555_OUT_PORT0_REG,PCA9555_PIN_P00);
	sleep(1);
	PCA9555_Set(PCA9555_OUT_PORT0_REG,PCA9555_PIN_P00);
	sleep(1);
	PCA9548_SwitchToBus(PCA9548A_IIC_SWID_68051_HDMI_IN);
	iTE6805_Init_fsm();

    WV_CMD_Register("set", "6805", "pca9555 set reg", iTE6805_SetCmd);
    WV_CMD_Register("get", "6805", "pca9555 get reg", iTE6805_GetCmd); 

	WV_THR_Create(&gIte68051Dev.thrHndl,ITE6805_Proc, WV_THR_PRI_DEFAULT, WV_THR_STACK_SIZE_DEFAULT, (void *)&gIte68051Dev);
	WV_printf("it68051 init end \n");
	

}
/******************************************************
 * void ITE6805_Close(void)
 ******************************************************/
void ITE6805_Close(void)
{
	if(gIte68051Dev.open == 1){
		gIte68051Dev.open = 0;
		while(gIte68051Dev.close == 1);
		WV_THR_Destroy(&gIte68051Dev.thrHndl);
	}
	printf("ite68051 deinit ok..");

}
