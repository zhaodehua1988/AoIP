#ifndef _WV_HIS_IIC_H__
#define _WV_HIS_IIC_H__
#include "wv_common.h"
WV_S32  HIS_IIC_Init();
WV_S32  HIS_IIC_DeInit();
WV_S32  HIS_IIC_Write(WV_U32 port,WV_U8 devAddr,WV_U8 regAddr,WV_U8  data);
WV_S32  HIS_IIC_Read(WV_U32 port,WV_U8 devAddr,WV_U8 regAddr,WV_U8 * pData);

#endif
