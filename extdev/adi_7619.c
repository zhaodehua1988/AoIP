#include "adi_7619.h"
#include "PCA9548A.h"
#include "his_iic.h"
#define EXT_ADV7619_DEBUG_MODE

#ifdef EXT_ADV7619_DEBUG_MODE
#define ADV_7619_printf(...)                \
	do                                      \
	{                                       \
		printf("\n\rADV7619:" __VA_ARGS__); \
		fflush(stdout);                     \
	} while (0)
#else
#define ADV_7619_printf(...)
#endif

#define   ADV_IIC_PORT           2

#define ADV_IO_SLA 0x98
#define ADV_CEC_SLA 0x80
#define ADV_INFO_SLA 0x7C
#define ADV_DPLL_SLA 0x4C
#define ADV_KSV_SLA 0x64
#define ADV_EDID_SLA 0x6C
#define ADV_HDMI_SLA 0x68
#define ADV_CP_SLA 0x44

typedef struct ADV7619_SATUS_S
{
	WV_U8 cableInset;
	WV_U8 tmdsLock;
	WV_U8 deLock;
	WV_U8 spa;
	WV_U8 edidEna;
	WV_U32 tmdsFrq;
	WV_U32 tmdsFrqFract;
	WV_U8 hsyncPola;
	WV_U8 vsyncPola;
	WV_U8 lock;
	WV_U8 mode;
	WV_U8 interlaced;
	WV_U32 lineAct;
	WV_U32 lineTotal;
	WV_U32 hFronPorch;
	WV_U32 hBackPorch;
	WV_U32 hPulseWidth;
	WV_U32 vField0Total;
	WV_U32 vField0Act;
	WV_U32 vFrontPorch;
	WV_U32 vBackPorch;
	WV_U32 vPulseWidth;
} ADV7619_SATUS_S;

typedef struct ADV7619_REG_TABLE
{
	WV_U8 port;
	WV_U8 flag;
	WV_U8 reg;
	WV_U8 data;

} ADV7619_REG_TABLE;

ADV7619_REG_TABLE adv7619_default[] =
	{

		{ADV_IO_SLA, 1, 0xF4, ADV_CEC_SLA},  //set the addr for the CEC-------80
		{ADV_IO_SLA, 1, 0xF5, ADV_INFO_SLA}, //set the addr for the INFOFRAME-7C
		{ADV_IO_SLA, 1, 0xF8, ADV_DPLL_SLA}, //set the addr for the DPLL-----4C
		{ADV_IO_SLA, 1, 0xF9, ADV_KSV_SLA},  //set the addr for the KSV-------64
		{ADV_IO_SLA, 1, 0xFA, ADV_EDID_SLA}, //set the addr for the EDID-----6C
		{ADV_IO_SLA, 1, 0xFB, ADV_HDMI_SLA}, //set the addr for the HDMI-----68
		{ADV_IO_SLA, 1, 0xFD, ADV_CP_SLA},   //set the addr for the CP---------44
		{ADV_HDMI_SLA, 1, 0xC0, 0x03},		 //ADI Recommended Write
		{ADV_HDMI_SLA, 1, 0x00, 0x08},		 //set port B as a background
		{ADV_IO_SLA, 1, 0x01, 0x06},		 //Prim_Mode =110b HDMI-GR
		{ADV_IO_SLA, 1, 0x02, 0xF2},		 //Auto CSC, RGB out, Set op_656 bit
		{ADV_IO_SLA, 1, 0x03, 0x54},		 //gwj:54); //2x24 bit SDR 444 interleaved mode 0
		{ADV_IO_SLA, 1, 0x04, 0x60},		 //27M crystal
		{ADV_IO_SLA, 1, 0x05, 0x28},		 //AV Codes Off
		{ADV_IO_SLA, 1, 0x06, 0xA0},		 //No inversion on VS,HS LLC pins,Negative polarity of HS VS
		{ADV_IO_SLA, 1, 0x0C, 0x42},		 //Power up part
		{ADV_IO_SLA, 1, 0x14, 0x6a},		 //7f);//GWJ A); //driver //gwj:通道1测试数据6e
		{ADV_IO_SLA, 1, 0x15, 0xa0},		 //gwj:音频输出时能，B0); //Disable Tristate of Pins-----A0/80
		{ADV_IO_SLA, 1, 0x19, 0x80},		 //gwj:93); //LLC DLL phase-80=normal-90=invert
		{ADV_IO_SLA, 1, 0x33, 0x40},		 //LLC DLL MUX enable
		//{ADV_IO_SLA  , 1,0xBF,0x00},  //2x24 bit SDR 444 interleaved mode 0
		{ADV_IO_SLA, 1, 0xDD, 0xA0},   //LLC Half frequency for 2x24 bit SDR 444 interleaved mode
		{ADV_DPLL_SLA, 1, 0xB5, 0x03}, //Setting MCLK to 512Fs
		{ADV_DPLL_SLA, 1, 0xC3, 0x80}, //ADI recommended writes for 2x24 bit (HiFreq)
		{ADV_DPLL_SLA, 1, 0xCF, 0x03}, //ADI recommended writes for 2x24 bit(HiFreq)
		{ADV_HDMI_SLA, 1, 0xC0, 0x03}, //ADI Required write
		{ADV_HDMI_SLA, 1, 0x00, 0x08}, //Set HDMI Input Port A (BG_MEAS_PORT_SEL = 001b)
		{ADV_HDMI_SLA, 1, 0x02, 0x03}, //ALL BG Ports enabled
		{ADV_HDMI_SLA, 1, 0x03, 0x98}, //ADI Required Write-------AUDIO
		{ADV_HDMI_SLA, 1, 0x10, 0xA5}, //ADI Required Write-------25-NONE
		{ADV_HDMI_SLA, 1, 0x45, 0x04}, //ADI Required Write--------NONE
		{ADV_HDMI_SLA, 1, 0x97, 0xC0}, //ADI Required Write--------NONE
		{ADV_HDMI_SLA, 1, 0x3D, 0x10}, //ADI Required Write--------NONE
		{ADV_HDMI_SLA, 1, 0x3E, 0x69}, //ADI reccommended writes---NONE
		{ADV_HDMI_SLA, 1, 0x3F, 0x46}, //ADI reccommended writes---NONE
		{ADV_HDMI_SLA, 1, 0x4E, 0xFE}, //ADI reccommended writes---NONE
		{ADV_HDMI_SLA, 1, 0x4F, 0x08}, //ADI reccommended writes---NONE
		{ADV_HDMI_SLA, 1, 0x50, 0x00}, //ADI Recommended Write
		{ADV_HDMI_SLA, 1, 0x57, 0xA3}, //ADI Recommended Write
		{ADV_HDMI_SLA, 1, 0x58, 0x07}, //ADI Recommended Write
		{ADV_HDMI_SLA, 1, 0x6C, 0x62}, //enable HPA automatic MODE,&OUTPUT AFTER +5V DETECTION 0.4S
		{ADV_HDMI_SLA, 1, 0x83, 0xFC}, //Enable clock terminators for port A & B
		{ADV_HDMI_SLA, 1, 0x84, 0x03}, //FP MODE
		{ADV_HDMI_SLA, 1, 0x85, 0x10}, //ADI Recommended Write
		{ADV_HDMI_SLA, 1, 0x86, 0x9B}, //ADI Recommended Write
		{ADV_HDMI_SLA, 1, 0x89, 0x03}, //HF Gain
		{ADV_HDMI_SLA, 1, 0x9B, 0x03}, //ADI Recommended Write
		{ADV_HDMI_SLA, 1, 0x93, 0x03}, //ADI Recommended Write
		{ADV_HDMI_SLA, 1, 0x5A, 0x80},  //ADI Recommended Write
		{ADV_IO_SLA,1,0x19,0x90}
};

/****************************************************************************************************

WV_S32  ADV_7619_SetConf();

****************************************************************************************************/
WV_S32 ADV_7619_SetConf()
{
//WV_S32 PCA9548_IIC_Read(WV_U8 id,WV_U8 devAddr,WV_U8 regAddr,WV_U8 *data);
//WV_S32 PCA9548_IIC_Write(WV_U8 id,WV_U8 devAddr,WV_U8 regAddr,WV_U8 data);

	WV_S32 i, num;
	WV_U8 devAddr, regAddr, data;

	num = sizeof(adv7619_default) / sizeof(adv7619_default[0]);
	WV_printf("adv 7619 set conf start.....\n");
	//PCA9548_SwitchToBus(PCA9548A_IIC_SWID_7619);
	for (i = 0; i < num; i++)
	{
		if (adv7619_default[i].flag == 0)
		{
			continue;
		}
		devAddr = adv7619_default[i].port;
		regAddr = adv7619_default[i].reg;
		data = adv7619_default[i].data;

		//WV_CHECK_RET(PCA9548_IIC_Write(PCA9548A_IIC_SWID_7619,devAddr,regAddr, data));
		PCA9548_IIC_Write(PCA9548A_IIC_SWID_7619,devAddr,regAddr,data);
		usleep(200000);
	}

	WV_printf("adv 7619 set conf end.....\n");
	return WV_SOK;
}

/****************************************************************************************************

WV_S32  ADV_7619_Check(ADV7619_SATUS_S * pStat)

****************************************************************************************************/
#if 1
WV_S32 ADV_7619_Check(ADV7619_SATUS_S *pStat)
{
	WV_U8 data;
	////PCA9548_SwitchToBus(PCA9548A_IIC_SWID_7619);

	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_IO_SLA, 0x6F, &data); //0 means no cable and 1 means cable detected on PORT A
	pStat->cableInset = data;
	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_IO_SLA, 0x21, &data); //bit3=0 means SPA=0,bit3=1 means SPA=1
	pStat->spa = (data >> 3) & 0x1;

	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_IO_SLA, 0x6A, &data); //bit6=PORT A locked clock,bit4=TMDS clock detected on PORT A,bit1=VSYNC is valid.bit0=DE is locked
	pStat->deLock = data & 0x1;
	pStat->tmdsLock = data;
	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_KSV_SLA, 0x74, &data); //bit1=1 means PORT B can access internal EDID,bit0=1 means PORT A
	pStat->edidEna = data & 0x1;

	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x51, &data); //0X51&0X52[8]=TMDS frequency
	if (data < 70)
		PCA9548_IIC_Write(PCA9548A_IIC_SWID_7619, ADV_IO_SLA, 0x14, 0x55);
	else if (data > 100)
		PCA9548_IIC_Write(PCA9548A_IIC_SWID_7619, ADV_IO_SLA, 0x14, 0x7F);
	else
		PCA9548_IIC_Write(PCA9548A_IIC_SWID_7619, ADV_IO_SLA, 0x14, 0x6A);

	pStat->tmdsFrq = data;

	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x52, &data); //0x52[6:0]=TMDS fractional frequency measurement in 1/128MHz
	pStat->tmdsFrq = (pStat->tmdsFrq << 1) | ((data >> 7) & 0x1);

	pStat->tmdsFrqFract = data & 0x3f;

	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x05, &data); //bit0=HDMI or DVI,bit1=HDCP,bit2=HSync active low,bit3=V active low
	pStat->mode = (data >> 7) & 0x1;
	pStat->hsyncPola = (data >> 2) & 0x1;
	pStat->vsyncPola = (data >> 3) & 0x1;
	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x07, &data); //bit7=V locked,bit5=DE locked
	pStat->lock = ((data >> 5) & 0x1) | ((data >> 6) & 0x2);
	pStat->lineAct = (data & 0x0f) << 8;
	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x08, &data); //0x7[4:0]&0x8[7:0]=total number of active pixel per line
	pStat->lineAct |= data & 0xff;

	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x1E, &data);
	pStat->lineTotal = (data & 0x1f) << 8;
	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x1F, &data); //0x1E[5:0]&0x1F[7:0]=total number of pixel per line
	pStat->lineTotal |= (data & 0xff);

	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x20, &data);
	pStat->hFronPorch = (data & 0x0f) << 8;
	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x21, &data); //0x20[4:0]&0x21[7:0]=total number of pixel in the front porch
	pStat->hFronPorch |= (data & 0xff);

	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x22, &data);
	pStat->hPulseWidth = (data & 0x0f) << 8;
	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x23, &data); //0x22[4:0]&0x23[7:0]=total number of pixel in the hsync pulse
	pStat->hPulseWidth |= data;

	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x24, &data);
	pStat->hBackPorch = (data & 0x0f) << 8;
	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x25, &data); //0x24[4:0]&0x25[7:0]=total number of pixel in the back porch
	pStat->hBackPorch |= data;

	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x26, &data);
	pStat->vField0Total = (data & 0x1f) << 7;
	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x27, &data); //0x26[5:0]&0x27[7:0]=total number of line * 2
	pStat->vField0Total |= data >> 1;

	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x09, &data);
	pStat->vField0Act = (data & 0x0f) << 8;
	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x0A, &data); //0x09[4:0]&0x0A[7:0]=total number of active line in field0
	pStat->vField0Act |= data;

	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x2A, &data);
	pStat->vFrontPorch = (data & 0x1f) << 7;
	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x2B, &data); //0x2A[5:0]&0x2B[7:0]=total number of Vsync front porch * 2
	pStat->vFrontPorch |= data >> 1;

	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x2E, &data);
	pStat->vPulseWidth = (data & 0x1f) << 7;
	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x2F, &data); //0x2E[5:0]&0x2F[7:0]=total number of line in Vsync * 2
	pStat->vPulseWidth |= data >> 1;

	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x32, &data);
	pStat->vBackPorch = (data & 0x1f) << 7;
	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x33, &data); //0x32[5:0]&0x33[7:0]=total number of Vsync back porch * 2
	pStat->vBackPorch |= data >> 1;

	PCA9548_IIC_Read(PCA9548A_IIC_SWID_7619, ADV_HDMI_SLA, 0x0B, &data); //bit5=1 means interlaced input,0 means progressive input
	pStat->interlaced = (data >> 5) & 0x1;
	return WV_SOK;
}
#endif
/****************************************************************************

WV_S32 ADV_7619_GetHdmi(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 ADV_7619_GetHdmi(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{

	ADV7619_SATUS_S stat;
	ADV_7619_Check(&stat);
	prfBuff += sprintf(prfBuff, " cableInset   : %d\r\n", stat.cableInset);
	prfBuff += sprintf(prfBuff, " spa          : %d\r\n", stat.spa);

	prfBuff += sprintf(prfBuff, " deLock       : %d\r\n", stat.deLock);
	prfBuff += sprintf(prfBuff, " edidEna      : %d\r\n", stat.edidEna);
	prfBuff += sprintf(prfBuff, " tmdsFrq      : %d\r\n", stat.tmdsFrq);
	prfBuff += sprintf(prfBuff, " tmdsFrqFract : %d\r\n", stat.tmdsFrqFract);
	prfBuff += sprintf(prfBuff, " hsyncPola    : %d\r\n", stat.hsyncPola);
	prfBuff += sprintf(prfBuff, " vsyncPola    : %d\r\n", stat.vsyncPola);
	prfBuff += sprintf(prfBuff, " lock         : %d\r\n", stat.lock);
	prfBuff += sprintf(prfBuff, " mode         : %d\r\n", stat.mode);
	prfBuff += sprintf(prfBuff, " interlaced   : %d\r\n", stat.interlaced);
	prfBuff += sprintf(prfBuff, " lineAct      : %d\r\n", stat.lineAct);
	prfBuff += sprintf(prfBuff, " lineTotal    : %d\r\n", stat.lineTotal);
	prfBuff += sprintf(prfBuff, " hFronPorch   : %d\r\n", stat.hFronPorch);
	prfBuff += sprintf(prfBuff, " hBackPorch   : %d\r\n", stat.hBackPorch);
	prfBuff += sprintf(prfBuff, " hPulseWidth   : %d\r\n", stat.hPulseWidth);
	prfBuff += sprintf(prfBuff, " vField0Total : %d\r\n", stat.vField0Total);
	prfBuff += sprintf(prfBuff, " vField0Act   : %d\r\n", stat.vField0Act);
	prfBuff += sprintf(prfBuff, " vFrontPorch  : %d\r\n", stat.vFrontPorch);
	prfBuff += sprintf(prfBuff, " vBackPorch   : %d\r\n", stat.vBackPorch);
	prfBuff += sprintf(prfBuff, " vPulseWidth  : %d\r\n", stat.vPulseWidth);
	return WV_SOK;
}

/****************************************************************************************************

WV_S32  ADV_7619_Init();

****************************************************************************************************/
WV_S32 ADV_7619_Init()
{
	ADV_7619_SetConf();
	WV_CMD_Register("get", "hdmi", "hdmi satuts read", ADV_7619_GetHdmi);
	return WV_SOK;
}

/****************************************************************************************************

WV_S32  ADV_7619_DeInit();

****************************************************************************************************/
WV_S32 ADV_7619_DeInit()
{
	return WV_SOK;
}
