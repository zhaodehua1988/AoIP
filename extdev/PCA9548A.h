/************************************************* 
Copyright:亿视芯
Author: zhaodehua
Date:2018-11-22 
Description: 
**************************************************/ 
#ifndef __PCA9548A_H_H_
#define __PCA9548A_H_H_

#include "wv_common.h"

#define PCA9548A_IIC_SWID_7619      0x40
#define PCA9548A_IIC_SWID_68051_HDMI_IN        0x20
#define PCA9548A_IIC_SWID_6615_HDMI_OUT       0x10
#define PCA9548A_IIC_SWID_9555      0x80 
#define PCA9548A_IIC_SWID_1297      0x80

//WV_S32 PCA9548_SwitchToBus(WV_U8 id);
WV_S32 PCA9548_IIC_Read(WV_U8 id,WV_U8 devAddr,WV_U8 regAddr,WV_U8 *data);
WV_S32 PCA9548_IIC_Write(WV_U8 id,WV_U8 devAddr,WV_U8 regAddr,WV_U8 data);
WV_S32 PCA9548_Init();
WV_S32 PCA9548_DeInit();

#endif