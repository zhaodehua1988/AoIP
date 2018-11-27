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
//#include "iTE6805_Global.h"
#include "iTE6805_Init.h"
#include "iTE6805_DRV.h"
//#include "version.h"
#include "iTE6805_DEV_DEFINE.h"
#include "iTE6805_SYS.h"
#include "PCA9548A.h"
#include "wv_common.h"
_iTE6805_DATA iTE6805_DATA;
_iTE6805_VTiming iTE6805_CurVTiming;
_iTE6805_PARSE3D_STR iTE6805_EDID_Parse3D;

void ITE6805_Init(void)
{
	// iTE_u16 INT_TimeOutCnt = 0;
	//P2_3 = 0;	// for DAC enable !!!
	//initialTimer1();
	//InitMessage();
	//HoldSystem();
	WV_printf("it68051 init start \n");
	PCA9548_SwitchToBus(PCA9548A_IIC_SWID_68051_HDMI_IN);
	iTE6805_Init_fsm();

	//iTE6805_HDCP_Detect();
#ifdef EVB_AUTO_DETECT_PORT_BY_PIN
//iTE6805_Port_Detect();
#endif
//iTE6805_Port_Detect();
	if (iTE6805_DATA.STATEV != STATEV_Unplug && iTE6805_DATA.STATEV != STATEV_VideoOff)
	{
		if (iTE6805_DATA.CurrentPort == PORT0)
		{
			printf("----------------port 0 -----------------------\n");
			iTE6805_hdmirx_port0_SYS_irq();
#define _ENABLE_AUTO_EQ_

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
	//INT_TimeOutCnt = 0;

	//   HoldSystem();
	//}
	
	WV_printf("it68051 init end \n");
}

void InitMessage()
{
#if Debug_message
	init_printf();
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	printf("                   %s\n", VERSION_STRING);
	printf("                   %s\n", DATE_STRING);
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
#endif
}

/*
void HoldSystem(void)
{
    Hold_Pin=1;
    while(!Hold_Pin)
    {
    #if Debug_message
            printf("Hold\\\r");
            printf("Hold-\r");
            printf("Hold/\r");
            printf("Hold|\r");
    #endif
    }
}
*/