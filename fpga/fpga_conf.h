#ifndef _FPGA_CONF_H_H_
#define _FPGA_CONF_H_H_

#include "wv_common.h"
#include "fpga_sdp.h"
#include "fpga_check.h"
#define FPGA_CONF_WIN_VIDEO_ETH (0)
#define FPGA_CONF_WIN_VIDEO_SDI (1)
#define FPGA_CONF_WIN_VIDEO_HDMI (2)
#define FPGA_CONF_WINNUM_D (16)
#define FPGA_CONF_ETHNUM_D (4)
#define FPGA_CONF_SRCNUM_D (4)
#define FPGA_CONF_IPLEN  (64)
#define FPGA_CONF_MACLEN (24)
#define FPGA_CONF_SDP_DATALEN (2048)

typedef struct FPGA_CONF_WIN
{
    WV_U16 win_ena; //窗口是否有效
    WV_U16 x;
    WV_U16 y;
    WV_U16 w;
    WV_U16 h;
    WV_S8 type[16];  //"eth" /"sdi" /"hdmi" //暂时没用
    WV_U16 channel;  //代表第几路输入
    WV_U16 IGMPVersion;    //IGMP版本
    WV_U16 video_ipv6_ena;  //0 ipv4;  1 ipv6;
    WV_S8  video_ip[FPGA_CONF_IPLEN];
    WV_S8  video_igmpSrc[FPGA_CONF_IPLEN]; //如果是组播地址，则需要设置源ip地址(如果没有设置源地址，则默认为0)
    WV_U16 video_port;   
    WV_U16 audio_ipv6_ena;//0 ipv4;1 ipv6
    WV_S8  audio_ip[FPGA_CONF_IPLEN];
    WV_S8  audio_igmpSrc[FPGA_CONF_IPLEN]; 
    WV_U16 audio_port;
    WV_U16 sdp_ena;   //sdp是否有效
    FPGA_SDP_Info sdpInfo; //sdp信息
}FPGA_CONF_WIN_T;

//源地址只包含ip和端口号
typedef struct FPGA_CONF_SRC_ADDR
{
    WV_U16 ena;
    WV_U16 ipv4OrIpv6;
    WV_S8 ipv6[FPGA_CONF_IPLEN];
    WV_U16 port;
    WV_U16 video_interlace; //0:逐行 1:隔行
    
} FPGA_CONF_SRC_ADDR_T;

//本地网卡信息设置
typedef struct FPGA_CONF_ETH_T
{
    WV_U32 ipv6Ena;
    WV_S8 ipv6[FPGA_CONF_IPLEN];       //如果是ip4，只用前四个字节 例如 :192.168.1.10 ipv6[0] = 0x0B ,ipv6[1] = 0x10，ipv6[2] = 0xC0,ipv6[3]=0xA8
    WV_S8 mac[FPGA_CONF_MACLEN];         //例如:80-9f-fb-88-88-00 mac[0]=0x00;mac[1]=0x88;mac[2]=0x80;mac[3]=0xfb;mac[4]=0x9f;mac[5]=0x80
    WV_S8 subnet_mask[FPGA_CONF_IPLEN]; //子网掩码//例如:255.255.0.0，mask[0] = 0;mask[1]=0;mask[1]=0xff;mask[1]=0xff;
    WV_S8 getway[FPGA_CONF_IPLEN];      //网关//
}FPGA_CONF_ETH_T;

typedef struct FPGA_CONF_VER
{
    WV_U16 year;
    WV_U16 month;
    WV_U16 day;
    WV_U16 ver;
}FPGA_CONF_VER;
typedef struct FPGA_CONF_DEV
{
    WV_S8 fpgaVer[24];
    FPGA_CONF_ETH_T eth[FPGA_CONF_ETHNUM_D];
} FPGA_CONF_DEV;
//查询对应网卡的ip和mac
WV_S32 FPGA_CONF_GetEthInt(WV_U8 ip[],WV_U8 mac[],WV_S32 ethID);

//查询fpga版本号
WV_S32 FPGA_CONF_GetVersion(FPGA_CONF_VER *pFpgaVer);

//设置16个窗口的信息，必须时一次设置16个窗口
WV_S32 FPGA_CONF_SetWin(FPGA_CONF_WIN_T winArray[]);


//设置fpga万兆网卡信息
WV_S32 FPGA_CONF_SetETH(FPGA_CONF_ETH_T *pEth,WV_S32 ethID);
//查询fpga万兆网卡信息
WV_S32 FPGA_CONF_GetETH(FPGA_CONF_ETH_T *pEth,WV_S32 ethID);

/*************************************************************
 * WV_S32 FPGA_CONF_Resolution(WV_S32 resolution);
 * 函数说明:设置视频输出参数(分辨率，输出色位宽，色彩空间)
 * 参数说明:
 *     Resolution(分辨率):     0 4k60;    1 4k50;     2 4k30;    3 1080p   其他:不支持
 *     colorDepth(输出色位宽):  0 8bit;    1 10bit;    其他:不支持
 *     colorSpace(色彩空间):    0 709;     1 2020;     其他:不支持
 * 返回值： 0代表成功，其他代表失败      
*************************************************************/
WV_S32 FPGA_CONF_Resolution(WV_S32 resolution);


/*************************************************************
 * void FPGA_CONF_SetOutPutAudioSel(WV_S32 audioSel，WV_S32 audioChl);
 * 函数说明:设置音频输出参数(选通哪一路窗口)
 * 参数说明:
 *    winID(选择哪个窗口): 取值范围[0~15]
 *    audioChl:声道信息来自该窗口查询的sdp信息的audiochl
 *        audioChl供使用4个bit,每个bit0～bit3 音频控制，bit0-bit3为通道使能，每一个bit对应2个声道，
 * ***********************************************************/
void FPGA_CONF_SetOutPutAudioSel(WV_U16 winID,WV_U16 audioChl); 


/*****************************************************************
 * void FPGA_CONF_SetOutPutVolume(WV_S32 volume);
 * 函数说明:设置输音量放大倍数
 * 参数说明:
 *     volume(输出音量):取值范围[0~15],0静音，最大支持15倍放大
 * ***************************************************************/
void FPGA_CONF_SetOutPutVolume(WV_S32 volume);

/*****************************************************************
 * void FPGA_CONF_SetDisAlpha(WV_S32 alpha);
 * 函数说明:设置输出显示音量等信息的透明度
 * 参数说明:
 *     alpha(显示信息叠加透明度):取值范围[0~128],0是完全透明，128时完全显示
 * ***************************************************************/
void FPGA_CONF_SetDisAlpha(WV_S32 alpha);


/****************************************************
 * WV_S32 FPGA_CONF_GetWinFreezeVal(WV_U32 winID)
 * 函数说明：查询窗口的是否静帧
 * 返回值  0：没有静帧
 *        1：视频静帧
 *        -1：查询错误
 * *************************************************/
WV_U32 FPGA_CONF_GetWinFreezeVal(WV_U32 winID);

/****************************************************
 * WV_S32 FPGA_CONF_CheckNoSignal(WV_U32 winID)
 * 函数说明：查询窗口是否有视频流
 * 返回值 0:流播放正常
 *       1:没有视频流
 *      -1:查询错误，包括输入窗口id超出范围，id范围[0~15]
 * *************************************************/
WV_S32 FPGA_CONF_CheckNoSignal(WV_U32 winID);

/****************************************************
 * WV_U16 FPGA_CONF_GetWinAudioStatus(WV_U32 winID)
 * 函数说明：查询当前窗口的音频状态
 * *************************************************/
WV_U16 FPGA_CONF_GetWinAudioStatus(WV_U32 winID);

/****************************************************
 * void FPGA_CONF_SetCheckWinFreezeParams(WV_U32 winID,WV_U16 thousandth,WV_U32 time_s,WV_U16 mutltiple)
 * 函数说明：设置窗口静帧参数
 * 参数说明：thousandth r_sum变化千分比  ，time_s 警告时间，结合音频加倍的倍数
 * *************************************************/
void FPGA_CONF_SetCheckWinFreezeParams(WV_U32 winID, WV_U16 thousandth, WV_U32 time_s, WV_U16 mutltiple);


/****************************************************
 * void FPGA_CONF_SetCheckVolumeToLowParams(WV_U32 winID, float threshold, WV_U32 time_s)
 * 函数说明：设置检测窗口音量过低
 * *************************************************/
void FPGA_CONF_SetCheckVolumeToLowParams(WV_U32 winID, float threshold, WV_U32 time_s);

/****************************************************
 * void FPGA_CONF_SetCheckAudioToHightParams(WV_U32 winID, float threshold, WV_U32 time_s)
 * 函数说明：设置检测窗口音量过高
 * *************************************************/
void FPGA_CONF_SetCheckVolumeToHightParams(WV_U32 winID, float threshold, WV_U32 time_s);

/****************************************************
 * void FPGA_CONF_SetCheckVolumeMuteParams(WV_U32 winID, float threshold, WV_U32 time_s)
 * 函数说明：设置检测窗口音频丢失的参数
 * 参数说明：threshold 门限值 单位dB,time_s 警告时间 秒
 * *************************************************/
void FPGA_CONF_SetCheckVolumeMuteParams(WV_U32 winID, float threshold, WV_U32 time_s);

/*******************************************************************
WV_S32 FPGA_CONF_UpdateFpga(WV_S8 *pFpgaBin);
* fpga升级
* 参数: pFpgabin 为需要升级的fpga二进制文件路径
* 返回值 0:成功
*       其他:失败
*******************************************************************/
WV_S32 FPGA_CONF_UpdateFpga(WV_S8 *pFpgaBin);

/****************************************************
 * void FPGA_CONF_SetIgmpSendSecond(WV_U32 sec)
 * 函数功能：设置组播发送时间间隔
 * 参说说明：sec 时间，单位 秒
 * *************************************************/
void FPGA_CONF_SetIgmpSendSecond(WV_U32 sec);

/****************************************************************************
void FPGA_CONF_GetVolume(WV_U16 winID,WV_U16 volume[])
函数说明：查询某个窗口的音量
参数说明：
    winID：窗口id
    volume：获取到的音量值，8字节数组，代表8个声道
****************************************************************************/
void FPGA_CONF_GetVolume(WV_U16 winID,WV_U16 volume[]);


/**************************************************************
 * void FPGA_CONF_SendArpRequest(WV_U16 ethID,WV_S8 *targetIp)
 * 函数说明：发送arp请求
 * 参数说明：
 *     ethID   ：网卡id
 *     targetIp：目标ip地址
 * ************************************************************/
void FPGA_CONF_SendArpRequest(WV_U16 ethID,WV_S8 *targetIp);


/**************************************************************
 * void FPGA_CONF_GetSdpInfo(WV_U16 winID,FPGA_SDP_Info *pSdpInfo)
 * 函数说明：发送arp请求
 * ************************************************************/
void FPGA_CONF_GetSdpInfo(WV_U16 winID,FPGA_SDP_Info *pSdpInfo);

/***************************************************************
 * void FPGA_CONF_SetInfoCntColor(WV_U16 r,WV_U16 g,WV_U16 b)
 * //设置窗口信息字体颜色
 * *************************************************************/
void FPGA_CONF_SetInfoCntColor(WV_U16 r,WV_U16 g,WV_U16 b);

/**************************************************************
 * void FPGA_CONF_SetInfoCntPos(WV_U16 winID,WV_U16 startX,WV_U16 startY,WV_U16 endX,WV_U16 endY)
 * //设置窗口字体的坐标值
 * ************************************************************/
void FPGA_CONF_SetInfoCntPos(WV_U16 winID,WV_U16 startX,WV_U16 startY,WV_U16 endX,WV_U16 endY);

void FPGA_CONF_Init();
void FPGA_CONF_DeInit();

#endif