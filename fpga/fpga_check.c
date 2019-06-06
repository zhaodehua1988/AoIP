#include "fpga_check.h"
#include "fpga_conf.h"
#include "his_spi.h"
#include "fpga_volume.h"

#define _D_FPGA_CHECK_INTERVAL_TIME (10)          //默认5秒中画面没有变化旧说明静帧了
#define _D_FPGA_CHECK_R_SUM_THRESHOLD_PERCENT (1) //画面变化小于千分之1的说明 画面没有变化
#define _D_FPGA_CHECK_DEFAULT_MULTIPLE (3)        //如果有音量则加倍检测静帧的时间，默认3倍时间

#define _D_FPGA_CHECK_VOLUME_TIME (10) //音量异常检测时间
#define _D_FPGA_CHECK_VOLUME_LOW_VAL (15)
#define _D_FPGA_CHECK_VOLUME_EXCESSIVE (200)

#define _D_FPGA_CHECK_VOLUME_AUDIO_CHANNEL_NUM (8)

#define _D_FPGA_CHECK_VOLUME_THRESHOLD_LOW    (-35.0)
#define _D_FPGA_CHECK_VOLUME_THRESHOLD_HIGHT  (-5.0)
#define _D_FPGA_CHECK_VOLUME_THRESHOLD_MUTE   (-50.0)


typedef struct _S_FPGA_VOLUME_STATUS
{
    WV_U16 volChl;
    WV_U16 muteCheckTimes;          //静音检测次数时间
    WV_U16 muteCheckTotalTimes;     //静音检测总次数
    float muteThreshold_dB;           //静音门限值
    
    WV_U16 higthCheckTimes;         //音量过高检测次数
    WV_U16 higthCheckTotalTimes;    //音量过高检测次数
    float hightThreshold_dB;          //音量过高门限值

    WV_U16 lowCheckTimes;           //音量过低检测次数计数
    WV_U16 lowCheckTotalTimes;      //音量过低检测总数
    float lowThreshold_dB;            //音量过低门限值
    float vol_dB[_D_FPGA_CHECK_VOLUME_AUDIO_CHANNEL_NUM];
    WV_U16 status;          //当前通道音量状态
} _S_FPGA_CHECK_AUDIO_STATUS;

typedef struct _S_FPGA_CHECK_VIDEO_DATA
{
    WV_U32 r_sum;
    WV_U32 checkTimes;      //记录检测到几次r_sum相同，如果超过_D_FPGA_CHECK_INTERVAL_TIME，则说明静帧了
    WV_U16 multipleAudio;   //接收上位机设置音频加倍的倍数
    WV_U16 thousandth;      //千分比rsum的变化范围小于这个千分比，说明图像静帧了
    WV_U16 multiple;        //这个值跟窗口有没有音量有关系，如果没有音量则时原值，如果有音量为multipleAudio的值
    WV_U32 checkTotalTimes; //总共检测多少次，
    WV_S32 isFreeze;        //1代表静帧 0代表没有静帧
    WV_S32 noSignal;        //1代表无信号 0代表有信号
} _S_FPGA_CHECK_VIDEO_DATA;

typedef struct _S_FPGA_CHECK_DEV
{
    WV_THR_HNDL_T thrHndl;
    WV_U32 open;
    WV_U32 close;
    _S_FPGA_CHECK_VIDEO_DATA videoData[FPGA_CONF_ETHNUM_D][FPGA_CONF_SRCNUM_D];     //视频状态                                               //
    _S_FPGA_CHECK_AUDIO_STATUS audioStatus[FPGA_CONF_ETHNUM_D][FPGA_CONF_SRCNUM_D]; //音频状态

} _S_FPGA_CHECK_DEV;

static _S_FPGA_CHECK_DEV gFgpaCheckDev;


/***************************************************
 * void FPGA_CHECK_SetAudioMuteCheck(WV_U16 ethID, WV_U16 ipID,float threshold,WV_U16 time_s)
 * 设置静音的门限值和检测音频丢失的时间
 * ************************************************/
void FPGA_CHECK_SetAudioMuteCheck(WV_U16 ethID, WV_U16 ipID,float threshold,WV_U16 time_s)
{
    gFgpaCheckDev.audioStatus[ethID][ipID].muteThreshold_dB = threshold;
    gFgpaCheckDev.audioStatus[ethID][ipID].muteCheckTotalTimes = time_s*10;
}

/***************************************************
 * void FPGA_CHECK_SetAudioMuteheck(WV_U16 ethID, WV_U16 ipID,float threshold,WV_U16 time_s)
 * 设置静音的门限值和检测音频过高的时间
 * ************************************************/
void FPGA_CHECK_SetAudioHightCheck(WV_U16 ethID, WV_U16 ipID,float threshold,WV_U16 time_s)
{
    //WV_printf("set[%d][%d] Hight(%f dB) times=%d s\n",ethID,ipID,threshold,time_s);
    gFgpaCheckDev.audioStatus[ethID][ipID].hightThreshold_dB = threshold;
    gFgpaCheckDev.audioStatus[ethID][ipID].higthCheckTotalTimes = time_s*10;
}


/***************************************************
 * void FPGA_CHECK_SetAudioLowCheck(WV_U16 ethID, WV_U16 ipID,float threshold,WV_U16 time_s)
 * 设置静音的门限值和检测音频过低的时间
 * ************************************************/
void FPGA_CHECK_SetAudioLowCheck(WV_U16 ethID, WV_U16 ipID,float threshold,WV_U16 time_s)
{
    gFgpaCheckDev.audioStatus[ethID][ipID].lowThreshold_dB = threshold;
    gFgpaCheckDev.audioStatus[ethID][ipID].lowCheckTotalTimes = time_s*10;
}

/***************************************************
 * void FPGA_CHECK_SetCheckFreezeTimeValue(WV_U16 ethID, WV_U16 ipID, WV_U16 thousandth,WV_U32 time_s,WV_U16 mutltiple)
 * 静帧检测时间和结合音频的倍数
 * ************************************************/
void FPGA_CHECK_SetCheckFreezeTimeValue(WV_U16 ethID, WV_U16 ipID, WV_U16 thousandth,WV_U32 time_s,WV_U16 mutltiple)
{
    if(time_s == 0)  time_s = 1;
    
    if(mutltiple == 0) mutltiple = 1;

    gFgpaCheckDev.videoData[ethID][ipID].thousandth = thousandth;
    gFgpaCheckDev.videoData[ethID][ipID].checkTotalTimes = time_s * 10;
    gFgpaCheckDev.videoData[ethID][ipID].multipleAudio = mutltiple;
}

/****************************************************
 * WV_U16 FPGA_CHECK_AudioStatus(WV_U16 ethID, WV_U16 ipID)
 * 返回当前音频的状态
 * *************************************************/
WV_U16 FPGA_CHECK_AudioStatus(WV_U16 ethID, WV_U16 ipID)
{
    if (ethID > 3 || ipID > 3)
        return WV_EFAIL;
    return gFgpaCheckDev.audioStatus[ethID][ipID].status;
}

/****************************************************
 * WV_S32 FPGA_CHECK_GetWinFreeze(WV_U16 ethID,WV_U16 ipID) 
 * *************************************************/
WV_S32 FPGA_CHECK_GetWinFreeze(WV_U16 ethID, WV_U16 ipID)
{
    if (ethID > 3 || ipID > 3)
        return WV_EFAIL;
    return gFgpaCheckDev.videoData[ethID][ipID].isFreeze;
}
/****************************************************
 * WV_S32 FPGA_Check_NoSignal(WV_U16 ethID, WV_U16 ipID)
 * *************************************************/
WV_S32 FPGA_Check_NoSignal(WV_U16 ethID, WV_U16 ipID)
{
    if (ethID > 3 || ipID > 3)
        return WV_EFAIL;
    return gFgpaCheckDev.videoData[ethID][ipID].noSignal;
}

/****************************************************
 * WV_S32 fpga_check_MultipleTimes(WV_U16 ethID,WV_U16 ipID )
 * 函数说明：检测静帧时间是否加倍,音量查询线程调用本接口，可以得知当前是否加倍检测时间
 * 参数说明：
 *    ethID:第几路网卡
 *    ipID :第几个ip地址
 * *************************************************/
void fpga_check_MultipleTimes(WV_U16 ethID, WV_U16 ipID)
{

    if (gFgpaCheckDev.audioStatus[ethID][ipID].status == VOLUME_MUTE){ 
        //没有音量则不加倍，即multiple为1
        gFgpaCheckDev.videoData[ethID][ipID].multiple = 1;
    }
    else
    {
        //有音量则不加倍，即multiple为_D_FPGA_CHECK_DEFAULT_MULTIPLE默认倍数
        gFgpaCheckDev.videoData[ethID][ipID].multiple = gFgpaCheckDev.videoData[ethID][ipID].multipleAudio;
    }
}
/****************************************************
 * void fpga_check_auidoGetStatus(_S_FPGA_CHECK_AUDIO_STATUS *pStatus,WV_U16 vol[])
 * *************************************************/
void fpga_check_auidoGetStatus(_S_FPGA_CHECK_AUDIO_STATUS *pStatus)
{
    WV_S32 i;
    WV_U8 muteNum = 0;
    WV_U8 lowNum = 0; 
    WV_U8 hightNum=0;
    for(i=0;i<_D_FPGA_CHECK_VOLUME_AUDIO_CHANNEL_NUM;i++)
    {
        //WV_printf("vol = %f db,hight = %f ,low=%f mute=%f\n",pStatus->vol_dB[i],pStatus->hightThreshold_dB,pStatus->lowThreshold_dB,pStatus->muteThreshold_dB);
        if(pStatus->vol_dB[i] >= pStatus->hightThreshold_dB){            //音量值有一个超过最好的门限值(vol > higth)，则说明音量过高
           hightNum ++;
            break;
        }
        else if(pStatus->vol_dB[i] <= pStatus->muteThreshold_dB){       //音量如果小于静音的值，则说明当前声道没有音频 muteNum+1,lowNum+1;
            //WV_printf("mute vol=%f,mute=%f\n",pStatus->vol_dB[i],pStatus->muteThreshold_dB);
            muteNum ++;
            lowNum ++;
        }
        else if(pStatus->vol_dB[i] <= pStatus->lowThreshold_dB){        //音量在low范围内则,lowNum +1 (vol <= low)
            //WV_printf("low vol=%f,low=%f\n",pStatus->vol_dB[i],pStatus->lowThreshold_dB);
            lowNum ++;
        }
    }
    if(hightNum > 0 ){
        pStatus->higthCheckTimes++;
        pStatus->lowCheckTimes = 0;
        pStatus->muteCheckTimes = 0;
        //WV_printf("higth vol= %f,hight=%f hitimes=%d totaltimes=%d\n",pStatus->vol_dB[i],pStatus->hightThreshold_dB,pStatus->higthCheckTimes,pStatus->higthCheckTotalTimes);
            
    }
    else if(muteNum == _D_FPGA_CHECK_VOLUME_AUDIO_CHANNEL_NUM){ //如果时静音状态，那么肯定时音量过低状态
        pStatus->muteCheckTimes++;
        pStatus->lowCheckTimes++;
        pStatus->higthCheckTimes = 0;
        //WV_printf("mute vol= %f,mute=%f times=%d totaltimes=%d\n",pStatus->vol_dB[i],pStatus->muteThreshold_dB,pStatus->muteCheckTimes,pStatus->muteCheckTotalTimes);    
    }
    else if(lowNum == _D_FPGA_CHECK_VOLUME_AUDIO_CHANNEL_NUM){
        pStatus->lowCheckTimes++;
        pStatus->muteCheckTimes = 0;
        pStatus->higthCheckTimes = 0;
        //WV_printf("low vol= %f,low=%f times=%d totaltimes=%d\n",pStatus->vol_dB[i],pStatus->lowThreshold_dB,pStatus->lowCheckTimes,pStatus->lowCheckTotalTimes);    

    }else{
        pStatus->lowCheckTimes = 0;
        pStatus->muteCheckTimes = 0;
        pStatus->higthCheckTimes = 0;
    }
      
}
/****************************************************
 * void fpga_check_auidoStatus()
 * **************************************************/
void fpga_check_auidoStatus(){
    
    WV_S32 i,j;
    
    for(i=0;i<FPGA_CONF_ETHNUM_D;i++){
        for(j=0;j<FPGA_CONF_SRCNUM_D;j++){

            FPGA_VOLUME_Get_dB(i, j, gFgpaCheckDev.audioStatus[i][j].vol_dB);
            fpga_check_auidoGetStatus(&gFgpaCheckDev.audioStatus[i][j]);
            if(gFgpaCheckDev.audioStatus[i][j].higthCheckTimes >= gFgpaCheckDev.audioStatus[i][j].higthCheckTotalTimes)
            {
                //WV_printf("");
                gFgpaCheckDev.audioStatus[i][j].status = VOLUME_HIGHT;
                gFgpaCheckDev.audioStatus[i][j].higthCheckTimes = gFgpaCheckDev.audioStatus[i][j].higthCheckTotalTimes;
            }
            else if(gFgpaCheckDev.audioStatus[i][j].muteCheckTimes >= gFgpaCheckDev.audioStatus[i][j].muteCheckTotalTimes){

                gFgpaCheckDev.audioStatus[i][j].status = VOLUME_MUTE;
                gFgpaCheckDev.audioStatus[i][j].muteCheckTimes = gFgpaCheckDev.audioStatus[i][j].muteCheckTotalTimes;
                gFgpaCheckDev.audioStatus[i][j].lowCheckTimes = gFgpaCheckDev.audioStatus[i][j].lowCheckTotalTimes;
            }
            else if(gFgpaCheckDev.audioStatus[i][j].lowCheckTimes >= gFgpaCheckDev.audioStatus[i][j].lowCheckTotalTimes){
                gFgpaCheckDev.audioStatus[i][j].status = VOLUME_LOW;
                gFgpaCheckDev.audioStatus[i][j].lowCheckTimes = gFgpaCheckDev.audioStatus[i][j].lowCheckTotalTimes;
            }else{
                gFgpaCheckDev.audioStatus[i][j].status = VOLUME_OK;
            }
            //WV_printf("[%d][%d]vol status = %d \n",i,j,gFgpaCheckDev.audioStatus[i][j].status);
        }
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
                gFgpaCheckDev.videoData[i][j].noSignal = 0;
            }
            else
            {
                gFgpaCheckDev.videoData[i][j].noSignal = 1;
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
            fpga_check_MultipleTimes(i, j);
            id = j + i * 4;
            baseAddr = 0x500;
            regAddr = 0x5;
            WV_U32 checkVal = 0;
            baseAddr = ((baseAddr >> 4) | id) << 4;
            regAddr |= baseAddr;
            HIS_SPI_FpgaRd(regAddr, &data);

            video_r_sum |= data << 16;
            HIS_SPI_FpgaRd(regAddr + 1, &data);
            video_r_sum |= data;

            if (video_r_sum >= gFgpaCheckDev.videoData[i][j].r_sum)
            {
                DValue = video_r_sum - gFgpaCheckDev.videoData[i][j].r_sum;
                checkVal = (video_r_sum * gFgpaCheckDev.videoData[i][j].thousandth) / 1000;
            }
            else
            {
                DValue = gFgpaCheckDev.videoData[i][j].r_sum - video_r_sum;
                checkVal = (gFgpaCheckDev.videoData[i][j].r_sum * gFgpaCheckDev.videoData[i][j].thousandth) / 1000;
            }
            if (DValue < checkVal)
            {
                //WV_printf("[%d][%d]r_sum=%d,Dvalue=%d,checkVal=%d checkTimes=%d\n",i,j,video_r_sum,DValue,checkVal,gFgpaCheckDev.videoData[i][j].checkTimes);
                gFgpaCheckDev.videoData[i][j].checkTimes++;
                if (gFgpaCheckDev.videoData[i][j].checkTimes > (gFgpaCheckDev.videoData[i][j].checkTotalTimes * gFgpaCheckDev.videoData[i][j].multiple))
                {
                    gFgpaCheckDev.videoData[i][j].checkTimes = gFgpaCheckDev.videoData[i][j].checkTotalTimes * gFgpaCheckDev.videoData[i][j].multiple;
                    gFgpaCheckDev.videoData[i][j].isFreeze = 1;
                }
            }
            else
            { //否则没有静帧,r_sum更新，标志清零，checkTimes 清零
                gFgpaCheckDev.videoData[i][j].r_sum = video_r_sum;
                gFgpaCheckDev.videoData[i][j].isFreeze = 0;
                gFgpaCheckDev.videoData[i][j].checkTimes = 0;
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
        fpga_check_auidoStatus();
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
            gFgpaCheckDev.videoData[i][j].multipleAudio = _D_FPGA_CHECK_DEFAULT_MULTIPLE;
            gFgpaCheckDev.videoData[i][j].multiple = _D_FPGA_CHECK_DEFAULT_MULTIPLE;
            gFgpaCheckDev.videoData[i][j].checkTotalTimes = _D_FPGA_CHECK_INTERVAL_TIME * 10;
            gFgpaCheckDev.videoData[i][j].thousandth = _D_FPGA_CHECK_R_SUM_THRESHOLD_PERCENT;      //thousandth
            gFgpaCheckDev.audioStatus[i][j].muteCheckTotalTimes = _D_FPGA_CHECK_INTERVAL_TIME * 10;
            gFgpaCheckDev.audioStatus[i][j].higthCheckTotalTimes = _D_FPGA_CHECK_INTERVAL_TIME * 10;
            gFgpaCheckDev.audioStatus[i][j].lowCheckTotalTimes = _D_FPGA_CHECK_INTERVAL_TIME * 10;
            gFgpaCheckDev.audioStatus[i][j].lowThreshold_dB = _D_FPGA_CHECK_VOLUME_THRESHOLD_LOW;     //-30dB
            gFgpaCheckDev.audioStatus[i][j].hightThreshold_dB = _D_FPGA_CHECK_VOLUME_THRESHOLD_HIGHT; //-1.0dB
            gFgpaCheckDev.audioStatus[i][j].muteThreshold_dB = _D_FPGA_CHECK_VOLUME_THRESHOLD_MUTE;   //-50dB

        }
    }
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