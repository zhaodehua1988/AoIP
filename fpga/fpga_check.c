#include "fpga_check.h"
#include "fpga_conf.h"
#include "his_spi.h"
#include "fpga_volume.h"

#define _D_FPGA_CHECK_INTERVAL_TIME (10)   //默认5秒中画面没有变化旧说明静帧了
#define _D_FPGA_CHECK_R_SUM_THRESHOLD (4096)
#define _D_FPGA_CHECK_DEFAULT_MULTIPLE (3) //如果有音量则加倍检测静帧的时间，默认3倍时间
typedef struct _S_FPGA_CHECK_DATA
{
    WV_U32 r_sum;
    WV_U32 checkTimes;      //记录检测到几次r_sum相同，如果超过_D_FPGA_CHECK_INTERVAL_TIME，则说明静帧了
    WV_U32 multiple;        //这个值跟窗口有没有音量有关系，如果没有音量则时原值，如果有音量则乘3倍
    WV_U32 checkTotalTimes; //总共检测多少次，
    WV_S32 isFreeze;        //1代表静帧 0代表没有静帧
    WV_S32 noSignal;        //1代表无信号 0代表有信号
} _S_FPGA_CHECK_DATA;

typedef struct _S_FPGA_CHECK_DEV
{
    WV_THR_HNDL_T thrHndl;
    WV_U32 open;
    WV_U32 close;
    _S_FPGA_CHECK_DATA checkData[FPGA_CONF_ETHNUM_D][FPGA_CONF_SRCNUM_D];
} _S_FPGA_CHECK_DEV;
static _S_FPGA_CHECK_DEV gFgpaCheckDev;
static WV_S32 gFpgaCheckThresholdVal; //颜色阈值可以设置

/***************************************************
 * void FPGA_CHECK_SetCheckTimeValue(WV_U32 time_s)
 * 设置检测时间,单位时秒
 * ************************************************/
void FPGA_CHECK_SetCheckTimeValue(WV_U32 time_s)
{
    WV_S32 i,j;
    for(i=0;i<FPGA_CONF_ETHNUM_D;i++)
    {
        for(j=0;j<FPGA_CONF_SRCNUM_D;j++)
        {
            gFgpaCheckDev.checkData[i][j].checkTotalTimes = time_s * 10;
        }
    }
}
/****************************************************
 * WV_S32 FPGA_CHECK_GetWinFreeze(WV_U16 ethID,WV_U16 ipID) 
 * *************************************************/
WV_S32 FPGA_CHECK_GetWinFreeze(WV_U16 ethID, WV_U16 ipID)
{
    if (ethID > 3 || ipID > 3)
        return WV_EFAIL;
    return gFgpaCheckDev.checkData[ethID][ipID].isFreeze;
}
/****************************************************
 * WV_S32 FPGA_Check_NoSignal(WV_U16 ethID, WV_U16 ipID)
 * *************************************************/
WV_S32 FPGA_Check_NoSignal(WV_U16 ethID, WV_U16 ipID)
{
    if (ethID > 3 || ipID > 3)
        return WV_EFAIL;
    return gFgpaCheckDev.checkData[ethID][ipID].noSignal;
}

/****************************************************
 * WV_S32 fpga_check_MultipleTimes(WV_U16 ethID,WV_U16 ipID )
 * 函数说明：检测静帧时间是否加倍（回调函数），音量查询线程调用本接口，可以得知当前是否加倍检测时间
 * 参数说明：
 *    ethID:第几路网卡
 *    ipID :第几个ip地址
 *    vol  :音量和值
 * *************************************************/
void fpga_check_MultipleTimes(WV_U16 ethID,WV_U16 ipID)
{
    WV_U16 vol[_D_FPGA_VOLUME_CHLNUM];
    WV_U32 volsum=0;
    WV_S32 i;
    FPGA_VOLUME_Get(ethID,ipID,vol);
    for(i=0;i<_D_FPGA_VOLUME_CHLNUM;i++)
    {
        volsum+=vol[i];
    }
    if (volsum == 0)
    {
        //没有音量则不加倍，即multiple为1
        gFgpaCheckDev.checkData[ethID][ipID].multiple = 1;

    }
    else
    {
        //有音量则不加倍，即multiple为_D_FPGA_CHECK_DEFAULT_MULTIPLE默认倍数
        gFgpaCheckDev.checkData[ethID][ipID].multiple = _D_FPGA_CHECK_DEFAULT_MULTIPLE;
    }
}


/****************************************************
 * void fpga_check_winStream()
 * **************************************************/
void fpga_check_winStream()
{
    WV_U16 regAddr, data = 0;
     WV_S32 i, j;
     regAddr = 0x18;

     HIS_SPI_FpgaRd(regAddr, &data);
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
         for (j = 0; j < FPGA_CONF_SRCNUM_D; j++)
         {

             if ((1 << (j + i * 4) & data) != 0)
             {
                 gFgpaCheckDev.checkData[i][j].noSignal = 0;
             }
             else
             {
                 gFgpaCheckDev.checkData[i][j].noSignal = 1;
             }
        }
    }
}

/******************************************************
 * void fpga_check_winFreeze()
 * 函数说明：检测窗口静帧状态
 * ****************************************************/
void fpga_check_winFreeze()
{

    WV_U16 baseAddr, regAddr, data = 0, id;
    WV_U32 video_r_sum = 0;
    WV_U32 DValue; //r sum 的差值
    WV_S32 i, j;

    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        for (j = 0; j < FPGA_CONF_SRCNUM_D; j++)
        {
            fpga_check_MultipleTimes(i,j);
            id = j + i * 4;
            baseAddr = 0x500;
            regAddr = 0x5;

            baseAddr = ((baseAddr >> 4) | id) << 4;
            regAddr |= baseAddr;
            HIS_SPI_FpgaRd(regAddr, &data);
            
            video_r_sum |= data << 16;
            HIS_SPI_FpgaRd(regAddr + 1, &data);
            video_r_sum |= data;
            
            if (video_r_sum >= gFgpaCheckDev.checkData[i][j].r_sum)
            {
                DValue = video_r_sum - gFgpaCheckDev.checkData[i][j].r_sum;
            }
            else
            {
                DValue = gFgpaCheckDev.checkData[i][j].r_sum - video_r_sum;
            }
            
            if (DValue < gFpgaCheckThresholdVal) //如果rSum差值 小于阈值，则说明图像有可能静帧了，
            {
                //r_sum不更新，checkTimes加1，

                gFgpaCheckDev.checkData[i][j].checkTimes++;
                if (gFgpaCheckDev.checkData[i][j].checkTimes > gFgpaCheckDev.checkData[i][j].checkTotalTimes * gFgpaCheckDev.checkData[i][j].multiple )
                {
                    gFgpaCheckDev.checkData[i][j].checkTimes = gFgpaCheckDev.checkData[i][j].checkTotalTimes * gFgpaCheckDev.checkData[i][j].multiple ;
                    gFgpaCheckDev.checkData[i][j].isFreeze = 1;
                }
            }
            else
            { //否则没有静帧,r_sum更新，标志清零，checkTimes 清零
                gFgpaCheckDev.checkData[i][j].r_sum = video_r_sum;
                gFgpaCheckDev.checkData[i][j].isFreeze = 0;
                gFgpaCheckDev.checkData[i][j].checkTimes = 0;
            }
        }
    }
}

/**************************************************************************
 * void *fpga_read_volume(void *prm)
 * ************************************************************************/
void *fpga_check_proc(void *prm)
{
    _S_FPGA_CHECK_DEV *pDev = (_S_FPGA_CHECK_DEV *)prm;
    pDev->open = 1;
    pDev->close = 0;

    while (pDev->open == 1)
    {
        fpga_check_winFreeze();
        fpga_check_winStream();
        usleep(100000);
    }
    pDev->open = 0;
    pDev->close = 1;
    return NULL;
}
/******************************************************
 * void FGPA_CHECK_Open()
 * ****************************************************/
void FGPA_CHECK_Open()
{
    WV_S32 i, j;

    memset(&gFgpaCheckDev, 0, sizeof(gFgpaCheckDev));

    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        for (j = 0; j < FPGA_CONF_SRCNUM_D; j++)
        {
            gFgpaCheckDev.checkData[i][j].multiple = 1;
            gFgpaCheckDev.checkData[i][j].checkTotalTimes = _D_FPGA_CHECK_INTERVAL_TIME*10;
        }
    }
    gFpgaCheckThresholdVal = _D_FPGA_CHECK_R_SUM_THRESHOLD;
    WV_THR_Create(&gFgpaCheckDev.thrHndl, fpga_check_proc, WV_THR_PRI_DEFAULT, WV_THR_STACK_SIZE_DEFAULT, &gFgpaCheckDev);
}
/******************************************************
 * void FPGA_CHECK_Close()
 * ****************************************************/
void FPGA_CHECK_Close()
{
    if (gFgpaCheckDev.open == 1)
    {
        gFgpaCheckDev.open = 0;
        
        while (gFgpaCheckDev.close == 1)
            ;
        WV_THR_Destroy(&gFgpaCheckDev.thrHndl);


    }
    //WV_printf("FPGA_CHECK_Close end\n");
}