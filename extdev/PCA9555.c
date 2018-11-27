//#include "stm32f10x.h"

#include "PCA9555.h"
#define PCA9555_DEV_ADDR 0x40
/************************************************
Function:       WV_S32 PCA9555_WriteRegData(unsigned char ucReg, unsigned char ucData)
Description:    pca9555 配置寄存器，实现P0和P1的输入输出配置
Input:          ucReg:寄存器； ucData:设置参数
Output:         // 无
Return:         // 0 成功 ；其他失败 
Other:          
************************************************/
WV_S32 PCA9555_WriteRegData(unsigned char ucReg, unsigned char ucData)
{
    return PCA9548_IIC_Write(PCA9548A_IIC_SWID_9555, PCA9555_DEV_ADDR, ucReg, ucData);
}

/************************************************
Function:       WV_S32 PCA9555_WriteRegData(unsigned char ucReg, unsigned char ucData)
Description:    pca9555 配置寄存器，实现P0和P1的输入输出配置
Input:          ucReg:寄存器； ucData:设置参数
Output:         // 无
Return:         // 0 成功 ；其他失败 
Other:          
************************************************/
WV_S32 PCA9555_ReadRegData(unsigned char ucReg, unsigned char *pUcData)
{
    return PCA9548_IIC_Read(PCA9548A_IIC_SWID_9555, PCA9555_DEV_ADDR, ucReg, pUcData);
}

/**************************************************************************
Function:       WV_S32 PCA9555_Set( unsigned char ucReg, unsigned char ucPin)
Description:    pca9555 配置某个管脚输出高电平
Input:          ucReg:寄存器； ucPin:管教定义
Output:         // 无
Return:         // 0 成功 ；其他失败 
Other:          
****************************************************************************/
WV_S32 PCA9555_Set(unsigned char ucReg, unsigned char ucPin)
{
    unsigned char ucData;
    WV_S32 ret;
    ret = PCA9555_ReadRegData(ucReg, &ucData);
    if (ret != 0)
    {
        printf("pca9555 set err\n");
        return ret;
    }
    ucData |= ucPin;
    return PCA9555_WriteRegData(ucReg, ucData);
}
/***************************************************************************
Function:       WV_S32 PCA9555_Clr( unsigned char ucReg, unsigned char ucPin)
Description:    pca9555 配置某个管脚输出低电平
Input:          ucReg:寄存器； ucPin:管教定义
Output:         // 无
Return:         // 0 成功 ；其他失败 
Other:          
****************************************************************************/
WV_S32 PCA9555_Clr(unsigned char ucReg, unsigned char ucPin)
{
    unsigned char ucData;
    WV_S32 ret;
    ret = PCA9555_ReadRegData(ucReg, &ucData);
    if (ret != 0)
    {
        printf("pca9555 clr err\n");
        return ret;
    }
    ucData &= ~ucPin;
    return PCA9555_WriteRegData(ucReg, ucData);
}
/***************************************************************************
Function:       WV_S32 PCA9555_Turn( unsigned char ucReg, unsigned char ucPin)
Description:    pca9555 配置某个管脚输出电平取反
Input:          ucReg:寄存器； ucPin:管教定义
Output:         // 无
Return:         // 0 成功 ；其他失败 
Other:          
****************************************************************************/
WV_S32 PCA9555_Turn(unsigned char ucReg, unsigned char ucPin)
{
    unsigned char ucData;
    WV_S32 ret;
    ret = PCA9555_ReadRegData(ucReg, &ucData);
    if (ret != 0)
    {
        printf("pca9555 turn err\n");
        return ret;
    }
    ucData ^= ucPin;
    return PCA9555_WriteRegData(ucReg, ucData);
}

/****************************************************************************

WV_S32 PCA9555_SetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 PCA9555_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
    WV_U32 regAddr, data;
    WV_S32 ret, i;
    if (argc < 3)
    {

        prfBuff += sprintf(prfBuff, "set 9555 <cmd> <regAddr> <data> ;cmd like:enapin/clrpin/reg\r\n");
        return 0;
    }
    //regAddr
    ret = WV_STR_S2v(argv[1], &regAddr);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
    }

    //data
    ret = WV_STR_S2v(argv[2], &data);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
    }

    
    if (strcmp(argv[0], "enapin") == 0)
    {

        ret = PCA9555_Set((WV_U8)regAddr, (WV_U8)data);
        if (ret != 0)
        {
            prfBuff += sprintf(prfBuff, "pca9555 cmd enapin err\r\n");
        }
        prfBuff += sprintf(prfBuff, "\r\n reg:0x%02x, data:0x%02x\r\n", (WV_U8)regAddr, (WV_U8)data);
    }
    else if (strcmp(argv[0], "clrpin") == 0){
        ret = PCA9555_Clr((WV_U8)regAddr,(WV_U8)data);
        if (ret != 0)
        {
            prfBuff += sprintf(prfBuff, "pca9555 cmd clrpin err\r\n");
        }
        prfBuff += sprintf(prfBuff, "\r\n reg:0x%02x, data:0x%02x\r\n", (WV_U8)regAddr, (WV_U8)data);
    }else if(strcmp(argv[0], "reg") == 0){
        ret = PCA9555_WriteRegData((WV_U8)regAddr,(WV_U8)data);
        prfBuff += sprintf(prfBuff, "\r\n reg:0x%02x, data:0x%02x\r\n", (WV_U8)regAddr, (WV_U8)data);
    }

    return WV_SOK;
}
/****************************************************************************

WV_S32 PCA9555_GetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 PCA9555_GetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
    WV_U32 regAddr,dataLen;
    WV_U8 buf[256];
    WV_S32 ret, i;
    if (argc < 2)
    {
        prfBuff += sprintf(prfBuff, "get 9555 <reg> <dataLen>\r\n");
        return WV_SOK;
    }
    //get regAddr
    ret = WV_STR_S2v(argv[0], &regAddr);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
    }
    //get dataLen
    ret = WV_STR_S2v(argv[1], &dataLen);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
    }
    if(dataLen >=255){
        prfBuff += sprintf(prfBuff, "dataLen is too long!\r\n");
        return WV_SOK;
    }
    for(i=0;i<dataLen;i++)
    {
        ret = PCA9555_ReadRegData((WV_U8)regAddr+i,&buf[i]);
        if(ret != 0 ){
            prfBuff += sprintf(prfBuff, "get 9555 reg err\r\n");
        }
        prfBuff += sprintf(prfBuff, " [0x%02x] = [0x%02x]\r\n",(WV_U8)regAddr+i,buf[i]);
    }
    return WV_SOK;
}
/*****************************************************
  
WV_S32 PCA9555_Init()
 
 ***************************************************/
WV_S32 PCA9555_Init()
{
    WV_CMD_Register("set", "9555", "pca9555 set reg", PCA9555_SetCmd);
    WV_CMD_Register("get", "9555", "pca9555 get reg", PCA9555_GetCmd); 
    return WV_SOK;
}
/*****************************************************
  
WV_S32 PCA9555_DeInit()
 
 ***************************************************/
WV_S32 PCA9555_DeInit()
{
    return WV_SOK;
}
