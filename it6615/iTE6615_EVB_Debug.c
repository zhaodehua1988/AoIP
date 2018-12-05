///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_EVB_Debug.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/
#include "iTE6615_Global.h"

#if _MCU_8051_EVB_

extern iTE6615_GlobalDATA iTE6615_DATA;

void iTE6615_EVB_Ca100ms_RCLK( void )
{
    u32 rddata, i;
    u32 sum;

    hdmitxwr(0xFC, (ADDR6615_CEC | 0x01));    // Enable CRCLK

    sum = 0;
    for (i = 0; i < 5; i++) {

        cec6615wr(0x09, 0x01);
        delay1ms(99);
        cec6615wr(0x09, 0x00);

        rddata = cec6615rd(0x45);
        rddata += (cec6615rd(0x46) << 8);
        rddata += (cec6615rd(0x47) << 16);

        sum += rddata;
    }
    sum /= 5;

    //cec6615set(0x0d, 0x10, 0x00);      // Disable CEC_IOPU

    #if _ENABLE_CEC_==FALSE
    hdmitxwr(0xFC, (ADDR6615_CEC & 0xFE));
    cec6615wr(0x0C, sum/1000);
    #endif

    iTE6615_DATA.RCLK = (sum * 16) / 100;

    HDMITX_DEBUG_PRINTF(("cal RCLK=%u\r\n", iTE6615_DATA.RCLK));

    if ( !iTE6615_Check_RCLK_Valid(iTE6615_DATA.RCLK)  ) {
        iTE6615_DATA.RCLK = 0;
    }
}

#endif// _MCU_8051_EVB_
