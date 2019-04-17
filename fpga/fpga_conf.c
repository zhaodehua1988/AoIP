#include "fpga_conf.h"
#include "fpga_common.h"
#include "his_spi.h"
#include "sys_env.h"
#include "sys_ip.h"
#include "fpga_conf_json.h"
#include "fpga_sdp.h"
#include "PCA9555.h"
#include "his_dis.h"
#include "iTE6615_Init.h"
#include "fpga_igmp.h"
#include "fpga_update.h"
#include "fpga_volume.h"
#include "fpga_check.h"
#define _FPGA_MUTEX_ENA_D (1)
#if _FPGA_MUTEX_ENA_D
static pthread_mutex_t gMutexSetWin;
static pthread_mutex_t gMutexUpdateSdpInfo;
#endif

FPGA_CONF_DEV *gpFpgaConfDev;

typedef struct _S_FPGA_CONF_WIN_INFO
{
    WV_U16 winID;
    FPGA_CONF_WIN_T win;
} _S_FPGA_CONF_WIN_INFO;


static _S_FPGA_CONF_WIN_INFO gEthWinInfo[FPGA_CONF_ETHNUM_D][FPGA_CONF_SRCNUM_D];
static _S_FPGA_CONF_WIN_INFO gEthWinInfoNew[FPGA_CONF_ETHNUM_D][FPGA_CONF_SRCNUM_D];


/******************************************************************
 * WV_S32 fpga_conf_setIgmp(_S_FPGA_CONF_WIN_INFO (*newAddr)[FPGA_CONF_SRCNUM_D])
 * 组播设置
 * ****************************************************************/
WV_S32 fpga_conf_setIgmp(_S_FPGA_CONF_WIN_INFO (*newAddr)[FPGA_CONF_SRCNUM_D])
{

    WV_S32 i, j;
    WV_U8 multicastAddrInt[5] = {0};
    WV_U8 srcAddrInt[5] = {0};

    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        for (j = 0; j < FPGA_CONF_SRCNUM_D; j++)
        {
            if (gEthWinInfo[i][j].win.win_ena == 0)
                continue;
            if (gEthWinInfo[i][j].win.video_ipv6_ena != newAddr[i][j].win.video_ipv6_ena ||
                strncmp(gEthWinInfo[i][j].win.video_ip, newAddr[i][j].win.video_ip, FPGA_CONF_IPLEN) != 0 ||
                gEthWinInfo[i][j].win.video_port != newAddr[i][j].win.video_port)
            {
                //退出组播gSrcAddr[i][j]
                FPGA_printf("eth[%d][%d]退出组播地址%s\n", i, j, gEthWinInfo[i][j].win.video_ip);
                FPGA_COMMON_getIpInt(gEthWinInfo[i][j].win.video_ip, multicastAddrInt);
                FPGA_IGMP_exit(i, j, multicastAddrInt);
            }
        }
    }

    //memcpy(gEthWinInfo, newAddr, sizeof(gEthWinInfo));
    //加入组播
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        for (j = 0; j < FPGA_CONF_SRCNUM_D; j++)
        {
            if (newAddr[i][j].win.win_ena == 0)
                continue;
            FPGA_COMMON_getIpInt(newAddr[i][j].win.video_ip, multicastAddrInt);
            FPGA_printf("eth[%d][%d]加入组播地址%s\n", i, j, newAddr[i][j].win.video_ip);
            FPGA_IGMP_join(i, j, multicastAddrInt, srcAddrInt);
        }
    }

    return WV_SOK;
}
/****************************************************
 * void FPGA_CONF_SetIgmpSendSecond(WV_U32 sec)
 * 函数功能：设置组播发送时间间隔
 * 参说说明：sec 时间，单位 秒
 * *************************************************/
void FPGA_CONF_SetIgmpSendSecond(WV_U32 sec)
{
    FPGA_IGMP_SetSecondOfIgmpSend(sec);
}

/***************************************************
 * void FPGA_CONF_SetCheckTimeValue(WV_U32 time)
 * 设置检测静帧的时间,单位 秒
 * ************************************************/
void FPGA_CONF_SetCheckTimeValue(WV_U32 time_s)
{
    FPGA_CHECK_SetCheckTimeValue(time_s);
}

/****************************************************
 * WV_S32 FPGA_CONF_GetWinFreezeVal(WV_U32 winID)
 * 函数说明：查询窗口的是否静帧
 * 返回值  0：没有静帧
 *        1：视频静帧
 *        -1：查询错误
 * *************************************************/
WV_U32 FPGA_CONF_GetWinFreezeVal(WV_U32 winID)
{
    if (winID > 15)
        return -1;

    WV_S32 i, j;
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        for (j = 0; j < FPGA_CONF_SRCNUM_D; j++)
        {
            if (gEthWinInfo[i][j].winID == winID)
            {
                return FPGA_CHECK_GetWinFreeze(i,j);
            }
        }
    }
    return 0;
}

/****************************************************
 * WV_S32 FPGA_CONF_CheckNoStream(WV_U32 winID)
 * 查询窗口是否有视频流
 * 返回值 0:流播放正常
 *       1:没有视频流
 *      -1:查询错误，包括输入窗口id超出范围，id范围[0~15]
 * *************************************************/
WV_S32 FPGA_CONF_CheckNoStream(WV_U32 winID)
{
    if (winID > 15)
        return -1;

    WV_S32 i, j;
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        for (j = 0; j < FPGA_CONF_SRCNUM_D; j++)
        {
            if (gEthWinInfo[i][j].winID == winID)
            {
                return FPGA_Check_NoSignal(i,j);
            }
        }
    }
    return 1;
}

/*****************************************************
WV_S32 fpga_conf_DisChangeEna(WV_U16 winena)
*设置视频输出更新（包括窗口坐标更新生效）
*****************************************************/
WV_S32 fpga_conf_DisChangeEna(WV_U16 winena)
{

    WV_S32 ret = 0;
    FPGA_printf("winena = 0x%X\n", winena);
    ret += HIS_SPI_FpgaWd(0x604, winena);
    return ret;
}
/*******************************************************************
static void fpga_conf_SetEthIpAsinterlaceOrder(FPGA_CONF_SRC_ADDR_T (*srcAddr)[FPGA_CONF_SRCNUM_D])
    调整网卡ip去隔行顺序，即把带有隔行的放到前面窗口
*******************************************************************/
static void fpga_conf_SetEthIpAsinterlaceOrder(_S_FPGA_CONF_WIN_INFO (*srcAddr)[FPGA_CONF_SRCNUM_D])
{
    _S_FPGA_CONF_WIN_INFO temp;
    WV_S32 i, j, k;

    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {

        for (j = 0; j < FPGA_CONF_SRCNUM_D - 1; j++)
        {
            for (k = 0; k < FPGA_CONF_SRCNUM_D - 1 - j; k++)
            {
                if (srcAddr[i][j].win.win_ena == 1 && srcAddr[i][k].win.sdpInfo.video_interlace < srcAddr[i][k + 1].win.sdpInfo.video_interlace)
                {
                    temp = srcAddr[i][k];
                    srcAddr[i][k] = srcAddr[i][k + 1];
                    srcAddr[i][k + 1] = temp;
                }
            }
        }
    }

    //设置去隔行
    WV_U16 data = 0;
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        if (srcAddr[i][0].win.sdpInfo.video_interlace == 1)
        {
            data |= 1 << i;
        }
    }
    FPGA_printf("去隔行设置 0x11 = 0x%04X \n", data);
    //HIS_SPI_FpgaWd(0x11,data);
}

/********************************************************************
 * WV_S32 FPGA_SDP_ReadFromFpga(WV_U16 eth,WV_U16 chl,FPGA_SDP_Info *pGetInfo)
 * ******************************************************************/
WV_S32 FPGA_SDP_ReadFromFpga(WV_U16 eth, WV_U16 chl, FPGA_SDP_Info *pGetInfo)
{
    WV_S32 ret = 0;
    WV_S8 *pSdpData = (WV_S8 *)malloc(FPGA_CONF_SDP_DATALEN + 8);
    memset(pSdpData, 0, FPGA_CONF_SDP_DATALEN + 8);

    //通过fgpa获取sdp报文
    //TODO


    //通过获取的的sdp数据，获取sdp信息 FPGA_SDP_Info
    FPGA_SDP_Info info;
    memset(&info, 0, sizeof(FPGA_SDP_Info));
    ret = FPGA_SDP_AnalysisSdpInfo(pSdpData, pGetInfo);
    free(pSdpData);

    return ret;
}
/**********************************************************
 * int fpga_conf_getSdpAudioChlNum(char *pAudioChlIn)
 * 查询sdp音频声道数量
**********************************************************/
int fpga_conf_getSdpAudioChlNum(char *pAudioChlIn)
{
    //printf();
    //audiochl = "ST,51"
    int chlNum = 0;
    int undefinedChl = 0, i = 0;
    int M = 0, DM = 0, ST = 0, LtRt = 0, _51 = 0, _71 = 0, _222 = 0, SGRP = 0, U = 0;
    M = strstr_cnt(pAudioChlIn, "M") - strstr_cnt(pAudioChlIn, "DM");
    DM = strstr_cnt(pAudioChlIn, "DM") * 2;
    ST = strstr_cnt(pAudioChlIn, "ST") * 2;
    LtRt = strstr_cnt(pAudioChlIn, "LtRt") * 2;
    _51 = strstr_cnt(pAudioChlIn, "51") * 6;
    _71 = strstr_cnt(pAudioChlIn, "71") * 8;
    _222 = strstr_cnt(pAudioChlIn, "222") * 24;
    SGRP = strstr_cnt(pAudioChlIn, "SGRP") * 4;

    undefinedChl = strstr_cnt(pAudioChlIn, "U");

    char *p = strstr(pAudioChlIn, "U");

    for (i = 0; i < undefinedChl; i++)
    {

        if (strlen(p) >= 3)
        {
            int num;
            if (sscanf(&p[1], "%d", &num) != 1)
            {
                //printf("can not get the item(%s) val(uint)\n", pItem);
                return -1;
            }
            else
            {
                //printf("U%02d = %d |",num,num);
                U += num;
            }
        }
    }
    chlNum = M + DM + ST + LtRt + _51 + _71 + _222 + SGRP + U;
#ifdef FPGA_DEBUG
    FPGA_printf("\r\nchlNum=%d,M=%d, DM=%d, ST=%d, LtRt=%d, 51=%d, 71=%d, 222=%d,SGPR=%d,U=%d\n", chlNum, M, DM, ST, LtRt, _51, _71, _222, SGRP, U);
#endif
    return chlNum;
}
/**********************************************************************************
 * int FPGA_CONF_SetSdpInfo(FPGA_SDP_Info *pSetInfo,unsigned short eth,unsigned short ipSel)
 * 功能：配置网络接收的sdp信息
 * 参数说明：
 * pSetInfo:sdp数据
 * type: 0=videosdp 1=audiosdp 2=videoAndAudio sdp
 * eth:网卡id【0～3】
 * ipSel:第几个ip地址
 * *******************************************************************************/
int FPGA_CONF_SetSdpInfo(FPGA_SDP_Info *pSetInfo, WV_U16 eth, WV_U16 ipSel)
{
    if (pthread_mutex_lock(&gMutexUpdateSdpInfo) != 0)
    {
        FPGA_printf("fpga update sdp info err!mutex lock err!\n");
        return WV_EFAIL;
    }
    WV_S32 ret = 0;
    WV_U16 baseAddr;
    WV_U16 regAddr;
    WV_U16 videoInfo = 0, videoWidth = 0, videoHight = 0, avPt = 0, audioInfo = 0;
    //video_sampling
    //0：YCbCr:4:4:4    1:YCbCr:4:2:2    ：YCbCr:4:2:0      3：RGB             4：CLYCbCr-4:4:4    5：CLYCbCr-4:2:2
    //6：CLYCbCr-4:2:0  7：ICtCp-4:4:4   8：ICtCp-4:2:2      9：ICtCp-4:2:0    10：XYZ 其他：预留
    // Bit[3:0]
    if (0 == strcmp(pSetInfo->video_sampling, "YCbCr-4:4:4") || 0 == strcmp(pSetInfo->video_sampling, "YCbCr:4:4:4"))
    {
        videoInfo |= 0x0;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "YCbCr-4:2:2") || 0 == strcmp(pSetInfo->video_sampling, "YCbCr:4:2:2"))
    {
        videoInfo |= 0x1;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "YCbCr-4:2:0") || 0 == strcmp(pSetInfo->video_sampling, "YCbCr:4:2:0"))
    {
        videoInfo |= 0x2;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "RGB") || 0 == strcmp(pSetInfo->video_sampling, "rgb"))
    {
        videoInfo |= 0x3;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "CLYCbCr-4:4:4") || 0 == strcmp(pSetInfo->video_sampling, "CLYCbCr:4:4:4"))
    {
        videoInfo |= 0x4;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "CLYCbCr-4:2:2") || 0 == strcmp(pSetInfo->video_sampling, "CLYCbCr:4:2:2"))
    {
        videoInfo |= 0x5;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "CLYCbCr-4:2:0") || 0 == strcmp(pSetInfo->video_sampling, "CLYCbCr:4:2:0"))
    {
        videoInfo |= 0x6;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "ICtCp-4:4:4") || 0 == strcmp(pSetInfo->video_sampling, "ICtCp:4:4:4"))
    {
        videoInfo |= 0x7;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "ICtCp-4:2:2") || 0 == strcmp(pSetInfo->video_sampling, "ICtCp:4:2:2"))
    {
        videoInfo |= 0x8;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "ICtCp-4:2:0") || 0 == strcmp(pSetInfo->video_sampling, "ICtCp:4:2:0"))
    {
        videoInfo |= 0x9;
    }
    else
    {
        videoInfo |= 0x1; //默认YCbCr-4:2:2
    }
    //video_depth
    if (8 == pSetInfo->video_depth)
    {
        videoInfo |= 0x0 << 4;
    }
    else if (10 == pSetInfo->video_depth)
    {
        videoInfo |= 0x1 << 4;
    }
    else if (12 == pSetInfo->video_depth)
    {
        videoInfo |= 0x2 << 4;
    }
    else if (16 == pSetInfo->video_depth)
    {
        videoInfo |= 0x3 << 4;
    }
    else if (24 == pSetInfo->video_depth)
    {
        videoInfo |= 0x4 << 4;
    }
    else
    {
        videoInfo |= 0x0 << 4; //默认为 video_depth=8
    }
    //video_framerate
    if (strstr(pSetInfo->video_framerate, "50") != NULL)
    {
        videoInfo |= 0x0 << 7;
    }
    else if (strstr(pSetInfo->video_framerate, "60") != NULL)
    {
        videoInfo |= 0x1 << 7;
    }
    else if (strstr(pSetInfo->video_framerate, "23.98") != NULL || strstr(pSetInfo->video_framerate, "23") != NULL)
    {
        videoInfo |= 0x2 << 7;
    }
    else if (strstr(pSetInfo->video_framerate, "24") != NULL)
    {
        videoInfo |= 0x3 << 7;
    }
    else if (strstr(pSetInfo->video_framerate, "47.95") != NULL || strstr(pSetInfo->video_framerate, "47") != NULL)
    {
        videoInfo |= 0x4 << 7;
    }
    else if (strstr(pSetInfo->video_framerate, "25") != NULL)
    {
        videoInfo |= 0x5 << 7;
    }
    else if (strstr(pSetInfo->video_framerate, "29.97") != NULL || strstr(pSetInfo->video_framerate, "29") != NULL)
    {
        videoInfo |= 0x6 << 7;
    }
    else if (strstr(pSetInfo->video_framerate, "30") != NULL)
    {
        videoInfo |= 0x7 << 7;
    }
    else if (strstr(pSetInfo->video_framerate, "48") != NULL)
    {
        videoInfo |= 0x8 << 7;
    }
    else if (strstr(pSetInfo->video_framerate, "59.94") != NULL || strstr(pSetInfo->video_framerate, "59") != NULL)
    {
        videoInfo |= 0x9 << 7;
    }
    else
    {
        videoInfo |= 0x1 << 7; //默认60MHZ
    }

    //video_interlace
    //Bit11 b1 隔行 b0逐行

    if (1 == pSetInfo->video_interlace)
    {
        videoInfo |= 0x1 << 11;
    }
    else
    {
        videoInfo |= 0x0 << 11; //默认逐行
    }
    //video_colorimetry
    if (0 == strcmp(pSetInfo->video_colorimetry, "BT.709") || 0 == strcmp(pSetInfo->video_colorimetry, "BT709"))
    {
        videoInfo |= 0x0 << 12;
    }
    else if (0 == strcmp(pSetInfo->video_colorimetry, "BT.2020") || 0 == strcmp(pSetInfo->video_colorimetry, "BT2020"))
    {
        videoInfo |= 0x1 << 12;
    }
    else if (0 == strcmp(pSetInfo->video_colorimetry, "BT.610") || 0 == strcmp(pSetInfo->video_colorimetry, "BT610"))
    {
        videoInfo |= 0x2 << 12;
    }
    else
    {
        videoInfo |= 0x0 << 12; //默认BT.709
    }
    /****************audio info***********************************************/
    /*         Table 1 -- Channel Order Convention Grouping Symbols
    ------------------------------------------------------------------------------------------------------------------
    ID      Channel Grouping |  Quantity of Audio | Description of group   | Order of Audio
            Symbol           |  Channels in group |                        | Channels in group
                                        
    0               M               1                   Mono                    Mono
    1               DM              2                   Dual Mono               M1, M2
    2               ST              2                   Standard Stereo         Left, Right
    3               LtRt            2                   Matrix Stereo           Left Total, Right Total
    4               51              6                   5.1 Surround            L, R, C, LFE, Ls, Rs
    5               71              8                   7.1 Surround            L, R, C, LFE, Lss, Rss, Lrs, Rrs
    6               222             24                  22.2 Surround           Order shall be per SMPTE ST 2036-2,
    7               SGRP            4                   One SDI audio group     1 , 2, 3, 4
    */
    //audio chl 因为fpga文档没有更新，暂时保留

    //WV_S32 chl = fpga_conf_getSdpAudioChlNum(pSetInfo->audio_channel);
    //if(chl != 0){
    //    pSetInfo->audio_chl_num = (WV_U16) chl;
    //}
    //audioInfo = 0xff & chl;
    audioInfo = 0xff & pSetInfo->audio_chl_num;
    //audio_depth
    if (8 == pSetInfo->audio_depth)
    {
        audioInfo |= 0x0 << 8;
    }
    else if (16 == pSetInfo->audio_depth)
    {
        audioInfo |= 0x1 << 8;
    }
    else
    {
        audioInfo |= 0x0 << 8;
    }
    /****************video width /hight***********************************************/
    videoWidth = pSetInfo->video_width;
    videoHight = pSetInfo->video_height;
    // if(pSetInfo->video_interlace == 1){
    //     //如果时隔行发送，则设置视频高度为原始高度的一半
    //     WV_printf("设置网卡");
    //     videoHight = pSetInfo->video_height/2;
    // }else{
    //     videoHight = pSetInfo->video_height;
    // }

    //avPt
    avPt = (pSetInfo->video_pt << 8) | (pSetInfo->audio_pt & 0xff);
    baseAddr = 0x100;
    regAddr = ((baseAddr >> 8) + eth) << 8;

    ret += HIS_SPI_FpgaWd(regAddr + 0x46 + ipSel * 5, videoInfo);
    ret += HIS_SPI_FpgaWd(regAddr + 0x47 + ipSel * 5, videoWidth);
    ret += HIS_SPI_FpgaWd(regAddr + 0x48 + ipSel * 5, videoHight);
    ret += HIS_SPI_FpgaWd(regAddr + 0x49 + ipSel * 5, avPt);
    ret += HIS_SPI_FpgaWd(regAddr + 0x4a + ipSel * 5, audioInfo);

    if (ret != 0)
    {
        WV_printf("set sdp info err \n");
    }

    //gpFpgaConfDev->win[0].sdpInfo = *pSetInfo;//更新sdp信息
#ifdef FPGA_SDP_DEBUG
    printf("--------------sdp---------------------------\n");
    printf("set spi  0x%04x  0x%04x\n", regAddr + 0x46 + ipSel * 5, videoInfo);
    printf("set spi  0x%04x  0x%04x\n", regAddr + 0x47 + ipSel * 5, videoWidth);
    printf("set spi  0x%04x  0x%04x\n", regAddr + 0x48 + ipSel * 5, videoHight);
    printf("set spi  0x%04x  0x%04x\n", regAddr + 0x49 + ipSel * 5, avPt);
    printf("set spi  0x%04x  0x%04x\n", regAddr + 0x4a + ipSel * 5, audioInfo);

#endif
    //setSdpInfoEnd:
    if (pthread_mutex_unlock(&gMutexUpdateSdpInfo) != 0)
    {
        FPGA_printf("fpga update sdp info err!mutex unlock err!\n");
        return WV_EFAIL;
    }
    return ret;
}

/******************************************************************
 * void fpga_conf_SetWinIpAndSdp(FPGA_CONF_WIN_T pWin[])
 * //把设置的窗口信息，转换为网卡信息，解析窗口ip,sdp并设置
 * ****************************************************************/
WV_S32 fpga_conf_SetWinIpAndSdp(FPGA_CONF_WIN_T pWin[])
{
    //_S_FPGA_CONF_WIN_INFO winInfo[FPGA_CONF_ETHNUM_D][FPGA_CONF_SRCNUM_D]={0};

    memset(gEthWinInfoNew, 0, sizeof(gEthWinInfoNew));
    WV_S32 i, j;
    for (i = 0; i < FPGA_CONF_WINNUM_D; i++)
    {
        if (pWin[i].win_ena == 0 || pWin[i].channel > 3)
            continue;
        for (j = 0; j < FPGA_CONF_SRCNUM_D; j++)
        {
            if (gEthWinInfoNew[pWin[i].channel][j].win.win_ena == 0)
            {
                gEthWinInfoNew[pWin[i].channel][j].win = pWin[i];
                gEthWinInfoNew[pWin[i].channel][j].winID = i;
                break;
            }
        }
    }

    fpga_conf_SetEthIpAsinterlaceOrder(gEthWinInfoNew);

#ifdef FPGA_DEBUG

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (gEthWinInfoNew[i][j].win.win_ena == 1)
                FPGA_printf("eth_ip[%d][%d].winid=%d,x=%d,y=%d,w=%d,h=%d,ip=%s\n", i, j,
                            gEthWinInfoNew[i][j].winID, gEthWinInfoNew[i][j].win.x, gEthWinInfoNew[i][j].win.y, gEthWinInfoNew[i][j].win.w, gEthWinInfoNew[i][j].win.h, gEthWinInfoNew[i][j].win.video_ip);
        }
    }
    FPGA_printf("\r\n");
#endif
    //加入组播
    fpga_conf_setIgmp(gEthWinInfoNew);

    //设置视频源ip和端口号
    WV_S32 ret = 0;
    WV_U16 baseAddr;
    WV_U16 regAddr;
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        baseAddr = 0x0100;
        baseAddr = ((baseAddr >> 8) + i) << 8;
        regAddr = baseAddr;
        //设置为ipv4因为本版本不支持ipv6
        ret += HIS_SPI_FpgaWd(regAddr + 0x1f, 0); //设置接收源ipv4Oripv6
        WV_U8 ip[16] = {0};
        for (j = 0; j < 4; j++)
        {
            if (gEthWinInfoNew[i][j].win.win_ena == 0)
            {
                ret += HIS_SPI_FpgaWd(regAddr + 0x20 + j * 9, 0);
                ret += HIS_SPI_FpgaWd(regAddr + 0x21 + j * 9, 0);
                //端口号
                ret += HIS_SPI_FpgaWd(regAddr + 0x28 + j * 9, 0);
                continue;
            }
            if (gEthWinInfoNew[i][j].win.video_ipv6_ena == 0)
            { //ipv4
                memset(ip, 0, sizeof(ip));
                FPGA_COMMON_getIpInt(gEthWinInfoNew[i][j].win.video_ip, ip);
                //ip
                FPGA_printf("设置eth_ip[%d][%d] = %d.%d.%d.%d port=%d[0x04x] \n", i, j, ip[0], ip[1], ip[2], ip[3], gEthWinInfoNew[i][j].win.video_port);
                ret += HIS_SPI_FpgaWd(regAddr + 0x20 + j * 9, (ip[2] << 8) | ip[3]);
                //FPGA_printf("reg[0x%04x] = [0x%04x] \n",regAddr + 0x20 + j * 9,(ip[2] << 8) | ip[3]);
                ret += HIS_SPI_FpgaWd(regAddr + 0x21 + j * 9, (ip[0] << 8) | ip[1]);
                //FPGA_printf("reg[0x%04x] = [0x%04x] \n",regAddr + 0x21 + j * 9,(ip[0] << 8) | ip[1]);
                //端口号
                ret += HIS_SPI_FpgaWd(regAddr + 0x28 + j * 9, gEthWinInfoNew[i][j].win.video_port);
                //FPGA_printf("reg[0x%04x] = [0x%04x] \n",regAddr + 0x28 + j * 9,gEthWinInfoNew[i][j].win.video_port);
            }
            else
            { //ipv6
                //测试阶段暂不处理
            }
        }
    }

    //设置sdp信息
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (gEthWinInfoNew[i][j].win.win_ena == 0)
                continue;

            //设置视频sdp信息
            FPGA_printf("sdp:eth[%d]ip[%d],window[%d]\n", i, j, gEthWinInfoNew[i][j].winID);
            ret = FPGA_CONF_SetSdpInfo(&gEthWinInfoNew[i][j].win.sdpInfo, (WV_U16)i, (WV_U16)j);
            if (ret != 0)
            {
                WV_ERROR("sdp:eth[%d]ip[%d],window[%d]::set sdp info error\n", i, j, gEthWinInfoNew[i][j].winID);
            }
        }
    }

    return WV_SOK;
}

/*******************************************************************
WV_S32 FPGA_CONF_SetWin(FPGA_CONF_WIN_T *pWin);
fpga窗口设置
*******************************************************************/
WV_S32 FPGA_CONF_SetWin(FPGA_CONF_WIN_T winArray[])
{

    pthread_mutex_lock(&gMutexSetWin);

    WV_S32 i, j, ret = 0;
    WV_U16 baseAddr = 0x0500;
    WV_U16 regAddr, winEna = 0;

    //
    WV_S32 mode = 0; //1080
    for (i = 0; i < FPGA_CONF_WINNUM_D; i++)
    {
        if (winArray[i].win_ena == 1)
        {
            if (winArray[i].sdpInfo.video_width > 1920 || winArray[i].sdpInfo.video_height > 1080)
            {
                mode = 1; // 4k
                break;
            }
        }
    }

    //设置ip和组播地址
    ret += fpga_conf_SetWinIpAndSdp(winArray);

    //设置窗口坐标
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        for (j = 0; j < FPGA_CONF_SRCNUM_D; j++)
        {
            if (gEthWinInfoNew[i][j].win.win_ena == 0)
                continue;
            regAddr = ((baseAddr >> 4) + i * 4) << 4;
            regAddr = ((regAddr >> 4) + j) << 4;
            //FPGA_printf("11 win[%d][%d]regAddr = %X, x=%d,y=%d,w=%d,h=%d\n",i,j, regAddr, gEthWinInfoNew[i][j].win.x, gEthWinInfoNew[i][j].win.y, gEthWinInfoNew[i][j].win.w, gEthWinInfoNew[i][j].win.h);
            ret += HIS_SPI_FpgaWd(regAddr, gEthWinInfoNew[i][j].win.x);
            ret += HIS_SPI_FpgaWd(regAddr + 1, gEthWinInfoNew[i][j].win.y);
            ret += HIS_SPI_FpgaWd(regAddr + 2, gEthWinInfoNew[i][j].win.w);
            ret += HIS_SPI_FpgaWd(regAddr + 3, gEthWinInfoNew[i][j].win.h);
            winEna |= 1 << (j + i * 4);
        }
    }

    //设置窗口使能
    ret += fpga_conf_DisChangeEna(winEna);

    if (ret == 0)
    {
        FPGA_printf("set windows ok \n");
        memcpy(&gEthWinInfo, &gEthWinInfoNew, sizeof(gEthWinInfo));
    }
    else
    {
        WV_ERROR("FPGA_CONF_SetWin err\n");
    }
    pthread_mutex_unlock(&gMutexSetWin);
    return ret;
}

/*******************************************************************
WV_S32 FPGA_CONF_SetETH(FPGA_CONF_ETH_T *pEth,WV_S32 ethID);
配置fpga网卡
pEth:网卡信息
ethID:网卡id[0~3]
*******************************************************************/
WV_S32 FPGA_CONF_SetETH(FPGA_CONF_ETH_T *pEth, WV_S32 ethID)
{

    WV_U16 baseAddr;
    WV_U16 regAddr;
    WV_S32 ret = 0;
    baseAddr = 0x100;
    regAddr = ((baseAddr >> 8) + ethID) << 8;
    WV_U8 ipInt[16] = {0};
    WV_U8 mac[6] = {0};
    WV_U8 netMask[4] = {0};
    WV_U8 getWay[4] = {0};
    ret += FPGA_COMMON_getIpInt(pEth->ipv6, ipInt);
    ret += FPGA_COMMON_getMacInt(pEth->mac, mac);
    ret += FPGA_COMMON_getIpInt(pEth->subnet_mask, netMask);
    ret += FPGA_COMMON_getIpInt(pEth->getway, getWay);
#ifdef FPGA_DEBUG
    if (ret == 0)
    {
        FPGA_printf("\n--------------eth[%d]------------------\n", ethID);
        FPGA_printf("ip:%d.%d.%d.%d\n", ipInt[0], ipInt[1], ipInt[2], ipInt[3]);                            //192.168.1.100
        FPGA_printf("netmask:%d.%d.%d.%d\n", netMask[0], netMask[1], netMask[2], netMask[3]);               //255.255.255.0
        FPGA_printf("getway:%d.%d.%d.%d\n", getWay[0], getWay[1], getWay[2], getWay[3]);                    //192.168.1.1
        FPGA_printf("mac:%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); //80-9f-fb-88-88-00
        FPGA_printf("----------------------------------------\n");
    }
#endif
    //mac
    ret += HIS_SPI_FpgaWd(regAddr + 0x2, mac[4] << 8 | mac[5]);
    ret += HIS_SPI_FpgaWd(regAddr + 0x3, mac[2] << 8 | mac[3]);
    ret += HIS_SPI_FpgaWd(regAddr + 0x4, mac[0] << 8 | mac[1]);
    //ip
    if (pEth->ipv6Ena == 0)
    { //ipv4
        ret += HIS_SPI_FpgaWd(regAddr + 0x5, ipInt[2] << 8 | ipInt[3]);
        ret += HIS_SPI_FpgaWd(regAddr + 0x6, ipInt[0] << 8 | ipInt[1]);
    }
    else
    {
    }
    //net_mask
    ret += HIS_SPI_FpgaWd(regAddr + 0xd, netMask[2] << 8 | netMask[3]);
    ret += HIS_SPI_FpgaWd(regAddr + 0xe, netMask[0] << 8 | netMask[1]);
    //getway

    ret += HIS_SPI_FpgaWd(regAddr + 0xf, getWay[2] << 8 | getWay[3]);
    ret += HIS_SPI_FpgaWd(regAddr + 0x10, getWay[0] << 8 | getWay[1]);
    if (ret != 0)
    {
        WV_ERROR("set eth[%d] err\n", ethID);
    }
    else
    {
        memcpy((WV_S8 *)&gpFpgaConfDev->eth[ethID], (WV_S8 *)pEth, sizeof(FPGA_CONF_ETH_T));
        //fpga_conf_SaveEthCfgToFile();
    }
    //复位组播
    FPGA_IGMP_Reset();
    return ret;
}
/*******************************************************************
WV_S32 FPGA_CONF_GetETH(FPGA_CONF_ETH_T *pEth,WV_S32 ethID);
配置fpga网卡
pEth:网卡信息
ethID:网卡id[0~3]
*******************************************************************/
WV_S32 FPGA_CONF_GetETH(FPGA_CONF_ETH_T *pEth, WV_S32 ethID)
{
    memcpy((WV_S8 *)pEth, (WV_S8 *)&gpFpgaConfDev->eth[ethID], sizeof(FPGA_CONF_ETH_T));
    return WV_SOK;
}

WV_S32 FPGA_CONF_GetEthInt(WV_U8 ip[], WV_U8 mac[], WV_S32 ethID)
{
    if (ethID > 3 || ethID < 0)
        return WV_EFAIL;
    WV_U8 ipInt[16] = {0};
    WV_U8 macInt[6] = {0};
    WV_U8 netMask[4] = {0};
    WV_U8 getWay[4] = {0};
    FPGA_COMMON_getIpInt(gpFpgaConfDev->eth[ethID].ipv6, ipInt);
    FPGA_COMMON_getMacInt(gpFpgaConfDev->eth[ethID].mac, macInt);
    FPGA_COMMON_getIpInt(gpFpgaConfDev->eth[ethID].subnet_mask, netMask);
    FPGA_COMMON_getIpInt(gpFpgaConfDev->eth[ethID].getway, getWay);
    WV_S32 i;
    for (i = 0; i < 4; i++)
    {
        ip[i] = ipInt[i];
    }
    for (i = 0; i < 6; i++)
    {
        mac[i] = macInt[i];
    }

    return WV_SOK;
}
/**************************************************************
* WV_S32 FPGA_CONF_GetVersion(WV_S8 *pFpgaVer);
*查询版本信息
*************************************************************/
WV_S32 FPGA_CONF_GetVersion(FPGA_CONF_VER *pFpgaVer)
{

    WV_U16 temp;
    HIS_SPI_FpgaRd(0, &pFpgaVer->ver);
    HIS_SPI_FpgaRd(0x2, &pFpgaVer->year);
    HIS_SPI_FpgaRd(0x3, &temp);
    pFpgaVer->month = (temp & 0xf00) >> 8;
    pFpgaVer->day = temp & 0xff;

    return WV_SOK;
}

/******************************************************************************
 * WV_S32 FPGA_CONF_SetResolution(WV_S32 resolution)
 * ***************************************************************************/
WV_S32 FPGA_CONF_Resolution(WV_S32 resolution)
{
    //resolution
    WV_U16 data = 0;
    WV_U16 winEna = 0;
    HIS_SPI_FpgaRd(0x600, &data);
    HIS_SPI_FpgaRd(0x604, &winEna);
    FPGA_printf("resolution = %d \n", resolution);
    switch (resolution)
    {
    case 0: //3840*2160 p60
        data &= 0xff;
        data |= 0xc00; //60HZ
        HIS_SPI_FpgaWd(0x15, 0xcccc);
        HIS_SPI_FpgaWd(0x600, data);
        HIS_SPI_FpgaWd(0x15, 0xcccc);
        HIS_SPI_FpgaWd(0x12, 0x20b);
        break;
    case 1: //3840*2160 p50
        data = data & 0xff;
        data = data | 0xc00; //60HZ
        HIS_SPI_FpgaWd(0x600, data);
        HIS_SPI_FpgaWd(0x15, 0xcccc);
        HIS_SPI_FpgaWd(0x12, 0x20b);
        break;
    case 2: //3840*2160 p30
        data = data & 0xff;
        data = data | 0x800;
        HIS_SPI_FpgaWd(0x600, data);
        HIS_SPI_FpgaWd(0x15, 0xf0f0);
        HIS_SPI_FpgaWd(0x12, 0x0b);
        break;
    case 3: //1920*1080 p60 VSEA
        data = data & 0xff;
        data = data | 0x900; //60HZ
        HIS_SPI_FpgaWd(0x600, data);
        HIS_SPI_FpgaWd(0x15, 0xf0f0);
        HIS_SPI_FpgaWd(0x12, 0x0b);
        break;
    case 4:                   //1920*1080 i60
        data = data & 0xff;
        data = data | 0x900; //60HZ
        HIS_SPI_FpgaWd(0x600, data);
        HIS_SPI_FpgaWd(0x15, 0xf0f0);
        HIS_SPI_FpgaWd(0x12, 0x0b);
        break;
    default:
        return WV_EFAIL;
        break;
    }
    return HIS_DIS_SetCustomTiming((WV_U32)resolution);
}

/****************************************************************************

void FPGA_CONF_SetDisAlpha(WV_S32 alpha)
//设置边框叠加显示透明度，取值范围[0~128],0代表透明，128代表不透明
****************************************************************************/
void FPGA_CONF_SetDisAlpha(WV_S32 alpha)
{
    if (alpha >= 0 && alpha <= 128)
    {
        HIS_SPI_FpgaWd(0x605, (WV_U16)alpha);
    }
    else if (alpha > 128)
    {
        HIS_SPI_FpgaWd(0x605, 128);
    }
}
/****************************************************************************

void FPGA_CONF_SetOutPutAudioSel(WV_U16 winID,WV_U16 audioChl) 
//设置输出声音来自那个窗口,第几个声道
****************************************************************************/
void FPGA_CONF_SetOutPutAudioSel(WV_U16 winID, WV_U16 audioChl)
{
    if (winID > 15 || audioChl < 0)
        return; //audioSel取值范围[0~15]
    WV_U16 data;
    WV_S32 i, j;

    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        for (j = 0; j < FPGA_CONF_SRCNUM_D; j++)
        {
            if (gEthWinInfo[i][j].winID == winID)
            {

                data = (audioChl & 0xf) | (i << 6) | (j << 4);
                FPGA_printf("eth_IP[%d][%d]set audio output 0x08 = 0x%04x\n", i, j, data);
                HIS_SPI_FpgaWd(0x08, data);
                return;
            }
        }
    }
}
/****************************************************************************
void FPGA_CONF_SetOutPutVolumeMultiple(WV_U16 multipl)
//设置输出音量的放大倍数 【0～15】
****************************************************************************/
void FPGA_CONF_SetOutPutVolumeMultiple(WV_U16 multipl)
{
    HIS_SPI_FpgaWd(0x09, multipl);
}
/****************************************************************************
void FPGA_CONF_GetVolume(WV_U16 winID,WV_U8 volume[])
函数说明：查询某个窗口的音量
参数说明：
    winID：窗口id
    volume：获取到的音量值，8字节数组，代表8个声道
****************************************************************************/
void FPGA_CONF_GetVolume(WV_U16 winID,WV_U16 volume[])
{   
    //WV_printf("get volume\n");
    WV_S32 i, j;
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        for (j = 0; j < FPGA_CONF_SRCNUM_D; j++)
        {
            if (gEthWinInfo[i][j].winID == winID)
            {
                FPGA_VOLUME_Get(i,j,volume);
                return;
            }
        }
    }

}
/*******************************************************************
WV_S32 FPGA_CONF_Reset();
fpga复位
*******************************************************************/
WV_S32 FPGA_CONF_Reset()
{
    PCA9555_Clr(0x3, 0x2);
    usleep(100000);
    PCA9555_Set(0x3, 0x2);
    usleep(100000);
    WV_U16 reg = 0x4, data = 0;
    WV_S32 i;
    for (i = 0; i < 100; i++)
    {
        HIS_SPI_FpgaRd(reg, &data);
        if (data == 0x3210)
        {
            HIS_SPI_FpgaWd(reg, 0x123);
        }
        if (data == 0x123)
        {
            break;
        }
        usleep(100000);
    }

    return WV_SOK;
}
/*******************************************************************
WV_S32 FPGA_CONF_UpdateFpga(WV_S8 *pFpgaBin);
fpga升级
*******************************************************************/
WV_S32 FPGA_CONF_UpdateFpga(WV_S8 *pFpgaBin)
{
    return FPGA_UPDATE_Update(pFpgaBin);
}

/****************************************************************************

WV_S32 FPGA_CONF_SetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 FPGA_CONF_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
    WV_U32 data, id;
    WV_U16 x = 0, y = 0, w = 0, h = 0;
    WV_S32 ret = 0;
    if (argc < 1)
    {
        prfBuff += sprintf(prfBuff, "set fpga <cmd>;//cmd like: wincfg/win/dis/alpha/reset/vol/volchl/getvol\r\n");
        return 0;
    }

    if (strcmp(argv[0], "wincfg") == 0)
    {

        if (argc < 2)
        {

            prfBuff += sprintf(prfBuff, "set fpga wincfg <win.json>\r\n");
        }
        if (strstr(argv[1], "win") == NULL || strstr(argv[1], "json") == NULL)
        {
            prfBuff += sprintf(prfBuff, "err!cmd like: set fpga wincfg <win.json>\r\n");
            return 0;
        }
        FPGA_CONF_WIN_T win[16];
        memset(win, 0, sizeof(FPGA_CONF_WIN_T) * 16);

        if (FPGA_CONF_WinAnalysisJson(argv[1], win) == 0)
        {
            WV_S32 i;
            for(i=0;i<16;i++){
                win[i].sdpInfo.audio_chl_num = 2;
            }
            FPGA_CONF_SetWin(win);
        }
        return 0;
    }
    else if (strcmp(argv[0], "win") == 0)
    {

        if (argc < 6)
        {
            prfBuff += sprintf(prfBuff, "set fpga win <id> <x> <y> <w> <h> \r\n");
            return WV_SOK;
        }
        //get win.id
        ret = WV_STR_S2v(argv[1], &id);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        if (id >= 16)
        {
            prfBuff += sprintf(prfBuff, "input id err!\r\n");
            return WV_SOK;
        }
        //get win x
        ret = WV_STR_S2v(argv[2], &data);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        x = data;
        //get win y
        ret = WV_STR_S2v(argv[3], &data);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        y = data;
        //get win w
        ret = WV_STR_S2v(argv[4], &data);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        w = data;
        //get win h
        ret = WV_STR_S2v(argv[5], &data);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        h = data;

        prfBuff += sprintf(prfBuff, "set win[%d],x=%d,y=%d,w=%d,h=%d\n", id, x, y, w, h);
        WV_U16 baseAddr = 0x0500;
        WV_U16 regAddr = 0;

        regAddr = ((baseAddr >> 4) + id) << 4;

        prfBuff += sprintf(prfBuff, "ret 0x%04x=0x%04x[%d]\r\n", regAddr, x, x);
        prfBuff += sprintf(prfBuff, "ret 0x%04x=0x%04x[%d]\r\n", regAddr + 1, y, y);
        prfBuff += sprintf(prfBuff, "ret 0x%04x=0x%04x[%d]\r\n", regAddr + 2, w, w);
        prfBuff += sprintf(prfBuff, "ret 0x%04x=0x%04x[%d]\r\n", regAddr + 3, h, h);

        ret += HIS_SPI_FpgaWd(regAddr, x);
        ret += HIS_SPI_FpgaWd(regAddr + 1, y);
        ret += HIS_SPI_FpgaWd(regAddr + 2, w);
        ret += HIS_SPI_FpgaWd(regAddr + 3, h);

        WV_U16 winEna = 0;
        HIS_SPI_FpgaRd(0x604, &winEna);
        winEna |= 1 << id;
        HIS_SPI_FpgaWd(0x604, winEna);
    }
    else if (strcmp(argv[0], "dis") == 0)
    {

        if (argc < 2)
        {

            prfBuff += sprintf(prfBuff, "set fpga dis <num>//num(0)4k60 1<4k50>  2<4k30> 3<1080p60> 4<1080p50> 5<1080i60> 6<1080i30> 7<1080P30>\r\n");
            return WV_SOK;
        }
        ret = WV_STR_S2v(argv[1], &data);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        prfBuff += sprintf(prfBuff, "set fpga dis =%d\r\n", data);
        //FPGA_CONF_SetResolution(data);
        FPGA_CONF_Resolution(data);
    }
    else if (strcmp(argv[0], "alpha") == 0)
    {

        if (argc < 2)
        {

            prfBuff += sprintf(prfBuff, "set fpga alpha <val>//val取值范围0～128\r\n");
            return WV_SOK;
        }
        ret = WV_STR_S2v(argv[1], &data);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        FPGA_CONF_SetDisAlpha((WV_S32)data);
    }
    else if (strcmp(argv[0], "reset") == 0)
    {
        PCA9555_Clr(0x3, 0x2);
        usleep(500000);
        PCA9555_Set(0x3, 0x2);
    }
    else if (strcmp(argv[0], "vol") == 0)
    {
        if (argc < 2)
        {

            prfBuff += sprintf(prfBuff, "set fpga vol <val>//val取值范围0～15,设置音量输出放大倍数\r\n");
            return WV_SOK;
        }
        WV_U32 multiple;
        ret = WV_STR_S2v(argv[1], &multiple);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "\r\ninput erro!set fpga vol <val>//val取值范围0～15,设置音量输出放大倍数\r\n");
            return WV_SOK;
        }

        FPGA_CONF_SetOutPutVolumeMultiple(multiple);
        prfBuff += sprintf(prfBuff, "设置音量输出放大倍数 %d 倍\r\n", multiple);
    }
    else if (strcmp(argv[0], "volchl") == 0)
    {
        if (argc < 3)
        {

            prfBuff += sprintf(prfBuff, "set fpga volchl <winID> <chl> \r\n");
            return WV_SOK;
        }
        WV_U32 winID;
        WV_U32 chl;
        ret = WV_STR_S2v(argv[1], &winID);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "\r\ninput erro!! [set fpga volchl <winID> <chl>]\r\n");
            return WV_SOK;
        }
        ret = WV_STR_S2v(argv[2], &chl);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "\r\ninput erro![set fpga volchl <winID> <chl>]\r\n");
            return WV_SOK;
        }
        FPGA_CONF_SetOutPutAudioSel((WV_U16)winID, (WV_U16)chl);
    }

    else
    {
        prfBuff += sprintf(prfBuff, "set fpga <cmd>;//cmd like: wincfg/win/dis/alpha/reset/vol/volchl\r\n");
        return 0;
    }

    return 0;
}

/****************************************************************************

WV_S32 FPGA_CONF_GetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 FPGA_CONF_GetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{

    WV_S32 ret = -1;
    WV_U32 winID;
    if (argc < 1)
    {

        prfBuff += sprintf(prfBuff, "get fpga <cmd>;//cmd like: win/vol\r\n");
        return 0;
    }
    //设置寄存器
    if (strcmp(argv[0], "win") == 0)
    {
        if (argc < 2)
        {

            prfBuff += sprintf(prfBuff, "get fpga win <cmd>;//cmd like:stream/\r\n");
            return 0;
        }

        if (strcmp(argv[1], "stream") == 0)
        {
            if (argc < 3)
            {

                prfBuff += sprintf(prfBuff, "get fpga win stream <winID>;\r\n");
                return 0;
            }
            ret = WV_STR_S2v(argv[2], &winID);
            if (ret != WV_SOK)
            {
                prfBuff += sprintf(prfBuff, "\r\ninput erro!! get fpga win stream <winID>;\r\n");
                return WV_SOK;
            }
            ret = FPGA_CONF_CheckNoStream(winID);
            if (ret == 0)
            {
                prfBuff += sprintf(prfBuff, "\r\n窗口[%d]有视频流\r\n", winID);
            }
            else
            {
                prfBuff += sprintf(prfBuff, "\r\n窗口[%d]无视频流\r\n", winID);
            }
        }
        else if (strcmp(argv[1], "freeze") == 0)
        {
            if (argc < 3)
            {

                prfBuff += sprintf(prfBuff, "get fpga win freeze <winID>;\r\n");
                return 0;
            }
            ret = WV_STR_S2v(argv[2], &winID);
            if (ret != WV_SOK)
            {
                prfBuff += sprintf(prfBuff, "\r\ninput erro!! get fpga win freeze <winID>;\r\n");
                return WV_SOK;
            }
            
            ret = FPGA_CONF_GetWinFreezeVal(winID);
            if(ret == 1){
                prfBuff += sprintf(prfBuff, "\r\n窗口[%d]静帧 \r\n", winID);
            }else{
                prfBuff += sprintf(prfBuff, "\r\n窗口[%d]没有静帧 \r\n", winID);
            }

            return WV_SOK;
            
        }
    }
    else if (strcmp(argv[0], "vol") == 0)
    {
        if (argc < 2)
        {

            prfBuff += sprintf(prfBuff, "get fpga vol <winID> \r\n");
            return WV_SOK;
        }
        ret = WV_STR_S2v(argv[1], &winID);
        WV_U16 vol[8]={0};
        FPGA_CONF_GetVolume(winID,vol);
        prfBuff += sprintf(prfBuff, "get fpga vol win[%d] vol %d %d %d %d %d %d %d %d  \r\n",winID,vol[0],vol[1],vol[2],vol[3],vol[4],vol[5],vol[6],vol[7]);
        return WV_SOK;        
    }
    else
    {
        prfBuff += sprintf(prfBuff, "set fpga <cmd>;//cmd like: win/vol\r\n");
    }
    return WV_SOK;
}

/*******************************************************************
void FPGA_CONF_Init();
fpga初始化
*******************************************************************/
void FPGA_CONF_Init()
{
#if _FPGA_MUTEX_ENA_D

    pthread_mutex_init(&gMutexSetWin, NULL);
    pthread_mutex_init(&gMutexUpdateSdpInfo, NULL);
    
#endif
    FPGA_CONF_Reset();
    gpFpgaConfDev = (FPGA_CONF_DEV *)malloc(sizeof(FPGA_CONF_DEV));
    memset(gpFpgaConfDev, 0, sizeof(FPGA_CONF_DEV));

    HIS_SPI_FpgaWd(0x601, 0x0);
    HIS_SPI_FpgaWd(0x602, 0x0);
    HIS_SPI_FpgaWd(0x603, 0x0);
    FPGA_CONF_SetDisAlpha(100);
#ifdef FPGA_DEBUG
    FPGA_CONF_Resolution(0);
#endif
    //sdp init
    FPGA_SDP_Init();
    FPGA_UPDATE_Init();
    FPGA_IGMP_Open();
    FPGA_VOLUME_Open();
    FGPA_CHECK_Open();
    
#ifdef FPGA_DEBUG
    int i;
    FPGA_CONF_ETH_T ethCfg;
    for (i = 0; i < 4; i++)
    {
        ethCfg.ipv6Ena = 0;
        sprintf(ethCfg.ipv6, "192.168.1.%d", 11 + i);
        sprintf(ethCfg.subnet_mask, "255.255.255.0");
        sprintf(ethCfg.getway, "192.168.1.1");
        sprintf(ethCfg.mac, "80:9f:fb:88:88:%02x", 1 + i);
        FPGA_CONF_SetETH(&ethCfg, i);
    }
#endif
    WV_CMD_Register("set", "fpga", " set fpga", FPGA_CONF_SetCmd);
    WV_CMD_Register("get", "fpga", "pca9555 get reg", FPGA_CONF_GetCmd);

    return;
}
/*******************************************************************
void FPGA_CONF_DeInit();
fpga初始化
*******************************************************************/
void FPGA_CONF_DeInit()
{
#if _FPGA_MUTEX_ENA_D
    pthread_mutex_destroy(&gMutexSetWin);
    pthread_mutex_destroy(&gMutexUpdateSdpInfo);
#endif

    //free(gpFpgaConfDev);
    //sdp init
    FPGA_CHECK_Close();
    FPGA_VOLUME_Close();
    FPGA_IGMP_Close();
    FPGA_UPDATE_DeInit();
    FPGA_SDP_DeInit();
    free(gpFpgaConfDev);
    return;
}