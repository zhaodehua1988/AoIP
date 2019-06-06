#include "his_spi.h"
#include "hi_unf_spi.h"

#define HIS_SPI_DEBUG_MODE
#ifdef HIS_SPI_DEBUG_MODE
#define HIS_SPI_printf(...)                 \
	do                                      \
	{                                       \
		printf("\n\rHIS-SPI:" __VA_ARGS__); \
		fflush(stdout);                     \
	} while (0)
#else
#define HIS_SPI_printf(...)
#endif

#define HIS_SPI_DEV_SEL HI_UNF_SPI_DEV_0 // HI_UNF_SPI_DEV_0  HI_UNF_SPI_DEV_1  HI_UNF_SPI_DEV_BUTT
//#define HIS_SPI_MAX_BUFFER  256

#define HIS_SPI_BASEADDR 0XF8B1A000
#define HIS_SPI_ADDR_SPICR0 HIS_SPI_BASEADDR + 0
#define HIS_SPI_ADDR_SPICPSR HIS_SPI_BASEADDR + 0X10

#define OVERLAY_PARAMETER_DATA_PORT 0X20

typedef struct _S_HIS_SPI_Mutex
{

	pthread_mutex_t _mutex;
	WV_S32 fpgaUpdateEna;

} S_HIS_SPI_Mutex;

S_HIS_SPI_Mutex gHisSpiMutex;

/*******************************************************************************************
 * void HIS_SPI_SetFpgaUpdateEna()
 * fpga升级
 * *****************************************************************************************/
void HIS_SPI_SetFpgaUpdateEna()
{
	gHisSpiMutex.fpgaUpdateEna = 1;
}
/*******************************************************************************************
 * void HIS_SPI_SetFpgaUpdateDisable()
 * fpga升级完成
 * *****************************************************************************************/
void HIS_SPI_SetFpgaUpdateDisable()
{
	gHisSpiMutex.fpgaUpdateEna = 0;
}

/********************************************************************************************
 * WV_S32 HIS_SPI_Write_then_Read(HI_U8 *pu8WtBuf,HI_U32 u32WtNum,HI_U8 *pu8RdBuf,HI_U32 u32RdNum)
 * ******************************************************************************************/
WV_S32 HIS_SPI_Write_then_Read(WV_U8 *pu8WtBuf, WV_U32 u32WtNum, WV_U8 *pu8RdBuf, WV_U32 u32RdNum)
{

	if (pthread_mutex_lock(&gHisSpiMutex._mutex) != 0)
	{
		return WV_EFAIL;
	}
	HI_UNF_SPI_DEV_E dev = HIS_SPI_DEV_SEL;

	WV_S32 ret = HI_UNF_SPI_ReadExt(dev, pu8WtBuf, u32WtNum, pu8RdBuf, u32RdNum);
	pthread_mutex_unlock(&gHisSpiMutex._mutex);
	return ret;
}
/******************************************************************************************
 * WV_S32 HIS_SPI_Read(HI_U8 *pu8RdBuf,HI_U32 u32RdNum)
 * ***************************************************************************************/
WV_S32 HIS_SPI_Read(WV_U8 *pu8RdBuf, WV_U32 u32RdNum)
{
	if (pthread_mutex_lock(&gHisSpiMutex._mutex) != 0)
	{
		return WV_EFAIL;
	}
	HI_UNF_SPI_DEV_E dev = HIS_SPI_DEV_SEL;
	WV_S32 ret = HI_UNF_SPI_Read(dev, pu8RdBuf, u32RdNum);
	pthread_mutex_unlock(&gHisSpiMutex._mutex);
	return ret;
}
/******************************************************************************************
 * WV_S32 HIS_SPI_Write(WV_U8 *pu8WtBuf, WV_U32 u32WtNum)
 * ***************************************************************************************/
WV_S32 HIS_SPI_Write(WV_U8 *pu8WtBuf, WV_U32 u32WtNum)
{
	if (pthread_mutex_lock(&gHisSpiMutex._mutex) != 0)
	{
		return WV_EFAIL;
	}
	HI_UNF_SPI_DEV_E dev = HIS_SPI_DEV_SEL;
	WV_S32 ret = HI_UNF_SPI_Write(dev, pu8WtBuf, u32WtNum);
	pthread_mutex_unlock(&gHisSpiMutex._mutex);
	return ret;
}

/*******************************************************************************************************

WV_S32  HIS_SPI_FpgaWd(WV_U16 addr,WV_U16 data);

*******************************************************************************************************/
WV_S32 HIS_SPI_FpgaWd(WV_U16 addr, WV_U16 data)
{
	if (gHisSpiMutex.fpgaUpdateEna == 1)
	{
		//WV_ERROR("fpga is updating !!");
		return WV_EFAIL;
	}

	WV_S32 ret;
	WV_U8 wBuf[20] = {0};
	wBuf[0] = (addr >> 4) | 0x80;
	wBuf[1] = (addr << 4) & 0xff;
	wBuf[2] = (data >> 8) & 0xff;
	wBuf[3] = data & 0xff;

	ret = HIS_SPI_Write(wBuf, 4);
	usleep(1);
	if (ret != 0)
	{
		HIS_SPI_printf("FPGA write : 0x%04x  = 0x%04x[%d]", addr, data, data);
	}
	return ret;
}

/*******************************************************************************************************

WV_S32  HIS_SPI_FpgaRd(WV_U16 addr,WV_U16 * pData);

*******************************************************************************************************/
WV_S32 HIS_SPI_FpgaRd(WV_U16 addr, WV_U16 *pData)
{

	if (gHisSpiMutex.fpgaUpdateEna == 1)
	{
		//WV_ERROR("fpga is updating !!");
		return WV_SOK;
	}

	WV_U8 rBuf[20], wBuf[20];
	WV_S32 ret;
	wBuf[0] = (addr >> 4) & 0x7f;
	wBuf[1] = (addr << 4) & 0xff;
	ret = HIS_SPI_Write_then_Read((HI_U8 *)wBuf, 2, (HI_U8 *)rBuf, 2);
	if (ret != 0)
	{
		HIS_SPI_printf("FPGA read : 0x%04x  = 0x%04x", addr, rBuf[0]);
		return WV_EFAIL;
	}
	*pData = (rBuf[0] << 8) | rBuf[1];
	return WV_SOK;
}
/*******************************************************************************************************

WV_S32  HIS_SPI_FpgaRdNum(WV_U16 addr,WV_U16 * pData,WV_U32 dataNum);

*******************************************************************************************************/
WV_S32 HIS_SPI_FpgaRdNum(WV_U16 addr, WV_U16 *pData, WV_U32 dataNum)
{

	if (gHisSpiMutex.fpgaUpdateEna == 1)
	{
		//WV_ERROR("fpga is updating !!");
		return WV_SOK;
	}

	WV_U8 wBuf[20];
	WV_U8 rBuf[1024] = {0};
	WV_S32 ret, i;
	wBuf[0] = (addr >> 4) & 0x7f;
	wBuf[1] = (addr << 4) & 0xff;
	ret = HIS_SPI_Write_then_Read((HI_U8 *)wBuf, 2, (HI_U8 *)rBuf, dataNum * 2);
	for (i = 0; i < dataNum; i++)
	{
		pData[i] = (rBuf[2 * i] << 8) | rBuf[2 * i + 1];
	}

	if (ret != 0)
	{
		HIS_SPI_printf("FPGA read : 0x%04x err\r\n.", addr);
	}
	return WV_SOK;
}

/****************************************************************************

WV_S32 HIS_SPI_CMDWrite(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_SPI_CMDWrite(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{

	WV_U16 data, addr;
	WV_U32 temp;
	WV_S32 ret = 0;
	;
	if (argc < 2)
	{
		prfBuff += sprintf(prfBuff, "set spi1  <u16Addr> <u16Data>  \r\n");
		return WV_SOK;
	}
	//
	ret = WV_STR_S2v(argv[0], &temp);
	if (ret != WV_SOK)
	{
		prfBuff += sprintf(prfBuff, "input erro !\r\n");
		return WV_SOK;
	}
	addr = temp & 0xffff;
	//
	ret = WV_STR_S2v(argv[1], &temp);
	if (ret != WV_SOK)
	{
		prfBuff += sprintf(prfBuff, "input erro !\r\n");
		return WV_SOK;
	}
	data = temp & 0xffff;
	HIS_SPI_FpgaWd(addr, data);
	prfBuff += sprintf(prfBuff, "spi writ fpga   0x%x = 0x%x \r\n", addr, data);
	return WV_SOK;
}

/****************************************************************************

WV_S32 HIS_SPI_CMDWriteFromFile(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_SPI_CMDWriteFromFile(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{

	WV_U16 addr;
	WV_U32 dataNum, temp;
	WV_S32 ret = 0, i = 0;
	WV_U32 data[2048];
	WV_S8 buf[10] = {0};
	if (argc < 2)
	{
		prfBuff += sprintf(prfBuff, "set spifile <u16Addr>  <num>\r\n");
		return WV_SOK;
	}
	//get addr
	ret = WV_STR_S2v(argv[0], &temp);
	if (ret != WV_SOK)
	{
		prfBuff += sprintf(prfBuff, "input erro !\r\n");
		return WV_SOK;
	}
	addr = temp & 0xffff;
	//get datanum
	ret = WV_STR_S2v(argv[1], &temp);
	if (ret != WV_SOK)
	{
		prfBuff += sprintf(prfBuff, "input erro !\r\n");
		return WV_SOK;
	}
	dataNum = temp;
	if (dataNum >= 2048)
	{
		prfBuff += sprintf(prfBuff, "datanum  erro !\r\n");
		return WV_SOK;
	}
	// get data from file
	FILE *fp = NULL;
	fp = fopen("./spi.txt", "r");
	if (fp == NULL)
	{
		prfBuff += sprintf(prfBuff, "get spi cmd  file err\r\n");
		return WV_SOK;
	}

	for (i = 0; i < dataNum; i++)
	{
		if (fgets(buf, 8, fp) == NULL)
			break;
		sscanf(buf, "%x", &data[i]);
	}
	fclose(fp);
	if (i == dataNum)
	{
		for (i = 0; i < dataNum; i++)
		{
			usleep(100000);
			HIS_SPI_FpgaWd(addr, (WV_U16)data[i]);
			prfBuff += sprintf(prfBuff, "[%d]set 0x%X=0x%X\r\n", i + 1, addr, data[i]);
		}
	}
	else
	{
		prfBuff += sprintf(prfBuff, "datanum  erro num=[%d]!\r\n", dataNum);
	}

	return WV_SOK;
}

/****************************************************************************

WV_S32 HIS_SPI_CMDRead(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_SPI_CMDRead(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{

	WV_U16 data[4096], addr;
	WV_U32 temp, dataNum;
	WV_S32 ret = 0, i = 0;
	if (argc < 2)
	{
		prfBuff += sprintf(prfBuff, "get spi  <u16Addr> <dataNum> \r\n");
		return WV_SOK;
	}
	//
	ret = WV_STR_S2v(argv[0], &temp);
	if (ret != WV_SOK)
	{
		prfBuff += sprintf(prfBuff, "input erro !\r\n");
		return WV_SOK;
	}
	ret = WV_STR_S2v(argv[1], &dataNum);
	if (ret != WV_SOK)
	{
		prfBuff += sprintf(prfBuff, "input erro !\r\n");
		return WV_SOK;
	}

	if (dataNum >= 4096)
	{
		prfBuff += sprintf(prfBuff, "read spi number can not >= 4096\r\n");
		return WV_SOK;
	}

	addr = temp & 0xffff;
	prfBuff += sprintf(prfBuff, "spi read 0x%0X ,dataNum=%d\n", addr, dataNum);
	ret = HIS_SPI_FpgaRdNum(addr, data, dataNum);
	//ret = HIS_SPI_FpgaRd(addr,data);
	if (ret != 0)
	{
		prfBuff += sprintf(prfBuff, "get spi err \r\n");
	}
	else
	{
		for (i = 0; i < dataNum; i++)
		{
			prfBuff += sprintf(prfBuff, "spi read fpga   0x%x = 0x%x[%d] \r\n", addr, data[i], data[i]);
		}
	}

	return WV_SOK;
}
/*******************************************************************************************************

WV_S32  HIS_SPI_Init();

*******************************************************************************************************/

WV_S32 HIS_SPI_Init()
{

	HI_UNF_SPI_DEV_E dev;
	HI_UNF_SPI_ATTR_S attr;
	WV_U32 regAddr, regData, scr, cpsdvr;

	WV_ASSERT_RET(HI_UNF_SPI_Init());
	dev = HIS_SPI_DEV_SEL;
	WV_ASSERT_RET(HI_UNF_SPI_Open(dev));
	WV_ASSERT_RET(HI_UNF_SPI_GetAttr(dev, &attr));

	//最初
	// attr.enDev = dev;
	// attr.csCfg = HI_UNF_SPI_LOGIC_CS;
	// attr.u32Baud = 25;
	// attr.enFrf = HI_UNF_SPI_FRF_MOTO;
	// attr.u32Dss = 16;
	// attr.unExtAttr.stMoto.enSph = HI_UNF_SPI_SPH_1;
	// attr.unExtAttr.stMoto.enSpo = HI_UNF_SPI_SPO_1;
	// attr.enBigend = HI_UNF_SPI_BIGEND_BIG;

	//可用1
	attr.enDev = dev;
	attr.csCfg = HI_UNF_SPI_LOGIC_CS;
	attr.u32Baud = 10;
	attr.enFrf = HI_UNF_SPI_FRF_MOTO; //HI_UNF_SPI_FRF_NM;//HI_UNF_SPI_FRF_TI;//HI_UNF_SPI_FRF_MOTO;
	attr.u32Dss = 8;
	attr.unExtAttr.stMoto.enSph = HI_UNF_SPI_SPH_1;
	attr.unExtAttr.stMoto.enSpo = HI_UNF_SPI_SPO_1;
	attr.enBigend = HI_UNF_SPI_BIGEND_LITTLE;

	WV_ASSERT_RET(HI_UNF_SPI_SetAttr(dev, &attr));

	// u32Baud = 100 /(cpsdvr * (1+ scr));

	scr = 0;
	//cpsdvr =4;// 4 :25M  2 : 50M
	cpsdvr = 10;
	regAddr = HIS_SPI_ADDR_SPICR0;
	WV_ASSERT_RET(HI_SYS_ReadRegister(regAddr, &regData));
	regData = (regData & 0xFF) | (scr << 8);
	WV_ASSERT_RET(HI_SYS_WriteRegister(regAddr, regData));

	regAddr = HIS_SPI_ADDR_SPICPSR;
	regData = cpsdvr;
	WV_ASSERT_RET(HI_SYS_WriteRegister(regAddr, regData));
	WV_ASSERT_RET(HI_UNF_SPI_GetAttr(dev, &attr));

	// HIS_SPI_printf("attr.enDev[%d]", attr.enDev);
	// HIS_SPI_printf("attr.csCfg[%d]", attr.csCfg);
	// HIS_SPI_printf("attr.u32Baud[%d]", attr.u32Baud);
	// HIS_SPI_printf("attr.enFrf[%d]", attr.enFrf);
	// HIS_SPI_printf("attr.u32Dss[%d]", attr.u32Dss);
	// HIS_SPI_printf("attr.enBigend[%d]", attr.enBigend);
	// HIS_SPI_printf("attr.enSph[%d]", attr.unExtAttr.stMoto.enSph);
	// HIS_SPI_printf("attr.enSpoh[%d]", attr.unExtAttr.stMoto.enSpo);

	WV_CMD_Register("set", "spi", "spi bus write fpga1 sigle", HIS_SPI_CMDWrite);
	WV_CMD_Register("get", "spi", "spi bus read fpga1 sigle", HIS_SPI_CMDRead);
	WV_CMD_Register("set", "spif", "spi bus read fpga1 sigle", HIS_SPI_CMDWriteFromFile);

	if (pthread_mutex_init(&gHisSpiMutex._mutex, NULL) != 0)
	{
		WV_ERROR("spi mutex init err!!!");
	}

	gHisSpiMutex.fpgaUpdateEna = 0;
	return WV_SOK;
}

/*******************************************************************************************************

WV_S32  HIS_SPI_DeInit();

*******************************************************************************************************/

WV_S32 HIS_SPI_DeInit()
{
	HI_UNF_SPI_DEV_E dev;
	dev = HIS_SPI_DEV_SEL;
	WV_ASSERT_RET(HI_UNF_SPI_Close(dev));
	WV_ASSERT_RET(HI_UNF_SPI_DeInit());

	return WV_SOK;
}
