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

OVERLAY_PARAMETER fpga_parameter[16] = {
	{.BITS.video_channel = 1,
	 .BITS.outline_channel = 0,
	 .BITS.linenumber = 200,
	 .BITS.pixelnumber = 450,
	 .BITS.yoffset = 0,
	 .BITS.xoffset = 0},
	{.BITS.video_channel = 1,
	 .BITS.outline_channel = 0,
	 .BITS.linenumber = 200,
	 .BITS.pixelnumber = 450,
	 .BITS.yoffset = 0,
	 .BITS.xoffset = 450},
	{.BITS.video_channel = 1,
	 .BITS.outline_channel = 2,
	 .BITS.linenumber = 200,
	 .BITS.pixelnumber = 450,
	 .BITS.yoffset = 0,
	 .BITS.xoffset = 900},
	{.BITS.video_channel = 1,
	 .BITS.outline_channel = 3,
	 .BITS.linenumber = 200,
	 .BITS.pixelnumber = 450,
	 .BITS.yoffset = 0,
	 .BITS.xoffset = 1300},
	{.BITS.video_channel = 1,
	 .BITS.outline_channel = 4,
	 .BITS.linenumber = 200,
	 .BITS.pixelnumber = 450,
	 .BITS.yoffset = 0,
	 .BITS.xoffset = 1800},
	{.BITS.video_channel = 1,
	 .BITS.outline_channel = 5,
	 .BITS.linenumber = 200,
	 .BITS.pixelnumber = 450,
	 .BITS.yoffset = 0,
	 .BITS.xoffset = 2250},
	{.BITS.video_channel = 1,
	 .BITS.outline_channel = 6,
	 .BITS.linenumber = 200,
	 .BITS.pixelnumber = 450,
	 .BITS.yoffset = 0,
	 .BITS.xoffset = 2700},
	{.BITS.video_channel = 1,
	 .BITS.outline_channel = 7,
	 .BITS.linenumber = 200,
	 .BITS.pixelnumber = 450,
	 .BITS.yoffset = 0,
	 .BITS.xoffset = 3150},
	{.BITS.video_channel = 1,
	 .BITS.outline_channel = 8,
	 .BITS.linenumber = 200,
	 .BITS.pixelnumber = 450,
	 .BITS.yoffset = 600,
	 .BITS.xoffset = 0},
	{.BITS.video_channel = 1,
	 .BITS.outline_channel = 9,
	 .BITS.linenumber = 200,
	 .BITS.pixelnumber = 450,
	 .BITS.yoffset = 600,
	 .BITS.xoffset = 450},
	{.BITS.video_channel = 1,
	 .BITS.outline_channel = 10,
	 .BITS.linenumber = 200,
	 .BITS.pixelnumber = 450,
	 .BITS.yoffset = 600,
	 .BITS.xoffset = 900},
	{.BITS.video_channel = 1,
	 .BITS.outline_channel = 11,
	 .BITS.linenumber = 200,
	 .BITS.pixelnumber = 450,
	 .BITS.yoffset = 600,
	 .BITS.xoffset = 1350},
	{.BITS.video_channel = 1,
	 .BITS.outline_channel = 12,
	 .BITS.linenumber = 200,
	 .BITS.pixelnumber = 450,
	 .BITS.yoffset = 600,
	 .BITS.xoffset = 1800},
	{.BITS.video_channel = 0,
	 .BITS.outline_channel = 13,
	 .BITS.linenumber = 200,
	 .BITS.pixelnumber = 450,
	 .BITS.yoffset = 6000,
	 .BITS.xoffset = 2250},
	{.BITS.video_channel = 0,
	 .BITS.outline_channel = 14,
	 .BITS.linenumber = 200,
	 .BITS.pixelnumber = 450,
	 .BITS.yoffset = 600,
	 .BITS.xoffset = 2700},
	{.BITS.video_channel = 1,
	 .BITS.outline_channel = 15,
	 .BITS.linenumber = 200,
	 .BITS.pixelnumber = 450,
	 .BITS.yoffset = 600,
	 .BITS.xoffset = 3150},
};

OVERLAY_REG fpga_reg = {
	.ADDR.S.frame_number_addr = 0X09,

	.ADDR.S.splite_start_line_addr = 0X10,
	.ADDR.S.splite_end_line_addr = 0X11,
	.ADDR.S.start_pixel_split1_addr = 0X12,
	.ADDR.S.start_pixel_split2_addr = 0X14,
	.ADDR.S.start_pixel_split3_addr = 0X16,
	.ADDR.S.start_pixel_split4_addr = 0X18,
	.ADDR.S.end_pixel_split1_addr = 0X13,
	.ADDR.S.end_pixel_split2_addr = 0X15,
	.ADDR.S.end_pixel_split3_addr = 0X17,
	.ADDR.S.end_pixel_split4_addr = 0X19,
	.ADDR.S.split_start_addr = 0x0c,
	.ADDR.S.video_gen_test_addr = 0x25,

	.ADDR.S.start_addr = 0x0b,

	.DATA.S.frame_number = 0X2,
	.DATA.S.splite_start_line = 0X00,
	.DATA.S.splite_end_line = 0X437,
	.DATA.S.start_pixel_split1 = 0X0,
	.DATA.S.start_pixel_split2 = 0X3C1,
	.DATA.S.start_pixel_split3 = 0X0,
	.DATA.S.start_pixel_split4 = 0x3c1,

	.DATA.S.end_pixel_split1 = 0x3c0,
	.DATA.S.end_pixel_split2 = 0x780,
	.DATA.S.end_pixel_split3 = 0x3c0,
	.DATA.S.end_pixel_split4 = 0x780,
	.DATA.S.split_start = 0x000f,
	.DATA.S.video_gen_test = 0x00,

	.DATA.S.start = 0x0000

};

OUTPUT_REG output_reg = {
	.ADDR.S.ch_en_addr = 0x0f,
	.ADDR.S.freq_l_addr = 0x11,
	.ADDR.S.freq_h_addr = 0x12,
	.ADDR.S.freq_up_addr1 = 0x13,
	.ADDR.S.freq_up_addr2 = 0x13,
	.ADDR.S.total_pix_addr = 0x16,
	.ADDR.S.total_line_addr = 0x17,
	.ADDR.S.hsync_width_addr = 0x18,
	.ADDR.S.vsync_width_addr = 0x19,
	.ADDR.S.active_hstart_addr = 0x1a,
	.ADDR.S.active_hsize_addr = 0x1b,
	.ADDR.S.active_vstart_addr = 0x1c,
	.ADDR.S.active_vsize_addr = 0x1d,

	.DATA.S.ch_en = 0x00ff,
	.DATA.S.freq_l = 0x0,
	.DATA.S.freq_h = 0x0,
	.DATA.S.freq_up1 = 0x0,
	.DATA.S.freq_up2 = 0x0,
	.DATA.S.total_pix = 2200,
	.DATA.S.total_line = 1125,
	.DATA.S.hsync_width = 44,
	.DATA.S.vsync_width = 5,
	.DATA.S.active_hstart = 192,
	.DATA.S.active_hsize = 1920,
	.DATA.S.active_vstart = 41,
	.DATA.S.active_vsize = 1080,
};

/*******************************************************************************************************

WV_S32  HIS_SPI_SpiWrit(WV_U16 addr,WV_U16 data);

*******************************************************************************************************/
WV_S32 HIS_SPI_SpiWrit(WV_U16 addr, WV_U16 data)
{
	HI_UNF_SPI_DEV_E dev;
	WV_U16 buf[20];
	WV_S32 ret;
	dev = HIS_SPI_DEV_SEL;
	buf[0] = addr;
	buf[1] = data;

	ret = HI_UNF_SPI_Write(dev, (HI_U8 *)buf, 4);

	//HIS_SPI_printf("SPI write : 0x%04x  = 0x%04x",buf[0],buf[1]);
	//return WV_SOK;
	return ret;
}

/*******************************************************************************************************

WV_S32  HIS_SPI_FpgaWd(WV_U16 addr,WV_U16 data);

*******************************************************************************************************/
WV_S32 HIS_SPI_FpgaWd(WV_U16 addr, WV_U16 data)
{
	HI_UNF_SPI_DEV_E dev;
	WV_U16 buf[20];
	WV_S32 ret;
	dev = HIS_SPI_DEV_SEL;
	buf[0] = (addr << 4) | 0x8000;
	buf[1] = data;

	ret = HI_UNF_SPI_Write(dev, (HI_U8 *)buf, 4);
	usleep(1);
	if (ret != 0)
	{
		HIS_SPI_printf("FPGA write : 0x%04x  = 0x%04x[%d]", addr, buf[1], buf[1]);
	}
	//return WV_SOK;
	return ret;
}

/*******************************************************************************************************

WV_S32  HIS_SPI_FpgaWd2(WV_U16 addr,WV_U16 data);

*******************************************************************************************************/
WV_S32 HIS_SPI_FpgaWd2(WV_U16 addr, WV_U16 data)
{
	HI_UNF_SPI_DEV_E dev;
	WV_U16 buf[20];
	WV_S32 ret;
	dev = HIS_SPI_DEV_SEL;
	buf[0] = (addr << 6) | 0xc000;
	buf[1] = data;

	ret = HI_UNF_SPI_Write(dev, (HI_U8 *)buf, 4);
	usleep(1);
	if (ret != 0)
	{
		HIS_SPI_printf("FPGA write 2 : 0x%04x  = 0x%04x[%d]", addr, buf[1], buf[1]);
	}
	//return WV_SOK;
	return ret;
}
/*******************************************************************************************************

WV_S32  HIS_SPI_FpgaWd_buffer(WV_U16 *pBuffer,WV_U16 length);
// pBuffer[0]  =  addrs|0x8000;  first addr indicate write to fpga.
// length include addr, all pBuffer content means data to spi driver. addr only meaning to fpga

*******************************************************************************************************/

WV_S32 HIS_SPI_FpgaWd_buffer(WV_U16 *pBuffer, WV_U16 length)
{
	HI_UNF_SPI_DEV_E dev;
	WV_S32 ret;
	dev = HIS_SPI_DEV_SEL;
	//buf[0] = addr | 0x8000;
	//buf[1] = data;

	ret = HI_UNF_SPI_Write(dev, (HI_U8 *)pBuffer, sizeof(WV_U16) * length);

	return ret;

	//return WV_SOK;
}
/*******************************************************************************************************

WV_S32  HIS_SPI_FpgaRd(WV_U16 addr,WV_U16 * pData);

*******************************************************************************************************/
#if 1
WV_S32 HIS_SPI_FpgaRd(WV_U16 addr, WV_U16 *pData)
{
	HI_UNF_SPI_DEV_E dev;
	WV_U16 rBuf[20], wBuf[20];
	WV_S32 ret;
	dev = HIS_SPI_DEV_SEL;
	wBuf[0] = (addr << 4) & 0x7fff;
	ret = HI_UNF_SPI_ReadExt(dev, (HI_U8 *)wBuf, 2, (HI_U8 *)rBuf, 2);
	*pData = rBuf[0];
	if (ret != 0)
	{
		HIS_SPI_printf("FPGA read : 0x%04x  = 0x%04x", addr, rBuf[0]);
	}
	return WV_SOK;
}
#endif
/*******************************************************************************************************

WV_S32  HIS_SPI_FpgaRdNum(WV_U16 addr,WV_U16 * pData,WV_U32 dataNum);

*******************************************************************************************************/
WV_S32 HIS_SPI_FpgaRdNum(WV_U16 addr, WV_U16 *pData, WV_U32 dataNum)
{
	HI_UNF_SPI_DEV_E dev;
	WV_U16 rBuf[20], wBuf[20];
	WV_S32 ret;
	dev = HIS_SPI_DEV_SEL;
	wBuf[0] = (addr << 4) & 0x7fff;
	ret = HI_UNF_SPI_ReadExt(dev, (HI_U8 *)wBuf, 2, (HI_U8 *)pData, dataNum * 2);
	if (ret != 0)
	{
		HIS_SPI_printf("FPGA read : 0x%04x err\r\n.", addr);
	}
	return WV_SOK;
}

/*******************************************************************************************************

WV_S32  HIS_SPI_FpgaRd2(WV_U16 addr,WV_U16 * pData);

*******************************************************************************************************/
WV_S32 HIS_SPI_FpgaRd2(WV_U16 addr, WV_U16 *pData)
{
	HI_UNF_SPI_DEV_E dev;
	WV_U16 rBuf[20], wBuf[20];
	WV_S32 ret;
	dev = HIS_SPI_DEV_SEL;

	wBuf[0] = (addr << 6) & 0x7fff;
	wBuf[0] |= 0x4000;
	ret = HI_UNF_SPI_ReadExt(dev, (HI_U8 *)wBuf, 2, (HI_U8 *)rBuf, 2);
	*pData = rBuf[0];
	if (ret != 0)
	{
		HIS_SPI_printf("FPGA read 2: 0x%04x  = 0x%04x", addr, rBuf[0]);
	}
	return WV_SOK;
}

/****************************************************************************

WV_S32 HIS_SPI_SetFpga(WV_S
32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_SPI_SetFpga(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
	HIS_SPI_printf("FPGA write ");

	//HIS_SPI_FpgaWd(2,0xf0);
	WV_U16 i;
	for (i = 0; i < 14; i++)
	{
		HIS_SPI_FpgaWd(fpga_reg.ADDR.address[i], fpga_reg.DATA.data[i]);
	}

	return WV_SOK;
}

/****************************************************************************

WV_S32 HIS_SPI_GetFpga(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_SPI_GetFpga(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
	prfBuff += sprintf(prfBuff, "FPGA read\r\n");
	WV_U16 data;
	WV_U16 i;
	for (i = 0; i <= 0x30; i++)
	{
		HIS_SPI_FpgaRd(i, &data);
		prfBuff += sprintf(prfBuff, " fpga reg %d = %x\r\n", i, data);
	}
	return WV_SOK;
}
/****************************************************************************

WV_S32 HIS_SPI_SetParameter(WV_S
32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_SPI_SetParameter(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
	HIS_SPI_printf("FPGA Set Parameter ");

	//HIS_SPI_FpgaWd(2,0xf0);
	WV_U16 i, j;
	for (j = 0; j < 16; j++)
	{
		for (i = 0; i < 4; i++)
		{
			HIS_SPI_FpgaWd(OVERLAY_PARAMETER_DATA_PORT, fpga_parameter[j].parameter[3 - i]);
		}
	}
	return WV_SOK;
}
/****************************************************************************

WV_S32 HIS_SPI_TestFpga(WV_S
32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_SPI_TestFpga(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
	HIS_SPI_printf("FPGA test\r\n");
	WV_U16 data;
	WV_U16 i, j;
	WV_U16 error;
	WV_U16 error_data;

	error = 0xffff;
	for (j = 0; j < 100; j++)
	{
		for (i = 0; i < 13; i++)
		{
			HIS_SPI_FpgaWd(fpga_reg.ADDR.address[i], fpga_reg.DATA.data[i]);
		}
		for (i = 0; i < 13; i++)
		{
			HIS_SPI_FpgaRd(fpga_reg.ADDR.address[i], &data);
			if (data != fpga_reg.DATA.data[i])
			{
				error = i;
				error_data = data;
			}
		}
		if (error != 0xffff)
			break;
	}
	HIS_SPI_printf("TestFpga complete! j = %x  i = %x  errordata = %x\n", j, error, error_data);
	return WV_SOK;
}

/****************************************************************************

WV_S32 HIS_SPI_SetFpga2(WV_S
32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_SPI_SetFpga2(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
	HIS_SPI_printf("FPGA write 2");

	//HIS_SPI_FpgaWd(2,0xf0);
	WV_U16 i;
	for (i = 0; i < 13; i++)
	{
		HIS_SPI_FpgaWd2(output_reg.ADDR.address[i], output_reg.DATA.data[i]);
	}

	return WV_SOK;
}

/****************************************************************************

WV_S32 HIS_SPI_GetFpga2(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_SPI_GetFpga2(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
	HIS_SPI_printf("FPGA read 2");
	WV_U16 data;
	WV_U16 i;
	for (i = 0; i < 13; i++)
	{
		HIS_SPI_FpgaRd2(output_reg.ADDR.address[i], &data);
	}
	return WV_SOK;
}

/****************************************************************************

WV_S32 HIS_SPI_TestFpga2(WV_S
32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_SPI_TestFpga2(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
	HIS_SPI_printf("FPGA test 2");
	WV_U16 data;
	WV_U16 i, j;
	WV_U16 error;
	WV_U16 error_data;

	error = 0xffff;

	for (j = 0; j < 100; j++)
	{
		for (i = 0; i < 13; i++)
		{
			HIS_SPI_FpgaWd2(output_reg.ADDR.address[i], output_reg.DATA.data[i]);
		}
		for (i = 0; i < 13; i++)
		{
			HIS_SPI_FpgaRd2(output_reg.ADDR.address[i], &data);
			if (data != output_reg.DATA.data[i])
			{
				error = i;
				error_data = data;
			}
		}
		if (error != 0xffff)
			break;
	}

	/*HIS_SPI_FpgaWd2(0x01,0xaaaa);
	for( j = 0 ; j < 100 ; j ++)
	{
		HIS_SPI_FpgaRd2(0x01,&data);
	}
	*/
	HIS_SPI_printf("TestFpga complete! j = %x  i = %x  errordata = %x\n", j, error, error_data);
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
	//
	//WV_ASSERT_RET(HIS_SPI_SpiWrit(addr,data));
	HIS_SPI_FpgaWd(addr, data);
	prfBuff += sprintf(prfBuff, "spi writ fpga1   0x%x = 0x%x \r\n", addr, data);
	return WV_SOK;
}

/****************************************************************************

WV_S32 HIS_SPI_CMDWriteFromFile(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_SPI_CMDWriteFromFile(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{

	WV_U16 addr;
	WV_U32 dataNum, temp, readLen;
	WV_S32 ret = 0, i = 0, j = 0;
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
			prfBuff += sprintf(prfBuff, "[%d]set 0x%X=0x%X\r\n", i+1,addr, data[i]);
		}
	}else{
		prfBuff += sprintf(prfBuff, "datanum  erro num=[%d]!\r\n",dataNum);
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
	ret = HIS_SPI_FpgaRdNum(addr, &data, dataNum);
	if (ret != 0)
	{
		prfBuff += sprintf(prfBuff, "get spi err \r\n");
	}
	else
	{
		for (i = 0; i < dataNum; i++)
		{
			prfBuff += sprintf(prfBuff, "spi read fpga   0x%x = 0x%x \r\n", addr + i, data[i]);
		}
	}
	/*
	for(i=0;i<dataNum;i++){
		//temp=temp+i;
		addr = (temp+i) & 0xffff; 
		HIS_SPI_FpgaRd(addr, &data);
		prfBuff += sprintf(prfBuff,"spi read fpga   0x%x = 0x%x \r\n",addr,data);		
	}
	*/

	//addr = temp & 0xffff;
	//HIS_SPI_FpgaRd(addr, &data);
	// prfBuff += sprintf(prfBuff,"spi read fpga1   0x%x = 0x%x \r\n",addr,data);
	return WV_SOK;
}

/****************************************************************************

WV_S32 HIS_SPI_CMDWrite2(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_SPI_CMDWrite2(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{

	WV_U16 data, addr;
	WV_U32 temp;
	WV_S32 ret = 0;
	;
	if (argc < 2)
	{
		prfBuff += sprintf(prfBuff, "set spi2  <u16Addr> <u16Data>  \r\n");
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
	//
	//WV_ASSERT_RET(HIS_SPI_SpiWrit(addr,data));
	HIS_SPI_FpgaWd2(addr, data);
	prfBuff += sprintf(prfBuff, "spi writ fpga2   0x%x = 0x%x \r\n", addr, data);
	return WV_SOK;
}

/****************************************************************************

WV_S32 HIS_SPI_CMDRead2(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_SPI_CMDRead2(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{

	WV_U16 data, addr;
	WV_U32 temp;
	WV_S32 ret = 0;
	;
	if (argc < 1)
	{
		prfBuff += sprintf(prfBuff, "get spi2  <u16Addr>  \r\n");
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
	//WV_ASSERT_RET(HIS_SPI_SpiWrit(addr,data));

	HIS_SPI_FpgaRd2(addr, &data);
	prfBuff += sprintf(prfBuff, "spi read fpga2   0x%x = 0x%x \r\n", addr, data);
	return WV_SOK;
}

/*******************************************************************************************************

WV_S32  HIS_SPI_FpgaReset();

*******************************************************************************************************/

WV_S32 HIS_SPI_FpgaReset()
{
	HIS_SPI_FpgaWd2(0x33, 1);
	HIS_SPI_FpgaWd2(0x33, 0);
	HIS_SPI_FpgaWd(0x0d, 1);
	HIS_SPI_FpgaWd(0x0d, 0);

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
	/*
  HIS_SPI_printf("attr.enDev[%d]",attr.enDev);
  HIS_SPI_printf("attr.csCfg[%d]",attr.csCfg);
  HIS_SPI_printf("attr.u32Baud[%d]",attr.u32Baud);
  HIS_SPI_printf("attr.enFrf[%d]",attr.enFrf);
  HIS_SPI_printf("attr.u32Dss[%d]",attr.u32Dss);
  HIS_SPI_printf("attr.enBigend[%d]",attr.enBigend);
  HIS_SPI_printf("attr.enSph[%d]",attr.unExtAttr.stMoto.enSph);
  HIS_SPI_printf("attr.enSpoh[%d]",attr.unExtAttr.stMoto.enSpo);
  //HIS_SPI_printf("attr.unExtAttr[%d]",attr.unExtAttr); 
  */
	attr.enDev = dev;
	attr.csCfg = HI_UNF_SPI_LOGIC_CS;
	attr.u32Baud = 25;
	attr.enFrf = HI_UNF_SPI_FRF_MOTO;
	attr.u32Dss = 16;
	attr.unExtAttr.stMoto.enSph = HI_UNF_SPI_SPH_1;
	attr.unExtAttr.stMoto.enSpo = HI_UNF_SPI_SPO_1;
	WV_ASSERT_RET(HI_UNF_SPI_SetAttr(dev, &attr));

	// u32Baud = 100 /(cpsdvr * (1+ scr));

	scr = 0;
	//cpsdvr =4;// 4 :25M  2 : 50M
	cpsdvr = 10;
	regAddr = HIS_SPI_ADDR_SPICR0;
	WV_ASSERT_RET(HI_SYS_ReadRegister(regAddr, &regData));
	regData = (regData & 0xFF) | (scr << 8);
	WV_ASSERT_RET(HI_SYS_WriteRegister(regAddr, regData));
	// HIS_SPI_printf("WriteRegister(0x%04x,0x%04x)",regAddr,regData);

	regAddr = HIS_SPI_ADDR_SPICPSR;
	regData = cpsdvr;
	WV_ASSERT_RET(HI_SYS_WriteRegister(regAddr, regData));
	// HIS_SPI_printf("WriteRegister(0x%04x,0x%04x)",regAddr,regData);

	WV_ASSERT_RET(HI_UNF_SPI_GetAttr(dev, &attr));
	/*
  HIS_SPI_printf("attr.enDev[%d]",attr.enDev);
  HIS_SPI_printf("attr.csCfg[%d]",attr.csCfg);
  HIS_SPI_printf("attr.u32Baud[%d]",attr.u32Baud);
  HIS_SPI_printf("attr.enFrf[%d]",attr.enFrf);
  HIS_SPI_printf("attr.u32Dss[%d]",attr.u32Dss);
  HIS_SPI_printf("attr.enBigend[%d]",attr.enBigend);
  HIS_SPI_printf("attr.enSph[%d]",attr.unExtAttr.stMoto.enSph);
  HIS_SPI_printf("attr.enSpoh[%d]",attr.unExtAttr.stMoto.enSpo);
  */
	WV_CMD_Register("get", "fpga", "spi bus read fpga", HIS_SPI_GetFpga);
	WV_CMD_Register("set", "fpga", "spi bus write fpga", HIS_SPI_SetFpga);
	WV_CMD_Register("test", "fpga", "spi bus test fpga", HIS_SPI_TestFpga);
	WV_CMD_Register("set", "fpga_param", "spi bus set fpga parameter", HIS_SPI_SetParameter);

	//
	WV_CMD_Register("set", "spi", "spi bus write fpga1 sigle", HIS_SPI_CMDWrite);

	WV_CMD_Register("get", "spi", "spi bus read fpga1 sigle", HIS_SPI_CMDRead);
	WV_CMD_Register("set", "spif", "spi bus read fpga1 sigle",HIS_SPI_CMDWriteFromFile);

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
