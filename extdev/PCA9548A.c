/************************************************* 
Copyright:亿视芯
Author: zhaodehua
Date:2018-11-21 
Description:PCA9548A芯片控制 
**************************************************/
#include "PCA9548A.h"
#include "his_iic.h"

#define PCA9548A_DEBUG_MODE
#ifdef PCA9548A_DEBUG_MODE
#define PCA9548A_printf(...)                            \
    do                                                  \
    {                                                   \
        printf("\n\r[%s]-%d:", __FUNCTION__, __LINE__); \
        printf(__VA_ARGS__);                            \
        fflush(stdout);                                 \
    } while (0)
#else
#define PCA9548A_printf(...)
#endif
//Hi3798 使用的iic通道
#define HI3798_IIC_CHANEL 2
#define PCA9548A_ADDR 0xE0

static pthread_mutex_t mutex;
/************************************************
Function:       PCA9548_SwitchToBus(WV_U8 busID,WV_U8 rw)
Description:    切换9548A总线
Input:          busID:9548A总线ID； rw:读写控制 0x0代表写,0x1代表读
Output:         // 无 
Return:         // 0 成功 ；其他失败 
************************************************/
WV_S32 PCA9548_SwitchToBus(WV_U8 id)
{
    return HIS_IIC_Write(HI3798_IIC_CHANEL, PCA9548A_ADDR, 0, id);
}
/************************************************
Function:       PCA9548_I2C_Write(WV_U8 busID,WV_U16 data)
Description:    IIC写
Input:          busID:9548切换目标总线ID；devAddr:设备地址；regAddr:寄存器地址 data:写入数据（1个字节）
Output:         // 无 
Return:         // 0 成功 ；其他失败 
Other:          
************************************************/
WV_S32 PCA9548_IIC_Write(WV_U8 id, WV_U8 devAddr, WV_U8 regAddr, WV_U8 data)
{
    //unlock
    if (pthread_mutex_lock(&mutex) != 0)
    {
        PCA9548A_printf("9548lock error!\n");
    }

    WV_S32 ret;
    ret = PCA9548_SwitchToBus(id);
    if (ret != 0)
    {

        PCA9548A_printf("9548_iic switch to bus [0x%2x] err \n", id);
    }
    else
    {
        ret = HIS_IIC_Write(HI3798_IIC_CHANEL, devAddr, regAddr, data);
        if (ret != 0)
        {
            PCA9548A_printf("9548_iic write err \n");
        }
    }
    //lock
    if (pthread_mutex_unlock(&mutex) != 0)
    {
        PCA9548A_printf("9548unlock error!\n");
    }
    return ret;
}
/************************************************
Function:       PCA9548_I2C_Write(WV_U8 busID,WV_U16 data)
Description:    IIC写
Input:          busID:9548切换目标总线ID；devAddr:设备地址；regAddr:寄存器地址 
Output:         // data:读取数据（1个字节） 
Return:         // 0 成功 ；其他失败 
Other:          
************************************************/
WV_S32 PCA9548_IIC_Read(WV_U8 id, WV_U8 devAddr, WV_U8 regAddr, WV_U8 *data)
{

    if (pthread_mutex_lock(&mutex) != 0)
    {
        PCA9548A_printf("9548lock error!\n");
    }

    WV_S32 ret;
    ret = PCA9548_SwitchToBus(id);
    if (ret != 0)
    {
        PCA9548A_printf("9548_iic switch to bus [0x%2x] err \n", id);
    }
    else
    {
        ret = HIS_IIC_Read(HI3798_IIC_CHANEL, devAddr, regAddr, data);
        if (ret != 0)
        {
            PCA9548A_printf("9548_iic read err \n");
        }
    }

    if (pthread_mutex_unlock(&mutex) != 0)
    {
        PCA9548A_printf("9548unlock error!\n");
    }

    return ret;
}

/****************************************************************************

WV_S32 PCA9548_IIC_GetReg(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 PCA9548_IIC_GetReg(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
    WV_U32 busId, regAddr, dataLen;
    WV_U32 devAddr;
    WV_U8 buf[255] = {0};
    WV_S32 ret, i;

    if (argc < 4)
    {

        prfBuff += sprintf(prfBuff, "get 9548 <busID> <DevAddr> <RegAddr> <dataLen>\r\n");
        return 0;
    }
    //busid
    ret = WV_STR_S2v(argv[0], &busId);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
    }

    //devaddr
    ret = WV_STR_S2v(argv[1], &devAddr);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
    }
    //regaddr
    ret = WV_STR_S2v(argv[2], &regAddr);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
    }
    //datalen
    ret = WV_STR_S2v(argv[3], &dataLen);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
    }
    //printf("\nbusid:%d,devAddr:%02x,regAddr:%02x,dataLen:%d\n", busId, (WV_U8)devAddr, regAddr, dataLen);
    //prfBuff += sprintf("\nbusid:%d,devAddr:%02x,regAddr:%02x,dataLen:%d\n", busId, (WV_U8)devAddr, regAddr, dataLen);
    for (i = 0; i < dataLen; i++)
    {
        ret = PCA9548_IIC_Read((WV_U8)busId, (WV_U8)devAddr, (WV_U8)(regAddr + i), &buf[i]);
        if (ret != 0)
        {
            prfBuff += sprintf(prfBuff, "get %02x err\r\n", regAddr + i);
        }
        prfBuff += sprintf(prfBuff, "read 0x%02x = 0x%02x\r\n", regAddr + i, buf[i]);
    }
    return WV_SOK;
}

/****************************************************************************

WV_S32 PCA9548_IIC_SetReg(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 PCA9548_IIC_SetReg(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
    WV_U32 busId, regAddr, data;
    WV_U32 devAddr;
    WV_S32 ret;

    if (argc < 4)
    {

        prfBuff += sprintf(prfBuff, "set 9548 <busID> <DevAddr> <RegAddr> <data>\r\n");
        return 0;
    }
    //busid
    ret = WV_STR_S2v(argv[0], &busId);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
    }

    //devaddr
    ret = WV_STR_S2v(argv[1], &devAddr);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
    }
    //regaddr
    ret = WV_STR_S2v(argv[2], &regAddr);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
    }
    //datalen
    ret = WV_STR_S2v(argv[3], &data);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
    }
    //printf("\r\nbusid:%d,devAddr:%02x,regAddr:%02x,data:0x%02x\r\n", busId, (WV_U8)devAddr, regAddr, data);
    ret = PCA9548_IIC_Write((WV_U8)busId, (WV_U8)devAddr, (WV_U8)regAddr, (WV_U8)data);
    if (ret != 0)
    {
        prfBuff += sprintf(prfBuff, "get %02x err\r\n", regAddr);
    }
    prfBuff += sprintf(prfBuff, "write 0x%02x = 0x%02x\r\n", regAddr, data);

    return WV_SOK;
}

/*****************************************************
 
WV_S32 PCA9548_Init()

*****************************************************/
WV_S32 PCA9548_Init()
{
    WV_CMD_Register("get", "9548", "pca9548a iic bus read", PCA9548_IIC_GetReg);
    WV_CMD_Register("set", "9548", "pca9548aiic bus write", PCA9548_IIC_SetReg);
    return WV_SOK;
}
/*****************************************************
 
 WV_S32 PCA9548_DeInit()

*****************************************************/
WV_S32 PCA9548_DeInit()
{
    return WV_SOK;
}