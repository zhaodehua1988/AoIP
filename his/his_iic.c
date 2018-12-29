
#include "wv_cmd.h"
#include "hi_common.h"
#include "hi_unf_i2c.h"
#include "his_iic.h"

#define HIS_IIC_DEBUG_MODE
#ifdef HIS_IIC_DEBUG_MODE
#define HIS_IIC_printf(...)             \
  do                                    \
  {                                     \
    printf("\n\rHIS-IIC:" __VA_ARGS__); \
    fflush(stdout);                     \
  } while (0)
#else
#define HIS_IIC_printf(...)
#endif

/****************************************************************************

WV_S32 HIS_IIC_GetReg(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_IIC_GetReg(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
  WV_U32 busId, regAddr, addrCnt, dataLen;
  WV_U32 devAddr;
  WV_U8 buf[255] = {0};
  WV_S32 ret, i;

  if (argc < 5)
  {
    prfBuff += sprintf(prfBuff, "get iic <BusId> <DevAddr> <RegAddr> <addrcnt>  <dataLen>\r\n");
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

  //regAddr = 0;
  //regaddr
  ret = WV_STR_S2v(argv[2], &regAddr);
  if (ret != WV_SOK)
  {
    prfBuff += sprintf(prfBuff, "input erro!\r\n");
  }

  //addrcnt
  ret = WV_STR_S2v(argv[3], &addrCnt);
  if (ret != WV_SOK)
  {
    prfBuff += sprintf(prfBuff, "input erro!\r\n");
  }

  //datalen
  ret = WV_STR_S2v(argv[4], &dataLen);
  if (ret != WV_SOK)
  {
    prfBuff += sprintf(prfBuff, "input erro!\r\n");
  }

  printf("\nbusid:%d,devAddr:%02x,regAddr:%02x,addrcnt:%d,dataLen:%d\n", busId, (WV_U8)devAddr, regAddr, addrCnt, dataLen);
  ret = HI_UNF_I2C_Read(busId, (WV_U8)devAddr, regAddr, addrCnt, buf, dataLen);
  if (ret != WV_SOK)
  {
    prfBuff += sprintf(prfBuff, "HI_UNF_I2C_Read erro = 0x%04x\r\n", ret);
  }

  prfBuff += sprintf(prfBuff, "get bus[%d] DEV[%d]:\r\n", busId, devAddr);

  for (i = 0; i < dataLen; i++)
  {
    prfBuff += sprintf(prfBuff, "%02x =%02x\r\n", regAddr + i, buf[i]);
  }

  return WV_SOK;
}

/****************************************************************************

WV_S32 HIS_IIC_SetReg(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_IIC_SetReg(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
  WV_U32 busId, regAddr, addrCnt, dataLen, data;
  WV_U32 devAddr;
  WV_U8 buf[255];
  WV_S32 ret, i;

  if (argc < 6)
  {
    prfBuff += sprintf(prfBuff, "set iic <BusId> <DevAddr> <RegAddr> <addrcnt> <Data> <dataLen>\r\n");
    return 0;
  }
  ret = WV_STR_S2v(argv[0], &busId);
  if (ret != WV_SOK)
  {
    prfBuff += sprintf(prfBuff, "input erro!\r\n");
  }
  ret = WV_STR_S2v(argv[1], &devAddr);
  if (ret != WV_SOK)
  {
    prfBuff += sprintf(prfBuff, "input erro!\r\n");
  }

  ret = WV_STR_S2v(argv[2], &regAddr);
  if (ret != WV_SOK)
  {
    prfBuff += sprintf(prfBuff, "input erro!\r\n");
  }

  ret = WV_STR_S2v(argv[3], &addrCnt);
  if (ret != WV_SOK)
  {
    prfBuff += sprintf(prfBuff, "input erro!\r\n");
  }
  ret = WV_STR_S2v(argv[4], &data);
  if (ret != WV_SOK)
  {
    prfBuff += sprintf(prfBuff, "input erro!\r\n");
  }
  ret = WV_STR_S2v(argv[5], &dataLen);
  if (ret != WV_SOK)
  {
    prfBuff += sprintf(prfBuff, "input erro!\r\n");
  }

  buf[0] = (WV_U8)data;
  //printf("\nbusID:%d,devAddr%02x,regaddr%02x,addrCnt:%d,data:%02x ,datalen:%d\n", busId, (WV_U8)devAddr, regAddr, addrCnt, buf, dataLen);
  ret = HI_UNF_I2C_Write(busId, (WV_U8)devAddr, regAddr, addrCnt, buf, dataLen);
  if (ret != WV_SOK)
  {
    prfBuff += sprintf(prfBuff, "HI_UNF_I2C_Read erro = 0x%04x\r\n", ret);
  }

  prfBuff += sprintf(prfBuff, "set bus[%d] DEV[%d]:\r\n", busId, devAddr);

  for (i = 0; i < dataLen; i++)
  {
    prfBuff += sprintf(prfBuff, "%02x =%02x\r\n", regAddr + i, buf[i]);
  }

  return WV_SOK;
}

/****************************************************************************


WV_S32  HIS_IIC_Write(WV_U32 port,WV_U8 devAddr,WV_U8 regAddr,WV_U8  data);

****************************************************************************/
WV_S32 HIS_IIC_Write(WV_U32 port, WV_U8 devAddr, WV_U8 regAddr, WV_U8 data)
{
  WV_S32 ret;
  ret = HI_UNF_I2C_Write(port, devAddr, (WV_U32)regAddr, 1, &data, 1);
  if (ret != WV_SOK)
  {
    WV_ERROR("HI_UNF_I2C_Write erro = 0x%04x\r\n", ret);
  }
  return ret;
}

/****************************************************************************


WV_S32  HIS_IIC_read(WV_U32 port,WV_U8 devAddr,WV_U8 regAddr,WV_U8 *pData);

****************************************************************************/
WV_S32 HIS_IIC_Read(WV_U32 port, WV_U8 devAddr, WV_U8 regAddr, WV_U8 *pData)
{
  WV_S32 ret;
  ret = HI_UNF_I2C_Read(port, devAddr, (WV_U32)regAddr, 1, pData, 1);
  if (ret != WV_SOK)
  {
    WV_ERROR("HI_UNF_I2C_Read erro = 0x%04x\r\n", ret);
  }
  return ret;
}

/****************************************************************************

WV_S32 HIS_IIC_Init()

****************************************************************************/
WV_S32 HIS_IIC_Init()
{
  WV_CHECK_RET(HI_UNF_I2C_Init());
  WV_CMD_Register("get", "iic", "iic bus read", HIS_IIC_GetReg);
  WV_CMD_Register("set", "iic", "iic bus read", HIS_IIC_SetReg);
  return WV_SOK;
}

/****************************************************************************

WV_S32 HIS_IIC_DeInit()

****************************************************************************/
WV_S32 HIS_IIC_DeInit()
{
  WV_CHECK_RET(HI_UNF_I2C_DeInit());
  return WV_SOK;
}
